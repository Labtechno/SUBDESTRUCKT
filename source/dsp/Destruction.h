// SUBDESTRUCKT — L3 — Destruction Engine (référentiel §3) : emplacement de destruction.
// Catalogue (candidats R&D, définitions techniques consignées dans docs/V1-ARCH-G4-TRACKER.md) :
//  N1 Asymmetric Wavefolder : drive, asymétrie, mélange saturation/repli sinus, 1..4 étages, tone ; ADAA1.
//  D1 Variable Rate Crusher : échantillonnage-blocage 1..128, jitter, 2..16 bits, Raw (repliement) / Controlled
//     (pré-filtrage), Hold Step/Smooth.
//  Y1 Transient Destroyer : compresseur/expandeur à genou, gate, accentuation de transitoires, makeup.
//  S1 Ring/Frequency Shifter : modulation en anneau (sinus/triangle/carré) ou décalage de fréquence (Hilbert IIR).
//  N2 Feedback Waveshaper : ligne de retard courte bouclée sur un saturateur, amortissement, résonance, bias.
//  D2 Glitch Buffer : enregistrement circulaire, boucles de segments (taille, position, répétition aléatoire
//     déterministe, vitesse ±4, feedback, fondu), Freeze.
//  Y2 Envelope Shaper : détecteur crête/RMS, courbe, profondeur bipolaire, hold ; enveloppe exposée comme source.
//  S2 Spectral Freeze : STFT 512..4096 (recouvrement 75 %), gel, decay, blur, tilt, dérive de phase, décalage.
// États indépendants par emplacement ; Dry/Wet par processeur, aligné en latence (S2) ; bornes numériques
// permanentes ; Freeze/auto-oscillation : persistance après la voix seulement si l'emplacement est armé.
#pragma once
#include <array>
#include <complex>
#include <cstdint>
#include <vector>

#include "../core/Parameters.h"
#include "Fft.h"
#include "Rng.h"

namespace sbd {

enum class DstOff : int {
  Type = 0, Mix, Out, Arm, N1Drive, N1Fold, N1Bias, N1Sym, N1Stages, N1Tone, D1Bits, D1Rate, D1Jitter, D1Mode, D1Hold,
  Y1Thresh, Y1Ratio, Y1Attack, Y1Release, Y1Knee, Y1Gate, Y1Transient, Y1Makeup, S1Mode, S1Freq, S1Shift, S1Carrier,
  S1Phase, S1Stereo, N2Fb, N2Delay, N2Damp, N2Bias, N2Res, N2SelfOsc, D2Size, D2Speed, D2Repeat, D2Pos, D2Fb, D2Xfade,
  D2Freeze, Y2Attack, Y2Release, Y2Sens, Y2Curve, Y2Depth, Y2Hold, Y2Detect, S2Fft, S2Freeze, S2Decay, S2Blur, S2Tilt,
  S2Drift, S2Shift, Count
};
static_assert(idx(ParamId::DstWt1S2Shift) - idx(ParamId::DstWt1Type) == static_cast<int>(DstOff::S2Shift), "layout");
static_assert(idx(ParamId::DstWt2Type) - idx(ParamId::DstWt1Type) == static_cast<int>(DstOff::Count), "layout");
static_assert(idx(ParamId::DstGl2Type) - idx(ParamId::DstWt1Type) == 5 * static_cast<int>(DstOff::Count), "layout");

enum class DstType : int { Off = 0, N1, D1, Y1, S1, N2, D2, Y2, S2 };

struct DestructionContext {
  double fs = 48000.0;
  bool voiceIdle = false;
  Quality quality = Quality::Normal;
};

// Mode anti-repliement de N1 (comparaison DST-01) : 0 naïf, 1 ADAA1.
void setN1AntiAliasForTest(int mode) noexcept;

// Suréchantillonnage interne de N1 (hybride ADAA1 + OS, ARCH-G4.1) : Eco x1, Normal x2, High x4.
int n1OversamplingFor(Quality q) noexcept;
int n1LatencyFor(Quality q) noexcept;  // latence (échantillons moteur) du chemin traité, compensée sur le chemin sec

// Filtre demi-bande FIR (longueur 4M+3) pour monter/descendre d'un facteur 2 ; histoires préallouées.
class HalfbandOs {
 public:
  void configure(const std::vector<double>& h);
  void reset() noexcept;
  void up(double x, double* out2) noexcept;      // 1 échantillon -> 2 (gain 2)
  double down(const double* in2) noexcept;       // 2 échantillons -> 1
  int length() const noexcept { return static_cast<int>(h_.size()); }

 private:
  double push(std::vector<double>& buf, int& pos, double x) noexcept;
  std::vector<double> h_, hz_, bu_, bd_;
  std::vector<int> nz_;
  int pu_ = 0, pd_ = 0;
};

class DestructionSlot {
 public:
  static constexpr int kMaxFft = 4096;
  void configure(double fs, std::uint64_t seed, Quality q);  // hors RT : préallocation pire cas
  void reset() noexcept;
  void process(double& L, double& R, const double* E, int base, const DestructionContext& c) noexcept;
  // Latence (échantillons moteur) de l'algorithme actif (S2 : taille FFT ; autres : 0). Dry/wet aligné en interne.
  int latency() const noexcept {
    return type_ == static_cast<int>(DstType::S2) ? fftSize_ : (type_ == static_cast<int>(DstType::N1) ? n1LatencyFor(quality_) : 0);
  }
  static int latencyFor(int type, int fftChoice, Quality q = Quality::Normal) noexcept;
  double y2Env() const noexcept { return y2Env_; }
  // Captures R2 (thread audio, sans allocation) : copie dans un tampon préalloué ; renvoie le nombre de trames/bins
  // (0 si l'algorithme actif n'est ni D2 ni S2). kind : 0 D2, 1 S2.
  int captureInto(float* dst, int capacity, int& kind) const noexcept;
  // Restauration (thread audio, sans allocation) : rééchantillonnage linéaire si la fréquence diffère.
  void restoreFrom(const float* src, int frames, int kind, double srcRate) noexcept;
  static int captureCapacity(double fs) noexcept { return static_cast<int>(std::ceil(2.05 * fs)) + 8; }
  bool frozen() const noexcept { return autonomous_; }
  // D2 Feedback (ARCH-G4.3, A-G42-01 adopté) : 0 = LEGACY (historique), 1 = REGENERATIVE (C1, régénération dans la
  // boucle) ; fixé par le moteur à chaque bloc depuis `dst.<slot>.d2.fb_mode` (défaut de l'emplacement isolé : LEGACY).
  void setD2FeedbackMode(int m) noexcept { d2FbMode_ = m; }
  const std::vector<float>& testD2Buffer(int ch) const noexcept { return d2Buf_[static_cast<size_t>(ch)]; }  // tests
  int activeType() const noexcept { return type_; }
  bool settled() const noexcept { return pendingType_ < 0 && !fadingOut_; }

 private:
  void resetStates() noexcept;
  double n1(int ch, double x, const double* P) noexcept;
  double d1(int ch, double x, const double* P) noexcept;
  void y1(double& L, double& R, const double* P) noexcept;
  void s1(double& L, double& R, const double* P) noexcept;
  double n2(int ch, double x, const double* P, bool armed) noexcept;
  void d2(double& L, double& R, const double* P, bool armed) noexcept;
  void y2(double& L, double& R, const double* P) noexcept;
  void s2(double& L, double& R, const double* P, bool armed) noexcept;
  void s2Frame(int ch, const double* P, bool freeze) noexcept;

  double fs_ = 48000.0;
  Quality quality_ = Quality::Normal;
  int type_ = 0, pendingType_ = -1, fade_ = 0, fadeLen_ = 64;
  bool fadingOut_ = false;
  bool autonomous_ = false;
  double y2Env_ = 0.0;
  double idleSeconds_ = 0.0, energy_ = 0.0;
  bool guardFading_ = false;
  int guardFade_ = 0;
  RngStream rng_;
  std::uint64_t seed_ = 0;

  // N1
  struct N1State {
    std::array<double, 4> prev{};  // ADAA1 : entrée précédente par étage
    double tone = 0.0, dcx = 0.0, dcy = 0.0;
  };
  std::array<N1State, 2> n1_{};
  std::array<std::array<HalfbandOs, 2>, 2> n1Os_{};      // [canal][étage]
  std::array<std::array<double, 4>, 2> n1Pad_{};         // complément d'un demi-échantillon à x4
  std::array<std::vector<double>, 2> n1Dry_;            // chemin sec retardé
  int n1DryPos_ = 0;
  double n1Stages(int ch, double u, const double* P) noexcept;
  // D1
  struct D1State {
    double held = 0.0, prevHeld = 0.0, lp1 = 0.0, lp2 = 0.0;
    double counter = 0.0, period = 1.0;
  };
  std::array<D1State, 2> d1_{};
  // Y1 / Y2
  struct DynState {
    double env = 0.0, fast = 0.0, slow = 0.0, gate = 1.0, rms = 0.0, hold = 0.0;
  };
  DynState y1_{}, y2_{};
  // S1
  struct Allpass2 {
    double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
  };
  struct Hilbert {
    std::array<Allpass2, 4> a{}, b{};  // deux branches de passe-tout (différence de phase ~90°)
    double bDelay = 0.0;               // retard d'un échantillon de la branche B
  };
  std::array<Hilbert, 2> hil_{};
  double s1Phase_ = 0.0;
  // N2
  struct N2State {
    std::vector<double> buf;
    int pos = 0;
    double lp = 0.0, ic1 = 0.0, ic2 = 0.0, dcx = 0.0, dcy = 0.0;
  };
  std::array<N2State, 2> n2_{};
  // D2
  std::array<std::vector<float>, 2> d2Buf_;
  int d2Len_ = 0, d2Write_ = 0;
  double d2Read_ = 0.0, d2SegStart_ = 0.0, d2SegLen_ = 0.0, d2SegPos_ = 0.0;
  double d2Clock_ = 0.0;
  bool d2Looping_ = false;
  int d2FbMode_ = 0;
  // S2
  struct S2State {
    std::vector<double> in, out, dry;
    std::vector<double> mag, phase, inPhase, held;
    int inPos = 0, outPos = 0, dryPos = 0, hop = 0;
  };
  std::array<S2State, 2> s2_{};
  std::array<Fft, 4> ffts_;      // plans 512..4096 préparés hors RT
  const Fft* fft_ = nullptr;
  std::vector<std::complex<double>> work_;
  std::vector<double> window_, tmp_;
  int fftSize_ = 2048, fftChoice_ = 2;
};

}  // namespace sbd
