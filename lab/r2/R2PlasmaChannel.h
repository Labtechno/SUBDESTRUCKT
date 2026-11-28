// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — PLASMA CHANNEL Hybrid (DA-A06) : curseur mécanique, canal lumineux de Base,
// marqueur Effective indépendant, graduations et unité ; vertical et horizontal ; unipolaire et bipolaire.
// Une mesure audio n'est JAMAIS affichée dans ce composant (afficheur PEAK distinct, CyberDisplay::Meter).
#pragma once
#include "r2/R2Style.h"

namespace sbd::r2 {

class PlasmaChannel : public juce::Component, public juce::SettableTooltipClient {
 public:
  enum class Orientation { Vertical, Horizontal };
  PlasmaChannel(Orientation o, Domain d, const juce::String& label, const juce::String& fullName, ValueSource& src,
                juce::StringArray graduations);
  ~PlasmaChannel() override;

  void setForced(Forced f) {
    forced_ = f;
    repaint();
  }
  bool gestureOpen() const { return guard_.isOpen(); }

  void paint(juce::Graphics&) override;
  void mouseEnter(const juce::MouseEvent&) override { repaint(); }
  void mouseExit(const juce::MouseEvent&) override { repaint(); }
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void mouseDoubleClick(const juce::MouseEvent&) override;
  bool keyPressed(const juce::KeyPress&) override;
  void focusGained(FocusChangeType) override { repaint(); }
  void focusLost(FocusChangeType) override { repaint(); }
  void visibilityChanged() override;

 private:
  juce::Rectangle<float> slot() const;
  juce::Point<float> posOf(float norm) const;
  Orientation o_;
  Domain d_;
  juce::String label_;
  ValueSource& src_;
  juce::StringArray grads_;  // libellés de graduation, du minimum au maximum (répartis régulièrement)
  GestureGuard guard_;
  Forced forced_ = Forced::None;
  float startPos_ = 0.0f, startNorm_ = 0.0f;
  bool fine_ = false;
};

}  // namespace sbd::r2
