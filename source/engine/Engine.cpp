#include "Engine.h"

#include <algorithm>
#include <cmath>

namespace sbd {

namespace {
constexpr double kPmBound = 32.0;             // borne DSP dure de la somme PM d'une destination (rad)
constexpr double kOutputGuard = 32.0;         // garde numérique de sortie (+30 dBFS) ; > 0 dBFS reste autorisé
constexpr int kTransitionHostSamples = 128;   // candidat initial du contrat causal (128 échantillons hôte)
constexpr double kPanicSeconds = 0.005;       // rampe anti-clic du Panic
constexpr double kDcCutoffHz = 5.0;           // garde DC de sortie
constexpr double kSmoothingSeconds = 0.020;   // rampes des paramètres continus

double dbToGain(double db) { return std::pow(10.0, db / 20.0); }
double noteHz(int n) { return 440.0 * std::pow(2.0, (n - 69) / 12.0); }

bool sameRoute(const CompiledTopology::Edge& a, const CompiledTopology::Edge& b) noexcept {
  return a.routeId == b.routeId && a.src == b.src && a.dst == b.dst && a.delayed == b.delayed;
}
bool containsEdge(const CompiledTopology& t, const CompiledTopology::Edge& e) noexcept {
  for (int i = 0; i < t.numEdges; ++i)
    if (sameRoute(t.edges[static_cast<size_t>(i)], e)) return true;
  return false;
}
}  // namespace

Engine::Engine(std::shared_ptr<const SawWavetable> table) : table_(std::move(table)) {
  active_ = new CompiledTopology();  // topologie vide par défaut (hors RT)
}

Engine::~Engine() {
  delete active_;
  delete incoming_;
}

int Engine::oversamplingFor(double hostRate) noexcept {
  if (!(hostRate > 0.0)) return 0;
  if (std::fabs(hostRate - 44100.0) < 0.5) return 2;  // B-T2 : K >= 2 permanent à 44,1 kHz
  for (int K : {1, 2, 4})
    if (hostRate * K * kT2Seconds >= 1.0 - 1e-12) return K;  // D >= 1
  return 0;
}

bool Engine::prepare(double hostRate, int maxBlockHost) {
  setup_ = EngineSetup{};
  setup_.hostRate = hostRate;
  setup_.maxBlock = std::max(1, maxBlockHost);
  const int K = oversamplingFor(hostRate);
  setup_.oversampling = K > 0 ? K : 1;
  setup_.engineRate = hostRate * setup_.oversampling;
  setup_.delay = makeT2Delay(setup_.engineRate);
  setup_.supported = K > 0 && setup_.delay.valid;
  decimator_ = Decimator(setup_.oversampling);
  setup_.latencySamples = setup_.supported ? decimator_.latencyHostSamples() : 0;
  engineBuf_.assign(static_cast<size_t>(setup_.maxBlock) * setup_.oversampling, 0.0);
  hostBuf_.assign(static_cast<size_t>(setup_.maxBlock), 0.0);

  const double fsE = setup_.engineRate > 0 ? setup_.engineRate : 48000.0;
  const int smoothLen = std::max(1, static_cast<int>(std::lround(kSmoothingSeconds * fsE)));
  for (int i = 0; i < kNumParams; ++i) {
    ramps_[static_cast<size_t>(i)].setLength(smoothLen);
    ramps_[static_cast<size_t>(i)].reset(base_.v[static_cast<size_t>(i)]);
  }
  outGain_.setLength(std::max(1, static_cast<int>(std::lround(kSmoothingSeconds * hostRate))));
  outGain_.reset(dbToGain(base_[ParamId::OutGain]));
  adsr_.setSampleRate(fsE);
  adsr_.setParameters(base_[ParamId::AmpAttack], base_[ParamId::AmpDecay], base_[ParamId::AmpSustain],
                      base_[ParamId::AmpRelease]);
  adsr_.reset();
  transitionLength_ = kTransitionHostSamples * setup_.oversampling;
  panicLength_ = std::max(1, static_cast<int>(std::lround(kPanicSeconds * hostRate)));
  dc_.setup(hostRate > 0 ? hostRate : 48000.0, kDcCutoffHz);

  // état runtime remis à zéro (non persisté)
  oscPhase_.fill(0.0);
  out_.fill(0.0);
  delayed_.fill(0.0);
  prevPmWT_ = 0.0;
  for (auto& l : lines_) l.reset();
  decimator_.reset();
  dc_.reset();
  stackSize_ = 0;
  gate_ = false;
  currentNote_ = 60;
  velocity_ = 100;
  purgeRequested_ = muteRest_ = panicActive_ = false;
  panicLeft_ = 0;

  // une transition interrompue par prepare() est terminée directement (hors RT)
  if (incoming_) {
    delete active_;
    active_ = incoming_;
    incoming_ = nullptr;
  }
  transition_ = Transition::Idle;
  edgeGain_.fill(1.0);
  edgeRemoved_.fill(false);
  edgeAdded_.fill(false);

  telemetry_.hostRate.store(hostRate);
  telemetry_.engineRate.store(setup_.engineRate);
  telemetry_.oversampling.store(setup_.oversampling);
  telemetry_.delaySamples.store(setup_.delay.D);
  telemetry_.delayInteger.store(setup_.delay.integer);
  telemetry_.thiranCoefficient.store(setup_.delay.integer ? 0.0 : setup_.delay.a);
  telemetry_.latencySamples.store(setup_.latencySamples);
  telemetry_.supported.store(setup_.supported);
  publishTelemetry();
  return setup_.supported;
}

void Engine::setTopologyImmediate(const CompiledTopology& topology) {
  *active_ = topology;
  delete incoming_;
  incoming_ = nullptr;
  transition_ = Transition::Idle;
  edgeGain_.fill(1.0);
  edgeRemoved_.fill(false);
  edgeAdded_.fill(false);
  telemetry_.activeSerial.store(active_->serial);
}

void Engine::setParameters(const ParamValues& base) noexcept {
  base_ = base;
  for (int i = 0; i < kNumParams; ++i) {
    const auto& spec = paramSpec(i);
    if (static_cast<ParamId>(i) == ParamId::OutGain)
      outGain_.setTarget(dbToGain(base.v[static_cast<size_t>(i)]));
    else if (spec.smoothingMs > 0.0f)
      ramps_[static_cast<size_t>(i)].setTarget(base.v[static_cast<size_t>(i)]);
  }
  adsr_.setParameters(base[ParamId::AmpAttack], base[ParamId::AmpDecay], base[ParamId::AmpSustain],
                      base[ParamId::AmpRelease]);
}

void Engine::process(float* outL, float* outR, int numSamples, const MidiEvent* events, int numEvents,
                     double* preDcTap) noexcept {
  if (numSamples <= 0) return;
  if (!setup_.supported) {
    for (int i = 0; i < numSamples; ++i) {
      outL[i] = outR[i] = 0.0f;
      if (preDcTap) preDcTap[i] = 0.0;
    }
    return;
  }
  if (numSamples <= setup_.maxBlock) {
    processChunk(outL, outR, numSamples, events, numEvents, preDcTap);
    return;
  }
  // bloc hôte plus grand que prévu : découpage, sans allocation
  for (int start = 0; start < numSamples; start += setup_.maxBlock) {
    const int len = std::min(setup_.maxBlock, numSamples - start);
    int cnt = 0;
    for (int i = 0; i < numEvents; ++i) {
      const int off = events[i].offset;
      if (off >= start && off < start + len) {
        if (cnt < kMaxEvents) {
          chunkEvents_[static_cast<size_t>(cnt)] = events[i];
          chunkEvents_[static_cast<size_t>(cnt)].offset = off - start;
          ++cnt;
        } else {
          ++dropped_;
        }
      }
    }
    processChunk(outL + start, outR + start, len, chunkEvents_.data(), cnt, preDcTap ? preDcTap + start : nullptr);
  }
}

void Engine::processChunk(float* outL, float* outR, int n, const MidiEvent* events, int numEvents,
                          double* tap) noexcept {
  // tri stable (offset, rang) par insertion dans un tampon préalloué
  int cnt = 0;
  for (int i = 0; i < numEvents; ++i) {
    if (cnt >= kMaxEvents) {
      ++dropped_;
      continue;
    }
    MidiEvent e = events[i];
    e.offset = std::min(std::max(e.offset, 0), n - 1);
    int p = cnt;
    while (p > 0) {
      const MidiEvent& q = events_[static_cast<size_t>(p - 1)];
      if (q.offset > e.offset || (q.offset == e.offset && eventRank(q.type) > eventRank(e.type))) {
        events_[static_cast<size_t>(p)] = q;
        --p;
      } else {
        break;
      }
    }
    events_[static_cast<size_t>(p)] = e;
    ++cnt;
  }
  muteRest_ = false;
  blockPeakL_ = blockPeakR_ = 0.0f;
  if (purgeRequested_) {
    purgeStates();
    purgeRequested_ = false;
  }
  if (transition_ == Transition::Idle && incoming_ == nullptr && exchange_.canRetire()) {
    if (CompiledTopology* p = exchange_.acquire()) beginTransition(p);
  }
  int pos = 0, ei = 0;
  while (pos < n) {
    while (ei < cnt && events_[static_cast<size_t>(ei)].offset == pos) handleEvent(events_[static_cast<size_t>(ei++)]);
    int end = n;
    if (ei < cnt) end = std::min(end, events_[static_cast<size_t>(ei)].offset);
    if (panicActive_) end = std::min(end, pos + std::max(panicLeft_, 1));
    renderSegment(pos, end, outL, outR, tap);
    pos = end;
    if (panicActive_ && panicLeft_ <= 0) {  // fin de rampe : purge à une frontière de segment
      purgeStates();
      adsr_.reset();
      panicActive_ = false;
    }
  }
  publishTelemetry();
}

void Engine::renderSegment(int a, int b, float* outL, float* outR, double* tap) noexcept {
  const int nh = b - a;
  const int ne = nh * setup_.oversampling;
  for (int i = 0; i < ne; ++i) engineBuf_[static_cast<size_t>(i)] = tick();
  decimator_.process(engineBuf_.data(), hostBuf_.data(), nh);
  for (int h = 0; h < nh; ++h) {
    double y = hostBuf_[static_cast<size_t>(h)] * outGain_.next();
    if (tap) tap[a + h] = y;
    y = dc_.process(y);
    if (!std::isfinite(y) && !muteRest_) {  // un incident = une réinitialisation (purge au bloc suivant)
      ++nonFinite_;
      purgeRequested_ = true;
      muteRest_ = true;
    }
    if (muteRest_) {
      y = 0.0;
    } else if (y > kOutputGuard) {
      y = kOutputGuard;
      ++clampHits_;
    } else if (y < -kOutputGuard) {
      y = -kOutputGuard;
      ++clampHits_;
    }
    if (panicActive_) {
      y *= static_cast<double>(panicLeft_) / panicLength_;
      --panicLeft_;
    }
    const float f = static_cast<float>(y);
    outL[a + h] = f;
    outR[a + h] = f;
    blockPeakL_ = std::max(blockPeakL_, std::fabs(f));
    blockPeakR_ = blockPeakL_;
  }
}

double Engine::tick() noexcept {
  const double fsE = setup_.engineRate;
  // rampes des paramètres continus (valeurs Effective)
  const double wtLevel = ramps_[static_cast<size_t>(ParamId::WtLevel)].next();
  const double op1Level = ramps_[static_cast<size_t>(ParamId::Op1Level)].next();
  const double op2Level = ramps_[static_cast<size_t>(ParamId::Op2Level)].next();
  const double r1 = ramps_[static_cast<size_t>(ParamId::Op1Ratio)].next();
  const double r2 = ramps_[static_cast<size_t>(ParamId::Op2Ratio)].next();
  const double master = ramps_[static_cast<size_t>(ParamId::CrossMaster)].next();
  const double fwd = ramps_[static_cast<size_t>(ParamId::CrossFwd)].next();
  const double rev = ramps_[static_cast<size_t>(ParamId::CrossRev)].next();
  const double velAmount = ramps_[static_cast<size_t>(ParamId::AmpVelocity)].next();
  for (int s = 0; s < kMaxRoutes; ++s) {
    const auto& info = routeSlotInfo(s);
    const double depth = ramps_[static_cast<size_t>(info.depthParam)].next();
    double factor = 1.0;
    if (info.kind == RouteKind::CrossForward)
      factor = master * fwd;
    else if (info.kind == RouteKind::CrossReverse)
      factor = master * rev;
    effective_[static_cast<size_t>(s)] = depth * factor;
  }
  const double f0 = noteHz(currentNote_);
  const std::array<double, kNumNodes> inc = {f0 / fsE, f0 * r1 / fsE, f0 * r2 / fsE};

  // lectures retardées T2 : tous les lecteurs tournent en continu (état toujours valide)
  for (int s = 0; s < kNumNodes; ++s) delayed_[static_cast<size_t>(s)] = lines_[static_cast<size_t>(s)].read(setup_.delay);

  advanceTransition();

  if (poison_) out_.fill(std::numeric_limits<double>::quiet_NaN());
  const CompiledTopology& T = *active_;
  bool sawRemoved = false, sawAdded = false;
  for (int k = 0; k < kNumNodes; ++k) {
    const int node = reverseOrder_ ? T.order[static_cast<size_t>(kNumNodes - 1 - k)] : T.order[static_cast<size_t>(k)];
    double pm = 0.0;
    for (int e = T.inBegin[static_cast<size_t>(node)]; e < T.inBegin[static_cast<size_t>(node + 1)]; ++e) {
      const auto& E = T.edges[static_cast<size_t>(e)];
      const double g = edgeGain_[static_cast<size_t>(e)];
      if (g > 0.0) {
        sawRemoved = sawRemoved || edgeRemoved_[static_cast<size_t>(e)];
        sawAdded = sawAdded || edgeAdded_[static_cast<size_t>(e)];
      }
      const double x = E.delayed ? delayed_[E.src] : out_[E.src];
      pm += g * effective_[E.slot] * x;
    }
    if (pm > kPmBound) {
      pm = kPmBound;
      ++pmClampHits_;
    } else if (pm < -kPmBound) {
      pm = -kPmBound;
      ++pmClampHits_;
    }
    double y;
    if (!std::isfinite(pm)) {
      y = pm;  // propagé, détecté par la garde de sortie
    } else if (node == kNodeWT) {
      double ph = oscPhase_[kNodeWT] + pm / kTwoPi;
      ph -= std::floor(ph);
      const double inst = std::fabs(inc[kNodeWT] + (pm - prevPmWT_) / kTwoPi);
      y = table_->read(ph, inst);
    } else {
      y = std::sin(kTwoPi * oscPhase_[static_cast<size_t>(node)] + pm);
    }
    if (node == kNodeWT) prevPmWT_ = pm;
    out_[static_cast<size_t>(node)] = y;
  }
  if (sawRemoved && sawAdded) ++overlapSamples_;

  for (int s = 0; s < kNumNodes; ++s) {
    lines_[static_cast<size_t>(s)].write(out_[static_cast<size_t>(s)]);
    double p = oscPhase_[static_cast<size_t>(s)] + inc[static_cast<size_t>(s)];
    oscPhase_[static_cast<size_t>(s)] = p - std::floor(p);
  }

  const double mix = wtLevel * out_[kNodeWT] + op1Level * out_[kNodeOP1] + op2Level * out_[kNodeOP2];
  const double env = adsr_.next();
  const double velGain = 1.0 - velAmount + velAmount * (velocity_ / 127.0);
  return mix * env * velGain;
}

void Engine::beginTransition(CompiledTopology* incoming) noexcept {
  incoming_ = incoming;
  bool anyRemoved = false;
  for (int e = 0; e < active_->numEdges; ++e) {
    const bool removed = !containsEdge(*incoming_, active_->edges[static_cast<size_t>(e)]);
    edgeRemoved_[static_cast<size_t>(e)] = removed;
    edgeAdded_[static_cast<size_t>(e)] = false;
    anyRemoved = anyRemoved || removed;
  }
  if (anyRemoved) {
    transition_ = Transition::FadeOut;
    transitionLeft_ = transitionLength_;
  } else {
    swapToIncoming();
  }
}

void Engine::swapToIncoming() noexcept {
  CompiledTopology* old = active_;
  active_ = incoming_;
  incoming_ = nullptr;
  bool anyAdded = false;
  edgeGain_.fill(0.0);
  edgeRemoved_.fill(false);
  edgeAdded_.fill(false);
  for (int e = 0; e < active_->numEdges; ++e) {
    const bool survivor = containsEdge(*old, active_->edges[static_cast<size_t>(e)]);
    edgeGain_[static_cast<size_t>(e)] = survivor ? 1.0 : 0.0;
    edgeAdded_[static_cast<size_t>(e)] = !survivor;
    anyAdded = anyAdded || !survivor;
  }
  exchange_.retire(old);  // place garantie (vérifiée avant acquire)
  if (anyAdded) {
    transition_ = Transition::FadeIn;
    transitionLeft_ = transitionLength_;
  } else {
    transition_ = Transition::Idle;
  }
}

void Engine::advanceTransition() noexcept {
  if (transition_ == Transition::Idle) return;
  const double step = 1.0 / transitionLength_;
  if (transition_ == Transition::FadeOut) {
    for (int e = 0; e < active_->numEdges; ++e)
      if (edgeRemoved_[static_cast<size_t>(e)])
        edgeGain_[static_cast<size_t>(e)] = std::max(0.0, edgeGain_[static_cast<size_t>(e)] - step);
    if (--transitionLeft_ <= 0) {
      for (int e = 0; e < active_->numEdges; ++e)
        if (edgeRemoved_[static_cast<size_t>(e)]) edgeGain_[static_cast<size_t>(e)] = 0.0;
      swapToIncoming();  // le nouveau graphe est évalué dès cet échantillon, arêtes ajoutées à gain 0
    }
  } else {
    for (int e = 0; e < active_->numEdges; ++e)
      if (edgeAdded_[static_cast<size_t>(e)])
        edgeGain_[static_cast<size_t>(e)] = std::min(1.0, edgeGain_[static_cast<size_t>(e)] + step);
    if (--transitionLeft_ <= 0) {
      for (int e = 0; e < active_->numEdges; ++e)
        if (edgeAdded_[static_cast<size_t>(e)]) edgeGain_[static_cast<size_t>(e)] = 1.0;
      edgeAdded_.fill(false);
      transition_ = Transition::Idle;
    }
  }
}

void Engine::handleEvent(const MidiEvent& e) noexcept {
  switch (e.type) {
    case MidiEvent::Type::AllSoundOff:  // Panic (P0-C06)
      stackSize_ = 0;
      gate_ = false;
      adsr_.noteOff();
      panicActive_ = true;
      panicLeft_ = panicLength_;
      ++panics_;
      break;
    case MidiEvent::Type::AllNotesOff:
      stackSize_ = 0;
      gate_ = false;
      adsr_.noteOff();
      break;
    case MidiEvent::Type::NoteOff:
      noteOff(e.note);
      break;
    case MidiEvent::Type::NoteOn:
      if (e.velocity == 0)
        noteOff(e.note);
      else
        noteOn(e.note, e.velocity);
      break;
  }
}

void Engine::noteOn(int note, int velocity) noexcept {
  for (int i = 0; i < stackSize_; ++i)
    if (stack_[static_cast<size_t>(i)] == note) {
      for (int j = i; j + 1 < stackSize_; ++j) stack_[static_cast<size_t>(j)] = stack_[static_cast<size_t>(j + 1)];
      --stackSize_;
      break;
    }
  if (stackSize_ < static_cast<int>(stack_.size())) stack_[static_cast<size_t>(stackSize_++)] = static_cast<std::uint8_t>(note);
  currentNote_ = note;
  velocity_ = velocity;
  gate_ = true;
  adsr_.noteOn();  // relance depuis le niveau courant
}

void Engine::noteOff(int note) noexcept {
  for (int i = 0; i < stackSize_; ++i)
    if (stack_[static_cast<size_t>(i)] == note) {
      for (int j = i; j + 1 < stackSize_; ++j) stack_[static_cast<size_t>(j)] = stack_[static_cast<size_t>(j + 1)];
      --stackSize_;
      break;
    }
  if (gate_ && note == currentNote_) {
    if (stackSize_ > 0) {
      currentNote_ = stack_[static_cast<size_t>(stackSize_ - 1)];  // retour à la note tenue la plus récente
    } else {
      gate_ = false;
      adsr_.noteOff();
    }
  }
}

void Engine::purgeStates() noexcept {
  out_.fill(0.0);
  delayed_.fill(0.0);
  prevPmWT_ = 0.0;
  for (auto& l : lines_) l.reset();
  for (auto& p : oscPhase_)
    if (!std::isfinite(p)) p = 0.0;
  decimator_.reset();
  dc_.reset();
}

void Engine::publishTelemetry() noexcept {
  auto& t = telemetry_;
  t.currentNote.store(gate_ ? currentNote_ : -1, std::memory_order_relaxed);
  t.gate.store(gate_, std::memory_order_relaxed);
  t.envelope.store(static_cast<float>(adsr_.level()), std::memory_order_relaxed);
  t.peakL.store(blockPeakL_, std::memory_order_relaxed);
  t.peakR.store(blockPeakR_, std::memory_order_relaxed);
  t.nonFiniteResets.store(nonFinite_, std::memory_order_relaxed);
  t.outputClampHits.store(clampHits_, std::memory_order_relaxed);
  t.pmClampHits.store(pmClampHits_, std::memory_order_relaxed);
  t.droppedEvents.store(dropped_, std::memory_order_relaxed);
  t.panics.store(panics_, std::memory_order_relaxed);
  t.activeSerial.store(active_->serial, std::memory_order_relaxed);
  t.transitionPhase.store(static_cast<int>(transition_), std::memory_order_relaxed);
  for (int s = 0; s < kMaxRoutes; ++s) {
    t.effectiveDepth[static_cast<size_t>(s)].store(static_cast<float>(effective_[static_cast<size_t>(s)]),
                                                   std::memory_order_relaxed);
    float g = 0.0f;
    for (int e = 0; e < active_->numEdges; ++e)
      if (active_->edges[static_cast<size_t>(e)].slot == s) g = static_cast<float>(edgeGain_[static_cast<size_t>(e)]);
    t.edgeGain[static_cast<size_t>(s)].store(g, std::memory_order_relaxed);
  }
}

}  // namespace sbd
