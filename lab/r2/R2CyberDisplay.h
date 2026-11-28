// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Cybernetic Display (DA-A05, option D, et non Optical Glass Hybrid).
// Graphismes cybernétiques, graduations techniques, marqueurs lumineux maîtrisés, encadrements optiques, hiérarchie
// stricte données principales / secondaires. Aucune mesure inventée : chaque variante nomme sa source ; les données de
// démonstration portent l'étiquette DEMO DATA ; le spectre est INDICATIVE (décimation sans filtre anti-repliement).
#pragma once
#include <array>

#include "r2/R2Style.h"

namespace sbd::r2 {

struct DisplayData {
  bool demo = true;
  bool measure = false;  // la valeur numérique est une MESURE (ex. crête) et non un paramètre BASE / EFF
  juce::String title, source;  // titre (principal) et source de la donnée (secondaire, obligatoire)
  // valeur numérique (R1 : échelle instrumentale avec repères BASE et EFF)
  juce::String value, unit, secondary, effText, minText, maxText;
  float baseNorm = -1.0f, effNorm = -1.0f;
  // oscilloscope (-1..1) et spectre (dB, bins linéaires de 0 à fs/2 du signal décimé)
  std::vector<float> samples, spectrumDb;
  float decimatedRate = 22050.0f;  // fs hôte / 2 (une valeur sur deux)
  // wavetable
  std::vector<std::vector<float>> frames;
  float posBase = 0.0f;
  std::optional<float> posEffective;
  // courbe de modulation (points normalisés)
  std::vector<juce::Point<float>> curve;
  float curveInput = -1.0f;  // < 0 : pas de marqueur d'entrée
  // enveloppe : DLY, ATK, HOLD, DEC, SUS (niveau 0..1), REL (durées relatives)
  std::array<float, 6> env{0.05f, 0.2f, 0.1f, 0.3f, 0.6f, 0.4f};
  std::optional<float> envLevel;  // niveau instantané : publié seulement pour AMP
  // crête (dBFS) et maintien
  float peakL = -120.0f, peakR = -120.0f, holdL = -120.0f, holdR = -120.0f;
  // lignes d'état et de diagnostic : niveau 0 = principal, 1 = secondaire ; témoin 0/1/2 (éteint, anneau, plein)
  struct Row {
    juce::String key, value;
    int level = 0, led = -1;
    juce::Colour colour;
  };
  std::vector<Row> rows;
};

class CyberDisplay : public juce::Component {
 public:
  enum class Variant { Numeric, Scope, Spectrum, Wavetable, ModCurve, Envelope, Meter, Autonomy, Diagnostic };
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
  void r1Numeric(juce::Graphics&, juce::Rectangle<float>);
  void r1Scope(juce::Graphics&, juce::Rectangle<float>);
  void r1Spectrum(juce::Graphics&, juce::Rectangle<float>);
  void r1Wavetable(juce::Graphics&, juce::Rectangle<float>);
  void r1Curve(juce::Graphics&, juce::Rectangle<float>);
  void r1Envelope(juce::Graphics&, juce::Rectangle<float>);
  void r1Meter(juce::Graphics&, juce::Rectangle<float>);
  void r1Rows(juce::Graphics&, juce::Rectangle<float>);
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
