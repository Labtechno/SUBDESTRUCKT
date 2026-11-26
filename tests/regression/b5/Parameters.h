// ORACLE FIGE 0.1.0-b5 (copie de 8d23342, namespace sbd_b5) : ne jamais modifier. Reference de non-regression ARCH-G4.
// SUBDESTRUCKT — L1 — registre canonique des paramètres de la tranche ARCH-G3 (V1-CORE-CONTRACTS-P0, C23–C26).
//
// StableID explicites, immuables, jamais réutilisés ; jamais dérivés d'un indice ou d'un ordre JUCE.
// Le registre décrit le contrat ; il ne stocke pas la valeur courante. Les valeurs Base vivent dans ParamValues
// (état éditable / hôte) ; les valeurs Effective sont calculées par le moteur sans réécrire la Base (C24).
#pragma once
#include <array>
#include <cstdint>
#include <string_view>

namespace sbd_b5 {

enum class ParamId : int {
  AmpAttack = 0,
  AmpDecay,
  AmpSustain,
  AmpRelease,
  AmpVelocity,
  OutGain,
  WtLevel,
  Op1Level,
  Op2Level,
  Op1Ratio,
  Op2Ratio,
  DepthOp2Op1,
  DepthOp1Op2,
  DepthOp1Op1,
  DepthOp2Op2,
  DepthWtOp1,
  DepthWtOp2,
  DepthOp1Wt,
  DepthOp2Wt,
  CrossMaster,
  CrossFwd,
  CrossRev,
  Count
};
constexpr int kNumParams = static_cast<int>(ParamId::Count);

enum class ParamCategory : std::uint8_t { Continuous = 0, Event = 1, Structural = 2, Telemetry = 3 };

struct ParamSpec {
  const char* stableId;   // identité persistante (state, hôte)
  const char* name;       // libellé UI (non persistant, modifiable)
  const char* unit;       // unité du domaine
  float minValue, maxValue, defaultValue;
  float smoothingMs;      // 0 = appliqué aux événements / sans lissage
  bool automatable;
  ParamCategory category;
  int version;            // version d'introduction du paramètre
  const char* presetScope;
};

const ParamSpec& paramSpec(ParamId id) noexcept;
inline const ParamSpec& paramSpec(int index) noexcept { return paramSpec(static_cast<ParamId>(index)); }
// Recherche par StableID ; retourne false si inconnu.
bool findParam(std::string_view stableId, ParamId& out) noexcept;

struct ParamValues {
  std::array<float, kNumParams> v{};
  static ParamValues defaults() noexcept;
  float& operator[](ParamId id) noexcept { return v[static_cast<size_t>(id)]; }
  float operator[](ParamId id) const noexcept { return v[static_cast<size_t>(id)]; }
  bool operator==(const ParamValues& o) const noexcept { return v == o.v; }
};

}  // namespace sbd_b5
