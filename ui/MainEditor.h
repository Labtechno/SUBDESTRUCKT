// SUBDESTRUCKT — UI — éditeur principal ARCH-G4 (V1-UI-UX-DESIGN) : trois espaces sur un état unique
// (Performance / Sound Design / Engineering), CROSS au centre de Sound Design, Destination Inspector intégré
// rétractable et épinglable, barre persistante (preset, état modifié, Undo/Redo, Quality, Drone, Panic, mètre),
// moniteurs réels, redimensionnement et zoom global. Changer de vue, de zoom ou de moniteur ne modifie ni le DSP ni
// les valeurs Base. L'UI n'est jamais source de vérité DSP : elle édite via l'API de contrôle du processeur.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>
#include <set>

#include "Monitors.h"
#include "Widgets.h"
#include "plugin/PluginProcessor.h"

namespace sbd::ui {

class Inspector;
class PerformanceView;
class SoundDesignView;
class EngineeringView;

struct UiContext {
  SubdestrucktProcessor& proc;
  SelectFn select;
  RandomOptions random;  // réglages Randomizer de la session (verrous compris)
};

class MainEditor : public juce::AudioProcessorEditor, private juce::Timer {
 public:
  explicit MainEditor(SubdestrucktProcessor& p);
  ~MainEditor() override;
  void paint(juce::Graphics&) override;
  void resized() override;
  void showSpace(int k);            // 0 Performance, 1 Sound Design, 2 Engineering
  void showSubTab(int space, int tab);
  void refreshNow() { timerCallback(); }
  void selectParam(ParamId id);

 private:
  void timerCallback() override;
  void refreshBar();
  void layoutContent();
  SubdestrucktProcessor& proc_;
  Look look_;
  UiContext ctx_;
  // barre persistante
  juce::Label preset_, status_, meter_;
  juce::TextButton prev_{"<"}, next_{">"}, undo_{"Undo"}, redo_{"Redo"}, panic_{"PANIC"}, inspectorBtn_{"Inspector"};
  juce::TextButton spaceBtn_[3];
  juce::ComboBox quality_, zoom_;
  juce::ToggleButton drone_{"Drone"};
  // contenu (mis à l'échelle par le zoom)
  juce::Component content_;
  std::unique_ptr<PerformanceView> perf_;
  std::unique_ptr<SoundDesignView> sound_;
  std::unique_ptr<EngineeringView> eng_;
  std::unique_ptr<Inspector> inspector_;
  int space_ = 0;
  float zoom_f_ = 1.0f;
  bool inspectorOpen_ = true;
  int presetIndex_ = 0;
  int ticks_ = 0;
};

}  // namespace sbd::ui
