#include "Topology.h"

#include <algorithm>

namespace sbd {

namespace {
constexpr RouteSlotInfo kSlots[kMaxRoutes] = {
    {kNodeOP2, kNodeOP1, ParamId::DepthOp2Op1, RouteKind::Internal},
    {kNodeOP1, kNodeOP2, ParamId::DepthOp1Op2, RouteKind::Internal},
    {kNodeOP1, kNodeOP1, ParamId::DepthOp1Op1, RouteKind::Internal},
    {kNodeOP2, kNodeOP2, ParamId::DepthOp2Op2, RouteKind::Internal},
    {kNodeWT, kNodeOP1, ParamId::DepthWtOp1, RouteKind::CrossForward},
    {kNodeWT, kNodeOP2, ParamId::DepthWtOp2, RouteKind::CrossForward},
    {kNodeOP1, kNodeWT, ParamId::DepthOp1Wt, RouteKind::CrossReverse},
    {kNodeOP2, kNodeWT, ParamId::DepthOp2Wt, RouteKind::CrossReverse},
};
}  // namespace

const char* nodeName(int node) noexcept {
  switch (node) {
    case kNodeWT: return "WT";
    case kNodeOP1: return "OP1";
    case kNodeOP2: return "OP2";
    default: return "?";
  }
}

int nodeFromName(const std::string& name) noexcept {
  if (name == "WT") return kNodeWT;
  if (name == "OP1") return kNodeOP1;
  if (name == "OP2") return kNodeOP2;
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
  // parcours en largeur sur le sous-graphe Immediate, mémoire fixe
  std::array<bool, kNumNodes> seen{};
  std::array<int, kNumNodes> stack{};
  int sp = 0;
  stack[sp++] = from;
  seen[static_cast<size_t>(from)] = true;
  while (sp > 0) {
    const int u = stack[--sp];
    if (u == to) return true;
    for (const auto& e : r)
      if (e.cls == CausalClass::Immediate && e.src == u && !seen[static_cast<size_t>(e.dst)]) {
        seen[static_cast<size_t>(e.dst)] = true;
        stack[sp++] = e.dst;
      }
  }
  return false;
}

bool RouteTable::acyclic(const std::vector<Route>& r) noexcept {
  std::array<int, kNumNodes> indeg{};
  for (const auto& e : r)
    if (e.cls == CausalClass::Immediate) {
      if (e.src == e.dst) return false;
      indeg[static_cast<size_t>(e.dst)]++;
    }
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
    for (const auto& e : r)
      if (e.cls == CausalClass::Immediate && e.src == pick) indeg[static_cast<size_t>(e.dst)]--;
  }
  return true;
}

CausalClass RouteTable::classify(int src, int dst) const noexcept {
  if (src == dst) return CausalClass::Delayed;
  // src->dst ferme un cycle Immediate s'il existe déjà un chemin Immediate dst ~> src
  return reachesImmediate(routes_, dst, src) ? CausalClass::Delayed : CausalClass::Immediate;
}

bool RouteTable::immediateAcyclic() const noexcept { return acyclic(routes_); }

bool RouteTable::validate(const std::vector<Route>& routes, std::uint32_t nextRouteId, std::string& error) {
  for (size_t i = 0; i < routes.size(); ++i) {
    const Route& r = routes[i];
    if (r.src < 0 || r.src >= kNumNodes || r.dst < 0 || r.dst >= kNumNodes) {
      error = "noeud invalide";
      return false;
    }
    if (routeSlot(r.src, r.dst) < 0) {
      error = std::string("paire non autorisee ") + nodeName(r.src) + ">" + nodeName(r.dst);
      return false;
    }
    if (r.id == 0 || r.id >= nextRouteId) {
      error = "identifiant de route hors plage (0 < id < nextRouteId)";
      return false;
    }
    if (r.src == r.dst && r.cls != CausalClass::Delayed) {
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
      if (op.src < 0 || op.src >= kNumNodes || op.dst < 0 || op.dst >= kNumNodes || routeSlot(op.src, op.dst) < 0)
        return fail("ajout refuse : paire non autorisee");
      for (const auto& r : work)
        if (r.src == op.src && r.dst == op.dst)
          return fail(std::string("ajout refuse : route deja presente ") + nodeName(op.src) + ">" + nodeName(op.dst));
      Route r;
      r.id = next++;
      r.src = op.src;
      r.dst = op.dst;
      r.cls = (op.src == op.dst || reachesImmediate(work, op.dst, op.src)) ? CausalClass::Delayed : CausalClass::Immediate;
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
  // ordre topologique déterministe (Kahn, plus petit indice d'abord) sur le sous-graphe Immediate
  std::array<int, kNumNodes> indeg{};
  for (const auto& r : routes_)
    if (r.cls == CausalClass::Immediate) indeg[static_cast<size_t>(r.dst)]++;
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
    for (const auto& r : routes_)
      if (r.cls == CausalClass::Immediate && r.src == pick) indeg[static_cast<size_t>(r.dst)]--;
  }
  int e = 0;
  for (int dst = 0; dst < kNumNodes; ++dst) {
    c.inBegin[static_cast<size_t>(dst)] = static_cast<std::uint8_t>(e);
    for (int src = 0; src < kNumNodes; ++src) {
      const Route* r = findPair(src, dst);
      if (!r) continue;
      auto& edge = c.edges[static_cast<size_t>(e)];
      edge.routeId = r->id;
      edge.src = static_cast<std::uint8_t>(src);
      edge.dst = static_cast<std::uint8_t>(dst);
      edge.slot = static_cast<std::uint8_t>(routeSlot(src, dst));
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

}  // namespace sbd
