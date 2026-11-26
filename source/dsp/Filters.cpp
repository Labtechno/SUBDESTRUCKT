#include "Filters.h"

#include <algorithm>
#include <cmath>

#include "DspConstants.h"

namespace sbd {

namespace {
inline double clampd(double v, double lo, double hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }
inline double softSat(double x, double lim) noexcept { return lim * std::tanh(x / lim); }

// Barre libre-libre (métal) et membrane circulaire (zéros de Bessel), normalisés au premier mode.
constexpr double kMetal[16] = {1.0,   2.756, 5.404, 8.933, 13.344, 18.638, 24.815, 31.873,
                               39.81, 48.63, 58.33, 68.91, 80.37,  92.71,  105.93, 120.04};
constexpr double kMembrane[16] = {1.0,   1.594, 2.136, 2.296, 2.653, 2.918, 3.156, 3.501,
                                  3.600, 3.652, 4.060, 4.154, 4.230, 4.601, 4.832, 4.903};
}  // namespace

double modalRatio(int model, int k) noexcept {
  k = std::min(std::max(k, 1), 16);
  switch (model) {
    case 1: return kMetal[k - 1];
    case 2: return kMembrane[k - 1];
    case 3: return std::pow(static_cast<double>(k), 1.37);  // Inharmonic : série étirée
    default: return static_cast<double>(k);                 // Harmonic
  }
}

void FilterSlot::configure(double fs) {
  fs_ = fs;
  for (auto& c : comb_) c.assign(kCombMax, 0.0);
  fadeLen_ = std::max(16, static_cast<int>(std::lround(0.002 * fs)));
  reset();
}

void FilterSlot::resetStates() noexcept {
  for (auto& ch : svf_)
    for (auto& s : ch) s = Svf{};
  for (auto& c : comb_) std::fill(c.begin(), c.end(), 0.0);
  combPos_.fill(0);
  combLp_.fill(0.0);
  for (auto& ch : modes_)
    for (auto& m : ch) m = Res2{};
  excState_.fill(0.0);
  energy_ = 0.0;
  autonomous_ = false;
  modalKey_ = -1.0;
}

void FilterSlot::reset() noexcept {
  resetStates();
  type_ = 0;
  pendingType_ = -1;
  fade_ = 0;
  fadingOut_ = false;
  idleSeconds_ = 0.0;
  guardFading_ = false;
  guardFade_ = 0;
}

double FilterSlot::svf(Svf& s, double x, double g, double k, int out, bool saturate, double drive, double bias,
                       double chr) noexcept {
  if (saturate) x = std::tanh(drive * x + bias) - std::tanh(bias);
  const double a1 = 1.0 / (1.0 + g * (g + k)), a2 = g * a1, a3 = g * a2;
  const double v3 = x - s.ic2;
  double v1 = a1 * s.ic1 + a2 * v3;
  const double v2 = s.ic2 + a2 * s.ic1 + a3 * v3;
  s.ic1 = 2.0 * v1 - s.ic1;
  s.ic2 = 2.0 * v2 - s.ic2;
  if (saturate && chr > 0.0) s.ic1 = (1.0 - chr) * s.ic1 + chr * softSat(s.ic1, 1.5);  // saturation dans la boucle
  // borne de l'auto-oscillation (résonance extrême) : saturation douce de l'état passe-bande
  if (std::fabs(s.ic1) > 4.0) s.ic1 = softSat(s.ic1, 8.0);
  if (std::fabs(s.ic2) > 8.0) s.ic2 = softSat(s.ic2, 16.0);
  switch (out) {
    case 0: return v2;                       // LP
    case 1: return v1;                       // BP (gain unité au centre pour k=1 ; normalisé ci-dessous)
    case 2: return x - k * v1 - v2;          // HP
    default: return x - k * v1;              // Notch = LP + HP
  }
}

double FilterSlot::comb(int ch, double x, double delay, double fb, double damp) noexcept {
  auto& buf = comb_[static_cast<size_t>(ch)];
  const int N = kCombMax;
  delay = clampd(delay, 2.0, N - 4.0);
  const int base = static_cast<int>(std::floor(delay)) - 1;
  const double t = delay - base;  // [1,2)
  const double h0 = -(t - 1.0) * (t - 2.0) * (t - 3.0) / 6.0, h1 = t * (t - 2.0) * (t - 3.0) / 2.0;
  const double h2 = -t * (t - 1.0) * (t - 3.0) / 2.0, h3 = t * (t - 1.0) * (t - 2.0) / 6.0;
  const int p = combPos_[static_cast<size_t>(ch)];
  auto at = [&](int j) { return buf[static_cast<size_t>((p - base - j + 2 * N) % N)]; };
  const double v = h0 * at(0) + h1 * at(1) + h2 * at(2) + h3 * at(3);
  double& lp = combLp_[static_cast<size_t>(ch)];
  lp = (1.0 - damp) * v + damp * lp;
  double y = x + fb * lp;
  if (std::fabs(y) > 8.0) y = softSat(y, 16.0);
  buf[static_cast<size_t>(p)] = y;
  combPos_[static_cast<size_t>(ch)] = (p + 1) % N;
  return y;
}

void FilterSlot::updateModal(const double* E, int base, double f0) noexcept {
  auto P = [&](FltOff o) { return E[base + static_cast<int>(o)]; };
  const int model = static_cast<int>(P(FltOff::Modal) + 0.5);
  const int n = std::min(kMaxModes, std::max(1, static_cast<int>(P(FltOff::Modes) + 0.5)));
  const double decay = P(FltOff::Decay), damp = P(FltOff::Damp), bright = P(FltOff::Bright), inh = P(FltOff::Inharm);
  const bool freeze = P(FltOff::Freeze) >= 0.5 && P(FltOff::Arm) >= 0.5;  // Freeze : état autonome armé seulement
  const double key = f0 * 7.0 + decay * 3.0 + damp * 5.0 + bright * 11.0 + inh * 13.0 + model * 17.0 + n * 19.0 + (freeze ? 23.0 : 0.0);
  if (key == modalKey_) return;
  modalKey_ = key;
  numModes_ = n;
  double norm = 0.0;
  for (int k = 1; k <= n; ++k) {
    const double B = 0.0005 * inh;
    const double f = f0 * modalRatio(model, k) * std::sqrt(1.0 + B * k * k);
    const auto i = static_cast<size_t>(k - 1);
    modeFreq_[i] = f;
    if (f >= 0.45 * fs_) {  // mode au-dessus de la limite : muet
      modeA1_[i] = modeA2_[i] = modeG_[i] = 0.0;
      continue;
    }
    const double t60 = decay / (1.0 + damp * 8.0 * (k - 1) / 15.0);
    const double r = freeze ? 1.0 : std::exp(-6.907755 / (std::max(t60, 1e-3) * fs_));
    const double th = kTwoPi * f / fs_;
    modeA1_[i] = 2.0 * r * std::cos(th);
    modeA2_[i] = r * r;
    const double amp = std::pow(1.0 / k, 2.0 * (1.0 - bright));
    modeG_[i] = freeze ? 0.0 : (1.0 - r) * 2.0 * amp;
    norm += amp * amp;
  }
  const double s = norm > 0.0 ? 1.0 / std::sqrt(norm) : 1.0;
  for (int k = 0; k < n; ++k) modeG_[static_cast<size_t>(k)] *= s;
}

void FilterSlot::process(double& L, double& R, const double* E, int base, const FilterContext& c) noexcept {
  auto P = [&](FltOff o) { return E[base + static_cast<int>(o)]; };
  const int want = std::min(4, std::max(0, static_cast<int>(P(FltOff::Type) + 0.5)));
  if (want != type_ && pendingType_ != want) {  // changement structurel : fondu de sortie, remise à zéro, fondu d'entrée
    pendingType_ = want;
    fadingOut_ = type_ != 0;
    fade_ = fadingOut_ ? fadeLen_ : 0;
  }
  if (pendingType_ >= 0 && !fadingOut_) {
    resetStates();
    type_ = pendingType_;
    pendingType_ = -1;
    fade_ = fadeLen_;  // fondu d'entrée
  }
  if (type_ == 0) return;
  const bool armed = P(FltOff::Arm) >= 0.5;
  // garde-fou temporel des états non armés (Tail) : 10 s après la fin de la voix
  idleSeconds_ = c.voiceIdle ? idleSeconds_ + 1.0 / fs_ : 0.0;
  if (!armed && idleSeconds_ > 10.0 && energy_ > 1e-7 && !guardFading_) {
    guardFading_ = true;
    guardFade_ = fadeLen_;
  }
  const double dryL = L, dryR = R;
  double xL = L, xR = R;
  if (c.excite) {
    const double ex = P(FltOff::ExcLevel);
    xL += ex;
    xR += ex;
  }
  const bool dual = P(FltOff::Stereo) >= 0.5;
  const double keyF = std::pow(std::max(c.noteHz, 1.0) / 261.6255653005986, P(FltOff::Key));
  const double fc = P(FltOff::Cutoff) * keyF;
  const double spread = dual ? P(FltOff::Spread) : 0.0;
  const double fcL = clampd(fc * std::exp2(0.5 * spread), 10.0, 0.45 * fs_), fcR = clampd(fc * std::exp2(-0.5 * spread), 10.0, 0.45 * fs_);
  double wL = 0.0, wR = 0.0;
  switch (type_) {
    case 1:
    case 3: {
      const bool nl = type_ == 3;
      const double res = P(FltOff::Res);
      const double k = 1.41421356237 * (1.0 - res);
      int out = 0;
      bool four = false;
      if (!nl) {
        const int m = static_cast<int>(P(FltOff::F1Mode) + 0.5);
        static constexpr int kOut[7] = {0, 0, 1, 1, 2, 2, 3};
        out = kOut[std::min(6, std::max(0, m))];
        four = m == 1 || m == 3 || m == 5;
      } else {
        out = std::min(2, std::max(0, static_cast<int>(P(FltOff::F3Mode) + 0.5)));
      }
      const double drive = std::pow(10.0, P(FltOff::Drive) / 20.0), bias = P(FltOff::Bias), chr = P(FltOff::Char);
      const double gL = std::tan(kPi * fcL / fs_), gR = dual ? std::tan(kPi * fcR / fs_) : gL;
      const double bpNorm = out == 1 ? k : 1.0;  // BP : gain unité au centre
      wL = svf(svf_[0][0], xL, gL, k, out, nl, drive, bias, chr) * bpNorm;
      wR = svf(svf_[1][0], xR, gR, k, out, nl, drive, bias, chr) * bpNorm;
      if (four) {
        wL = svf(svf_[0][1], wL, gL, k, out, false, 1.0, 0.0, 0.0) * bpNorm;
        wR = svf(svf_[1][1], wR, gR, k, out, false, 1.0, 0.0, 0.0) * bpNorm;
      }
      if (nl) {  // compensation approximative du drive
        const double comp = 1.0 / std::max(1.0, std::sqrt(drive));
        wL *= comp;
        wR *= comp;
      }
      autonomous_ = res > 0.97 && energy_ > 1e-6;
      break;
    }
    case 2: {
      const double fb = clampd(P(FltOff::CombFb), -0.999, 0.999);
      const double damp = 0.95 * P(FltOff::CombDamp);
      const double det = dual ? P(FltOff::Detune) / 1200.0 : 0.0;
      auto loopDelay = [&](double f) {
        const double w = kTwoPi * f / fs_;
        const double tau = std::atan2(damp * std::sin(w), 1.0 - damp * std::cos(w)) / w;  // délai de phase du passe-bas
        return fs_ / f - tau;
      };
      const double dL = loopDelay(fcL * std::exp2(0.5 * det)), dR = loopDelay(fcR * std::exp2(-0.5 * det));
      const double norm = std::sqrt(1.0 - std::fabs(fb));
      wL = comb(0, xL, dL, fb, damp) * norm;
      wR = comb(1, xR, dR, fb, damp) * norm;
      autonomous_ = std::fabs(fb) > 0.99 && energy_ > 1e-6;
      break;
    }
    case 4: {
      if (modalCounter_ <= 0) {
        updateModal(E, base, fcL);
        modalCounter_ = 16;  // coefficients modaux à 1/16 du rythme moteur
      }
      --modalCounter_;
      double sL = 0.0, sR = 0.0;
      for (int k = 0; k < numModes_; ++k) {
        const auto i = static_cast<size_t>(k);
        auto& a = modes_[0][i];
        auto& b = modes_[1][i];
        const double yl = modeA1_[i] * a.y1 - modeA2_[i] * a.y2 + modeG_[i] * xL;
        const double yr = modeA1_[i] * b.y1 - modeA2_[i] * b.y2 + modeG_[i] * xR;
        a.y2 = a.y1;
        a.y1 = std::fabs(yl) > 8.0 ? softSat(yl, 16.0) : yl;
        b.y2 = b.y1;
        b.y1 = std::fabs(yr) > 8.0 ? softSat(yr, 16.0) : yr;
        sL += a.y1;
        sR += b.y1;
      }
      wL = sL;
      wR = sR;
      autonomous_ = P(FltOff::Freeze) >= 0.5 && armed;
      break;
    }
    default: break;
  }
  energy_ += 0.001 * (0.5 * (wL * wL + wR * wR) - energy_);
  const double mix = P(FltOff::Mix), og = std::pow(10.0, P(FltOff::Out) / 20.0);
  double wet = 1.0;
  if (fadingOut_) {
    wet = static_cast<double>(fade_) / fadeLen_;
    if (--fade_ <= 0) fadingOut_ = false;  // bascule au prochain échantillon
  } else if (fade_ > 0) {
    wet = 1.0 - static_cast<double>(fade_) / fadeLen_;
    --fade_;
  }
  if (guardFading_) {
    wet *= static_cast<double>(guardFade_) / fadeLen_;
    if (--guardFade_ <= 0) {
      resetStates();
      guardFading_ = false;
      idleSeconds_ = 0.0;
    }
  }
  const double yL = (dryL + mix * (wL - dryL)) * og, yR = (dryR + mix * (wR - dryR)) * og;
  L = dryL + wet * (yL - dryL);
  R = dryR + wet * (yR - dryR);
}

}  // namespace sbd
