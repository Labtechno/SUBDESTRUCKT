// SUBDESTRUCKT — tests ARCH-G4.3 : D2 Glitch Buffer LEGACY / REGENERATIVE. docs/V1-ARCH-G4.3-SPEC.md §5.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <sstream>
#include <random>
#include <string>
#include <vector>

#include "G4Helper.h"
#include "RenderHelper.h"
#include "TestKit.h"
#include "core/StateCodec.h"
#include "dsp/Destruction.h"

using namespace sbd;
using namespace g4;

namespace {

constexpr double kFs = 48000.0;
long long gD2Alloc = 0;

struct Cfg {
  double size = 250.0, speed = 1.0, repeat = 0.5, pos = 0.0, fb = 0.0, xfade = 5.0, amp = 0.5;
  bool freeze = false;
  bool noise = false;           // bruit stéréo décorrélé pleine échelle (D2R-10)
  double silenceAfter = -1.0;   // s
};
struct Out {
  std::vector<float> L, R;
  bool bufFinite = true;
  double bufPeak = 0.0;
};
// hook(E, base, échantillon, slot) appelé toutes les 256 échantillons
template <typename Hook>
Out run(int mode, const Cfg& c, double secs, Hook hook) {
  DestructionSlot sl;
  sl.configure(kFs, 0x0D2ULL, Quality::Normal);
  sl.setD2FeedbackMode(mode);
  const int b = idx(ParamId::DstWt1Type);
  std::vector<double> E(static_cast<size_t>(kNumParams));
  const ParamValues d = ParamValues::defaults();
  for (int i = 0; i < kNumParams; ++i) E[static_cast<size_t>(i)] = d.v[static_cast<size_t>(i)];
  auto P = [&](DstOff o) -> double& { return E[static_cast<size_t>(b + static_cast<int>(o))]; };
  P(DstOff::Type) = static_cast<double>(DstType::D2);
  P(DstOff::Mix) = 1.0;
  P(DstOff::D2Size) = c.size;
  P(DstOff::D2Speed) = c.speed;
  P(DstOff::D2Repeat) = c.repeat;
  P(DstOff::D2Pos) = c.pos;
  P(DstOff::D2Fb) = c.fb;
  P(DstOff::D2Xfade) = c.xfade;
  P(DstOff::D2Freeze) = c.freeze ? 1.0 : 0.0;
  DestructionContext ctx;
  ctx.fs = kFs;
  Out o;
  const long long n = static_cast<long long>(secs * kFs);
  o.L.resize(static_cast<size_t>(n));
  o.R.resize(static_cast<size_t>(n));
  const long long silent = c.silenceAfter >= 0.0 ? static_cast<long long>(c.silenceAfter * kFs) : n;
  std::mt19937_64 nz(0x5EEDULL);
  std::uniform_real_distribution<double> un(-1.0, 1.0);
  double ph = 0.0, phR = 0.0;
  for (long long i = 0; i < n; ++i) {
    if (i % 256 == 0) hook(E, b, i, sl);
    double L = 0.0, R = 0.0;
    if (i < silent) {
      if (c.noise) {
        L = un(nz);
        R = un(nz);
      } else {
        const double f = 110.0 * std::pow(2.0, static_cast<double>((i / 14400) % 7) / 6.0);
        ph += f / kFs;
        ph -= std::floor(ph);
        const double fr = 146.83 * std::pow(2.0, static_cast<double>((i / 11520) % 5) / 5.0);
        phR += fr / kFs;
        phR -= std::floor(phR);
        L = c.amp * (std::sin(2.0 * 3.14159265358979323846 * ph) + 0.3 * std::sin(4.0 * 3.14159265358979323846 * ph)) / 1.3;
        R = c.amp * std::sin(2.0 * 3.14159265358979323846 * phR + 1.0);
      }
    }
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    sl.process(L, R, E.data(), b, ctx);
    tk::countingAllocations().store(false);
    gD2Alloc += tk::allocationCount().load();
    o.L[static_cast<size_t>(i)] = static_cast<float>(L);
    o.R[static_cast<size_t>(i)] = static_cast<float>(R);
  }
  for (int ch = 0; ch < 2; ++ch)
    for (float v : sl.testD2Buffer(ch)) {
      if (!std::isfinite(v)) o.bufFinite = false;
      o.bufPeak = std::max(o.bufPeak, static_cast<double>(std::fabs(v)));
    }
  return o;
}
Out run(int mode, const Cfg& c, double secs) {
  return run(mode, c, secs, [](std::vector<double>&, int, long long, DestructionSlot&) {});
}
double diffDb(const std::vector<float>& a, const std::vector<float>& ref) {
  double num = 0.0, den = 0.0;
  for (size_t i = 0; i < a.size(); ++i) {
    const double e = static_cast<double>(a[i]) - ref[i];
    num += e * e;
    den += static_cast<double>(ref[i]) * ref[i];
  }
  return 10.0 * std::log10((num + 1e-30) / (den + 1e-30));
}
bool same(const Out& a, const Out& b) { return th::sameFloats(a.L, b.L) && th::sameFloats(a.R, b.R); }

void testD2Regenerative() {
  // D2R-01 : registre
  {
    const char* want[6] = {"dst.wt1.d2.fb_mode", "dst.wt2.d2.fb_mode", "dst.fm1.d2.fb_mode", "dst.fm2.d2.fb_mode", "dst.gl1.d2.fb_mode", "dst.gl2.d2.fb_mode"};
    bool ok = kNumParams == 703 && idx(ParamId::DstWt1D2FbMode) == 695;
    for (int k = 0; k < 6; ++k)
      ok = ok && std::string(paramSpec(695 + k).stableId) == want[k] && paramSpec(695 + k).type == ParamType::Choice && paramSpec(695 + k).version == 4 &&
           std::string(choiceLabel(static_cast<ParamId>(695 + k), 0)) == "Legacy" && std::string(choiceLabel(static_cast<ParamId>(695 + k), 1)) == "Regenerative";
    TK_CHECK(ok);
    tk::result("D2R-01", ok, "registre : 6 StableID dst.<slot>.d2.fb_mode ajoutes en fin (695-700, Choice Legacy|Regenerative, version 4) ; 695 premiers : REG-ID ; VST3 : G-22w");
  }
  // D2R-02 : migration
  {
    const ParamId modes[6] = {ParamId::DstWt1D2FbMode, ParamId::DstWt2D2FbMode, ParamId::DstFm1D2FbMode, ParamId::DstFm2D2FbMode, ParamId::DstGl1D2FbMode, ParamId::DstGl2D2FbMode};
    int legacy = 0, files = 0;
    for (int k : {3, 12, 17}) {
      std::ifstream in("tests/core/golden_g43_v3_" + std::to_string(k) + ".txt", std::ios::binary);
      if (!in) in.open("../tests/core/golden_g43_v3_" + std::to_string(k) + ".txt", std::ios::binary);
      std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
      PluginState s;
      std::string err;
      if (!decodeState(text, s, err)) continue;
      ++files;
      for (ParamId m : modes) legacy += s.params.choice(m) == 0;
    }
    // preset fichier v3 (scope preset) : même règle
    PluginState p = defaultState();
    p.params[ParamId::DstWt1Type] = static_cast<float>(DstType::D2);
    std::string pt = encodeState(p, "t", StateScope::Preset), v3;
    {
      std::istringstream ls(pt);
      std::string line;
      while (std::getline(ls, line)) {
        if (line.rfind("SUBDESTRUCKT_STATE", 0) == 0) line = "SUBDESTRUCKT_STATE 3";
        if (line.find(".d2.fb_mode ") != std::string::npos || line.find("param wt.za.shape ") != std::string::npos || line.find("param wt.zb.shape ") != std::string::npos) continue;
        v3 += line + "\n";
      }
    }
    PluginState pv;
    std::string err;
    const bool presetDecoded = decodeState(v3, pv, err);
    if (!presetDecoded) std::printf("  D2R-02 preset v3 refuse : %s\n", err.c_str());
    const bool presetOk = presetDecoded && pv.params.choice(ParamId::DstWt1D2FbMode) == 0 && pv.sourceVersion == 3;
    int regen = 0;
    const PluginState fresh = defaultState();
    for (ParamId m : modes) regen += fresh.params.choice(m) == 1;
    PluginState mixed = defaultState();
    for (int k = 0; k < 6; ++k) mixed.params[modes[k]] = static_cast<float>(k % 2);
    PluginState back;
    const bool rt = decodeState(encodeState(mixed, "t", StateScope::Daw), back, err) && back.params.v == mixed.params.v;
    const bool explicitInPreset = pt.find("param dst.wt1.d2.fb_mode ") != std::string::npos;
    const bool ok = files == 3 && legacy == 18 && presetOk && regen == 6 && rt && explicitInPreset;
    TK_CHECK(ok);
    tk::result("D2R-02", ok, "migration : etats v3 0.3.1 -> LEGACY %d/18 ; preset fichier v3 -> LEGACY %d ; instance neuve -> REGENERATIVE %d/6 ; aller-retour format 4 exact %d ; preset sauvegarde : valeur explicite %d",
               legacy, presetOk, regen, rt, explicitInPreset);
  }
  // D2R-04 : Feedback = 0, REGENERATIVE = LEGACY (30 configurations)
  std::mt19937_64 rng(0xD2B4ULL);
  std::uniform_real_distribution<double> u(0.0, 1.0);
  {
    int eq = 0;
    for (int k = 0; k < 30; ++k) {
      Cfg x;
      x.size = 10.0 + 990.0 * u(rng);
      x.speed = -4.0 + 8.0 * u(rng);
      x.repeat = u(rng);
      x.pos = u(rng);
      x.xfade = 50.0 * u(rng);
      x.freeze = k % 3 == 0;
      eq += same(run(0, x, 1.5), run(1, x, 1.5));
    }
    const bool ok = eq == 30;
    TK_CHECK(ok);
    tk::result("D2R-04", ok, "Feedback = 0 : REGENERATIVE = LEGACY bit a bit %d/30 (tailles 10-1000 ms, vitesses +-4 dont Reverse, Repeat, Position, Crossfade, gel)", eq);
  }
  // D2R-05 : Feedback audible à Position 0 (croissant) ; Positions 0,5 et 1
  {
    Cfg c;
    const auto r0 = run(1, c, 8.0);
    double d[4] = {0, 0, 0, 0};
    const double fbs[4] = {0.0, 0.25, 0.5, 1.0};
    for (int k = 1; k < 4; ++k) {
      Cfg x = c;
      x.fb = fbs[k];
      d[k] = diffDb(run(1, x, 8.0).L, r0.L);
    }
    double dp[2];
    for (int k = 0; k < 2; ++k) {
      Cfg x = c, y = c;
      x.pos = y.pos = k == 0 ? 0.5 : 1.0;
      x.fb = 1.0;
      dp[k] = diffDb(run(1, x, 8.0).L, run(1, y, 8.0).L);
    }
    const bool ok = d[3] >= -20.0 && d[1] < d[2] && d[2] < d[3] && dp[0] >= -20.0 && dp[1] >= -20.0;
    TK_CHECK(ok);
    tk::result("D2R-05", ok, "REGENERATIVE, Position 0 : ecart fb 0,25 / 0,5 / 1 vs fb 0 = %.1f / %.1f / %.1f dB (>= -20 a fb 1, croissant) ; Position 0,5 : %.1f dB, Position 1 : %.1f dB (>= -20)",
               d[1], d[2], d[3], dp[0], dp[1]);
  }
  // D2R-06 : tampon gelé inchangé bit à bit (deux modes, fb > 0)
  {
    int ok6 = 0;
    for (int mode = 0; mode < 2; ++mode) {
      Cfg c;
      c.fb = 0.8;
      c.repeat = 1.0;
      std::vector<float> snapL, snapR;
      bool sameBuf = true;
      run(mode, c, 4.0, [&](std::vector<double>& E, int b, long long i, DestructionSlot& sl) {
        if (i == 96000) E[static_cast<size_t>(b + static_cast<int>(DstOff::D2Freeze))] = 1.0;  // gel à 2 s
        if (i == 96256) {
          snapL = sl.testD2Buffer(0);
          snapR = sl.testD2Buffer(1);
        }
        if (i > 96256 && (sl.testD2Buffer(0) != snapL || sl.testD2Buffer(1) != snapR)) sameBuf = false;
      });
      ok6 += sameBuf && !snapL.empty();
    }
    const bool ok = ok6 == 2;
    TK_CHECK(ok);
    tk::result("D2R-06", ok, "gel (fb 0,8) : tampon gele inchange bit a bit pendant 2 s, LEGACY et REGENERATIVE %d/2", ok6);
  }
  // D2R-07 : transitions de mode (chaque bloc ; toutes les 100 ms), fb 0,7
  {
    bool ok = true;
    double pk = 0.0, bp = 0.0;
    for (int period : {256, 4800}) {
      Cfg c;
      c.fb = 0.7;
      c.repeat = 1.0;
      c.amp = 1.0;
      auto hook = [period](std::vector<double>&, int, long long i, DestructionSlot& sl) { sl.setD2FeedbackMode(static_cast<int>((i / period) % 2)); };
      const auto a = run(0, c, 6.0, hook), b = run(0, c, 6.0, hook);
      pk = std::max(pk, std::max(peak(a.L), peak(a.R)));
      bp = std::max(bp, a.bufPeak);
      ok = ok && allFinite(a.L) && allFinite(a.R) && peak(a.L) <= 4.0 && peak(a.R) <= 4.0 && a.bufFinite && a.bufPeak <= 4.0 && same(a, b);
    }
    TK_CHECK(ok);
    tk::result("D2R-07", ok, "transitions de mode a chaque bloc et toutes les 100 ms (fb 0,7, boucle) : sortie finie, crete %.3f (<= 4), tampon fini et <= 4 (%.3f), deux rendus identiques %d",
               pk, bp, ok);
  }
  // D2R-08 : indépendance par emplacement (moteur) : WT1 et FM1 en D2, modes différents
  {
    auto render = [](int wtMode, int fmMode, bool wtOn, bool fmOn) {
      G4Scenario sc;
      PluginState st = defaultState();
      for (ParamId t : {ParamId::DstWt1Type, ParamId::DstFm1Type}) {
        const int b = idx(t);
        st.params.v[static_cast<size_t>(b + static_cast<int>(DstOff::Type))] = static_cast<float>(DstType::D2);
        st.params.v[static_cast<size_t>(b + static_cast<int>(DstOff::Mix))] = 1.0f;
        st.params.v[static_cast<size_t>(b + static_cast<int>(DstOff::D2Fb))] = 0.9f;
        st.params.v[static_cast<size_t>(b + static_cast<int>(DstOff::D2Repeat))] = 0.8f;
      }
      st.params[ParamId::DstWt1D2FbMode] = static_cast<float>(wtMode);
      st.params[ParamId::DstFm1D2FbMode] = static_cast<float>(fmMode);
      st.params[ParamId::WtLevel] = wtOn ? 0.7f : 0.0f;
      st.params[ParamId::Op1Level] = fmOn ? 0.7f : 0.0f;
      sc.state = st;
      sc.samples = 144000;
      sc.events = {MidiEvent::noteOn(0, 45, 110), MidiEvent::noteOff(100000, 45)};
      return renderG4(sc);
    };
    // chaîne FM muette : changer le mode FM1 ne modifie rien ; chaîne WT muette : changer le mode WT1 ne modifie rien
    const auto a = render(0, 1, true, false), b = render(0, 0, true, false), c = render(1, 1, false, true), d = render(0, 1, false, true);
    const auto e = render(0, 1, true, false), f = render(1, 1, true, false);
    const bool ok = th::sameFloats(a.L, b.L) && th::sameFloats(c.L, d.L) && !th::sameFloats(e.L, f.L);
    TK_CHECK(ok);
    tk::result("D2R-08", ok, "independance par emplacement (moteur) : WT1 LEGACY + FM1 REGENERATIVE : chaine WT = reference WT1 LEGACY seule %d ; chaine FM = reference FM1 REGENERATIVE seule %d ; le mode WT1 change bien la chaine WT %d",
               th::sameFloats(a.L, b.L), th::sameFloats(c.L, d.L), !th::sameFloats(e.L, f.L));
  }
  // D2R-09 : automation (Feedback en rampes et sauts, Position, vitesse et Reverse par bloc)
  {
    Cfg c;
    c.amp = 1.0;
    auto hook = [](std::vector<double>& E, int b, long long i, DestructionSlot&) {
      auto P = [&](DstOff o) -> double& { return E[static_cast<size_t>(b + static_cast<int>(o))]; };
      const long long blk = i / 256;
      P(DstOff::D2Fb) = blk % 40 < 20 ? (blk % 20) / 19.0 : (blk % 2 ? 1.0 : 0.0);
      P(DstOff::D2Pos) = (blk % 7) / 6.0;
      P(DstOff::D2Speed) = (blk % 5 == 0 ? -1.0 : 1.0) * (0.25 + (blk % 4));
      P(DstOff::D2Repeat) = (blk % 3) / 2.0;
    };
    const long long a0 = gD2Alloc;
    bool ok = true;
    for (int mode = 0; mode < 2; ++mode) {
      const auto a = run(mode, c, 5.0, hook), b = run(mode, c, 5.0, hook);
      ok = ok && allFinite(a.L) && allFinite(a.R) && same(a, b);
    }
    ok = ok && gD2Alloc == a0;
    TK_CHECK(ok);
    tk::result("D2R-09", ok, "automation par bloc (Feedback rampes/sauts, Position, vitesse +-, Repeat), deux modes : fini, deterministe, 0 allocation %d", ok);
  }
  // D2R-10 : protocole de stabilité (défini avant mesure) : divergence, fluctuation bornée, rejeu, décroissance
  {
    int pass = 0, total = 0;
    double worstPk = 0.0, worstFluct = 0.0, worstReplay = 0.0, worstDecay = -400.0;
    std::string fails, summary[2];
    for (int mode = 0; mode < 2; ++mode) {
      double mPk = 0.0, mDecay = -400.0, mReplay = 0.0;
      for (double fb : {0.5, 0.9, 1.0})
        for (double sp : {-4.0, -1.0, 0.25, 1.0, 4.0})
          for (double size : {10.0, 100.0, 500.0}) {
            Cfg c;
            c.noise = true;
            c.fb = fb;
            c.speed = sp;
            c.size = size;
            c.repeat = 1.0;
            c.silenceAfter = 60.0;
            const auto r = run(mode, c, 180.0);
            ++total;
            bool ok = true;
            for (const auto* x : {&r.L, &r.R}) {
              const double pk = peak(*x);
              mPk = std::max(mPk, pk);
              if (!allFinite(*x) || pk > 4.0) ok = false;  // (a) divergence
              std::vector<double> w;
              for (int s = 20; s < 60; s += 10) w.push_back(rms(*x, static_cast<size_t>(s * kFs), static_cast<size_t>((s + 10) * kFs)));
              std::vector<double> ws = w;
              std::sort(ws.begin(), ws.end());
              const double med = 0.5 * (ws[1] + ws[2]), mx = ws.back();
              for (double v : w) {
                const double f = std::max(v / med, med / std::max(v, 1e-30));
                worstFluct = std::max(worstFluct, f);
                if (v < 0.25 * med || v > 4.0 * med) ok = false;  // (b) fluctuation bornée
              }
              for (int s = 60; s < 180; s += 10) {
                const double v = rms(*x, static_cast<size_t>(s * kFs), static_cast<size_t>((s + 10) * kFs));
                mReplay = std::max(mReplay, v / std::max(mx, 1e-30));
                if (v > 2.0 * mx) ok = false;  // (c) rejeu
              }
              const double e50 = rms(*x, static_cast<size_t>(50 * kFs), static_cast<size_t>(60 * kFs));
              const double e170 = rms(*x, static_cast<size_t>(170 * kFs), static_cast<size_t>(180 * kFs));
              const double dd = 20.0 * std::log10(std::max(e170, 1e-30) / std::max(e50, 1e-30));
              mDecay = std::max(mDecay, dd);
              if (dd > -40.0) ok = false;  // (d) décroissance à long terme
            }
            pass += ok;
            if (!ok) {
              char s[96];
              std::snprintf(s, sizeof s, " [%s fb %.1f v%.2f %.0f ms]", mode ? "REGEN" : "LEGACY", fb, sp, size);
              fails += s;
            }
          }
      worstPk = std::max(worstPk, mPk);
      worstReplay = std::max(worstReplay, mReplay);
      worstDecay = std::max(worstDecay, mDecay);
      char s[160];
      std::snprintf(s, sizeof s, "%s : crete %.3f, pire rejeu %.2f x, pire decroissance %.1f dB", mode ? "REGENERATIVE" : "LEGACY", mPk, mReplay, mDecay);
      summary[mode] = s;
    }
    const bool ok = pass == total;
    TK_CHECK(ok);
    tk::result("D2R-10", ok, "stabilite (bruit stereo pleine echelle 60 s puis silence 120 s, Repeat 1, fb 0,5/0,9/1, vitesses -4..4, tailles 10/100/500 ms) : %d/%d cas ; (a) divergence : aucune si cas PASS ; (b) pire fluctuation %.2f x la mediane (<= 4) ; %s ; %s%s",
               pass, total, worstFluct, summary[0].c_str(), summary[1].c_str(), fails.c_str());
  }
}

}  // namespace

void runG43D2Tests() {
  std::printf("[ARCH-G4.3 : D2 LEGACY / REGENERATIVE]\n");
  testD2Regenerative();
}
