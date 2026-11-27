// SUBDESTRUCKT — tests ARCH-G4 du Destruction Engine (DST-01..10). Seuils : docs/V1-ARCH-G4-TRACKER.md §3.
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include "Analysis.h"
#include "G4Helper.h"
#include "TestKit.h"
#include "dsp/Destruction.h"

using namespace sbd;
using namespace g4;

namespace {

struct DSlot {
  ParamValues p = ParamValues::defaults();
  DestructionSlot d;
  int base = idx(ParamId::DstGl1Type);
  std::array<double, kNumParams> E{};
  double fs;
  DestructionContext c;
  // Quality Eco par défaut : N1 sans suréchantillonnage interne = configuration mesurée par DST-01 historique
  explicit DSlot(double fs_, int type, Quality q = Quality::Eco) : fs(fs_) {
    d.configure(fs_, 1234, q);
    set(DstOff::Type, type);
    c.fs = fs_;
  }
  void set(DstOff o, double v) { p.v[static_cast<size_t>(base + static_cast<int>(o))] = static_cast<float>(v); }
  void sync() {
    for (int i = 0; i < kNumParams; ++i) E[static_cast<size_t>(i)] = p.v[static_cast<size_t>(i)];
  }
  // traite x (mono dupliqué) après un préchauffage silencieux couvrant le fondu d'entrée
  std::vector<double> run(const std::vector<double>& x, std::vector<double>* right = nullptr) {
    sync();
    for (int i = 0; i < 4096; ++i) {
      double l = 0.0, r = 0.0;
      d.process(l, r, E.data(), base, c);
    }
    std::vector<double> y(x.size());
    if (right) right->resize(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
      double l = x[i], r = x[i];
      d.process(l, r, E.data(), base, c);
      y[i] = l;
      if (right) (*right)[i] = r;
    }
    return y;
  }
};

std::vector<double> sine(double fs, double f, double a, size_t n) {
  std::vector<double> x(n);
  for (size_t i = 0; i < n; ++i) x[i] = a * std::sin(kTwoPi * f * i / fs);
  return x;
}
double thd(const std::vector<double>& y, double fs, double f0, size_t from = 8192) {
  std::vector<double> s(y.begin() + static_cast<long>(from), y.begin() + static_cast<long>(from + 32768));
  const auto m = an::amplitudeSpectrum(s.data(), s.size());
  const double bin = fs / 32768.0, a0 = an::lineAmplitude(m, bin, f0);
  double h = 0.0;
  for (int k = 2; k * f0 < 0.45 * fs; ++k) h += std::pow(an::lineAmplitude(m, bin, f0 * k), 2.0);
  return std::sqrt(h) / std::max(a0, 1e-12);
}
double nmse(const std::vector<double>& a, const std::vector<double>& ref, size_t from, size_t to) {
  double n = 0.0, d = 0.0;
  for (size_t i = from; i < to; ++i) {
    n += (a[i] - ref[i]) * (a[i] - ref[i]);
    d += ref[i] * ref[i];
  }
  return 10.0 * std::log10((n + 1e-300) / (d + 1e-300));
}
double levelDb(const std::vector<double>& y, size_t from, size_t to) {
  double p = 0.0;
  for (size_t i = from; i < to; ++i) p = std::max(p, std::fabs(y[i]));
  return 20.0 * std::log10(std::max(p, 1e-12));
}

void testN1() {
  const double fs = 48000.0;
  // neutralité : fold 0, drive 0, entrée -20 dBFS, référence retardée d'un demi-échantillon
  DSlot s(fs, 1);
  s.set(DstOff::N1Fold, 0.0);
  s.set(DstOff::N1Drive, 0.0);
  s.set(DstOff::N1Tone, 1.0);
  const auto x = sine(fs, 1000.0, 0.1, 48000);
  const auto y = s.run(x);
  const auto xh = sine(fs, 1000.0, 0.1, 48000);
  std::vector<double> ref(x.size());
  for (size_t i = 1; i < x.size(); ++i) ref[i] = 0.5 * (xh[i] + xh[i - 1]);
  const double neutral = nmse(y, ref, 8192, 48000);
  // THD croissante avec le drive
  auto th = [&](double drive) {
    DSlot t(fs, 1);
    t.set(DstOff::N1Drive, drive);
    t.set(DstOff::N1Fold, 0.5);
    t.set(DstOff::N1Tone, 1.0);
    return thd(t.run(sine(fs, 187.5, 0.3, 48000)), fs, 187.5);
  };
  const double t0 = th(0.0), t12 = th(12.0), t24 = th(24.0);
  // comparaison anti-repliement : sinus 3,1 kHz (fs n'est pas multiple de f : les repliements tombent hors grille)
  auto alias = [&](int mode, double fsx, double& ns) {
    setN1AntiAliasForTest(mode);
    DSlot t(fsx, 1);
    t.set(DstOff::N1Drive, 24.0);
    t.set(DstOff::N1Fold, 1.0);
    t.set(DstOff::N1Tone, 1.0);
    const auto in = sine(fsx, 3100.0, 0.8, 65536);
    const auto t0c = std::chrono::steady_clock::now();
    auto out = t.run(in);
    ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0c).count() / out.size();
    // décimation par 2 si K x 2 (moyenne de deux échantillons : approximation suffisante pour la mesure relative)
    std::vector<double> ev;
    if (fsx > fs) {
      for (size_t i = 0; i + 1 < out.size(); i += 2) ev.push_back(0.5 * (out[i] + out[i + 1]));
    } else {
      ev = out;
    }
    std::vector<double> seg(ev.begin() + 8192, ev.begin() + 8192 + 16384);
    const auto m = an::amplitudeSpectrum(seg.data(), seg.size());
    return an::offGridEnergyDb(m, fs / 16384.0, 3100.0, 20.0, 0.45 * fs, 6);
  };
  double nsNaive, nsAdaa, nsOs;
  const double aNaive = alias(0, fs, nsNaive), aAdaa = alias(1, fs, nsAdaa), aOs = alias(1, 2.0 * fs, nsOs);
  setN1AntiAliasForTest(1);
  const char* choice = aNaive <= -40.0 ? "naif" : (aAdaa <= -40.0 ? "ADAA1" : (aOs <= -40.0 ? "ADAA1 + Kx2 (Quality High)" : "aucun sous -40 dB : ADAA1 + Kx2 le plus bas"));
  const bool ok = neutral <= -40.0 && t0 < t12 && t12 < t24;
  TK_CHECK(ok);
  tk::result("DST-01", ok, "N1 neutre (fold 0, drive 0) : NMSE %.1f dB vs entree retardee 1/2 ech. (<= -40) ; THD drive 0/12/24 : %.3f < %.3f < %.3f ; repliement (3,1 kHz, drive 24, fold 1) naif %.1f dB (%.0f ns/ech), ADAA1 %.1f dB (%.0f ns), ADAA1+Kx2 %.1f dB (%.0f ns) ; retenu : %s",
             neutral, t0, t12, t24, aNaive, nsNaive, aAdaa, nsAdaa, aOs, nsOs, choice);
}

void testD1() {
  const double fs = 48000.0;
  DSlot a(fs, 2);
  a.set(DstOff::D1Bits, 16.0);
  a.set(DstOff::D1Rate, 1.0);
  const auto x = sine(fs, 997.0, 0.9, 32768);
  const double n16 = nmse(a.run(x), x, 0, x.size());
  DSlot b(fs, 2);
  b.set(DstOff::D1Bits, 4.0);
  b.set(DstOff::D1Rate, 1.0);
  const auto x1 = sine(fs, 997.0, 1.0, 32768);
  const auto y4 = b.run(x1);
  const double snr4 = -nmse(y4, x1, 0, x1.size());
  DSlot c(fs, 2);
  c.set(DstOff::D1Bits, 16.0);
  c.set(DstOff::D1Rate, 4.0);
  c.set(DstOff::D1Mode, 0.0);  // Raw
  c.set(DstOff::D1Hold, 0.0);  // Step
  const auto y = c.run(sine(fs, 100.0, 0.9, 4096));
  bool steps = true;
  int changes = 0;
  size_t last = 0;
  for (size_t i = 1; i < y.size(); ++i)
    if (y[i] != y[i - 1]) {
      if (changes > 0) steps = steps && (i - last) % 4 == 0;
      last = i;
      ++changes;
    }
  steps = steps && changes > 100;
  const bool ok = n16 <= -80.0 && std::fabs(snr4 - 25.8) <= 3.0 && steps;
  TK_CHECK(ok);
  tk::result("DST-02", ok, "D1 16 bits ratio 1 : NMSE %.1f dB (<= -80) ; 4 bits : SNR %.1f dB (25,8 +-3) ; ratio 4 Raw/Step : paliers de 4 echantillons %d", n16, snr4, steps);
}

void testY1() {
  const double fs = 48000.0;
  DSlot s(fs, 3);
  s.set(DstOff::Y1Thresh, -20.0);
  s.set(DstOff::Y1Ratio, 4.0);
  s.set(DstOff::Y1Knee, 0.0);
  s.set(DstOff::Y1Attack, 1.0);
  s.set(DstOff::Y1Release, 100.0);
  const auto y = s.run(sine(fs, 1000.0, 0.5, 48000));
  const double out = levelDb(y, 24000, 48000);
  DSlot g(fs, 3);
  g.set(DstOff::Y1Gate, -30.0);
  g.set(DstOff::Y1Thresh, 0.0);
  const auto x = sine(fs, 1000.0, std::pow(10.0, -50.0 / 20.0), 96000);
  const auto yg = g.run(x);
  const double att = levelDb(x, 48000, 96000) - levelDb(yg, 48000, 96000);
  auto burst = [&](double tr) {
    DSlot t(fs, 3);
    t.set(DstOff::Y1Thresh, 0.0);
    t.set(DstOff::Y1Transient, tr);
    std::vector<double> in(48000, 0.0);
    for (size_t i = 12000; i < 36000; ++i) in[i] = 0.5 * std::sin(kTwoPi * 500.0 * i / fs);
    const auto o = t.run(in);
    return levelDb(o, 12000, 12000 + 240) - levelDb(o, 30000, 36000);
  };
  const double r0 = burst(0.0), r1 = burst(1.0);
  const bool ok = std::fabs(out + 16.5) <= 0.5 && att >= 40.0 && r1 > r0;
  TK_CHECK(ok);
  tk::result("DST-03", ok, "Y1 : -6 dBFS seuil -20 ratio 4 -> %.2f dBFS (-16,5 +-0,5) ; gate -30 dB sur -50 dBFS : attenuation %.1f dB (>= 40) ; transient +1 : attaque/sustain %.1f dB > %.1f dB", out, att, r1, r0);
}

void testS1() {
  const double fs = 48000.0;
  auto spec = [&](bool shift) {
    DSlot s(fs, 4);
    s.set(DstOff::S1Mode, shift ? 1.0 : 0.0);
    s.set(DstOff::S1Freq, 100.0);
    s.set(DstOff::S1Shift, 100.0);
    const auto y = s.run(sine(fs, 1000.0, 0.5, 8192 + 65536));
    std::vector<double> seg(y.begin() + 8192, y.end());
    return an::amplitudeSpectrum(seg.data(), seg.size());
  };
  const double bin = fs / 65536.0;
  const auto r = spec(false), sh = spec(true);
  auto db = [&](const std::vector<double>& m, double f) { return 20.0 * std::log10(an::lineAmplitude(m, bin, f) + 1e-12); };
  const double ringCar = db(r, 1000.0) - std::max(db(r, 900.0), db(r, 1100.0));
  const double ringLines = std::fabs(db(r, 900.0) - db(r, 1100.0));
  const double image = db(sh, 900.0) - db(sh, 1100.0);
  const bool ok = ringCar <= -40.0 && ringLines < 1.0 && image <= -30.0;
  TK_CHECK(ok);
  tk::result("DST-04", ok, "S1 Ring 100 Hz sur 1 kHz : porteuse %.1f dB (<= -40), raies 900/1100 equilibrees (%.2f dB) ; Shift +100 Hz : image 900 Hz %.1f dB (<= -30)", ringCar, ringLines, image);
}

void testN2() {
  for (int cfg = 0; cfg < 5; ++cfg) {  // diagnostic de décroissance (configurations isolées)
    DSlot s(48000.0, 5);
    s.set(DstOff::N2Fb, 1.0);
    s.set(DstOff::N2Delay, 5.0);
    s.set(DstOff::N2Damp, cfg == 1 ? 0.5 : 0.0);
    s.set(DstOff::N2Res, cfg == 2 ? 0.9 : 0.0);
    s.set(DstOff::N2Bias, cfg == 3 ? 0.7 : 0.0);
    s.set(DstOff::N2SelfOsc, cfg == 4 ? 1.0 : 0.0);
    std::vector<double> x(48000 * 4, 0.0);
    x[100] = 0.5;
    const auto y = s.run(x);
    tk::info("DST-05 diagnostic cfg %d : crete par seconde %.3g %.3g %.3g %.3g", cfg, std::pow(10.0, levelDb(y, 0, 48000) / 20),
             std::pow(10.0, levelDb(y, 48000, 96000) / 20), std::pow(10.0, levelDb(y, 96000, 144000) / 20), std::pow(10.0, levelDb(y, 144000, 192000) / 20));
  }
  std::mt19937_64 rng(0x2B2BULL);
  int bad = 0;
  double unarmedPeak = -400.0;
  for (int k = 0; k < 200; ++k) {
    DSlot s(48000.0, 5);
    std::uniform_real_distribution<double> u(0.0, 1.0);
    s.set(DstOff::N2Fb, 1.0);
    s.set(DstOff::N2Delay, 0.1 + 49.9 * u(rng));
    s.set(DstOff::N2Damp, u(rng));
    s.set(DstOff::N2Bias, 2.0 * u(rng) - 1.0);
    s.set(DstOff::N2Res, u(rng));
    s.set(DstOff::N2SelfOsc, 1.0);
    s.set(DstOff::Arm, 0.0);
    std::vector<double> x(static_cast<size_t>(48000 * 12), 0.0);
    for (size_t i = 0; i < 2400; ++i) x[i] = 2.0 * u(rng) - 1.0;
    const auto y = s.run(x);
    for (double v : y)
      if (!std::isfinite(v) || std::fabs(v) > 16.0) {
        ++bad;
        break;
      }
    unarmedPeak = std::max(unarmedPeak, levelDb(y, 48000 * 11, 48000 * 12));  // queue après 11 s : décroissance
  }
  // armé : l'auto-oscillation persiste
  DSlot a(48000.0, 5);
  a.set(DstOff::N2Fb, 1.0);
  a.set(DstOff::N2Delay, 5.0);
  a.set(DstOff::N2SelfOsc, 1.0);
  a.set(DstOff::Arm, 1.0);
  std::vector<double> x(96000, 0.0);
  x[0] = 0.5;
  const auto ya = a.run(x);
  const double armedTail = levelDb(ya, 90000, 96000);
  const bool ok = bad == 0 && unarmedPeak < -60.0 && armedTail > -40.0;
  TK_CHECK(ok);
  tk::result("DST-05", ok, "N2 : 200 configurations fb max non armees : %d en defaut, queue a 11-12 s %.1f dBFS (< -60, pas d'auto-oscillation) ; armee : %.1f dBFS apres 1,9 s (persistance)", bad, unarmedPeak, armedTail);
}

void testD2() {
  const double fs = 48000.0;
  DSlot s(fs, 6);
  s.set(DstOff::D2Size, 50.0);  // 2400 échantillons
  s.set(DstOff::D2Speed, 1.0);
  s.set(DstOff::D2Repeat, 0.0);  // passage direct, puis gel du dernier segment enregistré
  s.set(DstOff::D2Xfade, 0.0);
  s.set(DstOff::Arm, 1.0);
  s.sync();
  for (int i = 0; i < 4096; ++i) {
    double l = 0.0, r = 0.0;
    s.d.process(l, r, s.E.data(), s.base, s.c);
  }
  std::mt19937_64 rng(5);
  std::uniform_real_distribution<double> u(-1.0, 1.0);
  for (int i = 0; i < 9600; ++i) {
    double l = u(rng), r = l;
    s.d.process(l, r, s.E.data(), s.base, s.c);
  }
  s.E[static_cast<size_t>(s.base + static_cast<int>(DstOff::D2Freeze))] = 1.0;
  std::vector<double> y(24000);
  for (auto& v : y) {
    double l = 0.0, r = 0.0;
    s.d.process(l, r, s.E.data(), s.base, s.c);
    v = l;
  }
  // période : autocorrélation maximale
  auto corr = [&](int lag) {
    double a = 0.0, b = 0.0, c = 0.0;
    for (size_t i = 6000; i < 18000; ++i) {
      a += y[i] * y[i + static_cast<size_t>(lag)];
      b += y[i] * y[i];
      c += y[i + static_cast<size_t>(lag)] * y[i + static_cast<size_t>(lag)];
    }
    return a / std::sqrt(b * c + 1e-300);
  };
  int best = 0;
  double bc = -2.0;
  for (int lag = 1000; lag <= 4000; ++lag) {
    const double c = corr(lag);
    if (c > bc) {
      bc = c;
      best = lag;
    }
  }
  const bool period = std::abs(best - 2400) <= 1;
  double ymax = 0.0;
  for (double v : y) ymax = std::max(ymax, std::fabs(v));
  tk::info("DST-06 diagnostic : crete de la sortie gelee %.3g", ymax);
  // vitesse -1 : gel du dernier segment puis lecture inversée
  DSlot rv(fs, 6);
  rv.set(DstOff::D2Size, 50.0);
  rv.set(DstOff::D2Speed, -1.0);
  rv.set(DstOff::D2Repeat, 0.0);
  rv.set(DstOff::D2Xfade, 0.0);
  rv.set(DstOff::Arm, 1.0);
  rv.sync();
  for (int i = 0; i < 4096; ++i) {
    double l = 0.0, r = 0.0;
    rv.d.process(l, r, rv.E.data(), rv.base, rv.c);
  }
  std::vector<double> in(24000);
  for (size_t i = 0; i < in.size(); ++i) {
    in[i] = std::sin(kTwoPi * 3.0 * i / fs) * std::sin(kTwoPi * 211.0 * i / fs) + 0.3 * u(rng);
    double l = in[i], r = l;
    rv.d.process(l, r, rv.E.data(), rv.base, rv.c);
  }
  rv.E[static_cast<size_t>(rv.base + static_cast<int>(DstOff::D2Freeze))] = 1.0;
  std::vector<double> out(9600);
  for (auto& v : out) {
    double l = 0.0, r = 0.0;
    rv.d.process(l, r, rv.E.data(), rv.base, rv.c);
    v = l;
  }
  // une période complète (2400) après l'engagement : comparer à la fin de l'entrée lue à l'envers
  double bestRev = -1.0;
  for (int shift = 0; shift < 2400; ++shift) {
    double num = 0.0, d1 = 0.0, d2 = 0.0;
    for (int i = 0; i < 2000; ++i) {
      const double o = out[static_cast<size_t>(4800 + i)];
      const long src = static_cast<long>(in.size()) - 1 - ((i + shift) % 2400);
      const double r = in[static_cast<size_t>(src)];
      num += o * r;
      d1 += o * o;
      d2 += r * r;
    }
    bestRev = std::max(bestRev, num / std::sqrt(d1 * d2 + 1e-300));
  }
  const bool ok = period && bestRev >= 0.9;
  TK_CHECK(ok);
  tk::result("DST-06", ok, "D2 Freeze arme : periode %d ech. (2400 +-1, correlation %.3f) ; vitesse -1 : correlation avec le segment inverse %.3f (>= 0,9)", best, bc, bestRev);
}

void testY2() {
  const double fs = 48000.0;
  auto ratio = [&](double depth, double& envMax, double& envMin) {
    DSlot t(fs, 7);
    t.set(DstOff::Y2Depth, depth);
    t.set(DstOff::Y2Sens, 0.7);
    std::vector<double> in(48000, 0.0);
    for (size_t i = 12000; i < 36000; ++i) in[i] = 0.4 * std::sin(kTwoPi * 500.0 * i / fs);
    t.sync();
    for (int i = 0; i < 4096; ++i) {
      double l = 0.0, r = 0.0;
      t.d.process(l, r, t.E.data(), t.base, t.c);
    }
    std::vector<double> o(in.size());
    envMax = 0.0;
    envMin = 1.0;
    for (size_t i = 0; i < in.size(); ++i) {
      double l = in[i], r = in[i];
      t.d.process(l, r, t.E.data(), t.base, t.c);
      o[i] = l;
      envMax = std::max(envMax, t.d.y2Env());
      envMin = std::min(envMin, t.d.y2Env());
    }
    return levelDb(o, 12000, 12000 + 480) - levelDb(o, 30000, 36000);
  };
  double e1, e2, f1, f2;
  const double r0 = ratio(0.0, e1, e2), r1 = ratio(1.0, f1, f2);
  const bool ok = r1 > r0 + 1.0 && f1 <= 1.0 && f2 >= 0.0 && f1 > 0.0;
  TK_CHECK(ok);
  tk::result("DST-07", ok, "Y2 profondeur +1 : attaque/sustain %.1f dB > %.1f dB (profondeur 0) ; enveloppe source dans [%.2f ; %.2f] c [0,1]", r1, r0, f2, f1);
}

void testS2() {
  const double fs = 48000.0;
  bool lat = true;
  for (int ch = 0; ch < 4; ++ch) {
    const int N = 512 << ch;
    for (double mix : {0.0, 1.0}) {
      DSlot s(fs, 8);
      s.set(DstOff::S2Fft, ch);
      s.set(DstOff::Mix, mix);
      s.set(DstOff::S2Decay, 0.0);
      std::vector<double> x(static_cast<size_t>(N * 4), 0.0);
      x[100] = 1.0;
      const auto y = s.run(x);
      size_t arg = 0;
      for (size_t i = 0; i < y.size(); ++i)
        if (std::fabs(y[i]) > std::fabs(y[arg])) arg = i;
      const long d = static_cast<long>(arg) - 100;
      lat = lat && (mix == 0.0 ? d == N : std::labs(d - N) <= 1) && s.d.latency() == N;
    }
  }
  // latence de la chaîne déclarée (moteur)
  ParamValues p = ParamValues::defaults();
  p[ParamId::DstGl1Type] = 8.0f;
  p[ParamId::DstGl1S2Fft] = 3.0f;
  const bool decl = Engine::chainLatencyFor(p, 96000.0, 2) == 2048 && Engine::chainLatencyFor(p, 48000.0, 1) == 4096;
  // Freeze armé : RMS stable sans entrée
  DSlot f(fs, 8);
  f.set(DstOff::S2Fft, 2);
  f.set(DstOff::S2Decay, 1.0);
  f.set(DstOff::Arm, 1.0);
  f.sync();
  for (int i = 0; i < 4096; ++i) {
    double l = 0.0, r = 0.0;
    f.d.process(l, r, f.E.data(), f.base, f.c);
  }
  std::mt19937_64 rng(9);
  std::uniform_real_distribution<double> u(-0.3, 0.3);
  for (int i = 0; i < 24000; ++i) {
    double l = u(rng) + 0.3 * std::sin(kTwoPi * 440.0 * i / fs), r = l;
    f.d.process(l, r, f.E.data(), f.base, f.c);
  }
  f.E[static_cast<size_t>(f.base + static_cast<int>(DstOff::S2Freeze))] = 1.0;
  std::vector<double> y(static_cast<size_t>(fs * 2.5));
  for (auto& v : y) {
    double l = 0.0, r = 0.0;
    f.d.process(l, r, f.E.data(), f.base, f.c);
    v = l;
  }
  auto rmsDb = [&](size_t a, size_t b) {
    double s = 0.0;
    for (size_t i = a; i < b; ++i) s += y[i] * y[i];
    return 10.0 * std::log10(s / (b - a) + 1e-300);
  };
  const double r1 = rmsDb(24000, 48000), r2 = rmsDb(96000, 120000);
  const bool fr = std::fabs(r1 - r2) <= 3.0 && r1 > -60.0;
  const bool ok = lat && decl && fr;
  TK_CHECK(ok);
  tk::result("DST-08", ok, "S2 : latence = N exactement (mix 0) et +-1 (mix 1) pour N = 512..4096 %d ; latence de chaine declaree (K) %d ; Freeze arme : RMS %.1f puis %.1f dB (+-3) sans entree", lat, decl, r1, r2);
}

void stress(bool limitGains, int& bad, long long& alloc) {
  std::mt19937_64 rng(0xD57ULL);
  for (int c = 0; c < 48; ++c) {
    G4Scenario sc;
    const double rates[4] = {44100.0, 48000.0, 88200.0, 96000.0};
    sc.fs = rates[c % 4];
    sc.samples = static_cast<long long>(sc.fs * 2.0);
    sc.state = defaultState();
    for (int slot = 0; slot < 6; ++slot) {
      const int b = idx(ParamId::DstWt1Type) + slot * static_cast<int>(DstOff::Count);
      for (int o = 0; o < static_cast<int>(DstOff::Count); ++o) {
        const auto& sp = paramSpec(b + o);
        std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
        sc.state.params.v[static_cast<size_t>(b + o)] = sanitize(static_cast<ParamId>(b + o), u(rng));
      }
      if (limitGains) {
        auto& og = sc.state.params.v[static_cast<size_t>(b + static_cast<int>(DstOff::Out))];
        og = std::min(og, 0.0f);
        sc.state.params.v[static_cast<size_t>(b + static_cast<int>(DstOff::Y1Makeup))] = 0.0f;
      }
    }
    for (int k = 0; k < 4; ++k) {
      const int at = static_cast<int>(rng() % static_cast<unsigned long long>(sc.samples - 1));
      sc.events.push_back(MidiEvent::noteOn(at, 30 + static_cast<int>(rng() % 60), 100));
      sc.events.push_back(MidiEvent::noteOff(std::min<int>(at + 15000, static_cast<int>(sc.samples - 1)), 30));
    }
    const auto r = renderG4(sc);
    alloc += r.allocations;
    if (!allFinite(r.L) || r.nonFinite != 0 || r.clamp != 0) {
      ++bad;
      tk::info("DST-09%s config %d : nonFinite %u, garde %u, types %d %d %d %d %d %d", limitGains ? "b" : "", c, r.nonFinite, r.clamp,
               static_cast<int>(sc.state.params[ParamId::DstWt1Type]), static_cast<int>(sc.state.params[ParamId::DstWt2Type]),
               static_cast<int>(sc.state.params[ParamId::DstFm1Type]), static_cast<int>(sc.state.params[ParamId::DstFm2Type]),
               static_cast<int>(sc.state.params[ParamId::DstGl1Type]), static_cast<int>(sc.state.params[ParamId::DstGl2Type]));
    }
  }
}

void testStressAndMix() {
  int bad = 0, badB = 0;
  long long alloc = 0, allocB = 0;
  stress(false, bad, alloc);
  stress(true, badB, allocB);
  // DST-10 : mix 0 -> contournement bit à bit (algorithmes sans latence)
  bool mixOk = true;
  for (int t = 1; t <= 7; ++t) {
    G4Scenario a;
    a.fs = 48000.0;
    a.samples = 12000;
    a.state = defaultState();
    a.events = {MidiEvent::noteOn(0, 48, 100)};
    if (t == 1) a.quality = Quality::Eco;  // N1 sans latence (le critere vise les algorithmes sans latence)
    const auto ref = renderG4(a);
    a.state.params[ParamId::DstWt1Type] = static_cast<float>(t);
    a.state.params[ParamId::DstWt1Mix] = 0.0f;
    const auto y = renderG4(a);
    mixOk = mixOk && th::sameFloats(ref.L, y.L);
  }
  const bool ok9 = bad == 0 && alloc == 0, ok9b = badB == 0 && allocB == 0;
  TK_CHECK(ok9 && ok9b && mixOk);
  tk::result("DST-09", ok9, "48 configurations (6 emplacements aleatoires dont gains de sortie jusqu'a +24 dB et makeup +36 dB, 4 Fs, 2 s, notes) : %d en defaut (garde +30 dBFS), %lld allocation(s)", bad, alloc);
  tk::result("DST-09b", ok9b, "memes configurations, gains de sortie <= 0 dB et makeup 0 dB : %d en defaut, %lld allocation(s)", badB, allocB);
  tk::result("DST-10", mixOk, "mix 0 = contournement bit a bit (N1, D1, Y1, S1, N2, D2, Y2) : %d", mixOk);
}

}  // namespace

void runDestructionTests() {
  std::printf("[arch-g4 destruction]\n");
  testN1();
  testD1();
  testY1();
  testS1();
  testN2();
  testD2();
  testY2();
  testS2();
  testStressAndMix();
}
