// SUBDESTRUCKT — L3 — transformations Z spectrales (ARCH-G4.3, catalogue Z validé ; docs/V1-ARCH-G4.3-SPEC.md §7).
//
// Opèrent sur les coefficients harmoniques complexes c[h] (h = 1..1024) d'une forme d'onde à un cycle : Z1 Harmonic Tilt,
// Z4 Harmonic Mask, Z5 Harmonic Focus, Z6 Harmonic Shift, Z7 Phase Scatter. Fonctions pures, déterministes, sans
// allocation (tampon de travail fourni par l'appelant) ; appelées hors chemin par échantillon (cadence de resynthèse).
// v = profondeur effective dans [0,1] (v = 0 : identité exacte) ; sigma = Shape dans [-1,1].
#pragma once
#include <cmath>
#include <complex>
#include <algorithm>
#include <cstdint>

namespace sbd {

enum class ZType : int { Off = 0, Fold = 1, Warp = 2, Mutation = 3, Tilt = 4, Mask = 5, Focus = 6, Shift = 7, Scatter = 8 };
inline bool zSpectral(int t) noexcept { return t >= static_cast<int>(ZType::Tilt) && t <= static_cast<int>(ZType::Scatter); }

namespace zspec {

constexpr int kH = 1024;
inline double lambda(double sigma) noexcept { return 0.5 + 1.5 * sigma; }  // [-1 ; 2]
inline double maskPeriod(double sigma) noexcept {                          // [2 ; 8], 3 à sigma = 0
  return sigma >= 0.0 ? 3.0 * std::pow(8.0 / 3.0, sigma) : 3.0 * std::pow(2.0 / 3.0, -sigma);
}
inline double focusCentre(double sigma) noexcept { return std::exp2(3.5 * (sigma + 1.0)); }  // [1 ; 128]
inline double focusWidth(double v) noexcept { return 0.25 + 1.25 * (1.0 - v); }             // octaves
inline double shiftRanks(double v, double sigma) noexcept { return 8.0 * v * lambda(sigma); }  // [-8 ; 16]

// motif de Phase Scatter : hachage déterministe (splitmix64) -> [-1 ; 1]
inline double scatterR(int h, int pattern) noexcept {
  std::uint64_t z = static_cast<std::uint64_t>(h) * 0x9E3779B97F4A7C15ULL + static_cast<std::uint64_t>(pattern + 1) * 0xD1B54A32D192ED03ULL;
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  z ^= z >> 31;
  return static_cast<double>(z >> 11) * (2.0 / 9007199254740992.0) - 1.0;
}
inline double scatterRho(int h, double sigma) noexcept {
  double P = 3.5 * (sigma + 1.0);
  int i = static_cast<int>(std::floor(P));
  if (i >= 7) i = 6;
  if (i < 0) i = 0;
  const double f = P - i;
  return (1.0 - f) * scatterR(h, i) + f * scatterR(h, i + 1);
}

// gain réel par rang (Tilt, Mask, Focus)
inline double gain(ZType t, int h, double v, double sigma) noexcept {
  switch (t) {
    case ZType::Tilt: return std::exp(v * lambda(sigma) * std::log(static_cast<double>(h)));
    case ZType::Mask: {
      const double m = 0.5 - 0.5 * std::cos(6.283185307179586 * h / maskPeriod(sigma));
      return 1.0 - v * (1.0 - m);
    }
    case ZType::Focus: {
      const double b = focusWidth(v), l = std::log2(h / focusCentre(sigma));
      return (1.0 - v) + v * std::exp(-l * l / (2.0 * b * b));
    }
    default: return 1.0;
  }
}

// Contrôle de crête désactivé (tests des fonctions pures) : un foncteur de crête renvoie la crête du cycle à bande lue
// resynthétisé depuis des coefficients ; une valeur négative désactive le contrôle.
struct NoPeak {
  double operator()(const std::complex<double>*) const noexcept { return -1.0; }
};
struct UnitBand {
  double operator()(int) const noexcept { return 1.0; }
};
constexpr double kTiltPeakRatio = 2.0;  // κ : élévation de crête maximale par étage Tilt (+6,02 dB), ARCH-G4.3-R1

struct StageStats {
  double energyFactor = 1.0;  // produit des facteurs énergétiques des réductions de crête (Tilt)
  int peakReductions = 0;
};

// Harmonic Tilt (ARCH-G4.3-R1, A-G43-01) : pondération h^(vλ) puis normalisation de l'énergie de la bande lue sur le
// domaine D (h >= 2 si la fondamentale est protégée), puis contrôle de crête (κ). Redistribution, jamais amplification.
template <typename Band, typename Peak>
inline void tiltStage(std::complex<double>* c, std::complex<double>* work, double v, double sigma, bool fund, Band band, Peak peak,
                      StageStats* st) noexcept {
  const int h0 = fund ? 2 : 1;
  for (int h = 0; h <= kH + 1; ++h) work[h] = c[h];  // entrée de l'étage (pour la crête de référence)
  double ein = 0.0, et = 0.0;
  for (int h = h0; h <= kH; ++h) {
    const double b = band(h);
    c[h] *= gain(ZType::Tilt, h, v, sigma);
    if (b <= 0.0) continue;
    ein += b * b * std::norm(work[h]);
    et += b * b * std::norm(c[h]);
  }
  if (!(ein > 1e-24 * (ein + et)) || !(et > 0.0) || !std::isfinite(et)) {  // énergie nulle ou négligeable : identité
    for (int h = h0; h <= kH; ++h) c[h] = work[h];
    return;
  }
  const double sc = std::sqrt(ein / et);
  for (int h = h0; h <= kH; ++h) c[h] *= sc;
  const double pin = peak(work);
  if (!(pin > 0.0)) return;  // contrôle désactivé (foncteur neutre) ou entrée nulle
  const double pout = peak(c), lim = kTiltPeakRatio * pin;
  if (!(pout > lim)) return;
  // réduction de gain continue des rangs de D : la crête est convexe en alpha (somme fondamentale + alpha x reste), la
  // sécante depuis alpha = 0 (crête de la fondamentale seule, <= 2 pin) garantit crête(alpha) <= lim sans itération
  const double p0 = fund ? std::abs(work[1]) * std::max(0.0, band(1)) : 0.0;
  const double alpha = std::max(0.0, std::min(1.0, (lim - p0) / (pout - p0)));
  for (int h = h0; h <= kH; ++h) c[h] *= alpha;
  if (st) {
    st->energyFactor *= alpha * alpha;
    ++st->peakReductions;
  }
}

// applique une transformation à c[1..kH] (c[0] ignoré) ; work : tampon de kH + 2 coefficients
template <typename Band = UnitBand, typename Peak = NoPeak>
inline void applyStage(ZType t, std::complex<double>* c, std::complex<double>* work, double v, double sigma, bool fund = false,
                       Band band = Band{}, Peak peak = Peak{}, StageStats* st = nullptr) noexcept {
  if (!(v > 0.0)) return;
  switch (t) {
    case ZType::Tilt: tiltStage(c, work, v, sigma, fund, band, peak, st); break;
    case ZType::Mask:
    case ZType::Focus:
      for (int h = 1; h <= kH; ++h) c[h] *= gain(t, h, v, sigma);
      break;
    case ZType::Shift: {
      // balayage de rang (A-G43-02) ; ARCH-G4.3-R1 : rangs virtuels nuls en 0 et kH + 1 (extinction continue aux bornes)
      // et phase interpolée sur l'arc le plus court (continuité en k ; amplitude interpolée inchangée)
      const double k = shiftRanks(v, sigma);
      for (int h = 0; h <= kH + 1; ++h) work[h] = h >= 1 && h <= kH ? c[h] : std::complex<double>(0.0, 0.0);
      for (int h = 1; h <= kH; ++h) {
        const double src = h - k;
        if (!(src > 0.0) || !(src < kH + 1.0)) {
          c[h] = 0.0;
          continue;
        }
        const int i = static_cast<int>(std::floor(src));
        const double f = src - i;
        if (f == 0.0) {
          c[h] = work[i];
          continue;
        }
        const std::complex<double> a = work[i], b = work[i + 1];
        const double ma = std::abs(a), mb = std::abs(b), m = (1.0 - f) * ma + f * mb;
        double ph;
        if (ma <= 0.0) {
          ph = std::arg(b);
        } else if (mb <= 0.0) {
          ph = std::arg(a);
        } else {
          const double pa = std::arg(a);
          double d = std::arg(b) - pa;
          d -= 6.283185307179586 * std::floor((d + 3.141592653589793) / 6.283185307179586);  // [-pi ; pi[
          ph = pa + f * d;
        }
        c[h] = std::polar(m, ph);
      }
      break;
    }
    case ZType::Scatter:
      for (int h = 1; h <= kH; ++h) c[h] *= std::polar(1.0, v * 3.141592653589793 * scatterRho(h, sigma));
      break;
    default: break;
  }
}
inline void apply(ZType t, std::complex<double>* c, std::complex<double>* work, double v, double sigma) noexcept {
  applyStage(t, c, work, v, sigma);
}

// Chaîne complète appliquée par le moteur : transformations spectrales dans l'ordre des emplacements (Tilt : énergie
// constante et contrôle de crête intrinsèques), préservation de la fondamentale (c1 = c1 d'origine), compensation exacte
// de l'énergie (Parseval) sur la bande lue, fondamentale préservée exclue, gain borné [0,25 ; 4] ; son énergie cible
// intègre les réductions de crête des étages Tilt (aucune double compensation). orig : coefficients d'entrée (non
// modifiés) ; band(h) : gain de bande lue ; peak : crête du cycle à bande lue (NoPeak : contrôle désactivé).
template <typename Band, typename Peak = NoPeak>
inline double process(std::complex<double>* c, const std::complex<double>* orig, std::complex<double>* work, const int* zt,
                      const double* zv, const double* zs, bool fund, bool gainComp, Band band, Peak peak = Peak{},
                      StageStats* stats = nullptr) noexcept {
  StageStats local;
  StageStats* st = stats ? stats : &local;
  for (int s = 0; s < 2; ++s)
    if (zSpectral(zt[s]) && zv[s] > 0.0) applyStage(static_cast<ZType>(zt[s]), c, work, zv[s], zs[s], fund, band, peak, st);
  if (fund) c[1] = orig[1];
  double gc = 1.0;
  if (gainComp) {
    double ein = 0.0, eout = 0.0;
    for (int h = fund ? 2 : 1; h <= kH; ++h) {
      const double g = band(h);
      if (g <= 0.0) continue;
      ein += g * g * std::norm(orig[h]);
      eout += g * g * std::norm(c[h]);
    }
    ein *= st->energyFactor;
    gc = eout > 1e-30 ? std::min(4.0, std::max(0.25, std::sqrt(ein / eout))) : 1.0;
    for (int h = fund ? 2 : 1; h <= kH; ++h) c[h] *= gc;
  }
  return gc;
}

}  // namespace zspec
}  // namespace sbd
