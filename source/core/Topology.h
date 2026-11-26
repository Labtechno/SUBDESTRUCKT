// SUBDESTRUCKT — L1 — graphe spécialisé FM/CROSS, stratégie B sticky (V1-CAUSAL-CONTRACT).
//
// Nœuds : WT, OP1..OP4, BUS (bus FM : somme pondérée de OP1..OP4, entrées implicites Immediate toujours présentes).
// Une route relie un nœud source à un PORT de destination :
//   ports 0..4  = entrée du nœud de même indice (0 = WT phase, 1..4 = OPn, mode PM/LinFM/TZFM/ExpFM de OPn) ;
//   ports 5..10 = WT.FREQ, WT.X, WT.Y, WT.Z, WT.LEVEL, WT.CHAR.
// 55 routes possibles : 16 FM internes (dont 4 auto-boucles), 4 WT>OPn (CROSS avant), 35 {OP1..4,BUS}>WT.* (CROSS
// arrière). Les 8 routes de la tranche 0.1.0-b5 conservent leurs indices de slot 0..7.
// Règles :
//   - nouvelle route : Delayed si auto-boucle ou si elle ferme un cycle dans le sous-graphe Immediate courant
//     (arêtes implicites OPk>BUS comprises, ajouts antérieurs de la même transaction compris), sinon Immediate ;
//   - survivantes : classe jamais recalculée (sticky), même si le cycle disparaît ;
//   - changement de source/destination = retrait + ajout (nouvel identifiant) ;
//   - profondeur nulle : la route et sa classe restent (la profondeur est un paramètre continu séparé) ;
//   - StableRouteID strictement croissants, jamais réutilisés ; transactions atomiques sans effet partiel ;
//   - validation au chargement : refus d'un sous-graphe Immediate cyclique, d'une auto-boucle Immediate, de doublons.
// Tout ce fichier s'exécute hors RT ; seul CompiledTopology (taille fixe, sans pointeur) est lu par le moteur.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "Parameters.h"

namespace sbd {

constexpr int kNumNodes = 6;  // 0 = WT, 1..4 = OP1..OP4, 5 = BUS
constexpr int kNodeWT = 0, kNodeOP1 = 1, kNodeOP2 = 2, kNodeOP3 = 3, kNodeOP4 = 4, kNodeBUS = 5;
constexpr int kNumOps = 4;
constexpr int kNumPorts = 11;
constexpr int kPortWtPhase = 0, kPortWtFreq = 5, kPortWtX = 6, kPortWtY = 7, kPortWtZ = 8, kPortWtLevel = 9,
              kPortWtChar = 10;
constexpr int kMaxRoutes = 55;
constexpr int kNumV1Routes = 8;

const char* nodeName(int node) noexcept;              // "WT", "OP1".."OP4", "BUS"
int nodeFromName(const std::string& name) noexcept;   // -1 si inconnu
const char* portName(int port) noexcept;              // "WT" (phase), "OP1".."OP4", "WT.FREQ", ...
int portFromName(const std::string& name) noexcept;   // -1 si inconnu
inline constexpr int portNode(int port) noexcept { return port < kNumNodes - 1 ? port : kNodeWT; }

enum class CausalClass : std::uint8_t { Immediate = 0, Delayed = 1 };
inline char classLetter(CausalClass c) noexcept { return c == CausalClass::Immediate ? 'I' : 'D'; }

enum class RouteKind : std::uint8_t { Internal = 0, CrossForward = 1, CrossReverse = 2 };

struct RouteSlotInfo {
  int src, dst;  // nœud source, port destination
  ParamId depthParam;
  RouteKind kind;
};
// Slot (0..54) d'une paire (src, port) autorisée, -1 sinon.
int routeSlot(int src, int dst) noexcept;
const RouteSlotInfo& routeSlotInfo(int slot) noexcept;

struct Route {
  std::uint32_t id = 0;  // StableRouteID
  int src = 0, dst = 0;  // nœud source, port destination
  CausalClass cls = CausalClass::Immediate;
  bool operator==(const Route& o) const noexcept { return id == o.id && src == o.src && dst == o.dst && cls == o.cls; }
};

// Topologie compilée pour le moteur : taille fixe, copiable, sans allocation.
struct CompiledTopology {
  struct Edge {
    std::uint32_t routeId = 0;
    std::uint8_t src = 0, dst = 0, slot = 0;  // dst = PORT
    bool delayed = false;
  };
  std::array<std::uint8_t, kNumNodes> order{};        // ordre d'évaluation topologique (Immediate)
  std::array<Edge, kMaxRoutes> edges{};                // groupées par nœud destination, puis source, puis port
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
    int src = 0, dst = 0;   // Add (dst = port)
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
  // Classe qu'aurait une nouvelle route src->port dans l'état courant.
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

// Préréglages de topologie FM (dix topologies candidates du référentiel) : transactions de routes OP>OP.
constexpr int kNumFmTopologies = 10;
const char* fmTopologyName(int index) noexcept;
// Opérations remplaçant toutes les routes FM internes par la topologie demandée (routes CROSS conservées).
std::vector<RouteTable::Op> fmTopologyOps(const RouteTable& current, int index);

}  // namespace sbd
