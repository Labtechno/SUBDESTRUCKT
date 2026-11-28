// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — SYNAPTIC TRACK (sélection discrète) : sélecteur segmenté, liste, onglets.
// Les indices de choix sont ceux du registre (inchangés) ; seul l'affichage compact change (nomenclature R2).
#pragma once
#include "r2/R2Style.h"

namespace sbd::r2 {

class SynapticTrack : public juce::Component, public juce::SettableTooltipClient {
 public:
  enum class Mode { Segments, List, Tabs };
  SynapticTrack(Mode mode, Domain domain, const juce::String& label, const juce::String& fullName, juce::StringArray options);

  void setIndex(int k);
  int index() const { return index_; }
  void setForced(Forced f, int forcedHover = -1) {
    forced_ = f;
    forcedHover_ = forcedHover;
    repaint();
  }
  std::function<void(int)> onChange;

  void paint(juce::Graphics&) override;
  void mouseMove(const juce::MouseEvent&) override;
  void mouseExit(const juce::MouseEvent&) override;
  void mouseDown(const juce::MouseEvent&) override;
  bool keyPressed(const juce::KeyPress&) override;
  void focusGained(FocusChangeType) override { repaint(); }
  void focusLost(FocusChangeType) override { repaint(); }

 private:
  juce::Rectangle<float> trackArea() const;
  juce::Rectangle<float> segment(int k) const;
  int hitSegment(juce::Point<float> p) const;
  Mode mode_;
  Domain domain_;
  juce::String label_;
  juce::StringArray options_;
  int index_ = 0, hover_ = -1, forcedHover_ = -1;
  Forced forced_ = Forced::None;
};

}  // namespace sbd::r2
