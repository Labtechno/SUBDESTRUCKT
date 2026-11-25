// SUBDESTRUCKT — outils d'analyse hors ligne pour les tests (FFT, spectres, métriques). Non RT.
#pragma once
#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

namespace an {

constexpr double kPi = 3.14159265358979323846;

inline void fft(std::vector<std::complex<double>>& a) {
  const size_t n = a.size();
  for (size_t i = 1, j = 0; i < n; ++i) {
    size_t bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(a[i], a[j]);
  }
  for (size_t len = 2; len <= n; len <<= 1) {
    const double ang = -2.0 * kPi / static_cast<double>(len);
    const std::complex<double> wl(std::cos(ang), std::sin(ang));
    for (size_t i = 0; i < n; i += len) {
      std::complex<double> w(1.0, 0.0);
      for (size_t k = 0; k < len / 2; ++k) {
        const auto u = a[i + k], v = a[i + k + len / 2] * w;
        a[i + k] = u + v;
        a[i + k + len / 2] = u - v;
        w *= wl;
      }
    }
  }
}

// Spectre d'amplitude (fenêtre Blackman-Harris 4 termes) normalisé : une sinusoïde d'amplitude A donne un pic ~A.
inline std::vector<double> amplitudeSpectrum(const double* x, size_t n) {
  std::vector<std::complex<double>> b(n);
  double wsum = 0.0;
  for (size_t i = 0; i < n; ++i) {
    const double t = 2.0 * kPi * static_cast<double>(i) / static_cast<double>(n);
    const double w = 0.35875 - 0.48829 * std::cos(t) + 0.14128 * std::cos(2 * t) - 0.01168 * std::cos(3 * t);
    wsum += w;
    b[i] = x[i] * w;
  }
  fft(b);
  std::vector<double> m(n / 2 + 1);
  for (size_t k = 0; k <= n / 2; ++k) m[k] = 2.0 * std::abs(b[k]) / wsum;
  return m;
}

// Spectre de puissance (Blackman-Harris) normalisé pour que la somme sur le lobe principal d'une sinusoïde
// d'amplitude A vaille A² : l'amplitude estimée par l'énergie du lobe est insensible au « scalloping »
// (position de la raie entre deux bins), contrairement à la lecture du maximum.
inline std::vector<double> powerSpectrum(const double* x, size_t n) {
  std::vector<std::complex<double>> b(n);
  double w2 = 0.0;
  for (size_t i = 0; i < n; ++i) {
    const double t = 2.0 * kPi * static_cast<double>(i) / static_cast<double>(n);
    const double w = 0.35875 - 0.48829 * std::cos(t) + 0.14128 * std::cos(2 * t) - 0.01168 * std::cos(3 * t);
    w2 += w * w;
    b[i] = x[i] * w;
  }
  fft(b);
  std::vector<double> p(n / 2 + 1);
  for (size_t k = 0; k <= n / 2; ++k) p[k] = 4.0 * std::norm(b[k]) / (static_cast<double>(n) * w2);
  return p;
}

// Amplitude d'une raie par l'énergie de son lobe principal (±r bins).
inline double lineAmplitudeEnergy(const std::vector<double>& p, double binHz, double f, int r = 5) {
  const long c = std::lround(f / binHz);
  double e = 0.0;
  for (long k = c - r; k <= c + r; ++k)
    if (k >= 0 && k < static_cast<long>(p.size())) e += p[static_cast<size_t>(k)];
  return std::sqrt(e);
}

// Amplitude d'une raie : maximum local dans ±r bins autour de f.
inline double lineAmplitude(const std::vector<double>& m, double binHz, double f, int r = 3) {
  const long c = std::lround(f / binHz);
  double best = 0.0;
  for (long k = c - r; k <= c + r; ++k)
    if (k >= 0 && k < static_cast<long>(m.size())) best = std::max(best, m[static_cast<size_t>(k)]);
  return best;
}

// Énergie hors des raies d'une grille (multiples de gridHz, ±r bins), relative au total, en dB.
inline double offGridEnergyDb(const std::vector<double>& m, double binHz, double gridHz, double fMin, double fMax,
                              int r = 4) {
  double tot = 0.0, off = 0.0;
  for (size_t k = 0; k < m.size(); ++k) {
    const double f = static_cast<double>(k) * binHz;
    if (f < fMin || f > fMax) continue;
    const double p = m[k] * m[k];
    tot += p;
    const double h = std::round(f / gridHz) * gridHz;
    if (h < gridHz * 0.5 || std::fabs(f - h) > r * binHz) off += p;
  }
  return 10.0 * std::log10((off + 1e-300) / (tot + 1e-300));
}

// Énergie hors d'une raie unique (porteuse), relative au total, en dB.
inline double offLineEnergyDb(const std::vector<double>& m, double binHz, double f0, double fMax, int r = 4) {
  double tot = 0.0, off = 0.0;
  for (size_t k = 1; k < m.size(); ++k) {
    const double f = static_cast<double>(k) * binHz;
    if (f > fMax) break;
    const double p = m[k] * m[k];
    tot += p;
    if (std::fabs(f - f0) > r * binHz) off += p;
  }
  return 10.0 * std::log10((off + 1e-300) / (tot + 1e-300));
}

inline double nmseDb(const double* a, const double* ref, size_t n) {
  double e = 0.0, s = 0.0;
  for (size_t i = 0; i < n; ++i) {
    const double d = a[i] - ref[i];
    e += d * d;
    s += ref[i] * ref[i];
  }
  return 10.0 * std::log10((e + 1e-300) / (s + 1e-300));
}

// Écart spectral moyen (dB) sur les bins significatifs (au-dessus de -80 dB du maximum commun), entre fMin et fMax.
inline double spectralDifferenceDb(const std::vector<double>& a, const std::vector<double>& b, double binHz, double fMin,
                                   double fMax) {
  double peak = 0.0;
  for (size_t k = 0; k < a.size(); ++k) peak = std::max(peak, std::max(a[k], b[k]));
  const double floor = peak * 1e-4;
  double acc = 0.0;
  int n = 0;
  for (size_t k = 0; k < a.size(); ++k) {
    const double f = static_cast<double>(k) * binHz;
    if (f < fMin || f > fMax) continue;
    if (std::max(a[k], b[k]) < floor) continue;
    acc += std::fabs(20.0 * std::log10(std::max(a[k], floor) / std::max(b[k], floor)));
    ++n;
  }
  return n ? acc / n : 0.0;
}

// Fonction de Bessel de première espèce J_n(x) par série (libc++ n'implémente pas std::cyl_bessel_j).
inline double besselJ(int n, double x) {
  double sum = 0.0, term = 1.0;
  for (int k = 1; k <= n; ++k) term *= (x / 2.0) / k;  // (x/2)^n / n!
  for (int m = 0; m < 60; ++m) {
    sum += term;
    term *= -(x / 2.0) * (x / 2.0) / ((m + 1.0) * (m + 1.0 + n));
    if (std::fabs(term) < 1e-18) break;
  }
  return sum;
}

// Énergie de la dérivée quatrième sur fenêtres glissantes (métrique de clic du spike S0.1, rapportée sans seuil).
inline double maxD4Burst(const std::vector<double>& y, size_t a, size_t b, size_t W = 32) {
  double best = 0.0, acc = 0.0;
  std::vector<double> e;
  for (size_t i = std::max<size_t>(a, 4); i < b && i < y.size(); ++i) {
    const double d4 = y[i] - 4 * y[i - 1] + 6 * y[i - 2] - 4 * y[i - 3] + y[i - 4];
    e.push_back(d4 * d4);
    acc += d4 * d4;
    if (e.size() > W) acc -= e[e.size() - 1 - W];
    if (e.size() >= W) best = std::max(best, acc);
  }
  return best;
}

}  // namespace an
