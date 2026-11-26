// G-01 (DAG / sticky) et G-02 (transactions atomiques).
#include <cstdio>
#include <random>
#include <vector>

#include "TestKit.h"
#include "core/Topology.h"

using namespace sbd;
using Op = RouteTable::Op;

namespace {

bool orderIsTopological(const RouteTable& t) {
  const auto c = t.compile();
  std::array<int, kNumNodes> pos{};
  for (int k = 0; k < kNumNodes; ++k) pos[c.order[static_cast<size_t>(k)]] = k;
  for (const auto& r : t.routes())
    if (r.cls == CausalClass::Immediate && pos[static_cast<size_t>(r.src)] >= pos[static_cast<size_t>(portNode(r.dst))]) return false;
  return true;
}

void testRules() {
  bool ok = true;
  RouteTable t;
  // 1. ajout simple -> Immediate ; fermeture de cycle -> Delayed ; auto-boucle -> Delayed
  auto r1 = t.apply({Op::add(kNodeWT, kNodeOP1)});
  ok = ok && r1.ok && t.findPair(kNodeWT, kNodeOP1)->cls == CausalClass::Immediate;
  auto r2 = t.apply({Op::add(kNodeOP1, kNodeWT)});
  ok = ok && r2.ok && t.findPair(kNodeOP1, kNodeWT)->cls == CausalClass::Delayed;
  auto r3 = t.apply({Op::add(kNodeOP1, kNodeOP1)});
  ok = ok && r3.ok && t.findPair(kNodeOP1, kNodeOP1)->cls == CausalClass::Delayed;
  // 2. sticky : retirer WT>OP1 (le cycle disparaît) ne reclasse pas OP1>WT
  const std::uint32_t idCycle = t.findPair(kNodeOP1, kNodeWT)->id;
  auto r4 = t.apply({Op::remove(t.findPair(kNodeWT, kNodeOP1)->id)});
  ok = ok && r4.ok && t.findId(idCycle) && t.findId(idCycle)->cls == CausalClass::Delayed;
  // 3. ré-ajouter WT>OP1 : le chemin Immediate OP1~>WT n'existe pas (OP1>WT est Delayed) -> Immediate
  auto r5 = t.apply({Op::add(kNodeWT, kNodeOP1)});
  ok = ok && r5.ok && t.findPair(kNodeWT, kNodeOP1)->cls == CausalClass::Immediate;
  // 4. changement de destination = retrait + ajout, nouvel identifiant
  const std::uint32_t old = t.findPair(kNodeWT, kNodeOP1)->id;
  auto r6 = t.apply({Op::remove(old), Op::add(kNodeWT, kNodeOP2)});
  ok = ok && r6.ok && !t.findId(old) && t.findPair(kNodeWT, kNodeOP2)->id > old;
  // 5. union cyclique dans une même transaction : le second ajout ferme le cycle -> Delayed
  RouteTable u;
  auto r7 = u.apply({Op::add(kNodeOP1, kNodeOP2), Op::add(kNodeOP2, kNodeOP1)});
  ok = ok && r7.ok && u.findPair(kNodeOP1, kNodeOP2)->cls == CausalClass::Immediate &&
       u.findPair(kNodeOP2, kNodeOP1)->cls == CausalClass::Delayed;
  // 6. cycle à trois via CROSS : WT>OP1, OP1>OP2 (I), OP2>WT ferme -> Delayed
  RouteTable v;
  auto r8 = v.apply({Op::add(kNodeWT, kNodeOP1), Op::add(kNodeOP1, kNodeOP2), Op::add(kNodeOP2, kNodeWT)});
  ok = ok && r8.ok && v.findPair(kNodeOP2, kNodeWT)->cls == CausalClass::Delayed && orderIsTopological(v);
  // 7. ordre topologique toujours valide
  ok = ok && orderIsTopological(t) && orderIsTopological(u);
  TK_CHECK(ok);
  tk::result("G-01a", ok, "regles B sticky : cycle -> D, auto-boucle -> D, survivants jamais reclasses, retarget = nouvel id, union dans une transaction");
}

void testRandomSequences() {
  std::mt19937_64 rng(0xA2C3F3ULL);
  int failures = 0;
  long long transactions = 0;
  const int kSequences = 1200;
  for (int seq = 0; seq < kSequences; ++seq) {
    RouteTable t;
    std::uint32_t lastNext = t.nextRouteId();
    for (int step = 0; step < 40; ++step) {
      std::vector<Op> ops;
      const int nops = 1 + static_cast<int>(rng() % 3);
      for (int k = 0; k < nops; ++k) {
        if (!t.routes().empty() && (rng() % 3 == 0)) {
          ops.push_back(Op::remove(t.routes()[rng() % t.routes().size()].id));
        } else {
          const int slot = static_cast<int>(rng() % kMaxRoutes);
          ops.push_back(Op::add(routeSlotInfo(slot).src, routeSlotInfo(slot).dst));
        }
      }
      const std::vector<Route> before = t.routes();
      const std::uint32_t beforeNext = t.nextRouteId();
      const auto res = t.apply(ops);
      ++transactions;
      if (!res.ok) {
        // rejet : état strictement identique (G-02)
        if (!(t.routes() == before) || t.nextRouteId() != beforeNext) ++failures;
        continue;
      }
      // invariants
      if (!t.immediateAcyclic() || !orderIsTopological(t)) ++failures;
      if (t.nextRouteId() < lastNext) ++failures;
      lastNext = t.nextRouteId();
      for (const auto& r : t.routes()) {
        if (r.id == 0 || r.id >= t.nextRouteId()) ++failures;
        if (r.src == portNode(r.dst) && r.cls != CausalClass::Delayed) ++failures;
        for (const auto& b : before)  // survivants : classe inchangée
          if (b.id == r.id && !(b == r)) ++failures;
      }
      for (std::uint32_t id : res.addedIds)  // identifiants nouveaux jamais réutilisés
        for (const auto& b : before)
          if (b.id == id) ++failures;
      std::string err;
      if (!RouteTable::validate(t.routes(), t.nextRouteId(), err)) ++failures;
    }
  }
  TK_CHECK(failures == 0);
  tk::result("G-01b", failures == 0, "%d sequences, %lld transactions aleatoires : Immediate acyclique, ordre topologique, survivants inchanges, ids croissants jamais reutilises (%d echecs)",
             kSequences, transactions, failures);
}

void testInvalidTransactions() {
  RouteTable t;
  bool ok = t.apply({Op::add(kNodeOP2, kNodeOP1), Op::add(kNodeWT, kNodeOP1)}).ok;
  const auto before = t.routes();
  const auto next = t.nextRouteId();
  struct Case { const char* name; std::vector<Op> ops; };
  const std::vector<Case> cases = {
      {"doublon", {Op::add(kNodeOP2, kNodeOP1)}},
      {"identifiant inconnu", {Op::remove(999)}},
      {"paire interdite WT>WT", {Op::add(kNodeWT, kNodeWT)}},
      {"noeud invalide", {Op::add(5, kNodeOP1)}},
      {"valide puis invalide", {Op::add(kNodeOP1, kNodeOP2), Op::remove(12345)}},
      {"ajout puis doublon interne", {Op::add(kNodeOP1, kNodeWT), Op::add(kNodeOP1, kNodeWT)}},
  };
  for (const auto& c : cases) {
    const auto res = t.apply(c.ops);
    const bool rejected = !res.ok && t.routes() == before && t.nextRouteId() == next && res.addedIds.empty();
    if (!rejected) tk::info("G-02 cas non rejete proprement : %s", c.name);
    ok = ok && rejected;
  }
  TK_CHECK(ok);
  tk::result("G-02", ok, "6 transactions invalides rejetees sans effet partiel (etat et nextRouteId identiques)");
}

void testValidation() {
  std::string err;
  bool ok = true;
  const auto I = CausalClass::Immediate, D = CausalClass::Delayed;
  ok = ok && RouteTable::validate({{1, kNodeWT, kNodeOP1, I}, {2, kNodeOP1, kNodeWT, D}}, 3, err);
  ok = ok && !RouteTable::validate({{1, kNodeWT, kNodeOP1, I}, {2, kNodeOP1, kNodeWT, I}}, 3, err);  // cycle Immediate
  ok = ok && !RouteTable::validate({{1, kNodeOP1, kNodeOP1, I}}, 2, err);                            // auto-boucle I
  ok = ok && !RouteTable::validate({{1, kNodeWT, kNodeOP1, I}, {1, kNodeOP2, kNodeOP1, I}}, 3, err);  // id dupliqué
  ok = ok && !RouteTable::validate({{1, kNodeWT, kNodeOP1, I}, {2, kNodeWT, kNodeOP1, D}}, 3, err);  // paire dupliquée
  ok = ok && !RouteTable::validate({{3, kNodeWT, kNodeOP1, I}}, 3, err);                              // id >= next
  ok = ok && !RouteTable::validate({{0, kNodeWT, kNodeOP1, I}}, 3, err);                              // id 0
  ok = ok && !RouteTable::validate({{1, kNodeWT, kNodeWT, D}}, 2, err);                               // paire interdite
  RouteTable t;
  ok = ok && !t.assign({{1, kNodeWT, kNodeOP1, I}, {2, kNodeOP1, kNodeWT, I}}, 3, err) && t.routes().empty();
  TK_CHECK(ok);
  tk::result("G-01c", ok, "validation au chargement : cycle Immediate, auto-boucle I, doublons, ids hors plage refuses");
}

}  // namespace

void runTopologyTests() {
  std::printf("[topologie]\n");
  testRules();
  testRandomSequences();
  testInvalidTransactions();
  testValidation();
}
