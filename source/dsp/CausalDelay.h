// SUBDESTRUCKT — L3 DSP primitives — retard causal T2 des arêtes Delayed (V1-CAUSAL-CONTRACT).
//
// D = Fs_moteur / 48000 échantillons moteur (durée nominale 1/48000 s), exigé >= 1.
//  - D entier : lecture entière exacte x[n-D].
//  - D fractionnaire : passe-tout de Thiran d'ordre 1, causal :
//        d0 = max(1, floor(D - 0.5)),  f = D - d0,  a = (1 - f) / (1 + f)
//        v[n] = a*x[n-d0] + x[n-d0-1] - a*v[n-1]
//    Ne lit que des échantillons passés (d0 >= 1). |a| < 1 pour f > 0 : stable. Gain unité à toutes les fréquences ;
//    retard de phase = D en DC seulement (le délai de groupe n'est pas constant, il est mesuré et rapporté).
// Aucun choix automatique d'interpolateur selon le preset : le type dépend uniquement de D (entier ou non).
#pragma once
#include <array>
#include <cmath>
#include <complex>

#include "DspConstants.h"

namespace sbd {

struct DelaySpec {
  double D = 1.0;       // retard nominal en échantillons moteur
  bool valid = false;   // D >= 1
  bool integer = true;  // lecture entière exacte
  int di = 1;           // retard entier
  int d0 = 1;           // première prise Thiran
  double a = 0.0;       // coefficient Thiran
};

inline DelaySpec makeT2Delay(double engineRate) noexcept {
  DelaySpec d;
  d.D = engineRate * kT2Seconds;
  d.valid = d.D >= 1.0 - 1e-12;
  if (!d.valid) return d;
  const double r = std::round(d.D);
  if (std::fabs(d.D - r) < 1e-9) {
    d.integer = true;
    d.di = static_cast<int>(r);
    return d;
  }
  d.integer = false;
  d.d0 = static_cast<int>(std::floor(d.D - 0.5));
  if (d.d0 < 1) d.d0 = 1;  // 1 < D < 1.5 : reste causal, approximation de phase moins bonne (hors fréquences officielles)
  const double f = d.D - d.d0;
  d.a = (1.0 - f) / (1.0 + f);
  return d;
}

// Réponse en fréquence théorique (omega en rad/échantillon moteur). Tests et rapports.
inline std::complex<double> delayResponse(const DelaySpec& d, double omega) {
  const std::complex<double> j(0.0, 1.0);
  if (d.integer) return std::exp(-j * omega * static_cast<double>(d.di));
  const std::complex<double> z1 = std::exp(-j * omega);
  return std::exp(-j * omega * static_cast<double>(d.d0)) * (d.a + z1) / (1.0 + d.a * z1);
}

// Historique d'une source et son lecteur retardé. Mémoire fixe, aucune allocation.
class CausalDelayLine {
 public:
  static constexpr int kRing = 32;  // puissance de 2, > d0 + 1 pour toutes les fréquences supportées

  void reset() noexcept {
    ring_.fill(0.0);
    state_ = 0.0;
    n_ = 0;
  }
  // Valeur retardée pour l'échantillon courant n (avant write) : n'utilise que x[n-1], x[n-2], ...
  double read(const DelaySpec& d) noexcept {
    if (d.integer) return ring_[static_cast<size_t>((n_ - d.di) & (kRing - 1))];
    const double v = d.a * ring_[static_cast<size_t>((n_ - d.d0) & (kRing - 1))] +
                     ring_[static_cast<size_t>((n_ - d.d0 - 1) & (kRing - 1))] - d.a * state_;
    state_ = v;
    return v;
  }
  // Enregistre x[n] puis avance n.
  void write(double x) noexcept {
    ring_[static_cast<size_t>(n_ & (kRing - 1))] = x;
    ++n_;
  }
  // Crochet de test : corrompt l'historique.
  void poisonForTest(double v) noexcept { ring_.fill(v); }

 private:
  std::array<double, kRing> ring_{};
  double state_ = 0.0;
  long long n_ = 0;
};

}  // namespace sbd
