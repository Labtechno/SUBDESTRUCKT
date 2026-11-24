// SUBDESTRUCKT — Spike S0 — campagnes de mesure. Usage : s0_cli <matrix|delay|transition|perf> <dir_sortie>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include "s0/analysis.hpp"
#include "s0/configs.hpp"

using namespace s0;

static const double kSRs[] = {44100, 48000, 88200, 96000};
static const Strategy kStrats[] = {Strategy::A, Strategy::B, Strategy::Bc};
static const TimeRef kRefs[] = {TimeRef::T1, TimeRef::T2};

static FILE* openOut(const std::string& dir, const char* f) {
  std::string p = dir + "/" + f;
  FILE* o = std::fopen(p.c_str(), "w");
  if (!o) { std::perror(p.c_str()); std::exit(2); }
  return o;
}

// ---------------------------------------------------------------------------- matrix
struct Win { std::vector<double> x; };

static void matrix(const std::string& dir) {
  FILE* o = openOut(dir, "matrix.csv");
  std::fprintf(o, "set,cfg,cfgname,sr,K,strat,ref,depth,f0,rset,hash,peak,rms,dc,nan,inf,nroutes,ndelayed,floored,Deff_eng,nmse_vs_K4_db,nmse_vs_A_db");
  for (double c : thirdOctaveCenters()) std::fprintf(o, ",b%.0f", c);
  std::fprintf(o, "\n");
  const auto cfgs = staticConfigs();
  const auto rsets = ratioSets();

  struct Job { const char* set; double sr; double f0; int rs; };
  std::vector<Job> jobs;
  for (double sr : kSRs) jobs.push_back({"main", sr, 220.0, 0});
  for (double f0 : {55.0, 110.0, 220.0, 440.0, 997.0}) for (int r = 0; r < 2; ++r) jobs.push_back({"sens", 48000.0, f0, r});

  long rows = 0;
  for (const Job& j : jobs) {
    const int Ks[3] = {4, 2, 1};
    const bool fullK = std::strcmp(j.set, "main") == 0;
    const std::int64_t hostSamples = std::int64_t(1.1 * j.sr);
    const size_t skip = size_t(0.1 * j.sr);
    for (const Config& c : cfgs)
      for (double depth : depthLevels())
        for (TimeRef ref : kRefs) {
          std::map<int, std::map<int, std::vector<double>>> win;  // [K][strat] fenêtre analysée (hôte)
          for (int ki = 0; ki < 3; ++ki) {
            const int K = Ks[ki];
            if (!fullK && K != 1) continue;
            for (int si = 0; si < 3; ++si) {
              const Strategy st = kStrats[si];
              RenderSpec sp = makeSpec(c.routes, st, ref, j.sr, K, j.f0, rsets[j.rs], depth, hostSamples);
              std::vector<double> eng = render(sp, 128);
              std::uint64_t h = fnv1a(eng.data(), eng.size());
              std::vector<double> host = decimate(eng, K);
              std::vector<double> w(host.begin() + skip, host.end());
              win[K][si] = w;
              Stats s = stats(w.data(), w.size());
              auto bands = bandLevelsDb(w.data(), w.size(), j.sr);
              DelaySpec d = makeDelay(ref, K, j.sr);
              double nK4 = std::nan(""), nA = std::nan("");
              if (fullK && K != 4) nK4 = nmseDb(w.data(), win[4][si].data(), std::min(w.size(), win[4][si].size()));
              if (si > 0) nA = nmseDb(w.data(), win[K][0].data(), w.size());
              std::fprintf(o, "%s,%d,%s,%.0f,%d,%s,%s,%g,%g,%s,%016llx,%.6f,%.6f,%.3e,%ld,%ld,%d,%d,%d,%.6f,%.3f,%.3f",
                           j.set, c.id, c.name, j.sr, K, name(st), name(ref), depth, j.f0, rsets[j.rs].name,
                           (unsigned long long)h, s.peak, s.rms, s.dc, s.nan, s.inf, sp.initial.nRoutes,
                           sp.initial.nDelayed, int(d.floored), d.D, nK4, nA);
              for (double b : bands) std::fprintf(o, ",%.3f", b);
              std::fprintf(o, "\n");
              ++rows;
            }
          }
        }
    std::fprintf(stderr, "matrix: %s sr=%.0f f0=%g rs=%d done (%ld rows)\n", j.set, j.sr, j.f0, j.rs, rows);
  }
  std::fclose(o);
}

// ---------------------------------------------------------------------------- delay
static void delayProbe(const std::string& dir) {
  FILE* o = openOut(dir, "delay.csv");
  std::fprintf(o, "strat,ref,sr,K,edge,measured_us,expected_us,floored,Deff_eng\n");
  for (Strategy st : kStrats) for (TimeRef ref : kRefs) for (double sr : kSRs) for (int K : {1, 2, 4})
    for (int delayedEdge = 0; delayedEdge < 2; ++delayedEdge) {
      std::vector<Route> r = mk({{1, 0}, {0, 1}});
      RatioSet probe{"probe", {1, 1, 1, 1, 1}};
      auto sp = makeSpec(r, st, ref, sr, K, 997.0, probe, 0.0, 48000);
      sp.params.freq = delayedEdge ? std::array<double, kNodes>{0, 997, 0, 0, 0} : std::array<double, kNodes>{997, 0, 0, 0, 0};
      sp.initialDepths.clear();
      sp.initialDepths.push_back({slotOf(1, 0), delayedEdge ? 0.0 : 1e-3, 0});
      sp.initialDepths.push_back({slotOf(0, 1), delayedEdge ? 1e-3 : 0.0, 0});
      std::vector<double> nodes;
      auto out = render(sp, 256, &nodes);
      const size_t n = out.size();
      std::vector<double> x(n), y(n);
      for (size_t i = 0; i < n; ++i) { x[i] = nodes[i * kNodes + (delayedEdge ? 1 : 0)]; y[i] = nodes[i * kNodes + (delayedEdge ? 0 : 1)]; }
      double tau = measureDelaySeconds(x.data() + n / 8, y.data() + n / 8, n * 3 / 4, sr * K, 997.0);
      DelaySpec d = makeDelay(ref, K, sr);
      // attendu pour la sonde : A -> D ; B -> 0 si l'arête est immédiate, D si retardée ; D en secondes
      bool isDelayed = (st == Strategy::A) || delayedEdge;
      double expect = isDelayed ? d.D / (sr * K) : 0.0;
      std::fprintf(o, "%s,%s,%.0f,%d,%s,%.5f,%.5f,%d,%.6f\n", name(st), name(ref), sr, K, delayedEdge ? "delayed_probe" : "immediate_probe",
                   tau * 1e6, expect * 1e6, int(d.floored), d.D);
    }
  std::fclose(o);
}

// ---------------------------------------------------------------------------- transition
struct Scenario { const char* name; std::vector<Route> before, after; bool removal; };

static double clickRatio(const std::vector<double>& y, size_t t0) {
  // rapport entre le plus grand |Δy| autour de t0 et le RMS de |Δy| sur une fenêtre précédente
  double ref = 0; size_t m = 0;
  for (size_t i = t0 - 3000; i < t0 - 100; ++i) { double d = y[i] - y[i - 1]; ref += d * d; ++m; }
  ref = std::sqrt(ref / m);
  double mx = 0;
  for (size_t i = t0 - 1; i <= t0 + 3; ++i) mx = std::max(mx, std::fabs(y[i] - y[i - 1]));
  return mx / std::max(ref, 1e-12);
}

static void transition(const std::string& dir) {
  FILE* o = openOut(dir, "transition.csv");
  std::fprintf(o, "scenario,strat,ref,sr,K,depth,mode,reclassified,delayed_before,delayed_after,causal_after,click_ratio,control_max,peak,nan,hash\n");
  const std::vector<Route> dense = staticConfigs()[4].routes;
  std::vector<Scenario> scs = {
      {"add_closing_cycle_cascade", cfg9Before(), cfg9After(), false},
      {"add_closing_cycle_reverse_chain", mk({{1, 2}, {2, 3}}), mk({{1, 2}, {2, 3}, {3, 1}}), false},
      {"add_closing_cycle_via_WT", mk({{1, 0}, {2, 1}, {3, 2}}), mk({{1, 0}, {2, 1}, {3, 2}, {0, 3}}), false},
      {"remove_oldest_route_cycle3", mk({{2, 1}, {3, 2}, {1, 3}}), mk({{3, 2}, {1, 3}}), true},
      {"remove_oldest_route_dense", dense, std::vector<Route>(dense.begin() + 1, dense.end()), true},
  };
  const RatioSet rs = ratioSets()[0];
  for (const Scenario& sc : scs)
    for (Strategy st : kStrats) for (TimeRef ref : kRefs)
      for (double sr : {44100.0, 96000.0}) for (int K : {1, 2})
        for (double depth : {1.0, 8.0}) for (int mode = 0; mode < 2; ++mode) {
          const std::int64_t N = std::int64_t(sr);       // 1 s
          const std::int64_t t0 = N / 2;                   // événement structurel
          const int ramp = 256;
          RenderSpec sp = makeSpec(sc.before, st, ref, sr, K, 220.0, rs, depth, N);
          Event e;
          Compiled after = compile(sc.after, st);
          e.compiled = after;
          if (!sc.removal) {
            // ajout : mode 0 = profondeur immédiate ; mode 1 = rampe 0 -> depth après la transaction
            e.hostSample = t0;
            for (const Route& r : sc.after) {
              bool isNew = !sp.initial.present[slotOf(r.dst, r.src)];
              if (isNew) e.depths.push_back({slotOf(r.dst, r.src), depth, mode ? ramp : 0});
            }
          } else {
            // retrait : mode 0 = retrait sec ; mode 1 = rampe vers 0 avant la transaction (événement différé)
            std::vector<int> gone;
            for (const Route& r : sc.before) if (!after.present[slotOf(r.dst, r.src)]) gone.push_back(slotOf(r.dst, r.src));
            if (mode) {
              Event ramps; ramps.hostSample = t0 - ramp - 1; ramps.compiled = sp.initial;
              for (int s : gone) ramps.depths.push_back({s, 0.0, ramp});
              sp.events.push_back(ramps);
            }
            e.hostSample = t0;
          }
          sp.events.push_back(e);
          std::vector<double> eng = render(sp, 64);
          std::vector<double> host = decimate(eng, K);
          Stats s = stats(host.data(), host.size());
          double cr = clickRatio(host, size_t(t0));
          double ctl = 0;  // variabilité naturelle : même mesure à des instants sans événement
          for (int k = 3; k <= 8; ++k) ctl = std::max(ctl, clickRatio(host, size_t(t0) + size_t(k) * 1500));
          std::fprintf(o, "%s,%s,%s,%.0f,%d,%g,%s,%d,%d,%d,%d,%.3f,%.3f,%.4f,%ld,%016llx\n", sc.name, name(st), name(ref), sr, K, depth,
                       mode ? "ramped" : "hard", reclassified(sp.initial, after), sp.initial.nDelayed, after.nDelayed,
                       int(verifyCausal(after)), cr, ctl, s.peak, s.nan, (unsigned long long)fnv1a(eng.data(), eng.size()));
        }
  std::fclose(o);
}

// ---------------------------------------------------------------------------- perf
static void perf(const std::string& dir) {
  FILE* o = openOut(dir, "perf.csv");
  std::fprintf(o, "cfg,cfgname,strat,ref,sr,K,ns_per_engine_sample,ns_per_host_sample,compile_ns\n");
  const auto cfgs = staticConfigs();
  const RatioSet rs = ratioSets()[0];
  for (int ci : {0, 2, 4, 7})
    for (Strategy st : kStrats) for (TimeRef ref : kRefs) for (int K : {1, 2, 4}) {
      const double sr = 48000;
      std::vector<double> t;
      RenderSpec sp = makeSpec(cfgs[ci].routes, st, ref, sr, K, 220.0, rs, 1.0, std::int64_t(sr) * 2);
      for (int rep = 0; rep < 5; ++rep) {
        auto a = std::chrono::steady_clock::now();
        auto out = render(sp, 256);
        auto b = std::chrono::steady_clock::now();
        volatile double sink = out[out.size() / 2]; (void)sink;
        t.push_back(std::chrono::duration<double, std::nano>(b - a).count() / double(out.size()));
      }
      std::sort(t.begin(), t.end());
      auto c0 = std::chrono::steady_clock::now();
      for (int i = 0; i < 20000; ++i) { Compiled c = compile(cfgs[ci].routes, st); volatile int x = c.nDelayed; (void)x; }
      auto c1 = std::chrono::steady_clock::now();
      double cns = std::chrono::duration<double, std::nano>(c1 - c0).count() / 20000.0;
      std::fprintf(o, "%d,%s,%s,%s,%.0f,%d,%.2f,%.2f,%.0f\n", cfgs[ci].id, cfgs[ci].name, name(st), name(ref), sr, K, t[2], t[2] * K, cns);
    }
  std::fclose(o);
}

int main(int argc, char** argv) {
  if (argc < 3) { std::fprintf(stderr, "usage: %s <matrix|delay|transition|perf> <dir>\n", argv[0]); return 1; }
  std::string cmd = argv[1], dir = argv[2];
  if (cmd == "matrix") matrix(dir);
  else if (cmd == "delay") delayProbe(dir);
  else if (cmd == "transition") transition(dir);
  else if (cmd == "perf") perf(dir);
  else { std::fprintf(stderr, "commande inconnue\n"); return 1; }
  return 0;
}
