#include "Wavetable.h"

#include <algorithm>
#include <cmath>

#include "DspConstants.h"

namespace sbd {

SawWavetable::SawWavetable() {
  table_.assign(static_cast<size_t>(kLevels) * (kLength + 1), 0.0);
  for (int L = 0; L < kLevels; ++L) {
    const int H = 1 << (kLevels - 1 - L);
    double* t = &table_[static_cast<size_t>(L) * (kLength + 1)];
    for (int h = 1; h <= H; ++h) {
      // dent de scie : amplitude 1/h, signes alternés (phase pi sur les harmoniques paires)
      const double a = 1.0 / h;
      const double ph = (h % 2) ? 0.0 : kPi;
      for (int i = 0; i < kLength; ++i) t[i] += a * std::sin(kTwoPi * h * i / kLength + ph);
    }
  }
  // normalisation commune à tous les niveaux : crête du niveau 0 = 1 (aucun saut de niveau entre mips)
  double peak = 0.0;
  for (int i = 0; i < kLength; ++i) peak = std::max(peak, std::fabs(table_[static_cast<size_t>(i)]));
  for (int L = 0; L < kLevels; ++L) {
    double* t = &table_[static_cast<size_t>(L) * (kLength + 1)];
    for (int i = 0; i < kLength; ++i) t[i] /= peak;
    t[kLength] = t[0];  // échantillon de garde pour l'interpolation
  }
}

double SawWavetable::lerp(int L, double phase) const noexcept {
  const double* t = level(L);
  const double x = phase * kLength;
  int i = static_cast<int>(x);
  if (i >= kLength) i = kLength - 1;
  if (i < 0) i = 0;
  const double f = x - i;
  return t[i] + f * (t[i + 1] - t[i]);
}

double SawWavetable::read(double phase, double absIncrement) const noexcept {
  // niveau continu : harmonique max admissible = 0.5/inc ; le niveau L admet 2^(10-L) => L >= 10 - log2(0.5/inc)
  const double lf = (absIncrement > 1e-12) ? 10.0 - std::log2(0.5 / absIncrement) : -1.0;
  int i0;
  double w;
  if (lf <= -1.0) {
    i0 = 0;
    w = 0.0;
  } else {
    i0 = static_cast<int>(std::ceil(lf));
    w = lf - static_cast<double>(i0 - 1);  // (0,1]
    if (i0 < 0) {
      i0 = 0;
      w = 0.0;
    }
  }
  if (i0 >= kLevels - 1) return lerp(kLevels - 1, phase);
  const double a = lerp(i0, phase);
  const double b = lerp(i0 + 1, phase);
  return a + w * (b - a);
}

}  // namespace sbd
