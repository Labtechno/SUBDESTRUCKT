// SUBDESTRUCKT — L2 — moteur temps réel ARCH-G4.
//
// Chaîne : MIDI mono (priorités, legato, sustain, Drone) -> sources de modulation (5 ENV, 4 LFO, aléatoires, chaos,
//          MIDI, macros, suiveurs) -> matrice globale (64) -> réseau WT / OP1..OP4 / BUS (PM, LinFM, TZFM, ExpFM)
//          avec CROSS bidirectionnel B-T2 (55 routes spécialisées) -> Amp (avant tout traitement à queue)
//          -> chaînes locales WT et FM (filtre + 2 destruction, Pre/Post) -> Cross Audio -> chaîne globale
//          -> décimation K -> gain de sortie -> garde DC -> garde numérique -> limiteur musical optionnel -> Panic.
// Horloge commune à Fs moteur = Fs hôte x K. K = 2 en permanence à 44,1 kHz (B-T2) ; politique Quality : voir
// oversamplingFor(). Arêtes Immediate : sortie du nœud source à l'échantillon courant (ordre topologique). Arêtes
// Delayed : historique de la source retardé de D = Fs_moteur/48000 (entier exact ou Thiran1).
// Transitions de topologie : fondu de sortie des arêtes retirées, bascule atomique, fondu d'entrée ; jamais d'union.
// process() n'alloue pas, ne verrouille pas, ne fait aucune I/O.
#pragma once
#include <algorithm>
#include <array>
#include <complex>
#include <atomic>
#include <limits>
#include <memory>
#include <vector>

#include "../core/ModMatrix.h"
#include "../core/Parameters.h"
#include "../core/StateCodec.h"
#include "../core/Topology.h"
#include "../dsp/CausalDelay.h"
#include "../dsp/Decimator.h"
#include "../dsp/Envelope.h"
#include "../dsp/Fft.h"
#include "../dsp/ModSources.h"
#include "../dsp/WtBank.h"
#include "Chain.h"
#include "MidiEvent.h"
#include "ModRuntime.h"
#include "Telemetry.h"
#include "AutonomyGate.h"
#include "TopologyExchange.h"

#if defined(SBD_RTSAN) && defined(__clang__)
#define SBD_NONBLOCKING [[clang::nonblocking]]
#else
#define SBD_NONBLOCKING
#endif

namespace sbd {

struct EngineSetup {
  double hostRate = 0.0;
  int oversampling = 1;
  double engineRate = 0.0;
  DelaySpec delay;
  int latencySamples = 0;        // latence totale déclarée (décimation + chaînes), échantillons hôte
  int decimatorLatency = 0;      // part de la décimation
  int maxBlock = 0;
  Quality quality = Quality::Normal;
  bool supported = false;
};

struct TransportInfo {
  bool valid = false;      // information de transport disponible (sinon : INCONNU, jamais assimilé à un ARRÊT)
  bool playing = false;
  bool recording = false;  // ARCH-G4.3 : début d'enregistrement (TRANSPORT AUTHORIZATION)
  bool offline = false;    // ARCH-G4.3 : traitement hors temps réel (VST3 kOffline) : rendu
  double bpm = 120.0;
  double ppq = 0.0;  // position en noires au début du bloc
};

// Captures R2 à restaurer (préparées hors RT, publiées au thread audio).
struct CaptureSet {
  std::vector<CaptureAsset> items;
  bool apply = true;  // false : ensemble retenu seulement (contenu déjà présent dans les tampons, ex. après une capture)
};

// Référence de banque publiée vers le thread audio (le shared_ptr est libéré hors RT au retrait).
struct BankRef {
  std::shared_ptr<const WtBank> bank;
};

class Engine {
 public:
  explicit Engine(std::shared_ptr<const WtBank> bank = nullptr);
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;

  // Politique de K : Normal = politique 0.1.0-b5 (44,1 kHz -> 2, 48/88,2/96 kHz -> 1) ; Eco = Normal (K=2 permanent à
  // 44,1 kHz interdit toute baisse) ; High = 2 x Normal. 0 si la fréquence n'est pas supportée.
  static int oversamplingFor(double hostRate, Quality q = Quality::Normal) noexcept;

  // ---- hors RT
  bool prepare(double hostRate, int maxBlockHost, Quality q = Quality::Normal);  // alloue, remet le runtime à zéro
  void setTopologyImmediate(const CompiledTopology& topology);  // hors lecture uniquement : sans transition
  void setModMatrixImmediate(const CompiledMod& mod);
  void setBankImmediate(std::shared_ptr<const WtBank> bank);
  void setLfoCurves(const std::array<CurvePoints, 4>& curves);  // hors lecture (prepare) ou via exchange
  void setSeed(std::uint64_t seed);                             // hors lecture : réinitialise les flux runtime
  const EngineSetup& setup() const noexcept { return setup_; }
  TopologyExchange& exchange() noexcept { return exchange_; }
  SpscExchange<CompiledMod>& modExchange() noexcept { return modExchange_; }
  SpscExchange<BankRef>& bankExchange() noexcept { return bankExchange_; }
  SpscExchange<std::array<CurvePoints, 4>>& curveExchange() noexcept { return curveExchange_; }
  Telemetry& telemetry() noexcept { return telemetry_; }
  // Latence que la configuration structurelle courante exige (hors RT : lue après publication des paramètres).
  static int chainLatencyFor(const ParamValues& p, double engineRate, int K, Quality q = Quality::Normal) noexcept;

  // ---- commandes instantanées (tout thread, atomiques)
  void setDrone(bool on) noexcept { droneCmd_.store(on ? 1 : 0, std::memory_order_release); }
  // Autorisation locale d'activation autonome (ARCH-G4.2, AUT) : état d'exécution NON persisté, distinct de la
  // configuration. Sans autorisation, les 9 armements configurés sont appliqués comme désarmés (armement effectif =
  // configuré ET autorisé) ; la configuration (base) reste intacte. Retirée par le Panic si un armement est configuré.
  // ARCH-G4.3 (TRANSPORT AUTHORIZATION) : machine d'états du thread audio (AutonomyGate) ; false = suspension (nouvelle
  // génération de chargement), true = autorisation explicite. Effet à la frontière de bloc suivante.
  void setAutonomyAuthorized(bool on) noexcept {
    if (on)
      auth_.requestAuthorize();
    else
      auth_.requestSuspend();
  }
  bool autonomyAuthorized() const noexcept { return auth_.authorizedView(); }
  TaLog& taLog() noexcept { return taLog_; }
  // captures retenues dont l'application est en attente (le thread de contrôle ne doit pas les remplacer par un tampon
  // d'exécution non encore restauré)
  bool capturesPending() const noexcept { return capturesPendingPub_.load(std::memory_order_acquire); }
  void requestCrossReset() noexcept { crossResetCmd_.store(true, std::memory_order_release); }
  void requestManualExcite() noexcept { exciteCmd_.store(true, std::memory_order_release); }
  // Captures R2 : demande (tout thread) ; lecture du résultat (thread de contrôle) quand il est prêt.
  void requestCapture(int slot) noexcept { captureReq_.fetch_or(1u << slot, std::memory_order_acq_rel); }
  bool fetchCapture(int slot, CaptureAsset& out);
  SpscExchange<CaptureSet>& captureExchange() noexcept { return captureExchange_; }

  // ---- RT
  void setParameters(const ParamValues& base) noexcept;  // valeurs Base -> cibles des rampes
  void setTransport(const TransportInfo& t) noexcept { transport_ = t; }
  void process(float* outL, float* outR, int numSamples, const MidiEvent* events, int numEvents,
               double* preDcTap = nullptr) noexcept SBD_NONBLOCKING;

  // ---- crochets de test (sans effet en usage normal)
  void testSetPoison(bool on) noexcept { poison_ = on; }
  void testSetReverseOrder(bool on) noexcept { reverseOrder_ = on; }
  Envelope::Stage testEnvStage(int k) const noexcept { return envs_[static_cast<size_t>(k)].stage(); }
  // contre-épreuve SPEC-HR-01 (tests uniquement, aucun paramètre ni état) : moteur sans ELP
  void setElpEnabledForTest(bool on) noexcept { elpEnabled_ = on; }
  struct TestCounters {
    std::uint32_t elp, clamp, limiter;
  };
  TestCounters testCounters() const noexcept { return {elpHits_, clampHits_, limiterHits_}; }
  void testInjectNaN() noexcept { lines_[kNodeOP1].poisonForTest(std::numeric_limits<double>::quiet_NaN()); }
  long long testOverlapSamples() const noexcept { return overlapSamples_; }
  double testEffectiveDepth(int slot) const noexcept;
  double testEffective(ParamId id) const noexcept { return eff_[static_cast<size_t>(id)]; }
  double testSource(ModSource s) const noexcept { return src_[static_cast<size_t>(s)]; }
  int testTransitionPhase() const noexcept { return static_cast<int>(transition_); }
  const CompiledTopology* testActiveTopology() const noexcept { return active_; }
  int testCurrentNote() const noexcept { return gate_ ? currentNote_ : -1; }
  double testF0() const noexcept { return f0_; }
  long long testSpectralResynth() const noexcept { return smResynth_; }
  long long testSpectralBlends() const noexcept { return smBlends_; }

 private:
  enum class Transition { Idle = 0, FadeOut = 1, FadeIn = 2 };
  static constexpr int kMaxEvents = 1024;

  void processChunk(float* outL, float* outR, int n, const MidiEvent* events, int numEvents, double* tap) noexcept;
  void renderSegment(int a, int b, float* outL, float* outR, double* tap) noexcept;
  void tick(double& L, double& R) noexcept;
  void updateSmoothing() noexcept;
  void updateSources() noexcept;
  void evaluateModulation() noexcept;
  double finalize(int dest, double acc) const noexcept;
  void evaluateNetwork(double& wtAudio, double& fmL, double& fmR) noexcept;
  double readWt(double phase, double inc, double slope, double x, double y) const noexcept;
  bool spectralActive() const noexcept;
  void spectralUpdate(double x, double y, double absInc, const int* zt, const double* zv, const double* zs) noexcept;
  double readOpWave(int op, int wave, double phase, double inc) const noexcept;
  void handleEvent(const MidiEvent& e) noexcept;
  void noteOn(int note, int velocity) noexcept;
  void noteOff(int note) noexcept;
  void releaseGateIfIdle() noexcept;
  int selectNote() const noexcept;
  void triggerVoice(bool retrigger, bool gateOpened, bool newNote) noexcept;
  void updateF0() noexcept;
  void beginTransition(CompiledTopology* incoming) noexcept;
  void swapToIncoming() noexcept;
  void advanceTransition() noexcept;
  void purgeStates() noexcept;
  void crossReset() noexcept;
  void applyEnvelopeParameters() noexcept;
  void applyAutonomyMask() noexcept;
  bool anyArmConfigured() const noexcept;
  void publishTelemetry() noexcept;
  double e(ParamId id) const noexcept { return eff_[static_cast<size_t>(id)]; }
  int ch(ParamId id) const noexcept { return static_cast<int>(eff_[static_cast<size_t>(id)] + 0.5); }

  std::shared_ptr<const WtBank> ownedBank_;  // banque initiale (possédée hors RT)
  const WtBank* bank_ = nullptr;
  BankRef* activeBankRef_ = nullptr;
  std::shared_ptr<const WtBank> opWaves_;
  EngineSetup setup_;
  Telemetry telemetry_;
  TopologyExchange exchange_;
  SpscExchange<CompiledMod> modExchange_;
  SpscExchange<BankRef> bankExchange_;
  SpscExchange<std::array<CurvePoints, 4>> curveExchange_;
  SpscExchange<CaptureSet> captureExchange_;
  std::atomic<std::uint32_t> captureReq_{0};
  CaptureSet* retained_ = nullptr;  // captures retenues (possédées par le thread audio jusqu'au remplacement)
  bool captureApplyPending_ = false;
  std::atomic<bool> capturesPendingPub_{false};
  AutonomyGate auth_;
  TaLog taLog_;
  std::uint64_t taClock_ = 0;
  bool droneEff() const noexcept { return drone_ && auth_.authorized(); }
  std::uint32_t captureApplied_ = 0;
  long long captureWait_ = 0;
  std::array<std::atomic<bool>, 6> captureReady_{};
  std::array<std::vector<float>, 6> captureBuf_;
  std::array<int, 6> captureFrames_{}, captureKind_{};
  DestructionSlot& slotAt(int k) noexcept { return (k < 2 ? chainWt_ : (k < 4 ? chainFm_ : chainGl_)).slot(k % 2); }
  Decimator decL_{1}, decR_{1};
  std::vector<double> engineL_, engineR_, hostL_, hostR_;
  std::array<MidiEvent, kMaxEvents> events_{};
  std::array<MidiEvent, kMaxEvents> chunkEvents_{};
  TransportInfo transport_;
  double ppqAtSample_ = 0.0;

  // paramètres : Base lissée (rampes) puis Effective (après modulation)
  ParamValues base_ = ParamValues::defaults();
  std::array<LinearRamp, kNumParams> ramps_{};
  std::array<std::int16_t, kNumParams> activeRamps_{};
  std::array<bool, kNumParams> rampActive_{};
  int numActiveRamps_ = 0;
  std::array<double, kNumParams> cur_{};  // Base lissée
  std::array<double, kNumParams> eff_{};  // Effective
  std::array<bool, kNumParams> modulated_{};
  LinearRamp outGain_;

  // modulation
  CompiledMod* mod_ = nullptr;
  std::array<double, kNumModSources> src_{};
  std::array<double, kMaxModRoutes> routeSmooth_{}, routeCtl_{}, routeCtlStep_{};
  std::array<double, kMaxModRoutes> destAcc_{};
  int controlCounter_ = 0, controlPeriod_ = 48;
  std::array<Envelope, 5> envs_{};  // Amp, WT, FM, AUX1, AUX2
  std::array<Lfo, 4> lfos_{};
  std::array<LfoCurve, 4> lfoCurves_{};
  RandomSource rndHold_, rndSmooth_;
  Chaos chaos_;
  std::array<Follower, 3> followers_{};
  std::uint64_t seed_ = 0x5B0D5EEDULL;
  RngStream phaseRng_;
  RngStream mutRng_;

  // voix (V1 monophonique)
  std::array<std::uint8_t, 128> stack_{};
  std::array<bool, 128> sustained_{};
  int stackSize_ = 0;
  int currentNote_ = 60;
  bool gate_ = false;
  int velocity_ = 100;
  double bend_ = 0.0;  // -1..1
  bool sustainPedal_ = false;
  bool drone_ = false;
  std::atomic<int> droneCmd_{-1};
  std::atomic<bool> crossResetCmd_{false}, exciteCmd_{false};
  bool noteOnEdge_ = false, manualExcitePending_ = false;
  double f0_ = 261.6255653005986;

  // réseau
  std::array<double, kNumNodes> oscPhase_{}, out_{}, modOut_{}, delayed_{};
  std::array<CausalDelayLine, kNumNodes> lines_{};
  std::array<bool, kNumNodes> wrapped_{};
  std::array<double, kNumOps + 1> acState_{};  // passe-haut AC des sorties de modulation des opérateurs
  double syncMasterPhase_ = 0.0;
  bool syncMasterWrapped_ = false;
  double wtDirection_ = 1.0;  // sync douce
  double prevPmWT_ = 0.0;
  std::array<double, kNumOps + 1> prevPmOp_{};
  std::array<double, kNumOps + 1> opCharHold_{};
  // Z / Character
  double zWalkSpeed_ = 0.0, zWalkAmp_ = 0.0, zRmsIn_ = 0.0, zRmsOut_ = 0.0;
  double charHold_ = 0.0;
  std::vector<double> smA_, smB_;                 // morph spectral : tables de départ et d'arrivée (préallouées)
  std::vector<std::complex<double>> smWork_;
  Fft smFft_;
  int smCounter_ = 0;
  bool smValid_ = false, smSettled_ = false;
  std::array<double, 18> smKey_{};
  // Z spectral (ARCH-G4.3) : coefficients de la forme morphée, copie d'origine, tampon de travail (préalloués)
  std::vector<std::complex<double>> smCoef_, smOrig_, smZWork_;
  bool zSpecActive_ = false;
  std::array<double, 8> smBlendKey_{};  // clé de la combinaison des formes en cache (smOrig_)
  long long smResynth_ = 0, smBlends_ = 0;  // compteurs de test (resynthèses, combinaisons recalculées)
  std::array<double, 2> foldPrev_{};          // ADAA1 du repli (par emplacement Z)
  std::array<double, kNumNodes> lastInc_{};   // incrément de phase du dernier échantillon (sync sous-échantillon)
  double syncGhostPhase_ = 0.0;
  int syncFade_ = 0, syncFadeLength_ = 4;
  int charCounter_ = 0;
  CompiledTopology* active_ = nullptr;
  CompiledTopology* incoming_ = nullptr;
  Transition transition_ = Transition::Idle;
  int transitionLeft_ = 0, transitionLength_ = 128;
  std::array<double, kMaxRoutes> edgeGain_{};
  std::array<bool, kMaxRoutes> edgeRemoved_{};
  std::array<bool, kMaxRoutes> edgeAdded_{};
  long long overlapSamples_ = 0;
  // changement de banque : fondu court du WT
  BankRef* pendingBank_ = nullptr;
  int bankFade_ = 0, bankFadeLength_ = 256;
  bool bankFadingOut_ = false;

  // traitements
  ProcessingChain chainWt_, chainFm_, chainGl_;
  // ligne d'alignement à retard variable entier (préallouée)
  struct AlignDelay {
    std::vector<double> buf;
    int pos = 0;
    void configure(int n) {
      buf.assign(static_cast<size_t>(n), 0.0);
      pos = 0;
    }
    void process(double& x, int d) noexcept {
      const int N = static_cast<int>(buf.size());
      buf[static_cast<size_t>(pos)] = x;
      x = buf[static_cast<size_t>((pos - d + N) % N)];
      pos = (pos + 1) % N;
    }
    void reset() noexcept { std::fill(buf.begin(), buf.end(), 0.0); }  // RT : aucune allocation
  };
  AlignDelay alignWtL_, alignWtR_, alignFmL_, alignFmR_, alignOutL_, alignOutR_;
  bool padActive_ = false;

  // sortie et sûreté
  std::array<DcGuard, 2> dc_{};
  double limGain_ = 1.0;
  bool purgeRequested_ = false;
  bool muteRest_ = false;
  bool elpEnabled_ = true;
  bool panicActive_ = false;
  int panicLeft_ = 0, panicLength_ = 1;
  float blockPeakL_ = 0.0f, blockPeakR_ = 0.0f;
  std::uint32_t nonFinite_ = 0, clampHits_ = 0, pmClampHits_ = 0, dropped_ = 0, panics_ = 0, limiterHits_ = 0, elpHits_ = 0;
  double envBpm_ = 120.0;
  double tailEnergy_ = 0.0;
  std::uint32_t scopeCounter_ = 0;

  // tests
  bool poison_ = false;
  bool reverseOrder_ = false;
};

}  // namespace sbd
