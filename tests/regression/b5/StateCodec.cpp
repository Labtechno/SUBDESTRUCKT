// ORACLE FIGE 0.1.0-b5 (copie de 8d23342, namespace sbd_b5) : ne jamais modifier. Reference de non-regression ARCH-G4.
#include "StateCodec.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace sbd_b5 {

namespace {

std::uint32_t floatBits(float f) {
  std::uint32_t u;
  std::memcpy(&u, &f, sizeof u);
  return u;
}
float bitsFloat(std::uint32_t u) {
  float f;
  std::memcpy(&f, &u, sizeof f);
  return f;
}

std::vector<std::string> splitWords(const std::string& line) {
  std::vector<std::string> w;
  size_t i = 0;
  while (i < line.size()) {
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) ++i;
    size_t j = i;
    while (j < line.size() && line[j] != ' ' && line[j] != '\t' && line[j] != '\r') ++j;
    if (j > i) w.push_back(line.substr(i, j - i));
    i = j;
  }
  return w;
}

// Entier non signé décimal strict (aucun signe, aucun caractère parasite).
bool parseUnsigned(const std::string& s, std::uint32_t& out) {
  if (s.empty() || s.size() > 10) return false;
  unsigned long long v = 0;
  for (char c : s) {
    if (c < '0' || c > '9') return false;
    v = v * 10 + static_cast<unsigned>(c - '0');
  }
  if (v > 0xffffffffULL) return false;
  out = static_cast<std::uint32_t>(v);
  return true;
}

bool parseHex32(const std::string& s, std::uint32_t& out) {
  if (s.size() != 10 || s[0] != '0' || s[1] != 'x') return false;
  std::uint32_t v = 0;
  for (size_t i = 2; i < s.size(); ++i) {
    const char c = s[i];
    int d;
    if (c >= '0' && c <= '9')
      d = c - '0';
    else if (c >= 'a' && c <= 'f')
      d = 10 + (c - 'a');
    else
      return false;
    v = (v << 4) | static_cast<std::uint32_t>(d);
  }
  out = v;
  return true;
}

}  // namespace

PluginState defaultState() {
  PluginState s;
  s.params = ParamValues::defaults();
  RouteTable t;
  const auto res = t.apply({RouteTable::Op::add(kNodeOP2, kNodeOP1), RouteTable::Op::add(kNodeWT, kNodeOP1),
                            RouteTable::Op::add(kNodeOP1, kNodeWT)});
  (void)res;  // toujours valide (vérifié par les tests)
  s.routes = t.routes();
  s.nextRouteId = t.nextRouteId();
  return s;
}

std::string encodeState(const PluginState& state, const std::string& writer) {
  std::string o;
  o += "SUBDESTRUCKT_STATE " + std::to_string(kStateVersion) + "\n";
  o += "writer " + (writer.empty() ? std::string("-") : writer) + "\n";
  char buf[160];
  for (int i = 0; i < kNumParams; ++i) {
    const float v = state.params.v[static_cast<size_t>(i)];
    // la valeur lisible est informative uniquement (la lecture utilise les bits)
    std::snprintf(buf, sizeof buf, "param %s 0x%08x %.9g\n", paramSpec(i).stableId, static_cast<unsigned>(floatBits(v)),
                  static_cast<double>(v));
    o += buf;
  }
  o += "topology next=" + std::to_string(state.nextRouteId) + "\n";
  std::vector<Route> r = state.routes;
  std::sort(r.begin(), r.end(), [](const Route& a, const Route& b) { return a.id < b.id; });
  for (const auto& x : r) {
    o += "route " + std::to_string(x.id) + " " + nodeName(x.src) + " " + nodeName(x.dst) + " " +
         std::string(1, classLetter(x.cls)) + "\n";
  }
  o += "end\n";
  return o;
}

bool decodeState(const std::string& text, PluginState& out, std::string& error) {
  std::vector<std::string> lines;
  {
    size_t i = 0;
    while (i <= text.size()) {
      size_t j = text.find('\n', i);
      if (j == std::string::npos) j = text.size();
      lines.push_back(text.substr(i, j - i));
      i = j + 1;
    }
  }
  PluginState s;
  std::array<bool, kNumParams> seen{};
  bool haveTopology = false, haveEnd = false;
  size_t li = 0;
  auto nextNonEmpty = [&]() -> const std::string* {
    while (li < lines.size()) {
      const std::string& l = lines[li++];
      if (!splitWords(l).empty()) return &l;
    }
    return nullptr;
  };
  const std::string* first = nextNonEmpty();
  if (!first) {
    error = "etat vide";
    return false;
  }
  {
    const auto w = splitWords(*first);
    std::uint32_t ver = 0;
    if (w.size() != 2 || w[0] != "SUBDESTRUCKT_STATE" || !parseUnsigned(w[1], ver) || ver == 0) {
      error = "en-tete d'etat invalide";
      return false;
    }
    if (ver > static_cast<std::uint32_t>(kStateVersion)) {
      error = "etat ecrit par une version plus recente (format " + w[1] + ") : refuse, aucune lecture des formats futurs";
      return false;
    }
  }
  while (const std::string* l = nextNonEmpty()) {
    const auto w = splitWords(*l);
    if (haveEnd) {
      error = "contenu apres 'end'";
      return false;
    }
    if (w[0] == "writer") continue;
    if (w[0] == "param") {
      if (haveTopology || w.size() != 4) {
        error = "ligne param invalide";
        return false;
      }
      ParamId id;
      if (!findParam(w[1], id)) {
        error = "parametre inconnu : " + w[1];
        return false;
      }
      std::uint32_t bits;
      if (!parseHex32(w[2], bits)) {
        error = "valeur invalide pour " + w[1];
        return false;
      }
      const float v = bitsFloat(bits);
      const auto& spec = paramSpec(id);
      if (!std::isfinite(v) || v < spec.minValue || v > spec.maxValue) {
        error = "valeur hors plage ou non finie pour " + w[1];
        return false;
      }
      if (seen[static_cast<size_t>(id)]) {
        error = "parametre duplique : " + w[1];
        return false;
      }
      seen[static_cast<size_t>(id)] = true;
      s.params[id] = v;
    } else if (w[0] == "topology") {
      std::uint32_t nx = 0;
      if (haveTopology || w.size() != 2 || w[1].rfind("next=", 0) != 0 || !parseUnsigned(w[1].substr(5), nx) || nx == 0) {
        error = "ligne topology invalide";
        return false;
      }
      haveTopology = true;
      s.nextRouteId = nx;
    } else if (w[0] == "route") {
      if (!haveTopology || w.size() != 5) {
        error = "ligne route invalide";
        return false;
      }
      Route r;
      if (!parseUnsigned(w[1], r.id)) {
        error = "identifiant de route invalide";
        return false;
      }
      r.src = nodeFromName(w[2]);
      r.dst = nodeFromName(w[3]);
      if (w[4] == "I")
        r.cls = CausalClass::Immediate;
      else if (w[4] == "D")
        r.cls = CausalClass::Delayed;
      else {
        error = "classe causale invalide";
        return false;
      }
      s.routes.push_back(r);
    } else if (w[0] == "end") {
      haveEnd = true;
    } else {
      error = "ligne inconnue : " + w[0];
      return false;
    }
  }
  if (!haveEnd || !haveTopology) {
    error = "etat incomplet";
    return false;
  }
  for (int i = 0; i < kNumParams; ++i)
    if (!seen[static_cast<size_t>(i)]) {
      error = std::string("parametre manquant : ") + paramSpec(i).stableId;
      return false;
    }
  if (!RouteTable::validate(s.routes, s.nextRouteId, error)) return false;
  std::sort(s.routes.begin(), s.routes.end(), [](const Route& a, const Route& b) { return a.id < b.id; });
  out = std::move(s);
  return true;
}

}  // namespace sbd_b5
