// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Cybernetic Display (DA-A05, option D, et non Optical Glass Hybrid).
// Graphismes cybernétiques, graduations techniques, marqueurs lumineux maîtrisés, encadrements optiques, hiérarchie
// stricte données principales / secondaires. Aucune mesure inventée : chaque variante nomme sa source ; les données de
// démonstration portent l'étiquette DEMO DATA ; le spectre est INDICATIVE (décimation sans filtre anti-repliement).
#pragma once
#include <array>

#include "r2/R2Style.h"
#include "ui/r2/R2Hud.h"

namespace sbd::r2 {

// DisplayData : ui/r2/R2Hud.h (partagée avec le plugin)


class CyberDisplay : public juce::Component {
 public:
  using Variant = HudVariant;
  CyberDisplay(Variant v, Domain d, DisplayData data);
  void setRevision(Rev r) {
    rev_ = r;
    repaint();
  }
  void setData(DisplayData d) {
    data_ = std::move(d);
    repaint();
  }
  const DisplayData& data() const { return data_; }
  void paint(juce::Graphics&) override;

 private:
  void paintB1(juce::Graphics&);
  void paintR1(juce::Graphics&);
  void paintNumeric(juce::Graphics&, juce::Rectangle<float>);
  void paintScope(juce::Graphics&, juce::Rectangle<float>);
  void paintSpectrum(juce::Graphics&, juce::Rectangle<float>);
  void paintWavetable(juce::Graphics&, juce::Rectangle<float>);
  void paintCurve(juce::Graphics&, juce::Rectangle<float>);
  void paintEnvelope(juce::Graphics&, juce::Rectangle<float>);
  void paintMeter(juce::Graphics&, juce::Rectangle<float>);
  void paintRows(juce::Graphics&, juce::Rectangle<float>);
  void graticule(juce::Graphics&, juce::Rectangle<float>, int nx, int ny) const;
  void tag(juce::Graphics&, juce::Rectangle<float> area, const juce::String& s, juce::Colour c, bool left) const;
  Variant v_;
  Domain d_;
  DisplayData data_;
  Rev rev_ = Rev::R1;
};

}  // namespace sbd::r2
