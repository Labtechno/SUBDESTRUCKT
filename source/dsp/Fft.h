// SUBDESTRUCKT — L3 DSP primitives — FFT complexe radix-2 itérative (taille puissance de 2).
// Plan (twiddles, permutation) construit hors RT ; transform() n'alloue pas (tampons fournis par l'appelant).
#pragma once
#include <cmath>
#include <complex>
#include <vector>

#include "DspConstants.h"

namespace sbd {

class Fft {
 public:
  Fft() = default;
  explicit Fft(int n) { init(n); }
  void init(int n) {  // hors RT
    n_ = n;
    log2n_ = 0;
    while ((1 << log2n_) < n) ++log2n_;
    tw_.resize(static_cast<size_t>(n / 2));
    for (int k = 0; k < n / 2; ++k) tw_[static_cast<size_t>(k)] = std::polar(1.0, -kTwoPi * k / n);
    rev_.resize(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
      int r = 0;
      for (int b = 0; b < log2n_; ++b)
        if (i & (1 << b)) r |= 1 << (log2n_ - 1 - b);
      rev_[static_cast<size_t>(i)] = r;
    }
  }
  int size() const noexcept { return n_; }
  // En place. inverse : transformée inverse NON normalisée (diviser par n).
  void transform(std::complex<double>* x, bool inverse) const noexcept {
    for (int i = 0; i < n_; ++i) {
      const int r = rev_[static_cast<size_t>(i)];
      if (r > i) std::swap(x[i], x[r]);
    }
    for (int len = 2; len <= n_; len <<= 1) {
      const int half = len >> 1, step = n_ / len;
      for (int i = 0; i < n_; i += len)
        for (int k = 0; k < half; ++k) {
          std::complex<double> w = tw_[static_cast<size_t>(k * step)];
          if (inverse) w = std::conj(w);
          const std::complex<double> u = x[i + k], v = x[i + k + half] * w;
          x[i + k] = u + v;
          x[i + k + half] = u - v;
        }
    }
  }

 private:
  int n_ = 0, log2n_ = 0;
  std::vector<std::complex<double>> tw_;
  std::vector<int> rev_;
};

}  // namespace sbd
