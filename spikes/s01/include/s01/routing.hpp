// SUBDESTRUCKT — Spike S0.1 — classification causale sticky et persistante (stratégie B validée).
// Hors RT : toutes les opérations de ce fichier s'exécutent côté contrôle ; seul Compiled est lu en RT.
//
// Contrat (docs/V1-S0.1-CAUSALITY-VALIDATION.md §5, revue S0 §2/§10) :
//   1. route survivante -> classe inchangée ;
//   2. nouvelle route -> Immediate si le sous-graphe Immediate reste acyclique ;
//   3. sinon -> Delayed ;
//   4. suppression -> aucun reclassement automatique ;
//   5. source/destination modifiée = suppression + création (nouvel identifiant) ;
//   6. état sauvegardé/rechargé -> même classification ;
//   7. validation à la lecture : sous-graphe Immediate obligatoirement acyclique.
// Choix du harness (documenté dans le rapport) : une route inactive conserve sa classe et compte dans le
// test d'acyclicité, pour qu'une réactivation ne soit jamais un changement structurel.
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace s01 {

constexpr int kNodes = 5;  // 0 = WT, 1..4 = OP1..OP4
constexpr int kMaxRoutes = kNodes * kNodes;
inline int slotOf(int dst, int src) { return dst * kNodes + src; }

enum class CausalClass : std::uint8_t { Immediate = 0, Delayed = 1 };
inline const char* name(CausalClass c) { return c == CausalClass::Immediate ? "I" : "D"; }

struct RouteRec {
  std::uint32_t id = 0;  // StableRouteID : jamais réutilisé
  int src = 0, dst = 0;
  CausalClass cls = CausalClass::Immediate;
  bool active = true;
  double depth = 0.0;  // unité du mode de la destination (rad / Hz / octaves)
};

// Topologie compilée, lue par le moteur. Ne dépend que de l'ensemble {(src,dst,classe,actif)} et
// jamais de l'ordre de création : deux états de même contenu donnent le même Compiled (bit à bit).
struct Compiled {
  std::array<int, kNodes> order{};                 // ordre d'évaluation (topologique sur Immediate)
  std::array<std::uint8_t, kMaxRoutes> present{};  // route active présente dans le slot
  std::array<std::uint8_t, kMaxRoutes> delayed{};
  std::array<int, kNodes + 1> off{};               // arêtes groupées par destination, src croissant
  std::array<int, kMaxRoutes> eSrc{};
  std::array<int, kMaxRoutes> eSlot{};
  std::array<std::uint8_t, kMaxRoutes> eDelayed{};
  std::array<std::uint8_t, kNodes> srcHasDelayed{};  // la source alimente au moins une arête Delayed
  int nRoutes = 0, nDelayed = 0;
};

class RouteTable {
 public:
  const std::vector<RouteRec>& routes() const { return r_; }
  std::uint32_t nextId() const { return next_; }

  const RouteRec* find(std::uint32_t id) const {
    for (const auto& x : r_) if (x.id == id) return &x;
    return nullptr;
  }
  const RouteRec* findPair(int src, int dst) const {
    for (const auto& x : r_) if (x.src == src && x.dst == dst) return &x;
    return nullptr;
  }

  // Classe qu'aurait une nouvelle route src->dst dans l'état courant (règles 2/3).
  CausalClass classify(int src, int dst) const {
    if (src == dst) return CausalClass::Delayed;
    // src->dst ferme un cycle Immediate s'il existe déjà un chemin Immediate dst ~> src
    return reachesImmediate(dst, src) ? CausalClass::Delayed : CausalClass::Immediate;
  }

  std::uint32_t add(int src, int dst, double depth = 0.0, bool active = true) {
    checkNode(src); checkNode(dst);
    if (findPair(src, dst)) throw std::invalid_argument("route dupliquee (src,dst)");
    RouteRec x;
    x.id = next_++;
    x.src = src; x.dst = dst; x.depth = depth; x.active = active;
    x.cls = classify(src, dst);
    r_.push_back(x);
    return x.id;
  }
  void remove(std::uint32_t id) {  // règle 4 : aucune autre route n'est touchée
    auto it = std::find_if(r_.begin(), r_.end(), [&](const RouteRec& x) { return x.id == id; });
    if (it == r_.end()) throw std::invalid_argument("route inconnue");
    r_.erase(it);
  }
  // Règle 5 : modification de source/destination = suppression + création (nouvel identifiant).
  std::uint32_t retarget(std::uint32_t id, int newSrc, int newDst) {
    const RouteRec* o = find(id);
    if (!o) throw std::invalid_argument("route inconnue");
    double d = o->depth; bool a = o->active;
    remove(id);
    return add(newSrc, newDst, d, a);
  }
  void setDepth(std::uint32_t id, double d) { mut(id).depth = d; }
  void setActive(std::uint32_t id, bool a) { mut(id).active = a; }

  // --- Persistance ----------------------------------------------------------------------------------
  // Format texte canonique : "S01ROUTES v1 next=<n>" puis une ligne par route, triées par id.
  std::string serialize() const {
    std::vector<RouteRec> v = r_;
    std::sort(v.begin(), v.end(), [](const RouteRec& a, const RouteRec& b) { return a.id < b.id; });
    std::ostringstream o;
    o << "S01ROUTES v1 next=" << next_ << "\n";
    char buf[160];
    for (const auto& x : v) {
      std::snprintf(buf, sizeof buf, "route id=%u src=%d dst=%d class=%s active=%d depth=%.17g\n", x.id, x.src,
                    x.dst, name(x.cls), int(x.active), x.depth);
      o << buf;
    }
    return o.str();
  }
  // Règle 7 : la lecture valide et refuse (exception) — elle ne reclasse jamais.
  static RouteTable deserialize(const std::string& s) {
    RouteTable t;
    std::istringstream in(s);
    std::string line;
    if (!std::getline(in, line)) throw std::runtime_error("etat vide");
    unsigned nx = 0;
    if (std::sscanf(line.c_str(), "S01ROUTES v1 next=%u", &nx) != 1) throw std::runtime_error("entete invalide");
    t.next_ = nx;
    while (std::getline(in, line)) {
      if (line.empty()) continue;
      RouteRec x; char cls = 0; int act = 0; unsigned id = 0;
      if (std::sscanf(line.c_str(), "route id=%u src=%d dst=%d class=%c active=%d depth=%lf", &id, &x.src, &x.dst,
                      &cls, &act, &x.depth) != 6)
        throw std::runtime_error("ligne invalide: " + line);
      x.id = id;
      if (cls != 'I' && cls != 'D') throw std::runtime_error("classe invalide");
      x.cls = cls == 'I' ? CausalClass::Immediate : CausalClass::Delayed;
      x.active = act != 0;
      checkNode(x.src); checkNode(x.dst);
      if (x.id >= t.next_) throw std::runtime_error("id >= next");
      if (t.find(x.id)) throw std::runtime_error("id duplique");
      if (t.findPair(x.src, x.dst)) throw std::runtime_error("paire dupliquee");
      t.r_.push_back(x);
    }
    if (!t.immediateAcyclic()) throw std::runtime_error("sous-graphe Immediate cyclique: etat refuse");
    return t;
  }

  bool immediateAcyclic() const {
    std::array<int, kNodes> indeg{};
    std::array<std::array<std::uint8_t, kNodes>, kNodes> adj{};  // adj[src][dst]
    for (const auto& x : r_)
      if (x.cls == CausalClass::Immediate) {
        if (x.src == x.dst) return false;
        if (!adj[x.src][x.dst]) { adj[x.src][x.dst] = 1; indeg[x.dst]++; }
      }
    std::array<std::uint8_t, kNodes> done{};
    for (int k = 0; k < kNodes; ++k) {
      int pick = -1;
      for (int u = 0; u < kNodes; ++u) if (!done[u] && indeg[u] == 0) { pick = u; break; }
      if (pick < 0) return false;
      done[pick] = 1;
      for (int v = 0; v < kNodes; ++v) if (adj[pick][v]) indeg[v]--;
    }
    return true;
  }

  // Compilation pour le moteur. Les routes inactives ne sont pas évaluées mais gardent leur classe.
  Compiled compile() const {
    if (!immediateAcyclic()) throw std::logic_error("compile: Immediate cyclique");
    Compiled c;
    std::array<std::array<std::uint8_t, kNodes>, kNodes> adj{};  // Immediate (actives ou non) pour l'ordre
    for (const auto& x : r_) {
      if (x.cls == CausalClass::Immediate) adj[x.src][x.dst] = 1;
      if (!x.active) continue;
      const int s = slotOf(x.dst, x.src);
      c.present[s] = 1;
      c.delayed[s] = x.cls == CausalClass::Delayed;
    }
    std::array<int, kNodes> indeg{};
    for (int u = 0; u < kNodes; ++u) for (int v = 0; v < kNodes; ++v) indeg[v] += adj[u][v];
    std::array<std::uint8_t, kNodes> done{};
    for (int k = 0; k < kNodes; ++k) {  // Kahn, plus petit indice d'abord : déterministe
      int pick = -1;
      for (int u = 0; u < kNodes; ++u) if (!done[u] && indeg[u] == 0) { pick = u; break; }
      done[pick] = 1;
      c.order[k] = pick;
      for (int v = 0; v < kNodes; ++v) if (adj[pick][v]) indeg[v]--;
    }
    int e = 0;
    for (int dst = 0; dst < kNodes; ++dst) {
      c.off[dst] = e;
      for (int src = 0; src < kNodes; ++src) {
        const int s = slotOf(dst, src);
        if (!c.present[s]) continue;
        c.eSrc[e] = src; c.eSlot[e] = s; c.eDelayed[e] = c.delayed[s];
        if (c.delayed[s]) { c.nDelayed++; c.srcHasDelayed[src] = 1; }
        ++e;
      }
    }
    c.off[kNodes] = e;
    c.nRoutes = e;
    return c;
  }

  // Fabrique de test : liste ordonnée (src,dst) ajoutée dans cet ordre.
  static RouteTable build(const std::vector<std::pair<int, int>>& srcDst, double depth) {
    RouteTable t;
    for (auto& p : srcDst) t.add(p.first, p.second, depth);
    return t;
  }

 private:
  static void checkNode(int n) {
    if (n < 0 || n >= kNodes) throw std::invalid_argument("noeud hors domaine");
  }
  RouteRec& mut(std::uint32_t id) {
    for (auto& x : r_) if (x.id == id) return x;
    throw std::invalid_argument("route inconnue");
  }
  bool reachesImmediate(int from, int to) const {
    std::array<std::uint8_t, kNodes> seen{};
    std::array<int, kNodes> st{};
    int sp = 0;
    st[sp++] = from; seen[from] = 1;
    while (sp) {
      int u = st[--sp];
      if (u == to) return true;
      for (const auto& x : r_)
        if (x.cls == CausalClass::Immediate && x.src == u && !seen[x.dst]) { seen[x.dst] = 1; st[sp++] = x.dst; }
    }
    return false;
  }

  std::vector<RouteRec> r_;
  std::uint32_t next_ = 1;
};

// Vérifie l'ordre d'évaluation : toute arête Immediate lit une source déjà calculée.
inline bool verifyCausal(const Compiled& c) {
  std::array<int, kNodes> pos{};
  for (int k = 0; k < kNodes; ++k) pos[c.order[k]] = k;
  for (int dst = 0; dst < kNodes; ++dst)
    for (int e = c.off[dst]; e < c.off[dst + 1]; ++e)
      if (!c.eDelayed[e] && pos[c.eSrc[e]] >= pos[dst]) return false;
  return true;
}

}  // namespace s01
