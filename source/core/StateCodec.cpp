#include "StateCodec.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace sbd {

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

bool hexDigit(char c, int& d) {
  if (c >= '0' && c <= '9')
    d = c - '0';
  else if (c >= 'a' && c <= 'f')
    d = 10 + (c - 'a');
  else
    return false;
  return true;
}

bool parseHex32(const std::string& s, std::uint32_t& out) {
  if (s.size() != 10 || s[0] != '0' || s[1] != 'x') return false;
  std::uint32_t v = 0;
  for (size_t i = 2; i < s.size(); ++i) {
    int d;
    if (!hexDigit(s[i], d)) return false;
    v = (v << 4) | static_cast<std::uint32_t>(d);
  }
  out = v;
  return true;
}

bool parseHex64(const std::string& s, std::uint64_t& out) {
  if (s.size() != 16) return false;
  std::uint64_t v = 0;
  for (char c : s) {
    int d;
    if (!hexDigit(c, d)) return false;
    v = (v << 4) | static_cast<std::uint64_t>(d);
  }
  out = v;
  return true;
}

bool parseFloatHex(const std::string& s, float& out) {
  std::uint32_t b;
  if (!parseHex32(s, b)) return false;
  out = bitsFloat(b);
  return std::isfinite(out);
}

std::string hex32(float f) {
  char b[16];
  std::snprintf(b, sizeof b, "0x%08x", static_cast<unsigned>(floatBits(f)));
  return b;
}
std::string hex64(std::uint64_t v) {
  char b[24];
  std::snprintf(b, sizeof b, "%016llx", static_cast<unsigned long long>(v));
  return b;
}

std::string bytesHex(const std::string& s) {
  static const char* d = "0123456789abcdef";
  std::string o;
  for (unsigned char c : s) {
    o += d[c >> 4];
    o += d[c & 15];
  }
  return o.empty() ? std::string("-") : o;
}
bool hexBytes(const std::string& h, std::string& out) {
  if (h == "-") {
    out.clear();
    return true;
  }
  if (h.size() % 2 || h.size() > 128) return false;
  std::string o;
  for (size_t i = 0; i < h.size(); i += 2) {
    int a, b;
    if (!hexDigit(h[i], a) || !hexDigit(h[i + 1], b)) return false;
    o += static_cast<char>((a << 4) | b);
  }
  out = o;
  return true;
}

bool validAssetId(const std::string& s) {
  if (s.empty() || s.size() > 96) return false;
  for (char c : s)
    if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ':' || c == '_' || c == '-' || c == '.')) return false;
  return true;
}

bool curveValid(const CurvePoints& c) {
  if (c.n < 2 || c.n > kMaxCurvePoints) return false;
  for (int k = 0; k < c.n; ++k) {
    const float x = c.x[static_cast<size_t>(k)], y = c.y[static_cast<size_t>(k)];
    if (!std::isfinite(x) || !std::isfinite(y) || x < 0.0f || x > 1.0f || y < -1.0f || y > 1.0f) return false;
    if (k > 0 && !(x > c.x[static_cast<size_t>(k - 1)])) return false;
  }
  return true;
}

}  // namespace

const char* qualityName(Quality q) noexcept {
  switch (q) {
    case Quality::Eco: return "Eco";
    case Quality::Normal: return "Normal";
    case Quality::High: return "High";
  }
  return "?";
}

bool CurvePoints::operator==(const CurvePoints& o) const noexcept {
  if (n != o.n) return false;
  for (int k = 0; k < n; ++k)
    if (x[static_cast<size_t>(k)] != o.x[static_cast<size_t>(k)] || y[static_cast<size_t>(k)] != o.y[static_cast<size_t>(k)])
      return false;
  return true;
}

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

bool validateState(const PluginState& s, std::string& error) {
  for (int i = 0; i < kNumParams; ++i) {
    const auto& spec = paramSpec(i);
    const float v = s.params.v[static_cast<size_t>(i)];
    if (!std::isfinite(v) || v < spec.minValue || v > spec.maxValue) {
      error = std::string("valeur hors plage ou non finie pour ") + spec.stableId;
      return false;
    }
    if (spec.type != ParamType::Float && v != std::round(v)) {
      error = std::string("valeur non entiere pour ") + spec.stableId;
      return false;
    }
  }
  if (!RouteTable::validate(s.routes, s.nextRouteId, error)) return false;
  if (!validAssetId(s.bank)) {
    error = "identifiant de banque invalide";
    return false;
  }
  if (!validateModMatrix(s.mod, error)) return false;
  for (const auto& c : s.lfoCurves)
    if (!curveValid(c)) {
      error = "courbe LFO invalide";
      return false;
    }
  for (const auto& n : s.macroNames)
    if (n.size() > 64) {
      error = "nom de macro trop long";
      return false;
    }
  return true;
}

std::string encodeState(const PluginState& state, const std::string& writer, StateScope scope) {
  std::string o;
  o += "SUBDESTRUCKT_STATE " + std::to_string(kStateVersion) + "\n";
  o += "writer " + (writer.empty() ? std::string("-") : writer) + "\n";
  o += std::string("scope ") + (scope == StateScope::Daw ? "daw" : "preset") + "\n";
  char buf[200];
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
  for (const auto& x : r)
    o += "route " + std::to_string(x.id) + " " + nodeName(x.src) + " " + portName(x.dst) + " " +
         std::string(1, classLetter(x.cls)) + "\n";
  o += "bank " + state.bank + "\n";
  o += "modnext " + std::to_string(state.mod.nextId) + "\n";
  for (const auto& m : state.mod.routes) {
    o += "mod " + std::to_string(m.id) + " " + modSourceName(m.source) + " " +
         (m.dest == ParamId::Count ? std::string("-") : std::string(paramSpec(m.dest).stableId)) + " " + hex32(m.depth) +
         " " + std::to_string(static_cast<int>(m.curve)) + " " + (m.polarity == ModPolarity::Centered ? "C" : "F") + " " +
         (m.invert ? "1" : "0") + " " + hex32(m.offset) + " " + modSourceName(m.via) + " " + hex32(m.viaAmount) + " " +
         hex32(m.smoothMs) + " " + (m.rate == ModRate::Audio ? "A" : "C") + " " + (m.enabled ? "1" : "0") + " " +
         (m.locked ? "1" : "0") + " " + std::to_string(m.numPoints);
    for (int k = 0; k < m.numPoints; ++k) o += " " + hex32(m.px[static_cast<size_t>(k)]) + " " + hex32(m.py[static_cast<size_t>(k)]);
    o += "\n";
  }
  for (int k = 0; k < 4; ++k) o += "macroname " + std::to_string(k + 1) + " " + bytesHex(state.macroNames[static_cast<size_t>(k)]) + "\n";
  for (int k = 0; k < 4; ++k) {
    const auto& c = state.lfoCurves[static_cast<size_t>(k)];
    o += "lfocurve " + std::to_string(k + 1) + " " + std::to_string(c.n);
    for (int j = 0; j < c.n; ++j) o += " " + hex32(c.x[static_cast<size_t>(j)]) + " " + hex32(c.y[static_cast<size_t>(j)]);
    o += "\n";
  }
  o += "seed " + hex64(state.seed) + " " + hex64(state.streamCounters[0]) + " " + hex64(state.streamCounters[1]) + " " +
       hex64(state.streamCounters[2]) + "\n";
  if (scope == StateScope::Daw)
    o += std::string("instance quality=") + qualityName(state.instance.quality) +
         " drone=" + (state.instance.drone ? "1" : "0") + " drone_restore=" + (state.instance.droneRestore ? "1" : "0") +
         "\n";
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
  PluginState s;  // défauts : migration implicite des sections absentes du format v1
  s.mod = ModMatrixState{};
  std::array<bool, kNumParams> seen{};
  bool haveTopology = false, haveEnd = false, haveScope = false, haveInstance = false, haveModNext = false;
  int version = 0;
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
    version = static_cast<int>(ver);
  }
  const bool v1 = version == 1;
  auto fail = [&](const std::string& why) {
    error = why;
    return false;
  };
  while (const std::string* l = nextNonEmpty()) {
    const auto w = splitWords(*l);
    if (haveEnd) return fail("contenu apres 'end'");
    const std::string& k = w[0];
    if (k == "writer") continue;
    if (k == "param") {
      if (haveTopology || w.size() != 4) return fail("ligne param invalide");
      ParamId id;
      if (!findParam(w[1], id)) return fail("parametre inconnu : " + w[1]);
      const auto& spec = paramSpec(id);
      if (spec.version > version) return fail("parametre " + w[1] + " absent du format " + std::to_string(version));
      std::uint32_t bits;
      if (!parseHex32(w[2], bits)) return fail("valeur invalide pour " + w[1]);
      const float v = bitsFloat(bits);
      if (!std::isfinite(v) || v < spec.minValue || v > spec.maxValue) return fail("valeur hors plage ou non finie pour " + w[1]);
      if (seen[static_cast<size_t>(id)]) return fail("parametre duplique : " + w[1]);
      seen[static_cast<size_t>(id)] = true;
      s.params[id] = v;
    } else if (k == "scope" && !v1) {
      if (haveScope || haveTopology || w.size() != 2 || (w[1] != "daw" && w[1] != "preset")) return fail("ligne scope invalide");
      haveScope = true;
      s.hasInstance = w[1] == "daw";
    } else if (k == "topology") {
      std::uint32_t nx = 0;
      if (haveTopology || w.size() != 2 || w[1].rfind("next=", 0) != 0 || !parseUnsigned(w[1].substr(5), nx) || nx == 0)
        return fail("ligne topology invalide");
      haveTopology = true;
      s.nextRouteId = nx;
    } else if (k == "route") {
      if (!haveTopology || w.size() != 5) return fail("ligne route invalide");
      Route r;
      if (!parseUnsigned(w[1], r.id)) return fail("identifiant de route invalide");
      r.src = nodeFromName(w[2]);
      r.dst = portFromName(w[3]);
      if (v1 && (r.src > kNodeOP2 || r.dst > kNodeOP2)) return fail("noeud inconnu du format 1");
      if (w[4] == "I")
        r.cls = CausalClass::Immediate;
      else if (w[4] == "D")
        r.cls = CausalClass::Delayed;
      else
        return fail("classe causale invalide");
      s.routes.push_back(r);
    } else if (k == "bank" && !v1) {
      if (w.size() != 2 || !validAssetId(w[1])) return fail("ligne bank invalide");
      s.bank = w[1];
    } else if (k == "modnext" && !v1) {
      std::uint32_t n = 0;
      if (haveModNext || w.size() != 2 || !parseUnsigned(w[1], n) || n == 0) return fail("ligne modnext invalide");
      haveModNext = true;
      s.mod.nextId = n;
    } else if (k == "mod" && !v1) {
      if (!haveModNext || w.size() < 16) return fail("ligne mod invalide");
      ModRoute m;
      std::uint32_t u = 0;
      if (!parseUnsigned(w[1], m.id)) return fail("identifiant de modulation invalide");
      if (!modSourceFromName(w[2], m.source) || !modSourceFromName(w[9], m.via)) return fail("source de modulation inconnue");
      ParamId dest;
      if (w[3] == "-")
        m.dest = ParamId::Count;
      else if (findParam(w[3], dest))
        m.dest = dest;
      else
        return fail("destination de modulation inconnue : " + w[3]);
      if (!parseFloatHex(w[4], m.depth) || !parseUnsigned(w[5], u) || u >= static_cast<std::uint32_t>(ModCurve::Count))
        return fail("ligne mod invalide (profondeur/courbe)");
      m.curve = static_cast<ModCurve>(u);
      if (w[6] != "C" && w[6] != "F") return fail("polarite invalide");
      m.polarity = w[6] == "C" ? ModPolarity::Centered : ModPolarity::Free;
      if (w[7] != "0" && w[7] != "1") return fail("inversion invalide");
      m.invert = w[7] == "1";
      if (!parseFloatHex(w[8], m.offset) || !parseFloatHex(w[10], m.viaAmount) || !parseFloatHex(w[11], m.smoothMs))
        return fail("ligne mod invalide (valeurs)");
      if (w[12] != "C" && w[12] != "A") return fail("rate invalide");
      m.rate = w[12] == "A" ? ModRate::Audio : ModRate::Control;
      if ((w[13] != "0" && w[13] != "1") || (w[14] != "0" && w[14] != "1")) return fail("drapeaux invalides");
      m.enabled = w[13] == "1";
      m.locked = w[14] == "1";
      if (!parseUnsigned(w[15], u) || u < 2 || u > static_cast<std::uint32_t>(kMaxCurvePoints) || w.size() != 16 + 2 * u)
        return fail("points de courbe invalides");
      m.numPoints = static_cast<std::uint8_t>(u);
      for (std::uint32_t p = 0; p < u; ++p)
        if (!parseFloatHex(w[16 + 2 * p], m.px[p]) || !parseFloatHex(w[17 + 2 * p], m.py[p])) return fail("point invalide");
      s.mod.routes.push_back(m);
    } else if (k == "macroname" && !v1) {
      std::uint32_t n = 0;
      std::string name;
      if (w.size() != 3 || !parseUnsigned(w[1], n) || n < 1 || n > 4 || !hexBytes(w[2], name)) return fail("ligne macroname invalide");
      s.macroNames[n - 1] = name;
    } else if (k == "lfocurve" && !v1) {
      std::uint32_t n = 0, c = 0;
      if (w.size() < 3 || !parseUnsigned(w[1], n) || n < 1 || n > 4 || !parseUnsigned(w[2], c) || c < 2 ||
          c > static_cast<std::uint32_t>(kMaxCurvePoints) || w.size() != 3 + 2 * c)
        return fail("ligne lfocurve invalide");
      CurvePoints cp;
      cp.n = static_cast<std::uint8_t>(c);
      for (std::uint32_t p = 0; p < c; ++p)
        if (!parseFloatHex(w[3 + 2 * p], cp.x[p]) || !parseFloatHex(w[4 + 2 * p], cp.y[p])) return fail("point invalide");
      s.lfoCurves[n - 1] = cp;
    } else if (k == "seed" && !v1) {
      if (w.size() != 5 || !parseHex64(w[1], s.seed) || !parseHex64(w[2], s.streamCounters[0]) ||
          !parseHex64(w[3], s.streamCounters[1]) || !parseHex64(w[4], s.streamCounters[2]))
        return fail("ligne seed invalide");
    } else if (k == "instance" && !v1) {
      if (haveInstance || w.size() != 4) return fail("ligne instance invalide");
      haveInstance = true;
      if (w[1] == "quality=Eco")
        s.instance.quality = Quality::Eco;
      else if (w[1] == "quality=Normal")
        s.instance.quality = Quality::Normal;
      else if (w[1] == "quality=High")
        s.instance.quality = Quality::High;
      else
        return fail("qualite invalide");
      if (w[2] != "drone=0" && w[2] != "drone=1") return fail("drone invalide");
      if (w[3] != "drone_restore=0" && w[3] != "drone_restore=1") return fail("drone_restore invalide");
      s.instance.drone = w[2] == "drone=1";
      s.instance.droneRestore = w[3] == "drone_restore=1";
    } else if (k == "end") {
      haveEnd = true;
    } else {
      return fail("ligne inconnue : " + k);
    }
  }
  if (!haveEnd || !haveTopology) return fail("etat incomplet");
  if (!v1 && (!haveScope || !haveModNext)) return fail("etat incomplet (scope/modnext)");
  if (!v1 && s.hasInstance != haveInstance) return fail("section instance incoherente avec le scope");
  for (int i = 0; i < kNumParams; ++i)
    if (!seen[static_cast<size_t>(i)] && paramSpec(i).version <= version)
      return fail(std::string("parametre manquant : ") + paramSpec(i).stableId);
  if (v1) s.hasInstance = false;  // 0.1.0-b5 : aucune préférence d'instance enregistrée
  if (!validateState(s, error)) return false;
  std::sort(s.routes.begin(), s.routes.end(), [](const Route& a, const Route& b) { return a.id < b.id; });
  s.sourceVersion = version;
  out = std::move(s);
  return true;
}

}  // namespace sbd
