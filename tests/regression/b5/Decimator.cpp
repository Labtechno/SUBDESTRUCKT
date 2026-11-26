// ORACLE FIGE 0.1.0-b5 (copie de 8d23342, namespace sbd_b5) : ne jamais modifier. Reference de non-regression ARCH-G4.
#include "Decimator.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "DspConstants.h"

namespace sbd_b5 {

namespace {
double besselI0(double x) {
  double s = 1.0, t = 1.0;
  for (int k = 1; k < 50; ++k) {
    t *= (x / (2.0 * k)) * (x / (2.0 * k));
    s += t;
    if (t < 1e-20 * s) break;
  }
  return s;
}
}  // namespace

// Demi-bande : h[c] = 0.5, h[c±2m] = 0 (m != 0), h[c±(2m+1)] = sinc fenêtré. Longueur N = 4M+3.
std::vector<double> designHalfband(double transition, double attenuationDb) {
  const double beta = attenuationDb > 50 ? 0.1102 * (attenuationDb - 8.7)
                                         : 0.5842 * std::pow(attenuationDb - 21, 0.4) + 0.07886 * (attenuationDb - 21);
  int N = static_cast<int>(std::ceil((attenuationDb - 7.95) / (14.36 * transition))) + 1;
  while ((N - 3) % 4 != 0) ++N;
  std::vector<double> h(static_cast<size_t>(N), 0.0);
  const int c = N / 2;
  for (int i = 0; i < N; ++i) {
    const int t = i - c;
    double s;
    if (t == 0)
      s = 0.5;
    else if (t % 2 == 0)
      s = 0.0;
    else
      s = std::sin(kPi * t / 2.0) / (kPi * t);
    const double r = 2.0 * i / (N - 1) - 1.0;
    h[static_cast<size_t>(i)] = s * besselI0(beta * std::sqrt(std::max(0.0, 1.0 - r * r))) / besselI0(beta);
  }
  double sum = 0;
  for (double v : h) sum += v;
  for (double& v : h) v /= sum;  // gain DC = 1
  return h;
}

Halfband2::Halfband2(std::vector<double> h) : h_(std::move(h)) {
  N_ = static_cast<int>(h_.size());
  c_ = N_ / 2;
  buf_.assign(static_cast<size_t>(2 * N_), 0.0);
  for (int i = 0; i < N_; ++i)
    if (i != c_ && (i - c_) % 2 != 0) {
      odd_.push_back(i);
      oddW_.push_back(h_[static_cast<size_t>(i)]);
    }
}

void Halfband2::reset() noexcept {
  std::fill(buf_.begin(), buf_.end(), 0.0);
  pos_ = 0;
  phase_ = 0;
}

bool Halfband2::push(double x, double* y) noexcept {
  buf_[static_cast<size_t>(pos_)] = x;
  buf_[static_cast<size_t>(pos_ + N_)] = x;  // double écriture : fenêtre contiguë sans modulo
  pos_ = (pos_ + 1 == N_) ? 0 : pos_ + 1;
  phase_ ^= 1;
  if (phase_) return false;
  const double* w = &buf_[static_cast<size_t>(pos_)];  // w[0] = plus ancien, w[N-1] = plus récent
  double acc = h_[static_cast<size_t>(c_)] * w[c_];
  const size_t n = odd_.size();
  for (size_t k = 0; k < n; ++k) acc += oddW_[k] * w[odd_[k]];
  *y = acc;
  return true;
}

Decimator::Decimator(int K) : K_(K) {
  if (K != 1 && K != 2 && K != 4) throw std::invalid_argument("Decimator: K doit valoir 1, 2 ou 4");
  // Retard d'un étage : c-1 échantillons d'entrée par rapport à l'instant hôte idéal (sortie émise après les entrées
  // d'indice impair, centre c = (N-1)/2). c-1 est pair par construction.
  int lagEngine = 0;
  if (K >= 2) last_ = Halfband2(designHalfband(0.05, 100.0));
  if (K == 4) {
    first_ = Halfband2(designHalfband(0.275, 100.0));
    lagEngine = (first_.length() / 2 - 1) + 2 * (last_.length() / 2 - 1);
  } else if (K == 2) {
    lagEngine = last_.length() / 2 - 1;
  }
  pad_ = (K - lagEngine % K) % K;  // complément : latence totale = multiple entier de K
  latencyHost_ = (lagEngine + pad_) / K;
  padBuf_.assign(static_cast<size_t>(pad_ + 1), 0.0);
}

void Decimator::reset() noexcept {
  first_.reset();
  last_.reset();
  std::fill(padBuf_.begin(), padBuf_.end(), 0.0);
  padPos_ = 0;
}

void Decimator::process(const double* in, double* out, int nHost) noexcept {
  if (K_ == 1) {
    for (int i = 0; i < nHost; ++i) out[i] = in[i];
    return;
  }
  int o = 0;
  const int n = nHost * K_;
  for (int i = 0; i < n; ++i) {
    double x = in[i];
    if (pad_) {
      padBuf_[static_cast<size_t>(padPos_)] = x;
      padPos_ = (padPos_ + 1) % (pad_ + 1);
      x = padBuf_[static_cast<size_t>(padPos_)];
    }
    double y;
    if (K_ == 4) {
      if (!first_.push(x, &y)) continue;
      x = y;
    }
    if (last_.push(x, &y)) out[o++] = y;
  }
}

}  // namespace sbd_b5
