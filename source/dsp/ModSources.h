// SUBDESTRUCKT — L3 — sources de modulation : enveloppes DAHDSR, LFO, aléatoires, chaos, suiveurs.
// Toutes déterministes, sans allocation, au rythme moteur.
#pragma once
#include <array>
#include <cmath>
#include <cstdint>

#include "DspConstants.h"
#include "Rng.h"

namespace sbd {

// Enveloppe Delay-Attack-Hold-Decay-Sustain-Release.
//  - Linear : segments linéaires, temps = parcours pleine échelle (identique à l'ADSR de la tranche b5 si delay = hold = 0).
//  - Exponential : segments à une constante de temps (attaque vers 1,2 tronquée à 1 ; décroissances vers la cible,
//    seuil de fin à -100 dB) ; le temps réglé correspond à environ -60 dB du parcours.
//  - Loop : tant que le gate est tenu, le cycle Attack-Hold-Decay redémarre à l'arrivée au sustain.
// Note-on relance depuis le niveau courant (pas de saut à zéro) ; note-off passe en relâchement depuis le niveau courant.
class Envelope {
 public:
  enum class Stage { Idle = 0, Delay, Attack, Hold, Decay, Sustain, Release };
  void setSampleRate(double fs) noexcept { fs_ = fs; }
  void setParameters(double delayS, double attackS, double holdS, double decayS, double sustain, double releaseS,
                     bool exponential, bool loop) noexcept {
    delaySamples_ = static_cast<long long>(std::llround(std::fmax(delayS, 0.0) * fs_));
    holdSamples_ = static_cast<long long>(std::llround(std::fmax(holdS, 0.0) * fs_));
    attackStep_ = 1.0 / (std::fmax(attackS, 1e-6) * fs_);
    decayStep_ = 1.0 / (std::fmax(decayS, 1e-6) * fs_);
    sustain_ = std::fmin(std::fmax(sustain, 0.0), 1.0);
    releaseStep_ = 1.0 / (std::fmax(releaseS, 1e-6) * fs_);
    exp_ = exponential;
    loop_ = loop;
    if (exp_) {
      attackCoef_ = 1.0 - std::exp(-2.0 / (std::fmax(attackS, 1e-6) * fs_));
      decayCoef_ = 1.0 - std::exp(-6.9 / (std::fmax(decayS, 1e-6) * fs_));
      releaseCoef_ = 1.0 - std::exp(-6.9 / (std::fmax(releaseS, 1e-6) * fs_));
    }
  }
  void noteOn() noexcept {
    if (delaySamples_ > 0) {
      stage_ = Stage::Delay;
      counter_ = delaySamples_;
    } else {
      stage_ = Stage::Attack;
    }
  }
  void noteOff() noexcept {
    if (stage_ != Stage::Idle) stage_ = Stage::Release;
  }
  void reset() noexcept {
    stage_ = Stage::Idle;
    level_ = 0.0;
  }
  double next() noexcept {
    switch (stage_) {
      case Stage::Idle: level_ = 0.0; break;
      case Stage::Delay:
        if (--counter_ <= 0) stage_ = Stage::Attack;
        break;
      case Stage::Attack:
        if (exp_) {
          level_ += (1.2 - level_) * attackCoef_;
        } else {
          level_ += attackStep_;
        }
        if (level_ >= 1.0) {
          level_ = 1.0;
          if (holdSamples_ > 0) {
            stage_ = Stage::Hold;
            counter_ = holdSamples_;
          } else {
            stage_ = Stage::Decay;
          }
        }
        break;
      case Stage::Hold:
        if (--counter_ <= 0) stage_ = Stage::Decay;
        break;
      case Stage::Decay:
        if (level_ > sustain_) {
          if (exp_) {
            level_ += (sustain_ - level_) * decayCoef_;
            if (level_ - sustain_ <= 1e-5) {
              level_ = sustain_;
              stage_ = Stage::Sustain;
            }
          } else {
            level_ -= decayStep_;
            if (level_ <= sustain_) {
              level_ = sustain_;
              stage_ = Stage::Sustain;
            }
          }
        } else {
          stage_ = Stage::Sustain;
        }
        if (stage_ == Stage::Sustain && loop_) stage_ = Stage::Attack;
        break;
      case Stage::Sustain:
        if (loop_) {
          stage_ = Stage::Attack;
          break;
        }
        // suit un changement de sustain avec la pente de décroissance/attaque, sans saut
        if (level_ > sustain_) {
          level_ = std::fmax(sustain_, level_ - decayStep_);
        } else if (level_ < sustain_) {
          level_ = std::fmin(sustain_, level_ + attackStep_);
        }
        break;
      case Stage::Release:
        if (exp_) {
          level_ -= level_ * releaseCoef_;
          if (level_ <= 1e-5) {
            level_ = 0.0;
            stage_ = Stage::Idle;
          }
        } else {
          level_ -= releaseStep_;
          if (level_ <= 0.0) {
            level_ = 0.0;
            stage_ = Stage::Idle;
          }
        }
        break;
    }
    return level_;
  }
  double level() const noexcept { return level_; }
  Stage stage() const noexcept { return stage_; }
  bool active() const noexcept { return stage_ != Stage::Idle; }

 private:
  double fs_ = 48000.0;
  double attackStep_ = 1.0, decayStep_ = 1.0, sustain_ = 1.0, releaseStep_ = 1.0;
  double attackCoef_ = 1.0, decayCoef_ = 1.0, releaseCoef_ = 1.0;
  long long delaySamples_ = 0, holdSamples_ = 0, counter_ = 0;
  bool exp_ = false, loop_ = false;
  double level_ = 0.0;
  Stage stage_ = Stage::Idle;
};

// Courbe personnalisée d'un LFO (2..8 points), y dans [-1,1].
struct LfoCurve {
  int n = 2;
  std::array<float, 8> x{0.0f, 1.0f}, y{-1.0f, 1.0f};
  double eval(double p) const noexcept {
    if (p <= x[0]) return y[0];
    for (int k = 1; k < n; ++k)
      if (p <= x[static_cast<size_t>(k)]) {
        const double t = (p - x[static_cast<size_t>(k - 1)]) / (x[static_cast<size_t>(k)] - x[static_cast<size_t>(k - 1)]);
        return y[static_cast<size_t>(k - 1)] + t * (y[static_cast<size_t>(k)] - y[static_cast<size_t>(k - 1)]);
      }
    return y[static_cast<size_t>(n - 1)];
  }
};

// LFO : sortie bipolaire [-1,1]. Formes : 0 Sine, 1 Triangle, 2 Saw Up, 3 Saw Down, 4 Square, 5 Random Hold,
// 6 Random Smooth, 7 Custom. Phase continue ; un nouveau tirage aléatoire à chaque cycle (flux dédié).
class Lfo {
 public:
  void init(std::uint64_t seed, StreamId id) noexcept {
    rng_.init(seed, id);
    a_ = rng_.bipolar();
    b_ = rng_.bipolar();
  }
  void resetPhase(double p) noexcept { phase_ = p - std::floor(p); }
  void setPhase(double p) noexcept { phase_ = p - std::floor(p); }
  double phase() const noexcept { return phase_; }
  double next(int shape, double inc, double offset, const LfoCurve* curve) noexcept {
    double p = phase_ + offset;
    p -= std::floor(p);
    double y = 0.0;
    switch (shape) {
      case 0: y = std::sin(kTwoPi * p); break;
      case 1: y = p < 0.25 ? 4.0 * p : (p < 0.75 ? 2.0 - 4.0 * p : 4.0 * p - 4.0); break;
      case 2: y = 2.0 * p - 1.0; break;
      case 3: y = 1.0 - 2.0 * p; break;
      case 4: y = p < 0.5 ? 1.0 : -1.0; break;
      case 5: y = a_; break;
      case 6: {
        const double t = 0.5 - 0.5 * std::cos(kPi * phase_);
        y = a_ + t * (b_ - a_);
        break;
      }
      case 7: y = curve ? curve->eval(p) : 0.0; break;
      default: break;
    }
    phase_ += inc;
    if (phase_ >= 1.0 || phase_ < 0.0) {
      phase_ -= std::floor(phase_);
      a_ = b_;
      b_ = rng_.bipolar();
    }
    return y;
  }

 private:
  double phase_ = 0.0, a_ = 0.0, b_ = 0.0;
  RngStream rng_;
};

// Aléatoire global : Hold (palier par période) et Smooth (interpolation cosinus), bipolaires.
class RandomSource {
 public:
  void init(std::uint64_t seed, StreamId id) noexcept {
    rng_.init(seed, id);
    a_ = rng_.bipolar();
    b_ = rng_.bipolar();
    phase_ = 0.0;
  }
  double next(double inc, bool smooth) noexcept {
    const double y = smooth ? a_ + (0.5 - 0.5 * std::cos(kPi * phase_)) * (b_ - a_) : a_;
    phase_ += inc;
    if (phase_ >= 1.0) {
      phase_ -= std::floor(phase_);
      a_ = b_;
      b_ = rng_.bipolar();
    }
    return y;
  }

 private:
  RngStream rng_;
  double a_ = 0.0, b_ = 0.0, phase_ = 0.0;
};

// Chaos déterministe. Trois modèles candidats (décision CHAOS-01 dans le tracker) ; le moteur utilise kChaosModel.
// Sorties normalisées dans [-1,1] (bornes dures), intégration bornée par pas maximal.
enum class ChaosModel : std::uint8_t { Lorenz = 0, Rossler = 1, Logistic = 2 };
class Chaos {
 public:
  void reset(ChaosModel m, double seedJitter = 0.0) noexcept {
    model_ = m;
    x_ = 0.1 + seedJitter;
    y_ = 0.0;
    z_ = 0.0;
    if (m == ChaosModel::Rossler) {
      x_ = 1.0 + seedJitter;
      y_ = 1.0;
    }
    if (m == ChaosModel::Logistic) x_ = 0.4 + 0.1 * seedJitter;
    phase_ = 0.0;
    prev_ = cur_ = 0.0;
  }
  // rateHz : vitesse d'évolution ; fs : rythme d'appel.
  double next(double rateHz, double fs) noexcept {
    switch (model_) {
      case ChaosModel::Lorenz: {
        // pas d'intégration proportionnel à la vitesse, découpé pour rester stable (dt <= 0.01)
        double dt = rateHz / fs * 0.5;
        int steps = 1;
        while (dt / steps > 0.01 && steps < 64) ++steps;
        const double h = dt / steps;
        for (int i = 0; i < steps; ++i) {
          const double dx = 10.0 * (y_ - x_), dy = x_ * (28.0 - z_) - y_, dz = x_ * y_ - (8.0 / 3.0) * z_;
          // Heun (RK2)
          const double xe = x_ + h * dx, ye = y_ + h * dy, ze = z_ + h * dz;
          const double dx2 = 10.0 * (ye - xe), dy2 = xe * (28.0 - ze) - ye, dz2 = xe * ye - (8.0 / 3.0) * ze;
          x_ += 0.5 * h * (dx + dx2);
          y_ += 0.5 * h * (dy + dy2);
          z_ += 0.5 * h * (dz + dz2);
        }
        if (!std::isfinite(x_) || std::fabs(x_) > 100.0) reset(model_);
        return clamp1(x_ / 20.0);
      }
      case ChaosModel::Rossler: {
        double dt = rateHz / fs * 3.0;
        int steps = 1;
        while (dt / steps > 0.02 && steps < 64) ++steps;
        const double h = dt / steps;
        for (int i = 0; i < steps; ++i) {
          const double dx = -y_ - z_, dy = x_ + 0.2 * y_, dz = 0.2 + z_ * (x_ - 5.7);
          const double xe = x_ + h * dx, ye = y_ + h * dy, ze = z_ + h * dz;
          const double dx2 = -ye - ze, dy2 = xe + 0.2 * ye, dz2 = 0.2 + ze * (xe - 5.7);
          x_ += 0.5 * h * (dx + dx2);
          y_ += 0.5 * h * (dy + dy2);
          z_ += 0.5 * h * (dz + dz2);
        }
        if (!std::isfinite(x_) || std::fabs(x_) > 100.0) reset(model_);
        return clamp1(x_ / 12.0);
      }
      case ChaosModel::Logistic: {
        // une itération par période, interpolation linéaire entre itérés
        phase_ += rateHz / fs;
        if (phase_ >= 1.0) {
          phase_ -= std::floor(phase_);
          prev_ = cur_;
          x_ = 3.9 * x_ * (1.0 - x_);
          if (!(x_ > 0.0 && x_ < 1.0)) x_ = 0.4;
          cur_ = 2.0 * x_ - 1.0;
        }
        return prev_ + phase_ * (cur_ - prev_);
      }
    }
    return 0.0;
  }

 private:
  static double clamp1(double v) noexcept { return v > 1.0 ? 1.0 : (v < -1.0 ? -1.0 : v); }
  ChaosModel model_ = ChaosModel::Lorenz;
  double x_ = 0.1, y_ = 0.0, z_ = 0.0, phase_ = 0.0, prev_ = 0.0, cur_ = 0.0;
};
constexpr ChaosModel kChaosModel = ChaosModel::Lorenz;

// Suiveur d'enveloppe crête (attaque/relâchement à une constante de temps). Sortie [0, ~].
class Follower {
 public:
  void setup(double fs, double attackS, double releaseS) noexcept {
    a_ = 1.0 - std::exp(-1.0 / (std::fmax(attackS, 1e-5) * fs));
    r_ = 1.0 - std::exp(-1.0 / (std::fmax(releaseS, 1e-5) * fs));
  }
  void reset() noexcept { y_ = 0.0; }
  double process(double x) noexcept {
    const double v = std::fabs(x);
    y_ += (v > y_ ? a_ : r_) * (v - y_);
    if (y_ < 1e-12) y_ = 0.0;
    return y_;
  }
  double value() const noexcept { return y_; }

 private:
  double a_ = 1.0, r_ = 1.0, y_ = 0.0;
};

}  // namespace sbd
