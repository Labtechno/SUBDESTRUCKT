// SUBDESTRUCKT — tests d'hôte : le bundle .vst3 compilé est chargé par l'hébergement VST3 de JUCE 9.0.6, sans interface
// graphique. Ce n'est PAS une DAW : découverte, instanciation, traitement, état et paramètres passent par les interfaces
// VST3 réelles, mais aucune lecture audio, aucun projet ni aucune PDC de DAW ne sont testés ici.
//
// Usage : subdestruckt_host_tests <chemin.vst3> <expected_vst3_param_ids.txt> [--quick] [--tag PREFIXE]
//   complet : G-20 (4 fréquences x blocs 32..512), G-21 (état via VST3), G-22 (identifiants VST3), G-19h (instances)
//   --quick : découverte + instanciation + rendu court (recette d'installation PKG-02 en VM)
#include <juce_audio_processors_headless/juce_audio_processors_headless.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

#include "TestKit.h"
#include "core/Parameters.h"

namespace {

const double kRates[4] = {44100.0, 48000.0, 88200.0, 96000.0};
juce::String g_tag;

std::string id(const char* base) { return (g_tag + base).toStdString(); }

std::unique_ptr<juce::AudioPluginInstance> create(juce::AudioPluginFormat& fmt, const juce::PluginDescription& d, double fs,
                                                  int block) {
  juce::String err;
  auto inst = fmt.createInstanceFromDescription(d, fs, block, err);
  if (!inst) std::printf("  echec d'instanciation : %s\n", err.toRawUTF8());
  return inst;
}

std::vector<float> render(juce::AudioPluginInstance& p, double fs, int block, int samples) {
  p.setRateAndBufferSizeDetails(fs, block);
  p.prepareToPlay(fs, block);
  juce::AudioBuffer<float> buf(juce::jmax(2, p.getTotalNumOutputChannels()), block);
  juce::MidiBuffer midi;
  std::vector<float> out(static_cast<size_t>(samples), 0.0f);
  const int on = static_cast<int>(0.1 * fs), off = static_cast<int>(1.0 * fs);
  for (int pos = 0; pos < samples; pos += block) {
    const int n = juce::jmin(block, samples - pos);
    midi.clear();
    if (on >= pos && on < pos + n) midi.addEvent(juce::MidiMessage::noteOn(1, 57, static_cast<juce::uint8>(110)), on - pos);
    if (off >= pos && off < pos + n) midi.addEvent(juce::MidiMessage::noteOff(1, 57), off - pos);
    buf.setSize(buf.getNumChannels(), n, false, false, true);
    buf.clear();
    p.processBlock(buf, midi);
    std::memcpy(&out[static_cast<size_t>(pos)], buf.getReadPointer(0), sizeof(float) * static_cast<size_t>(n));
  }
  p.releaseResources();
  return out;
}

bool same(const std::vector<float>& a, const std::vector<float>& b) {
  return a.size() == b.size() && std::memcmp(a.data(), b.data(), a.size() * sizeof(float)) == 0;
}

double peak(const std::vector<float>& v) {
  double p = 0;
  for (float x : v) p = std::max(p, static_cast<double>(std::fabs(x)));
  return p;
}

bool allFinite(const std::vector<float>& v) {
  for (float x : v)
    if (!std::isfinite(x)) return false;
  return true;
}

// L'interface VST3 peut exposer des paramètres ajoutés par le wrapper (bypass, émulation des CC MIDI) : on ne
// touche qu'aux paramètres du registre SUBDESTRUCKT, retrouvés par leur nom.
juce::AudioProcessorParameter* findParam(juce::AudioPluginInstance& p, const char* name) {
  for (auto* prm : p.getParameters())
    if (prm->getName(256) == juce::String(name)) return prm;
  return nullptr;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 3) {
    std::printf("usage : %s <chemin.vst3> <expected_vst3_param_ids.txt> [--quick] [--tag PREFIXE]\n", argv[0]);
    return 2;
  }
  const juce::String path = juce::String::fromUTF8(argv[1]);
  const std::string expectedFile = argv[2];
  bool quick = false;
  for (int i = 3; i < argc; ++i) {
    if (juce::String(argv[i]) == "--quick") quick = true;
    if (juce::String(argv[i]) == "--tag" && i + 1 < argc) g_tag = juce::String(argv[++i]);
  }
  juce::MessageManager::getInstance();  // l'hébergement VST3 exige un thread de messages ; ce thread en tient lieu
  std::printf("SUBDESTRUCKT ARCH-G3 — tests d'hote VST3 (JUCE %s) sur %s\n", juce::SystemStats::getJUCEVersion().toRawUTF8(),
              path.toRawUTF8());

  juce::VST3PluginFormatHeadless fmt;
  juce::OwnedArray<juce::PluginDescription> types;
  fmt.findAllTypesForFile(types, path);
  bool found = types.size() == 1;
  juce::PluginDescription d;
  if (found) {
    d = *types[0];
    std::printf("  decouvert : %s / %s / version %s / instrument %d / sorties %d / format %s\n", d.name.toRawUTF8(),
                d.manufacturerName.toRawUTF8(), d.version.toRawUTF8(), d.isInstrument ? 1 : 0, d.numOutputChannels,
                d.pluginFormatName.toRawUTF8());
    found = d.name == "SUBDESTRUCKT" && d.manufacturerName == "Labtechno" && d.isInstrument && d.version == "0.1.0";
  }
  TK_CHECK(found);
  tk::result(id("G-20a").c_str(), found, "decouverte VST3 : 1 classe SUBDESTRUCKT, fabricant Labtechno, instrument, version 0.1.0");
  if (!found) {
    juce::MessageManager::deleteInstance();
    return tk::finish("host_tests");
  }

  if (quick) {
    auto inst = create(fmt, d, 48000.0, 256);
    bool ok = inst != nullptr;
    if (ok) {
      const auto out = render(*inst, 48000.0, 256, 72000);
      ok = allFinite(out) && peak(out) > 1e-3 && peak(out) < 1.0;
      std::printf("  rendu 1,5 s : crete %.4f\n", peak(out));
    }
    TK_CHECK(ok);
    tk::result(id("G-20q").c_str(), ok, "instanciation et rendu d'une note depuis le bundle installe (sortie finie, non silencieuse, < 0 dBFS)");
    inst.reset();
    juce::MessageManager::deleteInstance();
    return tk::finish("host_tests");
  }

  // ---- G-20 : fréquences x blocs, latence, invariance aux blocs à travers l'interface VST3
  bool matrixOk = true;
  char detail[1024];
  int off = 0;
  for (double fs : kRates) {
    std::vector<float> ref;
    int lat = -1;
    bool rateOk = true;
    for (int block : {256, 32, 64, 128, 512}) {
      auto inst = create(fmt, d, fs, block);
      if (!inst) {
        rateOk = false;
        continue;
      }
      inst->setRateAndBufferSizeDetails(fs, block);
      inst->prepareToPlay(fs, block);
      lat = inst->getLatencySamples();
      inst->releaseResources();
      const auto out = render(*inst, fs, block, static_cast<int>(fs * 1.5));
      const double pk = peak(out);
      const int expectedLat = fs == 44100.0 ? 32 : 0;
      bool ok = allFinite(out) && pk > 1e-3 && pk < 1.0 && lat == expectedLat;
      if (block == 256)
        ref = out;
      else
        ok = ok && same(ref, out);
      if (!ok) std::printf("  G-20 echec fs=%.0f bloc=%d latence=%d crete=%.4f\n", fs, block, lat, pk);
      rateOk = rateOk && ok;
    }
    matrixOk = matrixOk && rateOk;
    off += std::snprintf(detail + off, sizeof detail - static_cast<size_t>(off), " [%.1fk latence %d : %s]", fs / 1000.0, lat,
                         rateOk ? "ok" : "ECHEC");
  }
  TK_CHECK(matrixOk);
  tk::result(id("G-20").c_str(), matrixOk, "hote VST3 : 4 frequences x blocs 32/64/128/256/512, rendu fini non silencieux < 0 dBFS, latence attendue, sorties identiques bit a bit entre tailles de bloc :%s",
             detail);

  // ---- G-22 : identifiants VST3 des paramètres = liste de référence (StableID -> hachage), indépendants de l'ordre
  std::map<std::string, juce::uint32> expected;
  {
    std::ifstream in(expectedFile);
    std::string line;
    while (std::getline(in, line)) {
      if (line.empty() || line[0] == '#') continue;
      std::istringstream ls(line);
      std::string sid;
      unsigned long v = 0;
      ls >> sid >> v;
      expected[sid] = static_cast<juce::uint32>(v);
    }
  }
  bool idsOk = static_cast<int>(expected.size()) == sbd::kNumParams;
  {
    auto inst = create(fmt, d, 48000.0, 256);
    std::map<juce::String, juce::uint32> byName;
    int hostedParams = 0;
    if (inst) {
      for (auto* prm : inst->getParameters()) {
        auto* hp = dynamic_cast<juce::HostedAudioProcessorParameter*>(prm);
        if (!hp) continue;
        ++hostedParams;
        byName[prm->getName(256)] = static_cast<juce::uint32>(hp->getParameterID().getLargeIntValue());
      }
    }
    for (int i = 0; i < sbd::kNumParams; ++i) {
      const auto& spec = sbd::paramSpec(i);
      const auto it = byName.find(juce::String(spec.name));
      const bool match = it != byName.end() && expected.count(spec.stableId) && expected[spec.stableId] == it->second;
      if (!match) std::printf("  G-22 identifiant inattendu pour %s\n", spec.stableId);
      idsOk = idsOk && match;
    }
    std::printf("  %d parametres exposes par l'interface VST3\n", hostedParams);
  }
  TK_CHECK(idsOk);
  tk::result(id("G-22").c_str(), idsOk, "identifiants VST3 des %d parametres = liste de reference derivee des StableID (stables d'une compilation a l'autre)",
             sbd::kNumParams);

  // ---- G-21 : état via l'interface VST3 (IComponent/IEditController), rendu identique après rechargement
  bool stateOk = false;
  {
    auto a = create(fmt, d, 48000.0, 256);
    auto b = create(fmt, d, 48000.0, 256);
    if (a && b) {
      for (int k = 0; k < sbd::kNumParams; ++k)
        if (auto* prm = findParam(*a, sbd::paramSpec(k).name)) prm->setValueNotifyingHost(static_cast<float>((k * 37) % 100) / 99.0f);
      // un bloc traité pour que le processeur reçoive les changements de paramètres, puis état
      {
        a->setRateAndBufferSizeDetails(48000.0, 256);
        a->prepareToPlay(48000.0, 256);
        juce::AudioBuffer<float> buf(2, 256);
        juce::MidiBuffer midi;
        a->processBlock(buf, midi);
        a->releaseResources();
      }
      juce::MemoryBlock blobA, blobB;
      a->getStateInformation(blobA);
      b->setStateInformation(blobA.getData(), static_cast<int>(blobA.getSize()));
      b->getStateInformation(blobB);
      auto a2 = create(fmt, d, 48000.0, 256);
      a2->setStateInformation(blobA.getData(), static_cast<int>(blobA.getSize()));
      const auto ra = render(*a2, 48000.0, 256, 72000);
      const auto rb = render(*b, 48000.0, 256, 72000);
      const bool sameBlob = blobA == blobB;
      stateOk = sameBlob && same(ra, rb) && peak(ra) > 1e-4;
      std::printf("  etat : %zu octets, blob identique apres rechargement %s, rendu identique %s\n",
                  static_cast<size_t>(blobA.getSize()), sameBlob ? "oui" : "NON", same(ra, rb) ? "oui" : "NON");
    }
  }
  TK_CHECK(stateOk);
  tk::result(id("G-21").c_str(), stateOk, "etat via VST3 : getState -> nouvelle instance -> setState, etat relu identique et rendu identique bit a bit");

  // ---- G-19h : deux instances hébergées entrelacées
  bool instOk = false;
  {
    auto a = create(fmt, d, 48000.0, 128), b = create(fmt, d, 48000.0, 128);
    auto a2 = create(fmt, d, 48000.0, 128), b2 = create(fmt, d, 48000.0, 128);
    if (a && b && a2 && b2) {
      const char* depth = sbd::paramSpec(sbd::ParamId::DepthOp2Op1).name;
      if (auto* prm = findParam(*b, depth)) prm->setValueNotifyingHost(0.9f);
      if (auto* prm = findParam(*b2, depth)) prm->setValueNotifyingHost(0.9f);
      const auto ra = render(*a2, 48000.0, 128, 48000), rb = render(*b2, 48000.0, 128, 48000);
      a->setRateAndBufferSizeDetails(48000.0, 128);
      a->prepareToPlay(48000.0, 128);
      b->setRateAndBufferSizeDetails(48000.0, 128);
      b->prepareToPlay(48000.0, 128);
      std::vector<float> la(48000), lb(48000);
      juce::AudioBuffer<float> buf(2, 128);
      juce::MidiBuffer midi;
      const int on = 4800, off = 48000;
      for (int pos = 0; pos < 48000; pos += 128) {
        for (int k = 0; k < 2; ++k) {
          midi.clear();
          if (on >= pos && on < pos + 128) midi.addEvent(juce::MidiMessage::noteOn(1, 57, static_cast<juce::uint8>(110)), on - pos);
          if (off >= pos && off < pos + 128) midi.addEvent(juce::MidiMessage::noteOff(1, 57), off - pos);
          buf.clear();
          (k == 0 ? *a : *b).processBlock(buf, midi);
          std::memcpy(&(k == 0 ? la : lb)[static_cast<size_t>(pos)], buf.getReadPointer(0), 128 * sizeof(float));
        }
      }
      instOk = same(la, ra) && same(lb, rb) && !same(la, lb);
    }
  }
  TK_CHECK(instOk);
  tk::result(id("G-19h").c_str(), instOk, "deux instances VST3 hebergees entrelacees : sorties identiques aux rendus separes, instances isolees");

  juce::MessageManager::deleteInstance();
  return tk::finish("host_tests");
}
