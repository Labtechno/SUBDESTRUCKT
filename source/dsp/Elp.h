// SUBDESTRUCKT — SPEC-HR-01 (PROPOSÉE, soumise à l'arbitrage du propriétaire) : protection d'extrême niveau (ELP).
// Fonction de transfert par échantillon, impaire, appliquée avant la garde numérique de sortie :
//   |y| <= T          : y inchangé (aucune opération : chemin bit à bit identique au moteur sans ELP)
//   |y| >  T          : sign(y) * (T + R' * tanh((|y| - T) / R')),  R' = (G - T) * (1 - 1e-7)
// avec T = 16 (+24,08 dBFS) et G = 32 (+30,10 dBFS, garde numérique). Propriétés (ELP-01..03) : continuité C0 en T,
// pente exactement 1 en T (C1), croissante (strictement en arithmétique exacte ; en double précision, plateau de tanh
// au-delà d'environ +50 dBFS d'entrée, voir ELP-01), 1-lipschitzienne (non expansive : aucun saut créé), bornée
// strictement sous G pour toute entrée finie ; la garde numérique reste un étage distinct. La marge 1e-7 sur
// l'asymptote garantit que la conversion float de la sortie reste < 32,0f (ELP-05, correctif G4.2 ; G4.1 : facteur
// global (1 - 1e-9), sortie float égale à 32,0f et pente 1 - 1e-9 au seuil).
#pragma once
#include <cmath>

namespace sbd {

constexpr double kElpThreshold = 16.0;  // seuil d'engagement (+24,08 dBFS)
constexpr double kOutputGuard = 32.0;   // garde numérique de sortie (+30,10 dBFS) ; > 0 dBFS reste autorisé

// Mise en forme ELP d'une valeur finie dont |y| > kElpThreshold (l'appelant teste le seuil : sous le seuil, aucune
// opération n'est effectuée, ce qui garantit l'identité bit à bit).
inline double elpShape(double y) noexcept {
  const double ay = std::fabs(y);
  constexpr double r = (kOutputGuard - kElpThreshold) * (1.0 - 1e-7);  // asymptote 32 - 1,6e-6 : float < 32,0f
  const double m = kElpThreshold + r * std::tanh((ay - kElpThreshold) / r);
  return y > 0.0 ? m : -m;
}

// Fonction complète (tests, documentation) : identité sous le seuil, mise en forme au-delà.
inline double elpTransfer(double y) noexcept {
  const double ay = std::fabs(y);
  return (ay > kElpThreshold && std::isfinite(ay)) ? elpShape(y) : y;
}

}  // namespace sbd
