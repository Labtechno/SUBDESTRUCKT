// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab — DEMO DATA.
// Toute valeur produite ici est une donnée de démonstration : aucune n'est une mesure du moteur SUBDESTRUCKT.
#pragma once
#include "r2/R2CyberDisplay.h"

namespace sbd::lab {

// Source de valeur de démonstration, instrumentée pour les tests de discipline de geste (comptage begin/end).
class DemoValue : public r2::ValueSource {
 public:
  DemoValue(float lo, float hi, float def, float value, juce::String unit, int decimals, bool bipolar = false, int steps = 0);
  float norm() const override { return n_; }
  float defaultNorm() const override { return toNorm(def_); }
  std::optional<float> effectiveNorm() const override { return eff_; }
  juce::String text(float norm) const override;
  bool bipolar() const override { return bip_; }
  int steps() const override { return steps_; }
  void begin() override;
  void during(float norm) override;
  void end() override;
  void complete(float norm) override;
  std::vector<float> routeDepths() const override { return routes_; }

  DemoValue& value(float v) {
    n_ = toNorm(v);
    return *this;
  }
  DemoValue& normValue(float n) {
    n_ = juce::jlimit(0.0f, 1.0f, n);
    return *this;
  }
  DemoValue& effective(std::optional<float> n) {
    eff_ = n;
    return *this;
  }
  DemoValue& routes(std::vector<float> r) {
    routes_ = std::move(r);
    return *this;
  }
  float toNorm(float v) const { return juce::jlimit(0.0f, 1.0f, (v - lo_) / (hi_ - lo_)); }

  // instrumentation
  int begins = 0, ends = 0, depth = 0, maxDepth = 0, nested = 0, writesOutside = 0, writes = 0;
  bool balanced() const { return begins == ends && depth == 0; }

 private:
  void set(float n);
  float lo_, hi_, def_, n_;
  juce::String unit_;
  int dec_, steps_;
  bool bip_;
  std::optional<float> eff_;
  std::vector<float> routes_;
};

// générateurs DEMO DATA pour les afficheurs
std::vector<float> demoScope(int n, float phase);
std::vector<float> demoSpectrumDb(const std::vector<float>& signal);  // DFT côté interface (comme 0.4.1)
std::vector<std::vector<float>> demoWavetable(int frames, int points);
std::vector<juce::Point<float>> demoCurve();
r2::DisplayData demoDisplay(r2::CyberDisplay::Variant v, float phase);
juce::String platformLine();  // plateforme, JUCE, polices résolues, commit de rendu (affiché sur chaque planche)
void setRenderSha(const juce::String& sha);  // SHA du code qui produit les rendus (CI), ou description locale
juce::String bannerLine();                   // bandeau DEMO DATA de la révision courante

}  // namespace sbd::lab
