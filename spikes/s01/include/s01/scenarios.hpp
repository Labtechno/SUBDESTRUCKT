// SUBDESTRUCKT — Spike S0.1 — topologies, profondeurs et fabrique de rendus (§7 de la mission).
#pragma once
#include <string>
#include <utility>
#include <vector>
#include "s01/engine.hpp"

namespace s01 {

struct Topology {
  int id;
  const char* name;
  std::vector<std::pair<int, int>> srcDst;  // ordre de création (sticky : il fixe la classe)
};

// Nœuds : 0 = WT, 1..4 = OP1..OP4.
inline std::vector<Topology> topologies() {
  std::vector<Topology> t;
  t.push_back({1, "cascade_ff", {{1, 2}, {2, 3}, {3, 4}}});
  t.push_back({2, "self_feedback", {{1, 1}, {1, 2}}});
  t.push_back({3, "cycle2", {{1, 2}, {2, 1}}});
  t.push_back({4, "cycle3", {{1, 2}, {2, 3}, {3, 1}}});
  {
    std::vector<std::pair<int, int>> v;
    for (int d = 1; d <= 4; ++d) for (int s = 1; s <= 4; ++s) v.push_back({s, d});
    t.push_back({5, "dense4x4", v});
  }
  t.push_back({6, "wt_op1", {{0, 1}, {1, 0}}});
  t.push_back({7, "wt_multi_op", {{0, 1}, {0, 2}, {0, 3}, {1, 0}, {2, 0}, {3, 0}}});
  // deux cycles indépendants {WT,OP1} et {OP2,OP3,OP4} + auto-feedback OP3 + ponts A->B seulement
  t.push_back({8, "wtfm_dense_2cycles", {{0, 1}, {1, 0}, {2, 3}, {3, 4}, {4, 2}, {1, 2}, {0, 4}, {3, 3}}});
  return t;
}

struct DepthLevel { const char* name; double v; };
// Profondeur par route, dans l'unité du mode. "max" = borne cumulée candidate du baseline.
inline std::vector<DepthLevel> depthLevels(Mode m) {
  switch (m) {
    case Mode::PM: return {{"low", 0.1}, {"nominal", 1.0}, {"strong", 4.0}, {"max", kPmClampRad}};
    case Mode::LinFM:
    case Mode::TZFM: return {{"low", 22.0}, {"nominal", 220.0}, {"strong", 1100.0}, {"max", kFmClampHz}};
    case Mode::ExpFM: default: return {{"low", 0.05}, {"nominal", 0.5}, {"strong", 1.5}, {"max", kExpClampOct}};
  }
}

inline const Mode kModes[] = {Mode::PM, Mode::LinFM, Mode::TZFM, Mode::ExpFM};
inline const double kSRs[] = {44100, 48000, 88200, 96000};
inline const int kBlocks[] = {16, 32, 64, 128, 256, 512, 1024};

// Ratios rationnels : toutes les partielles tombent sur une grille de f0/2.
inline std::array<double, kNodes> ratiosRational() { return {1.0, 1.0, 2.0, 3.0, 1.5}; }

inline EngineParams makeParams(double fsHost, int K, TimeRef ref, Mode mode, double f0 = 220.0,
                               Frac frac = Frac::Lagrange3, Shape wtShape = Shape::Saw) {
  EngineParams p;
  p.fsHost = fsHost; p.K = K; p.ref = ref; p.frac = frac;
  auto r = ratiosRational();
  for (int i = 0; i < kNodes; ++i) { p.freq[i] = f0 * r[i]; p.mode[i] = mode; }
  p.shape = {wtShape, Shape::Sine, Shape::Sine, Shape::Sine, Shape::Sine};
  return p;
}

inline RenderSpec makeSpec(const RouteTable& t, const EngineParams& p, long long hostSamples) {
  RenderSpec s;
  s.params = p;
  s.initial = t.compile();
  for (const auto& r : t.routes()) s.initialDepths.push_back({slotOf(r.dst, r.src), r.active ? r.depth : 0.0, 0});
  s.hostSamples = hostSamples;
  return s;
}

// K minimal imposé par la politique validée : T2 avec arête Delayed exige fsE >= 48 kHz (K>=2 à 44.1 kHz).
inline int minK(TimeRef ref, double fsHost, int nDelayed) {
  if (ref == TimeRef::T1 || nDelayed == 0) return 1;
  int K = 1;
  while (fsHost * K * kT2Seconds < 1.0 - 1e-12) K *= 2;
  return K;
}

}  // namespace s01
