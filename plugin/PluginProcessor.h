// SUBDESTRUCKT — L0 — adaptateur hôte JUCE/VST3 (V1-SOFTWARE-ARCHITECTURE §2 L0, §10).
//
// Responsabilités : paramètres hôte (StableID <-> identité hôte, jamais d'indice), MIDI -> événements du cœur,
// transport, sortie stéréo, état versionné (codec du cœur), latence déclarée, éditeur.
// L'état éditable non-hôte (routes, matrice, banque, noms, courbes, seed, instance) vit dans ControlState, protégé par
// controlMutex_ (jamais pris par le thread audio) ; il est compilé hors RT puis publié sans verrou vers le moteur.
// Thread audio : processBlock() n'alloue pas, ne verrouille pas, ne fait aucune I/O (vérifié par tests/wrapper).
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "core/ModMatrix.h"
#include "core/PresetOps.h"
#include "core/WtAssets.h"
#include "core/Parameters.h"
#include "core/StateCodec.h"
#include "core/WtLibrary.h"
#include "core/Topology.h"
#include "engine/Engine.h"

namespace sbd {

// Pont paramètres hôte -> valeurs Base exactes. Après une restauration d'état, la valeur Base sauvegardée est
// conservée telle quelle (l'aller-retour hôte normalisé 0..1 en float peut différer d'un ulp) tant que l'hôte ne
// modifie pas le paramètre. Lecture sans verrou ni allocation (thread audio).
class ParameterBridge {
 public:
  void attach(int index, juce::RangedAudioParameter* p) noexcept { params_[static_cast<size_t>(index)] = p; }
  juce::RangedAudioParameter* param(int index) const noexcept { return params_[static_cast<size_t>(index)]; }
  float base(int index) const noexcept;
  void setExact(int index, float value);  // thread de contrôle
  ParamValues values() const noexcept;

 private:
  std::array<juce::RangedAudioParameter*, kNumParams> params_{};
  std::array<std::atomic<float>, kNumParams> exact_{};
  std::array<std::atomic<float>, kNumParams> hostValueAtRestore_{};
  std::array<std::atomic<bool>, kNumParams> hasExact_{};
};

// MIDI Learn : correspondance globale CC -> paramètre, propre à l'utilisateur, jamais dans un preset (P0-C12).
struct MidiLearnMap {
  std::array<std::int16_t, 128> ccToParam{};  // -1 = libre
  MidiLearnMap() { ccToParam.fill(-1); }
  static std::string defaultPath();
  // profil utilisé : chemin par défaut, ou chemin isolé fixé par les tests (jamais le profil réel en test)
  static std::string path();
  static void setPathOverride(const std::string& p);
  // politique (ARCH-G4.2, ML) : CC réservées jamais apprises (Bank Select, Data Entry/RPN/NRPN, sustain, modes)
  static constexpr bool reservedCc(int cc) noexcept {
    return cc == 0 || cc == 6 || cc == 32 || cc == 38 || cc == 64 || (cc >= 96 && cc <= 101) || cc >= 120;
  }
  static bool learnableParam(int paramIndex) noexcept;  // automatisable et hors des 9 armements autonomes
  bool operator==(const MidiLearnMap& o) const { return ccToParam == o.ccToParam; }
  bool load(const std::string& path);
  bool save(const std::string& path) const;
};

class SubdestrucktProcessor : public juce::AudioProcessor, private juce::Timer {
 public:
  SubdestrucktProcessor();
  ~SubdestrucktProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
  void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;
  using juce::AudioProcessor::processBlock;

  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }
  const juce::String getName() const override { return "SUBDESTRUCKT"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override;

  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return "Init"; }
  void changeProgramName(int, const juce::String&) override {}

  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  // ---- API de contrôle (thread message / UI), jamais appelée depuis le thread audio
  RouteTable::Result applyTopology(const std::vector<RouteTable::Op>& ops);
  RouteTable::Result applyFmTopology(int index);
  std::vector<Route> currentRoutes() const;
  std::uint32_t currentNextRouteId() const;
  ModTxResult modAdd(const ModRoute& r);
  ModTxResult modUpdate(const ModRoute& r);
  ModTxResult modRemove(std::uint32_t id);
  ModMatrixState currentMod() const;
  bool setBank(const std::string& id);
  // Import WAV (cycles de `cycleLength` échantillons) -> banque utilisateur immuable (id SHA-256), sélectionnée.
  bool importBankWav(const std::string& path, int cycleLength);
  bool exportBankWav(const std::string& path);  // contenu brut exact de la banque active (float32)
  // Banques utilisateur (ARCH-G4.1 §3) : génération procédurale, édition d'une forme, bibliothèque .sdwt portable.
  // Chaque opération crée une nouvelle banque immuable, sélectionnée par une transaction annulable.
  bool generateBank(WtProcedure p, int nx, int ny, std::uint64_t seed, double amount);
  bool editCurrentWave(int wave, const std::vector<float>& samples);
  bool saveBankToLibrary(const std::string& name);
  std::vector<BankFileInfo> bankLibrary() const;
  bool loadBankFromLibrary(const std::string& path);
  std::string currentBank() const;
  void setMacroName(int index, const std::string& name);
  std::array<std::string, 4> macroNames() const;
  void setLfoCurve(int index, const CurvePoints& c);
  std::array<CurvePoints, 4> lfoCurves() const;
  void setQuality(Quality q);
  Quality quality() const;
  void setDrone(bool on);
  bool drone() const;
  void setDroneRestore(bool allowed);
  bool droneRestore() const;
  // Captures créatives R2 (Glitch D2 / Freeze S2) : demande au thread audio, collecte, effacement.
  void requestCapture(int slot);
  // Autorisation locale d'activation autonome (AUT, ARCH-G4.2) : non persistée ; retirée par une restauration externe
  // (projet DAW, duplication, preset) portant un armement ou une capture, et par le Panic ; accordée par Autoriser.
  void authorizeAutonomy();
  // chargement d'un preset déjà décodé (chemin commun de loadPreset ; tests) : désarmement, captures retenues
  bool applyPresetState(PluginState s, const std::string& name);
  bool autonomyAuthorized() const noexcept { return engine_.autonomyAuthorized(); }
  bool autonomySuspended() const;  // armement configuré ou capture retenue, sans autorisation
  std::uint32_t collectCaptures(std::uint32_t mask);
  void clearCapture(int slot);
  std::vector<CaptureAsset> captures() const;
  void requestCrossReset() noexcept { engine_.requestCrossReset(); }
  void requestManualExcite() noexcept { engine_.requestManualExcite(); }
  void requestPanic() noexcept { panicRequested_.store(true); }
  std::string lastMessage() const;
  bool applyState(const PluginState& state, bool fromPreset = false);
  void loadDefaultPreset() { applyState(defaultState(), true); }
  PluginState snapshotState() const;
  std::string encodeCurrentState(StateScope scope = StateScope::Daw) const;
  // Transaction unique sur l'état (Undo/Redo, Randomize...) : fonction appliquée à une copie, validée, appliquée.
  bool transact(const std::function<bool(PluginState&)>& edit, const std::string& cause);
  bool setMidiLearnTarget(int paramIndex);  // refus (message) si la cible n'est pas apprenable ; -1 annule
  int midiLearnTarget() const noexcept { return learnTarget_.load(); }
  MidiLearnMap midiLearn() const;
  void clearMidiLearn(int cc);
  // service du thread message (appelé par le timer ; appelable par les tests) : libérations hors RT, MIDI Learn
  // (capture, contrôleurs appris, relecture du profil modifié par une autre instance), Evolve, latence
  void serviceMessageThread();
  void setStateListener(std::function<void()> f);  // notifié (thread message) après tout changement d'état non-hôte

  // ---- Undo/Redo (pile d'instance, non sauvegardée), Randomizer, presets
  bool undo();
  bool redo();
  bool canUndo() const;
  bool canRedo() const;
  std::string undoLabel() const;
  std::string redoLabel() const;
  bool randomize(const RandomOptions& o);
  bool mutate(const RandomOptions& o);
  void evolveStart(const RandomOptions& o, EvolveMode m, float rateHz);
  void evolveStop();
  void evolveReset();
  bool evolveCommit();
  bool evolveRunning() const;
  std::vector<PresetInfo> presets() const;
  bool loadPreset(const PresetInfo& info);
  bool savePreset(const std::string& name, const std::vector<std::string>& tags);
  bool setPresetFavorite(const PresetInfo& info, bool fav);
  std::string presetName() const;
  bool dirty() const noexcept { return dirty_.load(); }
  std::string presetDirectory() const { return library_.directory(); }

  Telemetry& telemetry() noexcept { return engine_.telemetry(); }
  ParameterBridge& parameters() noexcept { return bridge_; }
  const Engine& engine() const noexcept { return engine_; }

 private:
  void timerCallback() override;          // libération hors RT, latence, MIDI Learn
  void publishTopologyLocked();           // compile + publication lock-free vers le moteur
  void publishModLocked();
  void publishBankLocked();
  bool adoptBank(std::shared_ptr<const WtBank> bank, const std::string& err, const char* what);
  void publishCurvesLocked();
  void reprepareLocked();                 // Quality : reconstruction hors RT (traitement suspendu)
  void updateLatency();
  void notifyState();
  void record(const PluginState& before, const std::string& cause);  // transaction Undo (hors application d'historique)

  // gestes hôte/UI : un geste continu = une transaction Undo (arbitrage 5B)
  struct GestureListener : juce::AudioProcessorParameter::Listener {
    SubdestrucktProcessor* owner = nullptr;
    void parameterValueChanged(int, float) override { owner->dirty_.store(true, std::memory_order_relaxed); }  // RT-safe
    void parameterGestureChanged(int index, bool starting) override;
  } gestures_;
  UndoStack undo_;
  bool applyingHistory_ = false;
  std::unique_ptr<PluginState> gestureBefore_;
  int gestureDepth_ = 0;
  std::string gestureCause_;
  Evolver evolver_;
  std::array<std::atomic<float>, kNumParams> evolved_{};
  std::array<std::atomic<bool>, kNumParams> evolvedOn_{};
  std::atomic<bool> evolveActive_{false};
  double lastTimer_ = 0.0;
  PresetLibrary library_;
  std::string presetName_ = "Init (0.1.0-b5)";
  std::atomic<bool> dirty_{false};

  ParameterBridge bridge_;
  Engine engine_;
  mutable std::mutex controlMutex_;       // protège l'état éditable (jamais pris par le thread audio)
  RouteTable routes_;
  ModMatrixState mod_;
  std::string bank_ = "factory:saw";
  std::array<std::string, 4> macroNames_{"Macro 1", "Macro 2", "Macro 3", "Macro 4"};
  std::array<CurvePoints, 4> lfoCurves_{};
  std::uint64_t seed_ = 0x5B0D5EEDULL;
  std::array<std::uint64_t, 3> streamCounters_{};
  InstanceState instance_;
  std::vector<CaptureAsset> captures_;
  bool capturesHeld_ = false;  // captures restaurées non injectées (autonomie suspendue)
  std::string lastMessage_;
  std::uint64_t serial_ = 0, modSerial_ = 0;
  double sampleRate_ = 0.0;
  int blockSize_ = 512;
  std::atomic<bool> panicRequested_{false};
  std::array<MidiEvent, 1024> midi_{};
  std::function<void()> stateListener_;
  // MIDI Learn (audio -> message : file sans verrou de taille fixe)
  MidiLearnMap learn_;
  std::array<std::atomic<std::int16_t>, 128> learnRt_{};
  std::atomic<int> learnTarget_{-1};
  std::atomic<int> learnCaptured_{-1};
  int learnPoll_ = 0;
  void updateLearnLocked(const std::function<void(MidiLearnMap&)>& change);  // lecture-modification-écriture du profil
  void adoptLearnLocked(const MidiLearnMap& m);
  struct CcEvent {
    std::int16_t param;
    float value;
  };
  std::array<CcEvent, 256> ccFifo_{};
  std::atomic<std::uint32_t> ccWrite_{0}, ccRead_{0};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SubdestrucktProcessor)
};

}  // namespace sbd
