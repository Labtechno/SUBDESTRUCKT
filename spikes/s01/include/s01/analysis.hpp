// SUBDESTRUCKT — Spike S0.1 — analyses hors ligne (non RT). Réutilise s0/analysis.hpp (FFT, Welch par
// bandes de tiers d'octave, NMSE, LSD, mesure de retard) et ajoute les métriques S0.1.
#pragma once
#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>
#include "s0/analysis.hpp"

namespace s01 {
using s0::bandLevelsDb;
using s0::fnv1a;
using s0::lsdDb;
using s0::measureDelaySeconds;
using s0::nmseDb;
using s0::stats;
using s0::Stats;

inline size_t pow2Above(double v) { size_t n = 1; while (double(n) < v) n <<= 1; return n; }

// PSD de Welch (Blackman-Harris 4 termes, recouvrement 50 %), résolution ~5.9 Hz quel que soit fs.
inline std::vector<double> welchPsd(const double* x, size_t n, double fs, size_t& N) {
  N = pow2Above(8192.0 * fs / 48000.0 - 1);
  std::vector<double> psd(N / 2 + 1, 0.0), win(N);
  for (size_t i = 0; i < N; ++i) {
    const double t = 2.0 * 3.14159265358979323846 * double(i) / double(N);
    win[i] = 0.35875 - 0.48829 * std::cos(t) + 0.14128 * std::cos(2 * t) - 0.01168 * std::cos(3 * t);
  }
  std::vector<std::complex<double>> buf(N);
  size_t segs = 0;
  for (size_t s = 0; s + N <= n; s += N / 2) {
    for (size_t i = 0; i < N; ++i) buf[i] = x[s + i] * win[i];
    s0::fft(buf);
    for (size_t k = 0; k <= N / 2; ++k) psd[k] += std::norm(buf[k]);
    ++segs;
  }
  if (segs) for (double& v : psd) v /= double(segs);
  return psd;
}

// Énergie hors des raies d'une grille harmonique (dB relatif au total 20 Hz..0.45 fs). Signal périodique de
// période 1/grid -> très bas ; repliement (raies non harmoniques) ou chaos (spectre continu) -> élevé.
inline double inharmonicDb(const double* x, size_t n, double fs, double gridHz) {
  size_t N;
  auto p = welchPsd(x, n, fs, N);
  const double bin = fs / double(N);
  double tot = 0, inh = 0;
  for (size_t k = 0; k <= N / 2; ++k) {
    const double f = k * bin;
    if (f < 20.0 || f > 0.45 * fs) continue;
    tot += p[k];
    const double h = std::round(f / gridHz) * gridHz;
    if (std::fabs(f - h) > 5.0 * bin) inh += p[k];
  }
  return 10.0 * std::log10((inh + 1e-300) / (tot + 1e-300));
}

// Énergie du signal moteur au-dessus de la Nyquist hôte, avant décimation (dB relatif au total).
inline double outOfBandDb(const std::vector<double>& eng, size_t skip, double fsE, double fsH) {
  if (fsE <= fsH * 1.0001) return std::nan("");
  size_t N;
  auto p = welchPsd(eng.data() + skip, eng.size() - skip, fsE, N);
  const double bin = fsE / double(N);
  double tot = 0, oob = 0;
  for (size_t k = 1; k <= N / 2; ++k) { tot += p[k]; if (k * bin > 0.5 * fsH) oob += p[k]; }
  return 10.0 * std::log10((oob + 1e-300) / (tot + 1e-300));
}

// Niveaux de bandes de tiers d'octave (grille S0 décalée d'1/6 d'octave), en dB relatifs à la puissance
// 0..20.16 kHz, calculés sur la PSD Welch Blackman-Harris à résolution constante (~5.9 Hz quel que soit fs).
// Plancher -70 dB (fuite BH < -92 dB) : une bande vide vaut -70 dB à tous les sample rates.
// Remplace s0::bandLevelsDb (Hann N fixe), dont la fuite dépendait de la résolution, donc de fs.
inline std::vector<double> bandLevels(const double* x, size_t n, double fs) {
  size_t N;
  auto p = welchPsd(x, n, fs, N);
  const double bin = fs / double(N);
  double total = 0;
  for (size_t k = 0; k <= N / 2; ++k) if (k * bin < 20160.0) total += p[k];
  total = std::max(total, 1e-300);
  const auto& cs = s0::thirdOctaveCenters();
  const double r = std::pow(2.0, 1.0 / 3.0);
  std::vector<double> out(cs.size(), -70.0);
  for (size_t b = 0; b < cs.size(); ++b) {
    double acc = 0;
    for (size_t k = 0; k <= N / 2; ++k) { const double f = k * bin; if (f >= cs[b] && f < cs[b] * r) acc += p[k]; }
    out[b] = std::max(-70.0, 10.0 * std::log10(acc / total + 1e-30));
  }
  return out;
}

// Centroïde spectral (Hz) sur 20 Hz .. 20 kHz.
inline double centroidHz(const double* x, size_t n, double fs) {
  size_t N;
  auto p = welchPsd(x, n, fs, N);
  const double bin = fs / double(N);
  double a = 0, b = 0;
  for (size_t k = 0; k <= N / 2; ++k) {
    const double f = k * bin;
    if (f < 20 || f > 20000) continue;
    a += f * p[k]; b += p[k];
  }
  return b > 0 ? a / b : 0.0;
}

// Indicateurs de click autour d'une transition [t0, t1] (échantillons hôte), fenêtre d'analyse
// [t0-32, t1+64] (couvre une transition sèche R=0) :
//  jump  = max |Δy| dans la transition / max |Δy| des régimes stationnaires avant ET après (fenêtres de
//          refLen éch.). <= 1 : aucun saut plus grand que ceux des régimes eux-mêmes (indicateur faible :
//          un signal riche masque un petit saut).
//  burst = max de l'énergie de Δ⁴y (accentuation +24 dB/oct : un click est large bande, donc dominé par
//          le haut du spectre) sur fenêtres glissantes de 32 éch. pendant la transition, en dB relatif au
//          max de la même mesure dans les régimes stationnaires. <= 0 dB : pas de bouffée HF au-delà de ce
//          que contient déjà le signal (click masqué ou absent).
//  burstAtDb = même mesure restreinte à [ti-32, ti+64] autour des seuls instants structurels (transaction de
//          topologie, début et fin de rampe) : isole les discontinuités du trajet de la rampe, qui peut
//          traverser des régimes intermédiaires plus riches que les deux extrémités (ce n'est pas un click).
struct ClickMetrics { double jump = 0, burstDb = 0, burstAtDb = 0; };
inline ClickMetrics clickMetrics(const std::vector<double>& y, long long t0, long long t1, long long refLen,
                                 const std::vector<long long>& instants = {}) {
  auto maxJump = [&](long long a, long long b) {
    double m = 0;
    for (long long i = std::max(1LL, a); i < b; ++i) m = std::max(m, std::fabs(y[size_t(i)] - y[size_t(i - 1)]));
    return m;
  };
  auto maxBurst = [&](long long a, long long b) {
    const int W = 32;
    double best = 0, acc = 0;
    std::vector<double> e;
    for (long long i = std::max(4LL, a); i < b; ++i) {
      const size_t k = size_t(i);
      const double d4 = y[k] - 4 * y[k - 1] + 6 * y[k - 2] - 4 * y[k - 3] + y[k - 4];
      e.push_back(d4 * d4);
      acc += d4 * d4;
      if (e.size() > size_t(W)) acc -= e[e.size() - 1 - W];
      if (e.size() >= size_t(W)) best = std::max(best, acc);
    }
    return best;
  };
  const long long w0 = t0 - 32, w1 = t1 + 64;
  const long long pre0 = w0 - 64 - refLen, pre1 = w0 - 64;
  const long long post0 = w1 + 64, post1 = w1 + 64 + refLen;
  ClickMetrics c;
  const double jr = std::max(maxJump(pre0, pre1), maxJump(post0, post1));
  c.jump = maxJump(w0, w1) / std::max(jr, 1e-15);
  const double br = std::max(maxBurst(pre0, pre1), maxBurst(post0, post1));
  c.burstDb = 10.0 * std::log10((maxBurst(w0, w1) + 1e-30) / (br + 1e-30));
  double bi = 0;
  for (long long ti : instants) bi = std::max(bi, maxBurst(ti - 32, ti + 64));
  c.burstAtDb = instants.empty() ? c.burstDb : 10.0 * std::log10((bi + 1e-30) / (br + 1e-30));
  return c;
}

}  // namespace s01
