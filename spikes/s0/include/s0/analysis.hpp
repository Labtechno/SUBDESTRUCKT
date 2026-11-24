// SUBDESTRUCKT — Spike S0 — outils d'analyse hors ligne (non RT).
#pragma once
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstring>
#include <vector>

namespace s0 {

inline std::uint64_t fnv1a(const double* x, size_t n) {
  std::uint64_t h = 1469598103934665603ull;
  for (size_t i = 0; i < n; ++i) {
    std::uint64_t b;
    std::memcpy(&b, &x[i], 8);
    for (int k = 0; k < 8; ++k) { h ^= (b >> (8 * k)) & 0xffull; h *= 1099511628211ull; }
  }
  return h;
}

struct Stats { double peak = 0, rms = 0, dc = 0; long nan = 0, inf = 0; };

inline Stats stats(const double* x, size_t n) {
  Stats s;
  double sum = 0, sq = 0;
  size_t m = 0;
  for (size_t i = 0; i < n; ++i) {
    if (std::isnan(x[i])) { ++s.nan; continue; }
    if (std::isinf(x[i])) { ++s.inf; continue; }
    s.peak = std::max(s.peak, std::fabs(x[i]));
    sum += x[i]; sq += x[i] * x[i]; ++m;
  }
  if (m) { s.dc = sum / m; s.rms = std::sqrt(sq / m); }
  return s;
}

inline void fft(std::vector<std::complex<double>>& a) {
  const size_t n = a.size();
  for (size_t i = 1, j = 0; i < n; ++i) {
    size_t bit = n >> 1;
    for (; j & bit; bit >>= 1) j ^= bit;
    j ^= bit;
    if (i < j) std::swap(a[i], a[j]);
  }
  for (size_t len = 2; len <= n; len <<= 1) {
    const double ang = -2.0 * 3.14159265358979323846 / double(len);
    const std::complex<double> wl(std::cos(ang), std::sin(ang));
    for (size_t i = 0; i < n; i += len) {
      std::complex<double> w(1.0, 0.0);
      for (size_t k = 0; k < len / 2; ++k) {
        auto u = a[i + k], v = a[i + k + len / 2] * w;
        a[i + k] = u + v; a[i + k + len / 2] = u - v;
        w *= wl;
      }
    }
  }
}

// Bandes 1/3 d'octave, 25 bandes. La bande b couvre [fc, fc*2^(1/3)[ (grille décalée d'1/6 d'octave par
// rapport aux bandes ISO centrées) : les raies à 220*n Hz tombent sinon pile sur les bords de bande,
// ce qui rend la métrique bruitée (constaté sur la 1re campagne).
inline const std::vector<double>& thirdOctaveCenters() {
  static const std::vector<double> c = {63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000,
                                        1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000,
                                        12500, 16000};
  return c;
}

// Welch (Hann, N=8192, recouvrement 50 %) -> niveaux de bandes 1/3 oct en dB relatifs à la
// puissance totale (0..fs/2). Insensible au niveau absolu, comparable entre fréquences d'échantillonnage.
inline std::vector<double> bandLevelsDb(const double* x, size_t n, double fs) {
  const size_t N = 8192;
  std::vector<double> psd(N / 2 + 1, 0.0), win(N);
  for (size_t i = 0; i < N; ++i) win[i] = 0.5 - 0.5 * std::cos(2.0 * 3.14159265358979323846 * i / N);
  size_t segs = 0;
  std::vector<std::complex<double>> buf(N);
  for (size_t s = 0; s + N <= n; s += N / 2) {
    for (size_t i = 0; i < N; ++i) buf[i] = x[s + i] * win[i];
    fft(buf);
    for (size_t k = 0; k <= N / 2; ++k) psd[k] += std::norm(buf[k]);
    ++segs;
  }
  std::vector<double> out(thirdOctaveCenters().size(), -80.0);
  if (!segs) return out;
  const double binHz = fs / N;
  // normalisation sur 0..20.16 kHz (bande commune à tous les fs) : sinon l'énergie >24 kHz
  // des rendus 88.2/96 kHz fausse la comparaison inter-fs.
  double total = 0;
  for (size_t k = 0; k <= N / 2; ++k) if (k * binHz < 20160.0) total += psd[k];
  total = std::max(total, 1e-300);
  const double r = std::pow(2.0, 1.0 / 3.0);
  const auto& cs = thirdOctaveCenters();
  for (size_t b = 0; b < cs.size(); ++b) {
    double lo = cs[b], hi = cs[b] * r, p = 0;
    for (size_t k = 0; k <= N / 2; ++k) {
      double f = k * binHz;
      if (f >= lo && f < hi) p += psd[k];
    }
    out[b] = std::max(-80.0, 10.0 * std::log10(p / total + 1e-12));  // plancher -80 dB : sous ce niveau = fuite de fenêtre
  }
  return out;
}

// Décimation K -> 1 (FIR sinc fenêtré Blackman, fc = 0.45 fsHost). K == 1 : copie.
inline std::vector<double> decimate(const std::vector<double>& x, int K) {
  if (K == 1) return x;
  const int taps = 64 * K + 1, mid = taps / 2;
  std::vector<double> h(taps);
  const double fc = 0.45 / K;
  double sum = 0;
  for (int i = 0; i < taps; ++i) {
    double t = i - mid;
    double s = (t == 0) ? 2.0 * fc : std::sin(2.0 * 3.14159265358979323846 * fc * t) / (3.14159265358979323846 * t);
    double w = 0.42 - 0.5 * std::cos(2.0 * 3.14159265358979323846 * i / (taps - 1)) +
               0.08 * std::cos(4.0 * 3.14159265358979323846 * i / (taps - 1));
    h[i] = s * w; sum += h[i];
  }
  for (double& v : h) v /= sum;
  const size_t outN = x.size() / K;
  std::vector<double> y(outN, 0.0);
  for (size_t m = 0; m < outN; ++m) {
    const long c = long(m) * K;  // centre (compense le retard de groupe)
    double acc = 0;
    for (int j = 0; j < taps; ++j) {
      long idx = c + (j - mid);
      if (idx >= 0 && idx < long(x.size())) acc += h[j] * x[idx];
    }
    y[m] = acc;
  }
  return y;
}

inline double nmseDb(const double* a, const double* b, size_t n) {
  double num = 0, den = 0;
  for (size_t i = 0; i < n; ++i) { double d = a[i] - b[i]; num += d * d; den += b[i] * b[i]; }
  return 10.0 * std::log10((num + 1e-300) / (den + 1e-300));
}

// Moyenne |ΔdB| sur les bandes (63 Hz .. 12.5 kHz = 23 premières bandes).
inline double lsdDb(const std::vector<double>& a, const std::vector<double>& b, size_t bands = 23) {
  double s = 0;
  for (size_t i = 0; i < bands && i < a.size(); ++i) s += std::fabs(a[i] - b[i]);
  return s / bands;
}

// Retard (s) de y par rapport à x à la fréquence f : y(t) = d * x(t - tau).
inline double measureDelaySeconds(const double* x, const double* y, size_t n, double fsE, double f) {
  std::complex<double> X(0, 0), Y(0, 0);
  const double w = 2.0 * 3.14159265358979323846 * f / fsE;
  for (size_t i = 0; i < n; ++i) {
    double h = 0.5 - 0.5 * std::cos(2.0 * 3.14159265358979323846 * i / (n - 1));
    std::complex<double> e(std::cos(w * i), -std::sin(w * i));
    X += h * x[i] * e;
    Y += h * y[i] * e;
  }
  double ph = std::arg(Y / X);  // = -w * tau
  return -ph / (2.0 * 3.14159265358979323846 * f);
}

}  // namespace s0
