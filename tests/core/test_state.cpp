// G-03 : état persistant versionné, aller-retour exact et refus transactionnels.
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>

#include "TestKit.h"
#include "core/StateCodec.h"

using namespace sbd;

namespace {

std::string replaceLine(const std::string& text, const std::string& prefix, const std::string& replacement) {
  const size_t p = text.find(prefix);
  if (p == std::string::npos) return text;
  const size_t e = text.find('\n', p);
  return text.substr(0, p) + replacement + text.substr(e);
}

std::string hexOf(float f) {
  std::uint32_t u;
  std::memcpy(&u, &f, sizeof u);
  char b[16];
  std::snprintf(b, sizeof b, "0x%08x", static_cast<unsigned>(u));
  return b;
}

void testDefaultAndRoundTrip() {
  bool ok = true;
  const PluginState d = defaultState();
  // preset minimal : OP2>OP1 (I), WT>OP1 (I), OP1>WT (D)
  ok = ok && d.routes.size() == 3 && d.routes[0].src == kNodeOP2 && d.routes[0].cls == CausalClass::Immediate &&
       d.routes[1].src == kNodeWT && d.routes[1].cls == CausalClass::Immediate && d.routes[2].src == kNodeOP1 &&
       d.routes[2].dst == kNodeWT && d.routes[2].cls == CausalClass::Delayed && d.nextRouteId == 4;
  const std::string a = encodeState(d, "test");
  const std::string b = encodeState(d, "test");
  ok = ok && a == b;  // déterministe
  PluginState r;
  std::string err;
  ok = ok && decodeState(a, r, err) && r == d && encodeState(r, "test") == a;
  // états aléatoires valides
  std::mt19937_64 rng(77);
  int bad = 0;
  for (int i = 0; i < 300; ++i) {
    PluginState s;
    for (int p = 0; p < kNumParams; ++p) {
      const auto& sp = paramSpec(p);
      std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
      s.params.v[static_cast<size_t>(p)] = sanitize(static_cast<ParamId>(p), u(rng));  // discrets : valeurs entières
    }
    RouteTable t;
    for (int k = 0; k < 6; ++k) {
      const int slot = static_cast<int>(rng() % kMaxRoutes);
      t.apply({RouteTable::Op::add(routeSlotInfo(slot).src, routeSlotInfo(slot).dst)});
      if (!t.routes().empty() && rng() % 4 == 0) t.apply({RouteTable::Op::remove(t.routes()[0].id)});
    }
    s.routes = t.routes();
    std::sort(s.routes.begin(), s.routes.end(), [](const Route& x, const Route& y) { return x.id < y.id; });
    s.nextRouteId = t.nextRouteId();
    PluginState back;
    if (!decodeState(encodeState(s, "w"), back, err) || !(back == s)) ++bad;
  }
  ok = ok && bad == 0;
  TK_CHECK(ok);
  tk::result("G-03a", ok, "preset minimal conforme, encodage deterministe, 300 allers-retours exacts (%d echecs)", bad);
}

void testRefusals() {
  const PluginState d = defaultState();
  const std::string good = encodeState(d, "test");
  struct Case { const char* name; std::string text; };
  std::vector<Case> cases = {
      {"version future", replaceLine(good, "SUBDESTRUCKT_STATE", "SUBDESTRUCKT_STATE " + std::to_string(kStateVersion + 1))},
      {"en-tete invalide", replaceLine(good, "SUBDESTRUCKT_STATE", "SUBDESTRUKT_STATE 1")},
      {"cycle Immediate", replaceLine(good, "route 3 ", "route 3 OP1 WT I")},
      {"auto-boucle Immediate", replaceLine(good, "route 3 ", "route 3 OP1 OP1 I")},
      {"id duplique", replaceLine(good, "route 3 ", "route 2 OP1 WT D")},
      {"paire dupliquee", replaceLine(good, "route 3 ", "route 3 WT OP1 D")},
      {"id >= next", replaceLine(good, "route 3 ", "route 9 OP1 WT D")},
      {"paire interdite", replaceLine(good, "route 3 ", "route 3 WT WT D")},
      {"valeur hors plage", replaceLine(good, "param fm.op2_op1.depth", "param fm.op2_op1.depth " + hexOf(9.0f) + " 9")},
      {"valeur NaN", replaceLine(good, "param wt.level", "param wt.level 0x7fc00000 nan")},
      {"parametre inconnu", replaceLine(good, "param wt.level", "param wt.levelx 0x3f000000 0.5")},
      {"parametre manquant", replaceLine(good, "param wt.level", "")},
      {"ligne inconnue", replaceLine(good, "writer", "garbage 1 2 3")},
      {"sans end", good.substr(0, good.find("end"))},
      {"vide", ""},
  };
  // une seconde copie d'un paramètre
  cases.push_back({"parametre duplique", replaceLine(good, "param wt.level", "param op1.level 0x3f000000 0.5")});
  bool ok = true;
  for (const auto& c : cases) {
    PluginState out = d;
    out.params[ParamId::WtLevel] = 0.123f;  // sentinelle
    const PluginState sentinel = out;
    std::string err;
    const bool accepted = decodeState(c.text, out, err);
    const bool fine = !accepted && out == sentinel;
    if (!fine) tk::info("G-03 cas non refuse proprement : %s", c.name);
    ok = ok && fine;
  }
  TK_CHECK(ok);
  tk::result("G-03b", ok, "%zu etats invalides refuses sans effet (version future, cycle I, auto-boucle I, doublons, hors plage, NaN, inconnu, manquant, tronque)",
             cases.size());
}

}  // namespace

void runStateTests() {
  std::printf("[etat]\n");
  testDefaultAndRoundTrip();
  testRefusals();
}
