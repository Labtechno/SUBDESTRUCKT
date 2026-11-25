// SUBDESTRUCKT — Spike S0.1 — retard causal des arêtes Delayed : T1 (1 éch. hôte) ou T2 (1/48000 s).
// Tous les lecteurs n'utilisent que y[n-d] avec d >= 1 : jamais l'échantillon courant.
// Méthodes fractionnaires comparées (§11) :
//   Lagrange3 : Lagrange cubique sur y[n-d0..n-d0-3], d0 = clamp(floor(D)-1, 1) (candidat S0) ;
//   Linear    : interpolation linéaire y[n-d0], y[n-d0-1], d0 = floor(D) (coût minimal) ;
//   Thiran1   : passe-tout du 1er ordre (|H| = 1 exactement), d0 = floor(D-0.5), frac dans [0.5,1.5).
#pragma once
#include <array>
#include <cmath>
#include <complex>
#include <stdexcept>

namespace s01 {

constexpr double kT2Seconds = 1.0 / 48000.0;  // 20.833333 µs

enum class TimeRef { T1, T2 };
enum class Frac { Lagrange3, Linear, Thiran1 };
inline const char* name(TimeRef t) { return t == TimeRef::T1 ? "T1" : "T2"; }
inline const char* name(Frac f) { return f == Frac::Lagrange3 ? "lagrange3" : f == Frac::Linear ? "linear" : "thiran1"; }

constexpr int kRing = 32;  // puissance de 2 ; retard max = 8 éch. moteur (T2, 96 kHz, K=4) + 4 prises

struct DelaySpec {
  TimeRef ref = TimeRef::T1;
  Frac frac = Frac::Lagrange3;
  double D = 1.0;          // retard demandé en échantillons moteur
  bool integer = true;
  int di = 1;              // retard entier
  int d0 = 1;              // première prise (fractionnaire)
  std::array<double, 4> w{};  // Lagrange/Linear
  double a = 0.0;          // Thiran
};

// T2 non représentable (D < 1 éch. moteur) -> refus explicite : jamais de plancher silencieux à 22.676 µs.
struct T2NotRepresentable : std::runtime_error {
  using std::runtime_error::runtime_error;
};

inline DelaySpec makeDelay(TimeRef ref, Frac frac, int K, double fsHost) {
  DelaySpec d;
  d.ref = ref; d.frac = frac;
  const double fsE = fsHost * K;
  d.D = (ref == TimeRef::T1) ? double(K) : fsE * kT2Seconds;
  if (d.D < 1.0 - 1e-12) throw T2NotRepresentable("T2 exige K>=2 a ce sample rate (D<1 ech. moteur)");
  const double r = std::round(d.D);
  if (std::fabs(d.D - r) < 1e-9) { d.integer = true; d.di = int(r); return d; }
  d.integer = false;
  switch (frac) {
    case Frac::Lagrange3: {
      d.d0 = std::max(1, int(std::floor(d.D)) - 1);
      for (int k = 0; k < 4; ++k) {
        double wk = 1.0;
        for (int m = 0; m < 4; ++m)
          if (m != k) wk *= (d.D - (d.d0 + m)) / double(k - m);
        d.w[k] = wk;
      }
      break;
    }
    case Frac::Linear: {
      d.d0 = int(std::floor(d.D));
      const double f = d.D - d.d0;
      d.w = {1.0 - f, f, 0.0, 0.0};
      break;
    }
    case Frac::Thiran1: {
      d.d0 = int(std::floor(d.D - 0.5));
      if (d.d0 < 1) throw T2NotRepresentable("Thiran1 exige D>=1.5");
      const double f = d.D - d.d0;  // [0.5, 1.5)
      d.a = (1.0 - f) / (1.0 + f);
      break;
    }
  }
  return d;
}

// Réponse en fréquence théorique du lecteur (om = rad/échantillon moteur).
inline std::complex<double> delayResponse(const DelaySpec& d, double om) {
  const std::complex<double> j(0, 1);
  if (d.integer) return std::exp(-j * om * double(d.di));
  if (d.frac == Frac::Thiran1) {
    const std::complex<double> z1 = std::exp(-j * om);
    return std::exp(-j * om * double(d.d0)) * (d.a + z1) / (1.0 + d.a * z1);
  }
  std::complex<double> h(0, 0);
  const int taps = d.frac == Frac::Lagrange3 ? 4 : 2;
  for (int k = 0; k < taps; ++k) h += d.w[k] * std::exp(-j * om * double(d.d0 + k));
  return h;
}

// Lecture du retard d'une source. ring[(n-d) & (kRing-1)] = y[n-d]. state : état Thiran par source.
inline double readDelay(const DelaySpec& d, const double* ring, long long n, double& state) {
  if (d.integer) return ring[(n - d.di) & (kRing - 1)];
  switch (d.frac) {
    case Frac::Lagrange3:
      return d.w[0] * ring[(n - d.d0) & (kRing - 1)] + d.w[1] * ring[(n - d.d0 - 1) & (kRing - 1)] +
             d.w[2] * ring[(n - d.d0 - 2) & (kRing - 1)] + d.w[3] * ring[(n - d.d0 - 3) & (kRing - 1)];
    case Frac::Linear:
      return d.w[0] * ring[(n - d.d0) & (kRing - 1)] + d.w[1] * ring[(n - d.d0 - 1) & (kRing - 1)];
    case Frac::Thiran1: default: {
      // v[n] = a*x[n-d0] + x[n-d0-1] - a*v[n-1]
      const double v = d.a * ring[(n - d.d0) & (kRing - 1)] + ring[(n - d.d0 - 1) & (kRing - 1)] - d.a * state;
      state = v;
      return v;
    }
  }
}

}  // namespace s01
