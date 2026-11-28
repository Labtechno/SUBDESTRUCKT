// SUBDESTRUCKT — tests ARCH-UI-04-R1 (wrapper) : anomalie R-28 reproduite au niveau du processeur réel (playhead
// simulé, AudioProcessor::setNonRealtime comme le wrapper VST3 en mode kOffline), sauvegarde DAW pendant et après le
// rendu avec un thread audio concurrent (comme un hôte réel). Critères enregistrés avant correction :
//   TA-R01w (reproduction)  : export avant PLAY puis annulation : silence et suspension après le retour temps réel
//   TA-R13w (reproduction)  : captures persistées intactes, restituées au premier ARRÊT -> LECTURE réel
//   TA-R14  (reproduction)  : aucune écriture dans l'état persistant (sauvegarde pendant et après le rendu)
//   TA-R15w (non-régression): 0 allocation dans processBlock sur ces séquences
#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <cmath>
#include <memory>
#include <thread>

#include "TestKit.h"
#include "core/StateCodec.h"
#include "dsp/Destruction.h"
#include "plugin/PluginProcessor.h"

using namespace sbd;

namespace {

struct PlayHeadR1 : juce::AudioPlayHead {
  std::atomic<int> mode{1};  // 0 : position absente (INCONNU) ; 1 : ARRÊT ; 2 : LECTURE
  juce::Optional<PositionInfo> getPosition() const override {
    const int m = mode.load();
    if (m == 0) return {};
    PositionInfo i;
    i.setIsPlaying(m == 2);
    i.setBpm(120.0);
    i.setPpqPosition(0.0);
    i.setTimeInSeconds(0.0);
    return i;
  }
};

long long gAllocR1w = 0;

void setP(SubdestrucktProcessor& p, ParamId id, float v) {
  auto* q = p.parameters().param(idx(id));
  q->setValueNotifyingHost(q->convertTo0to1(v));
}

// énergie (somme des carrés) et crête de `blocks` blocs ; allocations de processBlock comptées
struct Run {
  double energy = 0.0, peak = 0.0;
};
Run runR1(SubdestrucktProcessor& p, int blocks, juce::MidiBuffer* first = nullptr) {
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer empty;
  Run r;
  for (int i = 0; i < blocks; ++i) {
    buf.clear();
    juce::MidiBuffer& m = (i == 0 && first) ? *first : empty;
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    p.processBlock(buf, m);
    tk::countingAllocations().store(false);
    gAllocR1w += tk::allocationCount().load();
    for (int ch = 0; ch < 2; ++ch)
      for (int k = 0; k < 256; ++k) {
        const double v = buf.getSample(ch, k);
        r.energy += v * v;
        r.peak = std::max(r.peak, std::fabs(v));
      }
  }
  return r;
}

// sauvegarde DAW pendant qu'un thread audio traite des blocs (hôte réel : la capture automatique attend une frontière
// de bloc). Aucune allocation n'est comptée pendant ce segment concurrent.
juce::MemoryBlock saveWithAudioThread(SubdestrucktProcessor& p) {
  std::atomic<bool> stop{false};
  std::thread audio([&] {
    juce::AudioBuffer<float> buf(2, 256);
    juce::MidiBuffer m;
    while (!stop.load()) {
      buf.clear();
      p.processBlock(buf, m);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  });
  juce::MemoryBlock b;
  p.getStateInformation(b);
  stop.store(true);
  audio.join();
  return b;
}

// projet de référence : D2 (GL1) et S2 (GL2) gelés et armés, deux captures (comme buildFrozen des tests G4.2/G4.3)
juce::MemoryBlock frozenProject(std::vector<CaptureAsset>& capsOut) {
  auto p = std::make_unique<SubdestrucktProcessor>();
  p->prepareToPlay(48000.0, 256);
  setP(*p, ParamId::DstGl1Type, static_cast<float>(DstType::D2));
  setP(*p, ParamId::DstGl1Arm, 1.0f);
  setP(*p, ParamId::DstGl2Type, static_cast<float>(DstType::S2));
  setP(*p, ParamId::DstGl2Arm, 1.0f);
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  midi.addEvent(juce::MidiMessage::noteOn(1, 52, static_cast<juce::uint8>(120)), 0);
  for (int i = 0; i < 300; ++i) {
    if (i == 150) {
      setP(*p, ParamId::DstGl1D2Freeze, 1.0f);
      setP(*p, ParamId::DstGl2S2Freeze, 1.0f);
    }
    if (i == 200) midi.addEvent(juce::MidiMessage::noteOff(1, 52), 0);
    buf.clear();
    p->processBlock(buf, midi);
    midi.clear();
  }
  p->requestCapture(4);
  p->requestCapture(5);
  buf.clear();
  p->processBlock(buf, midi);
  p->collectCaptures((1u << 4) | (1u << 5));
  capsOut = p->captures();
  juce::MemoryBlock b;
  p->getStateInformation(b);
  return b;
}

bool sameCaps(const std::vector<CaptureAsset>& a, const std::vector<CaptureAsset>& b) {
  if (a.size() != b.size()) return false;
  for (const auto& x : a) {
    bool found = false;
    for (const auto& y : b) found = found || (y.slot == x.slot && y.kind == x.kind && y.frames == x.frames && y.data == x.data);
    if (!found) return false;
  }
  return true;
}

double dbfs(double v) { return v > 1e-12 ? 20.0 * std::log10(v) : -240.0; }

}  // namespace

void testTaR1Wrapper() {
  std::vector<CaptureAsset> caps;
  const juce::MemoryBlock blob = frozenProject(caps);
  PlayHeadR1 ph;
  ph.mode = 1;
  auto p = std::make_unique<SubdestrucktProcessor>();
  p->prepareToPlay(48000.0, 256);
  p->setPlayHead(&ph);
  p->setStateInformation(blob.getData(), static_cast<int>(blob.getSize()));
  // ouverture du projet : transport arrêté observé, silence, suspension
  const Run open = runR1(*p, 50);
  const bool susp0 = p->autonomySuspended() && !p->autonomyAuthorized();
  // export sans PLAY préalable : rendu hors temps réel (VST3 kOffline), transport en lecture, note jouée
  p->setNonRealtime(true);
  ph.mode = 2;
  juce::MidiBuffer note;
  note.addEvent(juce::MidiMessage::noteOn(1, 45, static_cast<juce::uint8>(100)), 0);
  const Run render = runR1(*p, 300, &note);
  const int srcRender = p->telemetry().autonomySource.load();
  const juce::MemoryBlock duringSave = saveWithAudioThread(*p);  // sauvegarde pendant le rendu (TA-R14)
  // annulation de l'export : retour en temps réel, transport arrêté
  p->setNonRealtime(false);
  ph.mode = 1;
  const Run cancel = runR1(*p, 400);
  const bool authAfter = p->autonomyAuthorized(), suspAfter = p->autonomySuspended();
  const juce::MemoryBlock afterSave = saveWithAudioThread(*p);  // sauvegarde après l'annulation (TA-R14)
  const bool t1 = open.energy < 1e-9 && susp0 && render.energy > 1e-3 && srcRender == static_cast<int>(AuthSource::Render) && cancel.energy < 1e-9 &&
                  !authAfter && suspAfter;
  TK_CHECK(t1);
  tk::result("TA-R01w", t1, "processeur, scenario R-28 : ouverture (ARRET) energie %.2e, suspendu %d ; export (setNonRealtime, LECTURE) : contenu %.3g, source %s (rendu attendu) ; annulation puis ARRET temps reel 2,1 s : energie %.3g, crete %.1f dBFS (silence attendu) ; autorise %d (0 attendu), suspendu %d (1 attendu)",
             open.energy, susp0, render.energy, authSourceName(static_cast<AuthSource>(srcRender)), cancel.energy, dbfs(cancel.peak), authAfter, suspAfter);
  // TA-R14 : état persistant inchangé (aucune capture automatique d'un rendu, configuration intacte)
  const bool t14 = duringSave == blob && afterSave == blob && sameCaps(p->captures(), caps);
  TK_CHECK(t14);
  tk::result("TA-R14", t14, "etat persistant : sauvegarde pendant le rendu = etat restaure %d ; apres l'annulation = etat restaure %d ; captures du processeur intactes %d (thread audio concurrent, capture automatique possible)",
             duringSave == blob, afterSave == blob, sameCaps(p->captures(), caps));
  // TA-R13w : premier véritable ARRÊT -> LECTURE : autorisation normale, captures persistées restituées
  ph.mode = 1;
  runR1(*p, 10);
  ph.mode = 2;
  const Run play = runR1(*p, 300);
  const bool t13 = p->autonomyAuthorized() && p->telemetry().autonomySource.load() == static_cast<int>(AuthSource::Transport) && play.energy > 1e-3 &&
                   sameCaps(p->captures(), caps);
  TK_CHECK(t13);
  tk::result("TA-R13w", t13, "premier ARRET->LECTURE apres l'export : autorise par %s (transport attendu) %d ; gel restitue %.3g ; captures persistees intactes %d",
             authSourceName(static_cast<AuthSource>(p->telemetry().autonomySource.load())), p->autonomyAuthorized(), play.energy, sameCaps(p->captures(), caps));
  // TA-R15w : sécurité temps réel du wrapper
  const bool t15 = gAllocR1w == 0;
  TK_CHECK(t15);
  tk::result("TA-R15w", t15, "processBlock sur les sequences TA-R (ouverture, rendu, annulation, lecture) : %lld allocation(s)", gAllocR1w);
}
