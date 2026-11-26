#include "PresetOps.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

#include "../dsp/Rng.h"

namespace sbd {

namespace {
bool startsWith(const char* s, const char* p) { return std::strncmp(s, p, std::strlen(p)) == 0; }
double profileScale(RandomProfile p) noexcept {
  switch (p) {
    case RandomProfile::Controlled: return 0.3;
    case RandomProfile::Industrial: return 0.6;
    default: return 1.0;
  }
}
bool logDomain(const ParamSpec& s) noexcept { return (s.domain == Domain::Oct || s.domain == Domain::Time) && s.minValue > 0.0f; }
// position normalisée [0,1] dans le domaine du paramètre (logarithmique pour fréquences/durées)
double toNorm(const ParamSpec& s, double v) noexcept {
  if (logDomain(s)) return std::log(v / s.minValue) / std::log(s.maxValue / s.minValue);
  return (v - s.minValue) / (s.maxValue - s.minValue);
}
double fromNorm(const ParamSpec& s, double n) noexcept {
  n = std::min(1.0, std::max(0.0, n));
  if (logDomain(s)) return s.minValue * std::pow(s.maxValue / s.minValue, n);
  return s.minValue + n * (s.maxValue - s.minValue);
}
bool eligible(ParamId id, const RandomOptions& opt) {
  const auto& s = paramSpec(id);
  if (!s.randomizable || opt.locks.count(id)) return false;
  for (int f = 0; f < static_cast<int>(RandomFamily::Count); ++f)
    if (opt.families[static_cast<size_t>(f)] && paramInFamily(id, static_cast<RandomFamily>(f))) return true;
  return false;
}
}  // namespace

const char* randomProfileName(RandomProfile p) noexcept {
  switch (p) {
    case RandomProfile::Controlled: return "Controlled";
    case RandomProfile::Industrial: return "Industrial";
    case RandomProfile::Experimental: return "Experimental";
    case RandomProfile::Custom: return "Custom";
  }
  return "?";
}

const char* randomFamilyName(RandomFamily f) noexcept {
  static const char* k[] = {"Wavetable", "FM", "CROSS", "Filters", "Destruction", "Modulation", "Envelopes", "Topology"};
  return k[static_cast<int>(f)];
}

bool paramInFamily(ParamId id, RandomFamily f) noexcept {
  const char* g = paramSpec(id).group;
  switch (f) {
    case RandomFamily::Wavetable: return std::strcmp(g, "wt") == 0;
    case RandomFamily::Fm: return startsWith(g, "op") || startsWith(g, "fm");
    case RandomFamily::Cross: return std::strcmp(g, "cross") == 0;
    case RandomFamily::Filters: return startsWith(g, "flt.");
    case RandomFamily::Destruction: return startsWith(g, "dst.");
    case RandomFamily::Modulation: return startsWith(g, "lfo") || std::strcmp(g, "random") == 0 || std::strcmp(g, "follower") == 0;
    case RandomFamily::Envelopes: return startsWith(g, "env.");
    case RandomFamily::Topology:
    case RandomFamily::Count: return false;
  }
  return false;
}

// ---------------------------------------------------------------- Randomize / Mutate
static OpResult randomOp(PluginState& state, const RandomOptions& opt, bool mutate) {
  OpResult res;
  PluginState w = state;
  const int stream = mutate ? 1 : 0;
  RngStream rng;
  rng.init(w.seed, mutate ? StreamId::Mutate : StreamId::Randomize);
  rng.seek(w.streamCounters[static_cast<size_t>(stream)]);
  const double inten = std::min(1.0, std::max(0.0, static_cast<double>(opt.intensity)));
  const double sc = opt.profile == RandomProfile::Custom ? 1.0 : profileScale(opt.profile);
  if (inten <= 0.0) {  // intensité zéro : neutre, aucun tirage
    res.ok = true;
    return res;
  }
  for (int i = 0; i < kNumParams; ++i) {
    const auto id = static_cast<ParamId>(i);
    if (!eligible(id, opt)) continue;
    const auto& s = paramSpec(i);
    float& v = w.params.v[static_cast<size_t>(i)];
    const float before = v;
    if (s.type == ParamType::Float) {
      const double cur = toNorm(s, v), def = toNorm(s, s.defaultValue);
      double target;
      if (!mutate) {
        const double lo = def - sc * def, hi = def + sc * (1.0 - def);
        target = cur + inten * (lo + rng.uniform() * (hi - lo) - cur);
      } else {
        target = cur + inten * sc * 0.15 * rng.bipolar();
      }
      v = static_cast<float>(fromNorm(s, target));
    } else {
      const double p = mutate ? inten * sc * 0.1 : inten * sc;
      const double draw = rng.uniform(), pick = rng.uniform();
      if (draw < p) v = static_cast<float>(std::floor(s.minValue + pick * (s.maxValue - s.minValue + 1.0)));
    }
    v = sanitize(id, v);
    if (v != before) ++res.changed;
  }
  if (!mutate && opt.families[static_cast<size_t>(RandomFamily::Topology)]) {
    RouteTable t;
    std::string err;
    if (t.assign(w.routes, w.nextRouteId, err)) {
      std::vector<RouteTable::Op> ops = fmTopologyOps(t, static_cast<int>(rng.nextU32() % kNumFmTopologies));
      RouteTable probe = t;
      if (probe.apply(ops).ok) {
        // routes CROSS aléatoires (0..3), classification B sticky par la transaction
        const int nc = static_cast<int>(rng.nextU32() % 4);
        for (int k = 0; k < nc; ++k) {
          const int slot = 4 + static_cast<int>(rng.nextU32() % (kMaxRoutes - 4));
          const auto& si = routeSlotInfo(slot);
          if (si.kind == RouteKind::Internal || probe.findPair(si.src, si.dst)) continue;
          probe.apply({RouteTable::Op::add(si.src, si.dst)});
        }
        w.routes = probe.routes();
        w.nextRouteId = probe.nextRouteId();
        ++res.changed;
      }
    }
  }
  w.streamCounters[static_cast<size_t>(stream)] = rng.counter();
  std::string err;
  if (!validateState(w, err)) {  // échec transactionnel : état et compteur inchangés
    res.error = err;
    res.changed = 0;
    return res;
  }
  state = std::move(w);
  res.ok = true;
  return res;
}

OpResult randomizeState(PluginState& state, const RandomOptions& opt) { return randomOp(state, opt, false); }
OpResult mutateState(PluginState& state, const RandomOptions& opt) { return randomOp(state, opt, true); }

// ---------------------------------------------------------------- Evolve
bool Evolver::whitelisted(ParamId id) noexcept {
  const auto& s = paramSpec(id);
  return s.type == ParamType::Float && s.randomizable && (s.rates & (kRateControl | kRateAudio)) != 0;
}

void Evolver::start(const PluginState& anchor, const RandomOptions& opt, EvolveMode mode, float rateHz) {
  opt_ = opt;
  mode_ = mode;
  rate_ = std::max(0.01f, rateHz);
  seed_ = anchor.seed;
  counter_ = anchor.streamCounters[2];
  for (int i = 0; i < kNumParams; ++i) {
    const auto id = static_cast<ParamId>(i);
    active_[static_cast<size_t>(i)] = whitelisted(id) && eligible(id, opt);
  }
  running_ = true;
}

void Evolver::step(double dt) {
  if (!running_) return;
  RngStream rng;
  rng.init(seed_, StreamId::Evolve);
  rng.seek(counter_);
  const double sc = opt_.profile == RandomProfile::Custom ? 1.0 : profileScale(opt_.profile);
  const double bound = std::min(1.0, std::max(0.0, static_cast<double>(opt_.intensity))) * sc * 0.25;
  const double sigma = bound * std::sqrt(std::max(dt, 0.0) * rate_);
  for (int i = 0; i < kNumParams; ++i) {
    const auto ii = static_cast<size_t>(i);
    if (!active_[ii]) continue;
    double o = off_[ii];
    if (mode_ == EvolveMode::Anchored)
      o += -rate_ * dt * o + sigma * rng.bipolar();  // rappel vers l'ancre (processus d'Ornstein-Uhlenbeck)
    else
      o += sigma * rng.bipolar();  // dérive bornée
    off_[ii] = static_cast<float>(std::min(bound, std::max(-bound, o)));
  }
  counter_ = rng.counter();
}

float Evolver::apply(ParamId id, float base) const noexcept {
  const auto i = static_cast<size_t>(idx(id));
  if (!running_ || !active_[i] || off_[i] == 0.0f) return base;
  const auto& s = paramSpec(id);
  return static_cast<float>(fromNorm(s, toNorm(s, base) + off_[i]));
}

void Evolver::reset() noexcept { off_.fill(0.0f); }

bool Evolver::commit(PluginState& state) const {
  PluginState w = state;
  for (int i = 0; i < kNumParams; ++i) w.params.v[static_cast<size_t>(i)] = sanitize(static_cast<ParamId>(i), apply(static_cast<ParamId>(i), w.params.v[static_cast<size_t>(i)]));
  w.streamCounters[2] = counter_;
  std::string err;
  if (!validateState(w, err)) return false;
  state = std::move(w);
  return true;
}

// ---------------------------------------------------------------- Undo / Redo
void UndoStack::push(const PluginState& before, const PluginState& after, const std::string& cause) {
  if (before == after) return;
  while (static_cast<int>(stack_.size()) > cursor_) stack_.pop_back();  // une nouvelle transaction efface le Redo
  Transaction t;
  t.id = next_++;
  t.cause = cause;
  t.before = before;
  t.after = after;
  stack_.push_back(std::move(t));
  while (static_cast<int>(stack_.size()) > kCapacity || (memoryEstimate() > kMemoryBudget && stack_.size() > 1)) stack_.pop_front();
  cursor_ = static_cast<int>(stack_.size());
}

const PluginState* UndoStack::undo() noexcept {
  if (!canUndo()) return nullptr;
  --cursor_;
  return &stack_[static_cast<size_t>(cursor_)].before;
}

const PluginState* UndoStack::redo() noexcept {
  if (!canRedo()) return nullptr;
  ++cursor_;
  return &stack_[static_cast<size_t>(cursor_ - 1)].after;
}

std::string UndoStack::undoCause() const { return canUndo() ? stack_[static_cast<size_t>(cursor_ - 1)].cause : std::string(); }
std::string UndoStack::redoCause() const { return canRedo() ? stack_[static_cast<size_t>(cursor_)].cause : std::string(); }

std::size_t UndoStack::memoryEstimate() const noexcept {
  std::size_t b = 0;
  for (const auto& t : stack_)
    for (const PluginState* s : {&t.before, &t.after})
      b += sizeof(PluginState) + s->routes.size() * sizeof(Route) + s->mod.routes.size() * sizeof(ModRoute) + s->bank.size();
  return b;
}

// ---------------------------------------------------------------- presets usine
namespace {
PluginState basePreset(const char* bank) {
  PluginState s = defaultState();
  s.bank = bank;
  return s;
}
void route(PluginState& s, std::initializer_list<std::pair<int, int>> r) {
  RouteTable t;
  std::string err;
  t.assign(s.routes, s.nextRouteId, err);
  std::vector<RouteTable::Op> ops;
  for (auto p : r) ops.push_back(RouteTable::Op::add(p.first, p.second));
  t.apply(ops);
  s.routes = t.routes();
  s.nextRouteId = t.nextRouteId();
}
void mod(PluginState& s, ModSource src, ParamId dst, float depth, ModRate rate = ModRate::Control) {
  ModRoute r;
  r.source = src;
  r.dest = dst;
  r.depth = depth;
  r.rate = rate;
  modAdd(s.mod, r);
}
}  // namespace

std::vector<std::pair<PresetInfo, PluginState>> factoryPresets() {
  std::vector<std::pair<PresetInfo, PluginState>> v;
  auto add = [&v](const char* name, std::vector<std::string> tags, PluginState s) {
    PresetInfo i;
    i.name = name;
    i.tags = std::move(tags);
    i.factory = true;
    v.emplace_back(i, std::move(s));
  };
  add("Init (0.1.0-b5)", {"init"}, defaultState());
  {
    PluginState s = basePreset("factory:metal");
    auto& p = s.params;
    p[ParamId::WtX] = 0.2f;
    p[ParamId::WtY] = 0.4f;
    p[ParamId::Op1Level] = 0.35f;
    p[ParamId::Op2Ratio] = 3.5f;
    p[ParamId::DepthOp2Op1] = 2.0f;
    p[ParamId::DepthWtOp1] = 1.2f;
    p[ParamId::DepthOp1Wt] = 0.9f;
    p[ParamId::DstGl1Type] = 1.0f;
    p[ParamId::DstGl1N1Drive] = 9.0f;
    p[ParamId::DstGl1N1Fold] = 0.6f;
    p[ParamId::DstGl1Mix] = 0.6f;
    p[ParamId::Lfo1Rate] = 0.2f;
    mod(s, ModSource::Lfo1, ParamId::WtX, 0.3f);
    s.macroNames[0] = "Metal X";
    mod(s, ModSource::Macro1, ParamId::WtY, 0.6f);
    add("Cross Metal", {"cross", "metal"}, std::move(s));
  }
  {
    PluginState s = basePreset("factory:saw");
    auto& p = s.params;
    p[ParamId::WtLevel] = 0.0f;
    p[ParamId::Op1Level] = 0.45f;
    p[ParamId::Op3Level] = 0.25f;
    p[ParamId::Op2Ratio] = 3.5f;
    p[ParamId::Op4Ratio] = 7.0f;
    p[ParamId::DepthOp2Op1] = 1.6f;
    p[ParamId::DepthOp4Op3] = 2.2f;
    p[ParamId::AmpDecay] = 2.0f;
    p[ParamId::AmpSustain] = 0.0f;
    p[ParamId::AmpRelease] = 2.5f;
    p[ParamId::FltGlType] = 4.0f;
    p[ParamId::FltGlModal] = 1.0f;
    p[ParamId::FltGlCutoff] = 220.0f;
    p[ParamId::FltGlKey] = 1.0f;
    p[ParamId::FltGlMix] = 0.4f;
    s.routes.clear();
    s.nextRouteId = 1;
    route(s, {{kNodeOP2, kNodeOP1}, {kNodeOP4, kNodeOP3}});
    add("FM Bell Bank", {"fm", "bell"}, std::move(s));
  }
  {
    PluginState s = basePreset("factory:saw");
    auto& p = s.params;
    p[ParamId::WtZAType] = 1.0f;
    p[ParamId::WtZ] = 0.35f;
    p[ParamId::AmpAttack] = 1.5f;
    p[ParamId::AmpRelease] = 6.0f;
    p[ParamId::FltWtType] = 2.0f;
    p[ParamId::FltWtCutoff] = 55.0f;
    p[ParamId::FltWtKey] = 1.0f;
    p[ParamId::FltWtCombFb] = 0.85f;
    p[ParamId::FltWtMix] = 0.5f;
    p[ParamId::DstGl1Type] = 6.0f;
    p[ParamId::DstGl1D2Size] = 400.0f;
    p[ParamId::DstGl1D2Repeat] = 0.3f;
    p[ParamId::DstGl1Mix] = 0.35f;
    p[ParamId::Lfo2Rate] = 0.05f;
    mod(s, ModSource::Lfo2, ParamId::WtZ, 0.2f);
    add("Doom Fold Comb", {"doom", "drone"}, std::move(s));
  }
  {
    PluginState s = basePreset("factory:pwm");
    auto& p = s.params;
    p[ParamId::DstWt1Type] = 2.0f;
    p[ParamId::DstWt1D1Bits] = 6.0f;
    p[ParamId::DstWt1D1Rate] = 6.0f;
    p[ParamId::DstWt1D1Jitter] = 0.4f;
    p[ParamId::DstGl1Type] = 6.0f;
    p[ParamId::DstGl1D2Size] = 90.0f;
    p[ParamId::DstGl1D2Repeat] = 0.6f;
    p[ParamId::DstGl1D2Speed] = -1.0f;
    p[ParamId::Lfo1Shape] = 5.0f;
    p[ParamId::Lfo1Rate] = 6.0f;
    mod(s, ModSource::Lfo1, ParamId::WtX, 0.5f);
    add("Glitch Lab", {"glitch"}, std::move(s));
  }
  {
    PluginState s = basePreset("factory:formant");
    auto& p = s.params;
    p[ParamId::AmpAttack] = 0.8f;
    p[ParamId::AmpRelease] = 3.0f;
    p[ParamId::WtZBType] = 2.0f;
    p[ParamId::WtZ] = 0.25f;
    p[ParamId::Lfo1Rate] = 0.15f;
    p[ParamId::DstGl2Type] = 8.0f;
    p[ParamId::DstGl2S2Decay] = 0.7f;
    p[ParamId::DstGl2S2Blur] = 0.3f;
    p[ParamId::DstGl2Mix] = 0.4f;
    mod(s, ModSource::Lfo1, ParamId::WtY, 0.5f);
    add("Spectral Formants", {"pad", "spectral"}, std::move(s));
  }
  {
    PluginState s = basePreset("factory:basic");
    auto& p = s.params;
    p[ParamId::WtX] = 0.66f;
    p[ParamId::DstGl1Type] = 4.0f;
    p[ParamId::DstGl1S1Freq] = 73.0f;
    p[ParamId::DstGl1Mix] = 0.7f;
    p[ParamId::DstGl2Type] = 3.0f;
    p[ParamId::DstGl2Y1Transient] = 0.8f;
    p[ParamId::DstGl2Y1Thresh] = -18.0f;
    add("Ring Destroyer", {"ring", "aggressive"}, std::move(s));
  }
  return v;
}

// ---------------------------------------------------------------- bibliothèque
std::string PresetLibrary::sanitizeName(const std::string& name) {
  std::string o;
  for (char c : name)
    o += (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '-' || c == '_') ? c : '_';
  if (o.empty()) o = "preset";
  return o.substr(0, 64);
}

namespace {
std::map<std::string, std::pair<std::string, bool>> readMeta(const std::string& dir) {
  std::map<std::string, std::pair<std::string, bool>> m;
  std::ifstream in(dir + "/library.txt");
  std::string line;
  while (std::getline(in, line)) {
    std::istringstream ls(line);
    std::string file, tags, fav;
    if (!std::getline(ls, file, '\t') || !std::getline(ls, tags, '\t')) continue;
    std::getline(ls, fav, '\t');
    m[file] = {tags, fav == "1"};
  }
  return m;
}
void writeMeta(const std::string& dir, const std::map<std::string, std::pair<std::string, bool>>& m) {
  std::ofstream out(dir + "/library.txt", std::ios::trunc);
  for (const auto& kv : m) out << kv.first << '\t' << kv.second.first << '\t' << (kv.second.second ? "1" : "0") << '\n';
}
std::vector<std::string> splitTags(const std::string& t) {
  std::vector<std::string> v;
  std::stringstream ss(t);
  std::string x;
  while (std::getline(ss, x, ','))
    if (!x.empty()) v.push_back(x);
  return v;
}
}  // namespace

std::vector<PresetInfo> PresetLibrary::list() const {
  std::vector<PresetInfo> v;
  for (auto& f : factoryPresets()) v.push_back(f.first);
  std::error_code ec;
  if (dir_.empty() || !std::filesystem::is_directory(dir_, ec)) return v;
  const auto meta = readMeta(dir_);
  std::vector<PresetInfo> user;
  for (const auto& e : std::filesystem::directory_iterator(dir_, ec)) {
    if (e.path().extension() != ".sdpreset") continue;
    PresetInfo i;
    i.path = e.path().string();
    i.name = e.path().stem().string();
    const auto it = meta.find(e.path().filename().string());
    if (it != meta.end()) {
      i.tags = splitTags(it->second.first);
      i.favorite = it->second.second;
    }
    user.push_back(i);
  }
  std::sort(user.begin(), user.end(), [](const PresetInfo& a, const PresetInfo& b) { return a.name < b.name; });
  v.insert(v.end(), user.begin(), user.end());
  return v;
}

bool PresetLibrary::save(const std::string& name, const PluginState& state, const std::vector<std::string>& tags, std::string& error) const {
  std::error_code ec;
  std::filesystem::create_directories(dir_, ec);
  const std::string file = sanitizeName(name) + ".sdpreset";
  const std::string path = dir_ + "/" + file, tmp = path + ".tmp";
  {
    std::ofstream out(tmp, std::ios::trunc | std::ios::binary);
    if (!out) {
      error = "ecriture impossible : " + path;
      return false;
    }
    out << encodeState(state, "preset", StateScope::Preset);
    if (!out) {
      error = "ecriture interrompue";
      return false;
    }
  }
  std::filesystem::rename(tmp, path, ec);  // sauvegarde résistante aux interruptions (fichier temporaire + renommage)
  if (ec) {
    error = "renommage impossible : " + ec.message();
    return false;
  }
  auto meta = readMeta(dir_);
  std::string t;
  for (size_t k = 0; k < tags.size(); ++k) t += (k ? "," : "") + tags[k];
  meta[file] = {t, meta.count(file) ? meta[file].second : false};
  writeMeta(dir_, meta);
  return true;
}

bool PresetLibrary::load(const PresetInfo& info, PluginState& out, std::string& error) const {
  if (info.factory) {
    for (auto& f : factoryPresets())
      if (f.first.name == info.name) {
        out = f.second;
        out.hasInstance = false;
        return true;
      }
    error = "preset usine inconnu";
    return false;
  }
  std::ifstream in(info.path, std::ios::binary);
  if (!in) {
    error = "fichier introuvable : " + info.path;
    return false;
  }
  std::stringstream ss;
  ss << in.rdbuf();
  if (ss.str().size() > (4u << 20)) {  // import borné (fichier non fiable)
    error = "fichier trop volumineux";
    return false;
  }
  return decodeState(ss.str(), out, error);
}

bool PresetLibrary::setFavorite(const PresetInfo& info, bool fav, std::string& error) const {
  if (info.factory) {
    error = "preset usine : favori non persistant";
    return false;
  }
  auto meta = readMeta(dir_);
  const std::string file = std::filesystem::path(info.path).filename().string();
  meta[file].second = fav;
  writeMeta(dir_, meta);
  return true;
}

}  // namespace sbd
