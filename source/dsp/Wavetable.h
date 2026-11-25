// SUBDESTRUCKT — L3 DSP primitives — table d'onde limitée en bande (WT minimal de la tranche ARCH-G3).
//
// Une seule forme (dent de scie), 2048 échantillons par cycle, 11 niveaux mip (le niveau L contient les harmoniques
// <= 2^(10-L)). Construite une fois hors RT, immuable ensuite, partageable en lecture entre instances.
// Lecture : interpolation linéaire en phase + fondu entre deux niveaux choisis d'après |incrément de phase instantané|,
// ce qui couvre la modulation de phase (PM) avec la même règle. Pas de X/Y, pas de Z, pas de Character (hors tranche).
#pragma once
#include <vector>

namespace sbd {

class SawWavetable {
 public:
  static constexpr int kLength = 2048;
  static constexpr int kLevels = 11;

  SawWavetable();  // non RT : synthèse additive des niveaux

  // phase dans [0,1) ; absIncrement = |fréquence instantanée| / Fs moteur (cycles par échantillon).
  double read(double phase, double absIncrement) const noexcept;

  // Accès direct à un niveau (tests).
  const double* level(int L) const noexcept { return &table_[static_cast<size_t>(L) * (kLength + 1)]; }

 private:
  double lerp(int L, double phase) const noexcept;
  std::vector<double> table_;
};

}  // namespace sbd
