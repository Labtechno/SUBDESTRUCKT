// SUBDESTRUCKT — L1 — registre canonique des paramètres (V1-CORE-CONTRACTS-P0, C23–C27).
//
// Source de vérité : ParamTable.inc, générée par tools/gen_params.py (append-only). Les 22 paramètres v1 (0.1.0-b5)
// sont en tête, inchangés. StableID explicites, immuables, jamais réutilisés ; jamais dérivés d'un indice JUCE.
// L'indice (ParamId) est un détail d'implémentation interne, jamais persisté : l'état et l'hôte utilisent le StableID.
// Le registre décrit le contrat ; il ne stocke pas la valeur courante. Les valeurs Base vivent dans ParamValues
// (état éditable / hôte) ; les valeurs Effective sont calculées par le moteur sans réécrire la Base (C24).
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace sbd {

enum class ParamType : std::uint8_t { Float = 0, Choice = 1, Bool = 2, Int = 3 };

// Domaine de sommation des modulations (architecture §7) : la somme base + contributions se fait dans ce domaine.
enum class Domain : std::uint8_t {
  Lin = 0,   // linéaire
  Db,        // décibels
  Oct,       // fréquence/ratio : somme en octaves (log2)
  Time,      // durée : somme en octaves de temps (log2)
  Semi,      // demi-tons
  Rad,       // radians
  Cyc,       // cyclique [0,1)
  Disc       // discret (choix, booléen, entier) : jamais destination continue
};

constexpr std::uint8_t kRateEvent = 1, kRateControl = 2, kRateAudio = 4;

// Quality : préférence d'instance/projet (P0-C13), distincte de Clean/Raw (artistique) et de l'oversampling.
enum class Quality : std::uint8_t { Eco = 0, Normal = 1, High = 2 };
const char* qualityName(Quality q) noexcept;

enum class ParamId : int {
#define SBD_PARAM(e, ...) e,
#include "ParamTable.inc"
#undef SBD_PARAM
  Count
};
constexpr int kNumParams = static_cast<int>(ParamId::Count);
constexpr int kNumV1Params = 22;  // paramètres du format d'état v1 (0.1.0-b5)

enum class ParamCategory : std::uint8_t { Continuous = 0, Event = 1, Structural = 2, Telemetry = 3 };

struct ParamSpec {
  const char* stableId;   // identité persistante (state, hôte)
  const char* name;       // libellé UI (non persistant, modifiable)
  const char* unit;       // unité du domaine
  float minValue, maxValue, defaultValue;  // plage Base (= plage artistique de la V1) et défaut
  float smoothingMs;      // 0 = appliqué aux événements / sans lissage
  bool automatable;
  ParamCategory category;
  int version;            // version d'introduction du paramètre (1 = 0.1.0-b5)
  const char* presetScope;
  ParamType type;
  Domain domain;
  std::uint8_t rates;     // capacités : kRateEvent | kRateControl | kRateAudio
  bool randomizable;
  float modScale;         // unités du domaine pour une profondeur de modulation de 1.0
  float hardMin, hardMax; // bornes DSP dures
  const char* choices;    // libellés séparés par '|' (Choice)
  const char* group;      // chemin UI / famille Randomize
  int stage;              // étage du pipeline audio (0 réseau, 1 chaîne WT, 2 chaîne FM, 3 Cross Audio, 4 global, 5 sortie)
};

const ParamSpec& paramSpec(ParamId id) noexcept;
inline const ParamSpec& paramSpec(int index) noexcept { return paramSpec(static_cast<ParamId>(index)); }
// Recherche par StableID ; retourne false si inconnu.
bool findParam(std::string_view stableId, ParamId& out) noexcept;
inline constexpr int idx(ParamId id) noexcept { return static_cast<int>(id); }

// Libellés de choix (hors RT).
int choiceCount(ParamId id) noexcept;
std::string choiceLabel(ParamId id, int index);
// Texte d'affichage d'une valeur Base (hors RT).
std::string formatValue(ParamId id, float value);

// Conversion vers/depuis le domaine de sommation (RT-safe, sans allocation).
double toDomain(Domain d, double v) noexcept;
double fromDomain(Domain d, double v) noexcept;
// Valeur conforme au type : arrondi des discrets, bornage dans la plage Base.
float sanitize(ParamId id, float v) noexcept;

struct ParamValues {
  std::array<float, kNumParams> v{};
  static ParamValues defaults() noexcept;
  float& operator[](ParamId id) noexcept { return v[static_cast<size_t>(id)]; }
  float operator[](ParamId id) const noexcept { return v[static_cast<size_t>(id)]; }
  int choice(ParamId id) const noexcept { return static_cast<int>(v[static_cast<size_t>(id)] + 0.5f); }
  bool flag(ParamId id) const noexcept { return v[static_cast<size_t>(id)] >= 0.5f; }
  bool operator==(const ParamValues& o) const noexcept { return v == o.v; }
  bool operator!=(const ParamValues& o) const noexcept { return !(*this == o); }
};

}  // namespace sbd
