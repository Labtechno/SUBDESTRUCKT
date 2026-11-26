// SUBDESTRUCKT — UI de diagnostic ARCH-G3 (fonctionnelle, sans graphisme définitif).
// Observe et édite l'état via le processeur ; ne touche jamais un objet DSP actif (V1-SOFTWARE-ARCHITECTURE §4).
// Affiche : configuration active (Fs, K, Fs moteur, D, lecteur, latence), paramètres Base (curseurs hôte),
// topologie FM/CROSS (routes, classes I/D, identifiants, profondeur Effective, gain de transition), télémétrie et sûreté.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <memory>

#include "plugin/PluginProcessor.h"

namespace sbd {

class DiagnosticEditor : public juce::AudioProcessorEditor, private juce::Timer {
 public:
  explicit DiagnosticEditor(SubdestrucktProcessor& processor);
  ~DiagnosticEditor() override;

  void paint(juce::Graphics& g) override;
  void resized() override;
  void refreshNow() { timerCallback(); }  // rafraîchissement immédiat (capture d'écran de test)

 private:
  void timerCallback() override;
  void refreshTopology();
  void addRoute(int slot);
  void removeRoute(int slot);

  SubdestrucktProcessor& proc_;

  struct ParamRow {
    juce::Label label;
    juce::Slider slider;
    std::unique_ptr<juce::SliderParameterAttachment> attachment;
  };
  std::array<ParamRow, kNumV1Params> params_;  // paramètres v1 (diagnostic)

  struct RouteRow {
    juce::Label name, status, effective;
    juce::TextButton add{"+"}, remove{"-"};
  };
  std::array<RouteRow, kNumV1Routes> routes_;  // routes v1 (diagnostic)

  juce::Label title_, runtime_, telemetry_, message_;
  juce::TextButton panic_{"Panic"}, init_{"Preset init"};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DiagnosticEditor)
};

}  // namespace sbd
