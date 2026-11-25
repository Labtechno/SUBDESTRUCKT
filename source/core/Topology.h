// SUBDESTRUCKT — L1 — graphe spécialisé FM/CROSS, stratégie B sticky (V1-CAUSAL-CONTRACT).
//
// Nœuds de la tranche : WT, OP1, OP2. Huit routes possibles (WT->WT n'existe pas dans le design) :
//   FM interne : OP2>OP1, OP1>OP2, OP1>OP1 (auto), OP2>OP2 (auto)
//   CROSS WT->FM : WT>OP1, WT>OP2       CROSS FM->WT : OP1>WT, OP2>WT
// Règles :
//   - nouvelle route : Delayed si auto-boucle ou si elle ferme un cycle dans le sous-graphe Immediate courant
//     (y compris les ajouts antérieurs de la même transaction), sinon Immediate ;
//   - survivantes : classe jamais recalculée (sticky), même si le cycle disparaît ;
//   - changement de source/destination = retrait + ajout (nouvel identifiant) ;
//   - profondeur nulle : la route et sa classe restent (la profondeur est un paramètre continu séparé) ;
//   - StableRouteID strictement croissants, jamais réutilisés ;
//   - transactions atomiques : en cas d'erreur, aucun effet partiel ;
//   - validation au chargement : refus d'un sous-graphe Immediate cyclique, d'une auto-boucle Immediate, de doublons.
// Tout ce fichier s'exécute hors RT ; seul CompiledTopology (taille fixe, sans pointeur) est lu par le moteur.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "Parameters.h"

namespace sbd {

constexpr int kNumNodes = 3;  // 0 = WT, 1 = OP1, 2 = OP2
constexpr int kNodeWT = 0, kNodeOP1 = 1, kNodeOP2 = 2;
constexpr int kMaxRoutes = 8;

const char* nodeName(int node) noexcept;     // "WT", "OP1", "OP2"
int nodeFromName(const std::string& name) noexcept;  // -1 si inconnu

enum class CausalClass : std::uint8_t { Immediate = 0, Delayed = 1 };
inline char classLetter(CausalClass c) noexcept { return c == CausalClass::Immediate ? 'I' : 'D'; }

enum class RouteKind : std::uint8_t { Internal = 0, CrossForward = 1, CrossReverse = 2 };

struct RouteSlotInfo {
  int src, dst;
  ParamId depthParam;
  RouteKind kind;
};
// Slot (0..7) d'une paire (src,dst) autorisée, -1 sinon.
int routeSlot(int src, int dst) noexcept;
const RouteSlotInfo& routeSlotInfo(int slot) noexcept;

struct Route {
  std::uint32_t id = 0;  // StableRouteID
  int src = 0, dst = 0;
  CausalClass cls = CausalClass::Immediate;
  bool operator==(const Route& o) const noexcept { return id == o.id && src == o.src && dst == o.dst && cls == o.cls; }
};

// Topologie compilée pour le moteur : taille fixe, copiable, sans allocation.
struct CompiledTopology {
  struct Edge {
    std::uint32_t routeId = 0;
    std::uint8_t src = 0, dst = 0, slot = 0;
    bool delayed = false;
  };
  std::array<std::uint8_t, kNumNodes> order{};        // ordre d'évaluation topologique (Immediate)
  std::array<Edge, kMaxRoutes> edges{};                // groupées par destination puis source croissante
  std::array<std::uint8_t, kNumNodes + 1> inBegin{};   // arêtes entrantes du nœud d : [inBegin[d], inBegin[d+1])
  std::uint8_t numEdges = 0;
  std::array<Route, kMaxRoutes> routes{};              // copie pour télémétrie/UI
  std::uint8_t numRoutes = 0;
  std::uint32_t nextRouteId = 1;
  std::uint64_t serial = 0;                            // numéro de publication (diagnostic)
};

class RouteTable {
 public:
  struct Op {
    enum class Kind { Add, Remove } kind = Kind::Add;
    int src = 0, dst = 0;   // Add
    std::uint32_t id = 0;   // Remove
    static Op add(int s, int d) { return Op{Kind::Add, s, d, 0}; }
    static Op remove(std::uint32_t routeId) { return Op{Kind::Remove, 0, 0, routeId}; }
  };
  struct Result {
    bool ok = false;
    std::string error;
    std::vector<std::uint32_t> addedIds;
  };

  RouteTable() = default;
  // Construit depuis un état chargé : validation complète, refus sinon (retourne false, *this inchangé).
  bool assign(const std::vector<Route>& routes, std::uint32_t nextRouteId, std::string& error);

  // Transaction atomique.
  Result apply(const std::vector<Op>& ops);

  const std::vector<Route>& routes() const noexcept { return routes_; }
  std::uint32_t nextRouteId() const noexcept { return next_; }
  const Route* findPair(int src, int dst) const noexcept;
  const Route* findId(std::uint32_t id) const noexcept;
  // Classe qu'aurait une nouvelle route src->dst dans l'état courant.
  CausalClass classify(int src, int dst) const noexcept;
  bool immediateAcyclic() const noexcept;

  CompiledTopology compile() const;  // pré-condition : immediateAcyclic()

  static bool validate(const std::vector<Route>& routes, std::uint32_t nextRouteId, std::string& error);

 private:
  static bool reachesImmediate(const std::vector<Route>& r, int from, int to) noexcept;
  static bool acyclic(const std::vector<Route>& r) noexcept;
  std::vector<Route> routes_;
  std::uint32_t next_ = 1;
};

}  // namespace sbd
