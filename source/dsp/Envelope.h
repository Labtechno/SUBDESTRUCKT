// SUBDESTRUCKT — L3 DSP primitives — enveloppe Amp ADSR à segments linéaires, rampe linéaire, garde DC.
#pragma once
#include <cmath>

namespace sbd {

// ADSR linéaire. Les temps sont des temps de parcours pleine échelle (0 -> 1) : pente = 1 / (temps * Fs).
// Note-on relance l'attaque à partir du niveau courant (pas de saut à zéro) ; note-off passe en relâchement
// depuis le niveau courant. Aucun état DSP extérieur n'est touché (contrat P0-C02).
class Adsr {
 public:
  enum class Stage { Idle = 0, Attack, Decay, Sustain, Release };

  void setSampleRate(double fs) noexcept { fs_ = fs; }
  void setParameters(double attackS, double decayS, double sustain, double releaseS) noexcept {
    attackStep_ = 1.0 / (std::fmax(attackS, 1e-6) * fs_);
    decayStep_ = 1.0 / (std::fmax(decayS, 1e-6) * fs_);
    sustain_ = std::fmin(std::fmax(sustain, 0.0), 1.0);
    releaseStep_ = 1.0 / (std::fmax(releaseS, 1e-6) * fs_);
  }
  void noteOn() noexcept { stage_ = Stage::Attack; }
  void noteOff() noexcept {
    if (stage_ != Stage::Idle) stage_ = Stage::Release;
  }
  void reset() noexcept {
    stage_ = Stage::Idle;
    level_ = 0.0;
  }
  double next() noexcept {
    switch (stage_) {
      case Stage::Idle:
        level_ = 0.0;
        break;
      case Stage::Attack:
        level_ += attackStep_;
        if (level_ >= 1.0) {
          level_ = 1.0;
          stage_ = Stage::Decay;
        }
        break;
      case Stage::Decay:
        if (level_ > sustain_) {
          level_ -= decayStep_;
          if (level_ <= sustain_) {
            level_ = sustain_;
            stage_ = Stage::Sustain;
          }
        } else {
          stage_ = Stage::Sustain;
        }
        break;
      case Stage::Sustain:
        // suit un changement de sustain avec la pente de décroissance/attaque, sans saut
        if (level_ > sustain_) {
          level_ = std::fmax(sustain_, level_ - decayStep_);
        } else if (level_ < sustain_) {
          level_ = std::fmin(sustain_, level_ + attackStep_);
        }
        break;
      case Stage::Release:
        level_ -= releaseStep_;
        if (level_ <= 0.0) {
          level_ = 0.0;
          stage_ = Stage::Idle;
        }
        break;
    }
    return level_;
  }
  double level() const noexcept { return level_; }
  Stage stage() const noexcept { return stage_; }

 private:
  double fs_ = 48000.0;
  double attackStep_ = 1.0, decayStep_ = 1.0, sustain_ = 1.0, releaseStep_ = 1.0;
  double level_ = 0.0;
  Stage stage_ = Stage::Idle;
};

// Rampe linéaire de durée fixe : atteint EXACTEMENT la cible à la fin (contrairement à un lissage exponentiel),
// ce qui permet des contributions exactement nulles (CROSS = 0 sans résidu).
class LinearRamp {
 public:
  void setLength(int samples) noexcept { length_ = samples < 1 ? 1 : samples; }
  void reset(double v) noexcept {
    current_ = target_ = v;
    remaining_ = 0;
    step_ = 0.0;
  }
  void setTarget(double t) noexcept {
    if (t == target_) return;
    target_ = t;
    remaining_ = length_;
    step_ = (target_ - current_) / length_;
  }
  double next() noexcept {
    if (remaining_ > 0) {
      current_ += step_;
      if (--remaining_ == 0) current_ = target_;
    }
    return current_;
  }
  double current() const noexcept { return current_; }
  double target() const noexcept { return target_; }
  bool ramping() const noexcept { return remaining_ > 0; }

 private:
  double current_ = 0.0, target_ = 0.0, step_ = 0.0;
  int remaining_ = 0, length_ = 1;
};

// Garde DC : passe-haut d'ordre 1, sur la sortie audio uniquement (jamais sur un chemin de modulation).
class DcGuard {
 public:
  void setup(double fs, double cutoffHz) noexcept { r_ = std::exp(-2.0 * 3.14159265358979323846 * cutoffHz / fs); }
  void reset() noexcept { x1_ = y1_ = 0.0; }
  double process(double x) noexcept {
    const double y = x - x1_ + r_ * y1_;
    x1_ = x;
    y1_ = y;
    return y;
  }

 private:
  double r_ = 0.9995, x1_ = 0.0, y1_ = 0.0;
};

}  // namespace sbd
