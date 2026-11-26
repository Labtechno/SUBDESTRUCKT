// SUBDESTRUCKT — aides de rendu et de mesure pour les tests ARCH-G4 du cœur.
#pragma once
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include "RenderHelper.h"
#include "TestKit.h"
#include "core/StateCodec.h"
#include "engine/Engine.h"

namespace g4 {
using namespace sbd;

struct G4Scenario {
  double fs = 48000.0;
  int block = 256;
  std::vector<int> blockSizes;
  long long samples = 48000;
  PluginState state = defaultState();
  std::vector<MidiEvent> events;  // offsets absolus
  std::function<void(Engine&, ParamValues&, long long)> beforeBlock;
  TransportInfo transport;
  Quality quality = Quality::Normal;
};
struct G4Render {
  std::vector<float> L, R;
  long long allocations = 0;
  std::uint32_t nonFinite = 0, clamp = 0;
  EngineSetup setup;
};

inline long long& allocTotal() {
  static long long n = 0;
  return n;
}

inline G4Render renderG4(const G4Scenario& s) {
  G4Render r;
  auto bank = FactoryBanks::get(s.state.bank);
  Engine eng(bank ? bank : FactoryBanks::get("factory:saw"));
  ParamValues p = s.state.params;
  eng.setParameters(p);
  int maxBlock = s.block;
  for (int b : s.blockSizes) maxBlock = std::max(maxBlock, b);
  eng.setSeed(s.state.seed);
  eng.setLfoCurves(s.state.lfoCurves);
  eng.prepare(s.fs, std::min(maxBlock, 512), s.quality);
  RouteTable t;
  std::string err;
  TK_CHECK(t.assign(s.state.routes, s.state.nextRouteId, err));
  eng.setTopologyImmediate(t.compile());
  auto cm = compileModMatrix(s.state.mod);
  eng.setModMatrixImmediate(*cm);
  r.setup = eng.setup();
  r.L.assign(static_cast<size_t>(s.samples), 0.0f);
  r.R.assign(static_cast<size_t>(s.samples), 0.0f);
  std::vector<MidiEvent> ev;
  ev.reserve(256);
  long long pos = 0;
  size_t bi = 0;
  TransportInfo tr = s.transport;
  while (pos < s.samples) {
    int n = s.blockSizes.empty() ? s.block : s.blockSizes[bi++ % s.blockSizes.size()];
    n = static_cast<int>(std::min<long long>(n, s.samples - pos));
    if (s.beforeBlock) s.beforeBlock(eng, p, pos);
    ev.clear();
    for (const auto& e : s.events)
      if (e.offset >= pos && e.offset < pos + n) {
        MidiEvent x = e;
        x.offset = static_cast<int>(e.offset - pos);
        ev.push_back(x);
      }
    eng.setParameters(p);
    eng.setTransport(tr);
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    eng.process(&r.L[static_cast<size_t>(pos)], &r.R[static_cast<size_t>(pos)], n, ev.data(), static_cast<int>(ev.size()));
    tk::countingAllocations().store(false);
    r.allocations += tk::allocationCount().load();
    eng.exchange().collectGarbage();
    eng.modExchange().collectGarbage();
    eng.bankExchange().collectGarbage();
    if (tr.valid) tr.ppq += n * tr.bpm / 60.0 / s.fs;
    pos += n;
  }
  r.nonFinite = eng.telemetry().nonFiniteResets.load();
  r.clamp = eng.telemetry().outputClampHits.load();
  allocTotal() += r.allocations;
  return r;
}

inline double peak(const std::vector<float>& x, size_t a = 0, size_t b = 0) {
  if (b == 0 || b > x.size()) b = x.size();
  double p = 0.0;
  for (size_t i = a; i < b; ++i) p = std::max(p, static_cast<double>(std::fabs(x[i])));
  return p;
}
inline bool allFinite(const std::vector<float>& x) {
  for (float v : x)
    if (!std::isfinite(v)) return false;
  return true;
}
inline double nmseDb(const std::vector<float>& a, const std::vector<float>& ref) {
  double num = 0.0, den = 0.0;
  for (size_t i = 0; i < a.size(); ++i) {
    const double d = static_cast<double>(a[i]) - ref[i];
    num += d * d;
    den += static_cast<double>(ref[i]) * ref[i];
  }
  if (num == 0.0) return -400.0;
  return 10.0 * std::log10(num / std::max(den, 1e-300));
}
inline double rms(const std::vector<float>& x, size_t a, size_t b) {
  double s = 0.0;
  for (size_t i = a; i < b; ++i) s += static_cast<double>(x[i]) * x[i];
  return std::sqrt(s / std::max<size_t>(1, b - a));
}
// État « nu » : aucune route, profondeurs et niveaux à 0, Amp instantanée, gain de sortie 0 dB.
inline PluginState bareState() {
  PluginState s = defaultState();
  s.routes.clear();
  for (int k = 0; k < kMaxRoutes; ++k) s.params[routeSlotInfo(k).depthParam] = 0.0f;
  s.params[ParamId::WtLevel] = 0.0f;
  s.params[ParamId::Op1Level] = 0.0f;
  s.params[ParamId::AmpAttack] = 0.001f;
  s.params[ParamId::AmpSustain] = 1.0f;
  s.params[ParamId::OutGain] = 0.0f;
  return s;
}
inline void setRoutes(PluginState& s, const std::vector<RouteTable::Op>& ops) {
  RouteTable t;
  t.apply(ops);
  s.routes = t.routes();
  s.nextRouteId = t.nextRouteId();
}
// Fréquence par passages par zéro montants (interpolés), sur [a,b).
inline double zcFreq(const std::vector<float>& x, double fs, size_t a, size_t b) {
  double first = -1.0, last = -1.0;
  int n = 0;
  for (size_t i = a + 1; i < b; ++i)
    if (x[i - 1] < 0.0f && x[i] >= 0.0f) {
      const double t = (i - 1) + x[i - 1] / static_cast<double>(x[i - 1] - x[i]);
      if (first < 0) first = t;
      last = t;
      ++n;
    }
  return n > 1 ? (n - 1) * fs / (last - first) : 0.0;
}
inline std::vector<double> toD(const std::vector<float>& x, size_t a, size_t n) {
  std::vector<double> d(n);
  for (size_t i = 0; i < n; ++i) d[i] = x[a + i];
  return d;
}

}  // namespace g4
