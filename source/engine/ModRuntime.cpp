#include "ModRuntime.h"

#include <algorithm>
#include <vector>

namespace sbd {

std::unique_ptr<CompiledMod> compileModMatrix(const ModMatrixState& m) {
  auto c = std::make_unique<CompiledMod>();
  const auto status = modRouteStatus(m);
  for (size_t i = 0; i < status.size() && i < c->status.size(); ++i) c->status[i] = static_cast<std::uint8_t>(status[i]);
  // ordre topologique des macros (le graphe macro est acyclique : validé)
  std::array<std::array<bool, 4>, 4> adj{};
  auto macroDest = [](ParamId p) { return idx(p) - idx(ParamId::Macro1); };
  auto macroSrc = [](ModSource s) { return static_cast<int>(s) - static_cast<int>(ModSource::Macro1); };
  for (size_t i = 0; i < m.routes.size(); ++i) {
    if (status[i] != ModInactive::Active) continue;
    const auto& r = m.routes[i];
    const int d = macroDest(r.dest);
    if (d < 0 || d >= 4) continue;
    for (int s : {macroSrc(r.source), macroSrc(r.via)})
      if (s >= 0 && s < 4) adj[static_cast<size_t>(s)][static_cast<size_t>(d)] = true;
  }
  std::vector<int> topo;
  std::array<bool, 4> done{};
  for (int k = 0; k < 4; ++k)
    for (int u = 0; u < 4; ++u) {
      if (done[static_cast<size_t>(u)]) continue;
      bool ready = true;
      for (int v = 0; v < 4; ++v)
        if (!done[static_cast<size_t>(v)] && adj[static_cast<size_t>(v)][static_cast<size_t>(u)]) ready = false;
      if (ready) {
        done[static_cast<size_t>(u)] = true;
        topo.push_back(u);
        break;
      }
    }
  auto emit = [&](const ModRoute& r) {
    auto& o = c->routes[static_cast<size_t>(c->numRoutes++)];
    o.id = r.id;
    o.src = static_cast<std::uint8_t>(r.source);
    o.via = static_cast<std::uint8_t>(r.via);
    o.curve = static_cast<std::uint8_t>(r.curve);
    o.numPoints = r.numPoints;
    o.centered = r.polarity == ModPolarity::Centered;
    o.invert = r.invert;
    o.audio = r.rate == ModRate::Audio;
    o.srcBipolar = modSourceBipolar(r.source);
    o.viaBipolar = modSourceBipolar(r.via);
    o.dest = static_cast<std::int16_t>(idx(r.dest));
    o.depth = r.depth;
    o.offset = r.offset;
    o.viaAmount = r.viaAmount;
    o.smoothMs = r.smoothMs;
    o.scale = paramSpec(r.dest).modScale;
    o.px = r.px;
    o.py = r.py;
    int slot = -1;
    for (int k = 0; k < c->numDests; ++k)
      if (c->dests[static_cast<size_t>(k)] == o.dest) slot = k;
    if (slot < 0) {
      slot = c->numDests++;
      c->dests[static_cast<size_t>(slot)] = o.dest;
    }
    o.destSlot = static_cast<std::uint8_t>(slot);
  };
  for (int mk : topo)
    for (size_t i = 0; i < m.routes.size(); ++i)
      if (status[i] == ModInactive::Active && macroDest(m.routes[i].dest) == mk) emit(m.routes[i]);
  c->numMacroRoutes = c->numRoutes;
  c->numMacroDests = c->numDests;
  for (size_t i = 0; i < m.routes.size(); ++i) {
    const int d = macroDest(m.routes[i].dest);
    if (status[i] == ModInactive::Active && !(d >= 0 && d < 4)) emit(m.routes[i]);
  }
  return c;
}

}  // namespace sbd
