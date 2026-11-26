#include "PluginProcessor.h"

#include <fstream>
#include <sstream>

#include "BuildInfo.h"
#include "ui/MainEditor.h"

namespace sbd {

namespace {
const char* kWriter = SUBDESTRUCKT_BUILD_ID " " SUBDESTRUCKT_COMMIT;

std::unique_ptr<juce::RangedAudioParameter> makeParameter(int i) {
  const auto& s = paramSpec(i);
  const juce::ParameterID pid{s.stableId, s.version};
  switch (s.type) {
    case ParamType::Choice: {
      juce::StringArray labels;
      for (int k = 0; k < choiceCount(static_cast<ParamId>(i)); ++k) labels.add(choiceLabel(static_cast<ParamId>(i), k));
      return std::make_unique<juce::AudioParameterChoice>(
          pid, s.name, labels, static_cast<int>(s.defaultValue),
          juce::AudioParameterChoiceAttributes().withAutomatable(s.automatable));
    }
    case ParamType::Bool:
      return std::make_unique<juce::AudioParameterBool>(pid, s.name, s.defaultValue >= 0.5f,
                                                        juce::AudioParameterBoolAttributes().withAutomatable(s.automatable));
    case ParamType::Int:
      return std::make_unique<juce::AudioParameterInt>(
          pid, s.name, static_cast<int>(s.minValue), static_cast<int>(s.maxValue), static_cast<int>(s.defaultValue),
          juce::AudioParameterIntAttributes().withAutomatable(s.automatable).withLabel(s.unit));
    case ParamType::Float: break;
  }
  juce::NormalisableRange<float> range(s.minValue, s.maxValue);
  // Paramètres v1 : correspondance normalisée linéaire de 0.1.0-b5 conservée (automation existante). Nouveaux
  // paramètres en fréquence/durée : correspondance logarithmique centrée sur la moyenne géométrique.
  if (s.version > 1 && (s.domain == Domain::Oct || s.domain == Domain::Time) && s.minValue > 0.0f)
    range.setSkewForCentre(std::sqrt(s.minValue * s.maxValue));
  return std::make_unique<juce::AudioParameterFloat>(
      pid, s.name, range, s.defaultValue,
      juce::AudioParameterFloatAttributes().withLabel(s.unit).withAutomatable(s.automatable).withStringFromValueFunction(
          [i](float v, int) { return juce::String(formatValue(static_cast<ParamId>(i), v)); }));
}

float readTyped(const juce::RangedAudioParameter* p, ParamType t) noexcept {
  switch (t) {
    case ParamType::Float: return static_cast<const juce::AudioParameterFloat*>(p)->get();
    case ParamType::Choice: return static_cast<float>(static_cast<const juce::AudioParameterChoice*>(p)->getIndex());
    case ParamType::Bool: return static_cast<const juce::AudioParameterBool*>(p)->get() ? 1.0f : 0.0f;
    case ParamType::Int: return static_cast<float>(static_cast<const juce::AudioParameterInt*>(p)->get());
  }
  return 0.0f;
}
}  // namespace

// ---------------------------------------------------------------- ParameterBridge
float ParameterBridge::base(int index) const noexcept {
  const auto i = static_cast<size_t>(index);
  const float hostValue = readTyped(params_[i], paramSpec(index).type);
  // Valeur exacte restaurée conservée tant que l'hôte n'a pas réellement modifié le paramètre : un écho de la même
  // valeur normalisée (aller-retour float/double de l'hôte, correspondance logarithmique) n'est pas une modification.
  if (hasExact_[i].load(std::memory_order_acquire) &&
      std::fabs(params_[i]->getValue() - hostValueAtRestore_[i].load(std::memory_order_relaxed)) <= 1e-6f)
    return exact_[i].load(std::memory_order_relaxed);
  return hostValue;
}

void ParameterBridge::setExact(int index, float value) {
  const auto i = static_cast<size_t>(index);
  auto* p = params_[i];
  hasExact_[i].store(false, std::memory_order_release);
  p->setValueNotifyingHost(p->convertTo0to1(value));
  exact_[i].store(value, std::memory_order_relaxed);
  hostValueAtRestore_[i].store(p->getValue(), std::memory_order_relaxed);  // valeur normalisée hôte
  hasExact_[i].store(true, std::memory_order_release);
}

ParamValues ParameterBridge::values() const noexcept {
  ParamValues v;
  for (int i = 0; i < kNumParams; ++i) v.v[static_cast<size_t>(i)] = base(i);
  return v;
}

// ---------------------------------------------------------------- MIDI Learn (préférence utilisateur globale)
std::string MidiLearnMap::defaultPath() {
  return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
      .getChildFile("Labtechno")
      .getChildFile("SUBDESTRUCKT")
      .getChildFile("midi_learn.txt")
      .getFullPathName()
      .toStdString();
}

bool MidiLearnMap::load(const std::string& path) {
  std::ifstream in(path);
  if (!in) return false;
  MidiLearnMap m;
  std::string line;
  while (std::getline(in, line)) {
    std::istringstream ls(line);
    int cc = -1;
    std::string sid;
    if (!(ls >> cc >> sid) || cc < 0 || cc > 127) continue;
    ParamId id;
    if (findParam(sid, id)) m.ccToParam[static_cast<size_t>(cc)] = static_cast<std::int16_t>(idx(id));
  }
  *this = m;
  return true;
}

bool MidiLearnMap::save(const std::string& path) const {
  juce::File(path).getParentDirectory().createDirectory();
  std::ofstream out(path, std::ios::trunc);
  if (!out) return false;
  out << "# SUBDESTRUCKT MIDI Learn (global, hors presets) : <cc> <StableID>\n";
  for (int cc = 0; cc < 128; ++cc)
    if (ccToParam[static_cast<size_t>(cc)] >= 0) out << cc << " " << paramSpec(ccToParam[static_cast<size_t>(cc)]).stableId << "\n";
  return static_cast<bool>(out);
}

// ---------------------------------------------------------------- processeur
static std::string presetDir() {
  return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
      .getChildFile("Labtechno").getChildFile("SUBDESTRUCKT").getChildFile("Presets").getFullPathName().toStdString();
}

SubdestrucktProcessor::SubdestrucktProcessor()
    : juce::AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      engine_(FactoryBanks::get("factory:saw")),
      library_(presetDir()) {
  gestures_.owner = this;
  for (int i = 0; i < kNumParams; ++i) {
    auto param = makeParameter(i);
    bridge_.attach(i, param.get());
    param->addListener(&gestures_);
    addParameter(param.release());
  }
  const PluginState initial = defaultState();  // preset minimal
  std::string err;
  routes_.assign(initial.routes, initial.nextRouteId, err);
  engine_.setTopologyImmediate(routes_.compile());
  learn_.load(MidiLearnMap::defaultPath());
  for (int cc = 0; cc < 128; ++cc) learnRt_[static_cast<size_t>(cc)].store(learn_.ccToParam[static_cast<size_t>(cc)]);
  startTimerHz(30);
}

SubdestrucktProcessor::~SubdestrucktProcessor() {
  stopTimer();
  for (int i = 0; i < kNumParams; ++i) bridge_.param(i)->removeListener(&gestures_);
}

bool SubdestrucktProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() && layouts.getMainInputChannelSet().isDisabled();
}

void SubdestrucktProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  std::lock_guard<std::mutex> lock(controlMutex_);
  sampleRate_ = sampleRate;
  blockSize_ = samplesPerBlock;
  engine_.setParameters(bridge_.values());
  engine_.setSeed(seed_);
  engine_.setLfoCurves(lfoCurves_);
  if (auto b = resolveBank(bank_)) engine_.setBankImmediate(b);
  engine_.prepare(sampleRate, samplesPerBlock, instance_.quality);
  CompiledTopology c = routes_.compile();
  c.serial = ++serial_;
  engine_.setTopologyImmediate(c);
  auto m = compileModMatrix(mod_);
  m->serial = ++modSerial_;
  engine_.setModMatrixImmediate(*m);
  engine_.setDrone(instance_.drone);
  // Latence déclarée à l'hôte (PDC) : décimation + chaînes ; ne change qu'aux transitions structurelles.
  setLatencySamples(engine_.setup().latencySamples);
}

void SubdestrucktProcessor::reprepareLocked() {
  if (!(sampleRate_ > 0.0)) return;
  suspendProcessing(true);  // transition contrôlée hors RT : aucun bloc pendant la reconstruction
  engine_.setParameters(bridge_.values());
  engine_.setSeed(seed_);
  engine_.setLfoCurves(lfoCurves_);
  if (auto b = resolveBank(bank_)) engine_.setBankImmediate(b);
  engine_.prepare(sampleRate_, blockSize_, instance_.quality);
  CompiledTopology c = routes_.compile();
  c.serial = ++serial_;
  engine_.setTopologyImmediate(c);
  auto m = compileModMatrix(mod_);
  m->serial = ++modSerial_;
  engine_.setModMatrixImmediate(*m);
  engine_.setDrone(instance_.drone);
  setLatencySamples(engine_.setup().latencySamples);
  suspendProcessing(false);
}

void SubdestrucktProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;
  const auto t0 = juce::Time::getHighResolutionTicks();
  const int n = buffer.getNumSamples();
  if (buffer.getNumChannels() < 2 || n <= 0) {
    buffer.clear();
    return;
  }
  ParamValues pv = bridge_.values();
  if (evolveActive_.load(std::memory_order_acquire))
    for (int i = 0; i < kNumParams; ++i)
      if (evolvedOn_[static_cast<size_t>(i)].load(std::memory_order_relaxed)) pv.v[static_cast<size_t>(i)] = evolved_[static_cast<size_t>(i)].load(std::memory_order_relaxed);
  engine_.setParameters(pv);
  TransportInfo tr;
  if (auto* ph = getPlayHead()) {
    if (const auto pos = ph->getPosition()) {
      tr.valid = true;
      tr.playing = pos->getIsPlaying();
      if (const auto bpm = pos->getBpm()) tr.bpm = *bpm;
      if (const auto ppq = pos->getPpqPosition()) tr.ppq = *ppq;
    }
  }
  engine_.setTransport(tr);

  // MIDI -> événements du cœur, décodage des octets bruts (aucun objet alloué). Omni : tous les canaux.
  int count = 0;
  if (panicRequested_.exchange(false)) midi_[static_cast<size_t>(count++)] = MidiEvent::allSoundOff(0);
  for (const auto meta : midi) {
    if (count >= static_cast<int>(midi_.size())) break;
    const juce::uint8* d = meta.data;
    if (meta.numBytes < 2) continue;
    const int status = d[0] & 0xF0;
    const int pos = juce::jlimit(0, n - 1, meta.samplePosition);
    if (status == 0xD0) {
      midi_[static_cast<size_t>(count++)] = MidiEvent::channelPressure(pos, d[1] & 0x7F);
      continue;
    }
    if (meta.numBytes < 3) continue;
    if (status == 0x90) {
      midi_[static_cast<size_t>(count++)] =
          d[2] > 0 ? MidiEvent::noteOn(pos, d[1] & 0x7F, d[2] & 0x7F) : MidiEvent::noteOff(pos, d[1] & 0x7F);
    } else if (status == 0x80) {
      midi_[static_cast<size_t>(count++)] = MidiEvent::noteOff(pos, d[1] & 0x7F);
    } else if (status == 0xE0) {
      midi_[static_cast<size_t>(count++)] = MidiEvent::pitchBend(pos, ((d[2] & 0x7F) << 7 | (d[1] & 0x7F)) - 8192);
    } else if (status == 0xB0) {
      const int cc = d[1] & 0x7F, val = d[2] & 0x7F;
      if (cc == 120) {
        midi_[static_cast<size_t>(count++)] = MidiEvent::allSoundOff(pos);
      } else if (cc == 123) {
        midi_[static_cast<size_t>(count++)] = MidiEvent::allNotesOff(pos);
      } else {
        midi_[static_cast<size_t>(count++)] = MidiEvent::controller(pos, cc, val);
        // MIDI Learn : capture et contrôle (transmis au thread message par une file sans verrou)
        const int target = learnTarget_.load(std::memory_order_relaxed);
        if (target >= 0) {
          learnRt_[static_cast<size_t>(cc)].store(static_cast<std::int16_t>(target), std::memory_order_relaxed);
          learnCaptured_.store(cc, std::memory_order_release);
          learnTarget_.store(-1, std::memory_order_relaxed);
        }
        const int mapped = learnRt_[static_cast<size_t>(cc)].load(std::memory_order_relaxed);
        if (mapped >= 0) {
          const std::uint32_t w = ccWrite_.load(std::memory_order_relaxed);
          if (w - ccRead_.load(std::memory_order_acquire) < ccFifo_.size()) {
            ccFifo_[w % ccFifo_.size()] = CcEvent{static_cast<std::int16_t>(mapped), val / 127.0f};
            ccWrite_.store(w + 1, std::memory_order_release);
          }
        }
      }
    }
  }
  engine_.process(buffer.getWritePointer(0), buffer.getWritePointer(1), n, midi_.data(), count);
  for (int ch = 2; ch < buffer.getNumChannels(); ++ch) buffer.clear(ch, 0, n);
  const double el = juce::Time::highResolutionTicksToSeconds(juce::Time::getHighResolutionTicks() - t0);
  if (sampleRate_ > 0.0) engine_.telemetry().cpuLoad.store(static_cast<float>(el * sampleRate_ / n), std::memory_order_relaxed);
}

double SubdestrucktProcessor::getTailLengthSeconds() const {
  return static_cast<double>(bridge_.base(static_cast<int>(ParamId::AmpRelease)));
}

juce::AudioProcessorEditor* SubdestrucktProcessor::createEditor() { return new ui::MainEditor(*this); }

// ---------------------------------------------------------------- état
PluginState SubdestrucktProcessor::snapshotState() const {
  PluginState s;
  s.params = bridge_.values();
  std::lock_guard<std::mutex> lock(controlMutex_);
  s.routes = routes_.routes();
  s.nextRouteId = routes_.nextRouteId();
  s.bank = bank_;
  if (bank_.rfind("user:", 0) == 0)
    if (auto b = UserBanks::get(bank_)) {  // banque utilisateur : intégrée à l'état (projet/preset autonome)
      s.bankRaw = b->raw();
      s.bankNx = b->nx();
      s.bankNy = b->ny();
      s.bankName = b->name();
    }
  s.mod = mod_;
  s.macroNames = macroNames_;
  s.lfoCurves = lfoCurves_;
  s.seed = seed_;
  s.streamCounters = streamCounters_;
  s.instance = instance_;
  return s;
}

std::string SubdestrucktProcessor::encodeCurrentState(StateScope scope) const {
  return encodeState(snapshotState(), kWriter, scope);
}

void SubdestrucktProcessor::getStateInformation(juce::MemoryBlock& destData) {
  const std::string text = encodeCurrentState(StateScope::Daw);
  destData.replaceAll(text.data(), text.size());
}

void SubdestrucktProcessor::setStateInformation(const void* data, int sizeInBytes) {
  if (data == nullptr || sizeInBytes <= 0) return;
  const std::string text(static_cast<const char*>(data), static_cast<size_t>(sizeInBytes));
  PluginState s;
  std::string err;
  if (!decodeState(text, s, err)) {  // refus complet : l'état courant est conservé
    std::lock_guard<std::mutex> lock(controlMutex_);
    lastMessage_ = "Etat refuse : " + err;
    return;
  }
  applyState(s, false);
}

bool SubdestrucktProcessor::applyState(const PluginState& state, bool fromPreset) {
  bool reprepare = false;
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    std::string err;
    if (!validateState(state, err) || !routes_.assign(state.routes, state.nextRouteId, err)) {
      lastMessage_ = "Etat refuse : " + err;
      return false;
    }
    reprepare = state.seed != seed_ || (!fromPreset && state.hasInstance && state.instance.quality != instance_.quality);
    mod_ = state.mod;
    bank_ = state.bank;
    macroNames_ = state.macroNames;
    lfoCurves_ = state.lfoCurves;
    seed_ = state.seed;
    streamCounters_ = state.streamCounters;
    if (!fromPreset && state.hasInstance) {
      instance_.quality = state.instance.quality;
      instance_.droneRestore = state.instance.droneRestore;
      // Drone : restauré seulement si l'autorisation locale enregistrée le permet ; un preset ne l'active jamais
      instance_.drone = state.instance.droneRestore && state.instance.drone;
      engine_.setDrone(instance_.drone);
    }
    lastMessage_ = state.sourceVersion < kStateVersion ? "Etat v" + std::to_string(state.sourceVersion) + " migre et charge" : "Etat charge";
    if (!state.bankRaw.empty() && !resolveBank(state.bank)) {  // banque utilisateur intégrée : reconstruite hors RT
      std::string e2;
      if (auto b = WtBank::build(state.bank, state.bankName, state.bankNx, state.bankNy, state.bankRaw, WtBank::Options{}, e2)) UserBanks::add(b);
    }
    if (!resolveBank(bank_)) lastMessage_ += " ; banque absente : " + bank_ + " (reference conservee, lecture Saw)";
    publishTopologyLocked();
    publishModLocked();
    publishBankLocked();
    publishCurvesLocked();
  }
  // notifications hôte hors verrou (les écouteurs peuvent rappeler l'API de contrôle)
  for (int i = 0; i < kNumParams; ++i) bridge_.setExact(i, state.params.v[static_cast<size_t>(i)]);
  if (reprepare) {
    std::lock_guard<std::mutex> lock(controlMutex_);
    reprepareLocked();
  }
  notifyState();
  return true;
}

bool SubdestrucktProcessor::transact(const std::function<bool(PluginState&)>& edit, const std::string& cause) {
  const PluginState before = snapshotState();
  PluginState s = before;
  if (!edit(s)) return false;
  std::string err;
  if (!validateState(s, err)) {
    std::lock_guard<std::mutex> lock(controlMutex_);
    lastMessage_ = cause + " refuse : " + err;
    return false;
  }
  s.hasInstance = false;  // une transaction d'édition ne modifie pas la préférence d'instance
  const bool ok = applyState(s, true);
  if (ok) record(before, cause);
  return ok;
}

void SubdestrucktProcessor::record(const PluginState& before, const std::string& cause) {
  if (applyingHistory_) return;
  undo_.push(before, snapshotState(), cause);
  dirty_.store(true);
}

void SubdestrucktProcessor::GestureListener::parameterGestureChanged(int, bool starting) {
  if (!juce::MessageManager::existsAndIsCurrentThread()) return;  // gestes : thread message uniquement
  auto& o = *owner;
  if (starting) {
    if (o.gestureDepth_++ == 0) o.gestureBefore_ = std::make_unique<PluginState>(o.snapshotState());
  } else if (o.gestureDepth_ > 0 && --o.gestureDepth_ == 0 && o.gestureBefore_) {
    o.record(*o.gestureBefore_, "geste");
    o.gestureBefore_.reset();
  }
}

bool SubdestrucktProcessor::undo() {
  const PluginState* s = undo_.undo();
  if (!s) return false;
  const PluginState copy = *s;
  applyingHistory_ = true;
  const bool ok = applyState(copy, true);
  applyingHistory_ = false;
  return ok;
}

bool SubdestrucktProcessor::redo() {
  const PluginState* s = undo_.redo();
  if (!s) return false;
  const PluginState copy = *s;
  applyingHistory_ = true;
  const bool ok = applyState(copy, true);
  applyingHistory_ = false;
  return ok;
}

bool SubdestrucktProcessor::canUndo() const { return undo_.canUndo(); }
bool SubdestrucktProcessor::canRedo() const { return undo_.canRedo(); }
std::string SubdestrucktProcessor::undoLabel() const { return undo_.undoCause(); }
std::string SubdestrucktProcessor::redoLabel() const { return undo_.redoCause(); }

bool SubdestrucktProcessor::randomize(const RandomOptions& o) {
  return transact([&](PluginState& s) { return randomizeState(s, o).ok; }, "Randomize");
}

bool SubdestrucktProcessor::mutate(const RandomOptions& o) {
  return transact([&](PluginState& s) { return mutateState(s, o).ok; }, "Mutate");
}

void SubdestrucktProcessor::evolveStart(const RandomOptions& o, EvolveMode m, float rateHz) {
  evolver_.start(snapshotState(), o, m, rateHz);
  evolveActive_.store(true, std::memory_order_release);
}

void SubdestrucktProcessor::evolveStop() {
  evolver_.stop();
  evolveActive_.store(false, std::memory_order_release);
  for (auto& f : evolvedOn_) f.store(false);
}

void SubdestrucktProcessor::evolveReset() {
  evolver_.reset();
  for (auto& f : evolvedOn_) f.store(false);
}

bool SubdestrucktProcessor::evolveCommit() {
  const Evolver ev = evolver_;
  const bool ok = transact([&](PluginState& s) { return ev.commit(s); }, "Evolve Commit");
  if (ok) evolveReset();
  return ok;
}

bool SubdestrucktProcessor::evolveRunning() const { return evolver_.running(); }

std::vector<PresetInfo> SubdestrucktProcessor::presets() const { return library_.list(); }

bool SubdestrucktProcessor::loadPreset(const PresetInfo& info) {
  PluginState s;
  std::string err;
  if (!library_.load(info, s, err)) {
    std::lock_guard<std::mutex> lock(controlMutex_);
    lastMessage_ = "Preset refuse : " + err;
    return false;
  }
  const PluginState before = snapshotState();
  if (!applyState(s, true)) return false;
  record(before, "preset : " + info.name);
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    presetName_ = info.name;
  }
  dirty_.store(false);
  notifyState();
  return true;
}

bool SubdestrucktProcessor::savePreset(const std::string& name, const std::vector<std::string>& tags) {
  std::string err;
  if (!library_.save(name, snapshotState(), tags, err)) {
    std::lock_guard<std::mutex> lock(controlMutex_);
    lastMessage_ = "Sauvegarde refusee : " + err;
    return false;
  }
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    presetName_ = PresetLibrary::sanitizeName(name);
    lastMessage_ = "Preset sauvegarde : " + presetName_;
  }
  dirty_.store(false);
  notifyState();
  return true;
}

bool SubdestrucktProcessor::setPresetFavorite(const PresetInfo& info, bool fav) {
  std::string err;
  return library_.setFavorite(info, fav, err);
}

std::string SubdestrucktProcessor::presetName() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return presetName_;
}

// ---------------------------------------------------------------- topologie, matrice, banque, courbes
RouteTable::Result SubdestrucktProcessor::applyTopology(const std::vector<RouteTable::Op>& ops) {
  RouteTable::Result res;
  const PluginState before = snapshotState();
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    res = routes_.apply(ops);
    if (res.ok) {
      publishTopologyLocked();
      lastMessage_ = "Transaction appliquee";
    } else {
      lastMessage_ = "Transaction refusee : " + res.error;
    }
  }
  if (res.ok) {
    record(before, "topologie");
    notifyState();
  }
  return res;
}

RouteTable::Result SubdestrucktProcessor::applyFmTopology(int index) {
  std::vector<RouteTable::Op> ops;
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    ops = fmTopologyOps(routes_, index);
  }
  return applyTopology(ops);
}

void SubdestrucktProcessor::publishTopologyLocked() {
  engine_.exchange().collectGarbage();  // côté consommateur de la file de retrait : toujours sous controlMutex_
  auto c = std::make_unique<CompiledTopology>(routes_.compile());
  c->serial = ++serial_;
  engine_.exchange().publish(std::move(c));
}

void SubdestrucktProcessor::publishModLocked() {
  engine_.modExchange().collectGarbage();
  auto c = compileModMatrix(mod_);
  c->serial = ++modSerial_;
  engine_.modExchange().publish(std::move(c));
}

void SubdestrucktProcessor::publishBankLocked() {
  engine_.bankExchange().collectGarbage();
  auto b = resolveBank(bank_);
  if (!b) b = FactoryBanks::get("factory:saw");
  engine_.bankExchange().publish(std::make_unique<BankRef>(BankRef{b}));
}

void SubdestrucktProcessor::publishCurvesLocked() {
  engine_.curveExchange().collectGarbage();
  engine_.curveExchange().publish(std::make_unique<std::array<CurvePoints, 4>>(lfoCurves_));
}

ModTxResult SubdestrucktProcessor::modAdd(const ModRoute& r) {
  ModTxResult res;
  const PluginState before = snapshotState();
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    res = sbd::modAdd(mod_, r);
    if (res.ok) publishModLocked();
    lastMessage_ = res.ok ? "Modulation ajoutee" : "Modulation refusee : " + res.error;
  }
  if (res.ok) {
    record(before, "matrice : ajout");
    notifyState();
  }
  return res;
}

ModTxResult SubdestrucktProcessor::modUpdate(const ModRoute& r) {
  ModTxResult res;
  const PluginState before = snapshotState();
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    res = sbd::modUpdate(mod_, r);
    if (res.ok) publishModLocked();
    lastMessage_ = res.ok ? "Modulation modifiee" : "Modulation refusee : " + res.error;
  }
  if (res.ok) {
    record(before, "matrice : modification");
    notifyState();
  }
  return res;
}

ModTxResult SubdestrucktProcessor::modRemove(std::uint32_t id) {
  ModTxResult res;
  const PluginState before = snapshotState();
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    res = sbd::modRemove(mod_, id);
    if (res.ok) publishModLocked();
    lastMessage_ = res.ok ? "Modulation retiree" : "Modulation refusee : " + res.error;
  }
  if (res.ok) {
    record(before, "matrice : retrait");
    notifyState();
  }
  return res;
}

bool SubdestrucktProcessor::setBank(const std::string& id) {
  const PluginState before = snapshotState();
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    if (!resolveBank(id)) {
      lastMessage_ = "Banque inconnue : " + id;
      return false;
    }
    bank_ = id;
    publishBankLocked();
    lastMessage_ = "Banque : " + resolveBank(id)->name();
  }
  record(before, "banque");
  notifyState();
  return true;
}

bool SubdestrucktProcessor::importBankWav(const std::string& path, int cycleLength) {
  std::vector<float> mono;
  double sr = 0.0;
  std::string err;
  std::shared_ptr<const WtBank> bank;
  if (readWavMono(path, mono, sr, err)) bank = bankFromCycles(mono, cycleLength, juce::File(path).getFileNameWithoutExtension().toStdString().substr(0, 64), err);
  if (!bank) {
    std::lock_guard<std::mutex> lock(controlMutex_);
    lastMessage_ = "Import refuse : " + err;
    return false;
  }
  UserBanks::add(bank);
  return setBank(bank->id());
}

bool SubdestrucktProcessor::exportBankWav(const std::string& path) {
  auto b = resolveBank(currentBank());
  std::string err;
  const bool ok = b && writeWavFloat(path, b->raw(), 48000.0, err);
  std::lock_guard<std::mutex> lock(controlMutex_);
  lastMessage_ = ok ? "Banque exportee : " + path : "Export refuse : " + err;
  return ok;
}

void SubdestrucktProcessor::setMacroName(int index, const std::string& name) {
  if (index < 0 || index > 3) return;
  const PluginState before = snapshotState();
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    macroNames_[static_cast<size_t>(index)] = name.substr(0, 64);
  }
  record(before, "nom de macro");
  notifyState();
}

void SubdestrucktProcessor::setLfoCurve(int index, const CurvePoints& c) {
  if (index < 0 || index > 3) return;
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    PluginState probe;
    probe.lfoCurves[static_cast<size_t>(index)] = c;
    std::string err;
    if (!validateState(probe, err)) {
      lastMessage_ = "Courbe refusee : " + err;
      return;
    }
    lfoCurves_[static_cast<size_t>(index)] = c;
    publishCurvesLocked();
  }
  notifyState();
}

void SubdestrucktProcessor::setQuality(Quality q) {
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    if (q == instance_.quality) return;
    instance_.quality = q;
    reprepareLocked();
    lastMessage_ = std::string("Quality : ") + qualityName(q);
  }
  notifyState();
}

void SubdestrucktProcessor::setDrone(bool on) {
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    instance_.drone = on;
    engine_.setDrone(on);
  }
  notifyState();
}

void SubdestrucktProcessor::setDroneRestore(bool allowed) {
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    instance_.droneRestore = allowed;
  }
  notifyState();
}

std::vector<Route> SubdestrucktProcessor::currentRoutes() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return routes_.routes();
}
std::uint32_t SubdestrucktProcessor::currentNextRouteId() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return routes_.nextRouteId();
}
ModMatrixState SubdestrucktProcessor::currentMod() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return mod_;
}
std::string SubdestrucktProcessor::currentBank() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return bank_;
}
std::array<std::string, 4> SubdestrucktProcessor::macroNames() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return macroNames_;
}
std::array<CurvePoints, 4> SubdestrucktProcessor::lfoCurves() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return lfoCurves_;
}
Quality SubdestrucktProcessor::quality() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return instance_.quality;
}
bool SubdestrucktProcessor::drone() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return instance_.drone;
}
bool SubdestrucktProcessor::droneRestore() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return instance_.droneRestore;
}
std::string SubdestrucktProcessor::lastMessage() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return lastMessage_;
}
MidiLearnMap SubdestrucktProcessor::midiLearn() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return learn_;
}
void SubdestrucktProcessor::clearMidiLearn(int cc) {
  if (cc < 0 || cc > 127) return;
  std::lock_guard<std::mutex> lock(controlMutex_);
  learn_.ccToParam[static_cast<size_t>(cc)] = -1;
  learnRt_[static_cast<size_t>(cc)].store(-1);
  learn_.save(MidiLearnMap::defaultPath());
}

void SubdestrucktProcessor::setStateListener(std::function<void()> f) {
  std::lock_guard<std::mutex> lock(controlMutex_);
  stateListener_ = std::move(f);
}

void SubdestrucktProcessor::notifyState() {
  std::function<void()> f;
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    f = stateListener_;
  }
  if (f) f();
}

void SubdestrucktProcessor::updateLatency() {
  // latence exigée par la configuration structurelle courante (décimation + chaînes) ; l'hôte est notifié
  const auto& st = engine_.setup();
  if (!st.supported) return;
  const int required = st.decimatorLatency + Engine::chainLatencyFor(bridge_.values(), st.engineRate, st.oversampling);
  if (required != getLatencySamples()) setLatencySamples(required);
}

void SubdestrucktProcessor::timerCallback() {
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    engine_.exchange().collectGarbage();
    engine_.modExchange().collectGarbage();
    engine_.bankExchange().collectGarbage();
    engine_.curveExchange().collectGarbage();
    const int cap = learnCaptured_.exchange(-1, std::memory_order_acq_rel);
    if (cap >= 0) {
      learn_.ccToParam[static_cast<size_t>(cap)] = learnRt_[static_cast<size_t>(cap)].load();
      learn_.save(MidiLearnMap::defaultPath());
      lastMessage_ = "MIDI Learn : CC" + std::to_string(cap) + " -> " + paramSpec(learn_.ccToParam[static_cast<size_t>(cap)]).stableId;
    }
  }
  // contrôleurs appris : la valeur hôte est mise à jour depuis le thread message (automation cohérente)
  std::uint32_t r = ccRead_.load(std::memory_order_relaxed);
  const std::uint32_t w = ccWrite_.load(std::memory_order_acquire);
  while (r != w) {
    const CcEvent ev = ccFifo_[r % ccFifo_.size()];
    if (ev.param >= 0 && ev.param < kNumParams) {
      auto* p = bridge_.param(ev.param);
      const auto& sp = paramSpec(ev.param);
      p->setValueNotifyingHost(p->convertTo0to1(sp.minValue + ev.value * (sp.maxValue - sp.minValue)));
    }
    ++r;
  }
  ccRead_.store(r, std::memory_order_release);
  const double now = juce::Time::getMillisecondCounterHiRes() * 1e-3;
  const double dt = lastTimer_ > 0.0 ? std::min(0.2, now - lastTimer_) : 1.0 / 30.0;
  lastTimer_ = now;
  if (evolver_.running()) {
    evolver_.step(dt);
    for (int i = 0; i < kNumParams; ++i) {
      const auto id = static_cast<ParamId>(i);
      const float base = bridge_.base(i), v = evolver_.apply(id, base);
      evolved_[static_cast<size_t>(i)].store(v, std::memory_order_relaxed);
      evolvedOn_[static_cast<size_t>(i)].store(v != base, std::memory_order_relaxed);
    }
  }
  updateLatency();
}

}  // namespace sbd

// Point d'entrée exigé par le client de plugin JUCE.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new sbd::SubdestrucktProcessor(); }
