// SUBDESTRUCKT — L3 — Destruction Engine (référentiel §3) : emplacement de destruction.
// Catalogue : N1 Asymmetric Wavefolder, D1 Variable Rate Crusher, Y1 Transient Destroyer, S1 Ring/Frequency Shifter,
// N2 Feedback Waveshaper, D2 Glitch Buffer, Y2 Envelope Shaper, S2 Spectral Freeze. Implémentation : Destruction.cpp.
// États indépendants par emplacement ; Dry/Wet par processeur, aligné en latence ; bornes numériques permanentes.
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

class DestructionSlot {
 public:
  void configure(double fs, std::uint64_t seed, Quality q);  // hors RT : préallocation pire cas
  void reset() noexcept;
  void process(double& L, double& R, const double* E, int base, const DestructionContext& c) noexcept;
  // Latence (échantillons moteur) de l'algorithme actif (S2 : taille FFT ; autres : 0). Dry/wet aligné en interne.
  int latency() const noexcept { return latency_; }
  static int latencyFor(int type, int fftChoice) noexcept;
  double y2Env() const noexcept { return y2Env_; }
  bool frozen() const noexcept { return frozen_; }

 private:
  void switchType(int type, int fftChoice) noexcept;
  double fs_ = 48000.0;
  Quality quality_ = Quality::Normal;
  int type_ = 0, pendingType_ = 0, fftChoice_ = 2, pendingFft_ = 2, fade_ = 0, fadeLen_ = 1;
  bool fadingOut_ = false;
  int latency_ = 0;
  bool frozen_ = false;
  double y2Env_ = 0.0;
  RngStream rng_;
  std::uint64_t seed_ = 0;
  // états par algorithme (préalloués)
  struct N1State { double x1 = 0.0, tone = 0.0; double up1 = 0.0, up2 = 0.0; std::array<double, 8> hb{}; };
  std::array<N1State, 2> n1_{};
  struct D1State { double held = 0.0, prev = 0.0, phase = 0.0, next = 1.0; };
  std::array<D1State, 2> d1_{};
  struct Y1State { double env = 0.0, fast = 0.0, slow = 0.0, gain = 1.0; };
  std::array<Y1State, 2> y1_{};
  struct S1State { double phase = 0.0; double ap[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}}; double apy[4][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}}; double sinPhase = 0.0; };
  std::array<S1State, 2> s1_{};
  struct N2State { std::vector<double> buf; int pos = 0; double lp = 0.0, bp = 0.0, low = 0.0; };
  std::array<N2State, 2> n2_{};
  struct D2State { std::vector<float> buf; };
  std::array<D2State, 2> d2_{};
  int d2Write_ = 0, d2Len_ = 0, d2Remaining_ = 0, d2Count_ = 0;
  double d2Read_ = 0.0, d2SegStart_ = 0.0;
  bool d2Looping_ = false;
  struct Y2State { double fast = 0.0, slow = 0.0, hold = 0.0, rms = 0.0; };
  std::array<Y2State, 2> y2_{};
  // S2
  struct S2State {
    std::vector<double> inBuf, outBuf, dryBuf;
    std::vector<double> mag, phase, frozenMag;
    int pos = 0, hopCount = 0;
  };
  std::array<S2State, 2> s2_{};
  Fft fft_;
  std::vector<std::complex<double>> work_;
  std::vector<double> window_;
  int fftSize_ = 2048;
};

}  // namespace sbd
