// SUBDESTRUCKT — L3 — banques wavetable XY immuables (référentiel §5, architecture §3.5).
//
// Grille X (position de forme, 1..64) x Y (table, 1..16). Chaque forme : 2048 échantillons par cycle (float32 mono),
// préparée hors RT en 11 niveaux mip limités en bande (niveau L : harmoniques <= 2^(10-L)), longueur pleine 2048+1
// (échantillon de garde) — même construction que la table de la tranche 0.1.0-b5, ce qui garantit la non-régression.
// Z n'est PAS une dimension stockée : c'est une transformation calculée (voir WtVoice).
// Une banque construite est immuable et partageable entre instances ; elle n'est jamais modifiée après publication
// (Copy-on-Write : toute édition produit une nouvelle banque, avec un nouvel identifiant de contenu).
#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace sbd {

enum class WtNormalize : std::uint8_t { None = 0, PerWave = 1, PerBank = 2 };

class WtBank {
 public:
  static constexpr int kLength = 2048;
  static constexpr int kLevels = 11;
  static constexpr int kMaxX = 64, kMaxY = 16;
  static constexpr int kStride = kLength + 1;

  struct Options {
    WtNormalize normalize = WtNormalize::PerWave;
    bool removeDc = true;
  };

  // Hors RT. raw : ny tables x nx formes x 2048 échantillons (ordre table, puis forme). Refus : dimensions, valeurs.
  static std::shared_ptr<const WtBank> build(const std::string& id, const std::string& name, int nx, int ny,
                                             const std::vector<float>& raw, const Options& opt, std::string& error);

  const std::string& id() const noexcept { return id_; }
  const std::string& name() const noexcept { return name_; }
  int nx() const noexcept { return nx_; }
  int ny() const noexcept { return ny_; }
  int numWaves() const noexcept { return nx_ * ny_; }
  // Contenu brut exact (export, hash, édition COW).
  const std::vector<float>& raw() const noexcept { return raw_; }
  // Échantillon i (0..2048) du niveau L d'une forme (hors RT : tests, affichage).
  double sample(int wave, int L, int i) const noexcept {
    const size_t o = (static_cast<size_t>(wave) * kLevels + static_cast<size_t>(L)) * kStride + static_cast<size_t>(i);
    return precise_ ? mipsD_[o] : static_cast<double>(mipsF_[o]);
  }
  bool precise() const noexcept { return precise_; }
  std::size_t memoryBytes() const noexcept {
    return mipsF_.size() * sizeof(float) + mipsD_.size() * sizeof(double) + raw_.size() * sizeof(float);
  }
  // Banque de référence 0.1.0-b5 : niveaux mip en double, construits exactement comme la table de la tranche.
  static std::shared_ptr<const WtBank> buildB5Saw(const std::string& id, const std::string& name);

  // Lecture RT : phase [0,1), absIncrement = |fréquence instantanée|/Fs (cycles/échantillon) multiplié par la pente
  // locale maximale de la phase (Phase Warp) ; raw = niveau 0 (Raw), cubic = interpolation de Hermite.
  double read(int wave, double phase, double absIncrement, bool raw, bool cubic) const noexcept;

 private:
  WtBank() = default;
  std::string id_, name_;
  int nx_ = 1, ny_ = 1;
  std::vector<float> raw_;
  bool precise_ = false;
  std::vector<float> mipsF_;
  std::vector<double> mipsD_;
};

// ---------------------------------------------------------------- générateur procédural (hors RT, déterministe)
struct WtGenerator {
  // Forme additive : amplitudes/phases des harmoniques 1..n.
  static std::vector<float> additive(const std::vector<double>& amp, const std::vector<double>& phase);
  static std::vector<float> saw();          // dent de scie de la tranche b5 (1024 harmoniques en 1/k)
  static std::vector<float> sine();
  static std::vector<float> triangle();
  static std::vector<float> square();
  static std::vector<float> pulse(double width);
  static std::vector<float> foldedSine(double gain);
  static std::vector<float> randomSpectrum(std::uint64_t seed, int harmonics, double tilt, double density);
  static std::vector<float> formant(double f1, double f2, double bandwidth);
};

// Banques usine (générées hors RT au premier usage, mises en cache, immuables).
struct FactoryBanks {
  static const std::vector<std::string>& ids();         // "factory:saw", "factory:basic", ...
  static std::string displayName(const std::string& id);
  static std::shared_ptr<const WtBank> get(const std::string& id);  // nullptr si inconnu
};

}  // namespace sbd
