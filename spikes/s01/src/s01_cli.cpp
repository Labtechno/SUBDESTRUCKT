// SUBDESTRUCKT — Spike S0.1 — campagnes de mesure.
// Usage : s01_cli <matrix|delay|fracresp|fracfb|transition|audio|all> <dir_sortie> [threads]
#include <algorithm>
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include "s01/analysis.hpp"
#include "s01/scenarios.hpp"

using namespace s01;

static int g_threads = 0;

static FILE* openOut(const std::string& dir, const char* f) {
  std::string p = dir + "/" + f;
  FILE* o = std::fopen(p.c_str(), "wb");
  if (!o) { std::perror(p.c_str()); std::exit(2); }
  return o;
}

// Exécute n tâches en parallèle ; chaque tâche retourne ses lignes CSV ; écriture dans l'ordre.
static void parallel(size_t n, const std::function<std::string(size_t)>& job, FILE* o, const char* tag) {
  std::vector<std::string> res(n);
  std::atomic<size_t> next{0}, done{0};
  const int T = g_threads > 0 ? g_threads : std::max(1u, std::thread::hardware_concurrency());
  std::vector<std::thread> th;
  for (int t = 0; t < T; ++t)
    th.emplace_back([&] {
      for (;;) {
        size_t i = next.fetch_add(1);
        if (i >= n) break;
        res[i] = job(i);
        size_t d = done.fetch_add(1) + 1;
        if (d % 200 == 0 || d == n) std::fprintf(stderr, "\r%s %zu/%zu", tag, d, n);
      }
    });
  for (auto& x : th) x.join();
  std::fprintf(stderr, "\n");
  for (auto& s : res) std::fputs(s.c_str(), o);
}

static std::string fmt(const char* f, ...) {
  char buf[4096];
  va_list ap;
  va_start(ap, f);
  std::vsnprintf(buf, sizeof buf, f, ap);
  va_end(ap);
  return buf;
}

// ============================================================================================ matrix
struct MJob { const char* set; double sr; int K; Mode mode; int topo; int depth; TimeRef ref; double f0; };

static void matrix(const std::string& dir) {
  FILE* o = openOut(dir, "matrix.csv");
  std::fprintf(o, "set,sr,K,mode,topo,depth_name,depth,f0,ref,status,nroutes,ndelayed,D_eng,latency,hash,peak,rms,dc,nan,inf,"
                  "max_hz,clamp_frac,negblock_frac,tzrev_frac,max_abs_delayed,inharm_db,oob_db,centroid,chaos_db,lsd_perturb");
  for (double c : s0::thirdOctaveCenters()) std::fprintf(o, ",b%.0f", c);
  std::fprintf(o, "\n");
  const auto topos = topologies();
  std::vector<MJob> jobs;
  for (double sr : kSRs) for (int K : {1, 2, 4}) for (Mode m : kModes) for (size_t t = 0; t < topos.size(); ++t)
    for (int d = 0; d < 4; ++d) for (TimeRef ref : {TimeRef::T1, TimeRef::T2})
      jobs.push_back({"main", sr, K, m, int(t), d, ref, 220.0});
  // sensibilité à f0 (invariance inter-sample-rate) : topologies récursives, profondeurs nominale/forte
  for (double f0 : {55.0, 880.0, 1760.0}) for (double sr : kSRs) for (int K : {1, 2}) for (Mode m : kModes)
    for (int t : {1, 2, 5, 7}) for (int d : {1, 2}) for (TimeRef ref : {TimeRef::T1, TimeRef::T2})
      jobs.push_back({"f0", sr, K, m, t, d, ref, f0});

  parallel(jobs.size(), [&](size_t i) -> std::string {
    const MJob& j = jobs[i];
    const Topology& tp = topos[size_t(j.topo)];
    const DepthLevel dl = depthLevels(j.mode)[size_t(j.depth)];
    RouteTable t = RouteTable::build(tp.srcDst, dl.v);
    const Compiled c = t.compile();
    std::string head = fmt("%s,%.0f,%d,%s,%s,%s,%g,%g,%s", j.set, j.sr, j.K, name(j.mode), tp.name, dl.name, dl.v, j.f0, name(j.ref));
    if (j.K < minK(j.ref, j.sr, c.nDelayed)) return head + fmt(",refused_T2_K,%d,%d\n", c.nRoutes, c.nDelayed);
    EngineParams p = makeParams(j.sr, j.K, j.ref, j.mode, j.f0);
    const long long N = (long long)(1.1 * j.sr), skip = (long long)(0.1 * j.sr);
    RenderOut r = render(makeSpec(t, p, N), 128, true);
    EngineParams pp = p; pp.phase0[1] = 1e-9;  // perturbation infinitésimale -> indicateur de chaos
    RenderOut rp = render(makeSpec(t, pp, N), 128);
    const double* w = r.host.data() + skip;
    const size_t n = size_t(N - skip);
    Stats s = stats(w, n);
    const size_t tail = size_t(0.25 * j.sr);
    const double chaos = nmseDb(rp.host.data() + N - tail, r.host.data() + N - tail, tail);
    const double engN = double(N) * j.K * kNodes;
    auto bands = bandLevels(w, n, j.sr);
    const double lsdP = lsdDb(bandLevels(rp.host.data() + skip, n, j.sr), bands);  // plancher de mesure du régime
    double Deng = std::nan("");
    try { Deng = makeDelay(j.ref, Frac::Lagrange3, j.K, j.sr).D; } catch (const T2NotRepresentable&) {}  // aucun Delayed : sans objet
    std::string row = head + fmt(",ok,%d,%d,%.6f,%d,%016llx,%.6f,%.6f,%.3e,%ld,%ld,%.2f,%.5f,%.5f,%.5f,%.5f,%.2f,%.2f,%.1f,%.2f,%.3f",
        c.nRoutes, c.nDelayed, Deng, r.latencyHost, (unsigned long long)fnv1a(r.host.data(), r.host.size()), s.peak, s.rms, s.dc,
        s.nan, s.inf, r.stats.maxAbsHz, r.stats.clampHits / engN, r.stats.negBlocked / engN, r.stats.tzReverse / engN,
        r.stats.maxAbsDelayed, inharmonicDb(w, n, j.sr, j.f0 / 2), outOfBandDb(r.engine, size_t(skip) * j.K, j.sr * j.K, j.sr),
        centroidHz(w, n, j.sr), chaos, lsdP);
    for (double b : bands) row += fmt(",%.3f", b);
    return row + "\n";
  }, o, "matrix");
  std::fclose(o);
}

// ============================================================================================ delay
static void delayProbe(const std::string& dir) {
  FILE* o = openOut(dir, "delay.csv");
  std::fprintf(o, "ref,frac,sr,K,edge,status,measured_us,expected_us,err_us,D_eng\n");
  for (TimeRef ref : {TimeRef::T1, TimeRef::T2}) for (Frac fr : {Frac::Lagrange3, Frac::Linear, Frac::Thiran1})
    for (double sr : kSRs) for (int K : {1, 2, 4}) for (int del = 0; del < 2; ++del) {
      if (ref == TimeRef::T1 && fr != Frac::Lagrange3) continue;
      RouteTable t; t.add(0, 1, 0.0); t.add(1, 0, 0.0);
      EngineParams p = makeParams(sr, K, ref, Mode::PM, 997.0, fr, Shape::Sine);
      p.freq = del ? std::array<double, kNodes>{0, 997, 0, 0, 0} : std::array<double, kNodes>{997, 0, 0, 0, 0};
      const char* edge = del ? "delayed" : "immediate";
      if (K < minK(ref, sr, 1)) { std::fprintf(o, "%s,%s,%.0f,%d,%s,refused_T2_K,,,,\n", name(ref), name(fr), sr, K, edge); continue; }
      RenderSpec sp = makeSpec(t, p, 24000);
      sp.initialDepths = {{slotOf(1, 0), del ? 0.0 : 1e-3, 0}, {slotOf(0, 1), del ? 1e-3 : 0.0, 0}};
      auto r = render(sp, 256, false, true);
      const size_t n = r.nodes.size() / kNodes;
      std::vector<double> x(n), y(n);
      for (size_t i = 0; i < n; ++i) { x[i] = r.nodes[i * kNodes + (del ? 1 : 0)]; y[i] = r.nodes[i * kNodes + (del ? 0 : 1)]; }
      const double tau = measureDelaySeconds(x.data() + n / 8, y.data() + n / 8, n * 3 / 4, sr * K, 997.0);
      const DelaySpec d = makeDelay(ref, fr, K, sr);
      const double exp = del ? d.D / (sr * K) : 0.0;
      std::fprintf(o, "%s,%s,%.0f,%d,%s,ok,%.6f,%.6f,%.2e,%.6f\n", name(ref), name(fr), sr, K, edge, tau * 1e6, exp * 1e6, (tau - exp) * 1e6, d.D);
    }
  std::fclose(o);
}

// ============================================================================================ fracresp
static void fracResp(const std::string& dir) {
  FILE* o = openOut(dir, "fracresp.csv");
  std::fprintf(o, "sr,K,D_eng,frac,taps_or_mults,f_hz,gain_db,phase_delay_us,delay_err_pct,group_delay_us\n");
  for (double sr : kSRs) for (int K : {1, 2, 4}) {
    if (sr * K < 48000) continue;
    for (Frac fr : {Frac::Lagrange3, Frac::Linear, Frac::Thiran1}) {
      DelaySpec d = makeDelay(TimeRef::T2, fr, K, sr);
      if (d.integer) continue;
      const double fsE = sr * K;
      const int cost = fr == Frac::Lagrange3 ? 4 : 2;
      for (double f : {20.0, 100.0, 1000.0, 2000.0, 5000.0, 10000.0, 15000.0, 20000.0, 30000.0, 40000.0}) {
        if (f >= 0.5 * fsE) continue;  // contenu moteur au-dessus de 20 kHz : il circule aussi dans la boucle
        const double om = 2 * kPi * f / fsE, dom = 1e-4 * om;
        auto H = delayResponse(d, om);
        double phDelay = -std::arg(H) / om;  // om*D < 2*pi au-delà de 24 kHz : déroulement explicite
        while (phDelay < 0.5 * d.D) phDelay += 2 * kPi / om;
        auto H1 = delayResponse(d, om + dom), H0 = delayResponse(d, om - dom);
        double dph = std::arg(H1 / H0);
        const double gd = -dph / (2 * dom);
        std::fprintf(o, "%.0f,%d,%.6f,%s,%d,%.0f,%.4f,%.5f,%.4f,%.5f\n", sr, K, d.D, name(fr), cost, f, 20 * std::log10(std::abs(H)),
                     phDelay / fsE * 1e6, (phDelay - d.D) / d.D * 100.0, gd / fsE * 1e6);
      }
    }
  }
  std::fclose(o);
}

// ============================================================================================ fracfb
// Méthodes fractionnaires en boucle de feedback : écart spectral entre méthodes et à T1 (même K).
static void fracFb(const std::string& dir) {
  FILE* o = openOut(dir, "fracfb.csv");
  std::fprintf(o, "sr,K,mode,topo,depth_name,variant,hash,peak,rms,dc,nan,max_abs_delayed,chaos_db,lsd_vs_lagrange3,lsd_vs_T1\n");
  struct J { double sr; int K; Mode m; int topo; int d; };
  std::vector<J> jobs;
  for (auto sk : std::vector<std::pair<double, int>>{{44100, 2}, {44100, 4}, {88200, 1}, {88200, 2}})
    for (Mode m : kModes) for (int t : {1, 2, 5, 7}) for (int d : {1, 2, 3}) jobs.push_back({sk.first, sk.second, m, t, d});
  const auto topos = topologies();
  parallel(jobs.size(), [&](size_t i) -> std::string {
    const J& j = jobs[i];
    const Topology& tp = topos[size_t(j.topo)];
    const DepthLevel dl = depthLevels(j.m)[size_t(j.d)];
    RouteTable t = RouteTable::build(tp.srcDst, dl.v);
    const long long N = (long long)(1.1 * j.sr), skip = (long long)(0.1 * j.sr);
    const size_t n = size_t(N - skip), tail = size_t(0.25 * j.sr);
    struct V { const char* name; TimeRef ref; Frac fr; };
    const V vs[] = {{"T2_lagrange3", TimeRef::T2, Frac::Lagrange3}, {"T2_linear", TimeRef::T2, Frac::Linear},
                    {"T2_thiran1", TimeRef::T2, Frac::Thiran1}, {"T1", TimeRef::T1, Frac::Lagrange3}};
    std::vector<std::vector<double>> bands(4);
    std::string out;
    std::vector<std::string> pre(4);
    for (int v = 0; v < 4; ++v) {
      EngineParams p = makeParams(j.sr, j.K, vs[v].ref, j.m, 220.0, vs[v].fr);
      RenderOut r = render(makeSpec(t, p, N), 128);
      EngineParams pp = p; pp.phase0[1] = 1e-9;
      RenderOut rp = render(makeSpec(t, pp, N), 128);
      Stats s = stats(r.host.data() + skip, n);
      bands[size_t(v)] = bandLevels(r.host.data() + skip, n, j.sr);
      pre[size_t(v)] = fmt("%.0f,%d,%s,%s,%s,%s,%016llx,%.5f,%.5f,%.3e,%ld,%.5f,%.2f", j.sr, j.K, name(j.m), tp.name, dl.name, vs[v].name,
                           (unsigned long long)fnv1a(r.host.data(), r.host.size()), s.peak, s.rms, s.dc, s.nan, r.stats.maxAbsDelayed,
                           nmseDb(rp.host.data() + N - tail, r.host.data() + N - tail, tail));
    }
    for (int v = 0; v < 4; ++v) out += pre[size_t(v)] + fmt(",%.3f,%.3f\n", lsdDb(bands[size_t(v)], bands[0]), lsdDb(bands[size_t(v)], bands[3]));
    return out;
  }, o, "fracfb");
  std::fclose(o);
}

// ============================================================================================ transition
// Topologie de base : WT->OP1 (I), OP1->OP2 (I), OP2->OP3 (I), OP3->OP1 (D, ferme le cycle).
enum class Sc { AddFF, AddCycle, RemoveImm, RemoveDel, ModSrcSeq, ModSrcXfade, ModDstSeq, ModDstXfade, DepthZero };
static const char* scName(Sc s) {
  const char* n[] = {"add_ff", "add_cycle", "remove_imm", "remove_del", "mod_src_seq", "mod_src_xfade", "mod_dst_seq", "mod_dst_xfade", "depth_zero_cross"};
  return n[int(s)];
}

struct TrSetup { RenderSpec spec; long long t0, t1; int newClass; int reclass; std::vector<long long> instants; };

static TrSetup buildTransition(Sc sc, double sr, int K, TimeRef ref, Mode m, double depth, int R, Curve cv) {
  RouteTable base;
  base.add(0, 1, depth); base.add(1, 2, depth); base.add(2, 3, depth); base.add(3, 1, depth);
  EngineParams p = makeParams(sr, K, ref, m);
  const long long t0 = (long long)(0.4 * sr);
  const long long N = t0 + 2 * 1024 + 8192 + 1024;
  TrSetup s;
  s.spec = makeSpec(base, p, N);
  s.t0 = t0; s.newClass = -1;
  auto topoEv = [&](long long at, const RouteTable& t) { Event e; e.hostSample = at; e.hasTopology = true; e.compiled = t.compile(); return e; };
  auto depthEv = [&](long long at, int slot, double target, int len) { Event e; e.hostSample = at; e.depths.push_back({slot, target, len, cv}); return e; };
  std::vector<RouteRec> before = base.routes();
  RouteTable t = base;
  std::vector<Event>& ev = s.spec.events;
  auto addFlow = [&](int src, int dst) {
    auto id = t.add(src, dst, depth);
    s.newClass = int(t.find(id)->cls);
    Event e = topoEv(t0, t);
    s.spec.initialDepths.push_back({slotOf(dst, src), 0.0, 0});  // slot à 0 avant activation
    e.depths.push_back({slotOf(dst, src), depth, R, cv});
    ev.push_back(e);
    s.t1 = t0 + R;
  };
  auto removeFlow = [&](int src, int dst) {
    ev.push_back(depthEv(t0, slotOf(dst, src), 0.0, R));
    t.remove(t.findPair(src, dst)->id);
    ev.push_back(topoEv(t0 + R, t));
    s.t1 = t0 + R;
  };
  auto modify = [&](int src, int dst, int nsrc, int ndst, bool xfade) {
    const int os = slotOf(dst, src), ns = slotOf(ndst, nsrc);
    s.spec.initialDepths.push_back({ns, 0.0, 0});
    if (!xfade) {  // séquentiel : descente, échange (suppression + création), montée
      ev.push_back(depthEv(t0, os, 0.0, R));
      auto nid = t.retarget(t.findPair(src, dst)->id, nsrc, ndst);
      s.newClass = int(t.find(nid)->cls);
      Event e = topoEv(t0 + R, t);
      e.depths.push_back({ns, depth, R, cv});
      ev.push_back(e);
      s.t1 = t0 + 2 * R;
    } else {  // fondu croisé : nouvelle route créée tant que l'ancienne existe, ancienne supprimée en fin
      auto nid = t.add(nsrc, ndst, depth);
      s.newClass = int(t.find(nid)->cls);
      Event e = topoEv(t0, t);
      e.depths.push_back({os, 0.0, R, cv});
      e.depths.push_back({ns, depth, R, cv});
      ev.push_back(e);
      t.remove(t.findPair(src, dst)->id);
      ev.push_back(topoEv(t0 + R, t));
      s.t1 = t0 + R;
    }
  };
  switch (sc) {
    case Sc::AddFF: addFlow(0, 4); break;           // WT -> OP4 : Immediate
    case Sc::AddCycle: addFlow(2, 0); break;        // OP2 -> WT : ferme WT->OP1->OP2->WT : Delayed
    case Sc::RemoveImm: removeFlow(1, 2); break;
    case Sc::RemoveDel: removeFlow(3, 1); break;
    case Sc::ModSrcSeq: modify(2, 3, 4, 3, false); break;
    case Sc::ModSrcXfade: modify(2, 3, 4, 3, true); break;
    case Sc::ModDstSeq: modify(0, 1, 0, 2, false); break;
    case Sc::ModDstXfade: modify(0, 1, 0, 2, true); break;
    case Sc::DepthZero: ev.push_back(depthEv(t0, slotOf(2, 1), -depth, R)); s.t1 = t0 + R; break;
  }
  std::stable_sort(ev.begin(), ev.end(), [](const Event& a, const Event& b) { return a.hostSample < b.hostSample; });
  // instants structurels : chaque événement (transaction / début de rampe) et chaque fin de rampe
  for (const Event& e : ev) {
    s.instants.push_back(e.hostSample);
    for (const DepthCmd& d : e.depths) s.instants.push_back(e.hostSample + d.rampHost);
  }
  int rc = 0;
  for (const auto& r : before) if (const RouteRec* x = t.find(r.id)) rc += (x->cls != r.cls);
  s.reclass = rc;
  return s;
}

static void transition(const std::string& dir) {
  FILE* o = openOut(dir, "transition.csv");
  std::fprintf(o, "scenario,sr,K,ref,mode,depth_name,R,curve,gesture_ms,new_class,reclassified,nan,jump,burst_db,burst_at_db,ctl_jump,ctl_burst_db,ctl_burst_at_db,chaos_db\n");
  struct J { Sc sc; double sr; TimeRef ref; Mode m; int d; int R; Curve cv; };
  std::vector<J> jobs;
  for (int s = 0; s < 9; ++s) for (double sr : {44100.0, 96000.0}) for (TimeRef ref : {TimeRef::T1, TimeRef::T2})
    for (Mode m : kModes) for (int d : {1, 2}) {
      jobs.push_back({Sc(s), sr, ref, m, d, 0, Curve::Linear});
      for (int R : {64, 128, 256, 512, 1024}) for (Curve cv : {Curve::Linear, Curve::Smooth}) jobs.push_back({Sc(s), sr, ref, m, d, R, cv});
    }
  parallel(jobs.size(), [&](size_t i) -> std::string {
    const J& j = jobs[i];
    const DepthLevel dl = depthLevels(j.m)[size_t(j.d)];
    const int K = minK(j.ref, j.sr, 1);  // politique produit : T1 1x ; T2 >= 2x à 44.1 kHz (la base contient un cycle)
    TrSetup s = buildTransition(j.sc, j.sr, K, j.ref, j.m, dl.v, j.R, j.cv);
    RenderOut r = render(s.spec, 64);
    // témoin : même rendu sans aucun événement, mêmes fenêtres -> variabilité naturelle du régime
    RenderSpec ctlSpec = s.spec; ctlSpec.events.clear();
    RenderOut rc = render(ctlSpec, 64);
    const long long L = r.latencyHost;  // la sortie hôte est retardée de la latence du décimateur
    std::vector<long long> inst;
    for (long long ti : s.instants) inst.push_back(ti + L);
    ClickMetrics cm = clickMetrics(r.host, s.t0 + L, s.t1 + L, 4096, inst);
    ClickMetrics cc = clickMetrics(rc.host, s.t0 + L, s.t1 + L, 4096, inst);
    EngineParams pp = s.spec.params; pp.phase0[1] = 1e-9;
    RenderSpec ps = s.spec; ps.params = pp; ps.events.clear();
    RenderOut rp = render(ps, 64);
    const size_t tail = 8192;
    const double chaos = nmseDb(rp.host.data() + rp.host.size() - tail, rc.host.data() + rc.host.size() - tail, tail);
    const double gest = double(s.t1 - s.t0) / j.sr * 1000.0;
    return fmt("%s,%.0f,%d,%s,%s,%s,%d,%s,%.3f,%s,%d,%ld,%.4f,%.3f,%.3f,%.4f,%.3f,%.3f,%.2f\n", scName(j.sc), j.sr, K, name(j.ref), name(j.m), dl.name,
               j.R, j.R ? name(j.cv) : "hard", gest, s.newClass < 0 ? "-" : (s.newClass ? "D" : "I"), s.reclass,
               stats(r.host.data(), r.host.size()).nan, cm.jump, cm.burstDb, cm.burstAtDb, cc.jump, cc.burstDb, cc.burstAtDb, chaos);
  }, o, "transition");
  std::fclose(o);
}

// ============================================================================================ audio
static void writeWav24(const std::string& path, const std::vector<double>& x, int sr) {
  FILE* f = std::fopen(path.c_str(), "wb");
  if (!f) { std::perror(path.c_str()); std::exit(2); }
  const std::uint32_t dataBytes = std::uint32_t(x.size() * 3);
  auto u32 = [&](std::uint32_t v) { std::fwrite(&v, 4, 1, f); };
  auto u16 = [&](std::uint16_t v) { std::fwrite(&v, 2, 1, f); };
  std::fwrite("RIFF", 1, 4, f); u32(36 + dataBytes); std::fwrite("WAVEfmt ", 1, 8, f);
  u32(16); u16(1); u16(1); u32(std::uint32_t(sr)); u32(std::uint32_t(sr) * 3); u16(3); u16(24);
  std::fwrite("data", 1, 4, f); u32(dataBytes);
  for (double v : x) {
    long s = std::lround(std::max(-1.0, std::min(1.0, v)) * 8388607.0);
    unsigned char b[3] = {(unsigned char)(s & 0xff), (unsigned char)((s >> 8) & 0xff), (unsigned char)((s >> 16) & 0xff)};
    std::fwrite(b, 1, 3, f);
  }
  std::fclose(f);
}

struct AudioCase {
  const char* id; const char* desc; Mode mode; std::vector<std::pair<int, int>> routes; double depth; double f0;
  Shape wt; std::array<double, kNodes> mix;
};

static void audio(const std::string& dir) {
  const std::vector<AudioCase> cases = {
      {"c1_fb_moderate", "OP1 self-feedback -> OP2, PM 1.2 rad, 110 Hz", Mode::PM, {{1, 1}, {1, 2}}, 1.2, 110, Shape::Saw, {0, 0.5, 0.5, 0, 0}},
      {"c2_fb_high", "OP1 self + cycle OP1<->OP2, Exponential FM 1.5 oct, 110 Hz", Mode::ExpFM, {{1, 1}, {1, 2}, {2, 1}}, 1.5, 110, Shape::Saw, {0, 0.5, 0.5, 0, 0}},
      {"c3_wtfm_moderate", "WT(saw)<->OP1, PM 1.0 rad, 110 Hz", Mode::PM, {{0, 1}, {1, 0}}, 1.0, 110, Shape::Saw, {0.6, 0.4, 0, 0, 0}},
      {"c4_wtfm_extreme", "WT(asym)<->OP1..3, TZFM 4000 Hz, 110 Hz", Mode::TZFM, {{0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 0}}, 4000, 110, Shape::Asym, {0.4, 0.2, 0.2, 0.2, 0}},
      {"c5_dense", "dense 4x4 OP, PM 1.0 rad, 110 Hz", Mode::PM, topologies()[4].srcDst, 1.0, 110, Shape::Saw, {0, 0.25, 0.25, 0.25, 0.25}},
      {"c6_drone", "WT/FM 2 cycles independants, ExpFM 0.3 oct, 55 Hz", Mode::ExpFM, topologies()[7].srcDst, 0.3, 55, Shape::Asym, {0.3, 0.2, 0.2, 0.15, 0.15}},
  };
  struct Var { const char* tag; double sr; int K; TimeRef ref; };
  // paires A/B : 44.1 kHz politique produit (T1 1x / T2 2x), 44.1 kHz à K égal (2x/2x), 96 kHz (1x/1x)
  struct Pair { const char* tag; Var a, b; };
  const std::vector<Pair> pairs = {
      {"44k1_policy", {"", 44100, 1, TimeRef::T1}, {"", 44100, 2, TimeRef::T2}},
      {"44k1_sameK2", {"", 44100, 2, TimeRef::T1}, {"", 44100, 2, TimeRef::T2}},
      {"96k", {"", 96000, 1, TimeRef::T1}, {"", 96000, 1, TimeRef::T2}},
  };
  const double seconds = 3.0, fade = 0.02, targetRmsDb = -20.0;
  FILE* key = openOut(dir, "KEY.md");
  FILE* sheet = openOut(dir, "LISTENING.md");
  FILE* meta = openOut(dir, "KEY_audio_metrics.csv");
  std::fprintf(key, "# S0.1 — clé des écoutes aveugles (NE PAS OUVRIR avant d'avoir rempli LISTENING.md)\n\n| fichier | référence | K | sample rate |\n|---|---|---|---|\n");
  std::fprintf(sheet, "# S0.1 — feuille d'écoute (à remplir par une personne ; Claude n'a rien écouté)\n\n"
                      "Pour chaque cas : écouter `REF_48k` (T1 = T2 à 48 kHz), puis X et Y. Niveaux RMS alignés à %.0f dBFS.\n"
                      "Noter : différence audible (oui/non/incertain), lequel est le plus proche de la référence 48 kHz, préférence, remarques.\n\n"
                      "| cas | paire | audible ? | plus proche de REF | préférence | remarques |\n|---|---|---|---|---|---|\n", targetRmsDb);
  std::fprintf(meta, "file,case,pair,ref,K,sr,rms_db_before,gain_db,peak_dbfs,latency_trim\n");
  std::uint64_t lcg = 0x5eed5eedULL;
  for (const AudioCase& c : cases) {
    auto renderVar = [&](double sr, int K, TimeRef ref, int& lat) {
      RouteTable t = RouteTable::build(c.routes, c.depth);
      EngineParams p = makeParams(sr, K, ref, c.mode, c.f0, Frac::Lagrange3, c.wt);
      p.mix = c.mix;
      const long long N = (long long)(seconds * sr);
      RenderOut r = render(makeSpec(t, p, N + 4096), 256);
      lat = r.latencyHost;
      std::vector<double> y(r.host.begin() + lat, r.host.begin() + lat + N);  // compense la latence du décimateur
      const long long nf = (long long)(fade * sr);
      for (long long i = 0; i < nf; ++i) { const double g = 0.5 - 0.5 * std::cos(kPi * double(i) / double(nf)); y[size_t(i)] *= g; y[size_t(N - 1 - i)] *= g; }
      return y;
    };
    // jeu de fichiers du cas : REF + paires ; gain par fichier pour RMS commun, puis abaissement commun si crête > -1 dBFS
    struct F { std::string file; std::vector<double> y; double sr; int K; TimeRef ref; std::string pair; int lat; };
    std::vector<F> fs;
    { int lat; auto y = renderVar(48000, 1, TimeRef::T1, lat); fs.push_back({std::string(c.id) + "_REF_48k.wav", y, 48000, 1, TimeRef::T1, "REF", lat}); }
    for (const Pair& pr : pairs) {
      lcg = lcg * 6364136223846793005ULL + 1442695040888963407ULL;
      const bool swap = (lcg >> 40) & 1;
      for (int side = 0; side < 2; ++side) {
        const Var& v = (side == 0) ? pr.a : pr.b;
        const char* label = ((side == 0) != swap) ? "X" : "Y";
        int lat; auto y = renderVar(v.sr, v.K, v.ref, lat);
        fs.push_back({std::string(c.id) + "_" + pr.tag + "_" + label + ".wav", y, v.sr, v.K, v.ref, pr.tag, lat});
      }
      std::fprintf(sheet, "| %s | %s | | | | |\n", c.id, pr.tag);
    }
    double minHead = 1e9;
    std::vector<double> rmsDb(fs.size());
    for (size_t i = 0; i < fs.size(); ++i) {
      Stats s = stats(fs[i].y.data(), fs[i].y.size());
      rmsDb[i] = 20 * std::log10(s.rms + 1e-30);
      minHead = std::min(minHead, -1.0 - (20 * std::log10(s.peak + 1e-30) + (targetRmsDb - rmsDb[i])));
    }
    const double common = std::min(0.0, minHead);
    for (size_t i = 0; i < fs.size(); ++i) {
      const double g = targetRmsDb - rmsDb[i] + common;
      const double lin = std::pow(10.0, g / 20.0);
      for (double& v : fs[i].y) v *= lin;
      writeWav24(dir + "/" + fs[i].file, fs[i].y, int(fs[i].sr));
      Stats s = stats(fs[i].y.data(), fs[i].y.size());
      std::fprintf(meta, "%s,%s,%s,%s,%d,%.0f,%.2f,%.2f,%.2f,%d\n", fs[i].file.c_str(), c.id, fs[i].pair.c_str(), name(fs[i].ref), fs[i].K, fs[i].sr,
                   rmsDb[i], g, 20 * std::log10(s.peak), fs[i].lat);
      std::fprintf(key, "| %s | %s | %d | %.0f |\n", fs[i].file.c_str(), name(fs[i].ref), fs[i].K, fs[i].sr);
    }
    std::fprintf(stderr, "audio %s (%s) ok\n", c.id, c.desc);
  }
  std::fprintf(sheet, "\nDescriptions des cas :\n\n");
  for (const AudioCase& c : cases) std::fprintf(sheet, "- `%s` : %s\n", c.id, c.desc);
  std::fclose(key); std::fclose(sheet); std::fclose(meta);
}

int main(int argc, char** argv) {
  if (argc < 3) { std::fprintf(stderr, "usage: %s <matrix|delay|fracresp|fracfb|transition|audio|all> <dir> [threads]\n", argv[0]); return 1; }
  std::string cmd = argv[1], dir = argv[2];
  if (argc > 3) g_threads = std::atoi(argv[3]);
  (void)TableBank::instance();  // tables construites hors campagne
  const bool all = cmd == "all";
  if (all || cmd == "delay") delayProbe(dir);
  if (all || cmd == "fracresp") fracResp(dir);
  if (all || cmd == "fracfb") fracFb(dir);
  if (all || cmd == "transition") transition(dir);
  if (all || cmd == "matrix") matrix(dir);
  if (cmd == "audio") audio(dir);  // écrit dans un répertoire audio dédié
  return 0;
}
