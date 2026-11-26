// SUBDESTRUCKT — L1 — opérations créatives sur l'état (référentiel §9, arbitrages Preset & State 1B..10B).
//
//  - Randomize : nouvelle configuration ; Mutate : variations bornées ; Evolve : couche temporaire continue (Reset /
//    Commit), modes Anchored (rappel vers l'ancre) et Drift borné.
//  - Familles (groupes du registre), verrous par paramètre (jamais modifiés pour satisfaire une contrainte),
//    intensité (0 = neutre), profils Controlled / Industrial / Experimental / Custom.
//  - Déterminisme : Master Seed + flux nommés (Randomize, Mutate, Evolve) ; compteur de tirages persistant dans l'état.
//    Une transaction refusée n'avance pas le compteur. Undo/Redo restaure la configuration ET les compteurs.
//  - Undo/Redo : pile d'instance (non sauvegardée dans les presets, arbitrage 9A), 128 transactions, budget mémoire.
// Tout ce fichier s'exécute hors RT.
#pragma once
#include <array>
#include <cstdint>
#include <deque>
#include <set>
#include <string>
#include <vector>

#include "Parameters.h"
#include "StateCodec.h"

namespace sbd {

enum class RandomProfile : std::uint8_t { Controlled = 0, Industrial = 1, Experimental = 2, Custom = 3 };
const char* randomProfileName(RandomProfile p) noexcept;

// Familles Randomize (sélection par préfixe de groupe du registre).
enum class RandomFamily : std::uint8_t { Wavetable = 0, Fm, Cross, Filters, Destruction, Modulation, Envelopes, Topology, Count };
const char* randomFamilyName(RandomFamily f) noexcept;
bool paramInFamily(ParamId id, RandomFamily f) noexcept;

struct RandomOptions {
  RandomProfile profile = RandomProfile::Industrial;
  float intensity = 1.0f;                                     // 0..1 (0 = neutre)
  std::array<bool, static_cast<size_t>(RandomFamily::Count)> families{true, true, true, true, true, true, true, false};
  std::set<ParamId> locks;                                    // paramètres verrouillés
};

struct OpResult {
  bool ok = false;
  std::string error;
  int changed = 0;
};

// Randomize / Mutate : modifient `state` (params, routes si famille Topology) ; avancent streamCounters[0|1].
OpResult randomizeState(PluginState& state, const RandomOptions& opt);
OpResult mutateState(PluginState& state, const RandomOptions& opt);

// Evolve : couche temporaire de décalages (domaine du paramètre) appliquée par-dessus la Base, jamais écrite dans la
// Base tant qu'elle n'est pas commitée.
enum class EvolveMode : std::uint8_t { Anchored = 0, Drift = 1 };
class Evolver {
 public:
  void start(const PluginState& anchor, const RandomOptions& opt, EvolveMode mode, float rateHz);
  void stop() noexcept { running_ = false; }
  bool running() const noexcept { return running_; }
  // Avance de dt secondes ; avance le flux Evolve (compteur exposé pour l'état).
  void step(double dt);
  // Valeur évoluée d'un paramètre (Base + décalage, bornée) — hors liste blanche : Base inchangée.
  float apply(ParamId id, float base) const noexcept;
  void reset() noexcept;                       // décalages remis à zéro
  bool commit(PluginState& state) const;       // écrit Base + décalage dans l'état (transaction)
  const std::array<float, kNumParams>& offsets() const noexcept { return off_; }
  std::uint64_t counter() const noexcept { return counter_; }
  static bool whitelisted(ParamId id) noexcept;  // continus, randomisables, sans effet structurel

 private:
  bool running_ = false;
  EvolveMode mode_ = EvolveMode::Anchored;
  RandomOptions opt_;
  float rate_ = 0.1f;
  std::array<float, kNumParams> off_{};
  std::array<float, kNumParams> vel_{};
  std::array<bool, kNumParams> active_{};
  std::uint64_t seed_ = 0, counter_ = 0;
};

// ---------------------------------------------------------------- Undo / Redo
struct Transaction {
  std::uint64_t id = 0;
  std::string cause;
  PluginState before, after;
};

class UndoStack {
 public:
  static constexpr int kCapacity = 128;                  // arbitrage 4B
  static constexpr std::size_t kMemoryBudget = 64u << 20;  // budget mémoire indépendant (octets, estimation)
  // Enregistre une transaction (avant/après) ; supprime l'historique Redo. Doublon (avant == après) ignoré.
  void push(const PluginState& before, const PluginState& after, const std::string& cause);
  bool canUndo() const noexcept { return cursor_ > 0; }
  bool canRedo() const noexcept { return cursor_ < static_cast<int>(stack_.size()); }
  // Renvoie l'état à appliquer (aucun tirage RNG : les compteurs font partie de l'état restauré).
  const PluginState* undo() noexcept;
  const PluginState* redo() noexcept;
  std::string undoCause() const;
  std::string redoCause() const;
  int size() const noexcept { return static_cast<int>(stack_.size()); }
  int cursor() const noexcept { return cursor_; }
  std::size_t memoryEstimate() const noexcept;
  void clear() noexcept {
    stack_.clear();
    cursor_ = 0;
  }

 private:
  std::deque<Transaction> stack_;
  int cursor_ = 0;
  std::uint64_t next_ = 1;
};

// ---------------------------------------------------------------- bibliothèque de presets
struct PresetInfo {
  std::string name;        // nom affiché
  std::string path;        // fichier (vide = usine)
  std::vector<std::string> tags;
  bool favorite = false;
  bool factory = false;
};

// Presets usine (générés, déterministes) : démonstration des moteurs réels, sans contenu inventé.
std::vector<std::pair<PresetInfo, PluginState>> factoryPresets();

class PresetLibrary {
 public:
  explicit PresetLibrary(std::string directory) : dir_(std::move(directory)) {}
  const std::string& directory() const noexcept { return dir_; }
  std::vector<PresetInfo> list() const;  // usine + utilisateur (fichiers .sdpreset), métadonnées incluses
  bool save(const std::string& name, const PluginState& state, const std::vector<std::string>& tags, std::string& error) const;
  bool load(const PresetInfo& info, PluginState& out, std::string& error) const;
  bool setFavorite(const PresetInfo& info, bool fav, std::string& error) const;
  static std::string sanitizeName(const std::string& name);

 private:
  std::string dir_;
};

}  // namespace sbd
