// SUBDESTRUCKT — L3 DSP primitives — décimation K -> 1 (K = 1, 2, 4) au rythme hôte.
//
// Cascade de demi-bandes FIR à phase linéaire (fenêtre de Kaiser, ~100 dB), reprise du spike S0.1 (validée) :
//   étage final 2fsH -> fsH : transition 0.225..0.275 de sa fréquence d'entrée ; étage 4fsH -> 2fsH : 0.1125..0.3875.
// La latence totale est arrondie à un nombre entier d'échantillons hôte par une ligne à retard d'entrée, pour une
// latence VST3 déclarée entière. Mémoire allouée à la construction (hors RT) ; process() n'alloue pas.
#pragma once
#include <vector>

namespace sbd {

std::vector<double> designHalfband(double transition, double attenuationDb);

class Halfband2 {
 public:
  Halfband2() = default;
  explicit Halfband2(std::vector<double> h);
  void reset() noexcept;
  int length() const noexcept { return N_; }
  // Pousse un échantillon ; retourne true et écrit *y quand une sortie est produite (une sur deux).
  bool push(double x, double* y) noexcept;
  int macsPerOutput() const noexcept { return static_cast<int>(odd_.size()) + 1; }

 private:
  std::vector<double> h_, buf_, oddW_;
  std::vector<int> odd_;
  int N_ = 0, c_ = 0, pos_ = 0, phase_ = 0;
};

class Decimator {
 public:
  explicit Decimator(int K = 1);  // K doit valoir 1, 2 ou 4
  void reset() noexcept;
  int K() const noexcept { return K_; }
  int latencyHostSamples() const noexcept { return latencyHost_; }
  // in : nHost*K échantillons moteur -> out : nHost échantillons hôte.
  void process(const double* in, double* out, int nHost) noexcept;

 private:
  int K_ = 1;
  Halfband2 first_, last_;
  int pad_ = 0, latencyHost_ = 0, padPos_ = 0;
  std::vector<double> padBuf_;
};

}  // namespace sbd
