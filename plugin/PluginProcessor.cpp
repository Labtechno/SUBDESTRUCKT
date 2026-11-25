#include "PluginProcessor.h"

#include "BuildInfo.h"
#include "ui/DiagnosticEditor.h"

namespace sbd {

namespace {
// Table d'onde immuable partagée en lecture par toutes les instances (construite hors RT, une seule fois).
std::shared_ptr<const SawWavetable> sharedTable() {
  static const std::shared_ptr<const SawWavetable> table = std::make_shared<const SawWavetable>();
  return table;
}
const char* kWriter = SUBDESTRUCKT_BUILD_ID " " SUBDESTRUCKT_COMMIT;
}  // namespace

// ---------------------------------------------------------------- ParameterBridge
float ParameterBridge::base(int index) const noexcept {
  const auto i = static_cast<size_t>(index);
  const float hostValue = params_[i]->get();
  if (hasExact_[i].load(std::memory_order_acquire) && hostValue == hostValueAtRestore_[i].load(std::memory_order_relaxed))
    return exact_[i].load(std::memory_order_relaxed);
  return hostValue;
}

void ParameterBridge::setExact(int index, float value) {
  const auto i = static_cast<size_t>(index);
  auto* p = params_[i];
  hasExact_[i].store(false, std::memory_order_release);
  p->setValueNotifyingHost(p->convertTo0to1(value));
  exact_[i].store(value, std::memory_order_relaxed);
  hostValueAtRestore_[i].store(p->get(), std::memory_order_relaxed);
  hasExact_[i].store(true, std::memory_order_release);
}

ParamValues ParameterBridge::values() const noexcept {
  ParamValues v;
  for (int i = 0; i < kNumParams; ++i) v.v[static_cast<size_t>(i)] = base(i);
  return v;
}

// ---------------------------------------------------------------- processeur
SubdestrucktProcessor::SubdestrucktProcessor()
    : juce::AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      engine_(sharedTable()) {
  for (int i = 0; i < kNumParams; ++i) {
    const auto& spec = paramSpec(i);
    // Identité hôte = StableID explicite (jamais un indice) ; le wrapper VST3 en dérive l'identifiant VST3.
    auto param = std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{spec.stableId, spec.version}, spec.name,
        juce::NormalisableRange<float>(spec.minValue, spec.maxValue), spec.defaultValue,
        juce::AudioParameterFloatAttributes().withLabel(spec.unit).withAutomatable(spec.automatable));
    bridge_.attach(i, param.get());
    addParameter(param.release());
  }
  const PluginState initial = defaultState();  // preset minimal
  std::string err;
  routes_.assign(initial.routes, initial.nextRouteId, err);
  engine_.setTopologyImmediate(routes_.compile());
  startTimerHz(20);
}

SubdestrucktProcessor::~SubdestrucktProcessor() { stopTimer(); }

bool SubdestrucktProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() && layouts.getMainInputChannelSet().isDisabled();
}

void SubdestrucktProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  engine_.setParameters(bridge_.values());
  engine_.prepare(sampleRate, samplesPerBlock);
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    CompiledTopology c = routes_.compile();
    c.serial = ++serial_;
    engine_.setTopologyImmediate(c);
  }
  // Latence de décimation déclarée à l'hôte (PDC) ; ne change qu'ici, jamais à l'édition de topologie.
  setLatencySamples(engine_.setup().latencySamples);
}

void SubdestrucktProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals noDenormals;
  const int n = buffer.getNumSamples();
  if (buffer.getNumChannels() < 2 || n <= 0) {
    buffer.clear();
    return;
  }
  engine_.setParameters(bridge_.values());

  // MIDI -> événements du cœur, décodage des octets bruts (aucun objet alloué). Omni : tous les canaux.
  int count = 0;
  if (panicRequested_.exchange(false)) midi_[static_cast<size_t>(count++)] = MidiEvent::allSoundOff(0);
  for (const auto meta : midi) {
    if (count >= static_cast<int>(midi_.size())) break;
    const juce::uint8* d = meta.data;
    if (meta.numBytes < 3) continue;
    const int status = d[0] & 0xF0;
    const int pos = juce::jlimit(0, n - 1, meta.samplePosition);
    if (status == 0x90)
      midi_[static_cast<size_t>(count++)] = d[2] > 0 ? MidiEvent::noteOn(pos, d[1] & 0x7F, d[2] & 0x7F) : MidiEvent::noteOff(pos, d[1] & 0x7F);
    else if (status == 0x80)
      midi_[static_cast<size_t>(count++)] = MidiEvent::noteOff(pos, d[1] & 0x7F);
    else if (status == 0xB0 && d[1] == 120)
      midi_[static_cast<size_t>(count++)] = MidiEvent::allSoundOff(pos);
    else if (status == 0xB0 && d[1] == 123)
      midi_[static_cast<size_t>(count++)] = MidiEvent::allNotesOff(pos);
    // autres messages (pitch bend, CC, pression) : hors périmètre de la tranche, ignorés
  }
  engine_.process(buffer.getWritePointer(0), buffer.getWritePointer(1), n, midi_.data(), count);
  for (int ch = 2; ch < buffer.getNumChannels(); ++ch) buffer.clear(ch, 0, n);
}

double SubdestrucktProcessor::getTailLengthSeconds() const {
  return static_cast<double>(bridge_.base(static_cast<int>(ParamId::AmpRelease)));
}

juce::AudioProcessorEditor* SubdestrucktProcessor::createEditor() { return new DiagnosticEditor(*this); }

// ---------------------------------------------------------------- état
std::string SubdestrucktProcessor::encodeCurrentState() const {
  PluginState s;
  s.params = bridge_.values();
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    s.routes = routes_.routes();
    s.nextRouteId = routes_.nextRouteId();
  }
  return encodeState(s, kWriter);
}

void SubdestrucktProcessor::getStateInformation(juce::MemoryBlock& destData) {
  const std::string text = encodeCurrentState();
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
  applyState(s);
}

bool SubdestrucktProcessor::applyState(const PluginState& state) {
  {
    std::lock_guard<std::mutex> lock(controlMutex_);
    std::string err;
    if (!routes_.assign(state.routes, state.nextRouteId, err)) {
      lastMessage_ = "Etat refuse : " + err;
      return false;
    }
    publishTopologyLocked();
    lastMessage_ = "Etat charge";
  }
  for (int i = 0; i < kNumParams; ++i) bridge_.setExact(i, state.params.v[static_cast<size_t>(i)]);
  return true;
}

// ---------------------------------------------------------------- topologie (transactions)
RouteTable::Result SubdestrucktProcessor::applyTopology(const std::vector<RouteTable::Op>& ops) {
  std::lock_guard<std::mutex> lock(controlMutex_);
  auto res = routes_.apply(ops);
  if (res.ok) {
    publishTopologyLocked();
    lastMessage_ = "Transaction appliquee";
  } else {
    lastMessage_ = "Transaction refusee : " + res.error;
  }
  return res;
}

void SubdestrucktProcessor::publishTopologyLocked() {
  engine_.exchange().collectGarbage();  // côté consommateur de la file de retrait : toujours sous controlMutex_
  auto c = std::make_unique<CompiledTopology>(routes_.compile());
  c->serial = ++serial_;
  engine_.exchange().publish(std::move(c));
}

std::vector<Route> SubdestrucktProcessor::currentRoutes() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return routes_.routes();
}

std::uint32_t SubdestrucktProcessor::currentNextRouteId() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return routes_.nextRouteId();
}

std::string SubdestrucktProcessor::lastMessage() const {
  std::lock_guard<std::mutex> lock(controlMutex_);
  return lastMessage_;
}

void SubdestrucktProcessor::timerCallback() {
  std::lock_guard<std::mutex> lock(controlMutex_);
  engine_.exchange().collectGarbage();
}

}  // namespace sbd

// Point d'entrée exigé par le client de plugin JUCE.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new sbd::SubdestrucktProcessor(); }
