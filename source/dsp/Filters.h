// SUBDESTRUCKT — L3 — Filter & Resonator Engine (référentiel §4) : emplacement de filtre.
// Algorithmes :
//  F1 Morphing Multimode : SVF TPT (Zavalishin), LP/BP/HP 12/24 dB et Notch, keytracking, résonance jusqu'à
//     l'auto-oscillation bornée (saturation douce de l'état passe-bande).
//  F2 Resonant Comb : retard fractionnaire (Lagrange 3), feedback bipolaire, amortissement (passe-bas 1 pôle compensé
//     en phase pour l'accord), keytracking, désaccord et étalement stéréo en Dual Mono, excitation.
//  F3 Nonlinear Resonant Filter : SVF TPT avec drive d'entrée, saturation dans la boucle (Character), bias (asymétrie).
//  F4 Modal Resonator Bank : 1..16 résonateurs à deux pôles (modèles Harmonic/Metal/Membrane/Inharmonic), decay,
//     damping des modes hauts, brightness, inharmonicité, Freeze (état autonome : seulement si l'emplacement est armé).
// Aucune boucle filtre -> oscillateur (pas de feedback post-FX). Mémoire préallouée par configure() (hors RT).
#pragma once
#include <array>
#include <cstdint>
#include <vector>

#include "../core/Parameters.h"

namespace sbd {

// Décalages des paramètres d'un emplacement de filtre par rapport à <slot>.type (ordre généré, vérifié).
enum class FltOff : int {
  Type = 0, Pos, Stereo, F1Mode, Cutoff, Res, Key, Drive, Bias, Char, F3Mode, CombFb, CombDamp, Detune, Spread, Modal,
  Modes, Decay, Damp, Bright, Inharm, Freeze, Exc, ExcLevel, Mix, Out, Arm, Count
};
static_assert(idx(ParamId::FltWtArm) - idx(ParamId::FltWtType) == static_cast<int>(FltOff::Arm), "layout filtre");
static_assert(idx(ParamId::FltFmType) - idx(ParamId::FltWtType) == static_cast<int>(FltOff::Count), "layout filtre");
static_assert(idx(ParamId::FltGlType) - idx(ParamId::FltFmType) == static_cast<int>(FltOff::Count), "layout filtre");

struct FilterContext {
  double fs = 48000.0;
  double noteHz = 261.63;
  bool excite = false;   // impulsion d'excitation à cet échantillon (déjà filtrée par la source choisie)
  bool voiceIdle = false;
};

// Rapports de fréquence des modèles modaux (mode k = 1..16), sans inharmonicité.
double modalRatio(int model, int k) noexcept;

class FilterSlot {
 public:
  static constexpr int kMaxModes = 16;
  static constexpr int kCombMax = 16384;
  void configure(double fs);  // hors RT : préallocation (comb : retard maximal à 20 Hz, fs <= 192 kHz)
  void reset() noexcept;
  // E : valeurs Effective ; base : indice de <slot>.type
  void process(double& L, double& R, const double* E, int base, const FilterContext& c) noexcept;
  bool selfOscillating() const noexcept { return autonomous_; }
  double energy() const noexcept { return energy_; }
  int activeType() const noexcept { return type_; }
  // Réponse d'un mode (tests) : fréquence et rayon du résonateur k du canal 0.
  double testModeFreq(int k) const noexcept { return modeFreq_[static_cast<size_t>(k)]; }

 private:
  struct Svf {
    double ic1 = 0.0, ic2 = 0.0;
  };
  struct Res2 {
    double y1 = 0.0, y2 = 0.0;
  };
  double svf(Svf& s, double x, double g, double k, int out, bool saturate, double drive, double bias, double chr) noexcept;
  double comb(int ch, double x, double delay, double fb, double damp) noexcept;
  void updateModal(const double* E, int base, double f0) noexcept;
  void resetStates() noexcept;
  double fs_ = 48000.0;
  int type_ = 0, pendingType_ = -1, fade_ = 0, fadeLen_ = 64;
  bool fadingOut_ = false;
  std::array<std::array<Svf, 2>, 2> svf_{};            // [canal][étage]
  std::array<std::vector<double>, 2> comb_;
  std::array<int, 2> combPos_{};
  std::array<double, 2> combLp_{};
  std::array<std::array<Res2, kMaxModes>, 2> modes_{};  // [canal][mode]
  std::array<double, kMaxModes> modeA1_{}, modeA2_{}, modeG_{}, modeFreq_{};
  int numModes_ = 0, modalCounter_ = 0;
  double modalKey_ = -1.0;
  std::array<double, 2> excState_{};
  double energy_ = 0.0;
  bool autonomous_ = false;
  double idleSeconds_ = 0.0;
  bool guardFading_ = false;
  int guardFade_ = 0;
};

}  // namespace sbd
