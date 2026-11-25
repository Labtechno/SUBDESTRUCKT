// SUBDESTRUCKT — L0 — adaptateur hôte JUCE/VST3 (V1-SOFTWARE-ARCHITECTURE §2 L0, §10).
//
// Responsabilités : paramètres hôte (StableID <-> identité hôte, jamais d'indice), MIDI -> événements du cœur,
// sortie stéréo, état versionné (codec du cœur), latence déclarée, UI de diagnostic.
// Ne contient ni règle musicale ni algorithme DSP : tout le DSP est dans le cœur (source/), testable sans hôte.
// Thread audio : processBlock() n'alloue pas, ne verrouille pas, ne fait aucune I/O (vérifié par tests/wrapper).
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "core/Parameters.h"
#include "core/StateCodec.h"
#include "core/Topology.h"
#include "engine/Engine.h"

namespace sbd {

// Pont paramètres hôte -> valeurs Base exactes. Après une restauration d'état, la valeur Base sauvegardée est
// conservée telle quelle (l'aller-retour hôte normalisé 0..1 en float peut différer d'un ulp) tant que l'hôte ne
// modifie pas le paramètre. Lecture sans verrou ni allocation (thread audio).
class ParameterBridge {
 public:
  void attach(int index, juce::AudioParameterFloat* p) noexcept { params_[static_cast<size_t>(index)] = p; }
  juce::AudioParameterFloat* param(int index) const noexcept { return params_[static_cast<size_t>(index)]; }
  float base(int index) const noexcept;
  void setExact(int index, float value);  // thread de contrôle
  ParamValues values() const noexcept;

 private:
  std::array<juce::AudioParameterFloat*, kNumParams> params_{};
  std::array<std::atomic<float>, kNumParams> exact_{};
  std::array<std::atomic<float>, kNumParams> hostValueAtRestore_{};
  std::array<std::atomic<bool>, kNumParams> hasExact_{};
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
  std::vector<Route> currentRoutes() const;
  std::uint32_t currentNextRouteId() const;
  std::string lastMessage() const;
  bool applyState(const PluginState& state);
  void loadDefaultPreset() { applyState(defaultState()); }
  void requestPanic() noexcept { panicRequested_.store(true); }
  std::string encodeCurrentState() const;

  Telemetry& telemetry() noexcept { return engine_.telemetry(); }
  ParameterBridge& parameters() noexcept { return bridge_; }

 private:
  void timerCallback() override;          // libération hors RT des topologies retirées
  void publishTopologyLocked();           // compile + publication lock-free vers le moteur

  ParameterBridge bridge_;
  Engine engine_;
  mutable std::mutex controlMutex_;       // protège l'état éditable (jamais pris par le thread audio)
  RouteTable routes_;
  std::string lastMessage_;
  std::uint64_t serial_ = 0;
  std::atomic<bool> panicRequested_{false};
  std::array<MidiEvent, 1024> midi_{};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SubdestrucktProcessor)
};

}  // namespace sbd
