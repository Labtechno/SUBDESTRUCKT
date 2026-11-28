// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — NEXUS Hybrid (DA-A03) : XL, S, Micro ; unipolaire et bipolaire ; WT/FM/CROSS/MOD.
// Spécification : docs/ARCH-UI-04-R2/R2-DESIGN-SYSTEM-SPEC.md §4 et B1-COMPONENT-LAB/COMPONENT-DESIGN-SPEC.md §2.
// Gestes conservés de 0.4.1 (220 px pour la plage complète, Maj x0,1, molette groupée 450 ms, flèches 1 %, Page 10 %,
// Début/Fin, Suppr = défaut, Alt+clic = défaut, double-clic). Discipline de geste R2 : un begin/end par geste, sans
// imbrication au double-clic, fermé à la destruction et au masquage (GestureGuard).
#pragma once
#include "r2/R2Style.h"
#include "ui/r2/R2Painters.h"

namespace sbd::r2 {

class Nexus : public juce::Component, public juce::SettableTooltipClient, private juce::Timer {
 public:
  using Size = NexusSize;
  Nexus(Size size, Domain domain, const juce::String& label, const juce::String& fullName, ValueSource& src);
  ~Nexus() override;

  static juce::Point<int> cellSize(Size s);  // gabarits 0.4.1 : XL 132x150, S 84x96, Micro 64x74

  void setForced(Forced f) {
    forced_ = f;
    repaint();
  }
  void setInactive(bool inactive, const juce::String& reason = {});
  void setLabelFontSize(float px) {
    labelPx_ = px;
    repaint();
  }
  void setEntryShown(bool e) {
    entry_ = e;
    repaint();
  }
  bool gestureOpen() const { return guard_.isOpen(); }
  void cancelGesture() { guard_.close(); }  // perte de capture signalée par l'hôte de la vue

  void paint(juce::Graphics&) override;
  void mouseEnter(const juce::MouseEvent&) override { repaint(); }
  void mouseExit(const juce::MouseEvent&) override { repaint(); }
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void mouseDoubleClick(const juce::MouseEvent&) override;
  void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
  bool keyPressed(const juce::KeyPress&) override;
  void focusGained(FocusChangeType) override { repaint(); }
  void focusLost(FocusChangeType) override { repaint(); }
  void visibilityChanged() override;
  void parentHierarchyChanged() override;

 private:
  void timerCallback() override;  // fin de geste molette
  juce::Rectangle<float> valueArea() const;
  juce::Rectangle<float> knobArea() const;
  float angleOf(float norm) const;
  void stepNorm(float delta);

  Size size_;
  Domain domain_;
  juce::String label_, full_, inactiveWhy_;
  ValueSource& src_;
  GestureGuard guard_;
  Forced forced_ = Forced::None;
  float labelPx_ = 0.0f;  // 0 = corps de la famille
  bool inactive_ = false, entry_ = false, dragging_ = false, wheel_ = false, fineAtStart_ = false;
  float dragStartY_ = 0.0f, dragStartNorm_ = 0.0f;
};

}  // namespace sbd::r2
