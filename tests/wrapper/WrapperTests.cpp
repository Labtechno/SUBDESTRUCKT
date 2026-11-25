// SUBDESTRUCKT — tests du wrapper JUCE (processeur instancié directement, sans client VST3).
// G-18w : 0 allocation pendant AudioProcessor::processBlock (notes, automation, transactions, Panic, 4 fréquences).
// G-21w : état versionné (paramètres exacts + topologie, identifiants et classes) ; refus d'un état invalide.
// G-22w : identité des paramètres (StableID, identifiants VST3 = liste de référence calculée indépendamment).
// G-06w / G-19w : latence déclarée, instances multiples.
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <new>
#include <random>
#include <set>
#include <sstream>

#include "TestKit.h"
#include "plugin/PluginProcessor.h"
#include "ui/DiagnosticEditor.h"

void* operator new(std::size_t n) {
  if (tk::countingAllocations().load(std::memory_order_relaxed)) tk::allocationCount().fetch_add(1);
  if (void* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void* operator new[](std::size_t n) { return operator new(n); }
void operator delete(void* p) noexcept {
  if (p && tk::countingAllocations().load(std::memory_order_relaxed)) tk::allocationCount().fetch_add(1);
  std::free(p);
}
void operator delete[](void* p) noexcept { operator delete(p); }
void operator delete(void* p, std::size_t) noexcept { operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { operator delete(p); }

using namespace sbd;

namespace {

const double kRates[4] = {44100.0, 48000.0, 88200.0, 96000.0};

struct Rendered {
  std::vector<float> L;
  long long allocations = 0;
};

// Rendu d'un processeur avec un scénario MIDI fixe (offsets absolus), blocs de taille donnée.
Rendered renderProcessor(SubdestrucktProcessor& p, double fs, int block, int samples,
                         const std::vector<std::pair<int, juce::MidiMessage>>& events,
                         const std::function<void(int pos)>& beforeBlock = {}) {
  Rendered r;
  p.prepareToPlay(fs, block);
  juce::AudioBuffer<float> buf(2, block);
  juce::MidiBuffer midi;
  midi.ensureSize(4096);
  r.L.assign(static_cast<size_t>(samples), 0.0f);
  for (int pos = 0; pos < samples; pos += block) {
    const int n = std::min(block, samples - pos);
    if (beforeBlock) beforeBlock(pos);
    midi.clear();
    for (const auto& e : events)
      if (e.first >= pos && e.first < pos + n) midi.addEvent(e.second, e.first - pos);
    buf.setSize(2, n, false, false, true);
    buf.clear();
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    p.processBlock(buf, midi);
    tk::countingAllocations().store(false);
    r.allocations += tk::allocationCount().load();
    std::memcpy(&r.L[static_cast<size_t>(pos)], buf.getReadPointer(0), sizeof(float) * static_cast<size_t>(n));
  }
  return r;
}

std::vector<std::pair<int, juce::MidiMessage>> notes(double fs) {
  return {{static_cast<int>(0.05 * fs), juce::MidiMessage::noteOn(1, 60, static_cast<juce::uint8>(100))},
          {static_cast<int>(0.30 * fs), juce::MidiMessage::noteOn(1, 64, static_cast<juce::uint8>(90))},
          {static_cast<int>(0.50 * fs), juce::MidiMessage::noteOff(1, 64)},
          {static_cast<int>(0.70 * fs), juce::MidiMessage::noteOff(1, 60)},
          {static_cast<int>(0.75 * fs), juce::MidiMessage::controllerEvent(1, 120, 0)},
          {static_cast<int>(0.80 * fs), juce::MidiMessage::noteOn(1, 67, static_cast<juce::uint8>(120))}};
}

bool sameFloats(const std::vector<float>& a, const std::vector<float>& b) {
  return a.size() == b.size() && std::memcmp(a.data(), b.data(), a.size() * sizeof(float)) == 0;
}

void testRealtimeAndLatency() {
  long long total = 0;
  bool latOk = true;
  for (double fs : kRates) {
    for (int block : {32, 512}) {
      SubdestrucktProcessor p;
      std::mt19937 rng(static_cast<unsigned>(fs) + static_cast<unsigned>(block));
      int step = 0;
      auto automation = [&](int pos) {
        // thread de contrôle simulé : automation hôte et transactions de topologie entre deux blocs
        if (pos % (block * 16) == 0) {
          auto* prm = p.parameters().param(static_cast<int>(rng() % kNumParams));
          prm->setValueNotifyingHost(static_cast<float>(rng() % 1000) / 999.0f);
        }
        if (pos > 0 && pos % static_cast<int>(0.1 * fs) < block) {
          const int slot = (step++) % kMaxRoutes;
          const auto& info = routeSlotInfo(slot);
          bool present = false;
          for (const auto& r : p.currentRoutes())
            if (r.src == info.src && r.dst == info.dst) {
              p.applyTopology({RouteTable::Op::remove(r.id)});
              present = true;
            }
          if (!present) p.applyTopology({RouteTable::Op::add(info.src, info.dst)});
        }
        if (pos == static_cast<int>(0.6 * fs) / block * block) p.requestPanic();
      };
      const auto r = renderProcessor(p, fs, block, static_cast<int>(fs), notes(fs), automation);
      total += r.allocations;
      const int expected = fs == 44100.0 ? 32 : 0;
      latOk = latOk && p.getLatencySamples() == expected;
      bool finite = true;
      for (float v : r.L) finite = finite && std::isfinite(v);
      TK_CHECK(finite);
    }
  }
  TK_CHECK(total == 0);
  tk::result("G-18w", total == 0, "AudioProcessor::processBlock : %lld allocation(s)/liberation(s) (4 frequences x blocs 32/512, notes, CC120, automation, transactions de topologie)",
             total);
  TK_CHECK(latOk);
  tk::result("G-06w", latOk, "latence declaree a l'hote (setLatencySamples) : 32 a 44,1 kHz, 0 a 48/88,2/96 kHz");
}

void testState() {
  bool ok = true;
  SubdestrucktProcessor a;
  std::mt19937 rng(1234);
  for (int i = 0; i < kNumParams; ++i)
    a.parameters().param(i)->setValueNotifyingHost(static_cast<float>(rng() % 10007) / 10006.0f);
  // topologie non triviale : retrait d'une route, ajouts (dont une fermeture de cycle -> D)
  const auto routes = a.currentRoutes();
  ok = ok && a.applyTopology({RouteTable::Op::remove(routes.front().id), RouteTable::Op::add(kNodeOP1, kNodeOP2),
                              RouteTable::Op::add(kNodeOP2, kNodeOP2)}).ok;
  juce::MemoryBlock blobA;
  a.getStateInformation(blobA);
  SubdestrucktProcessor b;
  b.setStateInformation(blobA.getData(), static_cast<int>(blobA.getSize()));
  juce::MemoryBlock blobB;
  b.getStateInformation(blobB);
  const bool sameBlob = blobA == blobB;
  // les routes (identifiants et classes) sont restaurées telles quelles
  const auto ra = a.currentRoutes(), rb = b.currentRoutes();
  const bool sameRoutes = ra == rb && a.currentNextRouteId() == b.currentNextRouteId();
  // rendu identique bit à bit (deux processeurs neufs, même état)
  const auto outA = renderProcessor(a, 48000.0, 256, 48000, notes(48000.0));
  const auto outB = renderProcessor(b, 48000.0, 256, 48000, notes(48000.0));
  const bool sameAudio = sameFloats(outA.L, outB.L);
  // état invalide : refus complet, état courant conservé
  juce::MemoryBlock before;
  b.getStateInformation(before);
  const std::string bad = "SUBDESTRUCKT_STATE 1\ntopology next=3\nroute 1 WT OP1 I\nroute 2 OP1 WT I\nend\n";
  b.setStateInformation(bad.data(), static_cast<int>(bad.size()));
  juce::MemoryBlock after;
  b.getStateInformation(after);
  const bool refused = before == after && juce::String(b.lastMessage()).startsWith("Etat refuse");
  const std::string future = "SUBDESTRUCKT_STATE 2\nend\n";
  b.setStateInformation(future.data(), static_cast<int>(future.size()));
  juce::MemoryBlock after2;
  b.getStateInformation(after2);
  const bool refusedFuture = before == after2;
  ok = ok && sameBlob && sameRoutes && sameAudio && refused && refusedFuture;
  TK_CHECK(ok);
  tk::result("G-21w", ok, "etat du wrapper : blob identique apres rechargement (%s), routes/ids/classes (%s), rendu bit a bit (%s), etat invalide refuse (%s), version future refusee (%s)",
             sameBlob ? "oui" : "NON", sameRoutes ? "oui" : "NON", sameAudio ? "oui" : "NON", refused ? "oui" : "NON",
             refusedFuture ? "oui" : "NON");
}

void testIdentity(const std::string& expectedFile) {
  std::map<std::string, juce::uint32> expected;
  std::ifstream in(expectedFile);
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream ls(line);
    std::string id;
    unsigned long v = 0;
    ls >> id >> v;
    expected[id] = static_cast<juce::uint32>(v);
  }
  SubdestrucktProcessor p;
  const auto& params = p.getParameters();
  bool ok = static_cast<int>(params.size()) == kNumParams && static_cast<int>(expected.size()) == kNumParams;
  std::set<juce::uint32> seen;
  for (int i = 0; i < params.size(); ++i) {
    auto* wp = dynamic_cast<juce::AudioProcessorParameterWithID*>(params[i]);
    ok = ok && wp != nullptr && wp->paramID == juce::String(paramSpec(i).stableId);
    const auto vst3 = juce::VST3ClientExtensions::convertJuceParameterId(wp->paramID, true);
    ok = ok && expected.count(paramSpec(i).stableId) && expected[paramSpec(i).stableId] == vst3;
    seen.insert(vst3);
  }
  ok = ok && static_cast<int>(seen.size()) == kNumParams;
  TK_CHECK(ok);
  tk::result("G-22w", ok, "%d parametres : identite hote = StableID ; identifiants VST3 (calcul JUCE) = liste de reference calculee independamment ; tous distincts",
             static_cast<int>(params.size()));
}

void testInstances() {
  SubdestrucktProcessor a, b;
  b.parameters().param(static_cast<int>(ParamId::DepthOp2Op1))->setValueNotifyingHost(0.6f);
  SubdestrucktProcessor a2, b2;
  b2.parameters().param(static_cast<int>(ParamId::DepthOp2Op1))->setValueNotifyingHost(0.6f);
  const auto ra = renderProcessor(a2, 48000.0, 128, 24000, notes(48000.0));
  const auto rb = renderProcessor(b2, 48000.0, 128, 24000, notes(48000.0));
  // entrelacé
  a.prepareToPlay(48000.0, 128);
  b.prepareToPlay(48000.0, 128);
  std::vector<float> la(24000), lb(24000);
  juce::AudioBuffer<float> buf(2, 128);
  juce::MidiBuffer midi;
  const auto ev = notes(48000.0);
  for (int pos = 0; pos < 24000; pos += 128) {
    const int n = std::min(128, 24000 - pos);
    for (int k = 0; k < 2; ++k) {
      midi.clear();
      for (const auto& e : ev)
        if (e.first >= pos && e.first < pos + n) midi.addEvent(e.second, e.first - pos);
      buf.setSize(2, n, false, false, true);
      buf.clear();
      (k == 0 ? a : b).processBlock(buf, midi);
      std::memcpy(&(k == 0 ? la : lb)[static_cast<size_t>(pos)], buf.getReadPointer(0), static_cast<size_t>(n) * sizeof(float));
    }
  }
  const bool ok = sameFloats(la, ra.L) && sameFloats(lb, rb.L) && !sameFloats(la, lb);
  TK_CHECK(ok);
  tk::result("G-19w", ok, "deux processeurs entrelaces : sorties identiques aux rendus separes, instances isolees");
}

void testPanicRequest() {
  SubdestrucktProcessor p;
  p.prepareToPlay(48000.0, 256);
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  p.requestPanic();
  p.processBlock(buf, midi);
  const bool ok = p.telemetry().panics.load() == 1;
  TK_CHECK(ok);
  tk::result("G-16w", ok, "Panic demande par l'UI : transmis au moteur comme evenement de securite (offset 0)");
}

// Capture PNG de l'UI de diagnostic (contrôle visuel, non bloquant) après quelques blocs joués à 44,1 kHz.
void snapshotEditor(const std::string& path) {
  SubdestrucktProcessor p;
  p.prepareToPlay(44100.0, 256);
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  midi.addEvent(juce::MidiMessage::noteOn(1, 57, static_cast<juce::uint8>(110)), 0);
  for (int i = 0; i < 60; ++i) {
    buf.clear();
    p.processBlock(buf, midi);
    midi.clear();
  }
  std::unique_ptr<juce::AudioProcessorEditor> ed(p.createEditorIfNeeded());
  auto* diag = dynamic_cast<sbd::DiagnosticEditor*>(ed.get());
  if (diag == nullptr) {
    tk::info("UI : editeur indisponible");
    return;
  }
  diag->setSize(1080, 720);
  diag->refreshNow();
  const auto img = diag->createComponentSnapshot(diag->getLocalBounds(), true, 1.0f);
  juce::File f(juce::String::fromUTF8(path.c_str()));
  f.deleteFile();
  bool ok = false;
  {
    juce::FileOutputStream os(f);
    juce::PNGImageFormat png;
    ok = os.openedOk() && png.writeImageToStream(img, os);
  }
  tk::info("UI : capture %dx%d -> %s (%s)", img.getWidth(), img.getHeight(), path.c_str(), ok ? "ecrite" : "ECHEC d'ecriture");
}

}  // namespace

int main(int argc, char** argv) {
  juce::ScopedJuceInitialiser_GUI juceInit;  // MessageManager (timers du processeur)
  std::printf("SUBDESTRUCKT ARCH-G3 — tests du wrapper JUCE\n");
  const std::string expected = argc > 1 ? argv[1] : "tests/host/expected_vst3_param_ids.txt";
  std::fprintf(stderr, "[temps reel et latence]\n");
  testRealtimeAndLatency();
  std::fprintf(stderr, "[etat]\n");
  testState();
  std::fprintf(stderr, "[identite]\n");
  testIdentity(expected);
  std::fprintf(stderr, "[instances]\n");
  testInstances();
  std::fprintf(stderr, "[panic]\n");
  testPanicRequest();
  if (argc > 2) {
    std::fprintf(stderr, "[capture UI]\n");
    snapshotEditor(argv[2]);
  }
  return tk::finish("wrapper_tests");
}
