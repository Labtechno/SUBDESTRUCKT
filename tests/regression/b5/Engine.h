// ORACLE FIGE 0.1.0-b5 (copie de 8d23342, namespace sbd_b5) : ne jamais modifier. Reference de non-regression ARCH-G4.
// SUBDESTRUCKT — L2 — moteur temps réel de la tranche ARCH-G3.
//
// Chaîne : MIDI mono -> voix (priorité dernière note) -> réseau WT / OP1 / OP2 (PM) avec CROSS bidirectionnel B-T2
//          -> mixage -> Amp ADSR (au rythme moteur, avant décimation) -> décimation K -> gain de sortie -> garde DC
//          -> garde numérique -> Panic -> stéréo.
// Horloge commune à Fs moteur = Fs hôte x K. K = 2 en permanence à 44,1 kHz ; K = 1 à 48 / 88,2 / 96 kHz.
// Arêtes Immediate : lecture de la sortie du nœud source à l'échantillon courant (ordre topologique).
// Arêtes Delayed : lecture de l'historique de la source retardé de D = Fs_moteur/48000 (entier exact ou Thiran1).
// Transitions de topologie séquentielles : fondu de sortie des arêtes retirées (ancien graphe), bascule atomique,
// fondu d'entrée des arêtes ajoutées (nouveau graphe) ; l'union n'est jamais exécutée.
// process() n'alloue pas, ne verrouille pas, ne fait aucune I/O.
#pragma once
#include <array>
#include <limits>
#include <memory>
#include <vector>

#include "Parameters.h"
#include "Topology.h"
#include "CausalDelay.h"
#include "Decimator.h"
#include "Envelope.h"
#include "Wavetable.h"
#include "MidiEvent.h"
#include "Telemetry.h"
#include "TopologyExchange.h"

// Annotation RealtimeSanitizer (LLVM >= 20, -fsanitize=realtime) : process() est un contexte temps réel ; toute
// allocation, libération, prise de verrou ou I/O dans son étendue dynamique est signalée. Active seulement dans le job
// CI dédié (-DSBD_RTSAN) pour ne pas bruiter la compilation normale.
#if defined(SBD_RTSAN) && defined(__clang__)
#define SBD_B5_NONBLOCKING [[clang::nonblocking]]
#else
#define SBD_B5_NONBLOCKING
#endif

namespace sbd_b5 {

struct EngineSetup {
  double hostRate = 0.0;
  int oversampling = 1;
  double engineRate = 0.0;
  DelaySpec delay;
  int latencySamples = 0;
  int maxBlock = 0;
  bool supported = false;
};

class Engine {
 public:
  explicit Engine(std::shared_ptr<const SawWavetable> table);
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;

  // Politique de K de la tranche (provisoire, Quality non implémentée) ; 0 si la fréquence n'est pas supportée.
  static int oversamplingFor(double hostRate) noexcept;

  // ---- hors RT
  bool prepare(double hostRate, int maxBlockHost);           // alloue et remet l'état runtime à zéro
  void setTopologyImmediate(const CompiledTopology& topology);  // hors lecture uniquement : sans transition
  const EngineSetup& setup() const noexcept { return setup_; }
  TopologyExchange& exchange() noexcept { return exchange_; }
  Telemetry& telemetry() noexcept { return telemetry_; }

  // ---- RT
  void setParameters(const ParamValues& base) noexcept;  // valeurs Base -> cibles des rampes
  void process(float* outL, float* outR, int numSamples, const MidiEvent* events, int numEvents,
               double* preDcTap = nullptr) noexcept SBD_B5_NONBLOCKING;

  // ---- crochets de test (sans effet en usage normal)
  void testSetPoison(bool on) noexcept { poison_ = on; }
  void testSetReverseOrder(bool on) noexcept { reverseOrder_ = on; }
  void testInjectNaN() noexcept { lines_[kNodeOP1].poisonForTest(std::numeric_limits<double>::quiet_NaN()); }
  long long testOverlapSamples() const noexcept { return overlapSamples_; }
  double testEffectiveDepth(int slot) const noexcept { return effective_[static_cast<size_t>(slot)]; }
  int testTransitionPhase() const noexcept { return static_cast<int>(transition_); }
  const CompiledTopology* testActiveTopology() const noexcept { return active_; }

 private:
  enum class Transition { Idle = 0, FadeOut = 1, FadeIn = 2 };
  static constexpr int kMaxEvents = 1024;

  void processChunk(float* outL, float* outR, int n, const MidiEvent* events, int numEvents, double* tap) noexcept;
  void renderSegment(int a, int b, float* outL, float* outR, double* tap) noexcept;
  double tick() noexcept;
  void handleEvent(const MidiEvent& e) noexcept;
  void noteOn(int note, int velocity) noexcept;
  void noteOff(int note) noexcept;
  void beginTransition(CompiledTopology* incoming) noexcept;
  void swapToIncoming() noexcept;
  void advanceTransition() noexcept;
  void purgeStates() noexcept;
  void publishTelemetry() noexcept;

  std::shared_ptr<const SawWavetable> table_;
  EngineSetup setup_;
  Telemetry telemetry_;
  TopologyExchange exchange_;
  Decimator decimator_{1};
  std::vector<double> engineBuf_, hostBuf_;
  std::array<MidiEvent, kMaxEvents> events_{};
  std::array<MidiEvent, kMaxEvents> chunkEvents_{};

  // paramètres
  ParamValues base_ = ParamValues::defaults();
  std::array<LinearRamp, kNumParams> ramps_{};  // rampes au rythme moteur (sauf gain de sortie : rythme hôte)
  LinearRamp outGain_;
  std::array<double, kMaxRoutes> effective_{};

  // voix
  std::array<std::uint8_t, 128> stack_{};
  int stackSize_ = 0;
  int currentNote_ = 60;
  bool gate_ = false;
  int velocity_ = 100;
  Adsr adsr_;

  // réseau
  std::array<double, kNumNodes> oscPhase_{}, out_{}, delayed_{};
  std::array<CausalDelayLine, kNumNodes> lines_{};
  double prevPmWT_ = 0.0;
  CompiledTopology* active_ = nullptr;
  CompiledTopology* incoming_ = nullptr;
  Transition transition_ = Transition::Idle;
  int transitionLeft_ = 0, transitionLength_ = 128;
  std::array<double, kMaxRoutes> edgeGain_{};   // par indice d'arête de la topologie active
  std::array<double, kMaxRoutes> edgeStep_{};
  std::array<bool, kMaxRoutes> edgeRemoved_{};  // arête de l'ancien graphe en cours de retrait
  std::array<bool, kMaxRoutes> edgeAdded_{};    // arête du nouveau graphe en cours d'ajout
  long long overlapSamples_ = 0;

  // sortie et sûreté
  DcGuard dc_;
  bool purgeRequested_ = false;
  bool muteRest_ = false;
  bool panicActive_ = false;
  int panicLeft_ = 0, panicLength_ = 1;
  float blockPeakL_ = 0.0f, blockPeakR_ = 0.0f;
  std::uint32_t nonFinite_ = 0, clampHits_ = 0, pmClampHits_ = 0, dropped_ = 0, panics_ = 0;

  // tests
  bool poison_ = false;
  bool reverseOrder_ = false;
};

}  // namespace sbd_b5
