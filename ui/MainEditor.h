// SUBDESTRUCKT — UI — éditeur principal ARCH-UI-04 « SYMBIOTIC INDUSTRIAL » : trois espaces sur un état unique
// (PERFORMANCE / SOUND DESIGN / ENGINEERING), barre globale adaptative (preset, état modifié, Ouvrir/Enregistrer,
// Undo/Redo, Quality, Drone, Autoriser, zoom, Inspector, PANIC toujours visible), Destination Inspector rétractable et
// épinglable, zoom 75–150 % avec défilement local. Changer d'espace, d'onglet, de zoom ou de taille ne modifie ni le
// DSP ni les valeurs Base. L'UI n'est jamais source de vérité DSP : elle édite via l'API de contrôle du processeur.
// Préférences de présentation (espace, zoom, Inspector) : propres à l'éditeur, jamais écrites dans l'état sonore.
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>

#include "Components.h"
#include "SymbioticLook.h"
#include "Views.h"
#include "plugin/PluginProcessor.h"

namespace sbd::ui {

class MainEditor : public juce::AudioProcessorEditor, private juce::Timer {
 public:
  explicit MainEditor(SubdestrucktProcessor& p);
  ~MainEditor() override;
  void paint(juce::Graphics&) override;
  void resized() override;
  void showSpace(int k);              // 0 Performance, 1 Sound Design, 2 Engineering
  void showSubTab(int space, int tab);
  void refreshNow() { timerCallback(); }
  void selectParam(ParamId id);

  // ---- navigation énumérable et réglages de présentation (tests UI04-*)
  int space() const { return space_; }
  int numTabs(int space) const;
  int numSubContexts(int space, int tab) const;
  void showSubContext(int space, int tab, int sub);
  juce::String contextName(int space, int tab, int sub) const;
  void setZoomPercent(int pct);
  int zoomPercent() const { return static_cast<int>(zoomF_ * 100.0f + 0.5f); }
  void setInspectorOpen(bool open);
  bool inspectorOpen() const { return inspectorOpen_; }
  UiContext& context() { return ctx_; }
  juce::Component& panicButton() { return panic_; }
  juce::Component& barComponent() { return bar_; }
  juce::Component* currentView();
  juce::Viewport& viewport() { return vp_; }
  InspectorPanel& inspector() { return *inspector_; }
  int barHeight() const { return barH_; }

 private:
  void timerCallback() override;
  void refreshBar();
  void layoutBar();
  void layoutContent();
  SubdestrucktProcessor& proc_;
  SymbioticLook look_;
  UiContext ctx_;
  juce::TooltipWindow tooltips_{this, 700};
  // barre globale
  juce::Component bar_;
  ActionCell prev_{juce::String::fromUTF8("‹"), tok::text}, next_{juce::String::fromUTF8("›"), tok::text}, open_{"Ouvrir", tok::text},
      save_{"Enregistrer", tok::text}, undo_{"Undo", tok::text}, redo_{"Redo", tok::text}, authorize_{"Autoriser", tok::warn}, inspectorBtn_{"Inspector", tok::mod},
      panic_{"PANIC", tok::alert, true};
  juce::Label preset_, status_, meter_;
  TabStrip spaces_{tok::cross};
  juce::ComboBox quality_, zoom_;
  NeuralCell drone_;
  // contenu (mis à l'échelle par le zoom, défilement local si la taille logique est insuffisante)
  juce::Component content_;
  juce::Viewport vp_;
  std::unique_ptr<SpaceView> views_[3];
  std::unique_ptr<InspectorPanel> inspector_;
  int space_ = 0, barH_ = tok::barH;
  float zoomF_ = 1.0f;
  bool inspectorOpen_ = true;
  int presetIndex_ = 0, ticks_ = 0;
};

}  // namespace sbd::ui
