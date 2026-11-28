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
#include "ui/MainEditor.h"

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
  const std::string future = "SUBDESTRUCKT_STATE " + std::to_string(kStateVersion + 1) + "\nend\n";
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

// CAP-02w : sauvegarde DAW autonome d'un gel D2 armé (capture à la sauvegarde), restauration dans une autre instance.
// ARCH-G4.2 : étape explicite « Autoriser » insérée (résultat historique G4.1 run 33 : gel rejoué sans autorisation).
void testCapturesWrapper() {
  auto setP = [](SubdestrucktProcessor& p, ParamId id, float v) {
    auto* q = p.parameters().param(idx(id));
    q->setValueNotifyingHost(q->convertTo0to1(v));
  };
  SubdestrucktProcessor a;
  a.prepareToPlay(48000.0, 256);
  setP(a, ParamId::DstGl1Type, static_cast<float>(DstType::D2));
  setP(a, ParamId::DstGl1Arm, 1.0f);
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  midi.addEvent(juce::MidiMessage::noteOn(1, 52, static_cast<juce::uint8>(120)), 0);
  for (int i = 0; i < 300; ++i) {
    if (i == 150) setP(a, ParamId::DstGl1D2Freeze, 1.0f);
    buf.clear();
    a.processBlock(buf, midi);
    midi.clear();
  }
  // sauvegarde : la capture est demandée ; le bloc audio suivant la réalise (hôte simulé : même thread)
  a.requestCapture(4);
  buf.clear();
  a.processBlock(buf, midi);
  const bool got = a.collectCaptures(1u << 4) == (1u << 4);
  juce::MemoryBlock blob;
  a.getStateInformation(blob);
  const bool inBlob = juce::String(static_cast<const char*>(blob.getData()), blob.getSize()).contains("capture 4 D2");
  SubdestrucktProcessor b;
  b.prepareToPlay(48000.0, 256);
  b.setStateInformation(blob.getData(), static_cast<int>(blob.getSize()));
  const bool same = b.captures() == a.captures() && !b.captures().empty();
  // ARCH-G4.2 (correction de la non-conformite DAW de CAP-01) : restauration suspendue, silence, puis Autoriser
  double e0 = 0.0;
  for (int i = 0; i < 100; ++i) {
    buf.clear();
    b.processBlock(buf, midi);
    for (int k = 0; k < 256; ++k) e0 += static_cast<double>(buf.getSample(0, k)) * buf.getSample(0, k);
  }
  const bool held = b.autonomySuspended() && e0 < 1e-9;
  b.authorizeAutonomy();
  double e = 0.0;
  for (int i = 0; i < 100; ++i) {
    buf.clear();
    b.processBlock(buf, midi);
    if (i >= 50)
      for (int k = 0; k < 256; ++k) e += static_cast<double>(buf.getSample(0, k)) * buf.getSample(0, k);
  }
  const bool sounds = e > 1e-3;
  const bool ok = got && inBlob && same && held && sounds;
  TK_CHECK(ok);
  tk::result("CAP-02w", ok, "sauvegarde DAW d'un gel D2 arme : capture %d, presente dans l'etat %d, restauree a l'identique %d ; G4.2 : suspendue et silencieuse avant autorisation %d (energie %.3g), gel rejoue sans entree apres Autoriser %d (energie %.3g)",
             got, inBlob, same, held, e0, sounds, e);
}

// WT-ED-02w : génération et édition de banques via le processeur ; chaque opération est annulable.
void testBankEditingWrapper() {
  SubdestrucktProcessor p;
  p.prepareToPlay(48000.0, 256);
  const std::string b0 = p.currentBank();
  const bool g = p.generateBank(WtProcedure::HarmonicTilt, 16, 4, 3, 0.5);
  const std::string b1 = p.currentBank();
  std::vector<float> w(WtBank::kLength);
  for (int i = 0; i < WtBank::kLength; ++i) w[static_cast<size_t>(i)] = i < WtBank::kLength / 2 ? 1.0f : -1.0f;
  const bool e = p.editCurrentWave(5, w);
  const std::string b2 = p.currentBank();
  auto bank2 = resolveBank(b2);
  const bool content = bank2 && std::equal(w.begin(), w.end(), bank2->raw().begin() + 5 * WtBank::kLength);
  const bool u1 = p.undo() && p.currentBank() == b1;
  const bool u2 = p.undo() && p.currentBank() == b0;
  // rendu sans allocation après changement de banque utilisateur
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  midi.addEvent(juce::MidiMessage::noteOn(1, 48, static_cast<juce::uint8>(100)), 0);
  p.redo();
  p.processBlock(buf, midi);
  const bool ok = g && e && b1.rfind("user:", 0) == 0 && b2 != b1 && content && u1 && u2 && p.currentBank() == b1;
  TK_CHECK(ok);
  tk::result("WT-ED-02w", ok, "processeur : generation (%s), edition d'une forme (%s, contenu exact %d), Undo x2 retour a %s, Redo %d",
             b1.c_str(), b2.c_str(), content, b0.c_str(), p.currentBank() == b1);
}

// UI-01w : cohérence des trois espaces (naviguer, sélectionner, redimensionner ne modifie jamais l'état), titres
// accessibles des contrôles de paramètres, couverture de l'UI (part des paramètres atteignables par un contrôle).
void collectComponents(juce::Component& c, std::vector<juce::Component*>& out) {
  out.push_back(&c);
  for (auto* ch : c.getChildren()) collectComponents(*ch, out);
}

void testUiCoherence() {
  SubdestrucktProcessor p;
  p.prepareToPlay(48000.0, 256);
  std::unique_ptr<juce::AudioProcessorEditor> ed(p.createEditorIfNeeded());
  auto* main = dynamic_cast<sbd::ui::MainEditor*>(ed.get());
  if (main == nullptr) {
    tk::result("UI-01w", false, "editeur indisponible");
    return;
  }
  main->setSize(1440, 900);
  juce::MemoryBlock before;
  p.getStateInformation(before);
  bool layoutOk = true;
  for (int space = 0; space < 3; ++space)
    for (int tab = 0; tab < 6; ++tab) {
      main->showSubTab(space, tab);
      main->refreshNow();
    }
  for (auto sz : {std::pair<int, int>{1000, 700}, {1920, 1200}, {1440, 900}}) {
    main->setSize(sz.first, sz.second);
    main->refreshNow();
    layoutOk = layoutOk && main->getWidth() == sz.first && main->getHeight() == sz.second;
  }
  for (int k = 0; k < kNumParams; k += 7) main->selectParam(static_cast<ParamId>(k));  // Inspector
  main->refreshNow();
  juce::MemoryBlock after;
  p.getStateInformation(after);
  const bool unchanged = before == after;
  // contrôles de paramètres (HelpText = StableID) : titre accessible = libellé du registre ; couverture
  std::vector<juce::Component*> all;
  for (int space = 0; space < 3; ++space)
    for (int tab = 0; tab < 6; ++tab) {
      main->showSubTab(space, tab);
      main->refreshNow();
      collectComponents(*main, all);
    }
  std::set<std::string> covered;
  int controls = 0, untitled = 0;
  for (auto* c : all) {
    const std::string help = c->getHelpText().toStdString();
    ParamId id;
    if (help.empty() || !findParam(help, id)) continue;
    ++controls;
    covered.insert(help);
    if (c->getTitle().toStdString() != paramSpec(id).name) ++untitled;
  }
  const bool ok = layoutOk && unchanged && untitled == 0 && controls > 0;
  TK_CHECK(ok);
  tk::result("UI-01w", ok, "3 espaces x 6 sous-onglets, 3 tailles, %d selections Inspector : etat inchange %d, tailles respectees %d ; %d controles de parametres, titres accessibles manquants %d ; %zu/%d parametres atteignables par un controle visible",
             (kNumParams + 6) / 7, unchanged, layoutOk, controls, untitled, covered.size(), kNumParams);
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
  auto* main = dynamic_cast<sbd::ui::MainEditor*>(ed.get());
  if (main == nullptr) {
    tk::info("UI : editeur indisponible");
    return;
  }
  main->setSize(1440, 900);
  // captures reelles des trois espaces (et des sous-onglets principaux)
  const struct {
    int space, tab;
    const char* suffix;
  } views[] = {{0, 0, ""}, {1, 0, "_sound_process"}, {1, 1, "_sound_mod"}, {2, 0, "_eng_network"}, {2, 1, "_eng_mod"}, {2, 2, "_eng_banks"}, {2, 3, "_eng_random"}, {2, 5, "_eng_diag"}};
  for (const auto& v : views) {
    main->showSubTab(v.space, v.tab);
    main->refreshNow();
    const auto img = main->createComponentSnapshot(main->getLocalBounds(), true, 1.0f);
    std::string out = path;
    const size_t dot = out.rfind(".png");
    if (dot != std::string::npos) out.insert(dot, v.suffix);
    juce::File f(juce::String::fromUTF8(out.c_str()));
    f.deleteFile();
    bool ok = false;
    {
      juce::FileOutputStream os(f);
      juce::PNGImageFormat png;
      ok = os.openedOk() && png.writeImageToStream(img, os);
    }
    tk::info("UI : capture %dx%d -> %s (%s)", img.getWidth(), img.getHeight(), out.c_str(), ok ? "ecrite" : "ECHEC d'ecriture");
  }
}

// ------------------------------------------------------------------ ARCH-G4.2 : AUT-02..08 (états autonomes)
void setPv(SubdestrucktProcessor& p, ParamId id, float v) {
  auto* q = p.parameters().param(idx(id));
  q->setValueNotifyingHost(q->convertTo0to1(v));
}
double runSilent(SubdestrucktProcessor& p, int blocks, int skip = 0) {  // énergie sans note (canal gauche + droit)
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  double e = 0.0;
  for (int i = 0; i < blocks; ++i) {
    buf.clear();
    p.processBlock(buf, midi);
    if (i >= skip)
      for (int ch = 0; ch < 2; ++ch)
        for (int k = 0; k < 256; ++k) e += static_cast<double>(buf.getSample(ch, k)) * buf.getSample(ch, k);
  }
  return e;
}
// gel D2 armé (GL1, emplacement 4) et gel S2 armé (GL2, emplacement 5), joués puis gelés pendant la note
void buildFrozen(SubdestrucktProcessor& p, float s2Decay = -1.0f) {
  p.prepareToPlay(48000.0, 256);
  if (s2Decay >= 0.0f) setPv(p, ParamId::DstGl2S2Decay, s2Decay);
  setPv(p, ParamId::DstGl1Type, static_cast<float>(DstType::D2));
  setPv(p, ParamId::DstGl1Arm, 1.0f);
  setPv(p, ParamId::DstGl2Type, static_cast<float>(DstType::S2));
  setPv(p, ParamId::DstGl2Arm, 1.0f);
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  midi.addEvent(juce::MidiMessage::noteOn(1, 52, static_cast<juce::uint8>(120)), 0);
  for (int i = 0; i < 300; ++i) {
    if (i == 150) {
      setPv(p, ParamId::DstGl1D2Freeze, 1.0f);
      setPv(p, ParamId::DstGl2S2Freeze, 1.0f);
    }
    if (i == 200) midi.addEvent(juce::MidiMessage::noteOff(1, 52), 0);
    buf.clear();
    p.processBlock(buf, midi);
    midi.clear();
  }
  // hôte simulé sur un seul thread : la capture automatique de getStateInformation attend un bloc audio qui ne vient
  // pas ; capture explicite (comme CAP-02w) avant toute sauvegarde
  p.requestCapture(4);
  p.requestCapture(5);
  buf.clear();
  p.processBlock(buf, midi);
  p.collectCaptures((1u << 4) | (1u << 5));
}
juce::MemoryBlock stateOf(SubdestrucktProcessor& p) {
  juce::MemoryBlock b;
  p.getStateInformation(b);
  return b;
}
bool sameCaptures(const std::vector<CaptureAsset>& a, const std::vector<CaptureAsset>& b) {
  if (a.size() != b.size()) return false;
  for (const auto& x : a) {
    bool found = false;
    for (const auto& y : b)
      if (y.slot == x.slot && y.kind == x.kind && y.frames == x.frames && y.data == x.data) found = true;
    if (!found) return false;
  }
  return true;
}

void testAutonomyWrapper() {
  // AUT-02 : restauration DAW
  auto aPtr = std::make_unique<SubdestrucktProcessor>();
  auto& a = *aPtr;
  buildFrozen(a);
  const juce::MemoryBlock blobA = stateOf(a);  // capture automatique des gels armés
  const auto capA = a.captures();
  auto bPtr = std::make_unique<SubdestrucktProcessor>();
  auto& b = *bPtr;
  b.prepareToPlay(48000.0, 256);
  b.setStateInformation(blobA.getData(), static_cast<int>(blobA.getSize()));
  const bool persisted = stateOf(b) == blobA;  // G-21w : configuration persistée identique (armements, captures)
  const double before = runSilent(b, 563);       // 3 s sans note, non autorisée
  const bool suspended = b.autonomySuspended() && b.telemetry().autonomySuspended.load() && !b.autonomyAuthorized();
  b.authorizeAutonomy();
  const double after = runSilent(b, 375, 100);  // gel rejoué sans entrée
  b.requestCapture(4);
  b.requestCapture(5);
  runSilent(b, 1);
  b.collectCaptures((1u << 4) | (1u << 5));
  const bool content = capA.size() == 2 && sameCaptures(b.captures(), capA);
  std::string capDetail;
  for (const auto& x : capA)
    for (const auto& y : b.captures())
      if (y.slot == x.slot) {
        double md = 0.0;
        for (size_t i = 0; i < std::min(x.data.size(), y.data.size()); ++i) md = std::max(md, std::fabs(static_cast<double>(x.data[i]) - y.data[i]));
        char t[160];
        std::snprintf(t, sizeof t, " [emplacement %d %s : trames %d/%d, ecart max %.3g]", x.slot, x.kind ? "S2" : "D2", x.frames, y.frames, md);
        capDetail += t;
      }
  const bool a2 = capA.size() == 2 && persisted && before < 1e-9 && suspended && after > 1e-3 && content;
  TK_CHECK(a2);
  tk::result("AUT-02", a2, "restauration DAW (gel D2 + gel S2 armes, 2 captures) : etat relu identique octet par octet %d ; avant autorisation : energie 3 s sans note %.3g (< 1e-9), suspendu (processeur + telemetrie) %d ; apres Autoriser : gel rejoue %.3g (> 1e-3), nouvelles captures identiques aux originales %d%s",
             persisted, before, suspended, after, content, capDetail.c_str());

  // AUT-02c (COMPLÉMENTAIRE, défini APRÈS le diagnostic de la clause (c) d'AUT-02, ne la remplace pas) : même
  // protocole avec S2 Decay = 1 (gel S2 sans demi-vie) : nouvelles captures identiques aux originales (D2 et S2)
  bool c2 = false;
  {
    auto aaPtr = std::make_unique<SubdestrucktProcessor>(), bbPtr = std::make_unique<SubdestrucktProcessor>();
    auto& aa = *aaPtr;
    auto& bb = *bbPtr;
    buildFrozen(aa, 1.0f);
    const juce::MemoryBlock blob = stateOf(aa);
    const auto cap0 = aa.captures();
    bb.prepareToPlay(48000.0, 256);
    bb.setStateInformation(blob.getData(), static_cast<int>(blob.getSize()));
    runSilent(bb, 100);
    bb.authorizeAutonomy();
    runSilent(bb, 375);
    bb.requestCapture(4);
    bb.requestCapture(5);
    runSilent(bb, 1);
    bb.collectCaptures((1u << 4) | (1u << 5));
    c2 = cap0.size() == 2 && sameCaptures(bb.captures(), cap0);
  }
  TK_CHECK(c2);
  tk::result("AUT-02c", c2, "COMPLEMENTAIRE (defini apres diagnostic d'AUT-02 (c)) : gel S2 sans demi-vie (S2 Decay = 1) : nouvelles captures D2 et S2 identiques aux originales apres restauration suspendue puis Autoriser %d ; avec le Decay par defaut (0,5), le spectre gele restaure decroit selon son contrat (demi-vie 0,1 + 30 d^2 s)", c2);

  // AUT-03 : duplication d'instance (A -> C) ; A inchangée (rendu identique à un jumeau A2 non dupliqué)
  auto a2pPtr = std::make_unique<SubdestrucktProcessor>();
  auto& a2p = *a2pPtr;
  buildFrozen(a2p);
  stateOf(a2p);
  auto cPtr = std::make_unique<SubdestrucktProcessor>();
  auto& c = *cPtr;
  c.prepareToPlay(48000.0, 256);
  const juce::MemoryBlock blobA2 = stateOf(a);
  c.setStateInformation(blobA2.getData(), static_cast<int>(blobA2.getSize()));
  const double ea = runSilent(a, 188), ea2 = runSilent(a2p, 188), ec = runSilent(c, 188);
  const bool a3 = c.autonomySuspended() && a.autonomyAuthorized() && ea == ea2 && ea > 1e-3 && ec < 1e-9;
  TK_CHECK(a3);
  tk::result("AUT-03", a3, "duplication : copie suspendue %d (energie %.3g) ; source toujours autorisee %d, rendu identique a son jumeau %d (energie %.3g)",
             c.autonomySuspended(), ec, a.autonomyAuthorized(), ea == ea2, ea);

  // AUT-04 : preset externe armé avec captures ; changement de preset pendant un gel autorisé
  auto dPtr = std::make_unique<SubdestrucktProcessor>();
  auto& d = *dPtr;
  d.prepareToPlay(48000.0, 256);
  PluginState ps = a.snapshotState();
  ps.captures = capA;
  const bool presetArmed = anyArmed(ps);
  d.applyPresetState(ps, "test arme");
  PluginState dcfg = d.snapshotState();
  const double ed = runSilent(d, 563);
  const bool a4a = presetArmed && !anyArmed(dcfg) && sameCaptures(d.captures(), capA) && d.autonomySuspended() && ed < 1e-9;
  const double playing = runSilent(a2p, 100);
  a2p.applyPresetState(defaultState(), "defaut");
  const double afterChange = runSilent(a2p, 375, 100);
  const bool a4 = a4a && playing > 1e-3 && afterChange < 1e-9;
  TK_CHECK(a4);
  tk::result("AUT-04", a4, "preset arme avec captures : armements remis a zero %d, captures conservees %d, suspendu %d, energie sans note %.3g ; changement de preset pendant un gel autorise : %.3g -> %.3g",
             !anyArmed(dcfg), sameCaptures(d.captures(), capA), d.autonomySuspended(), ed, playing, afterChange);

  // AUT-05 : Panic (autorisation retirée, configuration inchangée, Drone désarmé) puis Autoriser
  auto ePtr = std::make_unique<SubdestrucktProcessor>();
  auto& e = *ePtr;
  buildFrozen(e);
  stateOf(e);  // captures prises (gels armés)
  e.setDrone(true);
  runSilent(e, 2);
  std::vector<float> paramsBefore;
  for (int i = 0; i < kNumParams; ++i) paramsBefore.push_back(e.parameters().param(i)->getValue());
  const bool dirtyBefore = e.dirty();
  e.requestPanic();
  const double ep = runSilent(e, 375, 100);
  bool sameCfg = true;
  for (int i = 0; i < kNumParams; ++i) sameCfg = sameCfg && e.parameters().param(i)->getValue() == paramsBefore[static_cast<size_t>(i)];
  const bool revoked = !e.autonomyAuthorized() && e.autonomySuspended() && !e.telemetry().drone.load();
  e.authorizeAutonomy();
  const double er = runSilent(e, 375, 100);
  const bool a5 = revoked && sameCfg && e.dirty() == dirtyBefore && ep < 1e-9 && er > 1e-3;
  TK_CHECK(a5);
  tk::result("AUT-05", a5, "Panic : autorisation retiree et Drone desarme %d, configuration inchangee %d (dirty inchange %d), energie apres Panic %.3g ; Autoriser : gel restaure depuis la capture %.3g",
             revoked, sameCfg, e.dirty() == dirtyBefore, ep, er);

  // AUT-06 : fermeture / réouverture (2 cycles d'une instance suspendue)
  juce::MemoryBlock cyc = blobA;
  bool cycles = true;
  for (int k = 0; k < 2; ++k) {
    auto fPtr = std::make_unique<SubdestrucktProcessor>();
    auto& f = *fPtr;
    f.prepareToPlay(48000.0, 256);
    f.setStateInformation(cyc.getData(), static_cast<int>(cyc.getSize()));
    runSilent(f, 4);
    const juce::MemoryBlock next = stateOf(f);
    cycles = cycles && next == blobA && f.autonomySuspended() && sameCaptures(f.captures(), capA);
    cyc = next;
  }
  TK_CHECK(cycles);
  tk::result("AUT-06", cycles, "2 cycles fermeture/reouverture d'une instance suspendue : etat (armements + captures) identique octet par octet et toujours suspendue %d", cycles);

  // AUT-07 : compatibilité : état v2 armé (paramètres du format 2 uniquement) -> suspendu ; Drone (drone_restore)
  PluginState v2s = defaultState();
  v2s.params[ParamId::DstGl1Type] = static_cast<float>(DstType::D2);
  v2s.params[ParamId::DstGl1Arm] = 1.0f;
  v2s.params[ParamId::DstGl1D2Freeze] = 1.0f;
  std::istringstream in(encodeState(v2s, "test", StateScope::Daw));
  std::string line, v2txt;
  while (std::getline(in, line)) {
    if (line.rfind("SUBDESTRUCKT_STATE", 0) == 0) line = "SUBDESTRUCKT_STATE 2";
    if (line.rfind("param ", 0) == 0) {
      std::istringstream ls(line.substr(6));
      std::string sid;
      ls >> sid;
      ParamId id;
      if (findParam(sid, id) && paramSpec(id).version > 2) continue;
    }
    if (line.rfind("capture", 0) == 0 || line.rfind("bank", 0) == 0) continue;
    v2txt += line + "\n";
  }
  auto gPtr = std::make_unique<SubdestrucktProcessor>();
  auto& g = *gPtr;
  g.prepareToPlay(48000.0, 256);
  g.setStateInformation(v2txt.data(), static_cast<int>(v2txt.size()));
  const bool v2ok = juce::String(g.lastMessage()).contains("v2 migre") && g.autonomySuspended() && g.parameters().param(idx(ParamId::DstGl1Arm))->getValue() > 0.5f;
  auto droneCase = [](bool restore) {
    auto hPtr = std::make_unique<SubdestrucktProcessor>();
    auto& h = *hPtr;
    h.prepareToPlay(48000.0, 256);
    h.setDroneRestore(restore);
    h.setDrone(true);
    const juce::MemoryBlock s = stateOf(h);
    auto h2Ptr = std::make_unique<SubdestrucktProcessor>();
    auto& h2 = *h2Ptr;
    h2.prepareToPlay(48000.0, 256);
    h2.setStateInformation(s.getData(), static_cast<int>(s.getSize()));
    return h2.drone();
  };
  const bool d0 = !droneCase(false), d1 = droneCase(true);
  const bool a7 = v2ok && d0 && d1;
  TK_CHECK(a7);
  tk::result("AUT-07", a7, "etat v2 arme migre (%s) : suspendu, armement conserve %d ; Drone : drone_restore=0 non restaure %d, =1 restaure %d",
             g.lastMessage().c_str(), v2ok, d0, d1);

  // AUT-08 : UI : bouton Autoriser visible si suspendu ; ouvrir l'éditeur et naviguer ne change pas l'autorisation
  std::unique_ptr<juce::AudioProcessorEditor> edB(b.createEditorIfNeeded());
  auto s2Ptr = std::make_unique<SubdestrucktProcessor>();
  auto& s2 = *s2Ptr;
  s2.prepareToPlay(48000.0, 256);
  s2.setStateInformation(blobA.getData(), static_cast<int>(blobA.getSize()));
  std::unique_ptr<juce::AudioProcessorEditor> ed2(s2.createEditorIfNeeded());
  auto* m2 = dynamic_cast<sbd::ui::MainEditor*>(ed2.get());
  bool shown = false, stillSusp = true;
  if (m2) {
    for (int sp = 0; sp < 3; ++sp) m2->showSpace(sp);
    m2->refreshNow();
    std::vector<juce::Component*> all;
    collectComponents(*m2, all);
    for (auto* comp : all)
      if (auto* tb = dynamic_cast<juce::TextButton*>(comp))
        if (tb->getButtonText() == "Autoriser") shown = tb->isVisible();
    stillSusp = s2.autonomySuspended() && !s2.autonomyAuthorized();
  }
  const bool a8 = m2 && shown && stillSusp;
  TK_CHECK(a8);
  tk::result("AUT-08", a8, "UI : bouton Autoriser visible pour une instance suspendue %d ; navigation des 3 espaces sans effet sur l'autorisation %d", shown, stillSusp);
}

// ------------------------------------------------------------------ ARCH-G4.2 : ML-01w..07w (MIDI Learn)
void sendCc(SubdestrucktProcessor& p, int cc, int val, int count = 1) {
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  for (int i = 0; i < count; ++i) midi.addEvent(juce::MidiMessage::controllerEvent(1, cc, val), i % 256);
  buf.clear();
  p.processBlock(buf, midi);
  p.serviceMessageThread();
}
float native(SubdestrucktProcessor& p, ParamId id) {
  auto* q = p.parameters().param(idx(id));
  return q->convertFrom0to1(q->getValue());
}

void testMidiLearn(const std::string& profile) {
  std::remove(profile.c_str());
  const auto& sx = paramSpec(idx(ParamId::WtX));
  auto pPtr = std::make_unique<SubdestrucktProcessor>();
  auto& p = *pPtr;
  p.prepareToPlay(48000.0, 256);
  // ML-01w : affectation et suivi de valeur
  p.setMidiLearnTarget(idx(ParamId::WtX));
  sendCc(p, 20, 0);
  const bool mapped = p.midiLearn().ccToParam[20] == idx(ParamId::WtX);
  const float v0 = native(p, ParamId::WtX);
  sendCc(p, 20, 127);
  const float v127 = native(p, ParamId::WtX);
  sendCc(p, 20, 64);
  const float v64 = native(p, ParamId::WtX);
  const float step = (sx.maxValue - sx.minValue) / 127.0f;
  const float mid = sx.minValue + 64.0f * step;
  const bool m1 = mapped && std::fabs(v0 - sx.minValue) < 1e-4f && std::fabs(v127 - sx.maxValue) < 1e-4f && std::fabs(v64 - mid) <= step;
  TK_CHECK(m1);
  tk::result("ML-01w", m1, "CC20 -> wt.x : appris %d ; 0 -> %.4f, 127 -> %.4f, 64 -> %.4f (attendu %.4f +- %.4f)", mapped, v0, v127, v64, mid, step);
  // ML-02w : remplacement et suppression
  p.setMidiLearnTarget(idx(ParamId::WtY));
  sendCc(p, 20, 10);
  const bool repl = p.midiLearn().ccToParam[20] == idx(ParamId::WtY);
  p.setMidiLearnTarget(idx(ParamId::WtY));
  sendCc(p, 21, 10);
  const bool moved = p.midiLearn().ccToParam[21] == idx(ParamId::WtY) && p.midiLearn().ccToParam[20] == -1;
  p.clearMidiLearn(21);
  const float yBefore = native(p, ParamId::WtY);
  sendCc(p, 21, 127);
  const bool cleared = p.midiLearn().ccToParam[21] == -1 && native(p, ParamId::WtY) == yBefore;
  const bool m2 = repl && moved && cleared;
  TK_CHECK(m2);
  tk::result("ML-02w", m2, "remplacement CC20 -> wt.y %d ; wt.y appris sur CC21 retire CC20 %d ; suppression de CC21 sans effet ensuite %d", repl, moved, cleared);
  // ML-03w : persistance (profil isolé), relecture par une nouvelle instance, lignes invalides ignorées
  p.setMidiLearnTarget(idx(ParamId::WtZ));
  sendCc(p, 30, 0);
  {
    std::ofstream app(profile, std::ios::app);
    app << "garbage line\n999 wt.x\n31 param.inconnu\n64 wt.x\n";
  }
  auto qPtr = std::make_unique<SubdestrucktProcessor>();
  auto& q = *qPtr;
  const bool m3 = q.midiLearn().ccToParam[30] == idx(ParamId::WtZ) && q.midiLearn().ccToParam[31] == -1 && q.midiLearn().ccToParam[64] == -1 &&
                  juce::File(profile).existsAsFile();
  TK_CHECK(m3);
  tk::result("ML-03w", m3, "profil isole %s : affectation relue par une nouvelle instance %d ; lignes invalides, StableID inconnu et CC reservee ignores sans echec", profile.c_str(), m3);
  // ML-04w : séparation presets / projets
  const std::string st = p.encodeCurrentState(StateScope::Daw), pr = p.encodeCurrentState(StateScope::Preset);
  const bool noMap = st.find("learn") == std::string::npos && st.find("cc ") == std::string::npos && pr.find("learn") == std::string::npos;
  const MidiLearnMap mapBefore = p.midiLearn();
  p.applyPresetState(defaultState(), "defaut");
  juce::MemoryBlock blob;
  q.getStateInformation(blob);
  p.setStateInformation(blob.getData(), static_cast<int>(blob.getSize()));
  const bool m4 = noMap && p.midiLearn() == mapBefore;
  TK_CHECK(m4);
  tk::result("ML-04w", m4, "etat DAW et preset sans affectation %d ; chargement d'un preset et d'un etat : carte inchangee %d", noMap, p.midiLearn() == mapBefore);
  // ML-05w : multi-instance sans perte
  auto iaPtr = std::make_unique<SubdestrucktProcessor>(), ibPtr = std::make_unique<SubdestrucktProcessor>();
  auto& ia = *iaPtr;
  auto& ib = *ibPtr;
  ia.prepareToPlay(48000.0, 256);
  ib.prepareToPlay(48000.0, 256);
  ia.setMidiLearnTarget(idx(ParamId::WtLevel));
  sendCc(ia, 22, 0);
  ib.setMidiLearnTarget(idx(ParamId::Op1Level));
  sendCc(ib, 23, 0);
  MidiLearnMap disk;
  disk.load(profile);
  for (int i = 0; i < 31; ++i) ia.serviceMessageThread();  // relecture périodique (~1 s)
  const bool m5 = disk.ccToParam[22] == idx(ParamId::WtLevel) && disk.ccToParam[23] == idx(ParamId::Op1Level) && disk.ccToParam[30] == idx(ParamId::WtZ) &&
                  ia.midiLearn().ccToParam[23] == idx(ParamId::Op1Level);
  TK_CHECK(m5);
  tk::result("ML-05w", m5, "A apprend CC22, B apprend CC23 : profil contenant les deux et l'affectation anterieure (aucune perte) %d ; carte de A actualisee %d", m5,
             ia.midiLearn().ccToParam[23] == idx(ParamId::Op1Level));
  // ML-06w : limites et conflits
  const MidiLearnMap m6b = p.midiLearn();
  bool reservedOk = true;
  for (int cc : {0, 6, 32, 38, 64, 96, 101, 121, 127}) {
    p.setMidiLearnTarget(idx(ParamId::WtX));
    sendCc(p, cc, 5);
    reservedOk = reservedOk && p.midiLearn().ccToParam[static_cast<size_t>(cc)] == -1;
  }
  p.setMidiLearnTarget(-1);
  const bool refusedArm = !p.setMidiLearnTarget(idx(ParamId::DstGl1Arm)) && juce::String(p.lastMessage()).contains("refuse");
  bool refusedNonAuto = true;
  for (int i = 0; i < kNumParams; ++i)
    if (!paramSpec(i).automatable) {
      refusedNonAuto = !p.setMidiLearnTarget(i);
      break;
    }
  p.setMidiLearnTarget(-1);
  sendCc(p, 40, 3);
  const bool unchanged = p.midiLearn() == m6b;
  // automation hôte -> CC -> hôte : dernière écriture gagnante, aucune boucle
  setPv(p, ParamId::WtZ, 0.25f);
  const bool h1 = std::fabs(native(p, ParamId::WtZ) - 0.25f) < 1e-3f;
  sendCc(p, 30, 127);
  const bool c1 = std::fabs(native(p, ParamId::WtZ) - paramSpec(idx(ParamId::WtZ)).maxValue) < 1e-3f;
  setPv(p, ParamId::WtZ, 0.5f);
  p.serviceMessageThread();
  p.serviceMessageThread();
  const bool h2 = std::fabs(native(p, ParamId::WtZ) - 0.5f) < 1e-3f;
  const bool m6 = reservedOk && refusedArm && refusedNonAuto && unchanged && h1 && c1 && h2;
  TK_CHECK(m6);
  tk::result("ML-06w", m6, "CC reservees refusees %d ; armement refuse %d ; non automatisable refuse %d ; carte inchangee %d ; hote -> CC -> hote : derniere ecriture gagnante %d/%d/%d, aucune boucle",
             reservedOk, refusedArm, refusedNonAuto, unchanged, h1, c1, h2);
  // ML-07w : 512 CC appris par bloc, 0 allocation dans processBlock
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  midi.ensureSize(16384);
  for (int i = 0; i < 512; ++i) midi.addEvent(juce::MidiMessage::controllerEvent(1, 30, i % 128), i % 256);
  long long alloc = 0;
  for (int r = 0; r < 4; ++r) {
    buf.clear();
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    p.processBlock(buf, midi);
    tk::countingAllocations().store(false);
    alloc += tk::allocationCount().load();
    p.serviceMessageThread();
  }
  const bool m7 = alloc == 0;
  TK_CHECK(m7);
  tk::result("ML-07w", m7, "512 messages CC appris par bloc x 4 blocs : %lld allocation(s) dans processBlock ; chemin audio : atomiques uniquement (lecture de code)", alloc);
  std::remove(profile.c_str());
}

}  // namespace

// ------------------------------------------------------------------ ARCH-G4.3 : TRANSPORT AUTHORIZATION (wrapper)
struct FakePlayHead : juce::AudioPlayHead {
  int mode = 1;  // 0 : position absente ; 1 : ARRÊT ; 2 : LECTURE ; 3 : LECTURE + ENR ; 4 : contexte VST3 absent (temps 0/0)
  juce::Optional<PositionInfo> getPosition() const override {
    if (mode == 0) return {};
    PositionInfo i;
    i.setIsPlaying(mode == 2 || mode == 3);
    i.setIsRecording(mode == 3);
    i.setBpm(120.0);
    i.setPpqPosition(0.0);
    i.setTimeInSeconds(mode == 4 ? std::nan("") : 0.0);
    return i;
  }
};
long long gTaWrapAlloc = 0;
double runPh(SubdestrucktProcessor& p, int blocks, juce::MidiBuffer* first = nullptr) {  // énergie, allocations comptées
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer empty;
  double e = 0.0;
  for (int i = 0; i < blocks; ++i) {
    buf.clear();
    juce::MidiBuffer& m = (i == 0 && first) ? *first : empty;
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    p.processBlock(buf, m);
    tk::countingAllocations().store(false);
    gTaWrapAlloc += tk::allocationCount().load();
    for (int ch = 0; ch < 2; ++ch)
      for (int k = 0; k < 256; ++k) e += static_cast<double>(buf.getSample(ch, k)) * buf.getSample(ch, k);
  }
  return e;
}

void testTransportWrapper() {
  auto srcPtr = std::make_unique<SubdestrucktProcessor>();
  buildFrozen(*srcPtr);
  const juce::MemoryBlock blob = stateOf(*srcPtr);
  FakePlayHead ph;
  auto restored = [&](FakePlayHead* head) {
    auto q = std::make_unique<SubdestrucktProcessor>();
    q->prepareToPlay(48000.0, 256);
    q->setPlayHead(head);
    q->setStateInformation(blob.getData(), static_cast<int>(blob.getSize()));
    return q;
  };
  // TA-03w / TA-20w : STOP -> PLAY via le playhead, sans éditeur ni service du thread message
  ph.mode = 1;
  auto a = restored(&ph);
  const double stopE = runPh(*a, 100);
  const bool susp = a->autonomySuspended();
  ph.mode = 2;
  const double playE = runPh(*a, 300);
  const bool t3 = stopE < 1e-9 && susp && playE > 1e-3 && a->autonomyAuthorized() &&
                  a->telemetry().autonomySource.load() == static_cast<int>(AuthSource::Transport);
  TK_CHECK(t3);
  tk::result("TA-03w", t3, "processBlock + playhead simule : ARRET 100 blocs energie %.3g, suspendu %d ; LECTURE : gel rejoue %.3g, autorise (source transport) %d, sans Autoriser ni thread message",
             stopE, susp, playE, a->autonomyAuthorized());
  // TA-13w : classification INCONNU (position absente, playhead nul, contexte VST3 absent)
  bool unk = true;
  for (int m : {0, -1, 4}) {
    FakePlayHead h;
    h.mode = m < 0 ? 1 : m;
    auto q = restored(m < 0 ? nullptr : &h);
    runPh(*q, 20);
    unk = unk && q->telemetry().transportState.load() == 0 && !q->autonomyAuthorized();
  }
  {
    FakePlayHead h;
    h.mode = 1;
    auto q = restored(&h);
    runPh(*q, 5);
    unk = unk && q->telemetry().transportState.load() == static_cast<int>(TransportObs::Stopped);
  }
  TK_CHECK(unk);
  tk::result("TA-13w", unk, "position absente, playhead nul, contexte VST3 absent (temps non fini) : classes INCONNU (jamais observe, non autorise) ; position valide a l'arret : ARRET %d", unk);
  // TA-15w : rendu hors temps réel immédiatement après la restauration, premier bloc déjà en LECTURE
  FakePlayHead hr;
  hr.mode = 2;
  auto r = restored(&hr);
  r->setNonRealtime(true);
  const double renderE = runPh(*r, 300);
  const bool t15 = r->autonomyAuthorized() && r->telemetry().autonomySource.load() == static_cast<int>(AuthSource::Render) && renderE > 1e-3;
  FakePlayHead hrt;
  hrt.mode = 2;
  auto rt = restored(&hrt);  // même scénario en temps réel : insertion/chargement pendant la lecture
  const double rtE = runPh(*rt, 300);
  const bool t15b = !rt->autonomyAuthorized() && rtE < 1e-9;
  TK_CHECK(t15 && t15b);
  tk::result("TA-15w", t15 && t15b, "export hors temps reel juste apres l'ouverture (premier bloc en LECTURE, sans lecture prealable, sans Autoriser, sans editeur) : autorise par rendu %d, contenu gele %.3g ; meme scenario en temps reel : non autorise %d (energie %.3g)",
             t15, renderE, t15b, rtE);
  // TA-18w : état identique, instance autorisée (a) et suspendue
  auto s = restored(&ph);
  ph.mode = 1;
  runPh(*s, 10);
  const bool t18 = stateOf(*a) == stateOf(*s) && stateOf(*a) == blob && a->autonomyAuthorized() && !s->autonomyAuthorized();
  TK_CHECK(t18);
  tk::result("TA-18w", t18, "getStateInformation : instance autorisee par le transport et instance suspendue de meme configuration : etats identiques octet par octet (et = etat restaure) %d", t18);
  // TA-21w (c) : drone_restore
  bool dr = true;
  for (bool restore : {false, true}) {
    auto d = std::make_unique<SubdestrucktProcessor>();
    d->prepareToPlay(48000.0, 256);
    d->setDrone(true);
    d->setDroneRestore(restore);
    const auto b = stateOf(*d);
    auto e = std::make_unique<SubdestrucktProcessor>();
    e->prepareToPlay(48000.0, 256);
    e->setStateInformation(b.getData(), static_cast<int>(b.getSize()));
    runPh(*e, 3);
    dr = dr && e->drone() == restore && (restore ? (e->autonomySuspended() && !e->autonomyAuthorized()) : e->autonomyAuthorized());
  }
  TK_CHECK(dr);
  tk::result("TA-21w", dr, "drone_restore=0 : Drone non restaure, aucune suspension ; drone_restore=1 : configuration Drone restauree et suspendue jusqu'a autorisation (RF-03) %d", dr);
  // TA-22w : CC120 = Panic ; CC123 = All Notes Off
  juce::MidiBuffer m123, m120;
  m123.addEvent(juce::MidiMessage::controllerEvent(1, 123, 0), 10);
  m120.addEvent(juce::MidiMessage::controllerEvent(1, 120, 0), 10);
  const auto rev0 = a->telemetry().authRevocations.load();
  runPh(*a, 2, &m123);
  const bool after123 = a->autonomyAuthorized() && a->telemetry().authRevocations.load() == rev0;
  runPh(*a, 1, &m120);
  const bool after120 = !a->autonomyAuthorized() && a->telemetry().authRevocations.load() == rev0 + 1;
  TK_CHECK(after123 && after120);
  tk::result("TA-22w", after123 && after120, "MIDI : CC123 (All Notes Off) sans revocation %d ; CC120 (All Sound Off) = Panic, revocation %d", after123, after120);
  // TA-23w / TA-07w : éditeur ouvert/fermé x10 sans effet ; clic Drone et clic d'armement = autorisation explicite
  ph.mode = 2;
  auto w = restored(&ph);  // restaurée pendant une LECTURE déjà en cours : reste suspendue
  runPh(*w, 5);
  bool edOk = !w->autonomyAuthorized();
  for (int i = 0; i < 10; ++i) {
    std::unique_ptr<juce::AudioProcessorEditor> ed(w->createEditorIfNeeded());
    runPh(*w, 2);
    edOk = edOk && !w->autonomyAuthorized();
  }
  TK_CHECK(edOk);
  tk::result("TA-23w", edOk, "ouverture/fermeture de l'editeur x10 (LECTURE maintenue) : autorisation inchangee (suspendue) %d", edOk);
  bool droneClick = false, armClick = false, armFound = false;
  {
    auto u = restored(&ph);
    runPh(*u, 3);
    std::unique_ptr<juce::AudioProcessorEditor> ed(u->createEditorIfNeeded());
    auto* main = dynamic_cast<sbd::ui::MainEditor*>(ed.get());
    std::vector<juce::Component*> all;
    if (main) {
      main->setSize(1440, 900);
      for (int space = 0; space < 3; ++space)
        for (int tab = 0; tab < 6; ++tab) {
          main->showSubTab(space, tab);
          main->refreshNow();
          collectComponents(*main, all);
        }
    }
    for (auto* c : all)
      if (auto* b = dynamic_cast<juce::ToggleButton*>(c))
        if (b->getButtonText() == "Drone" && !droneClick) {
          b->setToggleState(true, juce::sendNotification);  // clic de l'utilisateur
          runPh(*u, 1);
          droneClick = u->autonomyAuthorized() && u->drone();
        }
    auto v = restored(&ph);
    runPh(*v, 3);
    std::unique_ptr<juce::AudioProcessorEditor> ed2(v->createEditorIfNeeded());
    auto* main2 = dynamic_cast<sbd::ui::MainEditor*>(ed2.get());
    all.clear();
    if (main2) {
      main2->setSize(1440, 900);
      for (int space = 0; space < 3; ++space)
        for (int tab = 0; tab < 6; ++tab) {
          main2->showSubTab(space, tab);
          main2->refreshNow();
          collectComponents(*main2, all);
        }
    }
    for (auto* c : all)
      if (auto* b = dynamic_cast<juce::ToggleButton*>(c))
        if (b->getHelpText() == "dst.gl1.arm" && !armFound) {
          armFound = true;
          b->setToggleState(false, juce::sendNotification);
          runPh(*v, 1);
          const bool stillSusp = !v->autonomyAuthorized();  // désarmer n'autorise pas
          b->setToggleState(true, juce::sendNotification);
          runPh(*v, 1);
          armClick = stillSusp && v->autonomyAuthorized();
        }
  }
  const bool t7 = droneClick && armFound && armClick;
  TK_CHECK(t7);
  tk::result("TA-07w", t7, "editeur : clic Drone -> autorisation explicite %d ; clic d'armement (dst.gl1.arm) -> autorisation explicite %d (controle trouve %d, desarmer n'autorise pas)",
             droneClick, armClick, armFound);
  TK_CHECK(gTaWrapAlloc == 0);
  tk::result("TA-20w", gTaWrapAlloc == 0, "processBlock avec playhead (ARRET, LECTURE, INCONNU, hors temps reel, CC120/123) : %lld allocation(s)", gTaWrapAlloc);
}

// ARCH-UI-04 (tests/wrapper/Ui04Tests.cpp)
void testUi04Coverage(const std::string& mapPath);
void testUi04Layout();
void testUi04Behaviour();
void testUi04Runtime();
void testUi04Operations();
void snapshotUi04(const std::string& prefix);
// ARCH-UI-04-R1 (tests/wrapper/TaR1Tests.cpp)
void testTaR1Wrapper();
// ARCH-UI-04-R2-B2-01 (tests/wrapper/GestureTests.cpp) : reproductions PB-01 / PB-02, tests permanents (CE-B2-01)
void testGestureReproduction();
// ARCH-UI-04-R2-B2-02 (tests/wrapper/R2B2Tests.cpp) : fondation R1 (identifiants, libelles)
void testR2B2Foundation();

int main(int argc, char** argv) {
  juce::ScopedJuceInitialiser_GUI juceInit;  // MessageManager (timers du processeur)
  // profil MIDI Learn isolé : les tests n'écrivent jamais dans le profil réel de l'utilisateur
  const std::string learnProfile =
      juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("sbd_test_midi_learn.txt").getFullPathName().toStdString();
  MidiLearnMap::setPathOverride(learnProfile);
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
  testCapturesWrapper();
  testBankEditingWrapper();
  testUiCoherence();
  std::fprintf(stderr, "[ARCH-UI-04 : interface SYMBIOTIC INDUSTRIAL]\n");
  testUi04Coverage(argc > 3 ? argv[3] : "");
  testUi04Layout();
  testUi04Behaviour();
  testUi04Runtime();
  testUi04Operations();
  std::fprintf(stderr, "[ARCH-G4.2 : autonomie, MIDI Learn]\n");
  testAutonomyWrapper();
  testMidiLearn(learnProfile);
  std::fprintf(stderr, "[ARCH-G4.3 : TRANSPORT AUTHORIZATION]\n");
  testTransportWrapper();
  testTaR1Wrapper();
  testGestureReproduction();
  testR2B2Foundation();
  if (argc > 2) {
    std::fprintf(stderr, "[capture UI]\n");
    snapshotEditor(argv[2]);
    std::string prefix = argv[2];
    if (prefix.size() > 4 && prefix.compare(prefix.size() - 4, 4, ".png") == 0) prefix.resize(prefix.size() - 4);
    snapshotUi04(prefix + "_ui04");
  }
  return tk::finish("wrapper_tests");
}
