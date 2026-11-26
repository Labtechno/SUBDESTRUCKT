// SUBDESTRUCKT — L3 — Filter & Resonator Engine (référentiel §4) : emplacement de filtre.
// Algorithmes : F1 Morphing Multimode (SVF TPT), F2 Resonant Comb, F3 Nonlinear Resonant Filter, F4 Modal Resonator
// Bank. Implémentation : Filters.cpp. Aucune boucle filtre -> oscillateur (pas de feedback post-FX).
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

class FilterSlot {
 public:
  static constexpr int kMaxModes = 16;
  void configure(double fs);  // hors RT : préallocation (comb : retard maximal à 20 Hz)
  void reset() noexcept;
  // E : valeurs Effective ; base : StableID <slot>.type
  void process(double& L, double& R, const double* E, int base, const FilterContext& c) noexcept;
  bool selfOscillating() const noexcept { return energy_ > 1e-6; }
  double energy() const noexcept { return energy_; }

 private:
  struct Svf {
    double ic1 = 0.0, ic2 = 0.0;
    void reset() noexcept { ic1 = ic2 = 0.0; }
  };
  struct Modal {
    double y1 = 0.0, y2 = 0.0;
  };
  void switchType(int type) noexcept;
  double fs_ = 48000.0;
  int type_ = 0, pendingType_ = 0, fade_ = 0, fadeLen_ = 1;
  bool fadingOut_ = false;
  // F1/F3 : deux étages SVF par canal
  std::array<std::array<Svf, 2>, 2> svf_{};
  // F2 : lignes de retard par canal
  std::array<std::vector<double>, 2> comb_;
  std::array<int, 2> combPos_{};
  std::array<double, 2> combLp_{};
  // F4 : 16 modes par canal (résonateurs à deux pôles)
  std::array<std::array<Modal, kMaxModes>, 2> modes_{};
  std::array<double, 2> dcState_{};
  double energy_ = 0.0;
  double excPending_ = 0.0;
};

}  // namespace sbd
