#include "Engine.h"

#include "../dsp/ZSpectral.h"
#include "../dsp/Elp.h"

#include <algorithm>
#include <cmath>

namespace sbd {

namespace {
constexpr double kPmBound = 32.0;             // borne DSP dure de la somme PM d'une destination (rad)
constexpr double kLinBound = 8.0;             // LinFM/TZFM : ±800 % de la fréquence porteuse
constexpr double kExpBound = 4.0;             // ExpFM : ±4 octaves
constexpr int kTransitionHostSamples = 128;   // candidat initial du contrat causal (128 échantillons hôte)
constexpr double kPanicSeconds = 0.005;
// armements des comportements autonomes (filtres auto-oscillants/gel modal, gels D2/S2, N2) : mêmes 9 que StateCodec
constexpr ParamId kAutonomyArms[9] = {ParamId::FltWtArm, ParamId::FltFmArm, ParamId::FltGlArm, ParamId::DstWt1Arm, ParamId::DstWt2Arm,
                                      ParamId::DstFm1Arm, ParamId::DstFm2Arm, ParamId::DstGl1Arm, ParamId::DstGl2Arm};       // rampe anti-clic du Panic
constexpr double kDcCutoffHz = 5.0;           // garde DC de sortie
constexpr double kSmoothingSeconds = 0.020;   // rampes des paramètres continus
constexpr double kAcCutoffHz = 5.0;           // couplage AC des sorties de modulation (option par source)
constexpr double kBankFadeSeconds = 0.003;    // fondu du WT lors d'un changement de banque
constexpr int kSpectralPeriod = 128;          // morph spectral : période de resynthèse (échantillons moteur)

double dbToGain(double db) { return std::pow(10.0, db / 20.0); }
inline double clampd(double v, double lo, double hi) noexcept { return v < lo ? lo : (v > hi ? hi : v); }
inline double pitchRatio(double semis) noexcept { return semis == 0.0 ? 1.0 : std::exp2(semis / 12.0); }
// Panoramique à puissance constante normalisé au centre (centre = gain unité sur chaque canal).
inline void panGains(double pan, double& gl, double& gr) noexcept {
  if (pan == 0.0) {
    gl = gr = 1.0;
    return;
  }
  const double th = (clampd(pan, -1.0, 1.0) + 1.0) * (kPi / 4.0);
  gl = std::sqrt(2.0) * std::cos(th);
  gr = std::sqrt(2.0) * std::sin(th);
}
inline double triFold(double u) noexcept {
  u = std::fmod(u + 1.0, 4.0);
  if (u < 0) u += 4.0;
  return u <= 2.0 ? u - 1.0 : 3.0 - u;
}
// Primitive périodique du repli triangulaire (période 4, moyenne nulle) pour l'anti-repliement ADAA d'ordre 1.
inline double triFoldF(double u) noexcept {
  double w = std::fmod(u + 1.0, 4.0);
  if (w < 0) w += 4.0;
  w -= 1.0;  // [-1,3)
  return w <= 1.0 ? 0.5 * w * w - 0.5 : 2.0 * w - 0.5 * w * w - 1.5;
}
inline double zCurve(int c, double x) noexcept {
  switch (c) {
    case 1: return x * x;
    case 2: return 1.0 - (1.0 - x) * (1.0 - x);
    case 3: return x * x * (3.0 - 2.0 * x);
    default: return x;
  }
}
constexpr double kDivBeats[15] = {32.0, 16.0, 8.0, 4.0, 2.0, 1.0, 0.5, 0.25, 0.125, 2.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0,
                                  1.5, 0.75, 0.375};

bool sameRoute(const CompiledTopology::Edge& a, const CompiledTopology::Edge& b) noexcept {
  return a.routeId == b.routeId && a.src == b.src && a.dst == b.dst && a.delayed == b.delayed;
}
bool containsEdge(const CompiledTopology& t, const CompiledTopology::Edge& e) noexcept {
  for (int i = 0; i < t.numEdges; ++i)
    if (sameRoute(t.edges[static_cast<size_t>(i)], e)) return true;
  return false;
}

// Disposition des paramètres d'opérateur, relative à opN.tune (vérifiée à la compilation).
constexpr ParamId kOpTune[4] = {ParamId::Op1Tune, ParamId::Op2Tune, ParamId::Op3Tune, ParamId::Op4Tune};
inline int opTuneIndex(int n) noexcept { return idx(kOpTune[n - 1]); }
static_assert(idx(ParamId::Op1Sync) - idx(ParamId::Op1Tune) == 15, "disposition operateur");
static_assert(idx(ParamId::Op3Sync) - idx(ParamId::Op3Tune) == 15, "disposition operateur");
static_assert(idx(ParamId::Op4Wave) - idx(ParamId::Op4Tune) == 5, "disposition operateur");
static_assert(idx(ParamId::Op2Enable) - idx(ParamId::Op2Tune) == 10, "disposition operateur");
static_assert(idx(ParamId::Lfo2Shape) - idx(ParamId::Lfo1Shape) == 6, "disposition LFO");
static_assert(idx(ParamId::Lfo4Retrig) - idx(ParamId::Lfo4Shape) == 5, "disposition LFO");
static_assert(idx(ParamId::EnvWtLoop) - idx(ParamId::EnvWtDelay) == 7, "disposition ENV");
static_assert(idx(ParamId::EnvAux2Loop) - idx(ParamId::EnvAux2Delay) == 7, "disposition ENV");
static_assert(idx(ParamId::CrossBusW4) - idx(ParamId::CrossBusW1) == 3, "disposition bus");
static_assert(idx(ParamId::TunePc11) - idx(ParamId::TunePc0) == 11, "disposition accordage");
static_assert(idx(ParamId::Macro4) - idx(ParamId::Macro1) == 3, "disposition macros");

std::shared_ptr<const WtBank> makeOpWaves() {
  std::vector<float> raw;
  auto push = [&raw](const std::vector<float>& w) { raw.insert(raw.end(), w.begin(), w.end()); };
  // 1 Triangle, 2 Saw, 3 Pulse, 4 Folded Sine, 5 Asymmetric, 6 Phase Distorted, 7 Harmonic (0 = sinus calculé)
  push(WtGenerator::triangle());
  push(WtGenerator::saw());
  push(WtGenerator::square());
  push(WtGenerator::foldedSine(2.5));
  push(WtGenerator::additive({1.0, 0.5, 0.25, 0.125}, {0.0, kPi / 2.0, 0.0, kPi / 2.0}));
  {
    std::vector<float> pd(WtBank::kLength);
    for (int i = 0; i < WtBank::kLength; ++i) {
      const double p = static_cast<double>(i) / WtBank::kLength, k = 0.15;
      const double w = p < k ? 0.5 * p / k : 0.5 + 0.5 * (p - k) / (1.0 - k);
      pd[static_cast<size_t>(i)] = static_cast<float>(-std::cos(kTwoPi * w));
    }
    push(pd);
  }
  push(WtGenerator::additive({1.0, 0.5, 1.0 / 3.0, 0.25}, {0.0, 0.0, 0.0, 0.0}));
  std::string err;
  return WtBank::build("internal:opwaves", "Operator Waves", 7, 1, raw, WtBank::Options{}, err);
}
std::shared_ptr<const WtBank> opWavesShared() {
  static const std::shared_ptr<const WtBank> w = makeOpWaves();
  return w;
}
}  // namespace

Engine::Engine(std::shared_ptr<const WtBank> bank)
    : ownedBank_(bank ? std::move(bank) : FactoryBanks::get("factory:saw")), opWaves_(opWavesShared()) {
  bank_ = ownedBank_.get();
  // topologie vide par défaut (hors RT) : compilée depuis une table vide (ordre d'évaluation valide, chaque nœud une
  // fois). ARCH-G4.3 : un CompiledTopology construit par défaut a un ordre nul (six évaluations du nœud WT par
  // échantillon) ; seuls les moteurs utilisés sans setTopologyImmediate (tests directs) étaient concernés.
  active_ = new CompiledTopology(RouteTable().compile());
  mod_ = new CompiledMod();
  for (int i = 0; i < kNumParams; ++i) cur_[static_cast<size_t>(i)] = eff_[static_cast<size_t>(i)] = base_.v[static_cast<size_t>(i)];
}

Engine::~Engine() {
  delete active_;
  delete incoming_;
  delete mod_;
  delete activeBankRef_;
  delete pendingBank_;
  delete retained_;
}

int Engine::oversamplingFor(double hostRate, Quality q) noexcept {
  if (!(hostRate > 0.0)) return 0;
  int K = 0;
  if (std::fabs(hostRate - 44100.0) < 0.5) {
    K = 2;  // B-T2 : K >= 2 permanent à 44,1 kHz
  } else {
    for (int k : {1, 2, 4})
      if (hostRate * k * kT2Seconds >= 1.0 - 1e-12) {
        K = k;  // D >= 1
        break;
      }
  }
  if (K == 0) return 0;
  if (q == Quality::High && K < 4) K *= 2;
  return K;
}

double Engine::testEffectiveDepth(int slot) const noexcept {
  const auto& info = routeSlotInfo(slot);
  double factor = 1.0;
  const int dir = static_cast<int>(eff_[static_cast<size_t>(ParamId::CrossDir)] + 0.5);
  if (info.kind == RouteKind::CrossForward)
    factor = e(ParamId::CrossMaster) * e(ParamId::CrossFwd) * (dir == 2 ? 0.0 : 1.0);
  else if (info.kind == RouteKind::CrossReverse)
    factor = e(ParamId::CrossMaster) * e(ParamId::CrossRev) * (dir == 1 ? 0.0 : 1.0);
  return e(info.depthParam) * factor;
}

int Engine::chainLatencyFor(const ParamValues& p, double engineRate, int K, Quality q) noexcept {
  const int lw = ProcessingChain::latencyFor(ProcessingChain::Wt, p, engineRate, q);
  const int lf = ProcessingChain::latencyFor(ProcessingChain::Fm, p, engineRate, q);
  const int lg = ProcessingChain::latencyFor(ProcessingChain::Global, p, engineRate, q);
  const int le = std::max(lw, lf) + lg;
  return (le + K - 1) / std::max(K, 1);  // en échantillons hôte (arrondi supérieur, complété en interne)
}

bool Engine::prepare(double hostRate, int maxBlockHost, Quality q) {
  setup_ = EngineSetup{};
  setup_.hostRate = hostRate;
  setup_.quality = q;
  setup_.maxBlock = std::max(1, maxBlockHost);
  const int K = oversamplingFor(hostRate, q);
  setup_.oversampling = K > 0 ? K : 1;
  setup_.engineRate = hostRate * setup_.oversampling;
  setup_.delay = makeT2Delay(setup_.engineRate);
  setup_.supported = K > 0 && setup_.delay.valid;
  decL_ = Decimator(setup_.oversampling);
  decR_ = Decimator(setup_.oversampling);
  setup_.decimatorLatency = setup_.supported ? decL_.latencyHostSamples() : 0;
  setup_.latencySamples = setup_.decimatorLatency;
  engineL_.assign(static_cast<size_t>(setup_.maxBlock) * setup_.oversampling, 0.0);
  engineR_.assign(static_cast<size_t>(setup_.maxBlock) * setup_.oversampling, 0.0);
  hostL_.assign(static_cast<size_t>(setup_.maxBlock), 0.0);
  hostR_.assign(static_cast<size_t>(setup_.maxBlock), 0.0);

  const double fsE = setup_.engineRate > 0 ? setup_.engineRate : 48000.0;
  const int smoothLen = std::max(1, static_cast<int>(std::lround(kSmoothingSeconds * fsE)));
  numActiveRamps_ = 0;
  for (int i = 0; i < kNumParams; ++i) {
    const auto ii = static_cast<size_t>(i);
    ramps_[ii].setLength(smoothLen);
    ramps_[ii].reset(base_.v[ii]);
    rampActive_[ii] = false;
    cur_[ii] = base_.v[ii];
    eff_[ii] = base_.v[ii];
  }
  applyAutonomyMask();
  outGain_.setLength(std::max(1, static_cast<int>(std::lround(kSmoothingSeconds * hostRate))));
  outGain_.reset(dbToGain(base_[ParamId::OutGain]));
  for (auto& en : envs_) en.setSampleRate(fsE);
  applyEnvelopeParameters();
  for (auto& en : envs_) en.reset();
  transitionLength_ = kTransitionHostSamples * setup_.oversampling;
  panicLength_ = std::max(1, static_cast<int>(std::lround(kPanicSeconds * hostRate)));
  for (auto& d : dc_) d.setup(hostRate > 0 ? hostRate : 48000.0, kDcCutoffHz);
  controlPeriod_ = std::max(1, static_cast<int>(std::lround(fsE / 1000.0)));
  controlCounter_ = 0;
  bankFadeLength_ = std::max(1, static_cast<int>(std::lround(kBankFadeSeconds * fsE)));
  smA_.assign(WtBank::kStride, 0.0);
  smB_.assign(WtBank::kStride, 0.0);
  smWork_.assign(WtBank::kLength, std::complex<double>(0.0, 0.0));
  smCoef_.assign(WtBank::kHarmonics + 2, std::complex<double>(0.0, 0.0));
  smOrig_.assign(WtBank::kHarmonics + 2, std::complex<double>(0.0, 0.0));
  smZWork_.assign(WtBank::kHarmonics + 2, std::complex<double>(0.0, 0.0));
  zSpecActive_ = false;
  smBlendKey_.fill(0.0);
  if (smFft_.size() != WtBank::kLength) smFft_.init(WtBank::kLength);
  smCounter_ = 0;
  smValid_ = false;
  syncFadeLength_ = std::max(2, static_cast<int>(std::lround(4.0 * fsE / 48000.0)));
  syncFade_ = 0;
  syncGhostPhase_ = 0.0;
  foldPrev_.fill(0.0);
  lastInc_.fill(0.0);
  chainWt_.configure(ProcessingChain::Wt, fsE, q);
  chainFm_.configure(ProcessingChain::Fm, fsE, q);
  chainGl_.configure(ProcessingChain::Global, fsE, q);
  for (auto* a : {&alignWtL_, &alignWtR_, &alignFmL_, &alignFmR_, &alignOutL_, &alignOutR_}) a->configure(2 * DestructionSlot::kMaxFft + 8);
  for (int k = 0; k < 6; ++k) {  // tampons de capture préalloués (pire cas D2 : 2,05 s stéréo)
    captureBuf_[static_cast<size_t>(k)].assign(static_cast<size_t>(2 * DestructionSlot::captureCapacity(fsE)), 0.0f);
    captureReady_[static_cast<size_t>(k)].store(false);
  }
  // re-préparation (fréquence, taille de bloc, Quality, réactivation) : l'autorisation et les captures retenues sont
  // conservées ; les tampons DSP étant remis à zéro, les captures retenues seront réappliquées sous autorisation
  if (retained_) {
    captureApplyPending_ = !retained_->items.empty();
    captureApplied_ = 0;
    captureWait_ = 0;
  }
  padActive_ = false;

  // état runtime remis à zéro (non persisté)
  oscPhase_.fill(0.0);
  out_.fill(0.0);
  modOut_.fill(0.0);
  delayed_.fill(0.0);
  wrapped_.fill(false);
  acState_.fill(0.0);
  prevPmOp_.fill(0.0);
  opCharHold_.fill(0.0);
  prevPmWT_ = 0.0;
  syncMasterPhase_ = 0.0;
  syncMasterWrapped_ = false;
  wtDirection_ = 1.0;
  zWalkSpeed_ = zWalkAmp_ = zRmsIn_ = zRmsOut_ = 0.0;
  charHold_ = 0.0;
  charCounter_ = 0;
  for (auto& l : lines_) l.reset();
  decL_.reset();
  decR_.reset();
  for (auto& d : dc_) d.reset();
  limGain_ = 1.0;
  stackSize_ = 0;
  sustained_.fill(false);
  gate_ = false;
  currentNote_ = 60;
  velocity_ = 100;
  bend_ = 0.0;
  sustainPedal_ = false;
  purgeRequested_ = muteRest_ = panicActive_ = false;
  panicLeft_ = 0;
  routeSmooth_.fill(0.0);
  routeCtl_.fill(0.0);
  routeCtlStep_.fill(0.0);
  src_.fill(0.0);
  src_[static_cast<size_t>(ModSource::Expression)] = 1.0;  // CC11 au repos = 127
  for (auto& f : followers_) f.reset();
  setSeed(seed_);
  updateF0();

  // une transition interrompue par prepare() est terminée directement (hors RT)
  if (incoming_) {
    delete active_;
    active_ = incoming_;
    incoming_ = nullptr;
  }
  if (pendingBank_) {
    delete activeBankRef_;
    activeBankRef_ = pendingBank_;
    pendingBank_ = nullptr;
    bank_ = activeBankRef_->bank.get();
  }
  bankFade_ = 0;
  bankFadingOut_ = false;
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
  telemetry_.quality.store(static_cast<int>(q));
  publishTelemetry();
  return setup_.supported;
}

bool Engine::fetchCapture(int slot, CaptureAsset& out) {
  if (slot < 0 || slot > 5 || !captureReady_[static_cast<size_t>(slot)].load(std::memory_order_acquire)) return false;
  const int n = captureFrames_[static_cast<size_t>(slot)], kind = captureKind_[static_cast<size_t>(slot)];
  out = CaptureAsset{};
  out.slot = slot;
  out.kind = kind;
  out.sampleRate = setup_.engineRate;
  out.frames = n;
  const auto& b = captureBuf_[static_cast<size_t>(slot)];
  out.data.assign(b.begin(), b.begin() + 2 * n);
  captureReady_[static_cast<size_t>(slot)].store(false, std::memory_order_release);
  return n > 0;
}

void Engine::setSeed(std::uint64_t seed) {
  seed_ = seed;
  for (int k = 0; k < 4; ++k) lfos_[static_cast<size_t>(k)].init(seed, static_cast<StreamId>(static_cast<int>(StreamId::Lfo1) + k));
  rndHold_.init(seed, StreamId::RandomHold);
  rndSmooth_.init(seed, StreamId::RandomSmooth);
  chaos_.reset(kChaosModel, static_cast<double>(seed % 1000) * 1e-6);
  phaseRng_.init(seed, StreamId::WtPhase);
  mutRng_.init(seed, StreamId::ZMutation);
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

void Engine::setModMatrixImmediate(const CompiledMod& mod) {
  *mod_ = mod;
  modulated_.fill(false);
  for (int k = 0; k < mod_->numDests; ++k) modulated_[static_cast<size_t>(mod_->dests[static_cast<size_t>(k)])] = true;
  routeSmooth_.fill(0.0);
  routeCtl_.fill(0.0);
  routeCtlStep_.fill(0.0);
}

void Engine::setBankImmediate(std::shared_ptr<const WtBank> bank) {
  delete activeBankRef_;
  activeBankRef_ = new BankRef{std::move(bank)};
  bank_ = activeBankRef_->bank.get();
}

void Engine::setLfoCurves(const std::array<CurvePoints, 4>& curves) {
  for (int k = 0; k < 4; ++k) {
    auto& d = lfoCurves_[static_cast<size_t>(k)];
    const auto& s = curves[static_cast<size_t>(k)];
    d.n = s.n;
    d.x = s.x;
    d.y = s.y;
  }
}

// Synchronisation tempo optionnelle (règle ARCH-G4.1, provisoire) : les durées Delay/Attack/Hold/Decay/Release sont
// lues en noires (1 = une noire) et converties au tempo courant ; un changement de BPM est pris en compte au bloc
// suivant (sans saut de niveau : seules les pentes changent). Sans transport valide : 120 bpm.
void Engine::applyEnvelopeParameters() noexcept {
  const auto& b = base_;
  const double spb = 60.0 / (transport_.bpm > 1.0 ? transport_.bpm : 120.0);
  const double ka = b.flag(ParamId::AmpSync) ? spb : 1.0;
  if (ka == 1.0)
    envs_[0].setParameters(b[ParamId::AmpDelay], b[ParamId::AmpAttack], b[ParamId::AmpHold], b[ParamId::AmpDecay],
                           b[ParamId::AmpSustain], b[ParamId::AmpRelease], b.choice(ParamId::AmpCurve) == 1, false);
  else
    envs_[0].setParameters(ka * b[ParamId::AmpDelay], ka * b[ParamId::AmpAttack], ka * b[ParamId::AmpHold], ka * b[ParamId::AmpDecay],
                           b[ParamId::AmpSustain], ka * b[ParamId::AmpRelease], b.choice(ParamId::AmpCurve) == 1, false);
  static constexpr ParamId kFirst[4] = {ParamId::EnvWtDelay, ParamId::EnvFmDelay, ParamId::EnvAux1Delay, ParamId::EnvAux2Delay};
  static constexpr ParamId kSync[4] = {ParamId::EnvWtSync, ParamId::EnvFmSync, ParamId::EnvAux1Sync, ParamId::EnvAux2Sync};
  for (int k = 0; k < 4; ++k) {
    const int o = idx(kFirst[k]);
    const double kt = b.flag(kSync[k]) ? spb : 1.0;
    auto v = [&](int d) { return static_cast<double>(b.v[static_cast<size_t>(o + d)]); };
    envs_[static_cast<size_t>(k + 1)].setParameters(kt * v(0), kt * v(1), kt * v(2), kt * v(3), v(4), kt * v(5), v(6) >= 0.5, v(7) >= 0.5);
  }
  // suiveurs d'enveloppe : temps d'attaque/relâchement (correctif ARCH-G4.1 : paramètres auparavant non appliqués,
  // suiveurs instantanés ; détecté par l'audit AUD-01)
  static constexpr ParamId kFolA[3] = {ParamId::Fol1Attack, ParamId::Fol2Attack, ParamId::Fol3Attack};
  static constexpr ParamId kFolR[3] = {ParamId::Fol1Release, ParamId::Fol2Release, ParamId::Fol3Release};
  const double fsE = setup_.engineRate > 0.0 ? setup_.engineRate : 48000.0;
  for (int k = 0; k < 3; ++k) followers_[static_cast<size_t>(k)].setup(fsE, b[kFolA[k]], b[kFolR[k]]);
}

void Engine::setParameters(const ParamValues& base) noexcept {
  base_ = base;
  for (int i = 0; i < kNumParams; ++i) {
    const auto ii = static_cast<size_t>(i);
    const auto& spec = paramSpec(i);
    const double v = base.v[ii];
    if (static_cast<ParamId>(i) == ParamId::OutGain) {
      outGain_.setTarget(dbToGain(v));
    } else if (spec.smoothingMs > 0.0f) {
      auto& r = ramps_[ii];
      if (v != r.target()) {
        r.setTarget(v);
        if (!rampActive_[ii] && r.ramping()) {
          rampActive_[ii] = true;
          activeRamps_[static_cast<size_t>(numActiveRamps_++)] = static_cast<std::int16_t>(i);
        }
      }
    } else {
      cur_[ii] = v;
      if (!modulated_[ii]) eff_[ii] = v;
    }
  }
  applyAutonomyMask();
  applyEnvelopeParameters();
}

void Engine::applyAutonomyMask() noexcept {
  const bool ok = auth_.authorized();
  for (ParamId a : kAutonomyArms) {
    const auto i = static_cast<size_t>(a);
    cur_[i] = eff_[i] = ok ? base_.v[i] : 0.0;
  }
}

bool Engine::anyArmConfigured() const noexcept {
  for (ParamId a : kAutonomyArms)
    if (base_.flag(a)) return true;
  return false;
}

void Engine::process(float* outL, float* outR, int numSamples, const MidiEvent* events, int numEvents,
                     double* preDcTap) noexcept SBD_NONBLOCKING {
  if (numSamples <= 0) return;
  // TRANSPORT AUTHORIZATION (ARCH-G4.3) : évaluée une fois par bloc hôte ; un Panic du bloc domine toute autorisation
  {
    bool panicInBlock = false;
    for (int i = 0; i < numEvents; ++i)
      if (events[i].type == MidiEvent::Type::AllSoundOff) panicInBlock = true;
    const TransportObs obs = !transport_.valid ? TransportObs::Unknown : (transport_.playing ? TransportObs::Rolling : TransportObs::Stopped);
    const bool was = auth_.authorized();
    if (auth_.beginBlock(obs, transport_.recording, transport_.offline, panicInBlock, taClock_, taLog_) && retained_) {
      captureApplyPending_ = true;  // nouvelle autorisation : restitution des captures retenues (une fois)
      captureApplied_ = 0;
      captureWait_ = 0;
    }
    if (was && !auth_.authorized() && drone_) releaseGateIfIdle();  // suspension : le Drone ne maintient plus le Gate
    taClock_ += static_cast<std::uint64_t>(numSamples);
  }
  if (transport_.bpm != envBpm_) {  // tempo modifié : durées synchronisées recalculées (sans allocation)
    envBpm_ = transport_.bpm;
    applyEnvelopeParameters();
  }
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
    TransportInfo t = transport_;
    if (t.valid) t.ppq += start * t.bpm / 60.0 / setup_.hostRate;
    const TransportInfo saved = transport_;
    transport_ = t;
    processChunk(outL + start, outR + start, len, chunkEvents_.data(), cnt, preDcTap ? preDcTap + start : nullptr);
    transport_ = saved;
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
    MidiEvent ev = events[i];
    ev.offset = std::min(std::max(ev.offset, 0), n - 1);
    int p = cnt;
    while (p > 0) {
      const MidiEvent& q = events_[static_cast<size_t>(p - 1)];
      if (q.offset > ev.offset || (q.offset == ev.offset && eventRank(q.type) > eventRank(ev.type))) {
        events_[static_cast<size_t>(p)] = q;
        --p;
      } else {
        break;
      }
    }
    events_[static_cast<size_t>(p)] = ev;
    ++cnt;
  }
  muteRest_ = false;
  blockPeakL_ = blockPeakR_ = 0.0f;
  ppqAtSample_ = transport_.ppq;
  if (purgeRequested_) {
    purgeStates();
    purgeRequested_ = false;
  }
  // commandes instantanées
  const int dc = droneCmd_.exchange(-1, std::memory_order_acq_rel);
  if (dc >= 0) {
    drone_ = dc == 1;
    if (!drone_) releaseGateIfIdle();
  }
  applyAutonomyMask();  // autorisation lue à la frontière de bloc (aucune attente, aucune allocation)
  // D2 : mode de Feedback par emplacement (ARCH-G4.3), lu à la frontière de bloc ; le tampon n'est ni vidé ni réinitialisé
  for (int k = 0; k < 6; ++k) slotAt(k).setD2FeedbackMode(ch(static_cast<ParamId>(idx(ParamId::DstWt1D2FbMode) + k)));
  if (crossResetCmd_.exchange(false, std::memory_order_acq_rel)) crossReset();
  if (exciteCmd_.exchange(false, std::memory_order_acq_rel)) manualExcitePending_ = true;
  // captures R2 : copie cohérente à une frontière de bloc, dans les tampons préalloués (aucune allocation)
  if (const std::uint32_t req = captureReq_.exchange(0, std::memory_order_acq_rel)) {
    for (int k = 0; k < 6; ++k)
      if ((req & (1u << k)) && !captureReady_[static_cast<size_t>(k)].load(std::memory_order_acquire)) {
        auto& buf = captureBuf_[static_cast<size_t>(k)];
        int kind = 0;
        captureFrames_[static_cast<size_t>(k)] = slotAt(k).captureInto(buf.data(), static_cast<int>(buf.size()), kind);
        captureKind_[static_cast<size_t>(k)] = kind;
        captureReady_[static_cast<size_t>(k)].store(true, std::memory_order_release);
      }
  }
  // captures R2 (ARCH-G4.3) : l'ensemble publié hors RT est RETENU par le thread audio (l'ancien est retiré) ; il est
  // appliqué seulement sous autorisation, une fois par autorisation (et après re-préparation), différé jusqu'à ce que
  // l'emplacement exécute l'algorithme visé (D2/S2) hors transition ; abandon après ~2 s (type différent).
  if (captureExchange_.hasPending() && captureExchange_.canRetire()) {
    if (CaptureSet* incoming = captureExchange_.acquire()) {
      if (retained_) captureExchange_.retire(retained_);
      retained_ = incoming;
      captureApplyPending_ = incoming->apply && !incoming->items.empty();
      captureApplied_ = 0;
      captureWait_ = 0;
    }
  }
  if (retained_ && captureApplyPending_ && auth_.authorized()) {
    const auto& items = retained_->items;
    bool all = true;
    for (size_t i = 0; i < items.size() && i < 32; ++i) {
      if (captureApplied_ & (1u << i)) continue;
      const auto& c = items[i];
      if (c.slot < 0 || c.slot > 5) {
        captureApplied_ |= 1u << i;
        continue;
      }
      // capture = contenu d'un gel armé : appliquée seulement si l'emplacement est armé et autorisé (un preset
      // désarmé ou un emplacement non armé ne rejoue jamais un contenu mémorisé)
      if (eff_[static_cast<size_t>(idx(ParamId::DstWt1Arm) + c.slot * static_cast<int>(DstOff::Count))] < 0.5) {
        captureApplied_ |= 1u << i;
        continue;
      }
      DestructionSlot& sl = slotAt(c.slot);
      const int want = c.kind == 0 ? static_cast<int>(DstType::D2) : static_cast<int>(DstType::S2);
      if (sl.activeType() == want && sl.settled()) {
        sl.restoreFrom(c.data.data(), c.frames, c.kind, c.sampleRate);
        captureApplied_ |= 1u << i;
        taLog_.push(TaEvent{taClock_, TaEventType::CaptureApplied, auth_.source(), auth_.generation()});
      } else {
        all = false;
      }
    }
    captureWait_ += n;
    if (all || captureWait_ > static_cast<long long>(2.0 * setup_.engineRate)) captureApplyPending_ = false;
  }
  // publications hors RT : topologie (avec transition), matrice, banque (fondu), courbes
  if (transition_ == Transition::Idle && incoming_ == nullptr && exchange_.canRetire()) {
    if (CompiledTopology* p = exchange_.acquire()) beginTransition(p);
  }
  if (modExchange_.canRetire()) {
    if (CompiledMod* m = modExchange_.acquire()) {
      // conserve l'état lissé des connexions survivantes (même identifiant)
      std::array<double, kMaxModRoutes> sm{}, ctl{};
      for (int i = 0; i < m->numRoutes; ++i)
        for (int j = 0; j < mod_->numRoutes; ++j)
          if (m->routes[static_cast<size_t>(i)].id == mod_->routes[static_cast<size_t>(j)].id) {
            sm[static_cast<size_t>(i)] = routeSmooth_[static_cast<size_t>(j)];
            ctl[static_cast<size_t>(i)] = routeCtl_[static_cast<size_t>(j)];
          }
      CompiledMod* old = mod_;
      mod_ = m;
      routeSmooth_ = sm;
      routeCtl_ = ctl;
      routeCtlStep_.fill(0.0);
      for (int k = 0; k < old->numDests; ++k) {
        const auto d = static_cast<size_t>(old->dests[static_cast<size_t>(k)]);
        modulated_[d] = false;
        eff_[d] = cur_[d];
      }
      for (int k = 0; k < mod_->numDests; ++k) modulated_[static_cast<size_t>(mod_->dests[static_cast<size_t>(k)])] = true;
      modExchange_.retire(old);
    }
  }
  if (!pendingBank_ && bankExchange_.canRetire()) {
    if (BankRef* b = bankExchange_.acquire()) {
      pendingBank_ = b;
      bankFadingOut_ = true;
      bankFade_ = bankFadeLength_;
    }
  }
  if (curveExchange_.canRetire()) {
    if (auto* c = curveExchange_.acquire()) {
      for (int k = 0; k < 4; ++k) {
        auto& d = lfoCurves_[static_cast<size_t>(k)];
        d.n = (*c)[static_cast<size_t>(k)].n;
        d.x = (*c)[static_cast<size_t>(k)].x;
        d.y = (*c)[static_cast<size_t>(k)].y;
      }
      curveExchange_.retire(c);
    }
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
      for (auto& en : envs_) en.reset();
      chainWt_.reset();
      chainFm_.reset();
      chainGl_.reset();
      // ARCH-G4.3 : lignes d'alignement de latence (S2/N1) purgées aussi : sans cela, l'audio antérieur au Panic qu'elles
      // retiennent (jusqu'à 2 x 4096 échantillons) ressortait après la rampe et réexcitait les filtres
      for (auto* a : {&alignWtL_, &alignWtR_, &alignFmL_, &alignFmR_, &alignOutL_, &alignOutR_}) a->reset();
      panicActive_ = false;
    }
  }
  publishTelemetry();
}

void Engine::renderSegment(int a, int b, float* outL, float* outR, double* tap) noexcept {
  const int nh = b - a;
  const int K = setup_.oversampling;
  const int ne = nh * K;
  for (int i = 0; i < ne; ++i) {
    tick(engineL_[static_cast<size_t>(i)], engineR_[static_cast<size_t>(i)]);
    ppqAtSample_ += transport_.bpm / 60.0 / setup_.engineRate;
  }
  decL_.process(engineL_.data(), hostL_.data(), nh);
  decR_.process(engineR_.data(), hostR_.data(), nh);
  const bool limiter = base_.flag(ParamId::OutLimiter);
  // modulation (control-rate) du gain de sortie et du plafond : correctif ARCH-G4.1 (routes auparavant sans effet,
  // détecté par AUD-01) ; sans modulation, le chemin est inchangé bit à bit.
  const int og = idx(ParamId::OutGain), oc = idx(ParamId::OutCeiling);
  const double gainMod = modulated_[static_cast<size_t>(og)] ? dbToGain(eff_[static_cast<size_t>(og)] - base_[ParamId::OutGain]) : 1.0;
  const double ceiling = dbToGain(modulated_[static_cast<size_t>(oc)] ? eff_[static_cast<size_t>(oc)] : base_[ParamId::OutCeiling]);
  const double limRelease = std::exp(-1.0 / (0.1 * setup_.hostRate));
  for (int h = 0; h < nh; ++h) {
    double g = outGain_.next();
    if (gainMod != 1.0) g *= gainMod;
    double y[2] = {hostL_[static_cast<size_t>(h)] * g, hostR_[static_cast<size_t>(h)] * g};
    if (tap) tap[a + h] = y[0];
    for (int c = 0; c < 2; ++c) {
      y[c] = dc_[static_cast<size_t>(c)].process(y[c]);
      if (!std::isfinite(y[c]) && !muteRest_) {  // un incident = une réinitialisation (purge au bloc suivant)
        ++nonFinite_;
        purgeRequested_ = true;
        muteRest_ = true;
      }
    }
    for (int c = 0; c < 2; ++c) {
      // SPEC-HR-01 (proposée, à arbitrer) : au-delà de +24 dBFS, saturation douce asymptotique vers +30 dBFS ;
      // inactive en dessous (aucune opération) ; chaque engagement est compté et signalé.
      // (fonction : dsp/Elp.h ; elpEnabled_ n'est désactivable que par les tests de contre-épreuve)
      const double ay = std::fabs(y[c]);
      if (elpEnabled_ && ay > kElpThreshold && std::isfinite(ay)) {
        y[c] = elpShape(y[c]);
        ++elpHits_;
      }
      if (muteRest_) {
        y[c] = 0.0;
      } else if (y[c] > kOutputGuard) {
        y[c] = kOutputGuard;
        ++clampHits_;
      } else if (y[c] < -kOutputGuard) {
        y[c] = -kOutputGuard;
        ++clampHits_;
      }
    }
    if (limiter && !muteRest_) {
      // limiteur musical optionnel (distinct de la garde numérique) : attaque instantanée, relâchement 100 ms
      const double pk = std::max(std::fabs(y[0]), std::fabs(y[1]));
      const double target = pk > ceiling ? ceiling / pk : 1.0;
      if (target < limGain_) {
        limGain_ = target;
        ++limiterHits_;
      } else {
        limGain_ = target + (limGain_ - target) * limRelease;
      }
      y[0] *= limGain_;
      y[1] *= limGain_;
    }
    if (panicActive_) {
      const double r = static_cast<double>(panicLeft_) / panicLength_;
      y[0] *= r;
      y[1] *= r;
      --panicLeft_;
    }
    const float fl = static_cast<float>(y[0]), fr = static_cast<float>(y[1]);
    if ((scopeCounter_++ & 1) == 0) telemetry_.scope.push(0.5f * (fl + fr));  // capture décimée (moniteur)
    outL[a + h] = fl;
    outR[a + h] = fr;
    blockPeakL_ = std::max(blockPeakL_, std::fabs(fl));
    blockPeakR_ = std::max(blockPeakR_, std::fabs(fr));
  }
}

void Engine::updateSmoothing() noexcept {
  int w = 0;
  for (int k = 0; k < numActiveRamps_; ++k) {
    const auto i = static_cast<size_t>(activeRamps_[static_cast<size_t>(k)]);
    const double v = ramps_[i].next();
    cur_[i] = v;
    if (!modulated_[i]) eff_[i] = v;
    if (ramps_[i].ramping())
      activeRamps_[static_cast<size_t>(w++)] = static_cast<std::int16_t>(i);
    else
      rampActive_[i] = false;
  }
  numActiveRamps_ = w;
}

void Engine::updateSources() noexcept {
  const double fsE = setup_.engineRate;
  auto S = [this](ModSource s) -> double& { return src_[static_cast<size_t>(s)]; };
  // enveloppes : Amp avancée ici (une fois par échantillon), utilisée par le mixage
  S(ModSource::EnvAmp) = envs_[0].next();
  S(ModSource::EnvWt) = envs_[1].next();
  S(ModSource::EnvFm) = envs_[2].next();
  S(ModSource::EnvAux1) = envs_[3].next();
  S(ModSource::EnvAux2) = envs_[4].next();
  // LFO
  for (int k = 0; k < 4; ++k) {
    const int o = idx(ParamId::Lfo1Shape) + 6 * k;
    const int shape = static_cast<int>(eff_[static_cast<size_t>(o)] + 0.5);
    const bool sync = eff_[static_cast<size_t>(o + 2)] >= 0.5;
    const int div = std::min(14, std::max(0, static_cast<int>(eff_[static_cast<size_t>(o + 3)] + 0.5)));
    const double phase = eff_[static_cast<size_t>(o + 4)];
    double inc;
    auto& lfo = lfos_[static_cast<size_t>(k)];
    if (sync) {
      const double beats = kDivBeats[div];
      inc = transport_.bpm / 60.0 / beats / fsE;
      if (transport_.valid && transport_.playing) {
        // verrouillage de phase sur la position du transport
        lfo.setPhase(ppqAtSample_ / beats);
      }
    } else {
      inc = eff_[static_cast<size_t>(o + 1)] / fsE;
    }
    S(static_cast<ModSource>(static_cast<int>(ModSource::Lfo1) + k)) =
        lfo.next(shape, inc, phase, &lfoCurves_[static_cast<size_t>(k)]);
  }
  S(ModSource::RandomHold) = rndHold_.next(e(ParamId::RndHoldRate) / fsE, false);
  S(ModSource::RandomSmooth) = rndSmooth_.next(e(ParamId::RndSmoothRate) / fsE, true);
  S(ModSource::Chaos) = chaos_.next(e(ParamId::ChaosRate), fsE);
  for (int k = 0; k < 4; ++k)
    S(static_cast<ModSource>(static_cast<int>(ModSource::Macro1) + k)) = eff_[static_cast<size_t>(idx(ParamId::Macro1) + k)];
  for (int k = 0; k < 3; ++k) S(static_cast<ModSource>(static_cast<int>(ModSource::Follower1) + k)) = followers_[static_cast<size_t>(k)].value();
  for (int k = 0; k < 6; ++k) {
    const ProcessingChain& c = k < 2 ? chainWt_ : (k < 4 ? chainFm_ : chainGl_);
    S(static_cast<ModSource>(static_cast<int>(ModSource::Y2Wt1) + k)) = c.y2Env(k % 2);
  }
}

double Engine::finalize(int dest, double acc) const noexcept {
  const auto& sp = paramSpec(dest);
  const double b = cur_[static_cast<size_t>(dest)];
  double v;
  switch (sp.domain) {
    case Domain::Oct:
    case Domain::Time: v = b * std::exp2(acc); break;
    case Domain::Cyc: v = b + acc; v -= std::floor(v); return v;
    default: v = b + acc; break;
  }
  if (dest == idx(ParamId::WtX) || dest == idx(ParamId::WtY)) return clampd(v, -8.0, 8.0);  // politique XY du WT
  v = clampd(v, sp.minValue, sp.maxValue);  // bornes artistiques
  return clampd(v, sp.hardMin, sp.hardMax);
}

void Engine::evaluateModulation() noexcept {
  const CompiledMod& M = *mod_;
  if (M.numRoutes == 0) return;
  const bool ctlTick = controlCounter_ == 0;
  if (++controlCounter_ >= controlPeriod_) controlCounter_ = 0;
  for (int k = 0; k < M.numDests; ++k) destAcc_[static_cast<size_t>(k)] = 0.0;
  const double fsE = setup_.engineRate;
  // Connexions vers macros d'abord (ordre topologique des macros), puis finalisation des macros, puis le reste.
  // Une macro modulée par une autre macro lit la valeur finalisée de l'échantillon précédent (retard d'un
  // échantillon moteur, déterministe).
  auto finalizeMacros = [&]() {
    for (int k = 0; k < M.numMacroDests; ++k) {
      const int d = M.dests[static_cast<size_t>(k)];
      eff_[static_cast<size_t>(d)] = finalize(d, destAcc_[static_cast<size_t>(k)]);
      src_[static_cast<size_t>(static_cast<int>(ModSource::Macro1) + d - idx(ParamId::Macro1))] = eff_[static_cast<size_t>(d)];
    }
  };
  for (int i = 0; i < M.numRoutes; ++i) {
    if (i == M.numMacroRoutes) finalizeMacros();
    const auto& r = M.routes[static_cast<size_t>(i)];
    const auto ri = static_cast<size_t>(i);
    double target = 0.0;
    if (r.audio || ctlTick) {
      double s = src_[r.src];
      if (r.invert) s = r.srcBipolar ? -s : 1.0 - s;
      double c;
      const auto cv = static_cast<ModCurve>(r.curve);
      if (r.centered) {
        c = modCurveValue(cv, r.numPoints, r.px.data(), r.py.data(), static_cast<float>(std::fabs(s)));
        if (s < 0.0) c = -c;
      } else {
        if (r.srcBipolar)
          c = 2.0 * modCurveValue(cv, r.numPoints, r.px.data(), r.py.data(), static_cast<float>(0.5 * (s + 1.0))) - 1.0;
        else
          c = modCurveValue(cv, r.numPoints, r.px.data(), r.py.data(), static_cast<float>(s));
        c += r.offset;
      }
      double v = c * r.depth * r.scale;
      if (r.via != 0) {
        double vv = src_[r.via];
        if (r.viaBipolar) vv = 0.5 * (vv + 1.0);
        vv = clampd(vv, 0.0, 1.0);
        v *= (1.0 - r.viaAmount) + r.viaAmount * vv;  // Via multiplicatif unipolaire
      }
      target = v;
    }
    double val;
    if (r.audio) {
      val = target;
    } else {  // control-rate : nouvelle cible à chaque période, interpolation linéaire entre périodes
      if (ctlTick) routeCtlStep_[ri] = (target - routeCtl_[ri]) / controlPeriod_;
      routeCtl_[ri] += routeCtlStep_[ri];
      val = routeCtl_[ri];
    }
    if (r.smoothMs > 0.0f) {
      const double coef = 1.0 - std::exp(-1000.0 / (r.smoothMs * fsE));
      routeSmooth_[ri] += coef * (val - routeSmooth_[ri]);
      val = routeSmooth_[ri];
    }
    destAcc_[r.destSlot] += val;
  }
  if (M.numMacroRoutes == M.numRoutes) finalizeMacros();
  for (int k = M.numMacroDests; k < M.numDests; ++k) {
    const int d = M.dests[static_cast<size_t>(k)];
    eff_[static_cast<size_t>(d)] = finalize(d, destAcc_[static_cast<size_t>(k)]);
  }
}

bool Engine::spectralActive() const noexcept {
  return (ch(ParamId::WtXMorph) == 2 || ch(ParamId::WtYMorph) == 2) && bank_->numWaves() > 1;
}

// Morph spectral (ARCH-G4.1) : la forme morphée est resynthétisée hors chemin par échantillon, toutes les
// kSpectralPeriod échantillons moteur, à partir des spectres préparés de la banque (amplitudes interpolées, phases par
// moyenne circulaire pondérée), limitée en bande d'après la fréquence instantanée (Clean) ; fondu linéaire entre la
// table précédente et la nouvelle. Aucune allocation, aucune FFT par échantillon.
void Engine::spectralUpdate(double x, double y, double absInc, const int* zt, const double* zv, const double* zs) noexcept {
  if (smCounter_ > 0) {
    --smCounter_;
    return;
  }
  const WtBank& B = *bank_;
  const int nx = B.nx(), ny = B.ny();
  auto axis = [](double v, int n, int bound, int& i0, int& i1, double& f) {
    if (n == 1) {
      i0 = i1 = 0;
      f = 0.0;
      return;
    }
    double p;
    if (bound == 1) {
      p = (v - std::floor(v)) * n;
      i0 = std::min(static_cast<int>(p), n - 1);
      i1 = (i0 + 1) % n;
    } else {
      if (bound == 2) {
        const double u = std::fmod(std::fabs(v), 2.0);
        v = u <= 1.0 ? u : 2.0 - u;
      }
      p = clampd(v, 0.0, 1.0) * (n - 1);
      i0 = std::min(static_cast<int>(p), n - 1);
      i1 = std::min(i0 + 1, n - 1);
    }
    f = p - i0;
  };
  int x0, x1, y0, y1;
  double fx, fy;
  axis(x, nx, ch(ParamId::WtXBound), x0, x1, fx);
  axis(y, ny, ch(ParamId::WtYBound), y0, y1, fy);
  if (ch(ParamId::WtXMorph) == 1) fx = fx >= 0.5 ? 1.0 : 0.0;  // Stepped respecté par axe
  if (ch(ParamId::WtYMorph) == 1) fy = fy >= 0.5 ? 1.0 : 0.0;
  const int wv[4] = {y0 * nx + x0, y0 * nx + x1, y1 * nx + x0, y1 * nx + x1};
  const double wt[4] = {(1 - fx) * (1 - fy), fx * (1 - fy), (1 - fx) * fy, fx * fy};
  int dom = 0;
  for (int k = 1; k < 4; ++k)
    if (wt[k] > wt[dom]) dom = k;
  const bool raw = ch(ParamId::WtRender) == 1;
  int H = WtBank::kHarmonics;
  if (!raw && absInc > 1e-12) H = std::max(1, std::min(H, static_cast<int>(0.5 / absInc)));
  const int N = WtBank::kLength;
  // Z spectral (ARCH-G4.3) : transformations de rang appliquées aux coefficients complexes de la forme morphée
  const bool zOn = (zSpectral(zt[0]) && zv[0] > 0.0) || (zSpectral(zt[1]) && zv[1] > 0.0);
  const bool morphSpectral = spectralActive();
  // profil de bande : morph spectral = coupure à H (inchangé) ; sinon profil de la lecture mip (niveaux i0, i0+1, poids
  // w) pour que le chemin spectral soit continu avec la lecture temporelle à profondeur nulle
  int hLo = H, hHi = H;
  double wPart = 0.0;
  if (zOn && !morphSpectral) {
    if (raw || !(absInc > 1e-12)) {
      hLo = hHi = WtBank::kHarmonics;
    } else {
      const double lf = 10.0 - std::log2(0.5 / absInc);
      if (lf <= -1.0) {
        hLo = hHi = WtBank::kHarmonics;
      } else {
        int i0 = static_cast<int>(std::ceil(lf));
        double w = lf - static_cast<double>(i0 - 1);
        if (i0 < 0) i0 = 0, w = 0.0;
        if (i0 >= WtBank::kLevels - 1) {
          hLo = hHi = 1;
        } else {
          hHi = 1 << (10 - i0);
          hLo = 1 << (9 - i0);
          wPart = w;
        }
      }
    }
  }
  // optimisation (ARCH-G4.1, PERF-03) : position, poids, bande et Z inchangés -> table identique, resynthèse sautée
  const std::array<double, 18> key{static_cast<double>(wv[0] * 4096 + wv[1]), static_cast<double>(wv[2] * 4096 + wv[3]), wt[0], wt[1], wt[2],
                                   static_cast<double>(H), static_cast<double>(reinterpret_cast<std::uintptr_t>(&B)),
                                   zOn ? static_cast<double>(zt[0]) : 0.0, zOn ? zv[0] : 0.0, zOn ? zs[0] : 0.0,
                                   zOn ? static_cast<double>(zt[1]) : 0.0, zOn ? zv[1] : 0.0, zOn ? zs[1] : 0.0,
                                   zOn ? static_cast<double>(ch(ParamId::WtZFund)) : 0.0, zOn ? static_cast<double>(ch(ParamId::WtZGain)) : 0.0,
                                   static_cast<double>(hLo), static_cast<double>(hHi), wPart};
  if (smValid_ && key == smKey_) {
    if (!smSettled_) {
      std::copy(smB_.begin(), smB_.end(), smA_.begin());  // fondu terminé : départ = arrivée
      smSettled_ = true;
    }
    smCounter_ = kSpectralPeriod - 1;
    return;
  }
  smKey_ = key;
  smSettled_ = false;
  std::fill(smWork_.begin(), smWork_.end(), std::complex<double>(0.0, 0.0));
  if (!zOn) {
    for (int h = 1; h <= H; ++h) {
      double m = 0.0, re = 0.0, im = 0.0;
      for (int k = 0; k < 4; ++k) {
        if (wt[k] <= 0.0) continue;
        const double mk = B.specMag(wv[k])[h - 1], pk = B.specPhase(wv[k])[h - 1];
        m += wt[k] * mk;
        re += wt[k] * mk * std::cos(pk);
        im += wt[k] * mk * std::sin(pk);
      }
      const double ph = (re * re + im * im) > 1e-18 * (m * m + 1e-300) ? std::atan2(im, re) : B.specPhase(wv[dom])[h - 1];
      const std::complex<double> c = std::polar(0.5 * m * N, ph);
      smWork_[static_cast<size_t>(h)] = c;
      if (h < N / 2) smWork_[static_cast<size_t>(N - h)] = std::conj(c);
    }
  } else {
    auto* c = smCoef_.data();
    // coefficients de la forme morphée mis en cache : une modulation de Z seule (profondeur, Shape, Master) ne refait pas
    // la combinaison des quatre formes (ARCH-G4.3, coût)
    const std::array<double, 8> bkey{key[0], key[1], key[2], key[3], key[4], key[6], morphSpectral ? 1.0 : 0.0, 1.0};
    const bool blendCached = bkey == smBlendKey_;
    smBlendKey_ = bkey;
    if (blendCached) std::copy(smOrig_.begin(), smOrig_.end(), smCoef_.begin());
    else
      ++smBlends_;
    for (int h = 1; h <= WtBank::kHarmonics && !blendCached; ++h) {
      double m = 0.0, re = 0.0, im = 0.0;
      for (int k = 0; k < 4; ++k) {
        if (wt[k] <= 0.0) continue;
        const double mk = B.specMag(wv[k])[h - 1], pk = B.specPhase(wv[k])[h - 1];
        m += wt[k] * mk;
        re += wt[k] * mk * std::cos(pk);
        im += wt[k] * mk * std::sin(pk);
      }
      if (morphSpectral) {
        const double ph = (re * re + im * im) > 1e-18 * (m * m + 1e-300) ? std::atan2(im, re) : B.specPhase(wv[dom])[h - 1];
        c[h] = std::polar(m, ph);
      } else {
        c[h] = std::complex<double>(re, im);  // combinaison linéaire complexe = interpolation temporelle des formes
      }
      smOrig_[static_cast<size_t>(h)] = c[h];
    }
    auto band = [&](int h) { return morphSpectral ? (h <= H ? 1.0 : 0.0) : (h <= hLo ? 1.0 : (h <= hHi ? 1.0 - wPart : 0.0)); };
    zspec::process(c, smOrig_.data(), smZWork_.data(), zt, zv, zs, ch(ParamId::WtZFund) == 1, ch(ParamId::WtZGain) == 1, band);
    for (int h = 1; h <= WtBank::kHarmonics; ++h) {
      const double g = band(h);
      if (g <= 0.0) continue;
      const std::complex<double> v = c[h] * (0.5 * N * g);
      smWork_[static_cast<size_t>(h)] = v;
      if (h < N / 2) smWork_[static_cast<size_t>(N - h)] = std::conj(v);
    }
  }
  ++smResynth_;
  smFft_.transform(smWork_.data(), true);
  std::swap(smA_, smB_);  // la table cible précédente devient la table de départ
  for (int i = 0; i < N; ++i) smB_[static_cast<size_t>(i)] = smWork_[static_cast<size_t>(i)].real() / N;
  smB_[static_cast<size_t>(N)] = smB_[0];
  if (!smValid_) {
    smA_ = smB_;  // première table : pas de fondu depuis le silence (copie dans un tampon préalloué de même taille)
    smValid_ = true;
  }
  smCounter_ = kSpectralPeriod - 1;
}

double Engine::readWt(double phase, double inc, double slope, double x, double y) const noexcept {
  const WtBank& B = *bank_;
  const bool raw = ch(ParamId::WtRender) == 1, cubic = ch(ParamId::WtInterp) == 1;
  const int nx = B.nx(), ny = B.ny();
  if (smValid_ && (spectralActive() || zSpecActive_)) {
    const double t = 1.0 - static_cast<double>(smCounter_) / kSpectralPeriod;
    const double xx = phase * WtBank::kLength;
    int i = std::min(static_cast<int>(xx), WtBank::kLength - 1);
    const double f = xx - i;
    const double a = smA_[static_cast<size_t>(i)] + f * (smA_[static_cast<size_t>(i + 1)] - smA_[static_cast<size_t>(i)]);
    const double b = smB_[static_cast<size_t>(i)] + f * (smB_[static_cast<size_t>(i + 1)] - smB_[static_cast<size_t>(i)]);
    return a + t * (b - a);
  }
  const double absInc = std::fabs(inc) * slope;
  if (nx == 1 && ny == 1) return B.read(0, phase, absInc, raw, cubic);
  auto axis = [](double v, int n, int bound, int morph, int& i0, int& i1, double& f) {
    if (n == 1) {
      i0 = i1 = 0;
      f = 0.0;
      return;
    }
    double p;
    if (bound == 1) {  // Wrap : raccord dernière -> première forme
      p = (v - std::floor(v)) * n;
      i0 = static_cast<int>(p);
      if (i0 >= n) i0 = n - 1;
      i1 = (i0 + 1) % n;
    } else {
      if (bound == 2) {  // Mirror
        double u = std::fmod(std::fabs(v), 2.0);
        v = u <= 1.0 ? u : 2.0 - u;
      }
      p = clampd(v, 0.0, 1.0) * (n - 1);
      i0 = static_cast<int>(p);
      if (i0 >= n - 1) i0 = n - 1;
      i1 = std::min(i0 + 1, n - 1);
    }
    f = p - i0;
    if (morph == 1) {  // Stepped : forme la plus proche, sans fondu
      if (f >= 0.5) i0 = i1;
      f = 0.0;
    }
  };
  int x0, x1, y0, y1;
  double fx, fy;
  axis(x, nx, ch(ParamId::WtXBound), ch(ParamId::WtXMorph), x0, x1, fx);
  axis(y, ny, ch(ParamId::WtYBound), ch(ParamId::WtYMorph), y0, y1, fy);
  auto rd = [&](int yy, int xx) { return B.read(yy * nx + xx, phase, absInc, raw, cubic); };
  double a = rd(y0, x0);
  if (fx > 0.0) a += fx * (rd(y0, x1) - a);
  if (fy > 0.0) {
    double b = rd(y1, x0);
    if (fx > 0.0) b += fx * (rd(y1, x1) - b);
    a += fy * (b - a);
  }
  return a;
}

double Engine::readOpWave(int op, int wave, double phase, double inc) const noexcept {
  if (wave == 8) {  // Wavetable : banque WT commune, XY lent (control-rate), sans Z
    const WtBank& B = *bank_;
    static constexpr ParamId kX[4] = {ParamId::Op1WtX, ParamId::Op2WtX, ParamId::Op3WtX, ParamId::Op4WtX};
    static constexpr ParamId kY[4] = {ParamId::Op1WtY, ParamId::Op2WtY, ParamId::Op3WtY, ParamId::Op4WtY};
    const int nx = B.nx(), ny = B.ny();
    const double px = clampd(e(kX[op]), 0.0, 1.0) * (nx - 1), py = clampd(e(kY[op]), 0.0, 1.0) * (ny - 1);
    const int x0 = static_cast<int>(px), y0 = static_cast<int>(py);
    const int x1 = std::min(x0 + 1, nx - 1), y1 = std::min(y0 + 1, ny - 1);
    const double fx = px - x0, fy = py - y0;
    const double ai = std::fabs(inc);
    double a = B.read(y0 * nx + x0, phase, ai, false, false);
    if (fx > 0.0) a += fx * (B.read(y0 * nx + x1, phase, ai, false, false) - a);
    if (fy > 0.0) {
      double b = B.read(y1 * nx + x0, phase, ai, false, false);
      if (fx > 0.0) b += fx * (B.read(y1 * nx + x1, phase, ai, false, false) - b);
      a += fy * (b - a);
    }
    return a;
  }
  return opWaves_->read(wave - 1, phase, std::fabs(inc), false, false);
}

void Engine::evaluateNetwork(double& wtAudio, double& fmL, double& fmR) noexcept {
  const double fsE = setup_.engineRate;
  const int dir = ch(ParamId::CrossDir);
  const double master = e(ParamId::CrossMaster);
  const double fwdF = master * e(ParamId::CrossFwd) * (dir == 2 ? 0.0 : 1.0);
  const double revF = master * e(ParamId::CrossRev) * (dir == 1 ? 0.0 : 1.0);
  const double f0 = f0_;

  // fréquences de base
  const double wtF = f0 * pitchRatio(e(ParamId::WtCoarse) + e(ParamId::WtFine) / 100.0);
  std::array<double, kNumOps + 1> opF{};
  static constexpr ParamId kRatio[4] = {ParamId::Op1Ratio, ParamId::Op2Ratio, ParamId::Op3Ratio, ParamId::Op4Ratio};
  for (int n = 1; n <= kNumOps; ++n) {
    const int o = opTuneIndex(n);
    const bool fixed = eff_[static_cast<size_t>(o)] >= 0.5;
    const double pr = pitchRatio(eff_[static_cast<size_t>(o + 3)] + eff_[static_cast<size_t>(o + 4)] / 100.0);
    const double base = fixed ? eff_[static_cast<size_t>(o + 1)] : f0 * e(kRatio[n - 1]);
    opF[static_cast<size_t>(n)] = base * pr + eff_[static_cast<size_t>(o + 2)];
  }

  // lectures retardées T2 : tous les lecteurs tournent en continu (état toujours valide)
  for (int s = 0; s < kNumNodes; ++s) delayed_[static_cast<size_t>(s)] = lines_[static_cast<size_t>(s)].read(setup_.delay);

  advanceTransition();

  if (poison_) {
    out_.fill(std::numeric_limits<double>::quiet_NaN());
    modOut_.fill(std::numeric_limits<double>::quiet_NaN());
  }
  const CompiledTopology& T = *active_;
  bool sawRemoved = false, sawAdded = false;
  std::array<bool, kNumNodes> wrapNow{};
  // sync : drapeaux de l'échantillon précédent (causal, indépendant de l'ordre d'évaluation)
  const int wtSync = ch(ParamId::WtSync), wtSyncSrc = ch(ParamId::WtSyncSrc);
  bool wtSyncEvent = false;
  if (wtSync != 0) wtSyncEvent = wtSyncSrc == 0 ? syncMasterWrapped_ : wrapped_[static_cast<size_t>(wtSyncSrc)];
  if (wtSync == 1 && wtSyncEvent) {
    // position sous-échantillon du cycle maître : la nouvelle phase reprend d * inc (d = temps écoulé depuis le cycle)
    const double mInc = wtSyncSrc == 0 ? f0 / fsE : std::fabs(lastInc_[static_cast<size_t>(wtSyncSrc)]);
    const double mPh = wtSyncSrc == 0 ? syncMasterPhase_ : oscPhase_[static_cast<size_t>(wtSyncSrc)];
    const double dSamples = mInc > 1e-12 ? std::min(mPh / mInc, 1.0) : 0.0;
    syncGhostPhase_ = oscPhase_[kNodeWT];
    syncFade_ = syncFadeLength_;
    oscPhase_[kNodeWT] = dSamples * std::fabs(lastInc_[kNodeWT]);
  }
  if (wtSync == 2 && wtSyncEvent) wtDirection_ = -wtDirection_;
  if (wtSync != 2) wtDirection_ = 1.0;

  for (int k = 0; k < kNumNodes; ++k) {
    const int node = reverseOrder_ ? T.order[static_cast<size_t>(kNumNodes - 1 - k)] : T.order[static_cast<size_t>(k)];
    std::array<double, kNumPorts> acc{};
    for (int ed = T.inBegin[static_cast<size_t>(node)]; ed < T.inBegin[static_cast<size_t>(node + 1)]; ++ed) {
      const auto& E = T.edges[static_cast<size_t>(ed)];
      const double g = edgeGain_[static_cast<size_t>(ed)];
      if (g > 0.0) {
        sawRemoved = sawRemoved || edgeRemoved_[static_cast<size_t>(ed)];
        sawAdded = sawAdded || edgeAdded_[static_cast<size_t>(ed)];
      }
      const auto& info = routeSlotInfo(E.slot);
      const double factor = info.kind == RouteKind::CrossForward ? fwdF : (info.kind == RouteKind::CrossReverse ? revF : 1.0);
      const double eff = e(info.depthParam) * factor;
      const double x = E.delayed ? delayed_[E.src] : modOut_[E.src];
      acc[E.dst] += g * eff * x;
    }
    double y = 0.0;
    if (node == kNodeWT) {
      double pm = acc[kPortWtPhase];
      if (pm > kPmBound) {
        pm = kPmBound;
        ++pmClampHits_;
      } else if (pm < -kPmBound) {
        pm = -kPmBound;
        ++pmClampHits_;
      }
      if (!std::isfinite(pm)) {
        y = pm;  // propagé, détecté par la garde de sortie
      } else {
        // fréquence instantanée (mode WT FREQ)
        double f = wtF;
        const double fm = acc[kPortWtFreq];
        if (fm != 0.0) {
          const int fmode = ch(ParamId::CrossWtFreqMode);
          if (fmode == 2)
            f = wtF * std::exp2(clampd(fm, -kExpBound, kExpBound));
          else {
            f = wtF * (1.0 + clampd(fm, -kLinBound, kLinBound));
            if (fmode == 0 && f < 0.0) f = 0.0;  // Linear bloque les fréquences négatives ; TZ les traverse
          }
        }
        const double inc = wtDirection_ * f / fsE;
        double ph = oscPhase_[kNodeWT] + pm / kTwoPi;
        ph -= std::floor(ph);
        // Z : deux emplacements série A->B (ou B->A)
        const double zm = clampd(e(ParamId::WtZ) + acc[kPortWtZ], 0.0, 1.0);
        double slope = 1.0, ampMul = 1.0;
        int zt[2] = {0, 0};
        double zv[2] = {0.0, 0.0}, zs[2] = {0.0, 0.0};
        bool anyZ = false;
        if (zm > 0.0) {
          const bool rev = ch(ParamId::WtZOrder) == 1;
          for (int s = 0; s < 2; ++s) {
            const int slot = rev ? 1 - s : s;
            const ParamId tp = slot == 0 ? ParamId::WtZAType : ParamId::WtZBType;
            const ParamId dp = slot == 0 ? ParamId::WtZADepth : ParamId::WtZBDepth;
            const ParamId cp = slot == 0 ? ParamId::WtZACurve : ParamId::WtZBCurve;
            zt[s] = ch(tp);
            zv[s] = e(dp) * zCurve(ch(cp), zm);
            zs[s] = e(slot == 0 ? ParamId::WtZAShape : ParamId::WtZBShape);
            if (zt[s] != 0 && zv[s] > 0.0) anyZ = true;
          }
          // transformations de phase (ordre de la liste) ; le repli élargit la bande d'un facteur ~gain : la lecture
          // est limitée en bande en conséquence (sélection de mip), puis ADAA1 sur le repli
          for (int s = 0; s < 2; ++s)
            if (zt[s] == 1 && zv[s] > 0.0) slope *= 1.0 + 7.0 * zv[s];
          for (int s = 0; s < 2; ++s) {
            if (zv[s] <= 0.0) continue;
            if (zt[s] == 2) {  // Phase Warp : coude k, période conservée
              const double kn = 0.5 - 0.48 * zv[s];
              ph = ph < kn ? 0.5 * ph / kn : 0.5 + 0.5 * (ph - kn) / (1.0 - kn);
              slope *= 0.5 / kn;
            } else if (zt[s] == 3) {  // Spectral Mutation (DSWS candidat) : marche aléatoire par cycle
              if (ch(ParamId::WtZFund) == 1) {
                const double d = 0.15 * zv[s] * zWalkSpeed_;
                ph += d * std::sin(kTwoPi * ph);
                ph -= std::floor(ph);
                slope *= 1.0 + kTwoPi * std::fabs(d);
              }
              ampMul *= 1.0 + 0.5 * zv[s] * zWalkAmp_;
            }
          }
        }
        // Character : quantification de phase
        const double chr = clampd(e(ParamId::WtChar) + acc[kPortWtChar], 0.0, 1.0);
        const int cmode = ch(ParamId::WtCharMode);
        if (chr > 0.0 && cmode == 0) {
          const double steps = std::floor(2048.0 * std::exp2(-8.0 * chr));
          ph = std::floor(ph * steps) / steps;
        }
        const double xpos = e(ParamId::WtX) + acc[kPortWtX], ypos = e(ParamId::WtY) + acc[kPortWtY];
        const double inst = inc + (pm - prevPmWT_) / kTwoPi;
        zSpecActive_ = (zSpectral(zt[0]) && zv[0] > 0.0) || (zSpectral(zt[1]) && zv[1] > 0.0);
        if (spectralActive() || zSpecActive_) spectralUpdate(xpos, ypos, std::fabs(inc) * slope, zt, zv, zs);
        else smValid_ = false;
        y = readWt(ph, inst, slope, xpos, ypos);
        if (syncFade_ > 0) {  // fondu court depuis la trajectoire d'avant la sync (anti-repliement de la discontinuité)
          double gp = syncGhostPhase_ + pm / kTwoPi;
          gp -= std::floor(gp);
          const double yo = readWt(gp, inst, slope, xpos, ypos);
          const double t = 1.0 - static_cast<double>(syncFade_) / (syncFadeLength_ + 1);
          y = yo + t * (y - yo);
          syncGhostPhase_ += inc;
          syncGhostPhase_ -= std::floor(syncGhostPhase_);
          --syncFade_;
        }
        if (anyZ) {
          const double pre = y;
          for (int s = 0; s < 2; ++s)
            if (zt[s] == 1 && zv[s] > 0.0) {  // Harmonic Fold, ADAA d'ordre 1
              const double u = y * (1.0 + 7.0 * zv[s]);
              const double du = u - foldPrev_[static_cast<size_t>(s)];
              y = std::fabs(du) > 1e-9 ? (triFoldF(u) - triFoldF(foldPrev_[static_cast<size_t>(s)])) / du
                                        : triFold(0.5 * (u + foldPrev_[static_cast<size_t>(s)]));
              foldPrev_[static_cast<size_t>(s)] = u;
            }
          y *= ampMul;
          if (ch(ParamId::WtZGain) == 1) {  // compensation de gain : rapport des énergies lissées (50 ms)
            const double a = 1.0 - std::exp(-1.0 / (0.05 * fsE));
            zRmsIn_ += a * (pre * pre - zRmsIn_);
            zRmsOut_ += a * (y * y - zRmsOut_);
            const double gc = zRmsOut_ > 1e-12 ? std::sqrt(zRmsIn_ / zRmsOut_) : 1.0;
            y *= clampd(gc, 0.25, 4.0);
          }
        }
        if (chr > 0.0 && cmode == 1) {
          const double L = std::exp2(16.0 - 14.0 * chr);
          y = std::round(y * L) / L;
        } else if (chr > 0.0 && cmode == 2) {
          const int hold = 1 + static_cast<int>(63.0 * chr);
          if (charCounter_ <= 0) {
            charHold_ = y;
            charCounter_ = hold;
          }
          --charCounter_;
          y = charHold_;
        }
        prevPmWT_ = pm;
        // avance de phase et détection de cycle (sync, Spectral Mutation)
        lastInc_[kNodeWT] = inc;
        double p = oscPhase_[kNodeWT] + inc;
        double mutInc = 0.0;
        if (anyZ && ch(ParamId::WtZFund) == 0)
          for (int s = 0; s < 2; ++s)
            if (zt[s] == 3) mutInc += 0.06 * zv[s] * zWalkSpeed_ * inc;
        p += mutInc;
        wrapNow[kNodeWT] = p >= 1.0 || p < 0.0;
        if (wrapNow[kNodeWT]) {
          zWalkSpeed_ = clampd(0.9 * zWalkSpeed_ + 0.3 * mutRng_.bipolar(), -1.0, 1.0);
          zWalkAmp_ = clampd(0.9 * zWalkAmp_ + 0.3 * mutRng_.bipolar(), -1.0, 1.0);
        }
        oscPhase_[kNodeWT] = p - std::floor(p);
      }
      out_[kNodeWT] = y;
      modOut_[kNodeWT] = y;
      const double lvl = clampd(e(ParamId::WtLevel) + acc[kPortWtLevel], 0.0, 1.0);
      double bankGain = 1.0;
      if (pendingBank_) bankGain = static_cast<double>(bankFade_) / bankFadeLength_;
      else if (bankFade_ > 0) bankGain = 1.0 - static_cast<double>(bankFade_) / bankFadeLength_;
      wtAudio = lvl * y * bankGain;
    } else if (node == kNodeBUS) {
      double sum = 0.0, norm = 0.0;
      for (int n = 1; n <= kNumOps; ++n) {
        const double w = eff_[static_cast<size_t>(idx(ParamId::CrossBusW1) + n - 1)];
        sum += w * modOut_[static_cast<size_t>(n)];
        norm += std::fabs(w);
      }
      if (ch(ParamId::CrossBusMode) == 1) sum /= std::max(1.0, norm);
      out_[kNodeBUS] = modOut_[kNodeBUS] = sum;
    } else {
      const int n = node;
      const int o = opTuneIndex(n);
      // décalages relatifs à opN.tune : 0 tune,1 fixed,2 offset,3 coarse,4 fine,5 wave,6 wt_x,7 wt_y,8 pan,9 mute,
      // 10 enable,11 char,12 phase_mode,13 ac,14 mode,15 sync
      const bool enabled = eff_[static_cast<size_t>(o + 10)] >= 0.5;
      if (!enabled) {
        out_[static_cast<size_t>(n)] = modOut_[static_cast<size_t>(n)] = 0.0;
        continue;
      }
      if (eff_[static_cast<size_t>(o + 15)] >= 0.5 && wrapped_[kNodeWT]) oscPhase_[static_cast<size_t>(n)] = 0.0;
      double m = acc[static_cast<size_t>(n)];
      const int mode = static_cast<int>(eff_[static_cast<size_t>(o + 14)] + 0.5);
      const double f = opF[static_cast<size_t>(n)];
      double inc = f / fsE, pm = 0.0;
      if (mode == 0) {
        pm = m;
        if (pm > kPmBound) {
          pm = kPmBound;
          ++pmClampHits_;
        } else if (pm < -kPmBound) {
          pm = -kPmBound;
          ++pmClampHits_;
        }
      } else if (mode == 3) {
        inc = f * std::exp2(clampd(m, -kExpBound, kExpBound)) / fsE;
      } else {
        double fi = f * (1.0 + clampd(m, -kLinBound, kLinBound));
        if (mode == 1 && fi < 0.0) fi = 0.0;
        inc = fi / fsE;
      }
      const int wave = std::min(8, std::max(0, static_cast<int>(eff_[static_cast<size_t>(o + 5)] + 0.5)));
      const double chr = eff_[static_cast<size_t>(o + 11)] * e(ParamId::FmCharMaster);
      if (!std::isfinite(pm) || !std::isfinite(inc)) {
        y = std::numeric_limits<double>::quiet_NaN();
      } else if (wave == 0 && chr <= 0.0) {
        y = std::sin(kTwoPi * oscPhase_[static_cast<size_t>(n)] + pm);
      } else {
        double ph = oscPhase_[static_cast<size_t>(n)] + pm / kTwoPi;
        ph -= std::floor(ph);
        if (chr > 0.0) {
          const double steps = std::floor(2048.0 * std::exp2(-8.0 * std::min(chr, 1.0)));
          ph = std::floor(ph * steps) / steps;
        }
        const double inst = inc + (pm - prevPmOp_[static_cast<size_t>(n)]) / kTwoPi;
        y = wave == 0 ? std::sin(kTwoPi * ph) : readOpWave(n - 1, wave, ph, inst);
      }
      prevPmOp_[static_cast<size_t>(n)] = pm;
      out_[static_cast<size_t>(n)] = y;
      double mo = y;
      if (eff_[static_cast<size_t>(o + 13)] >= 0.5) {  // couplage AC de la sortie de modulation (option)
        const double a = 1.0 - std::exp(-kTwoPi * kAcCutoffHz / fsE);
        acState_[static_cast<size_t>(n)] += a * (y - acState_[static_cast<size_t>(n)]);
        mo = y - acState_[static_cast<size_t>(n)];
      }
      modOut_[static_cast<size_t>(n)] = mo;
      lastInc_[static_cast<size_t>(n)] = inc;
      double p = oscPhase_[static_cast<size_t>(n)] + inc;
      wrapNow[static_cast<size_t>(n)] = p >= 1.0 || p < 0.0;
      oscPhase_[static_cast<size_t>(n)] = p - std::floor(p);
      // sortie audio : niveau et panoramique distincts du niveau de modulation
      static constexpr ParamId kLevel[4] = {ParamId::Op1Level, ParamId::Op2Level, ParamId::Op3Level, ParamId::Op4Level};
      if (eff_[static_cast<size_t>(o + 9)] < 0.5) {
        const double a = e(kLevel[n - 1]) * y;
        double gl, gr;
        panGains(eff_[static_cast<size_t>(o + 8)], gl, gr);
        fmL += a * gl;
        fmR += a * gr;
      }
    }
  }
  if (sawRemoved && sawAdded) ++overlapSamples_;
  for (int s = 0; s < kNumNodes; ++s) lines_[static_cast<size_t>(s)].write(modOut_[static_cast<size_t>(s)]);
  // maître de sync interne (fréquence de la note)
  syncMasterPhase_ += f0 / fsE;
  syncMasterWrapped_ = syncMasterPhase_ >= 1.0;
  syncMasterPhase_ -= std::floor(syncMasterPhase_);
  wrapped_ = wrapNow;
}

void Engine::tick(double& L, double& R) noexcept {
  updateSmoothing();
  updateSources();
  evaluateModulation();
  double wt = 0.0, fmL = 0.0, fmR = 0.0;
  evaluateNetwork(wt, fmL, fmR);
  if (pendingBank_) {  // fondu de sortie terminé : bascule de banque
    if (--bankFade_ <= 0) {
      BankRef* old = activeBankRef_;
      activeBankRef_ = pendingBank_;
      pendingBank_ = nullptr;
      bank_ = activeBankRef_->bank.get();
      if (old) bankExchange_.retire(old);
      bankFade_ = bankFadeLength_;
      bankFadingOut_ = false;
    }
  } else if (bankFade_ > 0) {
    --bankFade_;
  }
  // Amp avant tout traitement capable de produire une queue (P0-C01)
  const double env = src_[static_cast<size_t>(ModSource::EnvAmp)];
  const double velAmount = e(ParamId::AmpVelocity);
  const double velGain = 1.0 - velAmount + velAmount * (velocity_ / 127.0);
  const double g = env * velGain;
  double wL = wt * g, wR = wL;
  double fL = fmL * g, fR = fmR * g;
  // Balance WT/FM (Performance) : gains audio des moteurs, sans effet sur la modulation, la topologie ni les phases.
  // Centre = mixage existant (aucune opération) ; -1 = WT seule ; +1 = FM seul ; jamais d'amplification (gains <= 1).
  const double bal = e(ParamId::PerfBalance);
  if (bal > 0.0) {
    wL *= 1.0 - bal;
    wR *= 1.0 - bal;
  } else if (bal < 0.0) {
    fL *= 1.0 + bal;
    fR *= 1.0 + bal;
  }
  ChainContext cc;
  cc.fs = setup_.engineRate;
  cc.noteHz = f0_;
  cc.gate = gate_;
  cc.voiceIdle = !envs_[0].active();
  cc.manualExcite = manualExcitePending_;
  cc.noteOnEdge = noteOnEdge_;
  manualExcitePending_ = false;
  noteOnEdge_ = false;
  cc.bpm = transport_.bpm;
  cc.quality = setup_.quality;
  chainWt_.process(wL, wR, eff_.data(), cc);
  chainFm_.process(fL, fR, eff_.data(), cc);
  // alignement des chaînes locales avant Cross Audio (latences de S2), puis complément au multiple de K
  const int lw = chainWt_.latency(), lf = chainFm_.latency(), lmax = std::max(lw, lf);
  if (lmax > 0) {
    alignWtL_.process(wL, lmax - lw);
    alignWtR_.process(wR, lmax - lw);
    alignFmL_.process(fL, lmax - lf);
    alignFmR_.process(fR, lmax - lf);
  }
  followers_[0].process(0.5 * (wL + wR));
  followers_[1].process(0.5 * (fL + fR));
  // Cross Audio (après chaînes locales alignées, avant global)
  const int am = ch(ParamId::CrossAudioMode);
  const double amt = e(ParamId::CrossAudioAmount);
  double oL, oR;
  switch (am) {
    case 1: {  // Crossfade linéaire : centre = mix, extrémités = moteur seul à gain unité
      const double gw = std::min(1.0, 2.0 * (1.0 - amt)), gf = std::min(1.0, 2.0 * amt);
      oL = gw * wL + gf * fL;
      oR = gw * wR + gf * fR;
      break;
    }
    case 2: {  // Ring canal par canal, référence de gain fixe (x2)
      oL = (1.0 - amt) * (wL + fL) + amt * 2.0 * wL * fL;
      oR = (1.0 - amt) * (wR + fR) + amt * 2.0 * wR * fR;
      break;
    }
    case 3: {  // AM, direction sélectionnée, référence de gain fixe
      const bool wtToFm = ch(ParamId::CrossAmDir) == 0;
      if (wtToFm) {
        oL = wL + fL * (1.0 + amt * 2.0 * wL) / (1.0 + amt);
        oR = wR + fR * (1.0 + amt * 2.0 * wR) / (1.0 + amt);
      } else {
        oL = fL + wL * (1.0 + amt * 2.0 * fL) / (1.0 + amt);
        oR = fR + wR * (1.0 + amt * 2.0 * fR) / (1.0 + amt);
      }
      break;
    }
    default:
      oL = wL + fL;
      oR = wR + fR;
      break;
  }
  chainGl_.process(oL, oR, eff_.data(), cc);
  const int le = lmax + chainGl_.latency(), K = setup_.oversampling;
  const int pad = (K - le % K) % K;  // latence totale entière en échantillons hôte
  if (pad > 0 || padActive_) {
    padActive_ = pad > 0;
    alignOutL_.process(oL, pad);
    alignOutR_.process(oR, pad);
  }
  followers_[2].process(0.5 * (oL + oR));
  L = oL;
  R = oR;
}

void Engine::beginTransition(CompiledTopology* incoming) noexcept {
  incoming_ = incoming;
  bool anyRemoved = false;
  for (int ed = 0; ed < active_->numEdges; ++ed) {
    const bool removed = !containsEdge(*incoming_, active_->edges[static_cast<size_t>(ed)]);
    edgeRemoved_[static_cast<size_t>(ed)] = removed;
    edgeAdded_[static_cast<size_t>(ed)] = false;
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
  for (int ed = 0; ed < active_->numEdges; ++ed) {
    const bool survivor = containsEdge(*old, active_->edges[static_cast<size_t>(ed)]);
    edgeGain_[static_cast<size_t>(ed)] = survivor ? 1.0 : 0.0;
    edgeAdded_[static_cast<size_t>(ed)] = !survivor;
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
    for (int ed = 0; ed < active_->numEdges; ++ed)
      if (edgeRemoved_[static_cast<size_t>(ed)])
        edgeGain_[static_cast<size_t>(ed)] = std::max(0.0, edgeGain_[static_cast<size_t>(ed)] - step);
    if (--transitionLeft_ <= 0) {
      for (int ed = 0; ed < active_->numEdges; ++ed)
        if (edgeRemoved_[static_cast<size_t>(ed)]) edgeGain_[static_cast<size_t>(ed)] = 0.0;
      swapToIncoming();  // le nouveau graphe est évalué dès cet échantillon, arêtes ajoutées à gain 0
    }
  } else {
    for (int ed = 0; ed < active_->numEdges; ++ed)
      if (edgeAdded_[static_cast<size_t>(ed)])
        edgeGain_[static_cast<size_t>(ed)] = std::min(1.0, edgeGain_[static_cast<size_t>(ed)] + step);
    if (--transitionLeft_ <= 0) {
      for (int ed = 0; ed < active_->numEdges; ++ed)
        if (edgeAdded_[static_cast<size_t>(ed)]) edgeGain_[static_cast<size_t>(ed)] = 1.0;
      edgeAdded_.fill(false);
      transition_ = Transition::Idle;
    }
  }
}

// ---------------------------------------------------------------- MIDI / voix
void Engine::updateF0() noexcept {
  const double a4 = e(ParamId::TuneA4);
  double detune = 0.0;
  if (ch(ParamId::TuneScale) == 1) detune = eff_[static_cast<size_t>(idx(ParamId::TunePc0) + (currentNote_ % 12))] / 100.0;
  double f = a4 * std::pow(2.0, (currentNote_ - 69 + detune) / 12.0);
  if (bend_ != 0.0) {
    const double semis = bend_ > 0.0 ? bend_ * e(ParamId::BendUp) : bend_ * e(ParamId::BendDown);
    f *= std::exp2(semis / 12.0);
  }
  f0_ = f;
  src_[static_cast<size_t>(ModSource::Note)] = (currentNote_ - 60) / 60.0;
}

int Engine::selectNote() const noexcept {
  const int pr = ch(ParamId::VoicePriority);
  int n = stack_[static_cast<size_t>(stackSize_ - 1)];
  if (pr == 1)
    for (int i = 0; i < stackSize_; ++i) n = std::min(n, static_cast<int>(stack_[static_cast<size_t>(i)]));
  else if (pr == 2)
    for (int i = 0; i < stackSize_; ++i) n = std::max(n, static_cast<int>(stack_[static_cast<size_t>(i)]));
  return n;
}

void Engine::triggerVoice(bool retrigger, bool gateOpened, bool newNote) noexcept {
  // Déclenchement par enveloppe : Voice = règle de la voix (Mono/Legato, défaut, comportement 0.2.0), Note = toute
  // nouvelle note, Gate = ouverture du gate seulement. Relance toujours depuis le niveau courant (aucun saut).
  static constexpr ParamId kTrig[5] = {ParamId::AmpTrigger, ParamId::EnvWtTrigger, ParamId::EnvFmTrigger, ParamId::EnvAux1Trigger, ParamId::EnvAux2Trigger};
  for (int k = 0; k < 5; ++k) {
    const int m = ch(kTrig[k]);
    const bool go = m == 1 ? newNote : (m == 2 ? gateOpened : retrigger);
    if (go) envs_[static_cast<size_t>(k)].noteOn();
  }
  if (!retrigger) return;
  noteOnEdge_ = true;  // excitation « Note » des filtres/résonateurs
  for (int k = 0; k < 4; ++k) {
    const int o = idx(ParamId::Lfo1Shape) + 6 * k;
    if (eff_[static_cast<size_t>(o + 5)] >= 0.5) lfos_[static_cast<size_t>(k)].resetPhase(0.0);
  }
  switch (ch(ParamId::WtPhaseMode)) {
    case 1: oscPhase_[kNodeWT] = 0.0; break;
    case 2: oscPhase_[kNodeWT] = e(ParamId::WtPhase); break;
    case 3: oscPhase_[kNodeWT] = phaseRng_.uniform(); break;
    default: break;
  }
  for (int n = 1; n <= kNumOps; ++n) {
    const int o = opTuneIndex(n);
    const int pm = static_cast<int>(eff_[static_cast<size_t>(o + 12)] + 0.5);
    if (pm == 1) oscPhase_[static_cast<size_t>(n)] = 0.0;
    if (pm == 2) oscPhase_[static_cast<size_t>(n)] = phaseRng_.uniform();
  }
  if (ch(ParamId::FmFbMode) == 1)  // Note Reset : historiques de retroaction FM remis à zéro
    for (int n = 1; n < kNumNodes; ++n) lines_[static_cast<size_t>(n)].reset();
}

void Engine::handleEvent(const MidiEvent& ev) noexcept {
  switch (ev.type) {
    case MidiEvent::Type::AllSoundOff:  // Panic (P0-C06) : rampe, purge, Drone désarmé
      stackSize_ = 0;
      sustained_.fill(false);
      gate_ = false;
      drone_ = false;
      // Panic (ARCH-G4.3, RF-01) : révocation systématique et verrou ; la configuration reste intacte
      auth_.panic(taClock_, taLog_);
      captureApplyPending_ = false;
      applyAutonomyMask();
      for (auto& en : envs_) en.noteOff();
      panicActive_ = true;
      panicLeft_ = panicLength_;
      ++panics_;
      break;
    case MidiEvent::Type::AllNotesOff:
      stackSize_ = 0;
      sustained_.fill(false);
      if (!droneEff()) {
        gate_ = false;
        for (auto& en : envs_) en.noteOff();
      }
      break;
    case MidiEvent::Type::NoteOff: noteOff(ev.note); break;
    case MidiEvent::Type::NoteOn:
      if (ev.velocity == 0)
        noteOff(ev.note);
      else
        noteOn(ev.note, ev.velocity);
      break;
    case MidiEvent::Type::PitchBend:
      bend_ = ev.value >= 0 ? ev.value / 8191.0 : ev.value / 8192.0;
      src_[static_cast<size_t>(ModSource::PitchBend)] = bend_;
      updateF0();
      break;
    case MidiEvent::Type::ChannelPressure: src_[static_cast<size_t>(ModSource::Pressure)] = ev.value / 127.0; break;
    case MidiEvent::Type::Controller: {
      const double v = ev.value / 127.0;
      if (ev.note == 1) src_[static_cast<size_t>(ModSource::ModWheel)] = v;
      if (ev.note == 2) src_[static_cast<size_t>(ModSource::Breath)] = v;
      if (ev.note == 11) src_[static_cast<size_t>(ModSource::Expression)] = v;
      if (ev.note == 64) {
        sustainPedal_ = ev.value >= 64;
        src_[static_cast<size_t>(ModSource::Sustain)] = sustainPedal_ ? 1.0 : 0.0;
        if (!sustainPedal_) {
          // pédale relâchée : les notes maintenues par la pédale sont relâchées
          for (int nn = 0; nn < 128; ++nn)
            if (sustained_[static_cast<size_t>(nn)]) {
              sustained_[static_cast<size_t>(nn)] = false;
              noteOff(nn);
            }
        }
      }
      break;
    }
  }
}

void Engine::noteOn(int note, int velocity) noexcept {
  sustained_[static_cast<size_t>(note)] = false;
  for (int i = 0; i < stackSize_; ++i)
    if (stack_[static_cast<size_t>(i)] == note) {
      for (int j = i; j + 1 < stackSize_; ++j) stack_[static_cast<size_t>(j)] = stack_[static_cast<size_t>(j + 1)];
      --stackSize_;
      break;
    }
  const bool wasGate = gate_;
  if (stackSize_ < static_cast<int>(stack_.size())) stack_[static_cast<size_t>(stackSize_++)] = static_cast<std::uint8_t>(note);
  const int prev = currentNote_;
  currentNote_ = selectNote();
  const bool changed = !wasGate || currentNote_ != prev || ch(ParamId::VoicePriority) == 0;
  if (changed) velocity_ = velocity;
  gate_ = true;
  const bool legato = ch(ParamId::VoiceMode) == 1;
  // Mono : relance à chaque note qui devient la note jouée ; Legato : relance seulement sans note tenue
  const bool retrigger = legato ? !wasGate : changed;
  updateF0();
  triggerVoice(retrigger, !wasGate, true);
}

void Engine::noteOff(int note) noexcept {
  if (sustainPedal_) {  // pédale enfoncée : la note reste tenue jusqu'au relâchement de la pédale
    bool held = false;
    for (int i = 0; i < stackSize_; ++i) held = held || stack_[static_cast<size_t>(i)] == note;
    if (held) {
      sustained_[static_cast<size_t>(note)] = true;
      return;
    }
  }
  for (int i = 0; i < stackSize_; ++i)
    if (stack_[static_cast<size_t>(i)] == note) {
      for (int j = i; j + 1 < stackSize_; ++j) stack_[static_cast<size_t>(j)] = stack_[static_cast<size_t>(j + 1)];
      --stackSize_;
      break;
    }
  if (gate_ && note == currentNote_) {
    if (stackSize_ > 0) {
      currentNote_ = selectNote();  // retour à la note tenue selon la priorité, sans relance
      updateF0();
    } else {
      releaseGateIfIdle();
    }
  }
}

void Engine::releaseGateIfIdle() noexcept {
  if (droneEff() || stackSize_ > 0 || !gate_) return;  // Drone autorisé maintient le Gate normal (P0-C05)
  gate_ = false;
  for (auto& en : envs_) en.noteOff();
}

void Engine::purgeStates() noexcept {
  out_.fill(0.0);
  modOut_.fill(0.0);
  delayed_.fill(0.0);
  acState_.fill(0.0);
  prevPmWT_ = 0.0;
  prevPmOp_.fill(0.0);
  zRmsIn_ = zRmsOut_ = 0.0;
  foldPrev_.fill(0.0);
  for (auto& l : lines_) l.reset();
  for (auto& p : oscPhase_)
    if (!std::isfinite(p)) p = 0.0;
  decL_.reset();
  decR_.reset();
  for (auto& d : dc_) d.reset();
  for (auto& f : followers_) f.reset();
  limGain_ = 1.0;
}

void Engine::crossReset() noexcept {
  // Cross Reset : purge des historiques des routes CROSS uniquement ; phases, rétroaction FM interne, FX et Drone
  // conservés. Les historiques des sources sont partagés : on purge WT (source CROSS avant) et BUS ; les opérateurs
  // ne sont purgés que s'ils n'alimentent aucune arête Delayed interne.
  const CompiledTopology& T = *active_;
  std::array<bool, kNumNodes> internalDelayed{};
  for (int ed = 0; ed < T.numEdges; ++ed) {
    const auto& E = T.edges[static_cast<size_t>(ed)];
    if (E.delayed && routeSlotInfo(E.slot).kind == RouteKind::Internal) internalDelayed[E.src] = true;
  }
  for (int s = 0; s < kNumNodes; ++s)
    if (!internalDelayed[static_cast<size_t>(s)]) lines_[static_cast<size_t>(s)].reset();
  prevPmWT_ = 0.0;
}

void Engine::publishTelemetry() noexcept {
  auto& t = telemetry_;
  t.currentNote.store(gate_ ? currentNote_ : -1, std::memory_order_relaxed);
  t.gate.store(gate_, std::memory_order_relaxed);
  t.drone.store(drone_, std::memory_order_relaxed);
  t.autonomySuspended.store(!auth_.authorized() && (anyArmConfigured() || drone_), std::memory_order_relaxed);
  t.autonomyAuthorized.store(auth_.authorized(), std::memory_order_relaxed);
  t.autonomyLatched.store(auth_.latched(), std::memory_order_relaxed);
  t.autonomySource.store(static_cast<int>(auth_.source()), std::memory_order_relaxed);
  t.transportState.store(static_cast<int>(auth_.lastKnown()), std::memory_order_relaxed);
  t.offline.store(auth_.offline(), std::memory_order_relaxed);
  t.authGrants.store(auth_.grants(), std::memory_order_relaxed);
  t.authRevocations.store(auth_.revocations(), std::memory_order_relaxed);
  t.authGeneration.store(auth_.generation(), std::memory_order_relaxed);
  t.taDropped.store(taLog_.dropped(), std::memory_order_relaxed);
  capturesPendingPub_.store(retained_ != nullptr && captureApplyPending_, std::memory_order_release);
  t.envelope.store(static_cast<float>(envs_[0].level()), std::memory_order_relaxed);
  t.peakL.store(blockPeakL_, std::memory_order_relaxed);
  t.peakR.store(blockPeakR_, std::memory_order_relaxed);
  t.nonFiniteResets.store(nonFinite_, std::memory_order_relaxed);
  t.outputClampHits.store(clampHits_, std::memory_order_relaxed);
  t.pmClampHits.store(pmClampHits_, std::memory_order_relaxed);
  t.droppedEvents.store(dropped_, std::memory_order_relaxed);
  t.panics.store(panics_, std::memory_order_relaxed);
  t.limiterHits.store(limiterHits_, std::memory_order_relaxed);
  t.elpHits.store(elpHits_, std::memory_order_relaxed);
  t.activeSerial.store(active_->serial, std::memory_order_relaxed);
  t.modSerial.store(mod_->serial, std::memory_order_relaxed);
  t.transitionPhase.store(static_cast<int>(transition_), std::memory_order_relaxed);
  t.wtX.store(static_cast<float>(e(ParamId::WtX)), std::memory_order_relaxed);
  t.wtY.store(static_cast<float>(e(ParamId::WtY)), std::memory_order_relaxed);
  t.wtZ.store(static_cast<float>(e(ParamId::WtZ)), std::memory_order_relaxed);
  for (int s = 0; s < kNumModSources; ++s) t.sources[static_cast<size_t>(s)].store(static_cast<float>(src_[static_cast<size_t>(s)]), std::memory_order_relaxed);
  for (int k = 0; k < mod_->numDests; ++k) {
    const auto d = static_cast<size_t>(mod_->dests[static_cast<size_t>(k)]);
    t.effective[d].store(static_cast<float>(eff_[d]), std::memory_order_relaxed);
  }
  for (int i = 0; i < kNumParams; ++i) t.modulated[static_cast<size_t>(i)].store(modulated_[static_cast<size_t>(i)] ? 1 : 0, std::memory_order_relaxed);
  for (int s = 0; s < kMaxRoutes; ++s) {
    t.effectiveDepth[static_cast<size_t>(s)].store(static_cast<float>(testEffectiveDepth(s)), std::memory_order_relaxed);
    float g = 0.0f;
    for (int ed = 0; ed < active_->numEdges; ++ed)
      if (active_->edges[static_cast<size_t>(ed)].slot == s) g = static_cast<float>(edgeGain_[static_cast<size_t>(ed)]);
    t.edgeGain[static_cast<size_t>(s)].store(g, std::memory_order_relaxed);
  }
}

}  // namespace sbd
