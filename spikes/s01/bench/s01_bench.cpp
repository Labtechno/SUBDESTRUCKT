// SUBDESTRUCKT â€” Spike S0.1 â€” benchmark CPU (cible : Apple M1, Â§10). Usage : s01_bench <out.csv> [secondes]
// Mesure par bloc hÃ´te de 128 Ã©chantillons (steady_clock), aprÃ¨s prÃ©chauffage :
//   moyenne, p50, p95, p99, max ; ns par Ã©chantillon hÃ´te ; charge (% du temps rÃ©el) ;
//   nombre d'allocations pendant la boucle temps rÃ©el (operator new global instrumentÃ©) â€” doit Ãªtre 0.
// CoÃ»ts isolÃ©s : lecteur de retard (entier / Lagrange3 / Linear / Thiran1) et dÃ©cimateur (K=2, K=4).
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <new>
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

struct Res { double mean, p50, p95, p99, mx; long long allocs; };

static Res timeBlocks(Processor& pr, int block, int nBlocks) {
  std::vector<double> out(size_t(block), 0.0);
  std::vector<double> t(static_cast<size_t>(nBlocks), 0.0);
  for (int i = 0; i < 200; ++i) pr.processBlock(out.data(), block);  // prÃ©chauffage
  g_allocs = 0; g_count = true;
  for (int i = 0; i < nBlocks; ++i) {
    auto a = Clock::now();
    pr.processBlock(out.data(), block);
    auto b = Clock::now();
    t[size_t(i)] = std::chrono::duration<double, std::nano>(b - a).count();
  }
  g_count = false;
  volatile double sink = out[0]; (void)sink;
  std::vector<double> s = t;
  std::sort(s.begin(), s.end());
  double m = 0; for (double v : t) m += v;
  auto q = [&](double p) { return s[size_t(std::min<double>(double(s.size() - 1), p * double(s.size())))]; };
  return {m / nBlocks, q(0.5), q(0.95), q(0.99), s.back(), g_allocs.load()};
}

int main(int argc, char** argv) {
  const std::string path = argc > 1 ? argv[1] : "bench.csv";
  const double seconds = argc > 2 ? std::atof(argv[2]) : 2.0;
  FILE* o = std::fopen(path.c_str(), "wb");
  if (!o) { std::perror(path.c_str()); return 2; }
  (void)TableBank::instance();
  std::fprintf(o, "kind,sr,K,ref,frac,mode,topo,nroutes,ndelayed,block,mean_ns_block,p50_ns,p95_ns,p99_ns,max_ns,ns_per_host_sample,load_pct,rt_allocs\n");
  const auto topos = topologies();
  const int block = 128;
  // sparse = WT<->OP1 (2 routes), self_feedback ; dense = dense4x4 (16 routes), rÃ©seau WT/FM 2 cycles (8 routes)
  const int topoIdx[] = {1, 5, 4, 7};
  struct Cfg { TimeRef ref; int K; Frac fr; };
  for (double sr : {44100.0, 48000.0, 96000.0})
    for (Mode m : kModes)
      for (int ti : topoIdx) {
        RouteTable t = RouteTable::build(topos[size_t(ti)].srcDst, depthLevels(m)[1].v);
        const Compiled c = t.compile();
        std::vector<Cfg> cfgs;
        for (int K : {1, 2, 4}) {
          cfgs.push_back({TimeRef::T1, K, Frac::Lagrange3});
          if (K >= minK(TimeRef::T2, sr, c.nDelayed))
            for (Frac fr : {Frac::Lagrange3, Frac::Linear, Frac::Thiran1}) {
              if (fr != Frac::Lagrange3 && makeDelay(TimeRef::T2, fr, K, sr).integer) continue;
              cfgs.push_back({TimeRef::T2, K, fr});
            }
        }
        for (const Cfg& cf : cfgs) {
          EngineParams p = makeParams(sr, cf.K, cf.ref, m, 220.0, cf.fr);
          Processor pr(p, block);
          pr.engine().apply(c);
          for (auto& r : t.routes()) pr.engine().setDepthNow(slotOf(r.dst, r.src), r.depth);
          const int nBlocks = int(seconds * sr / block);
          Res r = timeBlocks(pr, block, nBlocks);
          const double blockNs = block / sr * 1e9;
          std::fprintf(o, "full,%.0f,%d,%s,%s,%s,%s,%d,%d,%d,%.0f,%.0f,%.0f,%.0f,%.0f,%.2f,%.3f,%lld\n", sr, cf.K, name(cf.ref), name(cf.fr),
                       name(m), topos[size_t(ti)].name, c.nRoutes, c.nDelayed, block, r.mean, r.p50, r.p95, r.p99, r.mx, r.mean / block,
                       100.0 * r.mean / blockNs, r.allocs);
        }
        std::fprintf(stderr, "bench sr=%.0f %s %s\n", sr, name(m), topos[size_t(ti)].name);
      }
  // ---- coÃ»ts isolÃ©s : lecteur de retard (5 sources, 1 lecture/source/Ã©chantillon moteur)
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
      auto a = Clock::now();
      for (long long n = 0; n < N; ++n) {
        for (int i = 0; i < kNodes; ++i) acc += readDelay(d.d, ring[i], n, st[i]);
        ring[n % kNodes][n & (kRing - 1)] = acc * 1e-9;  // dÃ©pendance : empÃªche l'Ã©limination
      }
      auto b = Clock::now();
      volatile double sink = acc; (void)sink;
      const double ns = std::chrono::duration<double, std::nano>(b - a).count() / double(N * kNodes);
      std::fprintf(o, "delay_reader,44100,2,-,%s,-,-,-,-,-,-,-,-,-,-,%.3f,-,-\n", d.n, ns);
    }
  }
  // ---- coÃ»t isolÃ© du dÃ©cimateur temps rÃ©el (par Ã©chantillon hÃ´te)
  for (int K : {2, 4}) {
    Decimator dec(K);
    const int nb = 128;
    std::vector<double> in(size_t(nb) * K), out(nb);
    for (size_t i = 0; i < in.size(); ++i) in[i] = std::sin(0.01 * double(i));
    const int reps = 200000;
    auto a = Clock::now();
    for (int r = 0; r < reps; ++r) { dec.process(in.data(), out.data(), nb); in[0] = out[5] * 1e-9; }
    auto b = Clock::now();
    const double ns = std::chrono::duration<double, std::nano>(b - a).count() / (double(reps) * nb);
    std::fprintf(o, "decimator,-,%d,-,-,-,-,-,-,128,-,-,-,-,-,%.3f,-,-\n", K, ns);
    std::fprintf(stderr, "decimator K=%d : taps final %d, first %d, MAC/ech hote %d, latence %d ech hote\n", K, dec.tapsFinal(),
                 dec.tapsFirst(), dec.macsPerHostSample(), dec.latencyHost());
  }
  std::fclose(o);
  return 0;
}
