// SUBDESTRUCKT — Spike S0 (causalité FM/CROSS) — harness DSP minimal.
// Hors VST3, hors JUCE. Code de recherche : ne pas réutiliser tel quel en produit.
//
// Réseau : 5 nœuds sinus (0 = "WT" simplifié, 1..4 = OP1..OP4), modulation de
// phase (PM) en radians, matrice 5x5 (route = dst*5+src ; src==dst = feedback).
//
// Stratégies causales comparées :
//   A  : toute route lit une valeur retardée (retard uniforme).
//   B  : seules les routes qui ferment un cycle sont retardées ; les autres sont
//        évaluées dans le même échantillon, en ordre topologique.
//        Choix des routes retardées : glouton par priorité de création (prio croissante).
//   Bc : variante de B, classification canonique par DFS (ordre d'indices), sans priorité.
//
// Références temporelles :
//   T1 : retard = 1 échantillon hôte (= K échantillons moteur à K× oversampling).
//   T2 : retard = 1/48000 s ; délai fractionnaire causal (Lagrange cubique sur
//        y[n-d0..n-d0-3], d0>=1) ; plancher à 1 échantillon moteur (signalé).
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace s0 {

constexpr int kNodes = 5;
constexpr int kMaxRoutes = kNodes * kNodes;
constexpr int kRing = 64;  // puissance de 2, > délai max (8 éch. moteur + marge)
constexpr double kPi = 3.14159265358979323846;
constexpr double kT2Seconds = 1.0 / 48000.0;
constexpr double kPmClamp = 32.0;  // borne cumulée candidate (baseline §6)

enum class Strategy { A, B, Bc };
enum class TimeRef { T1, T2 };

inline const char* name(Strategy s) { return s == Strategy::A ? "A" : s == Strategy::B ? "B" : "Bc"; }
inline const char* name(TimeRef t) { return t == TimeRef::T1 ? "T1" : "T2"; }

inline int slotOf(int dst, int src) { return dst * kNodes + src; }

struct Route {
  int dst;
  int src;
  int prio;  // ordre de création (stable) : plus petit = plus ancien = prioritaire
};

// ----------------------------------------------------------------------------
// Compilation (hors RT)
// ----------------------------------------------------------------------------
struct Compiled {
  std::array<int, kNodes> order{};                    // ordre d'évaluation des nœuds
  std::array<std::uint8_t, kMaxRoutes> present{};     // par slot
  std::array<std::uint8_t, kMaxRoutes> delayed{};     // par slot
  std::array<int, kNodes + 1> off{};                  // routes groupées par destination
  std::array<int, kMaxRoutes> eSrc{};
  std::array<int, kMaxRoutes> eSlot{};
  std::array<std::uint8_t, kMaxRoutes> eDelayed{};
  int nRoutes = 0;
  int nDelayed = 0;
};

namespace detail {
inline bool reaches(const std::array<std::uint8_t, kMaxRoutes>& imm, int from, int to) {
  // chemin from -> ... -> to dans le graphe des routes immédiates (src -> dst)
  std::array<std::uint8_t, kNodes> seen{};
  std::array<int, kNodes> stack{};
  int sp = 0;
  stack[sp++] = from;
  seen[from] = 1;
  while (sp) {
    int u = stack[--sp];
    if (u == to) return true;
    for (int v = 0; v < kNodes; ++v)
      if (imm[slotOf(v, u)] && !seen[v]) { seen[v] = 1; stack[sp++] = v; }
  }
  return false;
}

inline void dfsBack(int u, std::array<int, kNodes>& st,
                    const std::array<std::uint8_t, kMaxRoutes>& pres,
                    std::array<std::uint8_t, kMaxRoutes>& del) {
  st[u] = 1;
  for (int v = 0; v < kNodes; ++v) {
    if (!pres[slotOf(v, u)]) continue;  // arête u -> v
    if (st[v] == 1) del[slotOf(v, u)] = 1;
    else if (st[v] == 0) dfsBack(v, st, pres, del);
  }
  st[u] = 2;
}
}  // namespace detail

inline Compiled compile(const std::vector<Route>& routes, Strategy strat) {
  Compiled c;
  std::array<int, kMaxRoutes> prio{};
  for (const Route& r : routes) {
    if (r.dst < 0 || r.dst >= kNodes || r.src < 0 || r.src >= kNodes)
      throw std::invalid_argument("route hors domaine");
    int s = slotOf(r.dst, r.src);
    if (c.present[s]) throw std::invalid_argument("route dupliquee");
    c.present[s] = 1;
    prio[s] = r.prio;
  }
  std::array<std::uint8_t, kMaxRoutes> imm{};
  if (strat == Strategy::A) {
    for (int s = 0; s < kMaxRoutes; ++s) c.delayed[s] = c.present[s];
  } else if (strat == Strategy::B) {
    std::vector<int> slots;
    for (int s = 0; s < kMaxRoutes; ++s) if (c.present[s]) slots.push_back(s);
    std::stable_sort(slots.begin(), slots.end(), [&](int a, int b) { return prio[a] < prio[b]; });
    for (int s : slots) {
      int dst = s / kNodes, src = s % kNodes;
      if (dst == src || detail::reaches(imm, dst, src)) c.delayed[s] = 1;  // ferme un cycle
      else imm[s] = 1;
    }
  } else {  // Bc
    std::array<int, kNodes> st{};
    for (int u = 0; u < kNodes; ++u) if (st[u] == 0) detail::dfsBack(u, st, c.present, c.delayed);
    for (int s = 0; s < kMaxRoutes; ++s) if (c.present[s] && c.delayed[s] == 0) imm[s] = 1;
  }
  if (strat != Strategy::A) {
    // Kahn, plus petit indice d'abord (déterministe)
    std::array<int, kNodes> indeg{};
    for (int s = 0; s < kMaxRoutes; ++s) if (imm[s]) indeg[s / kNodes]++;
    std::array<std::uint8_t, kNodes> done{};
    for (int k = 0; k < kNodes; ++k) {
      int pick = -1;
      for (int u = 0; u < kNodes; ++u) if (!done[u] && indeg[u] == 0) { pick = u; break; }
      if (pick < 0) throw std::logic_error("cycle immediat residuel");
      done[pick] = 1;
      c.order[k] = pick;
      for (int v = 0; v < kNodes; ++v) if (imm[slotOf(v, pick)]) indeg[v]--;
    }
  } else {
    for (int k = 0; k < kNodes; ++k) c.order[k] = k;
  }
  int e = 0;
  for (int dst = 0; dst < kNodes; ++dst) {
    c.off[dst] = e;
    for (int src = 0; src < kNodes; ++src) {
      int s = slotOf(dst, src);
      if (!c.present[s]) continue;
      c.eSrc[e] = src; c.eSlot[e] = s; c.eDelayed[e] = c.delayed[s];
      c.nDelayed += c.delayed[s];
      ++e;
    }
  }
  c.off[kNodes] = e;
  c.nRoutes = e;
  return c;
}

// Vérifie statiquement l'absence de dépendance instantanée implicite.
inline bool verifyCausal(const Compiled& c) {
  std::array<int, kNodes> pos{};
  for (int k = 0; k < kNodes; ++k) pos[c.order[k]] = k;
  for (int dst = 0; dst < kNodes; ++dst)
    for (int e = c.off[dst]; e < c.off[dst + 1]; ++e)
      if (!c.eDelayed[e] && pos[c.eSrc[e]] >= pos[dst]) return false;
  return true;
}

// Nombre de routes présentes dans a ET b dont la classe (immédiate/retardée) diffère.
inline int reclassified(const Compiled& a, const Compiled& b) {
  int n = 0;
  for (int s = 0; s < kMaxRoutes; ++s)
    if (a.present[s] && b.present[s] && a.delayed[s] != b.delayed[s]) ++n;
  return n;
}

// ----------------------------------------------------------------------------
// Délai causal
// ----------------------------------------------------------------------------
struct DelaySpec {
  double D = 1.0;       // retard effectif (éch. moteur)
  double Dwanted = 1.0; // retard demandé avant plancher
  bool integer = true;
  bool floored = false; // T2 non honorable (< 1 éch. moteur)
  int di = 1;
  int d0 = 1;
  std::array<double, 4> w{};
};

inline DelaySpec makeDelay(TimeRef ref, int K, double fsHost) {
  DelaySpec d;
  const double fsE = fsHost * K;
  d.Dwanted = (ref == TimeRef::T1) ? double(K) : fsE * kT2Seconds;
  d.D = d.Dwanted;
  if (d.D < 1.0) { d.D = 1.0; d.floored = true; }
  double r = std::round(d.D);
  if (std::fabs(d.D - r) < 1e-9) {
    d.integer = true;
    d.di = int(r);
  } else {
    d.integer = false;
    d.d0 = std::clamp(int(std::floor(d.D)) - 1, 1, kRing / 2);
    for (int k = 0; k < 4; ++k) {
      double wk = 1.0;
      for (int m = 0; m < 4; ++m)
        if (m != k) wk *= (d.D - (d.d0 + m)) / double((d.d0 + k) - (d.d0 + m));
      d.w[k] = wk;
    }
  }
  return d;
}

// ----------------------------------------------------------------------------
// Moteur
// ----------------------------------------------------------------------------
struct EngineParams {
  double fsHost = 48000.0;
  int K = 1;                       // oversampling interne
  TimeRef ref = TimeRef::T1;
  std::array<double, kNodes> freq{};  // Hz (fréquence porteuse de chaque nœud)
  bool poison = false;             // pose NaN sur les sorties courantes non encore calculées
};

class Engine {
 public:
  explicit Engine(const EngineParams& p) : p_(p), delay_(makeDelay(p.ref, p.K, p.fsHost)) {
    const double fsE = p.fsHost * p.K;
    for (int i = 0; i < kNodes; ++i) inc_[i] = p.freq[i] / fsE;
    reset();
  }
  void reset() {
    ph_.fill(0.0);
    for (auto& r : ring_) r.fill(0.0);
    cur_.fill(0.0);
    depth_.fill(0.0); tgt_.fill(0.0); step_.fill(0.0); left_.fill(0);
    anyRamp_ = false;
    n_ = 0;
  }
  // Transaction : remplacement atomique de la topologie compilée (point sûr = frontière d'éch.).
  void apply(const Compiled& c) { c_ = c; }
  // Profondeur (rad) d'une route ; rampEng = 0 -> immédiat.
  void setDepth(int slot, double target, int rampEng) {
    tgt_[slot] = target;
    if (rampEng <= 0) { depth_[slot] = target; left_[slot] = 0; }
    else { step_[slot] = (target - depth_[slot]) / rampEng; left_[slot] = rampEng; anyRamp_ = true; }
  }
  void setDepthNow(int slot, double v) { depth_[slot] = v; tgt_[slot] = v; left_[slot] = 0; }
  const DelaySpec& delay() const { return delay_; }
  std::int64_t position() const { return n_; }

  // Écrit n échantillons moteur (mono = moyenne des 5 nœuds). nodeOut (optionnel) : n*kNodes.
  void process(double* out, int n, double* nodeOut = nullptr) {
    const std::array<int, kNodes>& order = c_.order;
    for (int s = 0; s < n; ++s, ++n_) {
      if (p_.poison) cur_.fill(std::numeric_limits<double>::quiet_NaN());
      if (anyRamp_) stepRamps();
      for (int oi = 0; oi < kNodes; ++oi) {
        const int i = order[oi];
        ph_[i] += inc_[i];
        if (ph_[i] >= 1.0) ph_[i] -= 1.0;
        double pm = 0.0;
        for (int e = c_.off[i]; e < c_.off[i + 1]; ++e) {
          const double v = c_.eDelayed[e] ? readDelayed(c_.eSrc[e]) : cur_[c_.eSrc[e]];
          pm += depth_[c_.eSlot[e]] * v;
        }
        pm = std::clamp(pm, -kPmClamp, kPmClamp);
        cur_[i] = std::sin(2.0 * kPi * ph_[i] + pm);
      }
      double acc = 0.0;
      const int w = int(n_ & (kRing - 1));
      for (int i = 0; i < kNodes; ++i) { ring_[i][w] = cur_[i]; acc += cur_[i]; }
      out[s] = acc * (1.0 / kNodes);
      if (nodeOut) for (int i = 0; i < kNodes; ++i) nodeOut[s * kNodes + i] = cur_[i];
    }
  }

 private:
  double readDelayed(int src) const {
    // valeurs y[n-d], d >= 1 : jamais l'échantillon courant (n_ = indice courant)
    const auto& r = ring_[src];
    if (delay_.integer) return r[int((n_ - delay_.di) & (kRing - 1))];
    double v = 0.0;
    for (int k = 0; k < 4; ++k) v += delay_.w[k] * r[int((n_ - (delay_.d0 + k)) & (kRing - 1))];
    return v;
  }
  void stepRamps() {
    bool any = false;
    for (int s = 0; s < kMaxRoutes; ++s) {
      if (left_[s] > 0) {
        if (--left_[s] == 0) depth_[s] = tgt_[s]; else { depth_[s] += step_[s]; any = true; }
      }
    }
    anyRamp_ = any;
  }

  EngineParams p_;
  DelaySpec delay_;
  Compiled c_{};
  std::array<double, kNodes> inc_{}, ph_{}, cur_{};
  std::array<std::array<double, kRing>, kNodes> ring_{};
  std::array<double, kMaxRoutes> depth_{}, tgt_{}, step_{};
  std::array<int, kMaxRoutes> left_{};
  bool anyRamp_ = false;
  std::int64_t n_ = 0;
};

// ----------------------------------------------------------------------------
// Rendu avec événements transactionnels, découpé en blocs hôte
// ----------------------------------------------------------------------------
struct DepthSet { int slot; double target; int rampHost; };  // rampHost en échantillons hôte

struct Event {
  std::int64_t hostSample;   // position exacte (éch. hôte) de la transaction
  Compiled compiled;
  std::vector<DepthSet> depths;
};

struct RenderSpec {
  EngineParams params;
  Compiled initial;
  std::vector<DepthSet> initialDepths;
  std::vector<Event> events;       // triés par hostSample
  std::int64_t hostSamples = 0;
};

// Retourne les sorties au rythme moteur (hostSamples*K). blockHost : taille de bloc hôte.
inline std::vector<double> render(const RenderSpec& spec, int blockHost,
                                  std::vector<double>* nodeOut = nullptr) {
  Engine eng(spec.params);
  const int K = spec.params.K;
  eng.apply(spec.initial);
  for (const DepthSet& d : spec.initialDepths) eng.setDepth(d.slot, d.target, d.rampHost * K);
  const std::int64_t total = spec.hostSamples * K;
  std::vector<double> out(static_cast<size_t>(total), 0.0);
  if (nodeOut) nodeOut->assign(size_t(total) * kNodes, 0.0);
  size_t ev = 0;
  std::int64_t pos = 0;
  while (pos < total) {
    std::int64_t blockEnd = std::min<std::int64_t>(pos + std::int64_t(blockHost) * K, total);
    while (pos < blockEnd) {
      std::int64_t segEnd = blockEnd;
      if (ev < spec.events.size()) {
        std::int64_t at = spec.events[ev].hostSample * K;
        if (at <= pos) {  // applique la transaction à l'échantillon exact
          eng.apply(spec.events[ev].compiled);
          for (const DepthSet& d : spec.events[ev].depths) eng.setDepth(d.slot, d.target, d.rampHost * K);
          ++ev;
          continue;
        }
        segEnd = std::min(segEnd, at);
      }
      eng.process(out.data() + pos, int(segEnd - pos), nodeOut ? nodeOut->data() + pos * kNodes : nullptr);
      pos = segEnd;
    }
  }
  return out;
}

}  // namespace s0
