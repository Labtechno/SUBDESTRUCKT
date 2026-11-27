// SUBDESTRUCKT — UI — éditeurs de courbes (ARCH-UI-04, Design System) :
//  - EnvelopeEditor : DAHDSR d'une des cinq enveloppes, poignées liées aux paramètres réels (geste hôte par glisser,
//    une transaction Undo par geste) ; durées affichées en position normalisée du paramètre (correspondance hôte).
//  - PointCurveEditor : courbe à points (2..8, x croissants) : forme Custom d'un LFO (setLfoCurve) ou courbe Custom
//    d'une route de modulation (modUpdate) ; validation au relâchement (une transaction), double-clic = ajout/retrait.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <memory>

#include "Components.h"

namespace sbd::ui {

class EnvelopeEditor : public juce::Component, public juce::SettableTooltipClient {
 public:
  explicit EnvelopeEditor(UiContext& ctx);
  void setEnvelope(int k);  // 0 AMP, 1 WT, 2 FM, 3 AUX1, 4 AUX2
  int envelope() const { return env_; }
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void refresh() { repaint(); }
  static std::vector<ParamId> paramsOf(int env);  // ordre : delay, attack, hold, decay, sustain, release

 private:
  struct Seg {
    float x0, x1;
  };
  void geometry(std::array<juce::Point<float>, 6>& pts) const;
  UiContext& ctx_;
  int env_ = 0;
  std::array<std::unique_ptr<ParamLink>, 6> links_;
  int drag_ = -1;
  juce::Point<float> start_;
  float startNx_ = 0.0f, startNy_ = 0.0f;
};

class PointCurveEditor : public juce::Component, public juce::SettableTooltipClient {
 public:
  PointCurveEditor(juce::String title, juce::Colour accent, bool bipolarY);
  void setPoints(int n, const float* x, const float* y);  // sans notification
  std::function<void(int n, const float* x, const float* y)> onCommit;
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void mouseDoubleClick(const juce::MouseEvent&) override;
  int numPoints() const { return n_; }

 private:
  juce::Rectangle<float> area() const { return getLocalBounds().toFloat().reduced(8.0f).withTrimmedTop(14.0f); }
  juce::Point<float> toScreen(float x, float y) const;
  juce::String title_;
  juce::Colour accent_;
  bool bipolar_;
  int n_ = 2, drag_ = -1;
  std::array<float, 8> x_{0.0f, 1.0f}, y_{0.0f, 1.0f};
};

}  // namespace sbd::ui
