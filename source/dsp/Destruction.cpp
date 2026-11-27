#include "Destruction.h"

#include <algorithm>
#include <cmath>

#include "Decimator.h"
#include "DspConstants.h"

namespace sbd {

namespace {
int gN1Mode = 1;  // 1 = ADAA1 (retenu), 0 = naïf (comparaison DST-01 uniquement)
inline double clampd(double v, double lo, double hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }
inline double dbLin(double db) noexcept { return std::pow(10.0, db / 20.0); }
inline double softSat(double x, double lim) noexcept { return lim * std::tanh(x / lim); }
inline double logCosh(double w) noexcept {
  const double a = std::fabs(w);
  return a + std::log1p(std::exp(-2.0 * a)) - 0.6931471805599453;
}
// N1 : non-linéarité NL(w) = (1-f) tanh(w) + f sin(pi/2 w) et sa primitive (nulle en 0)
inline double nl(double w, double f) noexcept { return (1.0 - f) * std::tanh(w) + f * std::sin(0.5 * kPi * w); }
inline double nlF(double w, double f) noexcept {
  return (1.0 - f) * logCosh(w) + f * (2.0 / kPi) * (1.0 - std::cos(0.5 * kPi * w));
}
// étage asymétrique : pente (1+sym) pour v >= 0, (1-sym) pour v < 0
inline double stageF(double v, double f, double sym) noexcept {
  const double a = v >= 0.0 ? 1.0 + sym : 1.0 - sym;
  return a > 1e-9 ? nlF(a * v, f) / a : 0.0;
}
inline double stageY(double v, double f, double sym) noexcept {
  const double a = v >= 0.0 ? 1.0 + sym : 1.0 - sym;
  return nl(a * v, f);
}
// Hilbert IIR : deux branches de 4 passe-tout y[n] = c (x[n] + y[n-2]) - x[n-2] (coefficients d'Olli Niemitalo)
constexpr double kHa[4] = {0.6923878, 0.9360654322959, 0.9882295226860, 0.9987488452737};
constexpr double kHb[4] = {0.4021921162426, 0.8561710882420, 0.9722909545651, 0.9952884791278};
template <typename S>
inline double allpass2(S& s, double x, double coef) noexcept {
  const double c = coef * coef;
  const double y = c * (x + s.y2) - s.x2;
  s.x2 = s.x1;
  s.x1 = x;
  s.y2 = s.y1;
  s.y1 = y;
  return y;
}
}  // namespace

void setN1AntiAliasForTest(int mode) noexcept { gN1Mode = mode; }

namespace {
const std::vector<double>& n1Halfband() {
  static const std::vector<double> h = designHalfband(0.1, 80.0);  // 4M+3 coefficients, ~80 dB
  return h;
}
}  // namespace

int n1OversamplingFor(Quality q) noexcept { return q == Quality::Eco ? 1 : (q == Quality::High ? 4 : 2); }

int n1LatencyFor(Quality q) noexcept {
  const int M = (static_cast<int>(n1Halfband().size()) - 3) / 4;
  switch (n1OversamplingFor(q)) {
    case 2: return 2 * M + 1;  // montée + descente à 2 Fs : 2 x (2M+1)/2
    case 4: return 3 * M + 2;  // + étage à 4 Fs (2M+1)/2, complété d'un demi-échantillon
    default: return 0;
  }
}

void HalfbandOs::configure(const std::vector<double>& h) {
  h_ = h;
  bu_.assign(h.size(), 0.0);
  bd_.assign(h.size(), 0.0);
  reset();
}
void HalfbandOs::reset() noexcept {
  std::fill(bu_.begin(), bu_.end(), 0.0);
  std::fill(bd_.begin(), bd_.end(), 0.0);
  pu_ = pd_ = 0;
}
double HalfbandOs::push(std::vector<double>& buf, int& pos, double x) noexcept {
  const int N = static_cast<int>(buf.size());
  buf[static_cast<size_t>(pos)] = x;
  double y = 0.0;
  int k = pos;
  for (int i = 0; i < N; ++i) {
    y += h_[static_cast<size_t>(i)] * buf[static_cast<size_t>(k)];
    k = k == 0 ? N - 1 : k - 1;
  }
  pos = (pos + 1) % N;
  return y;
}
void HalfbandOs::up(double x, double* out2) noexcept {
  out2[0] = 2.0 * push(bu_, pu_, x);
  out2[1] = 2.0 * push(bu_, pu_, 0.0);
}
double HalfbandOs::down(const double* in2) noexcept {
  push(bd_, pd_, in2[0]);
  return push(bd_, pd_, in2[1]);
}

int DestructionSlot::latencyFor(int type, int fftChoice, Quality q) noexcept {
  if (type == static_cast<int>(DstType::N1)) return n1LatencyFor(q);
  return type == static_cast<int>(DstType::S2) ? (512 << std::min(3, std::max(0, fftChoice))) : 0;
}

void DestructionSlot::configure(double fs, std::uint64_t seed, Quality q) {
  fs_ = fs;
  seed_ = seed;
  quality_ = q;
  fadeLen_ = std::max(16, static_cast<int>(std::lround(0.003 * fs)));
  for (auto& n : n2_) n.buf.assign(16384, 0.0);
  for (auto& ch : n1Os_)
    for (auto& st : ch) st.configure(n1Halfband());
  for (auto& d : n1Dry_) d.assign(256, 0.0);
  d2Len_ = static_cast<int>(std::ceil(2.05 * fs)) + 8;
  for (auto& b : d2Buf_) b.assign(static_cast<size_t>(d2Len_), 0.0f);
  for (auto& s : s2_) {
    s.in.assign(kMaxFft, 0.0);
    s.out.assign(kMaxFft, 0.0);
    s.dry.assign(kMaxFft, 0.0);
    s.mag.assign(kMaxFft / 2 + 1, 0.0);
    s.phase.assign(kMaxFft / 2 + 1, 0.0);
    s.inPhase.assign(kMaxFft / 2 + 1, 0.0);
    s.held.assign(kMaxFft / 2 + 1, 0.0);
  }
  work_.assign(kMaxFft, std::complex<double>(0.0, 0.0));
  window_.assign(kMaxFft, 0.0);
  tmp_.assign(kMaxFft / 2 + 1, 0.0);
  for (int k = 0; k < 4; ++k) ffts_[static_cast<size_t>(k)].init(512 << k);
  fft_ = &ffts_[2];
  fftSize_ = 2048;
  reset();
}

void DestructionSlot::resetStates() noexcept {
  n1_.fill(N1State{});
  for (auto& ch : n1Os_)
    for (auto& st : ch) st.reset();
  for (auto& p : n1Pad_) p.fill(0.0);
  for (auto& d : n1Dry_) std::fill(d.begin(), d.end(), 0.0);
  n1DryPos_ = 0;
  d1_.fill(D1State{});
  y1_ = DynState{};
  y2_ = DynState{};
  hil_.fill(Hilbert{});
  s1Phase_ = 0.0;
  for (auto& n : n2_) {
    std::fill(n.buf.begin(), n.buf.end(), 0.0);
    n.pos = 0;
    n.lp = n.ic1 = n.ic2 = n.dcx = n.dcy = 0.0;
  }
  for (auto& b : d2Buf_) std::fill(b.begin(), b.end(), 0.0f);
  d2Write_ = 0;
  d2Read_ = d2SegStart_ = d2SegLen_ = d2SegPos_ = d2Clock_ = 0.0;
  d2Looping_ = false;
  for (auto& s : s2_) {
    std::fill(s.in.begin(), s.in.end(), 0.0);
    std::fill(s.out.begin(), s.out.end(), 0.0);
    std::fill(s.dry.begin(), s.dry.end(), 0.0);
    std::fill(s.mag.begin(), s.mag.end(), 0.0);
    std::fill(s.phase.begin(), s.phase.end(), 0.0);
    std::fill(s.held.begin(), s.held.end(), 0.0);
    s.inPos = s.outPos = s.dryPos = s.hop = 0;
  }
  rng_.init(seed_, StreamId::Destruction);
  y2Env_ = 0.0;
  energy_ = 0.0;
  autonomous_ = false;
}

void DestructionSlot::reset() noexcept {
  resetStates();
  type_ = 0;
  pendingType_ = -1;
  fade_ = 0;
  fadingOut_ = false;
  idleSeconds_ = 0.0;
  guardFading_ = false;
  guardFade_ = 0;
}

// ---------------------------------------------------------------- N1 Asymmetric Wavefolder
double DestructionSlot::n1Stages(int ch, double u, const double* P) noexcept {
  auto& s = n1_[static_cast<size_t>(ch)];
  const double f = P[static_cast<int>(DstOff::N1Fold)], sym = 0.95 * P[static_cast<int>(DstOff::N1Sym)];
  const int stages = std::min(4, std::max(1, static_cast<int>(P[static_cast<int>(DstOff::N1Stages)] + 0.5)));
  for (int k = 0; k < stages; ++k) {
    double& pv = s.prev[static_cast<size_t>(k)];
    double y;
    if (gN1Mode == 1) {
      const double d = u - pv;
      y = std::fabs(d) > 1e-7 ? (stageF(u, f, sym) - stageF(pv, f, sym)) / d : stageY(0.5 * (u + pv), f, sym);
    } else {
      y = stageY(u, f, sym);
    }
    pv = u;
    u = k + 1 < stages ? y * (1.0 + 0.5 * f) : y;  // étages suivants : gain modéré
  }
  return u;
}

double DestructionSlot::n1(int ch, double x, const double* P) noexcept {
  auto& s = n1_[static_cast<size_t>(ch)];
  const double drive = dbLin(P[static_cast<int>(DstOff::N1Drive)]);
  const double bias = P[static_cast<int>(DstOff::N1Bias)], sym = 0.95 * P[static_cast<int>(DstOff::N1Sym)];
  const double in = x * drive + bias;
  double u;
  const int os = n1OversamplingFor(quality_);
  auto& O = n1Os_[static_cast<size_t>(ch)];
  if (os == 1) {
    u = n1Stages(ch, in, P);
  } else if (os == 2) {
    double a[2];
    O[0].up(in, a);
    a[0] = n1Stages(ch, a[0], P);
    a[1] = n1Stages(ch, a[1], P);
    u = O[0].down(a);
  } else {
    double a[2], b[4], c2[2];
    O[0].up(in, a);
    O[1].up(a[0], b);
    O[1].up(a[1], b + 2);
    auto& pad = n1Pad_[static_cast<size_t>(ch)];
    for (int i = 0; i < 4; ++i) {  // complément de 2 échantillons à 4 Fs (un demi-échantillon moteur)
      const double v = n1Stages(ch, b[i], P);
      b[i] = pad[0];
      pad[0] = pad[1];
      pad[1] = v;
    }
    c2[0] = O[1].down(b);
    c2[1] = O[1].down(b + 2);
    u = O[0].down(c2);
  }
  // tone : passe-bas 1 pôle de 200 Hz (0) à ~25 kHz (1), contourné au-delà de 0,45 Fs
  const double fc = 200.0 * std::exp2(7.0 * P[static_cast<int>(DstOff::N1Tone)]);
  if (fc < 0.45 * fs_) {
    const double a = std::exp(-kTwoPi * fc / fs_);
    s.tone = (1.0 - a) * u + a * s.tone;
  } else {
    s.tone = u;
  }
  // garde DC : nécessaire seulement si le bias ou l'asymétrie créent une composante continue (NL impaire sinon)
  if (bias == 0.0 && sym == 0.0) {
    s.dcx = s.dcy = 0.0;
    return s.tone;
  }
  const double r = std::exp(-kTwoPi * 10.0 / fs_);
  const double y = s.tone - s.dcx + r * s.dcy;
  s.dcx = s.tone;
  s.dcy = y;
  return y;
}

// ---------------------------------------------------------------- D1 Variable Rate Crusher
double DestructionSlot::d1(int ch, double x, const double* P) noexcept {
  auto& s = d1_[static_cast<size_t>(ch)];
  const double R = clampd(P[static_cast<int>(DstOff::D1Rate)], 1.0, 128.0);
  const bool controlled = P[static_cast<int>(DstOff::D1Mode)] >= 0.5, smooth = P[static_cast<int>(DstOff::D1Hold)] >= 0.5;
  double xf = x;
  if (controlled && R > 1.0) {  // pré-filtrage anti-repliement (2 pôles) à 0,45 Fs/R
    const double a = std::exp(-kTwoPi * 0.45 / R);
    s.lp1 = (1.0 - a) * xf + a * s.lp1;
    s.lp2 = (1.0 - a) * s.lp1 + a * s.lp2;
    xf = s.lp2;
  }
  s.counter += 1.0;
  if (s.counter >= s.period) {
    s.counter -= s.period;
    const double bits = clampd(P[static_cast<int>(DstOff::D1Bits)], 2.0, 16.0);
    const double levels = std::exp2(bits - 1.0);
    s.prevHeld = s.held;
    s.held = std::round(clampd(xf, -1.0, 1.0) * levels) / levels;
    const double j = P[static_cast<int>(DstOff::D1Jitter)];
    s.period = std::max(1.0, R * (1.0 + (j > 0.0 ? 0.5 * j * rng_.bipolar() : 0.0)));
    if (s.counter >= s.period) s.counter = 0.0;
  }
  return smooth ? s.prevHeld + (s.held - s.prevHeld) * (s.counter / s.period) : s.held;
}

// ---------------------------------------------------------------- Y1 Transient Destroyer
void DestructionSlot::y1(double& L, double& R, const double* P) noexcept {
  auto& s = y1_;
  const double in = std::max(std::fabs(L), std::fabs(R));
  const double at = std::exp(-1000.0 / (std::max(P[static_cast<int>(DstOff::Y1Attack)], 0.01) * fs_));
  const double rl = std::exp(-1000.0 / (std::max(P[static_cast<int>(DstOff::Y1Release)], 1.0) * fs_));
  s.env = in > s.env ? in + at * (s.env - in) : in + rl * (s.env - in);
  const double lvl = 20.0 * std::log10(std::max(s.env, 1e-9));
  const double thr = P[static_cast<int>(DstOff::Y1Thresh)], ratio = std::max(1.0, P[static_cast<int>(DstOff::Y1Ratio)]);
  const double knee = P[static_cast<int>(DstOff::Y1Knee)];
  const double over = lvl - thr, slope = 1.0 / ratio - 1.0;
  double gr;
  if (2.0 * over <= -knee)
    gr = 0.0;
  else if (knee > 0.0 && 2.0 * std::fabs(over) < knee)
    gr = slope * (over + 0.5 * knee) * (over + 0.5 * knee) / (2.0 * knee);
  else
    gr = slope * over;
  const double gthr = P[static_cast<int>(DstOff::Y1Gate)];
  const double gt = (gthr > -89.5 && lvl < gthr) ? 0.0 : 1.0;
  s.gate += (gt > s.gate ? 0.01 : 0.0005) * (gt - s.gate);
  const double fa = std::exp(-1.0 / (0.001 * fs_)), fr = std::exp(-1.0 / (0.02 * fs_));
  const double sa = std::exp(-1.0 / (0.02 * fs_)), sr = std::exp(-1.0 / (0.2 * fs_));
  s.fast = in > s.fast ? in + fa * (s.fast - in) : in + fr * (s.fast - in);
  s.slow = in > s.slow ? in + sa * (s.slow - in) : in + sr * (s.slow - in);
  const double tr = P[static_cast<int>(DstOff::Y1Transient)];
  double tg = 1.0;
  if (tr != 0.0 && s.slow > 1e-7) tg = clampd(std::pow(std::max(s.fast, 1e-9) / s.slow, 2.0 * tr), 0.1, 10.0);
  const double g = dbLin(gr + P[static_cast<int>(DstOff::Y1Makeup)]) * s.gate * tg;
  L *= g;
  R *= g;
}

// ---------------------------------------------------------------- S1 Ring / Frequency Shifter
void DestructionSlot::s1(double& L, double& R, const double* P) noexcept {
  const bool shift = P[static_cast<int>(DstOff::S1Mode)] >= 0.5;
  const double ph0 = P[static_cast<int>(DstOff::S1Phase)], st = P[static_cast<int>(DstOff::S1Stereo)];
  double* io[2] = {&L, &R};
  if (!shift) {
    const int wave = static_cast<int>(P[static_cast<int>(DstOff::S1Carrier)] + 0.5);
    for (int c = 0; c < 2; ++c) {
      double p = s1Phase_ + ph0 + (c == 1 ? st : 0.0);
      p -= std::floor(p);
      double car;
      if (wave == 1)
        car = p < 0.25 ? 4.0 * p : (p < 0.75 ? 2.0 - 4.0 * p : 4.0 * p - 4.0);
      else if (wave == 2)
        car = p < 0.5 ? 1.0 : -1.0;
      else
        car = std::sin(kTwoPi * p);
      *io[c] *= car;
    }
    s1Phase_ += P[static_cast<int>(DstOff::S1Freq)] / fs_;
  } else {
    for (int c = 0; c < 2; ++c) {
      auto& h = hil_[static_cast<size_t>(c)];
      double xa = *io[c], xb = *io[c];
      for (int k = 0; k < 4; ++k) xa = allpass2(h.a[static_cast<size_t>(k)], xa, kHa[k]);
      for (int k = 0; k < 4; ++k) xb = allpass2(h.b[static_cast<size_t>(k)], xb, kHb[k]);
      const double I = h.bDelay, Q = xb;  // la première chaîne est retardée d'un échantillon
      h.bDelay = xa;
      double p = s1Phase_ + ph0 + (c == 1 ? st : 0.0);
      p -= std::floor(p);
      *io[c] = I * std::cos(kTwoPi * p) + Q * std::sin(kTwoPi * p);  // bande latérale supérieure (+f)
    }
    s1Phase_ += P[static_cast<int>(DstOff::S1Shift)] / fs_;
  }
  s1Phase_ -= std::floor(s1Phase_);
}

// ---------------------------------------------------------------- N2 Feedback Waveshaper
double DestructionSlot::n2(int ch, double x, const double* P, bool armed) noexcept {
  auto& s = n2_[static_cast<size_t>(ch)];
  const int N = static_cast<int>(s.buf.size());
  const double D = clampd(P[static_cast<int>(DstOff::N2Delay)] * 1e-3 * fs_, 1.0, N - 2.0);
  const int di = static_cast<int>(D);
  const double fr = D - di;
  const double a = s.buf[static_cast<size_t>((s.pos - di + N) % N)], b = s.buf[static_cast<size_t>((s.pos - di - 1 + N) % N)];
  double v = a + fr * (b - a);
  const double damp = 0.97 * P[static_cast<int>(DstOff::N2Damp)];
  s.lp = (1.0 - damp) * v + damp * s.lp;
  v = s.lp;
  // résonance : accentuation passe-bande (SVF) à la fréquence de boucle
  const double res = P[static_cast<int>(DstOff::N2Res)];
  if (res > 0.0) {
    const double f0 = clampd(1000.0 / std::max(P[static_cast<int>(DstOff::N2Delay)], 0.1), 20.0, 0.4 * fs_);
    const double g = std::tan(kPi * f0 / fs_), k = 2.0 - 1.9 * res;
    const double a1 = 1.0 / (1.0 + g * (g + k)), a2 = g * a1, a3 = g * a2;
    const double v3 = v - s.ic2, v1 = a1 * s.ic1 + a2 * v3, v2 = s.ic2 + a2 * s.ic1 + a3 * v3;
    s.ic1 = 2.0 * v1 - s.ic1;
    s.ic2 = 2.0 * v2 - s.ic2;
    v = (1.0 - res) * v + res * k * v1;  // passe-bande normalisé (gain unité au centre) : gain de boucle < 1
  }
  const bool selfOsc = armed && P[static_cast<int>(DstOff::N2SelfOsc)] >= 0.5;
  const double fbMax = selfOsc ? 1.3 : 0.95;  // auto-oscillation : uniquement armée
  const double fb = fbMax * P[static_cast<int>(DstOff::N2Fb)], bias = P[static_cast<int>(DstOff::N2Bias)];
  double y = x + fb * (std::tanh(v + bias) - std::tanh(bias));
  if (std::fabs(y) > 4.0) y = softSat(y, 8.0);
  s.buf[static_cast<size_t>(s.pos)] = y;
  s.pos = (s.pos + 1) % N;
  const double r = std::exp(-kTwoPi * 10.0 / fs_);
  const double o = y - s.dcx + r * s.dcy;
  s.dcx = y;
  s.dcy = o;
  autonomous_ = selfOsc && fb > 1.0;
  return o;
}

// ---------------------------------------------------------------- D2 Glitch Buffer
void DestructionSlot::d2(double& L, double& R, const double* P, bool armed) noexcept {
  const int N = d2Len_;
  const double Ls = clampd(P[static_cast<int>(DstOff::D2Size)] * 1e-3 * fs_, 8.0, N - 8.0);
  const double speed = P[static_cast<int>(DstOff::D2Speed)];
  const double xf = std::max(1.0, P[static_cast<int>(DstOff::D2Xfade)] * 1e-3 * fs_);
  const bool freeze = P[static_cast<int>(DstOff::D2Freeze)] >= 0.5;
  autonomous_ = freeze && armed;
  double outL = L, outR = R;
  // horloge des segments
  d2Clock_ += 1.0;
  if (d2Clock_ >= Ls) {
    d2Clock_ = 0.0;
    const bool keep = freeze || rng_.uniform() < P[static_cast<int>(DstOff::D2Repeat)];
    if (!d2Looping_ && keep) {
      d2Looping_ = true;
      const double back = Ls + P[static_cast<int>(DstOff::D2Pos)] * std::max(0.0, N - 2.0 * Ls - 8.0);
      d2SegStart_ = d2Write_ - back;
      while (d2SegStart_ < 0) d2SegStart_ += N;
      d2SegLen_ = Ls;
      d2SegPos_ = 0.0;
    } else if (d2Looping_ && !keep) {
      d2Looping_ = false;
    }
  }
  if (d2Looping_) {
    const double len = d2SegLen_;
    double pos = speed >= 0.0 ? d2SegPos_ : len - 1.0 - d2SegPos_;
    double rp = d2SegStart_ + pos;
    rp -= std::floor(rp / N) * N;
    const int i0 = static_cast<int>(rp);
    const double fr = rp - i0;
    const int i1 = (i0 + 1) % N;
    const double env = std::min(1.0, std::min(d2SegPos_ / xf, (len - d2SegPos_) / xf));
    const double vl = d2Buf_[0][static_cast<size_t>(i0)] + fr * (d2Buf_[0][static_cast<size_t>(i1)] - d2Buf_[0][static_cast<size_t>(i0)]);
    const double vr = d2Buf_[1][static_cast<size_t>(i0)] + fr * (d2Buf_[1][static_cast<size_t>(i1)] - d2Buf_[1][static_cast<size_t>(i0)]);
    outL = vl * std::max(0.0, env);
    outR = vr * std::max(0.0, env);
    d2SegPos_ += std::fabs(speed) > 1e-3 ? std::fabs(speed) : 1e-3;
    if (d2SegPos_ >= len) d2SegPos_ -= len;
  }
  // enregistrement (interrompu par le gel) avec feedback de la sortie
  if (!freeze) {
    const double fb = 0.95 * P[static_cast<int>(DstOff::D2Fb)];
    d2Buf_[0][static_cast<size_t>(d2Write_)] = static_cast<float>(softSat(L + fb * (d2Looping_ ? outL : 0.0), 4.0));
    d2Buf_[1][static_cast<size_t>(d2Write_)] = static_cast<float>(softSat(R + fb * (d2Looping_ ? outR : 0.0), 4.0));
    d2Write_ = (d2Write_ + 1) % N;
  }
  L = outL;
  R = outR;
}

// ---------------------------------------------------------------- Y2 Envelope Shaper
void DestructionSlot::y2(double& L, double& R, const double* P) noexcept {
  auto& s = y2_;
  const double in = std::max(std::fabs(L), std::fabs(R));
  const bool rmsMode = P[static_cast<int>(DstOff::Y2Detect)] >= 0.5;
  double det = in;
  if (rmsMode) {
    const double a = std::exp(-1.0 / (0.01 * fs_));
    s.rms = a * s.rms + (1.0 - a) * in * in;
    det = std::sqrt(s.rms);
  }
  const double atk = std::max(P[static_cast<int>(DstOff::Y2Attack)], 0.05) * 1e-3, rel = std::max(P[static_cast<int>(DstOff::Y2Release)], 1.0) * 1e-3;
  const double fa = std::exp(-1.0 / (atk * fs_)), fr = std::exp(-1.0 / (rel * fs_));
  const double sa = std::exp(-1.0 / (10.0 * atk * fs_)), sr = std::exp(-1.0 / (2.0 * rel * fs_));
  if (det > s.fast) {
    s.fast = det + fa * (s.fast - det);
    s.hold = P[static_cast<int>(DstOff::Y2Hold)] * 1e-3 * fs_;
  } else if (s.hold > 0.0) {
    s.hold -= 1.0;
  } else {
    s.fast = det + fr * (s.fast - det);
  }
  s.slow = det > s.slow ? det + sa * (s.slow - det) : det + sr * (s.slow - det);
  const double sens = 0.25 + 7.75 * P[static_cast<int>(DstOff::Y2Sens)];
  const double tr = clampd((s.fast - s.slow) * sens / std::max(s.slow, 1e-4), 0.0, 1.0);
  const double shaped = std::pow(tr, std::exp2(-2.0 * P[static_cast<int>(DstOff::Y2Curve)]));
  y2Env_ = shaped;
  const double d = P[static_cast<int>(DstOff::Y2Depth)];
  const double g = clampd(1.0 + 2.0 * d * shaped, 0.0, 3.0);
  L *= g;
  R *= g;
}

// ---------------------------------------------------------------- S2 Spectral Freeze
void DestructionSlot::s2Frame(int ch, const double* P, bool freeze) noexcept {
  auto& s = s2_[static_cast<size_t>(ch)];
  const int N = fftSize_, H = N / 4, B = N / 2;
  for (int i = 0; i < N; ++i) work_[static_cast<size_t>(i)] = s.in[static_cast<size_t>((s.inPos + i) % N)] * window_[static_cast<size_t>(i)];
  fft_->transform(work_.data(), false);
  const double decay = P[static_cast<int>(DstOff::S2Decay)], drift = P[static_cast<int>(DstOff::S2Drift)];
  const double frameSec = H / fs_;
  for (int k = 0; k <= B; ++k) {
    const auto kk = static_cast<size_t>(k);
    const double m = std::abs(work_[kk]);
    s.inPhase[kk] = std::arg(work_[kk]);
    if (!freeze) {
      const double a = 0.98 * decay;  // traînée spectrale
      s.mag[kk] = (1.0 - a) * m + a * s.mag[kk];
      s.held[kk] = s.mag[kk];
    } else {
      const double T = 0.1 + 30.0 * decay * decay;  // demi-vie du spectre gelé
      const double r = decay >= 0.999 ? 1.0 : std::pow(0.5, frameSec / T);
      s.held[kk] *= r;
      s.mag[kk] = s.held[kk];
    }
  }
  // flou spectral
  const int it = static_cast<int>(std::lround(8.0 * P[static_cast<int>(DstOff::S2Blur)]));
  for (int n = 0; n < it; ++n) {
    for (int k = 0; k <= B; ++k) tmp_[static_cast<size_t>(k)] = s.mag[static_cast<size_t>(k)];
    for (int k = 1; k < B; ++k)
      s.mag[static_cast<size_t>(k)] = 0.25 * tmp_[static_cast<size_t>(k - 1)] + 0.5 * tmp_[static_cast<size_t>(k)] + 0.25 * tmp_[static_cast<size_t>(k + 1)];
  }
  const double tilt = P[static_cast<int>(DstOff::S2Tilt)], shift = P[static_cast<int>(DstOff::S2Shift)];
  const double ratio = std::exp2(shift / 12.0);
  const bool transparent = !freeze && drift <= 0.0 && shift == 0.0;
  for (int k = 0; k <= B; ++k) {
    const auto kk = static_cast<size_t>(k);
    double m;
    if (shift != 0.0) {
      const double src = k / ratio;
      const int i0 = static_cast<int>(src);
      m = (i0 + 1 <= B) ? s.mag[static_cast<size_t>(i0)] + (src - i0) * (s.mag[static_cast<size_t>(i0 + 1)] - s.mag[static_cast<size_t>(i0)]) : 0.0;
    } else {
      m = s.mag[kk];
    }
    if (tilt != 0.0 && k > 0) m *= std::pow(10.0, tilt * std::log2(std::max(k * fs_ / N, 10.0) / 1000.0) / 20.0);
    double ph;
    if (transparent) {
      ph = s.inPhase[kk];
    } else {
      s.phase[kk] += kTwoPi * k * H / N + drift * kPi * rng_.bipolar();
      s.phase[kk] -= kTwoPi * std::floor(s.phase[kk] / kTwoPi);
      ph = s.phase[kk];
    }
    if (transparent) s.phase[kk] = ph;
    work_[kk] = std::polar(m, ph);
  }
  for (int k = 1; k < B; ++k) work_[static_cast<size_t>(N - k)] = std::conj(work_[static_cast<size_t>(k)]);
  work_[0] = std::complex<double>(work_[0].real(), 0.0);
  work_[static_cast<size_t>(B)] = std::complex<double>(work_[static_cast<size_t>(B)].real(), 0.0);
  fft_->transform(work_.data(), true);
  // synthèse (fenêtre de Hann) et recouvrement-addition ; normalisation : somme des Hann² à 75 % = 1,5
  const double norm = 1.0 / (N * 1.5);
  for (int i = 0; i < N; ++i)
    s.out[static_cast<size_t>((s.outPos + i) % N)] += work_[static_cast<size_t>(i)].real() * window_[static_cast<size_t>(i)] * norm;
}

void DestructionSlot::s2(double& L, double& R, const double* P, bool) noexcept {
  const int N = fftSize_, H = N / 4;
  const bool freeze = P[static_cast<int>(DstOff::S2Freeze)] >= 0.5;
  double* io[2] = {&L, &R};
  for (int c = 0; c < 2; ++c) {
    auto& s = s2_[static_cast<size_t>(c)];
    s.in[static_cast<size_t>(s.inPos)] = *io[c];
    s.inPos = (s.inPos + 1) % N;
    const double y = s.out[static_cast<size_t>(s.outPos)];
    s.out[static_cast<size_t>(s.outPos)] = 0.0;
    s.outPos = (s.outPos + 1) % N;
    *io[c] = y;
  }
  if (++s2_[0].hop >= H) {
    s2_[0].hop = 0;
    // la trame couvre les N derniers échantillons ; sa synthèse commence à la position de sortie courante :
    // un échantillon d'entrée ressort exactement N échantillons plus tard (latence déclarée)
    s2Frame(0, P, freeze);
    s2Frame(1, P, freeze);
  }
}

// ---------------------------------------------------------------- captures R2
int DestructionSlot::captureInto(float* dst, int capacity, int& kind) const noexcept {
  if (type_ == static_cast<int>(DstType::D2)) {
    kind = 0;
    const int N = d2Len_;
    // segment bouclé si une boucle est active, sinon le dernier segment enregistré
    const int len = static_cast<int>(std::min<double>(d2Looping_ ? d2SegLen_ : std::max(8.0, d2SegLen_ > 0 ? d2SegLen_ : 0.25 * fs_), capacity / 2));
    const double start = d2Looping_ ? d2SegStart_ : static_cast<double>(d2Write_ - len);
    for (int i = 0; i < len; ++i) {
      double p = start + i;
      p -= std::floor(p / N) * N;
      const auto k = static_cast<size_t>(p);
      dst[2 * i] = d2Buf_[0][k];
      dst[2 * i + 1] = d2Buf_[1][k];
    }
    return len;
  }
  if (type_ == static_cast<int>(DstType::S2)) {
    kind = 1;
    const int B = fftSize_ / 2 + 1;
    if (2 * B > capacity) return 0;
    for (int ch = 0; ch < 2; ++ch)
      for (int k = 0; k < B; ++k) dst[ch * B + k] = static_cast<float>(s2_[static_cast<size_t>(ch)].held[static_cast<size_t>(k)]);
    return B;
  }
  return 0;
}

void DestructionSlot::restoreFrom(const float* src, int frames, int kind, double srcRate) noexcept {
  if (kind == 0 && frames > 0) {
    const double ratio = srcRate / fs_;
    const int len = std::min(d2Len_ - 8, static_cast<int>(frames / ratio));
    for (int i = 0; i < len; ++i) {
      const double p = i * ratio;
      const int a = std::min(static_cast<int>(p), frames - 1), b = std::min(a + 1, frames - 1);
      const double f = p - a;
      for (int ch = 0; ch < 2; ++ch)
        d2Buf_[static_cast<size_t>(ch)][static_cast<size_t>(i)] = static_cast<float>(src[2 * a + ch] + f * (src[2 * b + ch] - src[2 * a + ch]));
    }
    d2SegStart_ = 0.0;
    d2SegLen_ = len;
    d2SegPos_ = 0.0;
    d2Write_ = len % d2Len_;
    d2Looping_ = true;  // le contenu restauré est le segment courant ; sa persistance reste soumise au Freeze armé
  } else if (kind == 1 && frames > 1) {
    const int B = fftSize_ / 2 + 1;
    const double srcN = 2.0 * (frames - 1);
    for (int ch = 0; ch < 2; ++ch)
      for (int k = 0; k < B; ++k) {
        const double hz = k * fs_ / fftSize_;
        const double sp = hz * srcN / srcRate;  // bin source de même fréquence
        const int a = std::min(static_cast<int>(sp), frames - 1), b = std::min(a + 1, frames - 1);
        const double f = std::min(1.0, sp - a);
        const double m = src[ch * frames + a] + f * (src[ch * frames + b] - src[ch * frames + a]);
        s2_[static_cast<size_t>(ch)].held[static_cast<size_t>(k)] = m * fftSize_ / srcN;  // normalisation de taille
        s2_[static_cast<size_t>(ch)].mag[static_cast<size_t>(k)] = s2_[static_cast<size_t>(ch)].held[static_cast<size_t>(k)];
      }
  }
}

// ---------------------------------------------------------------- traitement commun
void DestructionSlot::process(double& L, double& R, const double* E, int base, const DestructionContext& c) noexcept {
  const double* P = E + base;
  const int want = std::min(8, std::max(0, static_cast<int>(P[static_cast<int>(DstOff::Type)] + 0.5)));
  const int wantFft = std::min(3, std::max(0, static_cast<int>(P[static_cast<int>(DstOff::S2Fft)] + 0.5)));
  const bool fftChange = type_ == static_cast<int>(DstType::S2) && want == type_ && wantFft != fftChoice_;
  if ((want != type_ || fftChange) && pendingType_ < 0) {  // changement structurel : fondu, remise à zéro, fondu
    pendingType_ = want;
    fadingOut_ = type_ != 0;
    fade_ = fadingOut_ ? fadeLen_ : 0;
  }
  if (pendingType_ >= 0 && !fadingOut_) {
    resetStates();
    type_ = pendingType_;
    pendingType_ = -1;
    fade_ = fadeLen_;
    if (type_ == static_cast<int>(DstType::S2)) {
      fftChoice_ = wantFft;
      fftSize_ = 512 << fftChoice_;
      fft_ = &ffts_[static_cast<size_t>(fftChoice_)];  // plan préparé hors RT (aucune allocation)
      for (int i = 0; i < fftSize_; ++i) window_[static_cast<size_t>(i)] = 0.5 - 0.5 * std::cos(kTwoPi * i / fftSize_);
    }
  }
  if (type_ == 0) return;
  const bool armed = P[static_cast<int>(DstOff::Arm)] >= 0.5;
  idleSeconds_ = c.voiceIdle ? idleSeconds_ + 1.0 / fs_ : 0.0;
  if (!armed && idleSeconds_ > 10.0 && energy_ > 1e-7 && !guardFading_) {  // garde-fou Tail des états non armés
    guardFading_ = true;
    guardFade_ = fadeLen_;
  }
  double dryL = L, dryR = R, wL = L, wR = R;
  switch (static_cast<DstType>(type_)) {
    case DstType::N1: {
      wL = n1(0, L, P);
      wR = n1(1, R, P);
      const int lat = n1LatencyFor(quality_);
      if (lat > 0) {  // chemin sec retardé de la latence du suréchantillonnage (alignement dry/wet)
        const int N = static_cast<int>(n1Dry_[0].size());
        n1Dry_[0][static_cast<size_t>(n1DryPos_)] = dryL;
        n1Dry_[1][static_cast<size_t>(n1DryPos_)] = dryR;
        dryL = n1Dry_[0][static_cast<size_t>((n1DryPos_ - lat + N) % N)];
        dryR = n1Dry_[1][static_cast<size_t>((n1DryPos_ - lat + N) % N)];
        n1DryPos_ = (n1DryPos_ + 1) % N;
      }
      break;
    }
    case DstType::D1:
      wL = d1(0, L, P);
      wR = d1(1, R, P);
      break;
    case DstType::Y1: y1(wL, wR, P); break;
    case DstType::S1: s1(wL, wR, P); break;
    case DstType::N2:
      wL = n2(0, L, P, armed);
      wR = n2(1, R, P, armed);
      break;
    case DstType::D2: d2(wL, wR, P, armed); break;
    case DstType::Y2: y2(wL, wR, P); break;
    case DstType::S2: {
      s2(wL, wR, P, armed);
      autonomous_ = P[static_cast<int>(DstOff::S2Freeze)] >= 0.5 && armed;
      // chemin sec retardé de la latence de l'algorithme (alignement dry/wet)
      for (int ch = 0; ch < 2; ++ch) {
        auto& s = s2_[static_cast<size_t>(ch)];
        double& d = ch == 0 ? dryL : dryR;
        const double v = s.dry[static_cast<size_t>(s.dryPos)];
        s.dry[static_cast<size_t>(s.dryPos)] = d;
        s.dryPos = (s.dryPos + 1) % fftSize_;
        d = v;
      }
      break;
    }
    case DstType::Off: break;
  }
  if (!std::isfinite(wL) || !std::isfinite(wR)) {  // état numérique dangereux : purge locale
    resetStates();
    wL = wR = 0.0;
  }
  energy_ += 0.001 * (0.5 * (wL * wL + wR * wR) - energy_);
  const double mix = P[static_cast<int>(DstOff::Mix)], og = dbLin(P[static_cast<int>(DstOff::Out)]);
  double wet = 1.0;
  if (fadingOut_) {
    wet = static_cast<double>(fade_) / fadeLen_;
    if (--fade_ <= 0) fadingOut_ = false;
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
