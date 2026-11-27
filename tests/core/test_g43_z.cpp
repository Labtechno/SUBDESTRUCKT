// SUBDESTRUCKT — tests ARCH-G4.3 : catalogue Z (huit transformations). docs/V1-ARCH-G4.3-SPEC.md §7.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include "Analysis.h"
#include "G4Helper.h"
#include "RenderHelper.h"
#include "TestKit.h"
#include "core/StateCodec.h"
#include "dsp/Fft.h"
#include "dsp/ZSpectral.h"

using namespace sbd;
using namespace g4;
using cd = std::complex<double>;

namespace {

const int kNew[5] = {4, 5, 6, 7, 8};
const char* zName(int t) {
  static const char* n[9] = {"Off", "Fold", "Warp", "Mutation", "Tilt", "Mask", "Focus", "Shift", "Scatter"};
  return n[t];
}

// coefficients d'une forme de banque (amplitude, phase préparées)
std::vector<cd> coefs(const char* bank, int wave) {
  auto b = FactoryBanks::get(bank);
  std::vector<cd> c(zspec::kH + 2, cd(0.0, 0.0));
  for (int h = 1; h <= zspec::kH; ++h) c[h] = std::polar(static_cast<double>(b->specMag(wave)[h - 1]), static_cast<double>(b->specPhase(wave)[h - 1]));
  return c;
}
std::vector<cd> run1(const std::vector<cd>& in, int t, double v, double s, bool fund = false, bool gain = false, double* gc = nullptr) {
  std::vector<cd> c = in, w(zspec::kH + 2);
  const int zt[2] = {t, 0};
  const double zv[2] = {v, 0.0}, zs[2] = {s, 0.0};
  const double g = zspec::process(c.data(), in.data(), w.data(), zt, zv, zs, fund, gain, [](int) { return 1.0; });
  if (gc) *gc = g;
  return c;
}
double dbDist(const std::vector<cd>& a, const std::vector<cd>& b, int hmax = 64) {
  double mx = 0.0;
  for (int h = 1; h <= hmax; ++h) mx = std::max(mx, std::abs(b[h]));
  double acc = 0.0;
  int n = 0;
  for (int h = 1; h <= hmax; ++h) {
    if (std::abs(b[h]) < 1e-6 * mx) continue;
    const double fl = 1e-6 * mx;
    const double d = 20.0 * std::log10(std::max(std::abs(a[h]), fl) / std::max(std::abs(b[h]), fl));
    acc += d * d;
    ++n;
  }
  return n ? std::sqrt(acc / n) : 0.0;
}
std::vector<double> synth(const std::vector<cd>& c, int hmax = 1024, int N = 2048) {  // forme temporelle (somme de cosinus)
  std::vector<cd> x(static_cast<size_t>(N), cd(0.0, 0.0));
  for (int h = 1; h <= hmax && h < N / 2; ++h) {
    x[static_cast<size_t>(h)] = c[h] * (0.5 * N);
    x[static_cast<size_t>(N - h)] = std::conj(c[h] * (0.5 * N));
  }
  for (auto& v : x) v = std::conj(v);
  an::fft(x);
  std::vector<double> y(static_cast<size_t>(N));
  for (int i = 0; i < N; ++i) y[static_cast<size_t>(i)] = std::conj(x[static_cast<size_t>(i)]).real() / N;
  return y;
}
double crestDb(const std::vector<double>& y) {
  double pk = 0.0, e = 0.0;
  for (double v : y) pk = std::max(pk, std::fabs(v)), e += v * v;
  return 20.0 * std::log10(pk / std::sqrt(e / y.size()));
}

// état WT seule (aucune route, opérateurs muets), transformation Z donnée
PluginState zState(const char* bank, int ta, double da, double sa, int tb = 0, double db = 0.0, double sb = 0.0, double zm = 1.0) {
  PluginState s = bareState();
  s.bank = bank;
  s.params[ParamId::Op1Level] = 0.0f;
  s.params[ParamId::Op2Level] = 0.0f;
  s.params[ParamId::WtLevel] = 0.7f;
  s.params[ParamId::WtX] = 0.37f;
  s.params[ParamId::WtZ] = static_cast<float>(zm);
  s.params[ParamId::WtZAType] = static_cast<float>(ta);
  s.params[ParamId::WtZADepth] = static_cast<float>(da);
  s.params[ParamId::WtZAShape] = static_cast<float>(sa);
  s.params[ParamId::WtZBType] = static_cast<float>(tb);
  s.params[ParamId::WtZBDepth] = static_cast<float>(db);
  s.params[ParamId::WtZBShape] = static_cast<float>(sb);
  return s;
}
G4Render renderNote(const PluginState& s, int note, long long samples = 48000, double fs = 48000.0, Quality q = Quality::Normal) {
  G4Scenario sc;
  sc.state = s;
  sc.fs = fs;
  sc.quality = q;
  sc.samples = samples;
  sc.events = {MidiEvent::noteOn(0, note, 100)};
  return renderG4(sc);
}
double aliasDb(const std::vector<float>& L, double fs, int note) {
  const size_t n = 16384, a = L.size() - n;
  std::vector<double> seg(L.begin() + static_cast<long>(a), L.begin() + static_cast<long>(a + n));
  const auto m = an::amplitudeSpectrum(seg.data(), n);
  const double f0 = 440.0 * std::exp2((note - 69) / 12.0);
  return an::offGridEnergyDb(m, fs / n, f0, 20.0, 0.45 * fs, 6);
}

void testZ() {
  // ZC-01 : registre
  {
    bool ok = kNumParams == 703 && choiceCount(ParamId::WtZAType) == 9 && choiceCount(ParamId::WtZBType) == 9 &&
              std::string(paramSpec(ParamId::WtZAShape).stableId) == "wt.za.shape" && idx(ParamId::WtZAShape) == 701 && idx(ParamId::WtZBShape) == 702;
    const char* names[9] = {"Off", "Harmonic Fold", "Phase Warp", "Spectral Mutation", "Harmonic Tilt", "Harmonic Mask", "Harmonic Focus", "Harmonic Shift", "Phase Scatter"};
    for (int k = 0; k < 9; ++k) ok = ok && std::string(choiceLabel(ParamId::WtZAType, k)) == names[k] && std::string(choiceLabel(ParamId::WtZBType, k)) == names[k];
    TK_CHECK(ok);
    tk::result("ZC-01", ok, "registre : enumerations Z etendues en fin (0-3 inchanges : Off, Harmonic Fold, Phase Warp, Spectral Mutation ; 4-8 : Harmonic Tilt, Mask, Focus, Shift, Phase Scatter), wt.za.shape / wt.zb.shape ajoutes (701-702) ; etats v2/v3 : D2R-02, MIG-02");
  }
  // ZC-02 : neutralité exacte
  {
    int ok = 0, n = 0;
    std::mt19937_64 rng(0x2C02ULL);
    std::uniform_real_distribution<double> u(0.0, 1.0);
    for (int t : kNew)
      for (int k = 0; k < 4; ++k) {
        const bool masterZero = k % 2 == 0;
        const PluginState a = zState("factory:basic", t, masterZero ? u(rng) : 0.0, 2.0 * u(rng) - 1.0, k == 3 ? t : 0, k == 3 ? 0.0 : 0.0, 0.3, masterZero ? 0.0 : 0.5 + 0.5 * u(rng));
        const PluginState b = zState("factory:basic", 0, 0.0, 0.0, 0, 0.0, 0.0, 0.0);
        ok += th::sameFloats(renderNote(a, 50, 24000).L, renderNote(b, 50, 24000).L);
        ++n;
      }
    const bool pass = ok == n;
    TK_CHECK(pass);
    tk::result("ZC-02", pass, "neutralite : Z Master = 0 ou profondeur 0, 5 transformations x 4 configurations : bit a bit = moteur sans Z %d/%d", ok, n);
  }
  // ZC-04 : continuité à 0+ (v = 1e-4), morph Linear et Spectral, Clean et Raw
  {
    double worst = -400.0;
    std::string det;
    for (int t : kNew)
      for (int morph : {0, 2})
        for (int raw : {0, 1}) {
          PluginState a = zState("factory:basic", t, 1e-4, 0.0), b = zState("factory:basic", 0, 0.0, 0.0);
          for (auto* st : {&a, &b}) {
            st->params[ParamId::WtXMorph] = static_cast<float>(morph);
            st->params[ParamId::WtRender] = static_cast<float>(raw);
          }
          const auto ra = renderNote(a, 57, 24000), rb = renderNote(b, 57, 24000);
          const double d = nmseDb(ra.L, rb.L);
          worst = std::max(worst, d);
          if (d > -40.0) det += std::string(" [") + zName(t) + (morph ? " spectral" : " linear") + (raw ? " raw" : " clean") + " " + std::to_string(d) + "]";
        }
    const bool ok = worst <= -40.0;
    TK_CHECK(ok);
    tk::result("ZC-04", ok, "continuite a 0+ (v = 1e-4 vs 0 ; 5 transformations x morph Linear/Spectral x Clean/Raw) : pire ecart %.1f dB (<= -40)%s", worst, det.c_str());
  }
  // ZC-05, ZC-14, ZC-15 : propriétés des fonctions (coefficients des formes de banque)
  {
    const auto saw = coefs("factory:saw", 0), metal = coefs("factory:metal", 3);
    bool prog = true;
    double minEffect = 1e9;
    std::string det;
    for (int t : kNew)
      for (double s : {-1.0, 0.0, 1.0})
        for (const auto* src : {&saw, &metal}) {
          double prev = -1.0;
          for (int i = 0; i <= 20; ++i) {
            const double v = i / 20.0;
            const auto c = run1(*src, t, v, s);
            double d;
            if (t == 8) {  // Phase Scatter : écart temporel (dB), amplitudes inchangées
              const auto ya = synth(c, 64), yb = synth(*src, 64);
              double num = 0.0, den = 0.0;
              for (size_t k = 0; k < ya.size(); ++k) num += (ya[k] - yb[k]) * (ya[k] - yb[k]), den += yb[k] * yb[k];
              d = num > 0.0 ? 10.0 * std::log10(num / den) + 400.0 : 0.0;  // décalé pour la monotonie (0 à v = 0)
            } else {
              d = dbDist(c, *src);
            }
            if (d < prev - 1e-9) {
              prog = false;
              det += std::string(" [") + zName(t) + (src == &saw ? " Saw" : " Metal") + " s" + std::to_string(s).substr(0, 4) + " v" + std::to_string(v).substr(0, 4) + "]";
            }
            prev = d;
            if (i == 20) minEffect = std::min(minEffect, t == 8 ? d - 400.0 + 20.0 + 3.0 : d);  // Z7 : >= -20 dB -> marge comparable
          }
        }
    const bool ok5 = prog && minEffect >= 3.0;
    TK_CHECK(ok5);
    tk::result("ZC-05", ok5, "progressivite : distance spectrale (Z7 : ecart temporel) non decroissante sur 21 profondeurs, sigma -1/0/1, formes Saw et Metal %d ; effet minimal a v = 1 : %.2f dB (>= 3 ; Z7 normalise : ecart >= -20 dB)%s",
               prog, minEffect, det.c_str());

    // ZC-14 : distinction Tilt / Mask / Focus (gains) et propriétés structurelles
    auto gains = [](int t) {
      std::vector<double> g(65);
      for (int h = 1; h <= 64; ++h) g[h] = zspec::gain(static_cast<ZType>(t), h, 1.0, 0.0);
      return g;
    };
    const auto gt = gains(4), gm = gains(5), gf = gains(6);
    auto dist = [](const std::vector<double>& a, const std::vector<double>& b) {
      double acc = 0.0;
      for (int h = 1; h <= 64; ++h) {
        const double d = 20.0 * std::log10(std::max(a[h], 1e-6) / std::max(b[h], 1e-6));
        acc += d * d;
      }
      return std::sqrt(acc / 64.0);
    };
    const double dTM = dist(gt, gm), dTF = dist(gt, gf), dMF = dist(gm, gf);
    bool tiltMono = true;
    for (double s : {-1.0, 0.0, 1.0}) {
      double sgn = 0.0;
      for (int h = 1; h < 1024; ++h) {
        const double d = zspec::gain(ZType::Tilt, h + 1, 0.7, s) - zspec::gain(ZType::Tilt, h, 0.7, s);
        if (sgn == 0.0 && d != 0.0) sgn = d > 0 ? 1.0 : -1.0;
        if (d * sgn < 0.0) tiltMono = false;
      }
    }
    bool maskPeriodic = true;
    for (int h = 3; h <= 60; h += 3) maskPeriodic = maskPeriodic && std::fabs(gm[h] - 0.0) < 1e-12 && gm[h - 1] > gm[h] && gm[h + 1] > gm[h];
    int arg = 1;
    for (int h = 1; h <= 64; ++h)
      if (gf[h] > gf[arg]) arg = h;
    const double C = zspec::focusCentre(0.0);
    bool unimodal = true;
    for (int h = 1; h < arg; ++h) unimodal = unimodal && gf[h] <= gf[h + 1];
    for (int h = arg; h < 64; ++h) unimodal = unimodal && gf[h] >= gf[h + 1];
    const bool ok14 = dTM >= 3.0 && dTF >= 3.0 && dMF >= 3.0 && tiltMono && maskPeriodic && unimodal && std::fabs(arg - C) <= 1.0;
    TK_CHECK(ok14);
    tk::result("ZC-14", ok14, "distinction (v = 1, sigma = 0) : Tilt/Mask %.1f dB, Tilt/Focus %.1f dB, Mask/Focus %.1f dB (>= 3) ; Tilt monotone en rang %d ; Mask minima nuls aux multiples de p = 3 %d ; Focus unimodal, maximum au rang %d (C = %.2f) %d",
               dTM, dTF, dMF, tiltMono, maskPeriodic, arg, C, unimodal);

    // ZC-15 : propriétés exactes (implémentation de référence indépendante)
    bool tilt = true, mask = true, shift = true, scatter = true;
    for (double v : {0.1, 0.5, 1.0})
      for (double s : {-1.0, 0.0, 0.6}) {
        const auto ct = run1(saw, 4, v, s);
        for (int h = 1; h <= 1024; ++h) {
          const double ref = std::pow(static_cast<double>(h), v * (0.5 + 1.5 * s));
          if (std::abs(ct[h] - saw[h] * ref) > 1e-9 * std::abs(saw[h] * ref) + 1e-15) tilt = false;
        }
        const double p = s >= 0 ? 3.0 * std::pow(8.0 / 3.0, s) : 3.0 * std::pow(2.0 / 3.0, -s);
        const auto cm = run1(saw, 5, v, s);
        for (int h = 1; h <= 1024; ++h) {
          const double r = h / p;
          if (std::fabs(r - std::round(r)) < 1e-12 && std::abs(cm[h] - saw[h] * (1.0 - v)) > 1e-9 * std::abs(saw[h]) + 1e-15) mask = false;
        }
        const auto cs = run1(metal, 8, v, s);
        for (int h = 1; h <= 1024; ++h)
          if (std::fabs(std::abs(cs[h]) - std::abs(metal[h])) > 1e-9 * std::abs(metal[h]) + 1e-15) scatter = false;
      }
    for (int k : {1, 2, 5, 16, -3, -8}) {  // décalage entier : c'[h] = c[h-k]
      const double v = 1.0, s = (k / 8.0 - 0.5) / 1.5;  // 8 v lambda(s) = k
      const auto c = run1(metal, 7, v, s);
      for (int h = 1; h <= 1024; ++h) {
        const int src = h - k;
        const cd want = src >= 1 && src <= 1024 ? metal[src] : cd(0.0, 0.0);
        if (std::abs(c[h] - want) > 1e-12 * (std::abs(want) + 1e-30) && !(want == cd(0.0, 0.0) && c[h] == cd(0.0, 0.0))) shift = false;
      }
    }
    const bool ok15 = tilt && mask && shift && scatter;
    TK_CHECK(ok15);
    tk::result("ZC-15", ok15, "proprietes exactes (reference independante) : Tilt gains = h^(v lambda) %d ; Mask attenuation (1 - v) aux multiples de p %d ; Shift entier c'[h] = c[h-k] (k = 1, 2, 5, 16, -3, -8) %d ; Scatter amplitudes inchangees %d",
               tilt, mask, shift, scatter);

    // ZC-06 / ZC-07 : fondamentale et compensation de gain
    bool fundOk = true, gainOk = true, scatterUnit = true;
    int clamped = 0, cases = 0;
    double worstG = 0.0;
    for (int t : kNew)
      for (int i = 0; i <= 10; ++i)
        for (double s : {-1.0, 0.0, 1.0}) {
          const double v = i / 10.0;
          const auto cf = run1(saw, t, v, s, true, true);
          if (std::abs(cf[1] - saw[1]) > 1e-9 * std::abs(saw[1])) fundOk = false;
          double gc = 1.0;
          const auto cg = run1(metal, t, v, s, false, true, &gc);
          double ein = 0.0, eout = 0.0;
          for (int h = 1; h <= 1024; ++h) ein += std::norm(metal[h]), eout += std::norm(cg[h]);
          ++cases;
          if (gc <= 0.25 + 1e-12 || gc >= 4.0 - 1e-12) {
            ++clamped;
            continue;
          }
          const double d = std::fabs(10.0 * std::log10(eout / ein));
          worstG = std::max(worstG, d);
          if (d > 0.1) gainOk = false;
          if (t == 8 && std::fabs(gc - 1.0) > 1e-12) scatterUnit = false;
        }
    const bool ok6 = fundOk;
    TK_CHECK(ok6);
    tk::result("ZC-06", ok6, "fondamentale (z_fund = 1, compensation active) : c1 de sortie = c1 d'entree (ecart relatif <= 1e-9), 5 transformations x 11 profondeurs x 3 Shape %d", fundOk);
    const bool ok7 = gainOk && scatterUnit;
    TK_CHECK(ok7);
    tk::result("ZC-07", ok7, "compensation de gain : energie de sortie = energie d'entree a %.4f dB pres (<= 0,1) hors butee [0,25 ; 4] (%d/%d cas en butee, signales) ; Phase Scatter : gain 1 exact %d",
               worstG, clamped, cases, scatterUnit);
  }
  // ZC-08 : DC, bande, aliasing (rendus réels)
  {
    double worstAlias = -400.0;
    std::string det;
    for (int t : kNew)
      for (int note = 24; note <= 108; note += 12) {
        const auto r = renderNote(zState("factory:basic", t, 1.0, 0.0), note, 48000);
        const double a = aliasDb(r.L, 48000.0, note);
        worstAlias = std::max(worstAlias, a);
        if (a > -60.0) det += std::string(" [") + zName(t) + " note " + std::to_string(note) + " : " + std::to_string(a).substr(0, 6) + " dB]";
      }
    // DC et bande : structurels (c0 jamais écrit ; gains de bande appliqués après transformation)
    const auto c = run1(coefs("factory:basic", 2), 7, 1.0, 1.0);
    const auto y = synth(c, 64);
    double mean = 0.0, e = 0.0;
    for (double v : y) mean += v, e += v * v;
    mean /= y.size();
    const bool dc = std::fabs(mean) <= 1e-9 * std::sqrt(e / y.size());
    const bool ok = worstAlias <= -60.0 && dc;
    TK_CHECK(ok);
    tk::result("ZC-08", ok, "DC/Nyquist/aliasing : DC de table %d ; notes MIDI 24-108 (pas 12), 48 kHz, Clean, profondeur 1 : pire energie hors harmoniques %.1f dB (<= -60)%s",
               dc, worstAlias, det.c_str());
  }
  // ZC-09 : ordre (deux transformations spectrales non commutatives)
  {
    const auto metal = coefs("factory:metal", 5);
    auto two = [&](int ta, int tb) {
      std::vector<cd> c = metal, w(zspec::kH + 2);
      const int zt[2] = {ta, tb};
      const double zv[2] = {0.8, 0.7}, zs[2] = {0.3, 0.6};
      zspec::process(c.data(), metal.data(), w.data(), zt, zv, zs, false, false, [](int) { return 1.0; });
      return c;
    };
    auto ref = [&](int ta, int tb) {  // composition calculée hors moteur (deux applications successives)
      std::vector<cd> c = metal, w(zspec::kH + 2);
      zspec::apply(static_cast<ZType>(ta), c.data(), w.data(), 0.8, 0.3);
      zspec::apply(static_cast<ZType>(tb), c.data(), w.data(), 0.7, 0.6);
      return c;
    };
    bool ok = true;
    double minDiff = 0.0;
    for (auto pr : {std::pair<int, int>{4, 7}, {5, 7}}) {
      const auto ab = two(pr.first, pr.second), ba = two(pr.second, pr.first);
      const auto rab = ref(pr.first, pr.second), rba = ref(pr.second, pr.first);
      double num = 0.0, den = 0.0, e1 = 0.0, e2 = 0.0;
      for (int h = 1; h <= 1024; ++h) {
        num += std::norm(ab[h] - ba[h]);
        den += std::norm(ab[h]);
        e1 = std::max(e1, std::abs(ab[h] - rab[h]));
        e2 = std::max(e2, std::abs(ba[h] - rba[h]));
      }
      const double d = 10.0 * std::log10(num / den);
      minDiff = minDiff == 0.0 ? d : std::min(minDiff, d);
      ok = ok && d >= -40.0 && e1 <= 1e-9 && e2 <= 1e-9;
    }
    // moteur : le paramètre d'ordre est respecté
    PluginState a = zState("factory:metal", 4, 0.8, 0.3, 7, 0.7, 0.6), b = a;
    b.params[ParamId::WtZOrder] = 1.0f;
    const bool engineOrder = !th::sameFloats(renderNote(a, 50, 24000).L, renderNote(b, 50, 24000).L);
    ok = ok && engineOrder;
    TK_CHECK(ok);
    tk::result("ZC-09", ok, "ordre : Tilt/Shift et Mask/Shift : A>B != B>A (ecart minimal %.1f dB >= -40) et chaque ordre = composition de reference (<= 1e-9) ; moteur : z_order respecte %d ; domaines : spectral -> phase -> lecture -> Fold (fixe, documente)",
               minDiff, engineOrder);
  }
  // ZC-10 : morph XY et Quality
  {
    bool ok = true;
    double worstCont = -400.0, worstAlias = -400.0;
    for (int morph : {0, 1, 2})
      for (Quality q : {Quality::Eco, Quality::Normal, Quality::High})
        for (int t : kNew) {
          PluginState a = zState("factory:pwm", t, 1.0, 0.2), c0 = zState("factory:pwm", t, 1e-4, 0.2), z0 = zState("factory:pwm", 0, 0.0, 0.0);
          for (auto* st : {&a, &c0, &z0}) {
            st->params[ParamId::WtXMorph] = static_cast<float>(morph);
            st->params[ParamId::WtX] = 0.43f;
          }
          const auto ra = renderNote(a, 60, 24000, 48000.0, q);
          ok = ok && allFinite(ra.L);
          worstCont = std::max(worstCont, nmseDb(renderNote(c0, 60, 24000, 48000.0, q).L, renderNote(z0, 60, 24000, 48000.0, q).L));
          const auto rl = renderNote(a, 72, 32768, 48000.0, q);
          worstAlias = std::max(worstAlias, aliasDb(rl.L, 48000.0, 72));
        }
    ok = ok && worstCont <= -40.0 && worstAlias <= -60.0;
    TK_CHECK(ok);
    tk::result("ZC-10", ok, "morph XY (Linear, Stepped, Spectral, X fractionnaire) x Quality (Eco, Normal, High) x 5 transformations : finis ; continuite a 0+ pire %.1f dB (<= -40) ; aliasing pire %.1f dB (<= -60)",
               worstCont, worstAlias);
  }
  // ZC-11 : modulation rapide (LFO 20 Hz sur profondeur et Shape ; Z Master par CROSS audio)
  {
    bool ok = true;
    double ratioPk = 0.0, ratioJump = 0.0;
    for (int t : kNew) {
      double staticPk = 0.0, staticJump = 0.0;
      for (double v : {0.0, 0.25, 0.5, 0.75, 1.0})
        for (double s : {-1.0, 0.0, 1.0}) {
          const auto r = renderNote(zState("factory:basic", t, v, s), 48, 24000);
          staticPk = std::max(staticPk, peak(r.L, 4800));
          for (size_t i = 4801; i < r.L.size(); ++i) staticJump = std::max(staticJump, static_cast<double>(std::fabs(r.L[i] - r.L[i - 1])));
        }
      PluginState m = zState("factory:basic", t, 0.5, 0.0);
      m.params[ParamId::Lfo1Rate] = 20.0f;
      ModRoute r1;
      r1.source = ModSource::Lfo1;
      r1.dest = ParamId::WtZADepth;
      r1.depth = 0.5f;
      modAdd(m.mod, r1);
      ModRoute r2 = r1;
      r2.dest = ParamId::WtZAShape;
      r2.depth = 0.5f;
      modAdd(m.mod, r2);
      // CROSS : OP1 (audible, 3 x la fondamentale) -> WT.Z, taux audio
      setRoutes(m, {RouteTable::Op::add(kNodeOP1, kPortWtZ)});
      m.params[ParamId::DepthOp1WtZ] = 0.5f;
      m.params[ParamId::Op1Ratio] = 3.0f;
      m.params[ParamId::CrossMaster] = 1.0f;
      m.params[ParamId::CrossRev] = 1.0f;
      const auto r = renderNote(m, 48, 48000);
      double pk = peak(r.L, 4800), jump = 0.0;
      for (size_t i = 4801; i < r.L.size(); ++i) jump = std::max(jump, static_cast<double>(std::fabs(r.L[i] - r.L[i - 1])));
      ok = ok && allFinite(r.L) && pk <= 4.0 * staticPk && jump <= 1.5 * staticJump;
      ratioPk = std::max(ratioPk, pk / staticPk);
      ratioJump = std::max(ratioJump, jump / staticJump);
    }
    TK_CHECK(ok);
    tk::result("ZC-11", ok, "modulation rapide (LFO 20 Hz sur profondeur et Shape, Z Master par CROSS OP1 audio) : fini ; crete / crete statique max %.2f (<= 4) ; saut entre echantillons / max statique %.2f (<= 1,5) ; mise a jour spectrale toutes les 128 echantillons moteur (documente)",
               ratioPk, ratioJump);
  }
  // ZC-12 : déterminisme ; Phase Scatter indépendant de la graine d'instance
  {
    PluginState a = zState("factory:metal", 8, 0.9, 0.4, 7, 0.5, -0.2), b = a;
    b.seed = 0x1234567ULL;
    const bool det = th::sameFloats(renderNote(a, 52, 24000).L, renderNote(a, 52, 24000).L);
    const bool seedFree = th::sameFloats(renderNote(a, 52, 24000).L, renderNote(b, 52, 24000).L);
    const bool ok = det && seedFree;
    TK_CHECK(ok);
    tk::result("ZC-12", ok, "determinisme : deux rendus identiques bit a bit %d ; Phase Scatter identique entre instances de graines differentes %d", det, seedFree);
  }
  // ZC-13 : intérêt musical de Phase Scatter dans les chaînes réelles
  {
    const auto saw = coefs("factory:saw", 0);
    const auto cs = run1(saw, 8, 1.0, 0.0);
    double magDev = 0.0;
    for (int h = 1; h <= 1024; ++h)
      if (std::abs(saw[h]) > 1e-12) magDev = std::max(magDev, std::fabs(20.0 * std::log10(std::abs(cs[h]) / std::abs(saw[h]))));
    const double crest = std::fabs(crestDb(synth(cs, 256)) - crestDb(synth(saw, 256)));
    // (a) sortie FM avec route CROSS WT -> OP1 (phase) : WT muette en sortie, FM audible
    auto fmCross = [](double v) {
      PluginState s = zState("factory:saw", 8, v, 0.0);
      s.params[ParamId::WtLevel] = 0.0f;
      s.params[ParamId::Op1Level] = 0.7f;
      setRoutes(s, {RouteTable::Op::add(kNodeWT, kNodeOP1)});
      s.params[ParamId::DepthWtOp1] = 0.6f;
      s.params[ParamId::CrossMaster] = 1.0f;
      s.params[ParamId::CrossFwd] = 1.0f;
      return renderNote(s, 45, 24000).L;
    };
    const double dFm = nmseDb(fmCross(1.0), fmCross(0.0));
    // (b) après Harmonic Fold en emplacement B
    auto folded = [](double v) { return renderNote(zState("factory:saw", 8, v, 0.0, 1, 1.0, 0.0), 45, 32768); };  // >= 8000 + 16384 (fenetre d analyse)
    const auto f0 = folded(0.0), f1 = folded(1.0);
    auto h64 = [](const std::vector<float>& x) {
      std::uint64_t h = 1469598103934665603ULL;
      for (float v : x) {
        std::uint32_t b;
        std::memcpy(&b, &v, 4);
        h = (h ^ b) * 1099511628211ULL;
      }
      return static_cast<unsigned long long>(h);
    };
    std::printf("  ZC-13 (b) ecart temporel fold v0/v1 : %.1f dB ; pics %.3f %.3f ; empreintes %016llx %016llx\n", nmseDb(f1.L, f0.L), peak(f0.L),
                peak(f1.L), h64(f0.L), h64(f1.L));
    std::vector<double> a0(f0.L.begin() + 8000, f0.L.begin() + 8000 + 16384), a1(f1.L.begin() + 8000, f1.L.begin() + 8000 + 16384);
    const auto m0 = an::amplitudeSpectrum(a0.data(), a0.size()), m1 = an::amplitudeSpectrum(a1.data(), a1.size());
    int bandsDiff = 0;
    for (int k = -10; k <= 12; ++k) {
      const double fc = 1000.0 * std::pow(2.0, k / 3.0), lo = fc / std::pow(2.0, 1.0 / 6.0), hi = fc * std::pow(2.0, 1.0 / 6.0);
      double e0 = 0.0, e1 = 0.0;
      for (size_t j = static_cast<size_t>(lo * 16384 / 48000); j <= static_cast<size_t>(hi * 16384 / 48000) && j < m0.size(); ++j) e0 += m0[j] * m0[j], e1 += m1[j] * m1[j];
      if (e0 > 1e-12 && e1 > 1e-12 && std::fabs(10.0 * std::log10(e1 / e0)) >= 1.0) ++bandsDiff;
    }
    const bool ok = magDev <= 0.01 && dFm >= -30.0 && bandsDiff >= 3 && crest >= 1.0;
    TK_CHECK(ok);
    tk::result("ZC-13", ok, "Phase Scatter (v = 1) : spectre d'amplitude WT inchange (ecart max %.2e dB <= 0,01) ; (a) sortie FM avec route CROSS WT>OP1 : ecart %.1f dB (>= -30) ; (b) apres Harmonic Fold (B) : %d bandes de tiers d'octave a >= 1 dB (>= 3) ; (c) facteur de crete modifie de %.2f dB (>= 1)",
               magDev, dFm, bandsDiff, crest);
  }
  // ZC-16 : stabilité (60 s, modulations aléatoires de tous les paramètres Z, deux banques) ; diagnostic de crête :
  // niveau avant ELP (prise après le gain de sortie), configuration Z active au bloc de crête, variantes de contrôle
  {
    struct Run {
      bool finite = true;
      double peakOut = 0.0, peakPre = 0.0;
      std::uint32_t nonFinite = 0, clamp = 0, elp = 0;
      std::string at;
    };
    // mode 0 : protocole ; 1 : sans Harmonic Tilt ; 2 : compensation de gain forcée ; 3 : Tilt brillant seul (sigma 1)
    auto runZ = [](const char* bank, int mode) {
      Run out;
      PluginState st = zState(bank, 4, 0.5, 0.0, 7, 0.5, 0.0);
      auto b = resolveBank(st.bank);
      Engine e(b ? b : FactoryBanks::get("factory:saw"));
      ParamValues p = st.params;
      e.setParameters(p);
      e.prepare(48000.0, 256);
      RouteTable t;
      std::string err;
      t.assign(st.routes, st.nextRouteId, err);
      e.setTopologyImmediate(t.compile());
      std::mt19937_64 rng(0x2C16ULL);
      std::uniform_real_distribution<double> u(0.0, 1.0);
      std::vector<float> L(256), R(256);
      std::vector<double> tap(256);
      const long long total = 48000LL * 60;
      for (long long pos = 0; pos < total; pos += 256) {
        if (pos % 2560 == 0) {
          auto pick = [&]() {
            int v = static_cast<int>(u(rng) * 9.0) % 9;
            if (mode == 1 && v == 4) v = 5;
            return static_cast<float>(v);
          };
          p[ParamId::WtZAType] = mode == 3 ? 4.0f : pick();
          p[ParamId::WtZBType] = mode == 3 ? 0.0f : pick();
          p[ParamId::WtZADepth] = mode == 3 ? 1.0f : static_cast<float>(u(rng));
          p[ParamId::WtZBDepth] = static_cast<float>(u(rng));
          p[ParamId::WtZAShape] = mode == 3 ? 1.0f : static_cast<float>(2.0 * u(rng) - 1.0);
          p[ParamId::WtZBShape] = static_cast<float>(2.0 * u(rng) - 1.0);
          p[ParamId::WtZ] = mode == 3 ? 1.0f : static_cast<float>(u(rng));
          p[ParamId::WtZACurve] = static_cast<float>(static_cast<int>(u(rng) * 4.0) % 4);
          p[ParamId::WtZOrder] = u(rng) < 0.5 ? 0.0f : 1.0f;
          p[ParamId::WtZFund] = u(rng) < 0.5 ? 0.0f : 1.0f;
          p[ParamId::WtZGain] = (mode == 2 || u(rng) < 0.5) ? (mode == 2 ? 1.0f : 0.0f) : 1.0f;
          if (mode == 3) p[ParamId::WtZGain] = 0.0f;
          p[ParamId::WtX] = static_cast<float>(u(rng));
          p[ParamId::WtXMorph] = static_cast<float>(static_cast<int>(u(rng) * 3.0) % 3);
        }
        e.setParameters(p);
        MidiEvent ev[1];
        int ne = 0;
        for (long long n : {0LL, 48000LL * 20, 48000LL * 40})
          if (n == pos) ev[ne++] = MidiEvent::noteOn(0, n == 0 ? 40 : (n == 48000LL * 20 ? 64 : 88), 100);
        e.process(L.data(), R.data(), 256, ne ? ev : nullptr, ne, tap.data());
        for (int i = 0; i < 256; ++i) {
          out.finite = out.finite && std::isfinite(L[i]) && std::isfinite(R[i]);
          out.peakOut = std::max(out.peakOut, static_cast<double>(std::max(std::fabs(L[i]), std::fabs(R[i]))));
          if (std::fabs(tap[i]) > out.peakPre) {
            out.peakPre = std::fabs(tap[i]);
            char s[200];
            std::snprintf(s, sizeof s, "t=%.2f s A=%s(v %.2f, s %.2f) B=%s Z %.2f fund %d gain %d", (pos + i) / 48000.0, zName(static_cast<int>(p[ParamId::WtZAType])),
                          p[ParamId::WtZADepth], p[ParamId::WtZAShape], zName(static_cast<int>(p[ParamId::WtZBType])), p[ParamId::WtZ],
                          static_cast<int>(p[ParamId::WtZFund]), static_cast<int>(p[ParamId::WtZGain]));
            out.at = s;
          }
        }
        e.exchange().collectGarbage();
      }
      out.nonFinite = e.telemetry().nonFiniteResets.load();
      out.clamp = e.telemetry().outputClampHits.load();
      out.elp = e.telemetry().elpHits.load();
      return out;
    };
    bool ok = true;
    std::string det;
    for (const char* bank : {"factory:metal", "factory:basic"}) {
      const Run r = runZ(bank, 0);
      ok = ok && r.finite && r.nonFinite == 0 && r.clamp == 0;
      char s[400];
      std::snprintf(s, sizeof s, " [%s : sortie %.3f, avant ELP %.1f (%+.1f dBFS) a %s ; ELP %u, garde %u, non-finis %u]", bank, r.peakOut, r.peakPre,
                    20.0 * std::log10(r.peakPre), r.at.c_str(), r.elp, r.clamp, r.nonFinite);
      det += s;
    }
    const Run noTilt = runZ("factory:metal", 1), forced = runZ("factory:metal", 2), tilt = runZ("factory:metal", 3);
    char s[500];
    std::snprintf(s, sizeof s, " ; controles (Metal) : sans Tilt avant ELP %.2f (%+.1f dBFS) ; compensation forcee %.2f (%+.1f dBFS) ; Tilt brillant seul (v 1, sigma 1, sans compensation) %.1f (%+.1f dBFS)",
                  noTilt.peakPre, 20.0 * std::log10(noTilt.peakPre), forced.peakPre, 20.0 * std::log10(forced.peakPre), tilt.peakPre, 20.0 * std::log10(tilt.peakPre));
    det += s;
    ok = ok && noTilt.finite && forced.finite && tilt.finite;
    TK_CHECK(ok);
    tk::result("ZC-16", ok, "stabilite 60 s x 2 banques, modulations aleatoires de tous les parametres Z toutes les 2560 echantillons, notes 40/64/88 : finis, non-finis 0, garde 0 %d ;%s", ok, det.c_str());
  }
  // ZC-17 : coût (mesure de développement, Windows ; ne certifie pas M3 / M4 Max) ; cadence de resynthèse
  {
    struct Cost {
      double p50, p95;
      long long resynth, blends, samples;
    };
    auto cost = [](PluginState s, int modulate) {  // 0 statique ; 1 LFO sur la profondeur Z ; 2 LFO sur X (morph)
      if (modulate) {
        s.params[ParamId::Lfo1Rate] = 3.0f;
        ModRoute r;
        r.source = ModSource::Lfo1;
        r.dest = modulate == 1 ? ParamId::WtZADepth : ParamId::WtX;
        r.depth = 0.4f;
        modAdd(s.mod, r);
      }
      auto bank = resolveBank(s.bank);
      Engine e(bank ? bank : FactoryBanks::get("factory:saw"));
      e.setParameters(s.params);
      e.prepare(48000.0, 256);
      RouteTable t;
      std::string err;
      t.assign(s.routes, s.nextRouteId, err);
      e.setTopologyImmediate(t.compile());
      auto cm = compileModMatrix(s.mod);
      e.setModMatrixImmediate(*cm);
      std::vector<float> L(256), R(256);
      const MidiEvent on = MidiEvent::noteOn(0, 48, 100);
      e.process(L.data(), R.data(), 256, &on, 1);
      const long long r0 = e.testSpectralResynth(), b0 = e.testSpectralBlends();
      std::vector<double> ns;
      for (int b = 0; b < 1500; ++b) {
        const auto t0 = std::chrono::steady_clock::now();
        e.process(L.data(), R.data(), 256, nullptr, 0);
        ns.push_back(std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / 256.0);
      }
      std::sort(ns.begin(), ns.end());
      return Cost{ns[ns.size() / 2], ns[ns.size() * 95 / 100], e.testSpectralResynth() - r0, e.testSpectralBlends() - b0, 1500LL * 256};
    };
    const Cost off = cost(zState("factory:metal", 0, 0.0, 0.0, 0, 0.0, 0.0, 0.0), 0);
    const Cost fold = cost(zState("factory:metal", 1, 0.7, 0.0), 0);
    const Cost stat = cost(zState("factory:metal", 7, 0.7, 0.3, 4, 0.5, 0.0), 0);
    const Cost modZ = cost(zState("factory:metal", 7, 0.7, 0.3, 4, 0.5, 0.0), 1);
    const Cost modXY = cost(zState("factory:metal", 7, 0.7, 0.3, 4, 0.5, 0.0), 2);
    const Cost modScatter = cost(zState("factory:metal", 8, 0.7, 0.3), 1);
    // cadence : au plus une resynthèse complète toutes les 128 échantillons moteur (48 kHz : K = 1)
    const long long maxResynth = off.samples / 128 + 1;
    const bool cadence = modZ.resynth <= maxResynth && modXY.resynth <= maxResynth && modScatter.resynth <= maxResynth && stat.resynth <= 1;
    const double mem = 3.0 * (zspec::kH + 2) * sizeof(cd);
    // décomposition d'une resynthèse : transformations, combinaison XY (4 formes), FFT 2048, copie du cache
    const auto metal = coefs("factory:metal", 3);
    auto bankM = FactoryBanks::get("factory:metal");
    std::vector<cd> cc(zspec::kH + 2), ww(zspec::kH + 2), fx(2048);
    double tProc[9] = {0}, tFft = 0.0, tBlend = 0.0, tCopy = 0.0;
    Fft fft(2048);
    for (int t : {4, 5, 6, 7, 8}) {
      const int zt[2] = {t, 0};
      const double zv[2] = {0.7, 0.0}, zs[2] = {0.3, 0.0};
      const auto t0 = std::chrono::steady_clock::now();
      for (int k = 0; k < 200; ++k) {
        cc = metal;
        zspec::process(cc.data(), metal.data(), ww.data(), zt, zv, zs, true, true, [](int) { return 1.0; });
      }
      tProc[t] = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0).count() / 200.0;
    }
    {
      auto t0 = std::chrono::steady_clock::now();
      for (int k = 0; k < 200; ++k) fft.transform(fx.data(), true);
      tFft = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0).count() / 200.0;
      t0 = std::chrono::steady_clock::now();
      for (int k = 0; k < 200; ++k)
        for (int h = 1; h <= zspec::kH; ++h) {
          double re = 0.0, im = 0.0;
          for (int w = 0; w < 4; ++w) {
            const double mk = bankM->specMag(w)[h - 1], pk = bankM->specPhase(w)[h - 1];
            re += 0.25 * mk * std::cos(pk);
            im += 0.25 * mk * std::sin(pk);
          }
          cc[h] = cd(re, im);
        }
      tBlend = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0).count() / 200.0;
      t0 = std::chrono::steady_clock::now();
      for (int k = 0; k < 200; ++k) std::copy(metal.begin(), metal.end(), cc.begin());
      tCopy = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0).count() / 200.0;
    }
    TK_CHECK(cadence);
    tk::result("ZC-17", cadence, "cadence : resyntheses completes Z module %lld, XY module %lld, Scatter module %lld, statique %lld pour %lld echantillons (<= %lld = 1/128) %d ; combinaisons XY recalculees : Z module %lld, XY module %lld ; cout (Windows MSVC, ne certifie pas M3/M4 Max) ns/echantillon P50/P95 : Z off %.0f/%.0f, Fold %.0f/%.0f, Shift+Tilt statique %.0f/%.0f, Shift+Tilt profondeur modulee %.0f/%.0f, Shift+Tilt XY module %.0f/%.0f, Scatter module %.0f/%.0f ; par resynthese : Tilt %.1f, Mask %.1f, Focus %.1f, Shift %.1f, Scatter %.1f us ; combinaison XY 4 formes %.1f us ; FFT 2048 %.1f us ; copie du cache %.2f us ; memoire ajoutee %.0f Ko ; 0 allocation RT (TA-20, G-18w)",
               modZ.resynth, modXY.resynth, modScatter.resynth, stat.resynth, off.samples, maxResynth, cadence, modZ.blends, modXY.blends, off.p50, off.p95, fold.p50,
               fold.p95, stat.p50, stat.p95, modZ.p50, modZ.p95, modXY.p50, modXY.p95, modScatter.p50, modScatter.p95, tProc[4], tProc[5], tProc[6], tProc[7], tProc[8],
               tBlend, tFft, tCopy, mem / 1024.0);
  }
}

}  // namespace

void runG43ZTests() {
  std::printf("[ARCH-G4.3 : transformations Z]\n");
  testZ();
}
