// SUBDESTRUCKT — tests ARCH-G4.2 (ELP, AUT, D2). Critères : docs/V1-ARCH-G4.2-SPEC.md (écrits avant mesure).
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "Analysis.h"
#include "G4Helper.h"
#include "TestKit.h"
#include "core/StateCodec.h"
#include "dsp/Destruction.h"
#include "dsp/Elp.h"
#include "dsp/Filters.h"

using namespace sbd;
using namespace g4;

namespace {

// ------------------------------------------------------------------ ELP-01..03 : fonction statique
void testElpFunction() {
  // ELP-01 : identité exacte sous le seuil, imparité, monotonie stricte, borne < 32
  bool ident = true, odd = true, mono = true, bound = true;
  for (int i = 0; i <= 1000000; ++i) {
    const double y = -16.0 + 32.0 * i / 1000000.0;
    if (elpTransfer(y) != y) ident = false;
  }
  double prev = elpTransfer(16.0), plateau = 0.0, nonDecr = 1.0;
  for (int i = 1; i <= 100000; ++i) {
    const double y = 16.0 * std::pow(62500.0, i / 100000.0);  // 16 .. 1e6, grille logarithmique
    const double f = elpTransfer(y);
    if (!(f > prev)) {
      if (mono) plateau = y;  // première égalité : début du plateau numérique
      mono = false;
    }
    if (f < prev) nonDecr = 0.0;
    if (elpTransfer(-y) != -f) odd = false;
    prev = f;
  }
  for (double y : {std::nextafter(16.0, 100.0), 1e3, 1e6, 1e300, DBL_MAX})
    if (!(elpTransfer(y) < kOutputGuard) || !(elpTransfer(-y) > -kOutputGuard)) bound = false;
  const bool e1 = ident && odd && mono && bound;
  TK_CHECK(e1);
  tk::result("ELP-01", e1, "fonction ELP : identite exacte sur 10^6 points de [-16,16] %d ; impaire %d ; strictement croissante sur ]16,1e6] %d (premier plateau numerique a |y| = %.1f soit %+.1f dBFS ; non decroissante %d) ; < 32 pour toute entree finie (jusqu'a DBL_MAX) %d",
             ident, odd, mono, plateau, plateau > 0.0 ? 20.0 * std::log10(plateau) : 0.0, nonDecr == 1.0, bound);
  // ELP-02 : continuité au seuil, pente, 1-lipschitzienne
  bool cont = true;
  double worst = 0.0;
  for (double eps : {1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1}) {
    const double d = std::fabs(elpTransfer(16.0 + eps) - (16.0 + eps));
    worst = std::max(worst, d / (eps * eps / 16.0));
    if (d > eps * eps / 16.0) cont = false;
  }
  const double h = 1e-3;  // pas de différence finie (erreur d'arrondi ~1e-12, terme cubique ~1e-8)
  const double slope = (elpTransfer(16.0 + 2.0 * h) - elpTransfer(16.0 + h)) / h;
  const bool slopeOk = slope >= 1.0 - 1e-6 && slope <= 1.0;
  std::mt19937_64 rng(0xE1F2ULL);
  std::uniform_real_distribution<double> u(-200.0, 200.0);
  bool lip = true;
  for (int i = 0; i < 100000; ++i) {
    const double a = u(rng), b = u(rng);
    if (std::fabs(elpTransfer(a) - elpTransfer(b)) > std::fabs(a - b) * (1.0 + 1e-12)) lip = false;
  }
  const bool e2 = cont && slopeOk && lip;
  TK_CHECK(e2);
  tk::result("ELP-02", e2, "continuite au seuil : |f(16+e)-(16+e)| <= e^2/16 (pire rapport %.3f) %d ; pente a droite %.9f %d ; 1-lipschitzienne sur 10^5 paires %d",
             worst, cont, slope, slopeOk, lip);
  // ELP-03 : borne asymptotique
  const double f6 = elpTransfer(1e6), fmax = elpTransfer(DBL_MAX);
  const double dbMax = 20.0 * std::log10(fmax);
  const bool e3 = f6 >= 31.99 && dbMax <= 30.103;
  TK_CHECK(e3);
  tk::result("ELP-03", e3, "borne asymptotique : f(1e6) = %.6f (>= 31.99), f(DBL_MAX) = %.9f soit %.4f dBFS (<= 30.103)", f6, fmax, dbMax);
}

bool isGainParam(int p) {
  const std::string id = paramSpec(p).stableId;
  auto ends = [&](const char* s) {
    const std::string t(s);
    return id.size() >= t.size() && id.compare(id.size() - t.size(), t.size(), t) == 0;
  };
  return id == "out.gain" || ends(".out") || ends(".drive") || ends(".makeup");
}

// configuration aléatoire « destruction » (protocole DST-09) : 6 emplacements tirés dans leurs plages
void randomSlots(PluginState& st, std::mt19937_64& rng) {
  for (int slot = 0; slot < 6; ++slot) {
    const int b = idx(ParamId::DstWt1Type) + slot * static_cast<int>(DstOff::Count);
    for (int o = 0; o < static_cast<int>(DstOff::Count); ++o) {
      const auto& sp = paramSpec(b + o);
      std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
      st.params.v[static_cast<size_t>(b + o)] = sanitize(static_cast<ParamId>(b + o), u(rng));
    }
  }
}

// ------------------------------------------------------------------ ELP-04..07 : moteur
void testElpEngine() {
  // ELP-04 : bit à bit sous le seuil (crête dans ]1, 16[) : ELP vs contre-épreuve sans ELP
  std::mt19937_64 rng(0xE404ULL);
  int found = 0, tries = 0, same = 0, engaged = 0;
  while (found < 30 && tries < 400) {
    ++tries;
    G4Scenario sc;
    sc.samples = 24000;
    // configuration par défaut + niveaux, filtres et un emplacement de destruction tirés : crêtes de 0 à ~+30 dBFS
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);
    sc.state.params[ParamId::OutGain] = 6.0f * u01(rng);
    sc.state.params[ParamId::WtLevel] = u01(rng);
    sc.state.params[ParamId::Op1Level] = u01(rng);
    for (ParamId f : {ParamId::FltWtOut, ParamId::FltFmOut, ParamId::FltGlOut}) sc.state.params[f] = 24.0f * u01(rng);
    for (ParamId f : {ParamId::FltWtDrive, ParamId::FltFmDrive, ParamId::FltGlDrive}) sc.state.params[f] = 24.0f * u01(rng);
    for (ParamId f : {ParamId::FltWtType, ParamId::FltFmType, ParamId::FltGlType}) sc.state.params[f] = static_cast<float>(rng() % 5);
    const int base = idx(ParamId::DstGl1Type);
    for (int o = 0; o < static_cast<int>(DstOff::Count); ++o) {
      const auto& sp = paramSpec(base + o);
      std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
      sc.state.params.v[static_cast<size_t>(base + o)] = sanitize(static_cast<ParamId>(base + o), u(rng));
    }
    sc.state.params[ParamId::DstGl1Arm] = 0.0f;
    sc.state.params[ParamId::OutLimiter] = 0.0f;
    sc.events = {MidiEvent::noteOn(0, 36 + static_cast<int>(rng() % 48), 120)};
    const auto a = renderG4(sc);
    const double pk = std::max(peak(a.L), peak(a.R));
    if (!(pk > 1.0 && pk < 16.0) || !allFinite(a.L)) continue;
    ++found;
    G4Scenario off = sc;
    off.beforeBlock = [](Engine& e, ParamValues&, long long) { e.setElpEnabledForTest(false); };
    const auto b = renderG4(off);
    if (th::sameFloats(a.L, b.L) && th::sameFloats(a.R, b.R)) ++same;
    engaged += static_cast<int>(a.elp);
  }
  const bool e4 = found == 30 && same == 30 && engaged == 0;
  TK_CHECK(e4);
  tk::result("ELP-04", e4, "%d configurations a crete dans ]1,16[ (%d tirages) : identiques bit a bit avec et sans ELP %d/30, engagements %d",
             found, tries, same, engaged);

  // ELP-05 : configurations extrêmes (gains au maximum de leurs plages) et sauts de gain de sortie à chaque bloc
  std::mt19937_64 r5(0xE505ULL);
  int bad5 = 0, countBad = 0;
  unsigned guard5 = 0, elp5 = 0, nan5 = 0;
  double pk5 = 0.0;
  for (int c = 0; c < 24; ++c) {
    G4Scenario sc;
    const double rates[4] = {44100.0, 48000.0, 88200.0, 96000.0};
    sc.fs = rates[c % 4];
    sc.block = 64 << (c % 4);
    sc.samples = static_cast<long long>(sc.fs * 1.5);
    randomSlots(sc.state, r5);
    for (int p = 0; p < kNumParams; ++p)
      if (isGainParam(p)) sc.state.params.v[static_cast<size_t>(p)] = paramSpec(p).maxValue;
    sc.state.params[ParamId::OutLimiter] = 0.0f;
    sc.events = {MidiEvent::noteOn(0, 40 + c, 127)};
    sc.beforeBlock = [](Engine&, ParamValues& p, long long pos) {
      p[ParamId::OutGain] = ((pos / 64) % 2 == 0) ? paramSpec(idx(ParamId::OutGain)).maxValue : paramSpec(idx(ParamId::OutGain)).minValue;
    };
    const auto r = renderG4(sc);
    long long above = 0, atOrAbove = 0;
    for (const auto* ch : {&r.L, &r.R})
      for (float v : *ch) {
        if (std::fabs(v) > 16.0f) ++above;
        if (std::fabs(v) >= 16.0f) ++atOrAbove;
      }
    const double pk = std::max(peak(r.L), peak(r.R));
    pk5 = std::max(pk5, pk);
    guard5 += r.clamp;
    elp5 += r.elp;
    nan5 += r.nonFinite;
    if (!(static_cast<long long>(r.elp) >= above && static_cast<long long>(r.elp) <= atOrAbove)) ++countBad;
    if (r.nonFinite != 0 || r.clamp != 0 || !(pk < 32.0) || !allFinite(r.L) || !allFinite(r.R)) ++bad5;
  }
  const bool e5 = bad5 == 0 && countBad == 0 && elp5 > 0;
  TK_CHECK(e5);
  tk::result("ELP-05", e5, "24 configurations extremes (gains au maximum, sauts de gain de sortie par bloc, 4 Fs) : %d en defaut ; crete %.7f (< 32) ; garde %u ; non-finis %u ; engagements ELP %u ; coherence telemetrie/echantillons > 16 : %d ecart(s)",
             bad5, pk5, guard5, nan5, elp5, countBad);

  // ELP-06 : contre-épreuve DST-09 sans ELP : la garde s'engage (PASS de G-15/DST-09 conditionnel à SPEC-HR-01)
  unsigned guardOn = 0, guardOff = 0, elpOn = 0;
  for (int pass = 0; pass < 2; ++pass) {
    std::mt19937_64 r2(0xD57ULL);  // même graine que HR-02 / DST-09
    for (int c = 0; c < 48; ++c) {
      G4Scenario sc;
      const double rates[4] = {44100.0, 48000.0, 88200.0, 96000.0};
      sc.fs = rates[c % 4];
      sc.samples = static_cast<long long>(sc.fs * 2.0);
      randomSlots(sc.state, r2);
      sc.events = {MidiEvent::noteOn(100, 45, 100)};
      if (pass == 1) sc.beforeBlock = [](Engine& e, ParamValues&, long long) { e.setElpEnabledForTest(false); };
      const auto r = renderG4(sc);
      (pass == 0 ? guardOn : guardOff) += r.clamp;
      if (pass == 0) elpOn += r.elp;
    }
  }
  const bool e6 = guardOn == 0 && guardOff > 0;
  TK_CHECK(e6);
  tk::result("ELP-06", e6, "contre-epreuve du protocole DST-09 : avec ELP garde %u (engagements ELP %u) ; SANS ELP garde %u (> 0 attendu) : le PASS de G-15/DST-09 depend de SPEC-HR-01 (proposee)",
             guardOn, elpOn, guardOff);

  // ELP-07 : distinction ELP / limiteur utilisateur / garde numérique
  auto lim = [](float outGain, bool extreme) {
    G4Scenario sc;
    sc.samples = 48000;
    sc.state.params[ParamId::OutLimiter] = 1.0f;
    sc.state.params[ParamId::OutCeiling] = -6.0f;
    sc.state.params[ParamId::OutGain] = outGain;
    if (extreme) {
      sc.state.params[ParamId::FltGlType] = 1.0f;
      sc.state.params[ParamId::FltGlOut] = 24.0f;
      sc.state.params[ParamId::FltGlDrive] = 36.0f;
      sc.state.params[ParamId::DstGl1Type] = static_cast<float>(DstType::Y1);
      sc.state.params[ParamId::DstGl1Y1Makeup] = 36.0f;
      sc.state.params[ParamId::DstGl1Out] = 24.0f;
    }
    sc.events = {MidiEvent::noteOn(0, 48, 127)};
    return renderG4(sc);
  };
  const auto a = lim(6.0f, false), b = lim(6.0f, true);
  const double ceil = std::pow(10.0, -6.0 / 20.0);
  const double pa = std::max(peak(a.L), peak(a.R)), pb = std::max(peak(b.L), peak(b.R));
  const bool sepA = a.limiter > 0 && a.elp == 0 && a.clamp == 0 && pa <= ceil + 1e-6;
  const bool sepB = b.elp > 0 && b.limiter > 0 && b.clamp == 0 && pb <= ceil + 1e-6;
  const bool tele = a.elp == a.internal.elp && a.limiter == a.internal.limiter && a.clamp == a.internal.clamp && b.elp == b.internal.elp &&
                    b.limiter == b.internal.limiter && b.clamp == b.internal.clamp;
  const bool e7 = sepA && sepB && tele;
  TK_CHECK(e7);
  tk::result("ELP-07", e7, "(a) limiteur -6 dB sous +24 dBFS : limiteur %u, ELP %u, garde %u, crete %.4f (plafond %.4f) %d ; (b) > +24 dBFS : ELP %u, limiteur %u, garde %u, crete %.4f %d ; (c) telemetrie = compteurs internes %d",
             a.limiter, a.elp, a.clamp, pa, ceil, sepA, b.elp, b.limiter, b.clamp, pb, sepB, tele);
}

// ------------------------------------------------------------------ AUT-01 : armement effectif = configuré ET autorisé
// k = 0..2 : filtres WT/FM/GLOBAL (F4 Modal, gel modal) ; k = 3..8 : emplacements WT1..GL2 (D2 gel, S2 gel, N2 auto-osc.)
struct AutRun {
  std::vector<float> L;
  long long alloc = 0;
};
AutRun autRender(int k, bool arm, int authMode) {  // authMode : 0 inchangée, 1 retirée, 2 retirée puis accordée
  ParamValues p = ParamValues::defaults();
  int armIdx = 0, freezeIdx = -1;
  if (k < 3) {
    const int b = idx(ParamId::FltWtType) + k * static_cast<int>(FltOff::Count);
    p.v[static_cast<size_t>(b + static_cast<int>(FltOff::Type))] = 4.0f;  // F4 Modal
    p.v[static_cast<size_t>(b + static_cast<int>(FltOff::Decay))] = 1.0f;
    armIdx = b + static_cast<int>(FltOff::Arm);
    freezeIdx = b + static_cast<int>(FltOff::Freeze);
  } else {
    const int slot = k - 3, b = idx(ParamId::DstWt1Type) + slot * static_cast<int>(DstOff::Count);
    const int kind = slot % 3;
    const DstType t = kind == 0 ? DstType::D2 : kind == 1 ? DstType::S2 : DstType::N2;
    p.v[static_cast<size_t>(b + static_cast<int>(DstOff::Type))] = static_cast<float>(t);
    p.v[static_cast<size_t>(b + static_cast<int>(DstOff::Mix))] = 1.0f;
    armIdx = b + static_cast<int>(DstOff::Arm);
    if (kind == 0) freezeIdx = b + static_cast<int>(DstOff::D2Freeze);
    if (kind == 1) freezeIdx = b + static_cast<int>(DstOff::S2Freeze);
    if (kind == 2) {
      p.v[static_cast<size_t>(b + static_cast<int>(DstOff::N2SelfOsc))] = 1.0f;
      p.v[static_cast<size_t>(b + static_cast<int>(DstOff::N2Fb))] = 0.95f;
    }
  }
  p.v[static_cast<size_t>(armIdx)] = arm ? 1.0f : 0.0f;
  Engine e(FactoryBanks::get("factory:saw"));
  e.setParameters(p);
  e.prepare(48000, 256);
  if (authMode >= 1) e.setAutonomyAuthorized(false);
  if (authMode == 2) e.setAutonomyAuthorized(true);
  AutRun out;
  out.L.assign(static_cast<size_t>(256 * 2250), 0.0f);  // 12 s
  std::vector<float> R(256);
  const MidiEvent on = MidiEvent::noteOn(0, 52, 120), off = MidiEvent::noteOff(0, 52);
  for (int blk = 0; blk < 2250; ++blk) {
    const MidiEvent* ev = nullptr;
    int ne = 0;
    if (blk == 0) ev = &on, ne = 1;
    if (blk == 150) ev = &off, ne = 1;
    if (blk == 75 && freezeIdx >= 0) {  // gel activé pendant la note
      p.v[static_cast<size_t>(freezeIdx)] = 1.0f;
      e.setParameters(p);
    }
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    e.process(&out.L[static_cast<size_t>(blk * 256)], R.data(), 256, ev, ne);
    tk::countingAllocations().store(false);
    out.alloc += tk::allocationCount().load();
  }
  return out;
}

void testAutonomyEngine() {
  int suspendedSame = 0, grantedSame = 0, armAudible = 0;
  long long alloc = 0;
  std::string detail;
  for (int k = 0; k < 9; ++k) {
    const auto off = autRender(k, false, 0), armed = autRender(k, true, 0), susp = autRender(k, true, 1), again = autRender(k, true, 2);
    const bool s1 = th::sameFloats(susp.L, off.L), s2 = th::sameFloats(again.L, armed.L), aud = !th::sameFloats(armed.L, off.L);
    suspendedSame += s1;
    grantedSame += s2;
    armAudible += aud;
    alloc += off.alloc + armed.alloc + susp.alloc + again.alloc;
    detail += std::string(" ") + paramSpec(k < 3 ? idx(ParamId::FltWtArm) + k * static_cast<int>(FltOff::Count)
                                                 : idx(ParamId::DstWt1Arm) + (k - 3) * static_cast<int>(DstOff::Count)).stableId +
              (aud ? "(effet)" : "(sans effet)");
  }
  const bool ok = suspendedSame == 9 && grantedSame == 9 && alloc == 0;
  TK_CHECK(ok);
  tk::result("AUT-01", ok, "9 armements : suspendu = desarme bit a bit %d/9 ; autorise = moteur arme historique bit a bit %d/9 ; armement audible dans le scenario %d/9 :%s ; allocations RT %lld",
             suspendedSame, grantedSame, armAudible, detail.c_str(), alloc);
}

// ------------------------------------------------------------------ D2-01..05 : D2 Feedback (étude A-G41-06)
struct D2Cfg {
  double size = 250.0, speed = 1.0, repeat = 0.5, pos = 0.0, fb = 0.0, xfade = 5.0;
  bool freeze = false;
  double amp = 0.5;
  double silenceAfter = -1.0;  // s : entrée coupée au-delà (comportement sans excitation)
  bool stereo = false;         // canal droit décorrélé (autre suite de hauteurs, autre phase)
};
struct D2Out {
  std::vector<float> L, R;
  long long alloc = 0;
};
using D2Hook = void (*)(std::vector<double>&, int, long long, std::mt19937_64&);
D2Out d2Run(int mode, const D2Cfg& c, double seconds, D2Hook hook = nullptr, std::uint64_t hookSeed = 1) {
  const double fs = 48000.0;
  DestructionSlot sl;
  sl.configure(fs, 0x0D2ULL, Quality::Normal);
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
  ctx.fs = fs;
  D2Out o;
  const long long n = static_cast<long long>(seconds * fs);
  o.L.resize(static_cast<size_t>(n));
  o.R.resize(static_cast<size_t>(n));
  const long long silent = c.silenceAfter >= 0.0 ? static_cast<long long>(c.silenceAfter * fs) : n;
  std::mt19937_64 hr(hookSeed);
  double ph = 0.0, phR = 0.0;
  for (long long i = 0; i < n; ++i) {
    if (hook && i % 256 == 0) hook(E, b, i, hr);
    // entrée non stationnaire : suite de notes (changement de hauteur toutes les 0,3 s) + harmonique
    const double f = 110.0 * std::pow(2.0, static_cast<double>((i / 14400) % 7) / 6.0);
    ph += f / fs;
    ph -= std::floor(ph);
    double L = i < silent ? c.amp * (std::sin(kTwoPi * ph) + 0.3 * std::sin(2.0 * kTwoPi * ph)) / 1.3 : 0.0, R = L;
    if (c.stereo) {
      const double fr = 146.83 * std::pow(2.0, static_cast<double>((i / 11520) % 5) / 5.0);
      phR += fr / fs;
      phR -= std::floor(phR);
      R = i < silent ? c.amp * std::sin(kTwoPi * phR + 1.0) : 0.0;
    }
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    sl.process(L, R, E.data(), b, ctx);
    tk::countingAllocations().store(false);
    o.alloc += tk::allocationCount().load();
    o.L[static_cast<size_t>(i)] = static_cast<float>(L);
    o.R[static_cast<size_t>(i)] = static_cast<float>(R);
  }
  return o;
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
double rmsRange(const std::vector<float>& x, size_t a, size_t b) {
  double acc = 0.0;
  for (size_t i = a; i < b && i < x.size(); ++i) acc += static_cast<double>(x[i]) * x[i];
  return std::sqrt(acc / std::max<size_t>(1, b - a));
}

void testD2Feedback() {
  // D2-01 / D2-02 : Position = 0, fb 0 / 0,25 / 0,5 / 1 (8 s)
  D2Cfg c;
  const double secs = 8.0;
  const auto h0 = d2Run(0, c, secs);
  double hd[4] = {0, 0, 0, 0}, cd[4] = {0, 0, 0, 0};
  const double fbs[4] = {0.0, 0.25, 0.5, 1.0};
  const auto c0 = d2Run(1, c, secs);
  for (int k = 1; k < 4; ++k) {
    D2Cfg x = c;
    x.fb = fbs[k];
    hd[k] = diffDb(d2Run(0, x, secs).L, h0.L);
    cd[k] = diffDb(d2Run(1, x, secs).L, c0.L);
  }
  const bool d1 = hd[3] <= -60.0;
  TK_CHECK(d1);
  tk::result("D2-01", d1, "historique, Position 0 : ecart fb 0,25 / 0,5 / 1 vs fb 0 = %.1f / %.1f / %.1f dB (<= -60 attendu a fb 1) : Feedback inaudible",
             hd[1], hd[2], hd[3]);
  const bool d2 = cd[3] >= -20.0 && cd[1] < cd[2] && cd[2] < cd[3];
  TK_CHECK(d2);
  tk::result("D2-02", d2, "candidat C1, Position 0 : ecart fb 0,25 / 0,5 / 1 vs fb 0 = %.1f / %.1f / %.1f dB (>= -20 a fb 1, croissant) : Feedback audible",
             cd[1], cd[2], cd[3]);

  // D2-03 : fb = 0 (30 configurations) et gel actif (fb > 0) : candidat = historique bit à bit
  std::mt19937_64 rng(0xD203ULL);
  std::uniform_real_distribution<double> u(0.0, 1.0);
  int same = 0, frozenSame = 0;
  for (int k = 0; k < 30; ++k) {
    D2Cfg x;
    x.size = 10.0 + 990.0 * u(rng);
    x.speed = -4.0 + 8.0 * u(rng);
    x.repeat = u(rng);
    x.pos = u(rng);
    x.xfade = 50.0 * u(rng);
    x.freeze = k % 3 == 0;
    x.fb = 0.0;
    same += th::sameFloats(d2Run(0, x, 1.5).L, d2Run(1, x, 1.5).L);
    x.freeze = true;
    x.fb = 0.2 + 0.8 * u(rng);
    frozenSame += th::sameFloats(d2Run(0, x, 1.5).L, d2Run(1, x, 1.5).L);
  }
  const bool d3 = same == 30 && frozenSame == 30;
  TK_CHECK(d3);
  tk::result("D2-03", d3, "non-regression : fb 0, 30 configurations (taille, vitesse +-4, Repeat, Position, Crossfade, gel) candidat = historique bit a bit %d/30 ; gel actif avec fb > 0 : %d/30",
             same, frozenSame);

  // D2-04 : stabilité 60 s, fb 1, pleine échelle, Repeat 1, 5 vitesses, historique et candidat
  bool stable = true;
  double worstPk = 0.0, worstGrowth = 0.0;
  std::string cases;
  for (int mode = 0; mode < 2; ++mode)
    for (double sp : {-4.0, -1.0, 0.25, 1.0, 4.0}) {
      D2Cfg x;
      x.fb = 1.0;
      x.repeat = 1.0;
      x.speed = sp;
      x.amp = 1.0;
      const auto r = d2Run(mode, x, 60.0);
      const double pk = peak(r.L);
      const double g = rmsRange(r.L, 50 * 48000, 60 * 48000) / std::max(1e-12, rmsRange(r.L, 40 * 48000, 50 * 48000));
      worstPk = std::max(worstPk, pk);
      worstGrowth = std::max(worstGrowth, g);
      if (!allFinite(r.L) || pk > 4.0 || g > 1.05) stable = false;
      char line[160];
      std::snprintf(line, sizeof line, " %s v%.2f : crete %.3f, RMS 40-50 %.4f, 50-60 %.4f (x%.3f) ;", mode ? "C1" : "H", sp, pk,
                    rmsRange(r.L, 40 * 48000, 50 * 48000), rmsRange(r.L, 50 * 48000, 60 * 48000), g);
      cases += line;
    }
  TK_CHECK(stable);
  tk::result("D2-04", stable, "stabilite 60 s (fb 1, pleine echelle, Repeat 1, vitesses -4/-1/0,25/1/4, historique et candidat) : finis, crete max %.3f (<= 4), croissance RMS 50-60 s / 40-50 s max %.3f (<= 1,05) ; detail :%s",
             worstPk, worstGrowth, cases.c_str());

  // D2-04c (COMPLÉMENTAIRE, défini APRÈS le diagnostic de l'échec D2-04 ; ne remplace pas D2-04) : stabilité physique.
  // (1) sans excitation (entrée coupée à 60 s) : énergie par fenêtre de 10 s non croissante et RMS(170-180 s) <= -40 dB
  // du niveau 50-60 s ; (2) 300 s avec entrée stéréo décorrélée : finis et crête <= 4 sur les deux canaux ;
  // (3) rapports de fenêtres longues (moyennes 180-300 s / 60-180 s) : DESCRIPTIFS (aucun seuil).
  bool decay = true, bounded = true;
  std::string longR, rise;
  double worstDecayDb = -999.0, pk300 = 0.0;
  for (int mode = 0; mode < 2; ++mode)
    for (double sp : {-4.0, -1.0, 0.25, 1.0, 4.0}) {
      D2Cfg x;
      x.fb = 1.0;
      x.repeat = 1.0;
      x.speed = sp;
      x.amp = 1.0;
      x.stereo = true;
      const auto r = d2Run(mode, x, 300.0);
      for (const auto* ch : {&r.L, &r.R}) {
        pk300 = std::max(pk300, peak(*ch));
        if (!allFinite(*ch) || peak(*ch) > 4.0) bounded = false;
      }
      double m1 = 0.0, m2 = 0.0;
      for (int w = 6; w < 18; ++w) m1 += rmsRange(r.L, static_cast<size_t>(w) * 480000, static_cast<size_t>(w + 1) * 480000) / 12.0;
      for (int w = 18; w < 30; ++w) m2 += rmsRange(r.L, static_cast<size_t>(w) * 480000, static_cast<size_t>(w + 1) * 480000) / 12.0;
      char line[96];
      std::snprintf(line, sizeof line, " %s v%.2f x%.3f", mode ? "C1" : "H", sp, m2 / std::max(1e-12, m1));
      longR += line;
      x.silenceAfter = 60.0;
      const auto z = d2Run(mode, x, 180.0);
      for (const auto* ch : {&z.L, &z.R}) {
        double prev = rmsRange(*ch, 50 * 48000, 60 * 48000);
        const double ref = prev;
        for (int w = 6; w < 18; ++w) {
          const double v = rmsRange(*ch, static_cast<size_t>(w) * 480000, static_cast<size_t>(w + 1) * 480000);
          if (v > prev) {
            {
              char vl[160];
              std::snprintf(vl, sizeof vl, " hausse %s v%.2f canal %s fenetre %d-%d s %.4g > %.4g ;", mode ? "C1" : "H", sp,
                            ch == &z.L ? "L" : "R", w * 10, w * 10 + 10, v, prev);
              rise += vl;
            }
            decay = false;
          }
          prev = v;
        }
        const double db = 20.0 * std::log10((prev + 1e-30) / (ref + 1e-30));
        worstDecayDb = std::max(worstDecayDb, db);
        if (db > -40.0) decay = false;
      }
    }
  const bool d4c = decay && bounded;
  TK_CHECK(d4c);
  tk::result("D2-04c", d4c, "COMPLEMENTAIRE (defini apres diagnostic de D2-04, ne le remplace pas) : sans excitation, energie non croissante et RMS 170-180 s <= -40 dB (pire %.1f dB) %d%s ; 300 s stereo decorrelee : finis, crete %.3f (<= 4) %d ; rapports fenetres longues 180-300 / 60-180 s (descriptifs) :%s",
             worstDecayDb, decay, rise.c_str(), pk300, bounded, longR.c_str());

  // D2-05 : déterminisme et transitions (sauts par bloc de fb, Position, gel, vitesse)
  D2Hook jumps = [](std::vector<double>& E, int b, long long, std::mt19937_64& r) {
    std::uniform_real_distribution<double> v(0.0, 1.0);
    E[static_cast<size_t>(b + static_cast<int>(DstOff::D2Fb))] = v(r);
    E[static_cast<size_t>(b + static_cast<int>(DstOff::D2Pos))] = v(r);
    E[static_cast<size_t>(b + static_cast<int>(DstOff::D2Freeze))] = v(r) < 0.2 ? 1.0 : 0.0;
    E[static_cast<size_t>(b + static_cast<int>(DstOff::D2Speed))] = -4.0 + 8.0 * v(r);
  };
  bool det = true, fin = true;
  long long alloc = 0;
  for (int mode = 0; mode < 2; ++mode) {
    D2Cfg x;
    x.repeat = 0.8;
    const auto a = d2Run(mode, x, 6.0, jumps, 77), b2 = d2Run(mode, x, 6.0, jumps, 77);
    det = det && th::sameFloats(a.L, b2.L);
    fin = fin && allFinite(a.L);
    alloc += a.alloc + b2.alloc;
  }
  const bool d5 = det && fin && alloc == 0;
  TK_CHECK(d5);
  tk::result("D2-05", d5, "deterministe (2 rendus identiques bit a bit) %d ; sauts par bloc de fb/Position/gel/vitesse : finis %d ; allocations %lld (historique et candidat)",
             det, fin, alloc);
}

}  // namespace

void runG42Tests() {
  std::fprintf(stderr, "[ARCH-G4.2 : ELP]\n");
  testElpFunction();
  testElpEngine();
  std::fprintf(stderr, "[ARCH-G4.2 : AUT]\n");
  testAutonomyEngine();
  std::fprintf(stderr, "[ARCH-G4.2 : D2]\n");
  testD2Feedback();
}
