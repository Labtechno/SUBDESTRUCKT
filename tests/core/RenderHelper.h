// SUBDESTRUCKT — rendu hors ligne du moteur pour les tests natifs.
#pragma once
#include <algorithm>
#include <cstring>
#include <functional>
#include <memory>
#include <vector>

#include "TestKit.h"
#include "core/Topology.h"
#include "engine/Engine.h"

namespace th {

inline std::shared_ptr<const sbd::SawWavetable> table() {
  static std::shared_ptr<const sbd::SawWavetable> t = std::make_shared<const sbd::SawWavetable>();
  return t;
}

struct Scenario {
  double fs = 48000.0;
  int block = 256;
  std::vector<int> blockSizes;  // si non vide : tailles cycliques (blocs variables)
  long long samples = 48000;
  sbd::ParamValues params = sbd::ParamValues::defaults();
  std::vector<sbd::RouteTable::Op> topology;  // appliquées à une table vide
  std::vector<sbd::MidiEvent> events;          // offsets ABSOLUS
  // crochet « thread de contrôle » avant chaque bloc : changement de paramètres, publication de topologie
  std::function<void(sbd::Engine&, sbd::ParamValues&, sbd::RouteTable&, long long pos)> beforeBlock;
  bool poison = false;
  bool reverseOrder = false;
};

struct Render {
  std::vector<float> L, R;
  std::vector<double> tap;  // sortie moteur avant garde DC
  sbd::EngineSetup setup;
  sbd::RouteTable finalTable;
  std::uint32_t nonFinite = 0, outputClamp = 0, pmClamp = 0, panics = 0;
  long long allocations = 0;
  long long overlap = 0;
};

inline Render render(const Scenario& s) {
  Render r;
  sbd::Engine eng(table());
  sbd::ParamValues p = s.params;
  eng.setParameters(p);
  int maxBlock = s.block;
  for (int b : s.blockSizes) maxBlock = std::max(maxBlock, b);
  eng.prepare(s.fs, std::min(maxBlock, 512));  // blocs > 512 : chemin de découpage interne exercé
  sbd::RouteTable table;
  if (!s.topology.empty()) {
    const auto res = table.apply(s.topology);
    TK_CHECK(res.ok);
  }
  eng.setTopologyImmediate(table.compile());
  eng.testSetPoison(s.poison);
  eng.testSetReverseOrder(s.reverseOrder);
  r.setup = eng.setup();
  r.L.assign(static_cast<size_t>(s.samples), 0.0f);
  r.R.assign(static_cast<size_t>(s.samples), 0.0f);
  r.tap.assign(static_cast<size_t>(s.samples), 0.0);
  std::vector<sbd::MidiEvent> ev;
  ev.reserve(64);
  long long pos = 0;
  size_t bi = 0;
  while (pos < s.samples) {
    int n = s.blockSizes.empty() ? s.block : s.blockSizes[bi++ % s.blockSizes.size()];
    n = static_cast<int>(std::min<long long>(n, s.samples - pos));
    if (s.beforeBlock) s.beforeBlock(eng, p, table, pos);
    ev.clear();
    for (const auto& e : s.events)
      if (e.offset >= pos && e.offset < pos + n) {
        sbd::MidiEvent x = e;
        x.offset = static_cast<int>(e.offset - pos);
        ev.push_back(x);
      }
    eng.setParameters(p);
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    eng.process(&r.L[static_cast<size_t>(pos)], &r.R[static_cast<size_t>(pos)], n, ev.data(), static_cast<int>(ev.size()),
                &r.tap[static_cast<size_t>(pos)]);
    tk::countingAllocations().store(false);
    r.allocations += tk::allocationCount().load();
    eng.exchange().collectGarbage();  // rôle du thread de contrôle
    pos += n;
  }
  const auto& t = eng.telemetry();
  r.nonFinite = t.nonFiniteResets.load();
  r.outputClamp = t.outputClampHits.load();
  r.pmClamp = t.pmClampHits.load();
  r.panics = t.panics.load();
  r.overlap = eng.testOverlapSamples();
  r.finalTable = table;
  return r;
}

// Publie (rôle du thread de contrôle) la compilation de la table après une transaction.
inline bool publish(sbd::Engine& eng, sbd::RouteTable& table, const std::vector<sbd::RouteTable::Op>& ops) {
  const auto res = table.apply(ops);
  if (!res.ok) return false;
  auto c = std::make_unique<sbd::CompiledTopology>(table.compile());
  eng.exchange().publish(std::move(c));
  return true;
}

inline bool sameFloats(const std::vector<float>& a, const std::vector<float>& b) {
  return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](float x, float y) {
           return std::memcmp(&x, &y, sizeof x) == 0;
         });
}
inline bool sameDoubles(const double* a, const double* b, size_t n) {
  return std::memcmp(a, b, n * sizeof(double)) == 0;
}

}  // namespace th
