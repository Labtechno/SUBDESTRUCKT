// SUBDESTRUCKT — Spike S0.1 — tests automatisés.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <string>
#include "s01/analysis.hpp"
#include "s01/scenarios.hpp"

using namespace s01;

static int g_fail = 0, g_pass = 0;
#define CHECK(cond, ...)                                                                              \
  do {                                                                                                \
    if (cond) { ++g_pass; }                                                                           \
    else {                                                                                            \
      ++g_fail;                                                                                       \
      if (g_fail <= 60) { std::printf("FAIL %s:%d  %s  ", __FILE__, __LINE__, #cond); std::printf(__VA_ARGS__); std::printf("\n"); } \
    }                                                                                                 \
  } while (0)

static bool sameCompiled(const Compiled& a, const Compiled& b) {
  return a.order == b.order && a.present == b.present && a.delayed == b.delayed && a.off == b.off &&
         a.nRoutes == b.nRoutes && std::memcmp(a.eSrc.data(), b.eSrc.data(), sizeof(int) * a.nRoutes) == 0 &&
         std::memcmp(a.eDelayed.data(), b.eDelayed.data(), a.nRoutes) == 0 && a.srcHasDelayed == b.srcHasDelayed;
}

static std::uint64_t hashOf(const RenderOut& r) { return fnv1a(r.host.data(), r.host.size()); }

struct Lcg {
  std::uint64_t s;
  std::uint32_t next() { s = s * 6364136223846793005ull + 1442695040888963407ull; return std::uint32_t(s >> 33); }
};

// ------------------------------------------------------------------------------------------------
static void testSticky() {
  // Règles 2/3 : feed-forward -> I ; fermeture de cycle -> D ; self -> D.
  {
    RouteTable t;
    auto a = t.add(1, 2), b = t.add(2, 3), c = t.add(3, 1), d = t.add(4, 4), e = t.add(0, 3);
    CHECK(t.find(a)->cls == CausalClass::Immediate && t.find(b)->cls == CausalClass::Immediate, "ff");
    CHECK(t.find(c)->cls == CausalClass::Delayed, "fermeture de cycle");
    CHECK(t.find(d)->cls == CausalClass::Delayed, "self");
    CHECK(t.find(e)->cls == CausalClass::Immediate, "ff vers noeud de cycle");
    // Règle 4 : retirer la plus ancienne route du cycle ne reclasse pas la route retardée (S0 la reclassait).
    t.remove(a);
    CHECK(t.find(c)->cls == CausalClass::Delayed, "sticky apres retrait");
    // une nouvelle route identique à la supprimée est Immediate (rien ne la force) et 3->1 reste D
    auto a2 = t.add(1, 2);
    CHECK(t.find(a2)->cls == CausalClass::Immediate && t.find(c)->cls == CausalClass::Delayed, "re-ajout");
    CHECK(a2 != a, "StableRouteID jamais reutilise");
  }
  // Règle 5 : retarget = suppression + création (nouvel id), survivantes inchangées.
  {
    RouteTable t;
    auto a = t.add(1, 2), b = t.add(2, 1), c = t.add(0, 1);
    auto nb = t.retarget(b, 3, 1);
    CHECK(!t.find(b) && t.find(nb) && nb != b, "retarget nouvel id");
    CHECK(t.find(nb)->cls == CausalClass::Immediate, "3->1 sans cycle -> I");
    CHECK(t.find(a)->cls == CausalClass::Immediate && t.find(c)->cls == CausalClass::Immediate, "survivantes");
  }
  // Propriété : séquences pseudo-aléatoires de transactions — aucune survivante ne change jamais de classe,
  // sous-graphe Immediate toujours acyclique, ordre d'évaluation causal, sérialisation stable.
  for (std::uint64_t seed = 1; seed <= 400; ++seed) {
    Lcg g{seed};
    RouteTable t;
    for (int step = 0; step < 60; ++step) {
      std::vector<RouteRec> before = t.routes();
      const std::uint32_t op = g.next() % 10;
      if (op < 5 || t.routes().empty()) {
        int s = int(g.next() % kNodes), d = int(g.next() % kNodes);
        if (!t.findPair(s, d)) t.add(s, d, 1.0);
      } else if (op < 8) {
        auto& v = t.routes();
        t.remove(v[g.next() % v.size()].id);
      } else if (op < 9) {
        auto& v = t.routes();
        auto id = v[g.next() % v.size()].id;
        int s = int(g.next() % kNodes), d = int(g.next() % kNodes);
        if (!t.findPair(s, d)) t.retarget(id, s, d);
      } else {
        auto& v = t.routes();
        auto id = v[g.next() % v.size()].id;
        t.setActive(id, !t.find(id)->active);
      }
      bool unchanged = true;
      for (const auto& r : before) if (const RouteRec* x = t.find(r.id)) unchanged &= (x->cls == r.cls);
      CHECK(unchanged, "seed %llu step %d : survivante reclassee", (unsigned long long)seed, step);
      CHECK(t.immediateAcyclic(), "Immediate acyclique");
      CHECK(verifyCausal(t.compile()), "ordre causal");
    }
    RouteTable r = RouteTable::deserialize(t.serialize());
    CHECK(r.serialize() == t.serialize(), "roundtrip texte");
    CHECK(sameCompiled(r.compile(), t.compile()), "roundtrip compile");
  }
  // Modification src/dst en fondu croisé (nouvelle route créée AVANT la suppression de l'ancienne) :
  // la classe obtenue est toujours celle de la version séquentielle (suppression puis création), car
  // l'ancienne route partage une extrémité avec la nouvelle et ne peut donc pas appartenir à un chemin
  // simple qui fermerait un cycle avec elle. Vérifié exhaustivement sur des états pseudo-aléatoires.
  for (std::uint64_t seed = 1; seed <= 3000; ++seed) {
    Lcg g{seed * 7919};
    RouteTable t;
    for (int k = 0; k < 10; ++k) { int s = int(g.next() % kNodes), d = int(g.next() % kNodes); if (!t.findPair(s, d)) t.add(s, d, 1.0); }
    if (t.routes().empty()) continue;
    const RouteRec old = t.routes()[g.next() % t.routes().size()];
    const bool changeSrc = g.next() & 1;
    const int other = int(g.next() % kNodes);
    const int ns = changeSrc ? other : old.src, nd = changeSrc ? old.dst : other;
    if (t.findPair(ns, nd)) continue;
    RouteTable seq = t; seq.remove(old.id); const auto a = seq.find(seq.add(ns, nd))->cls;
    RouteTable xf = t; const auto idn = xf.add(ns, nd); const auto b = xf.find(idn)->cls; xf.remove(old.id);
    CHECK(a == b, "xfade == sequentiel (seed %llu)", (unsigned long long)seed);
    CHECK(xf.immediateAcyclic(), "xfade acyclique");
  }
  // Règle 7 : lecture d'un état invalide refusée, jamais reclassée silencieusement.
  {
    const char* bad = "S01ROUTES v1 next=3\nroute id=1 src=1 dst=2 class=I active=1 depth=1\nroute id=2 src=2 dst=1 class=I active=1 depth=1\n";
    bool threw = false;
    try { RouteTable::deserialize(bad); } catch (const std::runtime_error&) { threw = true; }
    CHECK(threw, "cycle Immediate refuse a la lecture");
    const char* self = "S01ROUTES v1 next=2\nroute id=1 src=1 dst=1 class=I active=1 depth=1\n";
    threw = false;
    try { RouteTable::deserialize(self); } catch (const std::runtime_error&) { threw = true; }
    CHECK(threw, "self Immediate refuse");
    const char* dup = "S01ROUTES v1 next=3\nroute id=1 src=1 dst=2 class=I active=1 depth=1\nroute id=2 src=1 dst=2 class=D active=1 depth=1\n";
    threw = false;
    try { RouteTable::deserialize(dup); } catch (const std::runtime_error&) { threw = true; }
    CHECK(threw, "paire dupliquee refusee");
    const char* ok = "S01ROUTES v1 next=3\nroute id=1 src=1 dst=2 class=D active=1 depth=1\nroute id=2 src=2 dst=1 class=D active=1 depth=1\n";
    threw = false;
    try { RouteTable::deserialize(ok); } catch (...) { threw = true; }
    CHECK(!threw, "deux Delayed (retard non minimal) accepte : sticky autorise un retard non necessaire");
  }
  // Route inactive : garde sa classe et réserve sa place dans l'acyclicité -> réactivation non structurelle.
  {
    RouteTable t;
    auto a = t.add(1, 2); t.setActive(a, false);
    auto b = t.add(2, 1);
    CHECK(t.find(b)->cls == CausalClass::Delayed, "inactive compte dans l'acyclicite");
    t.setActive(a, true);
    CHECK(t.find(a)->cls == CausalClass::Immediate && t.immediateAcyclic(), "reactivation sans reclassement");
  }

  // Topologie visuellement identique : rendu = f(état persistant), pas de l'historique d'identifiants.
  {
    const EngineParams p = makeParams(48000, 1, TimeRef::T2, Mode::PM);
    // H1 : 1->2 puis 2->1  => (1->2 I, 2->1 D)
    RouteTable h1; h1.add(1, 2, 2.0); h1.add(2, 1, 2.0); h1.add(0, 1, 1.0);
    // H3 : autre historique, mêmes classes finales (ids différents, routes ajoutées/supprimées entre-temps)
    RouteTable h3; auto x = h3.add(0, 1, 1.0); h3.add(3, 4, 1.0); h3.add(1, 2, 2.0); h3.add(2, 1, 2.0);
    h3.remove(h3.findPair(3, 4)->id); (void)x;
    // H2 : ordre inverse => (2->1 I, 1->2 D) : même dessin, autre état persistant
    RouteTable h2; h2.add(2, 1, 2.0); h2.add(1, 2, 2.0); h2.add(0, 1, 1.0);
    auto r1 = render(makeSpec(h1, p, 20000), 128), r3 = render(makeSpec(h3, p, 20000), 128);
    auto r2 = render(makeSpec(h2, p, 20000), 128);
    CHECK(sameCompiled(h1.compile(), h3.compile()), "meme contenu -> meme Compiled");
    CHECK(hashOf(r1) == hashOf(r3), "meme contenu, autre historique -> meme rendu bit-exact");
    CHECK(hashOf(r1) != hashOf(r2), "classes differentes -> rendu different (la classe fait partie de l'etat)");
    for (RouteTable* h : {&h1, &h2, &h3}) {
      RouteTable re = RouteTable::deserialize(h->serialize());
      for (int B : kBlocks) {
        auto a = render(makeSpec(*h, p, 20000), B), b = render(makeSpec(re, p, 20000), 128);
        CHECK(hashOf(a) == hashOf(b), "rechargement -> meme rendu (bloc %d)", B);
      }
    }
  }
}

// ------------------------------------------------------------------------------------------------
static void testCausalityAndStability() {
  const auto topos = topologies();
  for (const Topology& tp : topos)
    for (Mode m : kModes)
      for (TimeRef ref : {TimeRef::T1, TimeRef::T2})
        for (Frac fr : {Frac::Lagrange3, Frac::Linear, Frac::Thiran1})
          for (double sr : {44100.0, 88200.0, 96000.0})
            for (int K : {1, 2, 4}) {
              auto lv = depthLevels(m);
              RouteTable t = RouteTable::build(tp.srcDst, lv[3].v);  // profondeur maximale bornée
              const Compiled c = t.compile();
              CHECK(verifyCausal(c), "causal %s", tp.name);
              if (K < minK(ref, sr, c.nDelayed)) {
                bool threw = false;
                try { auto sp = makeSpec(t, makeParams(sr, K, ref, m, 220, fr), 64); render(sp, 64); }
                catch (const T2NotRepresentable&) { threw = true; }
                CHECK(threw, "T2 non representable doit etre refuse (pas de plancher silencieux) %s sr%.0f K%d", tp.name, sr, K);
                continue;
              }
              if (fr != Frac::Lagrange3 && (ref == TimeRef::T1 || sr == 96000.0)) continue;  // frac sans effet
              EngineParams p = makeParams(sr, K, ref, m, 220, fr);
              p.poison = true;
              auto o = render(makeSpec(t, p, 3000), 128, true);
              Stats s = stats(o.engine.data(), o.engine.size());
              CHECK(s.nan == 0 && s.inf == 0, "poison %s %s %s %s sr%.0f K%d nan=%ld", tp.name, name(m), name(ref), name(fr), sr, K, s.nan);
              CHECK(s.peak <= 1.0 + 1e-9, "borne de sortie moteur %s %s peak=%g", tp.name, name(m), s.peak);
              Stats h = stats(o.host.data(), o.host.size());
              CHECK(h.nan == 0 && h.inf == 0, "sortie hote finie");
              const double f0max = 660.0;  // plus haute porteuse (3 x 220)
              if (m == Mode::LinFM || m == Mode::TZFM) CHECK(o.stats.maxAbsHz <= f0max + kFmClampHz + 1e-6, "borne Hz %g", o.stats.maxAbsHz);
              if (m == Mode::ExpFM) CHECK(o.stats.maxAbsHz <= f0max * 16.0 + 1e-6, "borne Exp %g", o.stats.maxAbsHz);
              if (m == Mode::LinFM) CHECK(o.stats.tzReverse == 0, "LinFM jamais retrograde");
            }
  // Témoin négatif : cycle forcé en Immediate -> verifyCausal refuse et le poison révèle la dépendance.
  {
    RouteTable t; t.add(1, 2, 1.0); t.add(2, 1, 1.0);
    Compiled bad = t.compile();
    for (int e = 0; e < bad.nRoutes; ++e) bad.eDelayed[e] = 0;
    bad.srcHasDelayed.fill(0);
    CHECK(!verifyCausal(bad), "verifyCausal refuse un cycle immediat");
    EngineParams p = makeParams(48000, 1, TimeRef::T1, Mode::LinFM); p.poison = true;
    Engine e(p); e.apply(bad);
    e.setDepthNow(slotOf(2, 1), 100.0); e.setDepthNow(slotOf(1, 2), 100.0);
    std::vector<double> o(256); e.process(o.data(), 256);
    CHECK(stats(o.data(), 256).nan > 0, "poison revele la dependance instantanee");
  }
  // Panic/reset : après un régime extrême, reset() rend l'état neuf (bit-exact).
  for (Mode m : kModes) {
    RouteTable t = RouteTable::build(topologies()[7].srcDst, depthLevels(m)[3].v);
    EngineParams p = makeParams(44100, 2, TimeRef::T2, m);
    Engine a(p); a.apply(t.compile());
    for (auto& r : t.routes()) a.setDepthNow(slotOf(r.dst, r.src), r.depth);
    std::vector<double> x(30000), y(30000), z(30000);
    a.process(x.data(), 30000);
    a.reset(); a.apply(t.compile());
    for (auto& r : t.routes()) a.setDepthNow(slotOf(r.dst, r.src), r.depth);
    a.process(y.data(), 30000);
    Engine b(p); b.apply(t.compile());
    for (auto& r : t.routes()) b.setDepthNow(slotOf(r.dst, r.src), r.depth);
    b.process(z.data(), 30000);
    CHECK(fnv1a(y.data(), y.size()) == fnv1a(z.data(), z.size()) && fnv1a(x.data(), x.size()) == fnv1a(z.data(), z.size()), "panic/reset %s", name(m));
  }
}

// ------------------------------------------------------------------------------------------------
static void testDeterminism() {
  const auto topos = topologies();
  long n = 0;
  for (double sr : kSRs)
    for (TimeRef ref : {TimeRef::T1, TimeRef::T2})
      for (int K : {1, 2, 4})
        for (Mode m : kModes)
          for (const Topology& tp : topos) {
            RouteTable t = RouteTable::build(tp.srcDst, depthLevels(m)[1].v);
            if (K < minK(ref, sr, t.compile().nDelayed)) continue;
            RenderSpec sp = makeSpec(t, makeParams(sr, K, ref, m), 2048);
            // transaction à un offset non aligné : ajout d'une route vers OP4 (structure + rampe)
            RouteTable t2 = t;
            if (!t2.findPair(4, 1)) t2.add(4, 1, depthLevels(m)[1].v);
            Event e; e.hostSample = 777; e.hasTopology = true; e.compiled = t2.compile();
            e.depths.push_back({slotOf(1, 4), depthLevels(m)[1].v, 100, Curve::Smooth});
            if (K < minK(ref, sr, e.compiled.nDelayed)) continue;
            sp.initialDepths.push_back({slotOf(1, 4), 0.0, 0});
            sp.events.push_back(e);
            const auto h0 = hashOf(render(sp, 64));
            CHECK(hashOf(render(sp, 64)) == h0, "rejeu");
            for (int B : kBlocks) CHECK(hashOf(render(sp, B)) == h0, "bloc %d %s %s %s sr%.0f K%d", B, tp.name, name(m), name(ref), sr, K);
            ++n;
          }
  std::printf("  determinisme : %ld configurations x 7 tailles de bloc\n", n);
  // Événements aux mêmes offsets : bloc irrégulier (callback hôte de taille variable) == bloc fixe.
  {
    RouteTable t = RouteTable::build(topologies()[7].srcDst, 1.0);
    RenderSpec sp = makeSpec(t, makeParams(44100, 2, TimeRef::T2, Mode::TZFM), 9000);
    const auto ref = render(sp, 128);
    Processor pr(sp.params, 1024);
    pr.engine().apply(sp.initial);
    for (auto& d : sp.initialDepths) pr.engine().rampDepth(d.slot, d.target, 0, d.curve);
    std::vector<double> out(9000);
    Lcg g{7};
    long long pos = 0;
    while (pos < 9000) { int b = 1 + int(g.next() % 1024); b = int(std::min<long long>(b, 9000 - pos)); pr.processBlock(out.data() + pos, b); pos += b; }
    CHECK(fnv1a(out.data(), out.size()) == hashOf(ref), "blocs de taille variable");
  }
}

// ------------------------------------------------------------------------------------------------
static double probeDelay(TimeRef ref, Frac fr, double sr, int K, bool delayedEdge) {
  // WT <-> OP1, toutes sources sinus ; l'arête mesurée porte 1e-3 rad, l'autre 0. WT->OP1 = I, OP1->WT = D.
  RouteTable t; t.add(0, 1, 0.0); t.add(1, 0, 0.0);
  EngineParams p = makeParams(sr, K, ref, Mode::PM, 997.0, fr, Shape::Sine);
  p.freq = delayedEdge ? std::array<double, kNodes>{0, 997, 0, 0, 0} : std::array<double, kNodes>{997, 0, 0, 0, 0};
  p.tableForSine = false;
  RenderSpec sp = makeSpec(t, p, 24000);
  sp.initialDepths = {{slotOf(1, 0), delayedEdge ? 0.0 : 1e-3, 0}, {slotOf(0, 1), delayedEdge ? 1e-3 : 0.0, 0}};
  auto o = render(sp, 256, false, true);
  const size_t n = o.nodes.size() / kNodes;
  std::vector<double> x(n), y(n);
  for (size_t i = 0; i < n; ++i) {
    x[i] = o.nodes[i * kNodes + (delayedEdge ? 1 : 0)];
    y[i] = o.nodes[i * kNodes + (delayedEdge ? 0 : 1)];
  }
  return measureDelaySeconds(x.data() + n / 8, y.data() + n / 8, n * 3 / 4, sr * K, 997.0);
}

static void testDelays() {
  for (double sr : kSRs)
    for (int K : {1, 2, 4}) {
      CHECK(std::fabs(probeDelay(TimeRef::T1, Frac::Lagrange3, sr, K, true) - 1.0 / sr) < 2e-9, "T1 = 1 ech hote sr%.0f K%d", sr, K);
      CHECK(std::fabs(probeDelay(TimeRef::T1, Frac::Lagrange3, sr, K, false)) < 2e-9, "Immediate = 0");
      if (sr * K < 48000) continue;
      for (Frac fr : {Frac::Lagrange3, Frac::Linear, Frac::Thiran1}) {
        const double d = probeDelay(TimeRef::T2, fr, sr, K, true);
        // retard de phase à 997 Hz : erreur < 0.2 % de 20.833 µs
        CHECK(std::fabs(d - kT2Seconds) < 0.002 * kT2Seconds, "T2 %s sr%.0f K%d mesure %.5f us", name(fr), sr, K, d * 1e6);
      }
    }
  bool threw = false;
  try { makeDelay(TimeRef::T2, Frac::Lagrange3, 1, 44100); } catch (const T2NotRepresentable&) { threw = true; }
  CHECK(threw, "T2 44.1k 1x refuse");
  CHECK(minK(TimeRef::T2, 44100, 1) == 2 && minK(TimeRef::T2, 44100, 0) == 1 && minK(TimeRef::T1, 44100, 3) == 1 &&
        minK(TimeRef::T2, 48000, 2) == 1 && minK(TimeRef::T2, 88200, 2) == 1, "politique K");
  for (double sr : kSRs) for (int K : {1, 2, 4}) {
    if (sr * K < 48000) continue;
    DelaySpec d = makeDelay(TimeRef::T2, Frac::Lagrange3, K, sr);
    if (!d.integer) { double s = 0; for (double w : d.w) s += w; CHECK(std::fabs(s - 1.0) < 1e-12, "somme Lagrange"); }
    DelaySpec th = makeDelay(TimeRef::T2, Frac::Thiran1, K, sr);
    if (!th.integer) {
      CHECK(std::fabs(th.a) < 1.0, "Thiran stable");
      for (double f : {100.0, 5000.0, 20000.0}) CHECK(std::fabs(std::abs(delayResponse(th, 2 * kPi * f / (sr * K))) - 1.0) < 1e-12, "Thiran |H|=1");
    }
  }
}

// ------------------------------------------------------------------------------------------------
static void testResampler() {
  for (int K : {1, 2, 4}) {
    Decimator d(K);
    CHECK(d.latencyHost() >= 0, "latence");
    // sinus 1 kHz moteur -> sortie hôte = sinus 1 kHz retardé de latencyHost (écart < -80 dB)
    const double fsH = 48000, f = 1000;
    const int n = 8192;
    std::vector<double> in(size_t(n) * K), out(n);
    for (int i = 0; i < n * K; ++i) in[i] = std::sin(2 * kPi * f * i / (fsH * K));
    // traitement en blocs irréguliers : état streaming
    int pos = 0; const int bl[] = {1, 7, 64, 300, 1024};
    for (int b = 0; pos < n; b = (b + 1) % 5) { int m = std::min(bl[b], n - pos); d.process(in.data() + size_t(pos) * K, out.data() + pos, m); pos += m; }
    const int L = d.latencyHost();
    double err = 0, ref = 0;
    for (int i = 2000; i < n; ++i) { double e = out[i] - std::sin(2 * kPi * f * (i - L) / fsH); err += e * e; ref += 0.5; }
    CHECK(10 * std::log10(err / ref + 1e-300) < -80, "decimateur K%d alignement/bande passante err=%.1f dB", K, 10 * std::log10(err / ref + 1e-300));
    if (K == 1) continue;
    // réjection : sinus à 0.6 fsH (replié vers 0.4 fsH) doit être atténué de >= 90 dB
    Decimator d2(K);
    for (int i = 0; i < n * K; ++i) in[i] = std::sin(2 * kPi * 0.6 * fsH * i / (fsH * K));
    d2.process(in.data(), out.data(), n);
    double p = 0; for (int i = 2000; i < n; ++i) p += out[i] * out[i];
    p /= (n - 2000);
    CHECK(10 * std::log10(p / 0.5 + 1e-300) < -90, "rejection K%d %.1f dB", K, 10 * std::log10(p / 0.5 + 1e-300));
  }
}

// ------------------------------------------------------------------------------------------------
static void testWavetable() {
  for (Shape s : {Shape::Sine, Shape::Triangle, Shape::Saw, Shape::Asym}) {
    WaveTable w(s);
    for (int L = 0; L < WaveTable::kLevels; ++L) {
      std::vector<std::complex<double>> a(WaveTable::kLen);
      for (int i = 0; i < WaveTable::kLen; ++i) a[i] = w.level(L)[i];
      s0::fft(a);
      const int H = 1 << (WaveTable::kLevels - 1 - L);
      double above = 0, tot = 0;
      for (int k = 1; k < WaveTable::kLen / 2; ++k) { tot += std::norm(a[k]); if (k > H) above += std::norm(a[k]); }
      CHECK(above <= 1e-20 * tot + 1e-30, "table %s niveau %d band-limited", name(s), L);
      CHECK(std::fabs(w.level(L)[0] - w.level(L)[WaveTable::kLen]) < 1e-15, "garde");
    }
    double pk = 0; for (int i = 0; i < WaveTable::kLen; ++i) pk = std::max(pk, std::fabs(w.level(0)[i]));
    CHECK(std::fabs(pk - 1.0) < 1e-12, "normalisation");
  }
  // lecture à fréquence élevée : pas de repliement notable (énergie hors grille harmonique < -60 dB)
  for (Shape s : {Shape::Saw, Shape::Asym}) {
    WaveTable w(s);
    const double fs = 48000, f = 2500;
    std::vector<double> y(48000);
    double ph = 0;
    for (size_t i = 0; i < y.size(); ++i) { y[i] = w.read(ph, f / fs); ph += f / fs; ph -= std::floor(ph); }
    const double inh = inharmonicDb(y.data(), y.size(), fs, f);
    CHECK(inh < -60, "WT %s 2.5 kHz : energie inharmonique %.1f dB", name(s), inh);
  }
  // fondu entre niveaux continu en fréquence : à phase fixe, read() varie continûment quand finc traverse
  // les frontières de niveaux (pas de saut de mip-map).
  for (Shape s : {Shape::Saw, Shape::Asym, Shape::Triangle}) {
    WaveTable w(s);
    double maxd = 0;
    for (double ph : {0.01, 0.13, 0.49, 0.77, 0.999}) {
      double prev = w.read(ph, 1e-5);
      for (int i = 1; i <= 100000; ++i) {
        const double f = 1e-5 * std::pow(0.5 / 1e-5, double(i) / 100000.0);
        const double v = w.read(ph, f);
        maxd = std::max(maxd, std::fabs(v - prev));
        prev = v;
      }
    }
    CHECK(maxd < 0.01, "WT %s continuite en frequence (max saut %g)", name(s), maxd);
  }
}

// ------------------------------------------------------------------------------------------------
static void testRamps() {
  EngineParams p = makeParams(48000, 1, TimeRef::T1, Mode::PM);
  for (Curve cv : {Curve::Linear, Curve::Smooth}) {
    Engine e(p);
    RouteTable t; t.add(1, 2, 0.0);
    e.apply(t.compile());
    e.rampDepth(slotOf(2, 1), 2.0, 100, cv);
    std::vector<double> o(1);
    double prev = 0; bool mono = true;
    for (int i = 0; i < 100; ++i) { e.process(o.data(), 1); const double d = e.depth(slotOf(2, 1)); mono &= d >= prev; prev = d; }
    CHECK(mono && e.depth(slotOf(2, 1)) == 2.0, "rampe %s monotone et exacte", name(cv));
  }
  // profondeur traversant zéro : aucun changement de classe, rendu fini
  RouteTable t = RouteTable::build({{1, 2}, {2, 1}}, -1.0);
  const auto before = t.compile();
  RenderSpec sp = makeSpec(t, makeParams(44100, 2, TimeRef::T2, Mode::TZFM), 20000);
  Event e; e.hostSample = 1000;
  for (auto& r : t.routes()) e.depths.push_back({slotOf(r.dst, r.src), 1.0 * 220, 5000, Curve::Smooth});
  sp.events.push_back(e);
  auto o = render(sp, 64);
  CHECK(stats(o.host.data(), o.host.size()).nan == 0, "zero-cross fini");
  CHECK(sameCompiled(before, t.compile()), "zero-cross : aucune reclassification");
}

int main() {
  std::printf("S0.1 tests\n");
  testSticky();
  std::printf("  sticky ok (%d pass, %d fail)\n", g_pass, g_fail);
  testCausalityAndStability();
  std::printf("  causalite/stabilite (%d pass, %d fail)\n", g_pass, g_fail);
  testDeterminism();
  std::printf("  determinisme (%d pass, %d fail)\n", g_pass, g_fail);
  testDelays();
  testResampler();
  testWavetable();
  testRamps();
  std::printf("S0.1 tests: %d pass, %d fail\n", g_pass, g_fail);
  return g_fail ? 1 : 0;
}
