// SUBDESTRUCKT — Spike S0.1 — décimation temps réel K -> 1 (K = 1, 2, 4) dans le chemin de rendu.
// Cascade de demi-bandes FIR à phase linéaire (fenêtre de Kaiser), état en anneau préalloué, aucune
// allocation après construction. Spécification (au rythme hôte fsH) :
//   bande passante 0 .. 0.45 fsH, bande coupée >= 0.55 fsH (repliement vers 0..0.45 fsH), ~100 dB.
//   Étage final (2fsH -> fsH) : transition 0.225..0.275 fs ; étage 4fsH -> 2fsH : 0.1125..0.3875 fs.
// Latence : arrondie à un nombre entier d'échantillons hôte par une ligne à retard pure en entrée,
// pour que les rendus K=1/2/4 restent alignables et que la latence VST3 déclarée soit entière.
#pragma once
#include <array>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace s01 {

inline constexpr double kPiR() { return 3.14159265358979323846; }

inline double besselI0(double x) {
  double s = 1.0, t = 1.0;
  for (int k = 1; k < 50; ++k) { t *= (x / (2.0 * k)) * (x / (2.0 * k)); s += t; if (t < 1e-20 * s) break; }
  return s;
}

// Demi-bande : h[c] = 0.5, h[c±2m] = 0 (m != 0), h[c±(2m+1)] = sinc fenêtré. Longueur N = 4M+3.
inline std::vector<double> designHalfband(double transition, double attenDb) {
  const double beta = attenDb > 50 ? 0.1102 * (attenDb - 8.7) : 0.5842 * std::pow(attenDb - 21, 0.4) + 0.07886 * (attenDb - 21);
  int N = int(std::ceil((attenDb - 7.95) / (14.36 * transition))) + 1;
  while ((N - 3) % 4 != 0) ++N;
  std::vector<double> h(N, 0.0);
  const int c = N / 2;
  for (int i = 0; i < N; ++i) {
    const int t = i - c;
    double s;
    if (t == 0) s = 0.5;
    else if (t % 2 == 0) s = 0.0;
    else s = std::sin(kPiR() * t / 2.0) / (kPiR() * t);
    const double r = 2.0 * i / (N - 1) - 1.0;
    h[i] = s * besselI0(beta * std::sqrt(std::max(0.0, 1.0 - r * r))) / besselI0(beta);
  }
  double sum = 0;
  for (double v : h) sum += v;
  for (double& v : h) v /= sum;  // gain DC = 1
  return h;
}

class Halfband2 {  // décimateur 2:1 streaming
 public:
  Halfband2() = default;
  explicit Halfband2(std::vector<double> h) : h_(std::move(h)) {
    N_ = int(h_.size());
    c_ = N_ / 2;
    buf_.assign(size_t(2 * N_), 0.0);
    for (int i = 0; i < N_; ++i) if (i != c_ && (i - c_) % 2 != 0) { odd_.push_back(i); oddW_.push_back(h_[i]); }
  }
  void reset() { std::fill(buf_.begin(), buf_.end(), 0.0); pos_ = 0; phase_ = 0; }
  int length() const { return N_; }
  double groupDelayIn() const { return 0.5 * (N_ - 1); }  // en échantillons d'entrée
  // Pousse un échantillon ; retourne true et écrit *y quand une sortie est produite (1 sur 2).
  bool push(double x, double* y) {
    buf_[pos_] = x; buf_[pos_ + N_] = x;  // double écriture : fenêtre contiguë sans modulo
    pos_ = (pos_ + 1 == N_) ? 0 : pos_ + 1;
    phase_ ^= 1;
    if (phase_) return false;
    const double* w = &buf_[pos_];  // w[0] = plus ancien, w[N-1] = plus récent
    double acc = h_[c_] * w[c_];
    const size_t n = odd_.size();
    for (size_t k = 0; k < n; ++k) acc += oddW_[k] * w[odd_[k]];
    *y = acc;
    return true;
  }
  int macsPerOutput() const { return int(odd_.size()) + 1; }

 private:
  std::vector<double> h_, buf_, oddW_;
  std::vector<int> odd_;
  int N_ = 0, c_ = 0, pos_ = 0, phase_ = 0;
};

class Decimator {
 public:
  explicit Decimator(int K = 1) : K_(K) {
    if (K != 1 && K != 2 && K != 4) throw std::invalid_argument("K doit valoir 1, 2 ou 4");
    // Retard d'un étage (sortie émise après les entrées d'indice impair, centre c = (N-1)/2) : c-1 éch.
    // d'entrée par rapport à l'instant hôte idéal m*K (cf. rapport §4.3). c-1 est pair par construction.
    int lagEng = 0;  // en échantillons moteur
    if (K >= 2) last_ = Halfband2(designHalfband(0.05, 100.0));
    if (K == 4) {
      first_ = Halfband2(designHalfband(0.275, 100.0));
      lagEng = (first_.length() / 2 - 1) + 2 * (last_.length() / 2 - 1);
    } else if (K == 2) {
      lagEng = last_.length() / 2 - 1;
    }
    pad_ = (K - lagEng % K) % K;  // complément : latence totale = multiple entier de K
    latencyHost_ = (lagEng + pad_) / K;
    padBuf_.assign(size_t(pad_ + 1), 0.0);
  }
  void reset() { first_.reset(); last_.reset(); std::fill(padBuf_.begin(), padBuf_.end(), 0.0); padPos_ = 0; }
  int K() const { return K_; }
  int latencyHost() const { return latencyHost_; }
  int pad() const { return pad_; }
  int macsPerHostSample() const {
    if (K_ == 1) return 0;
    if (K_ == 2) return last_.macsPerOutput();
    return last_.macsPerOutput() + 2 * first_.macsPerOutput();
  }
  int tapsFinal() const { return last_.length(); }
  int tapsFirst() const { return first_.length(); }
  // in : nHost*K échantillons moteur -> out : nHost échantillons hôte.
  void process(const double* in, double* out, int nHost) {
    if (K_ == 1) { for (int i = 0; i < nHost; ++i) out[i] = in[i]; return; }
    int o = 0;
    for (int i = 0; i < nHost * K_; ++i) {
      double x = in[i];
      if (pad_) { padBuf_[padPos_] = x; padPos_ = (padPos_ + 1) % (pad_ + 1); x = padBuf_[padPos_]; }
      double y;
      if (K_ == 4) { if (!first_.push(x, &y)) continue; x = y; }
      if (last_.push(x, &y)) out[o++] = y;
    }
  }

 private:
  int K_;
  Halfband2 first_, last_;
  int pad_ = 0, latencyHost_ = 0, padPos_ = 0;
  std::vector<double> padBuf_;
};

}  // namespace s01
