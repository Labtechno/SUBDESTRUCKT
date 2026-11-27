// SUBDESTRUCKT — tests ARCH-G4.3-R1 (DSP FINAL CORRECTION). Critères : docs/V1-ARCH-G4.3-R1-SPEC.md (avant mesure).
#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <random>
#include <string>
#include <vector>

#include "Analysis.h"
#include "G4Helper.h"
#include "RenderHelper.h"
#include "TestKit.h"
#include "core/StateCodec.h"
#include "dsp/ZSpectral.h"

using namespace sbd;
using namespace g4;
using cd = std::complex<double>;

namespace {

std::vector<cd> bankCoefs(const char* bank, int wave) {
  auto b = FactoryBanks::get(bank);
  std::vector<cd> c(zspec::kH + 2, cd(0.0, 0.0));
  for (int h = 1; h <= zspec::kH; ++h) c[h] = std::polar(static_cast<double>(b->specMag(wave)[h - 1]), static_cast<double>(b->specPhase(wave)[h - 1]));
  return c;
}
std::vector<cd> shifted(const std::vector<cd>& in, double k) {  // Shift seul : v = 1, sigma tel que 8 v lambda = k
  std::vector<cd> c = in, w(zspec::kH + 2);
  const double s = (k / 8.0 - 0.5) / 1.5;
  zspec::apply(ZType::Shift, c.data(), w.data(), 1.0, s);
  return c;
}
double maxAbs(const std::vector<cd>& c) {
  double m = 0.0;
  for (int h = 1; h <= zspec::kH; ++h) m = std::max(m, std::abs(c[h]));
  return m;
}

double shiftPhaseFlipJump();
// ------------------------------------------------------------------ ZC-05R : Harmonic Shift (continuité, bornes)
void testShift() {
  bool exactInt = true, interp = true, cont = true, bounds = true, nearZero = true, none = true;
  double worstJump = 0.0, worstJumpK = 0.0;
  std::string where;
  for (const char* bank : {"factory:saw", "factory:metal", "factory:basic"}) {
    const auto c = bankCoefs(bank, bank[8] == 'b' ? 2 : (bank[8] == 'm' ? 3 : 0));
    const double mx = maxAbs(c);
    for (int k : {1, 2, 5, 16, -3, -8}) {
      const auto o = shifted(c, k);
      for (int h = 1; h <= zspec::kH; ++h) {
        const int src = h - k;
        const cd want = src >= 1 && src <= zspec::kH ? c[src] : cd(0.0, 0.0);
        if (!(o[h] == want)) exactInt = false;
      }
    }
    for (double k : {0.3, 2.7, -4.45, 11.9}) {  // (b) amplitude interpolée
      const auto o = shifted(c, k);
      for (int h = 1; h <= zspec::kH; ++h) {
        const double src = h - k;
        if (src < 1.0 || src >= zspec::kH) continue;
        const int i = static_cast<int>(std::floor(src));
        const double f = src - i, want = (1.0 - f) * std::abs(c[i]) + f * std::abs(c[i + 1]);
        if (std::fabs(std::abs(o[h]) - want) > 1e-12 * (want + mx * 1e-12)) interp = false;
      }
    }
    // (c)(d) continuité en k, y compris franchissement des bornes 1 et 1024 du rang source
    for (int n = 0; n < 2000; ++n) {
      const double k = -8.0 + 24.0 * (n + 0.5) / 2000.0, d = 1e-6;
      const auto a = shifted(c, k), b = shifted(c, k + d);
      double j = 0.0;
      for (int h = 1; h <= zspec::kH; ++h) j = std::max(j, std::abs(b[h] - a[h]));
      if (j / mx > worstJump) worstJump = j / mx, worstJumpK = k;
      if (j > 1e-3 * mx) cont = false;
    }
    for (int h : {1, 2, 3, 10}) {  // source de h franchit 1 quand k = h - 1 ; de part et d'autre de la borne
      const double kb = h - 1.0;
      if (kb > 16.0) continue;
      const auto a = shifted(c, kb - 1e-6), b = shifted(c, kb + 1e-6);
      if (std::abs(a[h] - b[h]) > 1e-3 * mx) {
        bounds = false;
        where += " [" + std::string(bank) + " h" + std::to_string(h) + " source 1]";
      }
    }
    for (double kb : {-1.0, -2.0, -8.0 + 1e-3}) {  // borne haute : source 1024 pour h = 1024 + kb
      const int h = static_cast<int>(1024 + kb);
      const double kk = h - 1024.0;
      const auto a = shifted(c, kk - 1e-6), b = shifted(c, kk + 1e-6);
      if (std::abs(a[h] - b[h]) > 1e-3 * mx) {
        bounds = false;
        where += " [" + std::string(bank) + " h" + std::to_string(h) + " source 1024]";
      }
    }
    // (e) près de zéro
    for (double kz : {1e-6, -1e-6}) {
      const auto o = shifted(c, kz);
      double e = 0.0;
      for (int h = 1; h <= zspec::kH; ++h) e = std::max(e, std::abs(o[h] - c[h]));
      if (e > 1e-5 * mx) nearZero = false;
    }
    // (g) aucune composante hors source
    const auto up = shifted(c, 5.5);
    for (int h = 1; h <= 4; ++h)  // source h - 5,5 <= 0 pour h <= 5 (h = 5 : source -0,5)
      if (std::abs(up[h]) != 0.0) none = false;
  }
  // (h) modulation : LFO 20 Hz sur la profondeur, moteur réel
  auto st = [](float depth) {
    PluginState s = bareState();
    s.bank = "factory:saw";
    s.params[ParamId::Op1Level] = 0.0f;
    s.params[ParamId::WtLevel] = 0.7f;
    s.params[ParamId::WtZ] = 1.0f;
    s.params[ParamId::WtZAType] = 7.0f;
    s.params[ParamId::WtZADepth] = depth;
    return s;
  };
  auto jump = [](const std::vector<float>& x) {
    double j = 0.0;
    for (size_t i = 4801; i < x.size(); ++i) j = std::max(j, static_cast<double>(std::fabs(x[i] - x[i - 1])));
    return j;
  };
  auto render = [](const PluginState& s) {
    G4Scenario sc;
    sc.state = s;
    sc.samples = 48000;
    sc.events = {MidiEvent::noteOn(0, 48, 100)};
    return renderG4(sc);
  };
  double staticJump = 0.0;
  for (float d : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f}) staticJump = std::max(staticJump, jump(render(st(d)).L));
  PluginState m = st(0.5f);
  m.params[ParamId::Lfo1Rate] = 20.0f;
  ModRoute r;
  r.source = ModSource::Lfo1;
  r.dest = ParamId::WtZADepth;
  r.depth = 0.5f;
  modAdd(m.mod, r);
  const auto rm = render(m);
  const double mj = jump(rm.L);
  const bool modOk = allFinite(rm.L) && mj <= 1.5 * staticJump;
  const double flip = shiftPhaseFlipJump();
  const bool ok = exactInt && interp && cont && bounds && nearZero && none && modOk && flip <= 1e-3;
  TK_CHECK(ok);
  tk::result("ZC-05R", ok, "Harmonic Shift : (a) k entier exact %d ; (b) amplitude interpolee %d ; (c) continuite en k (2000 valeurs, delta 1e-6) %d, pire saut %.3g x max|c| a k = %.4f ; (d) bornes du rang source (1 et 1024) %d%s ; (e) pres de zero %d ; (g) aucune composante hors source %d ; (h) LFO 20 Hz : saut %.4f / statique %.4f (<= 1,5x) %d ; continuite au point d'annulation de la moyenne complexe (rangs de phases opposees, pas 1e-9) : saut %.3g x |c| (<= 1e-3, critere (c) en ce point)",
             exactInt, interp, cont, worstJump, worstJumpK, bounds, where.c_str(), nearZero, none, mj, staticJump, modOk, flip);
}

// diagnostic (descriptif) : inversion de phase de la moyenne complexe au point exact où (1-f)|a| = f|b|, a et b opposés
double shiftPhaseFlipJump() {
  const auto c = bankCoefs("factory:saw", 0);
  double worst = 0.0;
  for (int i = 1; i < 40; ++i) {
    const cd a = c[i], b = c[i + 1];
    const double pa = std::abs(a), pb = std::abs(b);
    if (pa < 1e-12 || pb < 1e-12) continue;
    const double fstar = pa / (pa + pb);  // z = (1-f)a + f b s'annule si a et b sont opposés
    const int h = i + 5;                  // rang de sortie dont la source est i + f avec k = h - i - f
    const double k0 = h - (i + fstar);
    const auto lo = shifted(c, k0 - 1e-9), hi = shifted(c, k0 + 1e-9);
    worst = std::max(worst, std::abs(hi[h] - lo[h]) / std::max(pa, pb));
  }
  return worst;
}

// ------------------------------------------------------------------ outils : crête indépendante, rendu avec prise avant ELP
// crête du cycle à bande lue resynthétisé (implémentation indépendante du moteur : an::fft)
struct TestPeak {
  const std::vector<double>* band;
  double operator()(const cd* q) const {
    std::vector<cd> x(2048, cd(0.0, 0.0));
    for (int h = 1; h <= zspec::kH; ++h) {
      const double g = (*band)[static_cast<size_t>(h)];
      if (g <= 0.0) continue;
      x[static_cast<size_t>(h)] = q[h] * g;
      if (h < 1024) x[static_cast<size_t>(2048 - h)] = std::conj(q[h] * g);
    }
    for (auto& v : x) v = std::conj(v);  // FFT directe de conj -> inverse
    an::fft(x);
    double pk = 0.0;
    for (const auto& v : x) pk = std::max(pk, std::fabs(v.real() * 0.5));
    return pk;
  }
};
std::vector<double> bandProfile(int kind) {  // 0 : 1024 ; 1 : H = 64 ; 2 : H = 8 ; 3 : profil mip partiel (w = 0,4)
  std::vector<double> b(zspec::kH + 2, 0.0);
  for (int h = 1; h <= zspec::kH; ++h) {
    if (kind == 0) b[h] = 1.0;
    if (kind == 1) b[h] = h <= 64 ? 1.0 : 0.0;
    if (kind == 2) b[h] = h <= 8 ? 1.0 : 0.0;
    if (kind == 3) b[h] = h <= 32 ? 1.0 : (h <= 64 ? 0.6 : 0.0);
  }
  return b;
}
double bandEnergy(const std::vector<cd>& c, const std::vector<double>& b, int h0) {
  double e = 0.0;
  for (int h = h0; h <= zspec::kH; ++h) e += b[h] * b[h] * std::norm(c[h]);
  return e;
}
struct TiltRun {
  std::vector<cd> out;
  double ein = 0.0, eout = 0.0, pin = 0.0, pout = 0.0;
  int reductions = 0;
  bool finite = true;
};
TiltRun tiltOnly(const std::vector<cd>& in, double v, double s, bool fund, const std::vector<double>& band, bool gainComp = false) {
  TiltRun r;
  r.out = in;
  std::vector<cd> w(zspec::kH + 2);
  const int zt[2] = {4, 0};
  const double zv[2] = {v, 0.0}, zs[2] = {s, 0.0};
  TestPeak pk{&band};
  zspec::StageStats st;
  zspec::process(r.out.data(), in.data(), w.data(), zt, zv, zs, fund, gainComp, [&band](int h) { return band[static_cast<size_t>(h)]; }, pk, &st);
  const int h0 = fund ? 2 : 1;
  r.ein = bandEnergy(in, band, h0);
  r.eout = bandEnergy(r.out, band, h0);
  r.pin = pk(in.data());
  r.pout = pk(r.out.data());
  r.reductions = st.peakReductions;
  for (int h = 1; h <= zspec::kH; ++h) r.finite = r.finite && std::isfinite(r.out[h].real()) && std::isfinite(r.out[h].imag());
  return r;
}

PluginState wtState(const char* bank, int ta, double da, double sa, int tb = 0, double db = 0.0, double sb = 0.0, bool gain = true, bool fund = true) {
  PluginState s = bareState();
  s.bank = bank;
  s.params[ParamId::Op1Level] = 0.0f;
  s.params[ParamId::WtLevel] = 0.7f;
  s.params[ParamId::WtX] = 0.37f;
  s.params[ParamId::WtZ] = 1.0f;
  s.params[ParamId::WtZAType] = static_cast<float>(ta);
  s.params[ParamId::WtZADepth] = static_cast<float>(da);
  s.params[ParamId::WtZAShape] = static_cast<float>(sa);
  s.params[ParamId::WtZBType] = static_cast<float>(tb);
  s.params[ParamId::WtZBDepth] = static_cast<float>(db);
  s.params[ParamId::WtZBShape] = static_cast<float>(sb);
  s.params[ParamId::WtZGain] = gain ? 1.0f : 0.0f;
  s.params[ParamId::WtZFund] = fund ? 1.0f : 0.0f;
  return s;
}
struct TapRun {
  std::vector<float> L;
  double prePeak = 0.0;
  std::uint32_t elp = 0, clamp = 0;
  long long tiltReductions = 0;
};
// rendu direct du moteur avec la prise avant DC/ELP (après le gain de sortie)
TapRun renderTap(const PluginState& st, int note, double secs, const std::function<void(ParamValues&, long long)>& hook = {}) {
  TapRun r;
  auto b = resolveBank(st.bank);
  Engine e(b ? b : FactoryBanks::get("factory:saw"));
  ParamValues p = st.params;
  e.setParameters(p);
  e.prepare(48000.0, 256);
  RouteTable t;
  std::string err;
  t.assign(st.routes, st.nextRouteId, err);
  e.setTopologyImmediate(t.compile());
  auto cm = compileModMatrix(st.mod);
  e.setModMatrixImmediate(*cm);
  std::vector<float> L(256), R(256);
  std::vector<double> tap(256);
  const long long total = static_cast<long long>(secs * 48000.0);
  for (long long pos = 0; pos < total; pos += 256) {
    if (hook) hook(p, pos);
    e.setParameters(p);
    MidiEvent on = MidiEvent::noteOn(0, note, 100);
    e.process(L.data(), R.data(), 256, pos == 0 ? &on : nullptr, pos == 0 ? 1 : 0, tap.data());
    for (int i = 0; i < 256; ++i) r.prePeak = std::max(r.prePeak, std::fabs(tap[static_cast<size_t>(i)]));
    r.L.insert(r.L.end(), L.begin(), L.end());
    e.exchange().collectGarbage();
    e.modExchange().collectGarbage();
  }
  r.elp = e.telemetry().elpHits.load();
  r.clamp = e.telemetry().outputClampHits.load();
  r.tiltReductions = e.testTiltReductions();
  return r;
}
double db(double x) { return 20.0 * std::log10(std::max(x, 1e-300)); }

// ------------------------------------------------------------------ HT-01..05 : Harmonic Tilt à énergie constante
void testTilt() {
  // HT-01 : neutralité
  {
    int same = 0, n = 0;
    for (int fund : {0, 1})
      for (int gain : {0, 1})
        for (int slot : {0, 1}) {
          PluginState a = slot ? wtState("factory:metal", 0, 0.0, 0.0, 4, 0.0, 0.7, gain, fund) : wtState("factory:metal", 4, 0.0, 0.7, 0, 0.0, 0.0, gain, fund);
          PluginState z = a;
          z.params[ParamId::WtZ] = 0.0f;
          PluginState m = a;  // Z Master = 0 avec profondeur non nulle
          if (slot) m.params[ParamId::WtZBDepth] = 0.9f; else m.params[ParamId::WtZADepth] = 0.9f;
          m.params[ParamId::WtZ] = 0.0f;
          PluginState off = wtState("factory:metal", 0, 0.0, 0.0, 0, 0.0, 0.0, gain, fund);
          off.params[ParamId::WtZ] = 0.0f;
          const auto ro = renderTap(off, 45, 0.5).L;
          same += th::sameFloats(renderTap(a, 45, 0.5).L, ro) && th::sameFloats(renderTap(m, 45, 0.5).L, ro);
          ++n;
        }
    const bool ok = same == n;
    TK_CHECK(ok);
    tk::result("HT-01", ok, "neutralite : Depth = 0 et Z Master = 0 (Keep Fundamental 0/1, Gain Compensation 0/1, emplacements A/B) : bit a bit = moteur sans Z %d/%d", same, n);
  }
  // HT-02, HT-03, HT-05 (a) : 4 banques x 4 formes x 5 profondeurs x 5 Shape x fondamentale x 4 profils de bande
  {
    long long cases = 0, reductions = 0, energyBad = 0, fundBad = 0, propBad = 0, peakBad = 0;
    double worstE = 0.0, worstPeakRatio = 0.0;
    for (const char* bank : {"factory:basic", "factory:metal", "factory:formant", "factory:harmonic"}) {
      const int nw = FactoryBanks::get(bank)->numWaves();
      for (int wv = 0; wv < 4; ++wv) {
        const auto c = bankCoefs(bank, wv % nw);
        for (double v : {0.05, 0.25, 0.5, 0.75, 1.0})
          for (double s : {-1.0, -0.5, 0.0, 0.5, 1.0})
            for (int fund : {0, 1})
              for (int bp = 0; bp < 4; ++bp) {
                const auto band = bandProfile(bp);
                const TiltRun r = tiltOnly(c, v, s, fund == 1, band);
                ++cases;
                reductions += r.reductions > 0;
                if (r.reductions == 0) {
                  const double e = std::fabs(r.eout / r.ein - 1.0);
                  worstE = std::max(worstE, e);
                  if (!(e <= 1e-9)) ++energyBad;
                } else if (!(r.eout <= r.ein * (1.0 + 1e-12))) {
                  ++energyBad;
                }
                if (fund == 1 && !(r.out[1] == c[1])) ++fundBad;
                if (fund == 0 && r.reductions == 0) {  // facteur commun s
                  double s0 = 0.0;
                  for (int h = 1; h <= zspec::kH; ++h) {
                    const cd ref = c[h] * zspec::gain(ZType::Tilt, h, v, s);
                    if (std::abs(ref) < 1e-12) continue;
                    const double q = std::abs(r.out[h]) / std::abs(ref);
                    if (s0 == 0.0) s0 = q;
                    if (std::fabs(q / s0 - 1.0) > 1e-9) {
                      ++propBad;
                      break;
                    }
                  }
                }
                worstPeakRatio = std::max(worstPeakRatio, r.pout / std::max(r.pin, 1e-300));
                if (r.pout > zspec::kTiltPeakRatio * r.pin * (1.0 + 1e-9)) ++peakBad;
              }
      }
    }
    const bool h2 = energyBad == 0;
    TK_CHECK(h2);
    tk::result("HT-02", h2, "energie (Tilt seul, %lld cas) : sans reduction de crete |Eout/Ein - 1| max %.2e (<= 1e-9) ; avec reduction (%lld cas) : Eout <= Ein ; ecarts %lld",
               cases, worstE, reductions, energyBad);
    const bool h3 = fundBad == 0 && propBad == 0;
    TK_CHECK(h3);
    tk::result("HT-03", h3, "fondamentale : ON c1 == c1 d'entree (ecarts %lld) ; OFF : c''h = s . h^(v lambda) . ch avec s commun (ecarts %lld)", fundBad, propBad);
    const bool h5a = peakBad == 0;
    TK_CHECK(h5a);
    tk::result("HT-05a", h5a, "cretes (cas HT-02) : crete du cycle de sortie <= 2 x crete d'entree (+6,02 dB) : depassements %lld ; pire rapport %.4f (%.2f dB) ; reductions de crete %lld/%lld cas",
               peakBad, worstPeakRatio, db(worstPeakRatio), reductions, cases);
  }
  // HT-04 : cas limites
  {
    std::vector<std::pair<std::string, std::vector<cd>>> cs;
    cs.push_back({"nul", std::vector<cd>(zspec::kH + 2, cd(0.0, 0.0))});
    auto saw = bankCoefs("factory:saw", 0);
    auto tiny = saw;
    for (auto& x : tiny) x *= 1e-150;
    cs.push_back({"quasi nul", tiny});
    std::vector<cd> sine(zspec::kH + 2, cd(0.0, 0.0));
    sine[1] = cd(0.0, -1.0);
    cs.push_back({"sinus", sine});
    cs.push_back({"dent de scie", saw});
    cs.push_back({"Metal (creux)", bankCoefs("factory:metal", 5)});
    std::mt19937_64 rng(0x4704ULL);
    std::uniform_real_distribution<double> u(0.0, 1.0);
    std::vector<cd> dense(zspec::kH + 2, cd(0.0, 0.0));
    for (int h = 1; h <= zspec::kH; ++h) dense[h] = std::polar(u(rng) / std::sqrt(static_cast<double>(h)), 6.283185307179586 * u(rng));
    cs.push_back({"dense aleatoire (forme utilisateur)", dense});
    std::vector<cd> top(zspec::kH + 2, cd(0.0, 0.0));
    top[zspec::kH] = cd(0.3, 0.1);
    top[1] = cd(0.0, 0.2);
    cs.push_back({"rang 1024", top});
    bool ok = true;
    std::string det;
    for (const auto& cc : cs)
      for (int bp : {0, 2})
        for (int H : {0, 1, 2}) {
          auto band = bandProfile(bp);
          if (H > 0)
            for (int h = H + 1; h <= zspec::kH; ++h) band[h] = 0.0;
          for (double v : {0.3, 1.0})
            for (double s : {-1.0, 1.0})
              for (int fund : {0, 1}) {
                const TiltRun r = tiltOnly(cc.second, v, s, fund == 1, band);
                bool good = r.finite && r.eout <= r.ein * (1.0 + 1e-9) + 1e-300 && r.pout <= zspec::kTiltPeakRatio * r.pin * (1.0 + 1e-9) + 1e-300;
                if (cc.first == "nul")
                  for (int h = 1; h <= zspec::kH; ++h) good = good && r.out[h] == cd(0.0, 0.0);
                if (!good && det.size() < 400) det += " [" + cc.first + " H" + std::to_string(H) + " v" + std::to_string(v).substr(0, 3) + "]";
                ok = ok && good;
              }
        }
    TK_CHECK(ok);
    tk::result("HT-04", ok, "cas limites (nul, quasi nul 1e-150, sinus, dent de scie, Metal, dense aleatoire 1024 harmoniques, rang 1024 ; bandes 1024/8 et H = 1/2 ; v 0,3/1 ; sigma -1/1 ; fondamentale 0/1) : finis, energie non creee, crete <= 2 x entree, spectre nul -> nul exact %d%s",
               ok, det.c_str());
  }
  // HT-05 (b) scénario isolé de référence ; (c) rejeu des protocoles historiques (ZC-16)
  {
    const TapRun ref = renderTap(wtState("factory:metal", 0, 0.0, 0.0, 0, 0.0, 0.0, false), 45, 2.0);
    const TapRun iso = renderTap(wtState("factory:metal", 4, 1.0, 1.0, 0, 0.0, 0.0, false), 45, 2.0);
    const bool b = iso.prePeak <= ref.prePeak * std::pow(10.0, (6.0206 + 0.1) / 20.0) && iso.elp == 0;
    auto replay = [](const char* bank, int mode) {  // mode 0 : protocole ZC-16 ; 2 : compensation forcée ; 3 : Tilt brillant seul
      PluginState st = wtState(bank, 4, 0.5, 0.0, 7, 0.5, 0.0);
      std::mt19937_64 rng(0x2C16ULL);
      std::uniform_real_distribution<double> u(0.0, 1.0);
      TapRun out;
      double pk = 0.0;
      std::uint32_t elp = 0, clamp = 0;
      for (int note : {40, 64, 88}) {
        const TapRun r = renderTap(st, note, 20.0, [&](ParamValues& p, long long pos) {
          if (pos % 2560 != 0) return;
          auto pick = [&]() { return static_cast<float>(static_cast<int>(u(rng) * 9.0) % 9); };
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
          const bool g = u(rng) < 0.5;
          p[ParamId::WtZGain] = mode == 3 ? 0.0f : (mode == 2 ? 1.0f : (g ? 1.0f : 0.0f));
          p[ParamId::WtX] = static_cast<float>(u(rng));
          p[ParamId::WtXMorph] = static_cast<float>(static_cast<int>(u(rng) * 3.0) % 3);
        });
        pk = std::max(pk, r.prePeak);
        elp += r.elp;
        clamp += r.clamp;
      }
      out.prePeak = pk;
      out.elp = elp;
      out.clamp = clamp;
      return out;
    };
    const TapRun m0 = replay("factory:metal", 0), b0 = replay("factory:basic", 0), f2 = replay("factory:metal", 2), t3 = replay("factory:metal", 3);
    const bool c = m0.elp == 0 && b0.elp == 0 && f2.elp == 0 && t3.elp == 0 && m0.clamp == 0 && b0.clamp == 0 && f2.clamp == 0 && t3.clamp == 0;
    TK_CHECK(b && c);
    tk::result("HT-05", b && c, "(b) scenario isole (Metal, note 45, Tilt v 1 sigma 1, sans compensation) : crete avant ELP %.2f dBFS vs Z off %.2f dBFS (<= +6,12 dB), ELP %u ; reductions de crete %lld %d ; (c) rejeu des protocoles historiques (historique avant ELP : +40,9 / +46,8 / +30,3 / +78,9 dBFS) : Metal %.2f dBFS (ELP %u), Basic %.2f (ELP %u), compensation forcee %.2f (ELP %u), Tilt brillant seul %.2f (ELP %u) ; garde 0 %d",
               db(iso.prePeak), db(ref.prePeak), iso.elp, iso.tiltReductions, b, db(m0.prePeak), m0.elp, db(b0.prePeak), b0.elp, db(f2.prePeak), f2.elp, db(t3.prePeak), t3.elp, c);
  }
}

// ------------------------------------------------------------------ HT-06 composition ; HT-07 modulation ; HT-08 coût
void testTiltComposition() {
  bool ok = true;
  std::string det;
  int n = 0;
  for (int other : {1, 2, 3, 4, 5, 6, 7, 8})
    for (int order : {0, 1})
      for (double s : {-1.0, 1.0}) {
        PluginState a = wtState("factory:metal", 4, 0.8, s, other, 0.8, other == 4 ? -s : 0.3);
        a.params[ParamId::WtZOrder] = static_cast<float>(order);
        PluginState r0 = a;
        r0.params[ParamId::WtZADepth] = 0.0f;
        int tilts = 1;
        if (other == 4) {
          r0.params[ParamId::WtZBDepth] = 0.0f;
          tilts = 2;
        }
        const TapRun ra = renderTap(a, 45, 1.0), rr = renderTap(r0, 45, 1.0);
        const double bound = rr.prePeak * std::pow(10.0, (6.0206 * tilts + 0.1) / 20.0);
        const bool good = allFinite(ra.L) && ra.elp == 0 && ra.prePeak <= bound;
        ok = ok && good;
        ++n;
        if (!good) {
          char b[160];
          std::snprintf(b, sizeof b, " [Tilt+%d ordre %d sigma %.0f : %.2f dB vs ref %.2f dB, ELP %u]", other, order, s, db(ra.prePeak), db(rr.prePeak), ra.elp);
          det += b;
        }
      }
  TK_CHECK(ok);
  tk::result("HT-06", ok, "composition : Tilt (A, v 0,8) + chacune des 7 autres transformations et Tilt + Tilt, ordres A>B et B>A, sigma -1/1 (%d configurations) : finis, 0 ELP, crete avant ELP <= reference (Tilt a 0) + 6,02 dB par etage Tilt%s",
             n, det.empty() ? " : conforme" : det.c_str());
}

void testTiltModulation() {
  auto jump = [](const std::vector<float>& x) {
    double j = 0.0;
    for (size_t i = 4801; i < x.size(); ++i) j = std::max(j, static_cast<double>(std::fabs(x[i] - x[i - 1])));
    return j;
  };
  auto winRatio = [](const std::vector<float>& x) {
    double worst = 1.0, prev = -1.0;
    for (size_t a = 4800; a + 240 <= x.size() - 4800; a += 240) {
      const double r = rms(x, a, a + 240);
      if (prev > 1e-6 && r > 1e-6) worst = std::max(worst, std::max(r / prev, prev / r));
      prev = r;
    }
    return worst;
  };
  double staticJump = 0.0, staticWin = 1.0;
  for (double v : {0.0, 0.25, 0.5, 0.75, 1.0})
    for (double s : {-1.0, 0.0, 1.0}) {
      const auto r = renderTap(wtState("factory:metal", 4, v, s), 45, 1.0).L;
      staticJump = std::max(staticJump, jump(r));
      staticWin = std::max(staticWin, winRatio(r));
    }
  struct Case {
    const char* name;
    ParamId dest;
    float rate, depth;
    bool cross;
  };
  const Case cases[5] = {{"Depth LFO 0,5 Hz", ParamId::WtZADepth, 0.5f, 0.5f, false}, {"Shape LFO 0,5 Hz", ParamId::WtZAShape, 0.5f, 0.5f, false},
                         {"Z Master LFO 0,5 Hz", ParamId::WtZ, 0.5f, 0.5f, false}, {"Depth LFO 20 Hz", ParamId::WtZADepth, 20.0f, 0.5f, false},
                         {"CROSS OP1>WT.Z (audio)", ParamId::WtZ, 0.0f, 0.0f, true}};
  bool ok = true;
  std::string det;
  for (const auto& c : cases) {
    PluginState m = wtState("factory:metal", 4, 0.5, 0.0);
    m.params[ParamId::WtZ] = 0.5f;
    if (!c.cross) {
      m.params[ParamId::Lfo1Rate] = c.rate;
      ModRoute r;
      r.source = ModSource::Lfo1;
      r.dest = c.dest;
      r.depth = c.depth;
      modAdd(m.mod, r);
    } else {
      setRoutes(m, {RouteTable::Op::add(kNodeOP1, kPortWtZ)});
      m.params[ParamId::DepthOp1WtZ] = 0.5f;
      m.params[ParamId::Op1Ratio] = 3.0f;
      m.params[ParamId::CrossMaster] = 1.0f;
      m.params[ParamId::CrossRev] = 1.0f;
    }
    const auto r = renderTap(m, 45, 2.5);
    const double j = jump(r.L), w = winRatio(r.L);
    const bool good = allFinite(r.L) && j <= 1.5 * staticJump && w <= 2.0 && r.elp == 0;
    ok = ok && good;
    char b[200];
    std::snprintf(b, sizeof b, " [%s : saut %.3f (statique %.3f), fenetres 5 ms x%.2f, ELP %u%s]", c.name, j, staticJump, w, r.elp, good ? "" : " ECHEC");
    det += b;
  }
  TK_CHECK(ok);
  tk::result("HT-07", ok, "modulation de Harmonic Tilt : finis, saut entre echantillons <= 1,5 x statique, rapport entre fenetres de 5 ms consecutives <= 2 (statique max x%.2f), 0 ELP :%s",
             staticWin, det.c_str());
}

void testTiltCost() {
  auto cost = [](PluginState s, bool modulate) {
    if (modulate) {
      s.params[ParamId::Lfo1Rate] = 3.0f;
      ModRoute r;
      r.source = ModSource::Lfo1;
      r.dest = ParamId::WtZADepth;
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
    std::vector<double> ns;
    for (int b = 0; b < 1500; ++b) {
      const auto t0 = std::chrono::steady_clock::now();
      e.process(L.data(), R.data(), 256, nullptr, 0);
      ns.push_back(std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / 256.0);
    }
    std::sort(ns.begin(), ns.end());
    return std::pair<double, double>{ns[ns.size() / 2], ns[ns.size() * 95 / 100]};
  };
  const auto off = cost(wtState("factory:metal", 0, 0.0, 0.0, 0, 0.0, 0.0, true, true), false);
  const auto stat = cost(wtState("factory:metal", 4, 0.7, 0.3), false);
  const auto mod = cost(wtState("factory:metal", 4, 0.7, 0.3), true);
  const auto st2 = cost(wtState("factory:metal", 7, 0.7, 0.3, 4, 0.5, 0.0), false);
  const auto mod2 = cost(wtState("factory:metal", 7, 0.7, 0.3, 4, 0.5, 0.0), true);
  // une resynthèse : étage Tilt (pondération, normalisation, deux mesures de crête) — implémentation de test (an::fft)
  const auto metal = bankCoefs("factory:metal", 3);
  const auto band = bandProfile(0);
  const auto t0 = std::chrono::steady_clock::now();
  for (int k = 0; k < 200; ++k) tiltOnly(metal, 0.7, 0.3, true, band);
  const double us = std::chrono::duration<double, std::micro>(std::chrono::steady_clock::now() - t0).count() / 200.0;
  tk::result("HT-08", true, "cout (Windows MSVC, meme machine que la mesure 0.3.2 ; ne certifie pas M3/M4 Max) ns/echantillon P50/P95 : Z off %.0f/%.0f ; Tilt statique %.0f/%.0f ; Tilt profondeur modulee %.0f/%.0f ; Shift+Tilt statique %.0f/%.0f ; Shift+Tilt profondeur modulee %.0f/%.0f ; etage Tilt complet (harnais, 2 mesures de crete) %.1f us ; memoire ajoutee : tampon de crete 2048 complexes (32 Ko) par instance",
             off.first, off.second, stat.first, stat.second, mod.first, mod.second, st2.first, st2.second, mod2.first, mod2.second, us);
}

// ------------------------------------------------------------------ ZC-13R : Phase Scatter (critère prospectif)
void testScatter() {
  const auto saw = bankCoefs("factory:saw", 0);
  std::vector<cd> cs = saw, w(zspec::kH + 2);
  zspec::apply(ZType::Scatter, cs.data(), w.data(), 1.0, 0.0);
  double magDev = 0.0;
  for (int h = 1; h <= zspec::kH; ++h)
    if (std::abs(saw[h]) > 1e-12) magDev = std::max(magDev, std::fabs(db(std::abs(cs[h]) / std::abs(saw[h]))));
  auto synth64 = [](const std::vector<cd>& c) {
    std::vector<double> y(2048, 0.0);
    for (int i = 0; i < 2048; ++i)
      for (int h = 1; h <= 64; ++h) y[static_cast<size_t>(i)] += std::abs(c[h]) * std::cos(6.283185307179586 * h * i / 2048.0 + std::arg(c[h]));
    return y;
  };
  const auto ya = synth64(cs), yb = synth64(saw);
  double num = 0.0, den = 0.0, pa = 0.0, pb = 0.0;
  for (size_t i = 0; i < ya.size(); ++i) num += (ya[i] - yb[i]) * (ya[i] - yb[i]), den += yb[i] * yb[i], pa = std::max(pa, std::fabs(ya[i])), pb = std::max(pb, std::fabs(yb[i]));
  const double timeDb = 10.0 * std::log10(num / den), crest = db(pa / std::sqrt(den / ya.size())) - db(pb / std::sqrt(den / ya.size()));
  auto fmCross = [](double v) {
    PluginState s = wtState("factory:saw", 8, v, 0.0);
    s.params[ParamId::WtLevel] = 0.0f;
    s.params[ParamId::Op1Level] = 0.7f;
    setRoutes(s, {RouteTable::Op::add(kNodeWT, kNodeOP1)});
    s.params[ParamId::DepthWtOp1] = 0.6f;
    s.params[ParamId::CrossMaster] = 1.0f;
    s.params[ParamId::CrossFwd] = 1.0f;
    return renderTap(s, 45, 0.5).L;
  };
  const double dFm = nmseDb(fmCross(1.0), fmCross(0.0));
  auto bandsDiff = [](const std::vector<float>& x0, const std::vector<float>& x1) {
    std::vector<double> a0(x0.begin() + 8000, x0.begin() + 8000 + 16384), a1(x1.begin() + 8000, x1.begin() + 8000 + 16384);
    const auto m0 = an::amplitudeSpectrum(a0.data(), a0.size()), m1 = an::amplitudeSpectrum(a1.data(), a1.size());
    int n = 0;
    for (int k = -10; k <= 12; ++k) {
      const double fc = 1000.0 * std::pow(2.0, k / 3.0), lo = fc / std::pow(2.0, 1.0 / 6.0), hi = fc * std::pow(2.0, 1.0 / 6.0);
      double e0 = 0.0, e1 = 0.0;
      for (size_t j = static_cast<size_t>(lo * 16384 / 48000); j <= static_cast<size_t>(hi * 16384 / 48000) && j < m0.size(); ++j) e0 += m0[j] * m0[j], e1 += m1[j] * m1[j];
      if (e0 > 1e-12 && e1 > 1e-12 && std::fabs(10.0 * std::log10(e1 / e0)) >= 1.0) ++n;
    }
    return n;
  };
  const int afterFold = bandsDiff(renderTap(wtState("factory:saw", 8, 0.0, 0.0, 1, 1.0, 0.0), 45, 0.7).L, renderTap(wtState("factory:saw", 8, 1.0, 0.0, 1, 1.0, 0.0), 45, 0.7).L);
  auto n1 = [](double v) {
    PluginState s = wtState("factory:saw", 8, v, 0.0);
    s.params[ParamId::DstGl1Type] = 1.0f;
    s.params[ParamId::DstGl1N1Drive] = 18.0f;
    s.params[ParamId::DstGl1Mix] = 1.0f;
    return renderTap(s, 45, 0.7).L;
  };
  const int afterN1 = bandsDiff(n1(0.0), n1(1.0));
  PluginState a = wtState("factory:metal", 8, 0.9, 0.4), b = a;
  b.seed = 0x1234567ULL;
  const bool det = th::sameFloats(renderTap(a, 52, 0.5).L, renderTap(a, 52, 0.5).L) && th::sameFloats(renderTap(a, 52, 0.5).L, renderTap(b, 52, 0.5).L);
  PluginState z0 = wtState("factory:metal", 8, 0.0, 0.4), off = wtState("factory:metal", 0, 0.0, 0.0);
  off.params[ParamId::WtZ] = 0.0f;
  const bool neutral = th::sameFloats(renderTap(z0, 52, 0.5).L, renderTap(off, 52, 0.5).L);
  double sj = 0.0;
  for (double v : {0.0, 0.5, 1.0}) {
    const auto r = renderTap(wtState("factory:metal", 8, v, 0.0), 45, 1.0).L;
    for (size_t i = 4801; i < r.size(); ++i) sj = std::max(sj, static_cast<double>(std::fabs(r[i] - r[i - 1])));
  }
  PluginState m = wtState("factory:metal", 8, 0.5, 0.0);
  m.params[ParamId::Lfo1Rate] = 20.0f;
  ModRoute r;
  r.source = ModSource::Lfo1;
  r.dest = ParamId::WtZADepth;
  r.depth = 0.5f;
  modAdd(m.mod, r);
  const auto rm = renderTap(m, 45, 1.0).L;
  double mj = 0.0;
  for (size_t i = 4801; i < rm.size(); ++i) mj = std::max(mj, static_cast<double>(std::fabs(rm[i] - rm[i - 1])));
  const bool modOk = allFinite(rm) && mj <= 1.5 * sj;
  const bool ok = det && magDev <= 0.01 && timeDb >= -20.0 && dFm >= -30.0 && afterFold >= 3 && afterN1 >= 3 && modOk && neutral;
  TK_CHECK(ok);
  tk::result("ZC-13R", ok, "Phase Scatter : (a) deterministe et independant de la graine %d ; (b) spectre d'amplitude inchange (%.1e dB) ; (c) forme temporelle modifiee %.1f dB (>= -20) ; (d) FM via CROSS %.1f dB (>= -30) ; (e) non-linearites : apres Fold %d bandes, apres N1 %d bandes (>= 3) ; (f) LFO 20 Hz : saut %.3f / statique %.3f %d ; (g) neutralite %d ; facteur de crete (descriptif) %+.2f dB",
             det, magDev, timeDb, dFm, afterFold, afterN1, mj, sj, modOk, neutral, crest);
}

// ------------------------------------------------------------------ ZA-01 cadence ; ZA-02 matrice d'ordre 8 x 8
void testZAudit() {
  std::string table;
  bool counted = true;
  for (double fs : {44100.0, 48000.0, 88200.0, 96000.0})
    for (Quality q : {Quality::Eco, Quality::Normal, Quality::High}) {
      const int K = Engine::oversamplingFor(fs, q);
      const double fsE = fs * K, period = 128.0 / fsE * 1000.0;
      char b[120];
      std::snprintf(b, sizeof b, " [%.1f kHz %s : K %d, maj %.2f ms (%.0f Hz), reaction <= %.2f ms]", fs / 1000.0, q == Quality::Eco ? "Eco" : (q == Quality::Normal ? "Normal" : "High"),
                    K, period, fsE / 128.0, 2.0 * period);
      table += b;
      if (fs == 48000.0 && q == Quality::Normal) {  // vérification par compteur
        PluginState m = wtState("factory:metal", 5, 0.5, 0.0, 0, 0.0, 0.0, false);
        m.params[ParamId::Lfo1Rate] = 3.0f;
        ModRoute r;
        r.source = ModSource::Lfo1;
        r.dest = ParamId::WtZADepth;
        r.depth = 0.4f;
        modAdd(m.mod, r);
        auto bank = resolveBank(m.bank);
        Engine e(bank);
        e.setParameters(m.params);
        e.prepare(fs, 256, q);
        RouteTable t;
        e.setTopologyImmediate(t.compile());
        auto cm = compileModMatrix(m.mod);
        e.setModMatrixImmediate(*cm);
        std::vector<float> L(256), R(256);
        const MidiEvent on = MidiEvent::noteOn(0, 45, 100);
        e.process(L.data(), R.data(), 256, &on, 1);
        const long long r0 = e.testSpectralResynth();
        for (int k = 0; k < 375; ++k) e.process(L.data(), R.data(), 256, nullptr, 0);
        counted = e.testSpectralResynth() - r0 <= 375LL * 256 / 128 + 1;
      }
    }
  // réponse à un échelon de profondeur (Mask, sans compensation) : temps jusqu'à 90 % du changement d'énergie
  PluginState s = wtState("factory:saw", 5, 0.0, -1.0, 0, 0.0, 0.0, false);
  const auto step = renderTap(s, 45, 1.0, [](ParamValues& p, long long pos) { p[ParamId::WtZADepth] = pos >= 24000 ? 1.0f : 0.0f; }).L;
  const double e0 = rms(step, 14000, 23000), e1 = rms(step, 38000, 47000);
  double t90 = -1.0;
  const size_t per = 436;  // une période de la note 45 (110 Hz à 48 kHz) : RMS indépendante de la phase de l'onde
  for (size_t a = 24000 - per / 2; a + per < step.size(); a += 24) {
    const double e = rms(step, a, a + per);
    if (std::fabs(e - e0) >= 0.9 * std::fabs(e1 - e0)) {
      t90 = (static_cast<double>(a) + per / 2.0 - 24000.0) / 48.0;
      break;
    }
  }
  // atténuation d'une modulation sinusoïdale de profondeur (bandes latérales à f0 +- fm, harmonique 1..5), Mask vs Fold
  auto sidebands = [](int type, float fm) {
    PluginState m = wtState("factory:saw", type, 0.5, -1.0, 0, 0.0, 0.0, false);
    m.params[ParamId::Lfo1Rate] = fm;
    ModRoute r;
    r.source = ModSource::Lfo1;
    r.dest = ParamId::WtZADepth;
    r.depth = 0.5f;
    modAdd(m.mod, r);
    const auto x = renderTap(m, 45, 1.6).L;
    std::vector<double> seg(x.begin() + 8000, x.begin() + 8000 + 65536);
    const auto mag = an::amplitudeSpectrum(seg.data(), seg.size());
    const double bin = 48000.0 / 65536.0, f0 = 110.0;
    double side = 0.0, car = 0.0;
    for (int h = 1; h <= 5; ++h) {
      car += std::pow(an::lineAmplitude(mag, bin, h * f0), 2);
      side += std::pow(an::lineAmplitude(mag, bin, h * f0 + fm), 2) + std::pow(an::lineAmplitude(mag, bin, h * f0 - fm), 2);
    }
    return 10.0 * std::log10(side / car);
  };
  std::string sb;
  for (float fm : {10.0f, 50.0f, 200.0f}) {
    char b[100];
    std::snprintf(b, sizeof b, " [%g Hz : Mask %.1f dB, Fold %.1f dB]", fm, sidebands(5, fm), sidebands(1, fm));
    sb += b;
  }
  tk::result("ZA-01", counted, "cadence de resynthese (128 echantillons moteur, fondu lineaire sur la periode suivante) :%s ; compteur 48 kHz Normal conforme %d ; echelon de profondeur (Mask) : 90 %% du changement en %.2f ms (lissage du parametre 20 ms + resynthese) ; bandes laterales d'une modulation sinusoidale de profondeur (h1..5, relatif aux raies) :%s",
             table.c_str(), counted, t90, sb.c_str());
  std::printf("  ZA-01 echelon %.2f ms ; bandes laterales :%s\n", t90, sb.c_str());
  // rendus représentatifs (SBD_R1_WAV_DIR) : même modulation audio CROSS OP1 -> WT.Z appliquée à Tilt (cadence de
  // resynthèse) et à Fold (par échantillon) ; même gain pour les deux fichiers
  if (const char* dir = std::getenv("SBD_R1_WAV_DIR")) {
    auto crossRender = [](int type) {
      PluginState m = wtState("factory:saw", type, 0.8, 0.6, 0, 0.0, 0.0, false);
      m.params[ParamId::WtZ] = 0.5f;
      setRoutes(m, {RouteTable::Op::add(kNodeOP1, kPortWtZ)});
      m.params[ParamId::DepthOp1WtZ] = 0.5f;
      m.params[ParamId::Op1Ratio] = 0.5f;
      m.params[ParamId::CrossMaster] = 1.0f;
      m.params[ParamId::CrossRev] = 1.0f;
      return renderTap(m, 45, 3.0).L;
    };
    const auto t = crossRender(4), f = crossRender(1);
    const double g = 0.5 / std::max(1e-9, std::max(peak(t), peak(f)));
    for (const auto& pr : {std::pair<const char*, const std::vector<float>*>{"/za01_cross_audio_tilt.wav", &t}, {"/za01_cross_audio_fold.wav", &f}}) {
      FILE* fp = std::fopen((std::string(dir) + pr.first).c_str(), "wb");
      if (!fp) continue;
      const std::uint32_t n = static_cast<std::uint32_t>(pr.second->size()), data = n * 3;
      auto u32 = [&](std::uint32_t v) { std::fwrite(&v, 4, 1, fp); };
      auto u16 = [&](std::uint16_t v) { std::fwrite(&v, 2, 1, fp); };
      std::fwrite("RIFF", 1, 4, fp);
      u32(36 + data);
      std::fwrite("WAVEfmt ", 1, 8, fp);
      u32(16), u16(1), u16(1), u32(48000), u32(48000 * 3), u16(3), u16(24);
      std::fwrite("data", 1, 4, fp);
      u32(data);
      for (float v : *pr.second) {
        const long q = std::lround(std::fmax(-1.0, std::fmin(1.0, v * g)) * 8388607.0);
        const unsigned char b[3] = {static_cast<unsigned char>(q & 0xff), static_cast<unsigned char>((q >> 8) & 0xff), static_cast<unsigned char>((q >> 16) & 0xff)};
        std::fwrite(b, 1, 3, fp);
      }
      std::fclose(fp);
    }
  }
  // ZA-02 : matrice 8 x 8 (A x B), rendu z_order 0 vs 1
  std::string mat;
  int same = 0, level = 0, diff = 0;
  for (int ta = 1; ta <= 8; ++ta) {
    mat += " " + std::to_string(ta) + ":";
    for (int tb = 1; tb <= 8; ++tb) {
      PluginState x = wtState("factory:metal", ta, 0.7, 0.3, tb, 0.5, -0.4, false);
      PluginState y = x;
      y.params[ParamId::WtZOrder] = 1.0f;
      const auto rx = renderTap(x, 45, 0.4).L, ry = renderTap(y, 45, 0.4).L;
      char code;
      if (th::sameFloats(rx, ry)) {
        code = '=';
        ++same;
      } else {
        double xy = 0.0, xx = 0.0, yy = 0.0;
        for (size_t i = 4800; i < rx.size(); ++i) xy += rx[i] * ry[i], xx += rx[i] * rx[i], yy += ry[i] * ry[i];
        const double corr = xy / std::sqrt(xx * yy + 1e-300);
        if (corr > 0.99999) {
          code = 'L';
          ++level;
        } else {
          code = 'D';
          ++diff;
        }
      }
      mat += code;
    }
  }
  tk::result("ZA-02", true, "matrice d'ordre (lignes A = 1 Fold, 2 Warp, 3 Mutation, 4 Tilt, 5 Mask, 6 Focus, 7 Shift, 8 Scatter ; colonnes B ; '=' A>B identique a B>A, 'L' niveau seul, 'D' different) :%s ; identiques %d, niveau seul %d, differents %d",
             mat.c_str(), same, level, diff);
}

}  // namespace

void runG43R1Tests() {
  std::printf("[ARCH-G4.3-R1]\n");
  testShift();
  testTilt();
  testTiltComposition();
  testTiltModulation();
  testTiltCost();
  testScatter();
  testZAudit();
}
