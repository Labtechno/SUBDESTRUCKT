// Tests du moteur temps réel : G-01d, G-04, G-07 à G-19 (voir docs/V1-ARCH-G3-PLAN.md §7).
#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

#include "Analysis.h"
#include "RenderHelper.h"
#include "TestKit.h"
#include "core/StateCodec.h"

using namespace sbd;
using Op = RouteTable::Op;

namespace {

long long g_allocations = 0;  // G-18 : cumul sur tous les rendus

th::Render run(const th::Scenario& s) {
  th::Render r = th::render(s);
  g_allocations += r.allocations;
  return r;
}

const double kRates[4] = {44100.0, 48000.0, 88200.0, 96000.0};

ParamValues cleanParams() {
  ParamValues p = ParamValues::defaults();
  p[ParamId::AmpAttack] = 0.001f;
  p[ParamId::AmpDecay] = 0.001f;
  p[ParamId::AmpSustain] = 1.0f;
  p[ParamId::AmpVelocity] = 0.0f;  // gain de vélocité = 1
  p[ParamId::OutGain] = 0.0f;
  p[ParamId::WtLevel] = 0.0f;
  p[ParamId::Op1Level] = 0.0f;
  p[ParamId::Op2Level] = 0.0f;
  for (int s = 0; s < kMaxRoutes; ++s) p[routeSlotInfo(s).depthParam] = 0.0f;
  return p;
}

std::vector<double> segment(const std::vector<double>& x, double fs, double startS, size_t n) {
  const size_t a = static_cast<size_t>(startS * fs);
  return std::vector<double>(x.begin() + static_cast<long>(a), x.begin() + static_cast<long>(a + n));
}

bool allFinite(const std::vector<float>& v) {
  for (float x : v)
    if (!std::isfinite(x)) return false;
  return true;
}

double peakOf(const std::vector<float>& v, size_t a, size_t b) {
  double p = 0.0;
  for (size_t i = a; i < b && i < v.size(); ++i) p = std::max(p, static_cast<double>(std::fabs(v[i])));
  return p;
}

// ---------------------------------------------------------------- G-04 causalité (empoisonnement NaN)
void testCausality() {
  const std::vector<Op> rich = {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP1, kNodeWT),
                                Op::add(kNodeOP1, kNodeOP2), Op::add(kNodeOP1, kNodeOP1), Op::add(kNodeOP2, kNodeWT)};
  bool ok = true, negDetected = true;
  for (double fs : kRates) {
    th::Scenario s;
    s.fs = fs;
    s.samples = static_cast<long long>(fs * 0.5);
    s.params = ParamValues::defaults();
    for (int k = 0; k < kMaxRoutes; ++k) s.params[routeSlotInfo(k).depthParam] = 1.0f;
    s.params[ParamId::Op2Level] = 0.5f;
    s.topology = rich;
    s.events = {MidiEvent::noteOn(100, 60, 110)};
    s.poison = true;
    const auto r = run(s);
    bool finite = r.nonFinite == 0 && allFinite(r.L);
    for (double v : r.tap) finite = finite && std::isfinite(v);
    ok = ok && finite;
    // contrôle négatif : ordre inversé -> lecture Immediate d'un nœud non calculé -> NaN détecté
    s.reverseOrder = true;
    const auto n = run(s);
    negDetected = negDetected && n.nonFinite > 0;
  }
  TK_CHECK(ok && negDetected);
  tk::result("G-04", ok && negDetected, "empoisonnement NaN des sorties non calculees : 0 violation aux 4 frequences ; controle negatif (ordre inverse) detecte : %s",
             negDetected ? "oui" : "NON");
}

// ---------------------------------------------------------------- G-01d profondeur nulle
void testDepthZero() {
  th::Scenario s;
  s.fs = 48000;
  s.samples = 24000;
  s.params = ParamValues::defaults();
  s.params[ParamId::DepthOp1Wt] = 0.0f;
  s.topology = {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP1, kNodeWT)};
  s.events = {MidiEvent::noteOn(10, 60, 100)};
  const auto a = run(s);
  th::Scenario t = s;
  t.topology = {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeWT, kNodeOP1)};
  const auto b = run(t);
  const Route* r = a.finalTable.findPair(kNodeOP1, kNodeWT);
  const bool kept = r && r->cls == CausalClass::Delayed && a.finalTable.compile().numEdges == 3;
  const bool same = th::sameDoubles(a.tap.data(), b.tap.data(), a.tap.size());
  TK_CHECK(kept && same);
  tk::result("G-01d", kept && same, "profondeur 0 : la route et sa classe D restent dans le graphe compile ; contribution exactement nulle (sortie identique au graphe sans la route)");
}

// ---------------------------------------------------------------- G-07 déterminisme et invariance aux blocs
void testDeterminism() {
  bool ok = true;
  for (double fs : kRates) {
    th::Scenario s;
    s.fs = fs;
    s.samples = 70000;
    s.params = ParamValues::defaults();
    s.params[ParamId::DepthOp1Op1] = 0.7f;
    s.params[ParamId::Op2Level] = 0.3f;
    s.topology = {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP1, kNodeWT),
                  Op::add(kNodeOP1, kNodeOP1)};
    s.events = {MidiEvent::noteOn(1000, 60, 100), MidiEvent::noteOn(20000, 64, 80), MidiEvent::noteOff(30000, 64),
                MidiEvent::noteOff(40000, 60), MidiEvent::noteOn(50000, 67, 90), MidiEvent::allNotesOff(65000)};
    s.block = 256;
    const auto ref = run(s);
    const auto again = run(s);
    ok = ok && th::sameFloats(ref.L, again.L);
    for (int b : {1, 7, 32, 64, 128, 512}) {
      th::Scenario v = s;
      v.block = b;
      const auto r = run(v);
      const bool same = th::sameFloats(ref.L, r.L) && th::sameDoubles(ref.tap.data(), r.tap.data(), ref.tap.size());
      if (!same) tk::info("G-07 difference fs=%.0f bloc=%d", fs, b);
      ok = ok && same;
    }
    th::Scenario v = s;
    v.blockSizes = {37, 512, 1, 700, 129, 64, 1000, 3};  // variables, y compris > taille max préparée (découpage)
    const auto r = run(v);
    const bool same = th::sameFloats(ref.L, r.L);
    if (!same) tk::info("G-07 difference fs=%.0f blocs variables", fs);
    ok = ok && same;
  }
  TK_CHECK(ok);
  tk::result("G-07", ok, "rendus identiques bit a bit (2 executions) et invariants aux blocs 1/7/32/64/128/256/512/variables, aux 4 frequences");
}

// ---------------------------------------------------------------- G-08 offsets, ordre, priorité
void testMidi() {
  bool ok = true;
  for (double fs : kRates) {
    th::Scenario s;
    s.fs = fs;
    s.samples = 4000;
    s.block = 256;
    s.events = {MidiEvent::noteOn(1234, 60, 100)};
    s.topology = {Op::add(kNodeOP2, kNodeOP1)};
    const auto r = run(s);
    bool zeroBefore = true;
    for (int i = 0; i < 1234; ++i) zeroBefore = zeroBefore && r.L[static_cast<size_t>(i)] == 0.0f;
    const bool onset = r.setup.oversampling == 1 ? r.L[1234] != 0.0f : peakOf(r.L, 1234, 1400) > 0.0;
    if (!(zeroBefore && onset)) tk::info("G-08 offset incorrect a fs=%.0f", fs);
    ok = ok && zeroBefore && onset;
  }
  // ordre dans un même échantillon : note-off avant note-on, quel que soit l'ordre de réception
  auto table = th::table();
  {
    Engine e(table);
    e.prepare(48000, 512);
    std::vector<float> L(512), R(512);
    const MidiEvent ev[2] = {MidiEvent::noteOn(100, 60, 100), MidiEvent::noteOff(100, 60)};
    e.process(L.data(), R.data(), 512, ev, 2);
    ok = ok && e.telemetry().gate.load() && e.telemetry().currentNote.load() == 60;
  }
  // priorité dernière note, retour à la note tenue sans relance
  {
    Engine e(table);
    e.prepare(48000, 512);
    std::vector<float> L(512), R(512);
    const MidiEvent ev[3] = {MidiEvent::noteOn(10, 60, 100), MidiEvent::noteOn(20, 64, 100), MidiEvent::noteOff(30, 64)};
    e.process(L.data(), R.data(), 512, ev, 3);
    ok = ok && e.telemetry().gate.load() && e.telemetry().currentNote.load() == 60;
    const MidiEvent off[1] = {MidiEvent::noteOff(5, 60)};
    e.process(L.data(), R.data(), 512, off, 1);
    ok = ok && !e.telemetry().gate.load();
  }
  TK_CHECK(ok);
  tk::result("G-08", ok, "note-on appliquee a son offset exact (zero avant, son a l'offset a K=1) ; note-off avant note-on dans un meme echantillon ; priorite derniere note");
}

// ---------------------------------------------------------------- G-09 WT seul
void testWtOnly() {
  th::Scenario s;
  s.fs = 48000;
  s.samples = 120000;
  s.params = cleanParams();
  s.params[ParamId::WtLevel] = 1.0f;
  s.events = {MidiEvent::noteOn(0, 57, 127)};  // 220 Hz
  const auto r = run(s);
  const size_t N = 65536;
  const auto seg = segment(r.tap, s.fs, 0.5, N);
  const auto m = an::amplitudeSpectrum(seg.data(), N);
  const auto pw = an::powerSpectrum(seg.data(), N);
  const double bin = s.fs / N;
  const double a1 = an::lineAmplitudeEnergy(pw, bin, 220.0);
  double worst = 0.0;
  for (int k = 2; k <= 10; ++k)
    worst = std::max(worst, std::fabs(20.0 * std::log10(an::lineAmplitudeEnergy(pw, bin, 220.0 * k) / a1 * k)));
  const double inharm = an::offGridEnergyDb(m, bin, 220.0, 20.0, 20000.0);
  // indépendance : FM actif (OP2>OP1, niveaux FM nuls, aucune route vers WT) -> sortie WT identique bit à bit
  th::Scenario t = s;
  t.params[ParamId::DepthOp2Op1] = 3.0f;
  t.topology = {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeOP1, kNodeOP1)};
  t.params[ParamId::DepthOp1Op1] = 2.0f;
  const auto u = run(t);
  const bool indep = th::sameDoubles(r.tap.data(), u.tap.data(), r.tap.size());
  const bool ok = worst <= 0.5 && inharm <= -40.0 && indep;
  TK_CHECK(ok);
  tk::result("G-09", ok, "WT seul (220 Hz) : harmoniques 2..10 en 1/k a %.3f dB pres (seuil 0,5), energie inharmonique %.1f dB (seuil -40), independance vis-a-vis du FM : %s",
             worst, inharm, indep ? "identique bit a bit" : "NON");
}

// ---------------------------------------------------------------- G-10 FM seul (PM, bandes de Bessel)
void testFmOnly() {
  th::Scenario s;
  s.fs = 48000;
  s.samples = 120000;
  s.params = cleanParams();
  s.params[ParamId::Op1Level] = 1.0f;
  s.params[ParamId::Op1Ratio] = 1.0f;
  s.params[ParamId::Op2Ratio] = 0.25f;
  s.params[ParamId::DepthOp2Op1] = 1.0f;  // indice I = 1 rad
  s.topology = {Op::add(kNodeOP2, kNodeOP1)};
  s.events = {MidiEvent::noteOn(0, 69, 127)};  // porteuse 440 Hz, modulante 110 Hz
  const auto r = run(s);
  const size_t N = 65536;
  const auto seg = segment(r.tap, s.fs, 0.5, N);
  const auto pw = an::powerSpectrum(seg.data(), N);
  const double bin = s.fs / N;
  double worst = 0.0;
  char detail[256];
  int off = 0;
  for (int k = 0; k <= 3; ++k) {
    const double J = std::fabs(an::besselJ(k, 1.0));
    for (int sgn : {1, -1}) {
      if (k == 0 && sgn < 0) continue;
      const double f = 440.0 + sgn * k * 110.0;
      const double a = an::lineAmplitudeEnergy(pw, bin, f);
      const double errDb = 20.0 * std::log10(a / J);
      worst = std::max(worst, std::fabs(errDb));
      if (sgn > 0) off += std::snprintf(detail + off, sizeof detail - static_cast<size_t>(off), " J%d:%+.3fdB", k, errDb);
    }
  }
  const bool ok = worst <= 0.5;
  TK_CHECK(ok);
  tk::result("G-10", ok, "FM seul OP2>OP1, I=1 rad : bandes laterales k=0..3 conformes a |Jk(1)|, ecart max %.3f dB (seuil 0,5) ;%s", worst, detail);
}

// ---------------------------------------------------------------- G-11 / G-12 CROSS, effet de chaque direction
struct CrossOutcome {
  double nmse = 0, spec = 0;
  bool inactiveEqualsRef = false;
  bool delayedClass = true;
};

CrossOutcome crossCase(double fs, bool forward, bool cycle) {
  th::Scenario a;
  a.fs = fs;
  a.samples = static_cast<long long>(fs * 1.6);
  a.params = cleanParams();
  if (forward) {
    a.params[ParamId::Op1Level] = 1.0f;
    a.params[ParamId::DepthWtOp1] = 1.0f;
  } else {
    a.params[ParamId::WtLevel] = 1.0f;
    a.params[ParamId::DepthOp1Wt] = 1.0f;
    a.params[ParamId::Op1Ratio] = 1.5f;
  }
  if (cycle) {
    a.topology = {Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP1, kNodeWT)};  // OP1>WT : Delayed (T2)
    a.params[ParamId::DepthWtOp1] = forward ? 1.0f : 1.0f;
    a.params[forward ? ParamId::CrossRev : ParamId::CrossFwd] = 0.0f;  // isole la direction testée
  } else {
    a.topology = {forward ? Op::add(kNodeWT, kNodeOP1) : Op::add(kNodeOP1, kNodeWT)};
  }
  a.events = {MidiEvent::noteOn(0, 57, 127)};
  th::Scenario b = a;
  b.params[forward ? ParamId::CrossFwd : ParamId::CrossRev] = 0.0f;  // direction testée inactive
  th::Scenario ref = a;
  ref.topology.clear();
  const auto ra = run(a), rb = run(b), rr = run(ref);
  CrossOutcome o;
  const size_t N = 32768;
  const auto sa = segment(ra.tap, fs, 0.5, N), sb = segment(rb.tap, fs, 0.5, N);
  o.nmse = an::nmseDb(sa.data(), sb.data(), N);
  o.spec = an::spectralDifferenceDb(an::amplitudeSpectrum(sa.data(), N), an::amplitudeSpectrum(sb.data(), N), fs / N, 50.0,
                                    16000.0);
  o.inactiveEqualsRef = th::sameDoubles(rb.tap.data(), rr.tap.data(), rb.tap.size());
  if (cycle) {
    const Route* r = ra.finalTable.findPair(kNodeOP1, kNodeWT);
    o.delayedClass = r && r->cls == CausalClass::Delayed;
  }
  return o;
}

void testCross() {
  for (bool forward : {true, false}) {
    bool ok = true;
    char detail[512];
    int off = 0;
    for (double fs : {48000.0, 44100.0}) {
      for (bool cycle : {false, true}) {
        const auto o = crossCase(fs, forward, cycle);
        const bool pass = o.nmse >= -20.0 && o.spec >= 1.0 && o.inactiveEqualsRef && o.delayedClass;
        ok = ok && pass;
        off += std::snprintf(detail + off, sizeof detail - static_cast<size_t>(off), " [%.1fk %s NMSE %.1f dB, ecart %.2f dB, inactif=ref %s]",
                             fs / 1000.0, cycle ? "cycle B-T2" : "route seule", o.nmse, o.spec, o.inactiveEqualsRef ? "oui" : "NON");
      }
    }
    TK_CHECK(ok);
    tk::result(forward ? "G-11" : "G-12", ok, "CROSS %s mesurable (NMSE >= -20 dB, ecart spectral >= 1 dB), inactif identique bit a bit a la reference :%s",
               forward ? "WT>FM" : "FM>WT", detail);
  }
}

// ---------------------------------------------------------------- G-13 CROSS = 0 sans reset
void testCrossZero() {
  bool ok = true;
  char detail[512];
  int off = 0;
  for (double fs : kRates) {
    th::Scenario s;
    s.fs = fs;
    s.samples = static_cast<long long>(fs * 1.2);
    s.params = ParamValues::defaults();
    s.params[ParamId::DepthWtOp1] = 1.5f;
    s.params[ParamId::DepthOp1Wt] = 1.5f;
    s.topology = {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP1, kNodeWT)};
    s.events = {MidiEvent::noteOn(0, 60, 100)};
    const long long switchAt = static_cast<long long>(fs * 0.5);
    long long applied = -1;
    s.beforeBlock = [&](Engine&, ParamValues& p, RouteTable&, long long pos) {
      if (pos >= switchAt && applied < 0) {
        p[ParamId::CrossMaster] = 0.0f;
        applied = pos;
      }
    };
    const auto a = run(s);
    th::Scenario ref = s;
    ref.beforeBlock = nullptr;
    ref.params[ParamId::CrossMaster] = 0.0f;
    const auto b = run(ref);
    const long long from = applied + static_cast<long long>(std::ceil(0.020 * fs)) + 512;
    const size_t n = static_cast<size_t>(s.samples - from);
    const bool equalAfter = th::sameDoubles(&a.tap[static_cast<size_t>(from)], &b.tap[static_cast<size_t>(from)], n);
    const double before = an::nmseDb(&a.tap[static_cast<size_t>(fs * 0.2)], &b.tap[static_cast<size_t>(fs * 0.2)], 8192);
    ok = ok && equalAfter && before > -40.0;
    off += std::snprintf(detail + off, sizeof detail - static_cast<size_t>(off), " [%.1fk avant: NMSE %.1f dB ; apres rampe: %s]",
                         fs / 1000.0, before, equalAfter ? "identique bit a bit" : "DIFFERENT");
  }
  TK_CHECK(ok);
  tk::result("G-13", ok, "CROSS Master -> 0 en cours de note : apres la rampe, sortie identique bit a bit a une reference sans CROSS (aucun reset de phase) :%s",
             detail);
}

// ---------------------------------------------------------------- G-14 transitions de topologie
void testTransitions() {
  bool ok = true;
  char detail[768];
  int off = 0;
  for (double fs : {48000.0, 44100.0}) {
    th::Scenario s;
    s.fs = fs;
    s.samples = static_cast<long long>(fs * 1.4);
    s.params = ParamValues::defaults();
    for (int k = 0; k < kMaxRoutes; ++k) s.params[routeSlotInfo(k).depthParam] = 1.0f;
    s.params[ParamId::Op2Level] = 0.3f;
    s.topology = {Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP2, kNodeOP1)};
    s.events = {MidiEvent::noteOn(0, 55, 110)};
    s.poison = true;  // toute lecture Immediate d'un nœud non calculé pendant une transition serait détectée
    bool unionCyclic = false, classesOk = true;
    std::vector<long long> at;
    int stage = 0;
    s.beforeBlock = [&](Engine& e, ParamValues&, RouteTable& t, long long pos) {
      const double tsec = pos / fs;
      auto due = [&](double when) { return tsec >= when && stage == static_cast<int>(when * 5 + 0.5) - 1; };
      if (due(0.2)) {  // union cyclique : retirer WT>OP1 (I) et ajouter OP1>WT, classée I dans le nouveau graphe
        const std::uint32_t id = t.findPair(kNodeWT, kNodeOP1)->id;
        th::publish(e, t, {Op::remove(id), Op::add(kNodeOP1, kNodeWT)});
        unionCyclic = t.findPair(kNodeOP1, kNodeWT)->cls == CausalClass::Immediate;
        at.push_back(pos);
        ++stage;
      } else if (due(0.4)) {  // WT>OP1 ferme le cycle -> D
        th::publish(e, t, {Op::add(kNodeWT, kNodeOP1)});
        classesOk = classesOk && t.findPair(kNodeWT, kNodeOP1)->cls == CausalClass::Delayed;
        at.push_back(pos);
        ++stage;
      } else if (due(0.6)) {  // OP1>OP2 ferme OP2>OP1>OP2 -> D
        th::publish(e, t, {Op::add(kNodeOP1, kNodeOP2)});
        classesOk = classesOk && t.findPair(kNodeOP1, kNodeOP2)->cls == CausalClass::Delayed;
        at.push_back(pos);
        ++stage;
      } else if (due(0.8)) {  // retrait d'OP2>OP1 : OP1>OP2 reste D (sticky)
        th::publish(e, t, {Op::remove(t.findPair(kNodeOP2, kNodeOP1)->id)});
        classesOk = classesOk && t.findPair(kNodeOP1, kNodeOP2)->cls == CausalClass::Delayed;
        at.push_back(pos);
        ++stage;
      } else if (due(1.0)) {
        th::publish(e, t, {Op::add(kNodeOP1, kNodeOP1), Op::add(kNodeOP2, kNodeWT)});
        classesOk = classesOk && t.findPair(kNodeOP1, kNodeOP1)->cls == CausalClass::Delayed &&
                    t.findPair(kNodeOP2, kNodeWT)->cls == CausalClass::Immediate;
        at.push_back(pos);
        ++stage;
      } else if (due(1.2)) {
        std::vector<Op> ops;
        for (const auto& r : t.routes()) ops.push_back(Op::remove(r.id));
        th::publish(e, t, ops);
        at.push_back(pos);
        ++stage;
      }
    };
    const auto r = run(s);
    const bool pass = r.nonFinite == 0 && r.overlap == 0 && unionCyclic && classesOk && stage == 6 && allFinite(r.L) &&
                      r.finalTable.routes().empty();
    ok = ok && pass;
    // métrique de clic (rapportée, sans seuil) : énergie Δ⁴ pendant la transition vs régime établi avant
    std::vector<double> y(r.L.begin(), r.L.end());
    double worst = -300.0;
    const int K = r.setup.oversampling;
    for (long long t0 : at) {
      // fenêtre de transition, puis régimes établis avant ET après : un changement de timbre n'est pas un clic
      const size_t a = static_cast<size_t>(t0), b = a + static_cast<size_t>(2 * 128 + 64 + r.setup.latencySamples);
      const double burst = an::maxD4Burst(y, a, b);
      const double steady = std::max(an::maxD4Burst(y, a - 4096, a - 64), an::maxD4Burst(y, b + 512, b + 4608));
      worst = std::max(worst, 10.0 * std::log10((burst + 1e-30) / (steady + 1e-30)));
    }
    off += std::snprintf(detail + off, sizeof detail - static_cast<size_t>(off), " [%.1fk K=%d : union cyclique %s, chevauchement %lld ech., NaN %u, burst D4 max %+.1f dB]",
                         fs / 1000.0, K, unionCyclic ? "presente" : "absente", r.overlap, r.nonFinite, worst);
  }
  TK_CHECK(ok);
  tk::result("G-14", ok, "6 transactions en cours de note (dont union cyclique), sequence fondu-sortie / bascule / fondu-entree, poison actif :%s (metrique de clic rapportee sans seuil)",
             detail);
}

// ---------------------------------------------------------------- G-15 bornes numériques
void testBounds() {
  std::mt19937_64 rng(0x5EEDBEEFULL);
  int configs = 0, bad = 0;
  unsigned pmClamps = 0, outClamps = 0;
  for (int c = 0; c < 200; ++c) {
    th::Scenario s;
    s.fs = kRates[rng() % 4];
    const int blocks[5] = {32, 64, 128, 256, 512};
    s.block = blocks[rng() % 5];
    s.samples = static_cast<long long>(s.fs * 2.0);
    for (int p = 0; p < kNumParams; ++p) {
      const auto& sp = paramSpec(p);
      std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
      s.params.v[static_cast<size_t>(p)] = u(rng);
    }
    RouteTable t;
    for (int k = 0; k < 8; ++k) {
      const int slot = static_cast<int>(rng() % kMaxRoutes);
      if (!t.findPair(routeSlotInfo(slot).src, routeSlotInfo(slot).dst))
        t.apply({Op::add(routeSlotInfo(slot).src, routeSlotInfo(slot).dst)});
    }
    for (const auto& r : t.routes()) s.topology.push_back(Op::add(r.src, r.dst));  // même ordre d'ajout
    for (int e = 0; e < 6; ++e) {
      const int at = static_cast<int>(rng() % static_cast<unsigned long long>(s.samples));
      const int note = 24 + static_cast<int>(rng() % 84);
      s.events.push_back(MidiEvent::noteOn(at, note, 1 + static_cast<int>(rng() % 127)));
      s.events.push_back(MidiEvent::noteOff(std::min<int>(at + 20000, static_cast<int>(s.samples - 1)), note));
    }
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);
    std::mt19937_64 prng(rng());
    s.beforeBlock = [&prng, u01](Engine&, ParamValues& p, RouteTable&, long long) mutable {
      if (prng() % 50 == 0) {  // automation aléatoire en cours de rendu
        const int k = static_cast<int>(prng() % kNumParams);
        const auto& sp = paramSpec(k);
        p.v[static_cast<size_t>(k)] = sp.minValue + u01(prng) * (sp.maxValue - sp.minValue);
      }
    };
    const auto r = run(s);
    ++configs;
    pmClamps += r.pmClamp;
    outClamps += r.outputClamp;
    if (r.nonFinite != 0 || r.outputClamp != 0 || r.pmClamp != 0 || !allFinite(r.L)) ++bad;
  }
  const bool ok = bad == 0;
  TK_CHECK(ok);
  tk::result("G-15", ok, "%d configurations aleatoires x 2 s (parametres, topologies, notes, automation) : %d en defaut ; garde |y|>32 : %u ; borne PM +/-32 rad atteinte : %u",
             configs, bad, outClamps, pmClamps);
}

// ---------------------------------------------------------------- G-16 reset et récupération
void testResetRecovery() {
  bool ok = true;
  // Panic (CC120)
  th::Scenario s;
  s.fs = 48000;
  s.samples = 60000;
  s.events = {MidiEvent::noteOn(0, 60, 110), MidiEvent::allSoundOff(24000), MidiEvent::noteOn(36000, 67, 110)};
  s.topology = {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP1, kNodeWT)};
  const auto r = run(s);
  const size_t silentFrom = 24000 + static_cast<size_t>(std::lround(0.005 * s.fs));
  bool silent = true;
  for (size_t i = silentFrom; i < 36000; ++i) silent = silent && r.L[i] == 0.0f;
  const double after = peakOf(r.L, 36000, 60000);
  const bool panicOk = silent && after > 0.01 && r.panics == 1;
  // NaN injecté dans un historique de retard (crochet de test)
  th::Scenario n = s;
  n.events = {MidiEvent::noteOn(0, 60, 110)};
  bool injected = false;
  n.beforeBlock = [&](Engine& e, ParamValues&, RouteTable&, long long pos) {
    if (pos >= 24000 && !injected) {
      e.testInjectNaN();
      injected = true;
    }
  };
  const auto q = run(n);
  const bool nanOk = q.nonFinite >= 1 && allFinite(q.L) && peakOf(q.L, 40000, 60000) > 0.01;
  ok = panicOk && nanOk;
  TK_CHECK(ok);
  tk::result("G-16", ok, "Panic : silence exact apres 5 ms (%s), note suivante normale (crete %.3f) ; NaN injecte : %u reinitialisation(s), sortie toujours finie, reprise du son (%s)",
             silent ? "oui" : "NON", after, q.nonFinite, nanOk ? "oui" : "NON");
}

// ---------------------------------------------------------------- G-17 silence et notes
void testSilence() {
  bool ok = true;
  for (double fs : kRates) {
    th::Scenario s;
    s.fs = fs;
    s.samples = static_cast<long long>(fs);
    s.topology = {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP1, kNodeWT)};
    const auto r = run(s);
    bool zero = true;
    for (float v : r.L) zero = zero && v == 0.0f;
    th::Scenario t = s;
    t.samples = static_cast<long long>(fs * 2.0);
    t.events = {MidiEvent::noteOn(0, 60, 100), MidiEvent::noteOff(static_cast<int>(fs * 0.5), 60)};
    const auto u = run(t);
    const double tail = peakOf(u.L, static_cast<size_t>(fs * 1.8), static_cast<size_t>(fs * 2.0));
    ok = ok && zero && tail < 1e-6;
    if (!(zero && tail < 1e-6)) tk::info("G-17 fs=%.0f zero=%d queue=%.3g", fs, zero, tail);
  }
  TK_CHECK(ok);
  tk::result("G-17", ok, "sans note : sortie exactement nulle ; 1 s apres la fin du relachement : crete < -120 dBFS (4 frequences)");
}

// ---------------------------------------------------------------- G-19 instances multiples
void testInstances() {
  auto mk = [](double fs, int note, float depth, std::vector<Op> topo) {
    th::Scenario s;
    s.fs = fs;
    s.samples = 30000;
    s.params = ParamValues::defaults();
    s.params[ParamId::DepthOp2Op1] = depth;
    s.topology = std::move(topo);
    s.events = {MidiEvent::noteOn(100, note, 100)};
    return s;
  };
  const auto sa = mk(48000, 60, 1.0f, {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP1, kNodeWT)});
  const auto sb = mk(48000, 67, 3.0f, {Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeOP1, kNodeOP1)});
  const auto ra = run(sa), rb = run(sb);
  // rendu entrelacé : deux moteurs partageant la même table immuable
  Engine ea(th::table()), eb(th::table());
  ea.setParameters(sa.params);
  eb.setParameters(sb.params);
  ea.prepare(48000, 256);
  eb.prepare(48000, 256);
  RouteTable ta, tb;
  ta.apply(sa.topology);
  tb.apply(sb.topology);
  ea.setTopologyImmediate(ta.compile());
  eb.setTopologyImmediate(tb.compile());
  std::vector<float> la(30000), lb(30000), rr(256);
  for (int pos = 0; pos < 30000; pos += 256) {
    const int n = std::min(256, 30000 - pos);
    std::vector<MidiEvent> ev;
    if (pos <= 100 && 100 < pos + n) ev.push_back(MidiEvent::noteOn(100 - pos, 60, 100));
    ea.setParameters(sa.params);
    ea.process(&la[static_cast<size_t>(pos)], rr.data(), n, ev.data(), static_cast<int>(ev.size()));
    ev.clear();
    if (pos <= 100 && 100 < pos + n) ev.push_back(MidiEvent::noteOn(100 - pos, 67, 100));
    eb.setParameters(sb.params);
    eb.process(&lb[static_cast<size_t>(pos)], rr.data(), n, ev.data(), static_cast<int>(ev.size()));
  }
  const bool ok = th::sameFloats(la, ra.L) && th::sameFloats(lb, rb.L);
  TK_CHECK(ok);
  tk::result("G-19", ok, "deux instances entrelacees (table immuable partagee) : sorties identiques bit a bit aux rendus separes");
}

}  // namespace

void runEngineTests() {
  std::printf("[moteur]\n");
  testCausality();
  testDepthZero();
  testDeterminism();
  testMidi();
  testWtOnly();
  testFmOnly();
  testCross();
  testCrossZero();
  testTransitions();
  testBounds();
  testResetRecovery();
  testSilence();
  testInstances();
  TK_CHECK(g_allocations == 0);
  tk::result("G-18", g_allocations == 0, "allocations/liberations pendant Engine::process sur l'ensemble des rendus : %lld (seuil 0)", g_allocations);
}
