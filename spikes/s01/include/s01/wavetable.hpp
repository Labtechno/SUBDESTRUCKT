// SUBDESTRUCKT — Spike S0.1 — moteur de table d'onde minimal, réel et déterministe.
// 2048 échantillons/cycle (candidat baseline §5), float64 pour le harness. Tables préconstruites hors RT
// par synthèse additive, une table par octave (mip-map) : le niveau L contient les harmoniques <= 2^(10-L).
// Lecture : interpolation linéaire en phase + fondu entre deux niveaux, choisis à partir de la fréquence
// instantanée de lecture |Δphase| (couvre PM, FM linéaire, TZFM et FM exponentielle avec la même règle).
// Pas de Z, pas de morph XY, pas d'éditeur, pas de Character.
#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace s01 {

constexpr double kPi = 3.14159265358979323846;

enum class Shape { Sine = 0, Triangle = 1, Saw = 2, Asym = 3 };
inline const char* name(Shape s) {
  switch (s) { case Shape::Sine: return "sine"; case Shape::Triangle: return "tri"; case Shape::Saw: return "saw"; default: return "asym"; }
}

class WaveTable {
 public:
  static constexpr int kLen = 2048;
  static constexpr int kLevels = 11;  // harmoniques max 1024, 512, ..., 1

  explicit WaveTable(Shape s) : shape_(s) {
    tab_.assign(size_t(kLevels) * (kLen + 1), 0.0);
    for (int L = 0; L < kLevels; ++L) {
      const int H = 1 << (kLevels - 1 - L);
      double* t = &tab_[size_t(L) * (kLen + 1)];
      for (int h = 1; h <= H; ++h) {
        double a = 0, ph = 0;
        coeff(h, a, ph);
        if (a == 0) continue;
        for (int i = 0; i < kLen; ++i) t[i] += a * std::sin(2.0 * kPi * h * i / kLen + ph);
      }
    }
    // normalisation commune à tous les niveaux : crête du niveau 0 = 1 (pas de saut de niveau entre mips)
    double pk = 0;
    for (int i = 0; i < kLen; ++i) pk = std::max(pk, std::fabs(tab_[i]));
    for (int L = 0; L < kLevels; ++L) {
      double* t = &tab_[size_t(L) * (kLen + 1)];
      for (int i = 0; i < kLen; ++i) t[i] /= pk;
      t[kLen] = t[0];  // garde pour l'interpolation
    }
  }

  Shape shape() const { return shape_; }

  // phase dans [0,1) ; finc = |fréquence instantanée| / fs moteur (cycles par échantillon).
  double read(double phase, double finc) const {
    // niveau continu : harmonique max admissible = 0.5/finc ; niveau L admet 2^(10-L) => L >= 10 - log2(0.5/finc)
    double lf = (finc > 1e-12) ? 10.0 - std::log2(0.5 / finc) : -1.0;
    // fondu entre i0 et i0+1, i0 = ceil(lf) : les deux niveaux sont sans repliement ; continu aux frontières
    int i0; double w;
    if (lf <= -1.0) { i0 = 0; w = 0.0; }
    else {
      i0 = int(std::ceil(lf));
      w = lf - double(i0 - 1);  // (0,1]
      if (i0 < 0) { i0 = 0; w = 0.0; }
    }
    if (i0 >= kLevels - 1) return lerp(kLevels - 1, phase);
    const double a = lerp(i0, phase), b = lerp(i0 + 1, phase);
    return a + w * (b - a);
  }

  const double* level(int L) const { return &tab_[size_t(L) * (kLen + 1)]; }

 private:
  double lerp(int L, double phase) const {
    const double* t = &tab_[size_t(L) * (kLen + 1)];
    const double x = phase * kLen;
    int i = int(x);
    if (i >= kLen) i = kLen - 1;
    const double f = x - i;
    return t[i] + f * (t[i + 1] - t[i]);
  }
  void coeff(int h, double& a, double& ph) const {
    switch (shape_) {
      case Shape::Sine: a = (h == 1) ? 1.0 : 0.0; ph = 0; break;
      case Shape::Triangle:  // impaires, 1/h^2, signes alternés
        if (h % 2 == 0) { a = 0; ph = 0; }
        else { a = 1.0 / (double(h) * h); ph = ((h / 2) % 2) ? kPi : 0.0; }
        break;
      case Shape::Saw: a = 1.0 / h; ph = (h % 2) ? 0.0 : kPi; break;
      case Shape::Asym: {
        // forme riche asymétrique : pente 1/h^0.8, harmoniques paires renforcées, phases déterministes
        // (suite de Weyl, sans RNG) -> crête et DC non symétriques, spectre dense.
        a = std::pow(double(h), -0.8) * ((h % 2 == 0) ? 1.0 : 0.6);
        ph = 2.0 * kPi * std::fmod(0.6180339887498949 * h * h, 1.0);
        break;
      }
    }
  }

  Shape shape_;
  std::vector<double> tab_;
};

}  // namespace s01
