#include "ModMatrix.h"

#include <algorithm>
#include <cmath>

namespace sbd {

namespace {
struct SourceInfo {
  const char* name;
  const char* label;
  bool bipolar;
  int audioStage;
};
constexpr SourceInfo kSources[kNumModSources] = {
    {"none", "None", false, -1},
    {"velocity", "Velocity", false, -1},
    {"note", "Note (keytrack)", true, -1},
    {"bend", "Pitch Bend", true, -1},
    {"modwheel", "Mod Wheel", false, -1},
    {"breath", "Breath", false, -1},
    {"expression", "Expression", false, -1},
    {"pressure", "Channel Pressure", false, -1},
    {"sustain", "Sustain Pedal", false, -1},
    {"env.amp", "ENV Amp", false, -1},
    {"env.wt", "ENV WT", false, -1},
    {"env.fm", "ENV FM", false, -1},
    {"env.aux1", "ENV AUX1", false, -1},
    {"env.aux2", "ENV AUX2", false, -1},
    {"lfo1", "LFO1", true, -1},
    {"lfo2", "LFO2", true, -1},
    {"lfo3", "LFO3", true, -1},
    {"lfo4", "LFO4", true, -1},
    {"rnd.hold", "Random Hold", true, -1},
    {"rnd.smooth", "Random Smooth", true, -1},
    {"chaos", "Chaos", true, -1},
    {"macro1", "Macro 1", false, -1},
    {"macro2", "Macro 2", false, -1},
    {"macro3", "Macro 3", false, -1},
    {"macro4", "Macro 4", false, -1},
    {"fol1", "Follower WT", false, 1},
    {"fol2", "Follower FM", false, 2},
    {"fol3", "Follower Global", false, 4},
    {"y2.wt1", "Y2 WT1", false, 1},
    {"y2.wt2", "Y2 WT2", false, 1},
    {"y2.fm1", "Y2 FM1", false, 2},
    {"y2.fm2", "Y2 FM2", false, 2},
    {"y2.gl1", "Y2 GL1", false, 4},
    {"y2.gl2", "Y2 GL2", false, 4},
};
constexpr const char* kCurveNames[] = {"Linear", "Exponential", "Logarithmic", "S-Curve", "Stepped", "Fold", "Custom"};

bool isMacroParam(ParamId p, int& k) noexcept {
  const int i = idx(p) - idx(ParamId::Macro1);
  if (i >= 0 && i < 4) {
    k = i;
    return true;
  }
  return false;
}
int macroOfSource(ModSource s) noexcept {
  const int i = static_cast<int>(s) - static_cast<int>(ModSource::Macro1);
  return (i >= 0 && i < 4) ? i : -1;
}

bool followerAllowed(ModSource s, ParamId dest) noexcept {
  const int src = modSourceAudioStage(s);
  if (src < 0) return true;
  const int d = paramSpec(dest).stage;
  if (d > src) return true;
  return (src == 1 && d == 2) || (src == 2 && d == 1);  // chaînes locales parallèles, sans retour post-FX
}

bool macroGraphAcyclic(const ModMatrixState& m) {
  // arêtes macroI -> macroJ (source macro, destination paramètre macro)
  std::array<std::array<bool, 4>, 4> adj{};
  for (const auto& r : m.routes) {
    int dk = -1;
    const int sk = macroOfSource(r.source);
    if (sk >= 0 && isMacroParam(r.dest, dk)) adj[static_cast<size_t>(sk)][static_cast<size_t>(dk)] = true;
    const int vk = macroOfSource(r.via);
    if (vk >= 0 && isMacroParam(r.dest, dk)) adj[static_cast<size_t>(vk)][static_cast<size_t>(dk)] = true;
  }
  std::array<int, 4> indeg{};
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j) indeg[static_cast<size_t>(j)] += adj[static_cast<size_t>(i)][static_cast<size_t>(j)];
  std::array<bool, 4> done{};
  for (int k = 0; k < 4; ++k) {
    int pick = -1;
    for (int u = 0; u < 4; ++u)
      if (!done[static_cast<size_t>(u)] && indeg[static_cast<size_t>(u)] == 0) {
        pick = u;
        break;
      }
    if (pick < 0) return false;
    done[static_cast<size_t>(pick)] = true;
    for (int j = 0; j < 4; ++j)
      if (adj[static_cast<size_t>(pick)][static_cast<size_t>(j)]) indeg[static_cast<size_t>(j)]--;
  }
  return true;
}

bool finiteIn(float v, float lo, float hi) { return std::isfinite(v) && v >= lo && v <= hi; }
}  // namespace

const char* modSourceName(ModSource s) noexcept { return kSources[static_cast<int>(s)].name; }
const char* modSourceLabel(ModSource s) noexcept { return kSources[static_cast<int>(s)].label; }
bool modSourceBipolar(ModSource s) noexcept { return kSources[static_cast<int>(s)].bipolar; }
int modSourceAudioStage(ModSource s) noexcept { return kSources[static_cast<int>(s)].audioStage; }

bool modSourceFromName(const std::string& name, ModSource& out) noexcept {
  for (int i = 0; i < kNumModSources; ++i)
    if (name == kSources[i].name) {
      out = static_cast<ModSource>(i);
      return true;
    }
  return false;
}

const char* modCurveName(ModCurve c) noexcept { return kCurveNames[static_cast<int>(c)]; }

const char* modInactiveText(ModInactive r) noexcept {
  switch (r) {
    case ModInactive::Active: return "active";
    case ModInactive::Disabled: return "desactivee";
    case ModInactive::NoSource: return "aucune source";
    case ModInactive::DestinationNotModulatable: return "destination non modulable";
    case ModInactive::AudioRateUnsupported: return "audio-rate non supporte par la destination";
    case ModInactive::AudioRateBudget: return "budget audio-rate (16) atteint";
    case ModInactive::FollowerCycle: return "suiveur : destination en amont du point mesure (cycle audio)";
  }
  return "?";
}

bool ModRoute::operator==(const ModRoute& o) const noexcept { return id == o.id && sameConnection(o); }

bool ModRoute::sameConnection(const ModRoute& o) const noexcept {
  if (!(source == o.source && dest == o.dest && depth == o.depth && curve == o.curve && polarity == o.polarity &&
        invert == o.invert && offset == o.offset && via == o.via && viaAmount == o.viaAmount && smoothMs == o.smoothMs &&
        rate == o.rate && enabled == o.enabled && locked == o.locked && numPoints == o.numPoints))
    return false;
  for (int i = 0; i < numPoints; ++i)
    if (px[static_cast<size_t>(i)] != o.px[static_cast<size_t>(i)] || py[static_cast<size_t>(i)] != o.py[static_cast<size_t>(i)])
      return false;
  return true;
}

bool paramModulatable(ParamId id) noexcept {
  if (id == ParamId::Count) return false;
  const auto& s = paramSpec(id);
  return s.type == ParamType::Float && (s.rates & (kRateControl | kRateAudio)) != 0;
}

bool validateModMatrix(const ModMatrixState& m, std::string& error) {
  if (m.routes.size() > static_cast<size_t>(kMaxModRoutes)) {
    error = "matrice : plus de 64 connexions";
    return false;
  }
  for (size_t i = 0; i < m.routes.size(); ++i) {
    const auto& r = m.routes[i];
    if (r.id == 0 || r.id >= m.nextId) {
      error = "matrice : identifiant hors plage";
      return false;
    }
    for (size_t j = 0; j < i; ++j)
      if (m.routes[j].id == r.id) {
        error = "matrice : identifiant duplique";
        return false;
      }
    if (static_cast<int>(r.source) >= kNumModSources || static_cast<int>(r.via) >= kNumModSources ||
        static_cast<int>(r.curve) >= static_cast<int>(ModCurve::Count)) {
      error = "matrice : source, via ou courbe invalide";
      return false;
    }
    if (static_cast<int>(r.dest) < 0 || static_cast<int>(r.dest) >= kNumParams) {
      error = "matrice : destination invalide";
      return false;
    }
    if (!finiteIn(r.depth, -1.0f, 1.0f) || !finiteIn(r.offset, -1.0f, 1.0f) || !finiteIn(r.viaAmount, 0.0f, 1.0f) ||
        !finiteIn(r.smoothMs, 0.0f, 1000.0f)) {
      error = "matrice : valeur hors plage ou non finie";
      return false;
    }
    if (r.numPoints < 2 || r.numPoints > kMaxCurvePoints) {
      error = "matrice : courbe Custom (2 a 8 points)";
      return false;
    }
    for (int k = 0; k < r.numPoints; ++k) {
      const float x = r.px[static_cast<size_t>(k)], y = r.py[static_cast<size_t>(k)];
      if (!finiteIn(x, 0.0f, 1.0f) || !finiteIn(y, -1.0f, 1.0f) || (k > 0 && !(x > r.px[static_cast<size_t>(k - 1)]))) {
        error = "matrice : points de courbe invalides";
        return false;
      }
    }
  }
  if (!macroGraphAcyclic(m)) {
    error = "matrice : cycle entre macros refuse";
    return false;
  }
  return true;
}

std::vector<ModInactive> modRouteStatus(const ModMatrixState& m) {
  std::vector<ModInactive> st;
  st.reserve(m.routes.size());
  int audio = 0;
  for (const auto& r : m.routes) {
    ModInactive s = ModInactive::Active;
    if (!r.enabled)
      s = ModInactive::Disabled;
    else if (r.source == ModSource::None)
      s = ModInactive::NoSource;
    else if (!paramModulatable(r.dest))
      s = ModInactive::DestinationNotModulatable;
    else if (!followerAllowed(r.source, r.dest) || (r.via != ModSource::None && !followerAllowed(r.via, r.dest)))
      s = ModInactive::FollowerCycle;
    else if (r.rate == ModRate::Audio && !(paramSpec(r.dest).rates & kRateAudio))
      s = ModInactive::AudioRateUnsupported;
    else if (r.rate == ModRate::Audio && audio >= kMaxAudioRateRoutes)
      s = ModInactive::AudioRateBudget;
    if (s == ModInactive::Active && r.rate == ModRate::Audio) ++audio;
    st.push_back(s);
  }
  return st;
}

ModTxResult modAdd(ModMatrixState& m, ModRoute r) {
  ModTxResult res;
  ModMatrixState w = m;
  r.id = w.nextId++;
  w.routes.push_back(r);
  if (!validateModMatrix(w, res.error)) return res;
  m = std::move(w);
  res.ok = true;
  res.id = r.id;
  return res;
}

ModTxResult modUpdate(ModMatrixState& m, const ModRoute& r) {
  ModTxResult res;
  ModMatrixState w = m;
  auto it = std::find_if(w.routes.begin(), w.routes.end(), [&](const ModRoute& x) { return x.id == r.id; });
  if (it == w.routes.end()) {
    res.error = "matrice : identifiant inconnu";
    return res;
  }
  *it = r;
  if (!validateModMatrix(w, res.error)) return res;
  m = std::move(w);
  res.ok = true;
  res.id = r.id;
  return res;
}

ModTxResult modRemove(ModMatrixState& m, std::uint32_t id) {
  ModTxResult res;
  auto it = std::find_if(m.routes.begin(), m.routes.end(), [&](const ModRoute& x) { return x.id == id; });
  if (it == m.routes.end()) {
    res.error = "matrice : identifiant inconnu";
    return res;
  }
  m.routes.erase(it);
  res.ok = true;
  res.id = id;
  return res;
}

ModTxResult modMerge(ModMatrixState& m, const ModMatrixState& other, bool replace) {
  ModTxResult res;
  ModMatrixState w = replace ? ModMatrixState{{}, m.nextId} : m;
  for (const auto& r : other.routes) {
    bool dup = false;
    for (const auto& x : w.routes) {
      if (x.sameConnection(r)) {
        dup = true;  // doublon exact ignoré
        break;
      }
    }
    if (dup) continue;
    ModRoute n = r;
    n.id = w.nextId++;  // les identifiants entrants ne sont jamais réutilisés tels quels : pas de conflit silencieux
    w.routes.push_back(n);
  }
  if (!validateModMatrix(w, res.error)) return res;  // capacité/cycle : refus complet
  m = std::move(w);
  res.ok = true;
  return res;
}

float modCurveValue(ModCurve c, int numPoints, const float* px, const float* py, float x) noexcept {
  x = std::min(std::max(x, 0.0f), 1.0f);
  switch (c) {
    case ModCurve::Linear: return x;
    case ModCurve::Exponential: return x * x;
    case ModCurve::Logarithmic: return 1.0f - (1.0f - x) * (1.0f - x);
    case ModCurve::SCurve: return x * x * (3.0f - 2.0f * x);
    case ModCurve::Stepped: return std::floor(x * 8.0f) / 8.0f;
    case ModCurve::Fold: {
      const float u = 2.0f * x;
      return u <= 1.0f ? u : 2.0f - u;
    }
    case ModCurve::Custom: {
      if (x <= px[0]) return py[0];
      for (int k = 1; k < numPoints; ++k)
        if (x <= px[k]) {
          const float t = (x - px[k - 1]) / (px[k] - px[k - 1]);
          return py[k - 1] + t * (py[k] - py[k - 1]);
        }
      return py[numPoints - 1];
    }
    case ModCurve::Count: break;
  }
  return x;
}

float evalModCurve(const ModRoute& r, float x) noexcept {
  return modCurveValue(r.curve, r.numPoints, r.px.data(), r.py.data(), x);
}

}  // namespace sbd
