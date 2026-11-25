// SUBDESTRUCKT — Spike S0.1 — benchmark matriciel T1/T2 (cible : runner GitHub macos-15 arm64, Apple M1 virtualisé).
// Protocole et seuils : bench/THRESHOLDS.md (fixés avant mesure).
// Usage : s01_bench_matrix <dossier_sortie> [répétitions=7] [secondes_par_répétition=1.0] [chauffe_globale_s=3]
// Sorties : bench_matrix.csv (une ligne par configuration), bench_reps.csv (une ligne par configuration et répétition),
//           bench_isolated.csv (lecteurs de retard, décimateur).
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <new>
#include <random>
#include <string>
#include <vector>
#include "s01/analysis.hpp"
#include "s01/scenarios.hpp"

static std::atomic<long long> g_allocs{0};
static std::atomic<bool> g_count{false};
void* operator new(std::size_t n) {
  if (g_count.load(std::memory_order_relaxed)) g_allocs.fetch_add(1, std::memory_order_relaxed);
  if (void* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void* operator new[](std::size_t n) { return operator new(n); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t) noexcept { std::free(p); }

using namespace s01;
using Clock = std::chrono::steady_clock;

struct Cfg {
  double sr; int K; TimeRef ref; Frac frac; Mode mode; int topo; int block;
  bool rejected = false; std::string why;
  std::unique_ptr<Processor> pr;
  int nRoutes = 0, nDelayed = 0, latency = 0;
  double D = 0; bool integer = true;
  std::vector<double> times;     // ns par bloc, toutes répétitions
  std::vector<double> repMeans;  // ns moyen par répétition
  std::vector<double> out;
  long long allocs = 0, nan = 0, inf = 0;
  double peak = 0;
};

static const int kTopos[] = {1, 5, 4, 7};  // self_feedback, wt_op1 (représentatives) ; dense4x4, wtfm_dense_2cycles (stress)
static const char* topoClass(int t) { return (t == 4 || t == 7) ? "stress" : "representative"; }

static void runBlocks(Cfg& c, int nBlocks, bool record) {
  double* y = c.out.data();
  for (int i = 0; i < nBlocks; ++i) {
    const auto a = Clock::now();
    c.pr->processBlock(y, c.block);
    const auto b = Clock::now();
    if (record) c.times.push_back(std::chrono::duration<double, std::nano>(b - a).count());
    for (int k = 0; k < c.block; ++k) {  // stabilité, hors chronométrage
      const double v = y[k];
      if (std::isnan(v)) ++c.nan; else if (std::isinf(v)) ++c.inf; else c.peak = std::max(c.peak, std::fabs(v));
    }
  }
}

static double pct(std::vector<double> s, double p) {
  std::sort(s.begin(), s.end());
  return s[size_t(std::min<double>(double(s.size() - 1), std::floor(p * double(s.size()))))];
}

int main(int argc, char** argv) {
  const std::string dir = argc > 1 ? argv[1] : ".";
  const int reps = argc > 2 ? std::atoi(argv[2]) : 7;
  const double secs = argc > 3 ? std::atof(argv[3]) : 1.0;
  const double warm = argc > 4 ? std::atof(argv[4]) : 3.0;
  (void)TableBank::instance();
  const auto topos = topologies();

  // ---- construction de la matrice (allocations hors boucle temps réel)
  std::vector<Cfg> cfgs;
  for (double sr : {44100.0, 48000.0, 88200.0, 96000.0})
    for (int K : {1, 2, 4})
      for (TimeRef ref : {TimeRef::T1, TimeRef::T2})
        for (Frac fr : {Frac::Lagrange3, Frac::Thiran1})
          for (Mode m : kModes)
            for (int ti : kTopos)
              for (int block : {32, 64, 128, 256, 512}) {
                if (ref == TimeRef::T1 && fr != Frac::Lagrange3) continue;
                if (fr == Frac::Thiran1) {  // variante fractionnaire : bloc 128, seulement si le retard T2 est fractionnaire
                  if (block != 128) continue;
                  bool frac = false;
                  try { frac = !makeDelay(TimeRef::T2, Frac::Thiran1, K, sr).integer; } catch (const T2NotRepresentable&) {}
                  if (!frac) continue;
                }
                Cfg c;
                c.sr = sr; c.K = K; c.ref = ref; c.frac = fr; c.mode = m; c.topo = ti; c.block = block;
                RouteTable t = RouteTable::build(topos[size_t(ti)].srcDst, depthLevels(m)[1].v);
                const Compiled comp = t.compile();
                c.nRoutes = comp.nRoutes; c.nDelayed = comp.nDelayed;
                EngineParams p = makeParams(sr, K, ref, m, 220.0, fr);
                c.pr = std::make_unique<Processor>(p, block);
                c.latency = c.pr->decimator().latencyHost();
                try {
                  const DelaySpec d = makeDelay(ref, fr, K, sr);
                  c.D = d.D; c.integer = d.integer;
                } catch (const T2NotRepresentable&) { c.D = sr * K / 48000.0; c.integer = false; }
                try {
                  c.pr->engine().apply(comp);  // le moteur doit refuser T2 non représentable
                } catch (const T2NotRepresentable& e) {
                  c.rejected = true; c.why = e.what(); c.pr.reset();
                }
                if (!c.rejected) {
                  for (auto& r : t.routes()) c.pr->engine().setDepthNow(slotOf(r.dst, r.src), r.depth);
                  c.out.assign(size_t(block), 0.0);
                  const int nb = int(secs * sr / block + 0.5);
                  c.times.reserve(size_t(nb) * size_t(reps));
                  c.repMeans.reserve(size_t(reps));
                }
                cfgs.push_back(std::move(c));
              }
  size_t nValid = 0;
  for (auto& c : cfgs) nValid += !c.rejected;
  std::fprintf(stderr, "configurations : %zu (%zu mesurées, %zu rejetées)\n", cfgs.size(), nValid, cfgs.size() - nValid);

  // ---- chauffe globale (montée en fréquence CPU), puis chauffe par configuration
  {
    Cfg* heavy = nullptr;
    for (auto& c : cfgs) if (!c.rejected && c.sr == 96000 && c.K == 4 && c.topo == 4 && c.block == 128) { heavy = &c; break; }
    const auto t0 = Clock::now();
    while (std::chrono::duration<double>(Clock::now() - t0).count() < warm) runBlocks(*heavy, 64, false);
    heavy->nan = heavy->inf = 0; heavy->peak = 0;
  }
  for (auto& c : cfgs) if (!c.rejected) { runBlocks(c, std::max(1, int(0.25 * c.sr / c.block)), false); }

  // ---- répétitions entrelacées, ordre aléatoire différent à chaque répétition
  std::vector<size_t> order;
  for (size_t i = 0; i < cfgs.size(); ++i) if (!cfgs[i].rejected) order.push_back(i);
  std::mt19937_64 rng(0x5D51BE7CULL);
  for (int r = 0; r < reps; ++r) {
    std::shuffle(order.begin(), order.end(), rng);
    for (size_t i : order) {
      Cfg& c = cfgs[i];
      const int nb = int(secs * c.sr / c.block + 0.5);
      const size_t before = c.times.size();
      g_allocs = 0; g_count = true;
      runBlocks(c, nb, true);
      g_count = false;
      c.allocs += g_allocs.load();
      double s = 0; for (size_t k = before; k < c.times.size(); ++k) s += c.times[k];
      c.repMeans.push_back(s / double(c.times.size() - before));
    }
    std::fprintf(stderr, "répétition %d/%d terminée\n", r + 1, reps);
  }

  // ---- écriture
  const std::string pm = dir + "/bench_matrix.csv", pr = dir + "/bench_reps.csv", pi = dir + "/bench_isolated.csv";
  FILE* o = std::fopen(pm.c_str(), "wb");
  FILE* orep = std::fopen(pr.c_str(), "wb");
  if (!o || !orep) { std::perror("ouverture CSV"); return 2; }
  std::fprintf(o, "sr,K,ref,frac,mode,topo,topo_class,nroutes,ndelayed,block,status,D_eng,delay_us,latency_host,latency_ms,"
                  "reps,blocks,budget_ns,mean_ns,p50_ns,p95_ns,p99_ns,max_ns,rep_mean_min_ns,rep_mean_max_ns,"
                  "load_mean_pct,load_p99_pct,load_max_pct,ns_per_host_sample,rt_allocs,nan,inf,peak\n");
  std::fprintf(orep, "sr,K,ref,frac,mode,topo,block,rep,mean_ns\n");
  for (auto& c : cfgs) {
    const char* frn = c.ref == TimeRef::T1 || c.integer ? "integer" : name(c.frac);
    const double budget = c.block / c.sr * 1e9;
    std::fprintf(o, "%.0f,%d,%s,%s,%s,%s,%s,%d,%d,%d,", c.sr, c.K, name(c.ref), frn, name(c.mode), topos[size_t(c.topo)].name,
                 topoClass(c.topo), c.nRoutes, c.nDelayed, c.block);
    if (c.rejected) {
      std::fprintf(o, "rejected,%.6f,%.3f,%d,%.4f,0,0,%.0f,,,,,,,,,,,,,0,0,0,\n", c.D, c.D / (c.sr * c.K) * 1e6, c.latency,
                   1000.0 * c.latency / c.sr, budget);
      continue;
    }
    double mean = 0; for (double v : c.times) mean += v; mean /= double(c.times.size());
    const double p50 = pct(c.times, 0.5), p95 = pct(c.times, 0.95), p99 = pct(c.times, 0.99);
    const double mx = *std::max_element(c.times.begin(), c.times.end());
    const auto mm = std::minmax_element(c.repMeans.begin(), c.repMeans.end());
    std::fprintf(o, "ok,%.6f,%.3f,%d,%.4f,%d,%zu,%.0f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.4f,%.4f,%.4f,%.3f,%lld,%lld,%lld,%.4f\n",
                 c.D, c.D / (c.sr * c.K) * 1e6, c.latency, 1000.0 * c.latency / c.sr, reps, c.times.size(), budget, mean, p50, p95,
                 p99, mx, *mm.first, *mm.second, 100 * mean / budget, 100 * p99 / budget, 100 * mx / budget, mean / c.block,
                 c.allocs, c.nan, c.inf, c.peak);
    for (size_t r = 0; r < c.repMeans.size(); ++r)
      std::fprintf(orep, "%.0f,%d,%s,%s,%s,%s,%d,%zu,%.1f\n", c.sr, c.K, name(c.ref), frn, name(c.mode), topos[size_t(c.topo)].name,
                   c.block, r, c.repMeans[r]);
  }
  std::fclose(o); std::fclose(orep);

  // ---- coûts isolés (mêmes mesures que s01_bench)
  FILE* oi = std::fopen(pi.c_str(), "wb");
  std::fprintf(oi, "kind,name,K,ns_per_unit\n");
  {
    double ring[kNodes][kRing];
    for (int i = 0; i < kNodes; ++i) for (int k = 0; k < kRing; ++k) ring[i][k] = std::sin(0.1 * (k + 7 * i));
    struct D { const char* n; DelaySpec d; };
    std::vector<D> ds = {{"integer", makeDelay(TimeRef::T1, Frac::Lagrange3, 2, 44100)},
                         {"lagrange3", makeDelay(TimeRef::T2, Frac::Lagrange3, 2, 44100)},
                         {"linear", makeDelay(TimeRef::T2, Frac::Linear, 2, 44100)},
                         {"thiran1", makeDelay(TimeRef::T2, Frac::Thiran1, 2, 44100)}};
    for (auto& d : ds) {
      double st[kNodes] = {0}, acc = 0;
      const long long N = 20000000;
      const auto a = Clock::now();
      for (long long n = 0; n < N; ++n) {
        for (int i = 0; i < kNodes; ++i) acc += readDelay(d.d, ring[i], n, st[i]);
        ring[n % kNodes][n & (kRing - 1)] = acc * 1e-9;
      }
      const auto b = Clock::now();
      volatile double sink = acc; (void)sink;
      std::fprintf(oi, "delay_reader,%s,2,%.3f\n", d.n, std::chrono::duration<double, std::nano>(b - a).count() / double(N * kNodes));
    }
  }
  for (int K : {2, 4}) {
    Decimator dec(K);
    const int nb = 128;
    std::vector<double> in(size_t(nb) * K), out(nb);
    for (size_t i = 0; i < in.size(); ++i) in[i] = std::sin(0.01 * double(i));
    const int rr = 200000;
    const auto a = Clock::now();
    for (int r = 0; r < rr; ++r) { dec.process(in.data(), out.data(), nb); in[0] = out[5] * 1e-9; }
    const auto b = Clock::now();
    std::fprintf(oi, "decimator,latency%d_macs%d,%d,%.3f\n", dec.latencyHost(), dec.macsPerHostSample(), K,
                 std::chrono::duration<double, std::nano>(b - a).count() / (double(rr) * nb));
  }
  std::fclose(oi);
  std::fprintf(stderr, "écrit : %s, %s, %s\n", pm.c_str(), pr.c_str(), pi.c_str());
  return 0;
}
