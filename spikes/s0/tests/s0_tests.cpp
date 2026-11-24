// SUBDESTRUCKT — Spike S0 — tests automatisés des invariants (§9).
#include <cstdio>
#include <cstdlib>
#include <set>
#include "s0/analysis.hpp"
#include "s0/configs.hpp"

using namespace s0;

static int g_fail = 0, g_pass = 0;
#define CHECK(cond, ...)                                  \
  do {                                                    \
    if (cond) { ++g_pass; }                               \
    else { ++g_fail; std::printf("FAIL %s:%d  %s  ", __FILE__, __LINE__, #cond); std::printf(__VA_ARGS__); std::printf("\n"); } \
  } while (0)

static const double kSRs[] = {44100, 48000, 88200, 96000};
static const int kBlocks[] = {16, 32, 64, 128, 256, 512, 1024};
static const Strategy kStrats[] = {Strategy::A, Strategy::B, Strategy::Bc};
static const TimeRef kRefs[] = {TimeRef::T1, TimeRef::T2};

int main() {
  const auto cfgs = staticConfigs();
  const auto rs = ratioSets()[0];

  // 1. Compilation : causalité statique, déterminisme, A = tout retardé, B/Bc ne retardent que des cycles.
  for (const Config& c : cfgs)
    for (Strategy st : kStrats) {
      Compiled a = compile(c.routes, st), b = compile(c.routes, st);
      CHECK(verifyCausal(a), "cfg %d %s", c.id, name(st));
      CHECK(std::memcmp(&a.order, &b.order, sizeof(a.order)) == 0 && a.nDelayed == b.nDelayed, "determinisme compile");
      if (st == Strategy::A) CHECK(a.nDelayed == a.nRoutes, "A: tout retarde");
    }
  { // cascade : B ne retarde rien ; cycle simple : exactement 1 route retardée ; dense : B retarde moins que A
    CHECK(compile(cfgs[0].routes, Strategy::B).nDelayed == 0, "cascade B=0 retard");
    CHECK(compile(cfgs[2].routes, Strategy::B).nDelayed == 1, "cycle OP1<->OP2 B=1 retard");
    CHECK(compile(cfgs[4].routes, Strategy::B).nDelayed < compile(cfgs[4].routes, Strategy::A).nDelayed, "dense");
  }

  // 2. Preuve dynamique d'absence de dépendance instantanée : "poison" NaN sur les sorties courantes.
  for (const Config& c : cfgs)
    for (Strategy st : kStrats)
      for (TimeRef ref : kRefs)
        for (int K : {1, 2, 4})
          for (double sr : {44100.0, 96000.0}) {
            auto sp = makeSpec(c.routes, st, ref, sr, K, 220.0, rs, 1.0, 4096, /*poison*/ true);
            auto out = render(sp, 128);
            Stats s = stats(out.data(), out.size());
            CHECK(s.nan == 0 && s.inf == 0, "poison cfg %d %s %s K%d sr%.0f nan=%ld", c.id, name(st), name(ref), K, sr, s.nan);
            CHECK(s.peak <= 1.0 + 1e-12, "peak %f", s.peak);
          }
  { // le poison détecte bien une lecture immédiate fautive : topologie B compilée à la main avec cycle immédiat
    Compiled bad = compile(mk({{2, 1}, {1, 2}}), Strategy::B);
    // on force les deux routes en immédiat (violation volontaire) :
    for (int e = 0; e < bad.nRoutes; ++e) bad.eDelayed[e] = 0;
    CHECK(!verifyCausal(bad), "verifyCausal doit refuser un cycle immediat");
    EngineParams p; p.poison = true; p.freq = {0, 220, 220, 0, 0};
    Engine eng(p); eng.apply(bad);
    eng.setDepthNow(slotOf(2, 1), 1.0); eng.setDepthNow(slotOf(1, 2), 1.0);
    std::vector<double> o(256); eng.process(o.data(), 256);
    CHECK(stats(o.data(), 256).nan > 0, "poison doit reveler la dependance instantanee");
  }

  // 3. Déterminisme : même rendu, reset, taille de bloc (bit-exact).
  for (const Config& c : cfgs)
    for (Strategy st : kStrats)
      for (TimeRef ref : kRefs)
        for (double sr : kSRs)
          for (int K : {1, 2, 4}) {
            auto sp = makeSpec(c.routes, st, ref, sr, K, 220.0, rs, 1.0, 6000);
            auto ref0 = render(sp, 64);
            auto h0 = fnv1a(ref0.data(), ref0.size());
            auto again = render(sp, 64);
            CHECK(fnv1a(again.data(), again.size()) == h0, "rejeu");
            for (int B : kBlocks) {
              auto o = render(sp, B);
              CHECK(fnv1a(o.data(), o.size()) == h0, "bloc %d cfg %d %s %s K%d sr%.0f", B, c.id, name(st), name(ref), K, sr);
            }
          }
  { // reset d'état : Engine réutilisé après reset() == neuf
    auto sp = makeSpec(cfgs[4].routes, Strategy::B, TimeRef::T2, 48000, 2, 220.0, rs, 8.0, 5000);
    Engine e(sp.params); e.apply(sp.initial);
    for (auto& d : sp.initialDepths) e.setDepthNow(d.slot, d.target);
    std::vector<double> a(5000), b(5000);
    e.process(a.data(), 5000);
    e.reset(); e.apply(sp.initial);
    for (auto& d : sp.initialDepths) e.setDepthNow(d.slot, d.target);
    e.process(b.data(), 5000);
    CHECK(fnv1a(a.data(), 5000) == fnv1a(b.data(), 5000), "reset");
  }

  // 4. Retard mesuré (sonde petit signal) : A et B ; T1 et T2.
  {
    auto measure = [&](Strategy st, TimeRef ref, double sr, int K, bool delayedEdge) {
      // sonde : routes (1<-0),(0<-1) ; l'une porte la modulation (d=1e-3 rad), l'autre a une profondeur 0
      std::vector<Route> r = mk({{1, 0}, {0, 1}});
      RatioSet probe{"probe", {1, 1, 1, 1, 1}};
      auto sp = makeSpec(r, st, ref, sr, K, 997.0, probe, 0.0, 48000);
      sp.params.freq = delayedEdge ? std::array<double, kNodes>{0, 997, 0, 0, 0}
                                   : std::array<double, kNodes>{997, 0, 0, 0, 0};
      sp.initialDepths.clear();
      sp.initialDepths.push_back({slotOf(1, 0), delayedEdge ? 0.0 : 1e-3, 0});
      sp.initialDepths.push_back({slotOf(0, 1), delayedEdge ? 1e-3 : 0.0, 0});
      std::vector<double> nodes;
      auto out = render(sp, 256, &nodes);
      const size_t n = out.size();
      std::vector<double> x(n), y(n);
      for (size_t i = 0; i < n; ++i) {
        x[i] = nodes[i * kNodes + (delayedEdge ? 1 : 0)];
        y[i] = nodes[i * kNodes + (delayedEdge ? 0 : 1)];
      }
      return measureDelaySeconds(x.data() + n / 8, y.data() + n / 8, n * 3 / 4, sr * K, 997.0);
    };
    for (double sr : kSRs) {
      // A, T1 : 1/sr ; T2 : max(1/48000, 1/(sr*K)) — avec K=1 et 44.1 kHz, plancher à 1 éch. moteur
      double a1 = measure(Strategy::A, TimeRef::T1, sr, 1, true);
      CHECK(std::fabs(a1 - 1.0 / sr) < 2e-8, "A/T1 sr%.0f mesure %.4g us attendu %.4g", sr, a1 * 1e6, 1e6 / sr);
      double a2 = measure(Strategy::A, TimeRef::T2, sr, 4, true);
      CHECK(std::fabs(a2 - kT2Seconds) < 5e-8, "A/T2 K4 sr%.0f mesure %.4g us (Lagrange)", sr, a2 * 1e6);
      double bimm = measure(Strategy::B, TimeRef::T1, sr, 1, false);
      CHECK(std::fabs(bimm) < 2e-8, "B route immediate: retard %.4g us attendu 0", bimm * 1e6);
      double bdel = measure(Strategy::B, TimeRef::T1, sr, 1, true);
      CHECK(std::fabs(bdel - 1.0 / sr) < 2e-8, "B route retardee T1 %.4g us", bdel * 1e6);
    }
    DelaySpec d441 = makeDelay(TimeRef::T2, 1, 44100.0);
    CHECK(d441.floored, "T2 a 44.1k 1x doit etre plafonne (0.91875 < 1)");
    CHECK(!makeDelay(TimeRef::T2, 2, 44100.0).floored, "T2 a 44.1k 2x honorable");
    CHECK(!makeDelay(TimeRef::T2, 1, 48000.0).floored, "T2 48k 1x = 1 echantillon");
  }
  { // somme des poids de Lagrange = 1
    for (double sr : kSRs) for (int K : {1, 2, 4}) {
      DelaySpec d = makeDelay(TimeRef::T2, K, sr);
      if (!d.integer) { double s = 0; for (double w : d.w) s += w; CHECK(std::fabs(s - 1.0) < 1e-12, "Lagrange somme"); }
    }
  }

  // 5. Profondeur zéro : la classification ne dépend pas de la profondeur ; rendu == route absente.
  {
    auto cyc = cfgs[2].routes;
    auto spZero = makeSpec(cyc, Strategy::B, TimeRef::T1, 48000, 1, 220, rs, 0.0, 8000);
    auto spNone = makeSpec(cyc, Strategy::B, TimeRef::T1, 48000, 1, 220, rs, 0.0, 8000);
    auto o0 = render(spZero, 64);
    // même topologie, une route à 1.0 : la classe des routes ne change pas
    Compiled c0 = compile(cyc, Strategy::B);
    CHECK(c0.nDelayed == 1, "profondeur ignoree par compile");
    for (double d : {0.0, 1e-9, 0.01, 1.0}) {
      auto sp = makeSpec(cyc, Strategy::B, TimeRef::T1, 48000, 1, 220, rs, d, 100);
      CHECK(reclassified(sp.initial, c0) == 0, "classification stable depth=%g", d);
    }
    // balayage continu à travers 0 (rampe -1 -> +1) : pas de reclassification, pas de NaN
    auto sp = makeSpec(cyc, Strategy::B, TimeRef::T1, 48000, 1, 220, rs, -1.0, 20000);
    for (auto& d : sp.initialDepths) d.rampHost = 0;
    Event e; e.hostSample = 1000; e.compiled = sp.initial;
    for (const Route& r : cyc) e.depths.push_back({slotOf(r.dst, r.src), 1.0, 10000});
    sp.events.push_back(e);
    auto o = render(sp, 64);
    CHECK(stats(o.data(), o.size()).nan == 0, "sweep profondeur");
    CHECK(reclassified(sp.initial, e.compiled) == 0, "sweep: pas de reclassification");
    (void)o0;
  }

  // 6. Changement de topologie transactionnel (cfg 9) : indépendance au bloc, stabilité des classes.
  {
    for (Strategy st : kStrats) for (TimeRef ref : kRefs) {
      auto before = cfg9Before(), after = cfg9After();
      RenderSpec sp = makeSpec(before, st, ref, 48000, 2, 220, rs, 1.0, 12000);
      Event e; e.hostSample = 5000; e.compiled = compile(after, st);
      for (const Route& r : after) e.depths.push_back({slotOf(r.dst, r.src), 1.0, r.dst == 1 ? 256 : 0});
      sp.events.push_back(e);
      auto h0 = render(sp, 64);
      auto H0 = fnv1a(h0.data(), h0.size());
      for (int B : kBlocks) { auto o = render(sp, B); CHECK(fnv1a(o.data(), o.size()) == H0, "cfg9 bloc %d %s %s", B, name(st), name(ref)); }
      CHECK(verifyCausal(e.compiled), "cfg9 causal");
      int rc = reclassified(sp.initial, e.compiled);
      if (st == Strategy::A || st == Strategy::B) CHECK(rc == 0, "cfg9 %s reclassification=%d", name(st), rc);
    }
    // événements à des échantillons non alignés sur le bloc
    for (std::int64_t at : {1, 63, 64, 65, 4999, 5000}) {
      RenderSpec sp = makeSpec(cfg9Before(), Strategy::B, TimeRef::T1, 48000, 4, 220, rs, 1.0, 9000);
      Event e; e.hostSample = at; e.compiled = compile(cfg9After(), Strategy::B);
      for (const Route& r : cfg9After()) e.depths.push_back({slotOf(r.dst, r.src), 1.0, 0});
      sp.events.push_back(e);
      auto h = render(sp, 64); auto H = fnv1a(h.data(), h.size());
      for (int B : kBlocks) { auto o = render(sp, B); CHECK(fnv1a(o.data(), o.size()) == H, "event@%lld bloc %d", (long long)at, B); }
    }
  }

  std::printf("S0 tests: %d pass, %d fail\n", g_pass, g_fail);
  return g_fail ? 1 : 0;
}
