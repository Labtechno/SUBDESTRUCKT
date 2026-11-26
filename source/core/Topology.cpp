#include "Topology.h"

#include <algorithm>

namespace sbd {

namespace {
using K = RouteKind;
using I = ParamId;
// Slots 0..7 : tranche 0.1.0-b5 (ordre inchangé). Puis extension ARCH-G4.
constexpr RouteSlotInfo kSlots[kMaxRoutes] = {
    {kNodeOP2, kNodeOP1, I::DepthOp2Op1, K::Internal},
    {kNodeOP1, kNodeOP2, I::DepthOp1Op2, K::Internal},
    {kNodeOP1, kNodeOP1, I::DepthOp1Op1, K::Internal},
    {kNodeOP2, kNodeOP2, I::DepthOp2Op2, K::Internal},
    {kNodeWT, kNodeOP1, I::DepthWtOp1, K::CrossForward},
    {kNodeWT, kNodeOP2, I::DepthWtOp2, K::CrossForward},
    {kNodeOP1, kPortWtPhase, I::DepthOp1Wt, K::CrossReverse},
    {kNodeOP2, kPortWtPhase, I::DepthOp2Wt, K::CrossReverse},
    // FM interne restante (12)
    {kNodeOP3, kNodeOP1, I::DepthOp3Op1, K::Internal},
    {kNodeOP4, kNodeOP1, I::DepthOp4Op1, K::Internal},
    {kNodeOP3, kNodeOP2, I::DepthOp3Op2, K::Internal},
    {kNodeOP4, kNodeOP2, I::DepthOp4Op2, K::Internal},
    {kNodeOP1, kNodeOP3, I::DepthOp1Op3, K::Internal},
    {kNodeOP2, kNodeOP3, I::DepthOp2Op3, K::Internal},
    {kNodeOP3, kNodeOP3, I::DepthOp3Op3, K::Internal},
    {kNodeOP4, kNodeOP3, I::DepthOp4Op3, K::Internal},
    {kNodeOP1, kNodeOP4, I::DepthOp1Op4, K::Internal},
    {kNodeOP2, kNodeOP4, I::DepthOp2Op4, K::Internal},
    {kNodeOP3, kNodeOP4, I::DepthOp3Op4, K::Internal},
    {kNodeOP4, kNodeOP4, I::DepthOp4Op4, K::Internal},
    // CROSS avant restante (2)
    {kNodeWT, kNodeOP3, I::DepthWtOp3, K::CrossForward},
    {kNodeWT, kNodeOP4, I::DepthWtOp4, K::CrossForward},
    // CROSS arrière phase restante (3)
    {kNodeOP3, kPortWtPhase, I::DepthOp3Wt, K::CrossReverse},
    {kNodeOP4, kPortWtPhase, I::DepthOp4Wt, K::CrossReverse},
    {kNodeBUS, kPortWtPhase, I::DepthBusWt, K::CrossReverse},
    // CROSS arrière vers FREQ/X/Y/Z/LEVEL/CHAR (30)
#define SBD_XS(S, N)                                                         \
  {N, kPortWtFreq, I::Depth##S##WtFreq, K::CrossReverse},                    \
      {N, kPortWtX, I::Depth##S##WtX, K::CrossReverse},                      \
      {N, kPortWtY, I::Depth##S##WtY, K::CrossReverse},                      \
      {N, kPortWtZ, I::Depth##S##WtZ, K::CrossReverse},                      \
      {N, kPortWtLevel, I::Depth##S##WtLevel, K::CrossReverse},              \
  {                                                                          \
    N, kPortWtChar, I::Depth##S##WtChar, K::CrossReverse                     \
  }
    SBD_XS(Op1, kNodeOP1), SBD_XS(Op2, kNodeOP2), SBD_XS(Op3, kNodeOP3), SBD_XS(Op4, kNodeOP4), SBD_XS(Bus, kNodeBUS),
#undef SBD_XS
};

// Arêtes implicites OPk>BUS (Immediate, toujours présentes) : parcours d'accessibilité et ordre topologique.
template <typename Fn>
void forEachImmediateOut(const std::vector<Route>& r, int u, Fn&& fn) {
  for (const auto& e : r)
    if (e.cls == CausalClass::Immediate && e.src == u) fn(portNode(e.dst));
  if (u >= kNodeOP1 && u <= kNodeOP4) fn(kNodeBUS);
}
}  // namespace

const char* nodeName(int node) noexcept {
  switch (node) {
    case kNodeWT: return "WT";
    case kNodeOP1: return "OP1";
    case kNodeOP2: return "OP2";
    case kNodeOP3: return "OP3";
    case kNodeOP4: return "OP4";
    case kNodeBUS: return "BUS";
    default: return "?";
  }
}

int nodeFromName(const std::string& name) noexcept {
  for (int n = 0; n < kNumNodes; ++n)
    if (name == nodeName(n)) return n;
  return -1;
}

const char* portName(int port) noexcept {
  switch (port) {
    case kPortWtPhase: return "WT";  // nom v1 conservé (phase)
    case kNodeOP1: return "OP1";
    case kNodeOP2: return "OP2";
    case kNodeOP3: return "OP3";
    case kNodeOP4: return "OP4";
    case kPortWtFreq: return "WT.FREQ";
    case kPortWtX: return "WT.X";
    case kPortWtY: return "WT.Y";
    case kPortWtZ: return "WT.Z";
    case kPortWtLevel: return "WT.LEVEL";
    case kPortWtChar: return "WT.CHAR";
    default: return "?";
  }
}

int portFromName(const std::string& name) noexcept {
  for (int p = 0; p < kNumPorts; ++p)
    if (name == portName(p)) return p;
  return -1;
}

int routeSlot(int src, int dst) noexcept {
  for (int s = 0; s < kMaxRoutes; ++s)
    if (kSlots[s].src == src && kSlots[s].dst == dst) return s;
  return -1;
}

const RouteSlotInfo& routeSlotInfo(int slot) noexcept { return kSlots[slot]; }

const Route* RouteTable::findPair(int src, int dst) const noexcept {
  for (const auto& r : routes_)
    if (r.src == src && r.dst == dst) return &r;
  return nullptr;
}

const Route* RouteTable::findId(std::uint32_t id) const noexcept {
  for (const auto& r : routes_)
    if (r.id == id) return &r;
  return nullptr;
}

bool RouteTable::reachesImmediate(const std::vector<Route>& r, int from, int to) noexcept {
  std::array<bool, kNumNodes> seen{};
  std::array<int, kNumNodes> stack{};
  int sp = 0;
  stack[sp++] = from;
  seen[static_cast<size_t>(from)] = true;
  while (sp > 0) {
    const int u = stack[--sp];
    if (u == to) return true;
    forEachImmediateOut(r, u, [&](int v) {
      if (!seen[static_cast<size_t>(v)]) {
        seen[static_cast<size_t>(v)] = true;
        stack[sp++] = v;
      }
    });
  }
  return false;
}

bool RouteTable::acyclic(const std::vector<Route>& r) noexcept {
  std::array<int, kNumNodes> indeg{};
  for (const auto& e : r)
    if (e.cls == CausalClass::Immediate) {
      if (e.src == portNode(e.dst)) return false;
    }
  for (int u = 0; u < kNumNodes; ++u) forEachImmediateOut(r, u, [&](int v) { indeg[static_cast<size_t>(v)]++; });
  std::array<bool, kNumNodes> done{};
  for (int k = 0; k < kNumNodes; ++k) {
    int pick = -1;
    for (int u = 0; u < kNumNodes; ++u)
      if (!done[static_cast<size_t>(u)] && indeg[static_cast<size_t>(u)] == 0) {
        pick = u;
        break;
      }
    if (pick < 0) return false;
    done[static_cast<size_t>(pick)] = true;
    forEachImmediateOut(r, pick, [&](int v) { indeg[static_cast<size_t>(v)]--; });
  }
  return true;
}

CausalClass RouteTable::classify(int src, int dst) const noexcept {
  const int dn = portNode(dst);
  if (src == dn) return CausalClass::Delayed;
  // src->dst ferme un cycle Immediate s'il existe déjà un chemin Immediate dst ~> src
  return reachesImmediate(routes_, dn, src) ? CausalClass::Delayed : CausalClass::Immediate;
}

bool RouteTable::immediateAcyclic() const noexcept { return acyclic(routes_); }

bool RouteTable::validate(const std::vector<Route>& routes, std::uint32_t nextRouteId, std::string& error) {
  for (size_t i = 0; i < routes.size(); ++i) {
    const Route& r = routes[i];
    if (r.src < 0 || r.src >= kNumNodes || r.dst < 0 || r.dst >= kNumPorts) {
      error = "noeud ou port invalide";
      return false;
    }
    if (routeSlot(r.src, r.dst) < 0) {
      error = std::string("paire non autorisee ") + nodeName(r.src) + ">" + portName(r.dst);
      return false;
    }
    if (r.id == 0 || r.id >= nextRouteId) {
      error = "identifiant de route hors plage (0 < id < nextRouteId)";
      return false;
    }
    if (r.src == portNode(r.dst) && r.cls != CausalClass::Delayed) {
      error = "auto-boucle de classe Immediate refusee";
      return false;
    }
    for (size_t j = 0; j < i; ++j) {
      if (routes[j].id == r.id) {
        error = "identifiant de route duplique";
        return false;
      }
      if (routes[j].src == r.src && routes[j].dst == r.dst) {
        error = "paire (src,dst) dupliquee";
        return false;
      }
    }
  }
  if (!acyclic(routes)) {
    error = "sous-graphe Immediate cyclique : etat refuse";
    return false;
  }
  return true;
}

bool RouteTable::assign(const std::vector<Route>& routes, std::uint32_t nextRouteId, std::string& error) {
  if (!validate(routes, nextRouteId, error)) return false;
  routes_ = routes;
  next_ = nextRouteId;
  return true;
}

RouteTable::Result RouteTable::apply(const std::vector<Op>& ops) {
  Result res;
  auto fail = [&res](std::string why) {
    res.ok = false;
    res.error = std::move(why);
    res.addedIds.clear();  // aucun effet partiel, y compris dans le compte rendu
    return res;
  };
  std::vector<Route> work = routes_;
  std::uint32_t next = next_;
  for (const Op& op : ops) {
    if (op.kind == Op::Kind::Add) {
      if (op.src < 0 || op.src >= kNumNodes || op.dst < 0 || op.dst >= kNumPorts || routeSlot(op.src, op.dst) < 0)
        return fail("ajout refuse : paire non autorisee");
      for (const auto& r : work)
        if (r.src == op.src && r.dst == op.dst)
          return fail(std::string("ajout refuse : route deja presente ") + nodeName(op.src) + ">" + portName(op.dst));
      Route r;
      r.id = next++;
      r.src = op.src;
      r.dst = op.dst;
      const int dn = portNode(op.dst);
      r.cls = (op.src == dn || reachesImmediate(work, dn, op.src)) ? CausalClass::Delayed : CausalClass::Immediate;
      work.push_back(r);
      res.addedIds.push_back(r.id);
    } else {
      auto it = std::find_if(work.begin(), work.end(), [&](const Route& r) { return r.id == op.id; });
      if (it == work.end()) return fail("retrait refuse : identifiant inconnu");
      work.erase(it);  // aucune autre route n'est reclassée
    }
  }
  std::string err;
  if (!validate(work, next, err)) return fail("transaction invalide : " + err);  // garde de sûreté
  routes_ = std::move(work);
  next_ = next;
  res.ok = true;
  return res;
}

CompiledTopology RouteTable::compile() const {
  CompiledTopology c;
  // ordre topologique déterministe (Kahn, plus petit indice d'abord) sur le sous-graphe Immediate + arêtes OPk>BUS
  std::array<int, kNumNodes> indeg{};
  for (int u = 0; u < kNumNodes; ++u) forEachImmediateOut(routes_, u, [&](int v) { indeg[static_cast<size_t>(v)]++; });
  std::array<bool, kNumNodes> done{};
  for (int k = 0; k < kNumNodes; ++k) {
    int pick = -1;
    for (int u = 0; u < kNumNodes; ++u)
      if (!done[static_cast<size_t>(u)] && indeg[static_cast<size_t>(u)] == 0) {
        pick = u;
        break;
      }
    if (pick < 0) pick = k;  // inatteignable si immediateAcyclic() ; garde
    done[static_cast<size_t>(pick)] = true;
    c.order[static_cast<size_t>(k)] = static_cast<std::uint8_t>(pick);
    forEachImmediateOut(routes_, pick, [&](int v) { indeg[static_cast<size_t>(v)]--; });
  }
  int e = 0;
  for (int dn = 0; dn < kNumNodes; ++dn) {
    c.inBegin[static_cast<size_t>(dn)] = static_cast<std::uint8_t>(e);
    for (int src = 0; src < kNumNodes; ++src)
      for (int port = 0; port < kNumPorts; ++port) {
        if (portNode(port) != dn) continue;
        const Route* r = findPair(src, port);
        if (!r) continue;
        auto& edge = c.edges[static_cast<size_t>(e)];
        edge.routeId = r->id;
        edge.src = static_cast<std::uint8_t>(src);
        edge.dst = static_cast<std::uint8_t>(port);
        edge.slot = static_cast<std::uint8_t>(routeSlot(src, port));
        edge.delayed = r->cls == CausalClass::Delayed;
        ++e;
      }
  }
  c.inBegin[kNumNodes] = static_cast<std::uint8_t>(e);
  c.numEdges = static_cast<std::uint8_t>(e);
  std::vector<Route> sorted = routes_;
  std::sort(sorted.begin(), sorted.end(), [](const Route& a, const Route& b) { return a.id < b.id; });
  for (size_t i = 0; i < sorted.size() && i < c.routes.size(); ++i) c.routes[i] = sorted[i];
  c.numRoutes = static_cast<std::uint8_t>(sorted.size());
  c.nextRouteId = next_;
  return c;
}

// ---------------------------------------------------------------- préréglages de topologie FM
namespace {
struct TopoDef {
  const char* name;
  int n;
  int pairs[8][2];  // (source, destination) en indices d'opérateur 1..4, dans l'ordre d'ajout
};
constexpr TopoDef kTopologies[kNumFmTopologies] = {
    {"Cascade", 3, {{4, 3}, {3, 2}, {2, 1}}},
    {"Dual Pair", 2, {{2, 1}, {4, 3}}},
    {"Triple Stack", 3, {{2, 1}, {3, 1}, {4, 1}}},
    {"Fan Out", 3, {{4, 1}, {4, 2}, {4, 3}}},
    {"Diamond", 4, {{4, 2}, {4, 3}, {2, 1}, {3, 1}}},
    {"Parallel", 0, {}},
    {"Dual Cross", 4, {{2, 1}, {1, 2}, {4, 3}, {3, 4}}},
    {"Ring Network", 4, {{1, 2}, {2, 3}, {3, 4}, {4, 1}}},
    {"Recursive Stack", 4, {{4, 4}, {4, 3}, {3, 2}, {2, 1}}},
    {"Hybrid Mesh", 6, {{2, 1}, {3, 1}, {3, 2}, {4, 2}, {4, 3}, {1, 4}}},
};
}  // namespace

const char* fmTopologyName(int index) noexcept {
  return index >= 0 && index < kNumFmTopologies ? kTopologies[index].name : "?";
}

std::vector<RouteTable::Op> fmTopologyOps(const RouteTable& current, int index) {
  std::vector<RouteTable::Op> ops;
  if (index < 0 || index >= kNumFmTopologies) return ops;
  for (const auto& r : current.routes())
    if (r.src >= kNodeOP1 && r.src <= kNodeOP4 && r.dst >= kNodeOP1 && r.dst <= kNodeOP4)
      ops.push_back(RouteTable::Op::remove(r.id));
  const auto& t = kTopologies[index];
  for (int i = 0; i < t.n; ++i) ops.push_back(RouteTable::Op::add(t.pairs[i][0], t.pairs[i][1]));
  return ops;
}

}  // namespace sbd
