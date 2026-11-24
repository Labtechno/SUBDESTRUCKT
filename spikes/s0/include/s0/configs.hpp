// SUBDESTRUCKT — Spike S0 — configurations de test (§5 du document de mission).
#pragma once
#include <string>
#include <vector>
#include "s0/engine.hpp"

namespace s0 {

struct Config {
  int id;
  const char* name;
  std::vector<Route> routes;  // prio = ordre de création (ordre du vecteur)
};

inline std::vector<Route> mk(std::initializer_list<std::pair<int, int>> dstSrc) {
  std::vector<Route> r;
  int p = 0;
  for (auto& d : dstSrc) r.push_back({d.first, d.second, p++});
  return r;
}

// Nœuds : 0 = WT, 1..4 = OP1..OP4. Paire = (dst, src).
inline std::vector<Config> staticConfigs() {
  std::vector<Config> c;
  c.push_back({1, "cascade_OP1_OP2_OP3_OP4", mk({{2, 1}, {3, 2}, {4, 3}})});
  c.push_back({2, "self_feedback_OP1", mk({{1, 1}, {2, 1}})});
  c.push_back({3, "cycle_OP1_OP2", mk({{2, 1}, {1, 2}})});
  c.push_back({4, "cycle_OP1_OP2_OP3", mk({{2, 1}, {3, 2}, {1, 3}})});
  {
    std::vector<std::pair<int, int>> v;
    for (int d = 1; d <= 4; ++d) for (int s = 1; s <= 4; ++s) v.push_back({d, s});
    std::vector<Route> r; int p = 0;
    for (auto& x : v) r.push_back({x.first, x.second, p++});
    c.push_back({5, "dense_4x4", r});
  }
  c.push_back({6, "WT_OP1_both_ways", mk({{1, 0}, {0, 1}})});
  c.push_back({7, "WT_OP1_both_ways_fb_OP1", mk({{1, 0}, {0, 1}, {1, 1}})});
  c.push_back({8, "WT_multi_OP_both_ways", mk({{1, 0}, {2, 0}, {3, 0}, {0, 1}, {0, 2}, {0, 3}})});
  return c;
}

// Config 9 : cascade acyclique puis ajout transactionnel de OP4->OP1 (cycle à 4).
inline std::vector<Route> cfg9Before() { return mk({{2, 1}, {3, 2}, {4, 3}}); }
inline std::vector<Route> cfg9After() { return mk({{2, 1}, {3, 2}, {4, 3}, {1, 4}}); }

struct RatioSet { const char* name; std::array<double, kNodes> r; };
inline std::vector<RatioSet> ratioSets() {
  return {{"rational", {1.0, 1.0, 2.0, 3.0, 1.5}},
          {"irrational", {1.0, 1.41421356237, 1.61803398875, 1.57079632679, 1.35914091423}}};
}

inline std::vector<double> depthLevels() { return {0.0, 0.01, 1.0, 8.0}; }  // rad (par route)

inline RenderSpec makeSpec(const std::vector<Route>& routes, Strategy st, TimeRef ref, double fsHost, int K,
                           double f0, const RatioSet& rs, double depth, std::int64_t hostSamples,
                           bool poison = false) {
  RenderSpec s;
  s.params.fsHost = fsHost;
  s.params.K = K;
  s.params.ref = ref;
  s.params.poison = poison;
  for (int i = 0; i < kNodes; ++i) s.params.freq[i] = f0 * rs.r[i];
  s.initial = compile(routes, st);
  for (const Route& r : routes) s.initialDepths.push_back({slotOf(r.dst, r.src), depth, 0});
  s.hostSamples = hostSamples;
  return s;
}

}  // namespace s0
