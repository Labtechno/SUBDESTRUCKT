// SUBDESTRUCKT — tests ARCH-G4 du Filter & Resonator Engine (FLT-01..07). Seuils : docs/V1-ARCH-G4-TRACKER.md §3.
#include <array>
#include <cmath>
#include <complex>
#include <cstdio>
#include <random>
#include <vector>

#include "Analysis.h"
#include "G4Helper.h"
#include "TestKit.h"
#include "dsp/Filters.h"

using namespace sbd;
using namespace g4;

namespace {

struct Slot {
  ParamValues p = ParamValues::defaults();
  FilterSlot f;
  int base = idx(ParamId::FltGlType);
  std::array<double, kNumParams> E{};
  double fs;
  explicit Slot(double fs_) : fs(fs_) { f.configure(fs_); }
  void set(FltOff o, double v) { p.v[static_cast<size_t>(base + static_cast<int>(o))] = static_cast<float>(v); }
  void sync() {
    for (int i = 0; i < kNumParams; ++i) E[static_cast<size_t>(i)] = p.v[static_cast<size_t>(i)];
  }
  // réponse impulsionnelle (canal gauche), après n échantillons de silence pour finir le fondu d'entrée
  std::vector<double> impulse(int n, double noteHz = 261.6255653005986, bool exciteOnly = false) {
    sync();
    FilterContext c;
    c.fs = fs;
    c.noteHz = noteHz;
    for (int i = 0; i < 2048; ++i) {
      double l = 0.0, r = 0.0;
      f.process(l, r, E.data(), base, c);
    }
    std::vector<double> h(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
      double l = (i == 0 && !exciteOnly) ? 1.0 : 0.0, r = l;
      c.excite = exciteOnly && i == 0;
      f.process(l, r, E.data(), base, c);
      h[static_cast<size_t>(i)] = l;
    }
    return h;
  }
};

double magDb(const std::vector<double>& h, double fs, double f) {
  std::complex<double> s(0.0, 0.0);
  for (size_t n = 0; n < h.size(); ++n) s += h[n] * std::polar(1.0, -kTwoPi * f * n / fs);
  return 20.0 * std::log10(std::abs(s) + 1e-300);
}

void testF1() {
  const double fs = 48000.0, fc = 1000.0;
  bool ok = true;
  auto resp = [&](int mode) {
    Slot s(fs);
    s.set(FltOff::Type, 1);
    s.set(FltOff::F1Mode, mode);
    s.set(FltOff::Cutoff, fc);
    s.set(FltOff::Res, 0.0);
    return s.impulse(16384);
  };
  const auto lp12 = resp(0), lp24 = resp(1), bp12 = resp(2), hp12 = resp(4), notch = resp(6);
  const double a = magDb(lp12, fs, fc), b = magDb(hp12, fs, fc);
  const double s12 = magDb(lp12, fs, 4 * fc) - magDb(lp12, fs, 8 * fc), s24 = magDb(lp24, fs, 4 * fc) - magDb(lp24, fs, 8 * fc);
  // pic du passe-bande
  double best = -1e9, fbest = 0.0;
  for (double f = 800.0; f <= 1250.0; f += 1.0) {
    const double m = magDb(bp12, fs, f);
    if (m > best) {
      best = m;
      fbest = f;
    }
  }
  const double nd = magDb(notch, fs, fc);
  ok = std::fabs(a + 3.01) <= 0.5 && std::fabs(b + 3.01) <= 0.5 && std::fabs(s12 - 12.0) <= 1.5 && std::fabs(s24 - 24.0) <= 3.0 &&
       std::fabs(fbest / fc - 1.0) <= 0.02 && nd <= -30.0;
  TK_CHECK(ok);
  tk::result("FLT-01", ok, "F1 a 1 kHz : LP12 %.2f dB, HP12 %.2f dB (cible -3,01 +-0,5) ; pente 4fc-8fc LP12 %.1f dB/oct (12 +-1,5), LP24 %.1f (24 +-3) ; BP pic %.0f Hz (+-2 %%) ; Notch %.1f dB (<= -30)",
             a, b, s12, s24, fbest, nd);
}

void testStability() {
  std::mt19937_64 rng(0xF17EULL);
  int bad = 0;
  double worst = 0.0;
  for (int cfg = 0; cfg < 300; ++cfg) {
    const double fs = (cfg % 2) ? 96000.0 : 44100.0 * 2;
    Slot s(fs);
    for (int o = 0; o < static_cast<int>(FltOff::Count); ++o) {
      const auto& sp = paramSpec(s.base + o);
      std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
      s.p.v[static_cast<size_t>(s.base + o)] = sanitize(static_cast<ParamId>(s.base + o), u(rng));
    }
    s.set(FltOff::Type, 1 + static_cast<int>(rng() % 4));
    s.set(FltOff::Res, (rng() % 2) ? 1.0 : s.p.v[static_cast<size_t>(s.base + static_cast<int>(FltOff::Res))]);
    s.set(FltOff::Mix, 1.0);
    s.set(FltOff::Out, 0.0);
    s.sync();
    FilterContext c;
    c.fs = fs;
    std::uniform_real_distribution<double> nz(-1.0, 1.0);
    double phase = 0.0;
    for (int i = 0; i < 48000; ++i) {
      // cutoff audio-rate 20 Hz..20 kHz
      phase += 3.0 / fs;
      s.E[static_cast<size_t>(s.base + static_cast<int>(FltOff::Cutoff))] = 20.0 * std::pow(1000.0, 0.5 + 0.5 * std::sin(kTwoPi * phase * (1 + cfg % 7)));
      double l = nz(rng), r = nz(rng);
      c.excite = (i % 9000) == 0;
      s.f.process(l, r, s.E.data(), s.base, c);
      if (!std::isfinite(l) || !std::isfinite(r) || std::fabs(l) >= 16.0 || std::fabs(r) >= 16.0) {
        ++bad;
        break;
      }
      worst = std::max(worst, std::max(std::fabs(l), std::fabs(r)));
    }
  }
  TK_CHECK(bad == 0);
  tk::result("FLT-02", bad == 0, "300 configurations aleatoires (F1..F4, resonance max, cutoff audio-rate 20 Hz-20 kHz, bruit +-1, excitations) : %d en defaut ; |y| max %.2f (< 16)", bad, worst);
}

void testComb() {
  bool ok = true;
  double worst = 0.0;
  for (double fs : {48000.0, 96000.0})
    for (double fc : {40.0, 110.0, 440.0, 1000.0, 4000.0}) {
      Slot s(fs);
      s.set(FltOff::Type, 2);
      s.set(FltOff::Cutoff, fc);
      s.set(FltOff::CombFb, 0.9);
      s.set(FltOff::CombDamp, 0.0);
      const auto h = s.impulse(static_cast<int>(fs * 1.0));
      // pic au voisinage de fc (recherche fine en cents)
      double best = -1e9, cb = 0.0;
      for (double ct = -60.0; ct <= 60.0; ct += 0.25) {
        const double m = magDb(h, fs, fc * std::exp2(ct / 1200.0));
        if (m > best) {
          best = m;
          cb = ct;
        }
      }
      worst = std::max(worst, std::fabs(cb));
      ok = ok && std::fabs(cb) <= 5.0;
    }
  TK_CHECK(ok);
  tk::result("FLT-03", ok, "F2 comb : pic de resonance a fc, ecart max %.2f cents (seuil 5) sur 40/110/440/1000/4000 Hz a 48 et 96 kHz", worst);
}

void testModal() {
  const double fs = 48000.0;
  Slot s(fs);
  s.set(FltOff::Type, 4);
  s.set(FltOff::Cutoff, 220.0);
  s.set(FltOff::Modal, 0);
  s.set(FltOff::Modes, 8);
  s.set(FltOff::Decay, 1.0);
  s.set(FltOff::Damp, 0.0);
  s.set(FltOff::Inharm, 0.0);
  const auto h = s.impulse(static_cast<int>(fs * 2));
  bool freqs = true;
  for (int k = 1; k <= 8; ++k) freqs = freqs && std::fabs(1200.0 * std::log2(s.f.testModeFreq(k - 1) / (220.0 * k))) <= 1.0;
  // T60 du mode 1 : enveloppe du signal filtré autour de 220 Hz (démodulation)
  auto env = [&](double t0) {
    std::complex<double> acc(0.0, 0.0);
    const int a = static_cast<int>(t0 * fs), n = 4800;
    for (int i = a; i < a + n; ++i) acc += h[static_cast<size_t>(i)] * std::polar(1.0, -kTwoPi * 220.0 * i / fs);
    return std::abs(acc) / n;
  };
  const double e1 = env(0.2), e2 = env(1.2);
  const double t60 = 60.0 / (20.0 * std::log10(e1 / e2)) * 1.0;
  const bool decay = std::fabs(t60 - 1.0) <= 0.1;
  // Freeze non armé : garde-fou -> plus d'énergie ; armé : énergie conservée
  auto freezeRun = [&](bool armed) {
    Slot z(fs);
    z.set(FltOff::Type, 4);
    z.set(FltOff::Cutoff, 220.0);
    z.set(FltOff::Freeze, 0.0);
    z.set(FltOff::Arm, armed ? 1.0 : 0.0);
    z.sync();
    FilterContext c;
    c.fs = fs;
    c.voiceIdle = true;
    for (int i = 0; i < 2048; ++i) {
      double l = 0.0, r = 0.0;
      z.f.process(l, r, z.E.data(), z.base, c);
    }
    for (int i = 0; i < 4800; ++i) {  // excitation puis gel
      double l = (i < 100) ? 0.5 : 0.0, r = l;
      z.f.process(l, r, z.E.data(), z.base, c);
    }
    z.E[static_cast<size_t>(z.base + static_cast<int>(FltOff::Freeze))] = 1.0;
    double last = 0.0;
    for (int i = 0; i < static_cast<int>(fs * 12); ++i) {
      double l = 0.0, r = 0.0;
      z.f.process(l, r, z.E.data(), z.base, c);
      if (i > fs * 11.5) last = std::max(last, std::fabs(l));
    }
    return last;
  };
  const double unarmed = freezeRun(false), armed = freezeRun(true);
  const bool fr = unarmed < 1e-9 && armed > 1e-4;
  const bool ok = freqs && decay && fr;
  TK_CHECK(ok);
  tk::result("FLT-04", ok, "F4 modal : 8 modes harmoniques a 220 Hz a 1 cent pres %d ; T60 mode 1 = %.3f s (1 s +-10 %%) ; Freeze non arme apres garde-fou : crete %.2g (nulle), arme : %.2g (conservee)",
             freqs, t60, unarmed, armed);
}

void testNonlinear() {
  const double fs = 48000.0;
  auto lowLevel = [&](int type, double drive) {
    Slot s(fs);
    s.set(FltOff::Type, type);
    s.set(FltOff::F1Mode, 0);
    s.set(FltOff::F3Mode, 0);
    s.set(FltOff::Cutoff, 1000.0);
    s.set(FltOff::Res, 0.3);
    s.set(FltOff::Drive, drive);
    s.set(FltOff::Char, 0.0);
    // réponse à faible niveau (-40 dBFS) : impulsion de 0,01
    s.sync();
    FilterContext c;
    c.fs = fs;
    for (int i = 0; i < 2048; ++i) {
      double l = 0.0, r = 0.0;
      s.f.process(l, r, s.E.data(), s.base, c);
    }
    std::vector<double> h(16384);
    for (size_t i = 0; i < h.size(); ++i) {
      double l = i == 0 ? 0.01 : 0.0, r = l;
      s.f.process(l, r, s.E.data(), s.base, c);
      h[i] = l / 0.01;
    }
    return h;
  };
  const auto f1 = lowLevel(1, 0.0), f3 = lowLevel(3, 0.0);
  const double d1 = std::fabs(magDb(f1, fs, 1000.0) - magDb(f3, fs, 1000.0)), d4 = std::fabs(magDb(f1, fs, 4000.0) - magDb(f3, fs, 4000.0));
  // THD croissante avec le drive : sinus 200 Hz amplitude 0,5
  auto thd = [&](double drive) {
    Slot s(fs);
    s.set(FltOff::Type, 3);
    s.set(FltOff::F3Mode, 0);
    s.set(FltOff::Cutoff, 8000.0);
    s.set(FltOff::Res, 0.0);
    s.set(FltOff::Drive, drive);
    s.set(FltOff::Char, 0.5);
    s.sync();
    FilterContext c;
    c.fs = fs;
    std::vector<double> y(32768);
    for (int i = 0; i < 8192 + 32768; ++i) {
      double l = 0.5 * std::sin(kTwoPi * 187.5 * i / fs), r = l;
      s.f.process(l, r, s.E.data(), s.base, c);
      if (i >= 8192) y[static_cast<size_t>(i - 8192)] = l;
    }
    const auto m = an::amplitudeSpectrum(y.data(), y.size());
    const double bin = fs / 32768.0, f0 = an::lineAmplitude(m, bin, 187.5);
    double h = 0.0;
    for (int k = 2; k <= 10; ++k) h += std::pow(an::lineAmplitude(m, bin, 187.5 * k), 2.0);
    return std::sqrt(h) / f0;
  };
  const double t0 = thd(0.0), t12 = thd(12.0), t24 = thd(24.0);
  const bool ok = d1 <= 0.5 && d4 <= 0.5 && t0 < t12 && t12 < t24;
  TK_CHECK(ok);
  tk::result("FLT-05", ok, "F3 drive 0 a -40 dBFS : ecart a F1 %.3f dB a fc, %.3f dB a 4fc (<= 0,5) ; THD drive 0/12/24 dB : %.4f < %.4f < %.4f", d1, d4, t0, t12, t24);
}

void testChainBehaviour() {
  const double fs = 48000.0;
  // excitation Note sans signal d'entrée (WT et FM muets)
  G4Scenario sc;
  sc.fs = fs;
  sc.samples = 24000;
  sc.state = bareState();
  sc.state.params[ParamId::FltGlType] = 2.0f;
  sc.state.params[ParamId::FltGlExc] = 1.0f;
  sc.state.params[ParamId::FltGlCutoff] = 220.0f;
  sc.state.params[ParamId::FltGlCombFb] = 0.95f;
  sc.events = {MidiEvent::noteOn(1000, 57, 100)};
  const auto ex = renderG4(sc);
  const bool exc = peak(ex.L, 0, 1000) == 0.0 && peak(ex.L, 1000, 24000) > 1e-3;
  // Linked : L == R ; Dual Mono + spread : L != R
  auto stereo = [&](int dual) {
    G4Scenario s2;
    s2.fs = fs;
    s2.samples = 12000;
    s2.state = bareState();
    s2.state.params[ParamId::WtLevel] = 0.5f;
    s2.state.params[ParamId::FltGlType] = 1.0f;
    s2.state.params[ParamId::FltGlStereo] = static_cast<float>(dual);
    s2.state.params[ParamId::FltGlSpread] = 0.8f;
    s2.state.params[ParamId::FltGlCutoff] = 800.0f;
    s2.events = {MidiEvent::noteOn(0, 45, 100)};
    return renderG4(s2);
  };
  const auto lk = stereo(0), dm = stereo(1);
  const bool st = th::sameFloats(lk.L, lk.R) && !th::sameFloats(dm.L, dm.R);
  // changement de type en cours de note : aucun NaN
  G4Scenario sw;
  sw.fs = fs;
  sw.samples = 48000;
  sw.state = bareState();
  sw.state.params[ParamId::WtLevel] = 0.5f;
  sw.events = {MidiEvent::noteOn(0, 45, 100)};
  sw.beforeBlock = [](Engine&, ParamValues& p, long long pos) {
    p[ParamId::FltWtType] = static_cast<float>((pos / 4096) % 5);
    p[ParamId::FltFmType] = static_cast<float>((pos / 3000) % 5);
  };
  const auto swr = renderG4(sw);
  const bool sws = allFinite(swr.L) && swr.nonFinite == 0 && swr.allocations == 0;
  const bool ok = exc && st && sws;
  TK_CHECK(ok);
  tk::result("FLT-06/07", ok, "excitation Note sans entree : silence avant, resonance apres %d ; Linked L==R, Dual Mono+spread L!=R %d ; changements de type en cours de note : finis, 0 allocation %d",
             exc, st, sws);
}

}  // namespace

void runFilterTests() {
  std::printf("[arch-g4 filtres]\n");
  testF1();
  testStability();
  testComb();
  testModal();
  testNonlinear();
  testChainBehaviour();
}
