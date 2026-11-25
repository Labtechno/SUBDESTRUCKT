// SUBDESTRUCKT — Spike S0.1 — moteur du réseau récursif FM/CROSS (harness, hors VST3/JUCE).
// 5 nœuds : 0 = WT (table réelle), 1..4 = OP1..OP4 (sinus par défaut, forme configurable).
// Mode de modulation par destinataire (baseline §6) : PM (rad), Linear FM (Hz, fréquences négatives
// bloquées), TZFM (Hz, la phase recule), Exponential FM (octaves, sans compensation de pitch).
// Bornes cumulées candidates du baseline : ±32 rad, ±8000 Hz, ±4 octaves. Aucune autre compensation.
// Stratégie causale B : arêtes Immediate en ordre topologique ; arêtes Delayed lues via fracdelay.hpp.
// Le moteur tourne à fsE = K*fsH ; le chemin de rendu (Processor) décime en temps réel (resampler.hpp).
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>
#include "s01/fracdelay.hpp"
#include "s01/resampler.hpp"
#include "s01/routing.hpp"
#include "s01/wavetable.hpp"

namespace s01 {

enum class Mode { PM = 0, LinFM = 1, TZFM = 2, ExpFM = 3 };
inline const char* name(Mode m) {
  switch (m) { case Mode::PM: return "PM"; case Mode::LinFM: return "LinFM"; case Mode::TZFM: return "TZFM"; default: return "ExpFM"; }
}
constexpr double kPmClampRad = 32.0;
constexpr double kFmClampHz = 8000.0;
constexpr double kExpClampOct = 4.0;
inline double modClamp(Mode m) { return m == Mode::PM ? kPmClampRad : m == Mode::ExpFM ? kExpClampOct : kFmClampHz; }

enum class Curve { Linear = 0, Smooth = 1 };  // Smooth = smoothstep 3t²-2t³ (C1)
inline const char* name(Curve c) { return c == Curve::Linear ? "linear" : "smooth"; }

struct TableBank {
  WaveTable sine{Shape::Sine}, tri{Shape::Triangle}, saw{Shape::Saw}, asym{Shape::Asym};
  const WaveTable& get(Shape s) const {
    switch (s) { case Shape::Sine: return sine; case Shape::Triangle: return tri; case Shape::Saw: return saw; default: return asym; }
  }
  static const TableBank& instance() { static const TableBank b; return b; }  // construit hors RT
};

struct EngineParams {
  double fsHost = 48000.0;
  int K = 1;
  TimeRef ref = TimeRef::T1;
  Frac frac = Frac::Lagrange3;
  std::array<double, kNodes> freq{220, 220, 440, 660, 330};
  std::array<Mode, kNodes> mode{Mode::PM, Mode::PM, Mode::PM, Mode::PM, Mode::PM};
  std::array<Shape, kNodes> shape{Shape::Saw, Shape::Sine, Shape::Sine, Shape::Sine, Shape::Sine};
  std::array<double, kNodes> mix{0.2, 0.2, 0.2, 0.2, 0.2};
  std::array<double, kNodes> phase0{};
  bool tableForSine = false;  // OP sinus : std::sin exact (false) ou table (true)
  bool poison = false;        // NaN dans les sorties courantes non encore calculées (preuve de causalité)
};

struct EngineStats {
  double maxAbsHz = 0;     // |fréquence instantanée| max (Hz) — bornes pitch/fréquence
  long long clampHits = 0; // échantillons où la somme de modulation a atteint la borne
  long long negBlocked = 0;// LinFM : fréquence négative bloquée à 0
  long long tzReverse = 0; // TZFM : phase rétrograde
  double maxAbsDelayed = 0;// |valeur retardée| max (dépassement d'interpolation)
};

class Engine {
 public:
  explicit Engine(const EngineParams& p) : p_(p), bank_(&TableBank::instance()) {
    fsE_ = p.fsHost * p.K;
    try { delay_ = makeDelay(p.ref, p.frac, p.K, p.fsHost); delayOk_ = true; }
    catch (const T2NotRepresentable&) { delayOk_ = false; }
    for (int i = 0; i < kNodes; ++i) {
      inc_[i] = p.freq[i] / fsE_;
      useTable_[i] = (i == 0) || p.shape[i] != Shape::Sine || p.tableForSine;
      tab_[i] = &bank_->get(p.shape[i]);
    }
    reset();
  }
  // Panic/reset : purge phases, historiques, états fractionnaires et rampes.
  void reset() {
    for (int i = 0; i < kNodes; ++i) { ph_[i] = p_.phase0[i]; prevRead_[i] = p_.phase0[i]; }
    for (auto& r : ring_) r.fill(0.0);
    cur_.fill(0.0); dv_.fill(0.0); thiran_.fill(0.0);
    depth_.fill(0.0); rStart_.fill(0.0); rTgt_.fill(0.0); rLen_.fill(0); rPos_.fill(0);
    rCurve_.fill(Curve::Linear);
    anyRamp_ = false;
    n_ = 0;
    st_ = EngineStats{};
  }
  bool delayRepresentable() const { return delayOk_; }
  const DelaySpec& delay() const { return delay_; }
  double fsEngine() const { return fsE_; }
  const EngineStats& stats() const { return st_; }
  long long position() const { return n_; }
  double depth(int slot) const { return depth_[slot]; }

  // Transaction : remplacement atomique de la topologie (frontière d'échantillon).
  void apply(const Compiled& c) {
    if (c.nDelayed > 0 && !delayOk_) throw T2NotRepresentable("arete Delayed active avec T2 non representable (K insuffisant)");
    c_ = c;
  }
  void setDepthNow(int slot, double v) { depth_[slot] = v; rTgt_[slot] = v; rLen_[slot] = 0; }
  void rampDepth(int slot, double target, int lenEng, Curve cv) {
    if (lenEng <= 0) { setDepthNow(slot, target); return; }
    rStart_[slot] = depth_[slot]; rTgt_[slot] = target; rLen_[slot] = lenEng; rPos_[slot] = 0; rCurve_[slot] = cv;
    anyRamp_ = true;
  }

  // n échantillons moteur -> out (mix mono). nodeOut optionnel : n*kNodes.
  void process(double* out, int n, double* nodeOut = nullptr) {
    for (int s = 0; s < n; ++s, ++n_) {
      if (p_.poison) cur_.fill(std::numeric_limits<double>::quiet_NaN());
      if (anyRamp_) stepRamps();
      readDelayedSources();
      for (int oi = 0; oi < kNodes; ++oi) {
        const int i = c_.order[oi];
        double m = 0.0;
        for (int e = c_.off[i]; e < c_.off[i + 1]; ++e) {
          const double v = c_.eDelayed[e] ? dv_[c_.eSrc[e]] : cur_[c_.eSrc[e]];
          m += depth_[c_.eSlot[e]] * v;
        }
        cur_[i] = tick(i, m);
      }
      const int w = int(n_ & (kRing - 1));
      double acc = 0.0;
      for (int i = 0; i < kNodes; ++i) { ring_[i][w] = cur_[i]; acc += p_.mix[i] * cur_[i]; }
      out[s] = acc;
      if (nodeOut) for (int i = 0; i < kNodes; ++i) nodeOut[size_t(s) * kNodes + i] = cur_[i];
    }
  }

 private:
  void readDelayedSources() {
    const bool all = !delay_.integer && delay_.frac == Frac::Thiran1;  // état IIR continu pour toutes les sources
    for (int src = 0; src < kNodes; ++src) {
      if (!all && !c_.srcHasDelayed[src]) continue;
      const double v = readDelay(delay_, ring_[src].data(), n_, thiran_[src]);
      dv_[src] = v;
      const double a = std::fabs(v);
      if (a > st_.maxAbsDelayed) st_.maxAbsDelayed = a;
    }
  }

  double tick(int i, double m) {
    const Mode md = p_.mode[i];
    const double lim = modClamp(md);
    if (m > lim) { m = lim; ++st_.clampHits; }
    else if (m < -lim) { m = -lim; ++st_.clampHits; }
    double readPh, finc;
    switch (md) {
      case Mode::PM: {
        ph_[i] += inc_[i];
        ph_[i] -= std::floor(ph_[i]);
        readPh = ph_[i] + m * (1.0 / (2.0 * kPi));
        readPh -= std::floor(readPh);
        double d = readPh - prevRead_[i];
        d -= std::floor(d + 0.5);
        finc = std::fabs(d);
        prevRead_[i] = readPh;
        noteHz(finc * fsE_);
        break;
      }
      case Mode::LinFM: {
        double f = p_.freq[i] + m;
        if (f < 0.0) { f = 0.0; ++st_.negBlocked; }
        ph_[i] += f / fsE_;
        ph_[i] -= std::floor(ph_[i]);
        readPh = ph_[i]; finc = f / fsE_;
        noteHz(f);
        break;
      }
      case Mode::TZFM: {
        const double f = p_.freq[i] + m;
        if (f < 0.0) ++st_.tzReverse;
        ph_[i] += f / fsE_;
        ph_[i] -= std::floor(ph_[i]);
        readPh = ph_[i]; finc = std::fabs(f) / fsE_;
        noteHz(std::fabs(f));
        break;
      }
      case Mode::ExpFM: default: {
        const double f = p_.freq[i] * std::exp2(m);
        ph_[i] += f / fsE_;
        ph_[i] -= std::floor(ph_[i]);
        readPh = ph_[i]; finc = f / fsE_;
        noteHz(f);
        break;
      }
    }
    if (ph_[i] >= 1.0) ph_[i] = 0.0;   // garde floor(x)=x-1 sur arrondi
    if (readPh >= 1.0) readPh = 0.0;
    return useTable_[i] ? tab_[i]->read(readPh, finc) : std::sin(2.0 * kPi * readPh);
  }
  void noteHz(double f) { if (f > st_.maxAbsHz) st_.maxAbsHz = f; }

  void stepRamps() {
    bool any = false;
    for (int s = 0; s < kMaxRoutes; ++s) {
      if (rLen_[s] <= 0) continue;
      ++rPos_[s];
      if (rPos_[s] >= rLen_[s]) { depth_[s] = rTgt_[s]; rLen_[s] = 0; continue; }
      double t = double(rPos_[s]) / rLen_[s];
      if (rCurve_[s] == Curve::Smooth) t = t * t * (3.0 - 2.0 * t);
      depth_[s] = rStart_[s] + (rTgt_[s] - rStart_[s]) * t;
      any = true;
    }
    anyRamp_ = any;
  }

  EngineParams p_;
  const TableBank* bank_;
  double fsE_ = 48000;
  DelaySpec delay_{};
  bool delayOk_ = true;
  Compiled c_{};
  std::array<double, kNodes> inc_{}, ph_{}, prevRead_{}, cur_{}, dv_{}, thiran_{};
  std::array<bool, kNodes> useTable_{};
  std::array<const WaveTable*, kNodes> tab_{};
  std::array<std::array<double, kRing>, kNodes> ring_{};
  std::array<double, kMaxRoutes> depth_{}, rStart_{}, rTgt_{};
  std::array<int, kMaxRoutes> rLen_{}, rPos_{};
  std::array<Curve, kMaxRoutes> rCurve_{};
  bool anyRamp_ = false;
  long long n_ = 0;
  EngineStats st_{};
};

// ------------------------------------------------------------------------------------------------
// Rendu hôte : blocs hôte, événements à l'échantillon hôte exact, moteur K× + décimation temps réel.
// ------------------------------------------------------------------------------------------------
struct DepthCmd { int slot; double target; int rampHost; Curve curve = Curve::Linear; };

struct Event {
  long long hostSample = 0;
  bool hasTopology = false;
  Compiled compiled{};
  std::vector<DepthCmd> depths;
};

struct RenderSpec {
  EngineParams params;
  Compiled initial{};
  std::vector<DepthCmd> initialDepths;
  std::vector<Event> events;  // triés par hostSample
  long long hostSamples = 0;
};

struct RenderOut {
  std::vector<double> host;     // sortie hôte après décimation temps réel
  std::vector<double> engine;   // sortie moteur (avant décimation) si demandée
  std::vector<double> nodes;    // sorties par nœud au rythme moteur si demandées
  EngineStats stats{};
  int latencyHost = 0;
};

// Processeur temps réel : buffers préalloués à la construction, aucune allocation dans processBlock.
class Processor {
 public:
  Processor(const EngineParams& p, int maxBlockHost)
      : eng_(p), dec_(p.K), K_(p.K), maxBlock_(maxBlockHost), scratch_(size_t(maxBlockHost) * p.K, 0.0) {}
  Engine& engine() { return eng_; }
  const Decimator& decimator() const { return dec_; }
  void reset() { eng_.reset(); dec_.reset(); }
  // Produit nHost échantillons hôte. engineOut (optionnel) reçoit nHost*K échantillons moteur.
  void processBlock(double* hostOut, int nHost, double* engineOut = nullptr, double* nodeOut = nullptr) {
    eng_.process(scratch_.data(), nHost * K_, nodeOut);
    if (engineOut) std::copy(scratch_.begin(), scratch_.begin() + size_t(nHost) * K_, engineOut);
    dec_.process(scratch_.data(), hostOut, nHost);
  }
  int maxBlock() const { return maxBlock_; }

 private:
  Engine eng_;
  Decimator dec_;
  int K_, maxBlock_;
  std::vector<double> scratch_;
};

inline void applyEvent(Engine& eng, const Event& e, int K) {
  if (e.hasTopology) eng.apply(e.compiled);
  for (const DepthCmd& d : e.depths) eng.rampDepth(d.slot, d.target, d.rampHost * K, d.curve);
}

inline RenderOut render(const RenderSpec& spec, int blockHost, bool keepEngine = false, bool keepNodes = false) {
  Processor pr(spec.params, blockHost);
  Engine& eng = pr.engine();
  const int K = spec.params.K;
  eng.apply(spec.initial);
  for (const DepthCmd& d : spec.initialDepths) eng.rampDepth(d.slot, d.target, d.rampHost * K, d.curve);
  RenderOut ro;
  ro.host.assign(size_t(spec.hostSamples), 0.0);
  if (keepEngine) ro.engine.assign(size_t(spec.hostSamples) * K, 0.0);
  if (keepNodes) ro.nodes.assign(size_t(spec.hostSamples) * K * kNodes, 0.0);
  size_t ev = 0;
  long long pos = 0;
  while (pos < spec.hostSamples) {
    const long long blockEnd = std::min<long long>(pos + blockHost, spec.hostSamples);
    while (pos < blockEnd) {  // un bloc hôte peut être découpé aux événements (sample-accurate)
      while (ev < spec.events.size() && spec.events[ev].hostSample <= pos) applyEvent(eng, spec.events[ev++], K);
      long long segEnd = blockEnd;
      if (ev < spec.events.size()) segEnd = std::min(segEnd, spec.events[ev].hostSample);
      const int n = int(segEnd - pos);
      pr.processBlock(ro.host.data() + pos, n, keepEngine ? ro.engine.data() + pos * K : nullptr,
                      keepNodes ? ro.nodes.data() + size_t(pos) * K * kNodes : nullptr);
      pos = segEnd;
    }
  }
  ro.stats = eng.stats();
  ro.latencyHost = pr.decimator().latencyHost();
  return ro;
}

}  // namespace s01
