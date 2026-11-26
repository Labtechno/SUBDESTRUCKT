// SUBDESTRUCKT — L1 — Global Modulation Matrix (référentiel §8, architecture §3.9, P0-C24/C25/C27).
//
// 64 connexions maximum, macros incluses. Une connexion : source -> (polarité/inversion) -> courbe -> offset ->
// profondeur -> Via (V = (1-a) + a*v, multiplicatif unipolaire) -> lissage -> conversion vers le domaine de la
// destination (Parameters.h : Domain, modScale). L'automation hôte agit sur la Base, jamais sur la valeur modulée.
// Destinations : paramètres continus (Float) déclarant kRateControl et/ou kRateAudio. Les paramètres discrets
// (structurels) ne sont jamais destinations continues (C25).
// Règles de compilation (hors RT) :
//   - rate Audio demandé sur une destination qui ne le déclare pas : connexion conservée INACTIVE avec raison
//     (jamais de downgrade silencieux vers Control) ; au-delà de 16 connexions audio-rate : inactive, raison budget ;
//   - graphe indirect (macro -> macro) acyclique : une transaction qui créerait un cycle est refusée ;
//   - suiveurs d'enveloppe : la destination doit être strictement en aval du point mesuré (pas de cycle audio) ;
//   - doublon exact ignoré à la fusion, conflit d'identifiant explicite, dépassement de capacité refusé.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "Parameters.h"

namespace sbd {

enum class ModSource : std::uint8_t {
  None = 0,
  Velocity, Note, PitchBend, ModWheel, Breath, Expression, Pressure, Sustain,
  EnvAmp, EnvWt, EnvFm, EnvAux1, EnvAux2,
  Lfo1, Lfo2, Lfo3, Lfo4,
  RandomHold, RandomSmooth, Chaos,
  Macro1, Macro2, Macro3, Macro4,
  Follower1, Follower2, Follower3,
  Y2Wt1, Y2Wt2, Y2Fm1, Y2Fm2, Y2Gl1, Y2Gl2,
  Count
};
constexpr int kNumModSources = static_cast<int>(ModSource::Count);
constexpr int kMaxModRoutes = 64;
constexpr int kMaxAudioRateRoutes = 16;   // cible benchmark du référentiel
constexpr int kMaxCurvePoints = 8;

const char* modSourceName(ModSource s) noexcept;      // nom persistant ("lfo1", "env.amp", ...)
const char* modSourceLabel(ModSource s) noexcept;     // libellé UI
bool modSourceFromName(const std::string& name, ModSource& out) noexcept;
bool modSourceBipolar(ModSource s) noexcept;          // plage naturelle [-1,1] (sinon [0,1])
// Étage audio mesuré par une source (suiveurs, Y2) ; -1 si la source ne dépend pas du signal audio.
int modSourceAudioStage(ModSource s) noexcept;

enum class ModCurve : std::uint8_t { Linear = 0, Exponential, Logarithmic, SCurve, Stepped, Fold, Custom, Count };
const char* modCurveName(ModCurve c) noexcept;
enum class ModPolarity : std::uint8_t { Centered = 0, Free = 1 };
enum class ModRate : std::uint8_t { Control = 0, Audio = 1 };

struct ModRoute {
  std::uint32_t id = 0;                 // StableModRouteID, jamais réutilisé
  ModSource source = ModSource::None;
  ParamId dest = ParamId::Count;        // persisté par StableID
  float depth = 0.0f;                   // -1..1 (x modScale de la destination)
  ModCurve curve = ModCurve::Linear;
  ModPolarity polarity = ModPolarity::Centered;
  bool invert = false;
  float offset = 0.0f;                  // -1..1 (mode Free)
  ModSource via = ModSource::None;
  float viaAmount = 1.0f;               // a dans V = (1-a) + a*v
  float smoothMs = 0.0f;                // 0..1000
  ModRate rate = ModRate::Control;
  bool enabled = true;
  bool locked = false;                  // protégé contre Randomize/Mutate
  std::uint8_t numPoints = 2;           // courbe Custom : 2..8 points (x croissants dans [0,1])
  std::array<float, kMaxCurvePoints> px{0.0f, 1.0f}, py{0.0f, 1.0f};
  bool operator==(const ModRoute& o) const noexcept;
  bool sameConnection(const ModRoute& o) const noexcept;  // même contenu hors identifiant
};

// Raison d'inactivité d'une connexion (C27).
enum class ModInactive : std::uint8_t {
  Active = 0, Disabled, NoSource, DestinationNotModulatable, AudioRateUnsupported, AudioRateBudget, FollowerCycle
};
const char* modInactiveText(ModInactive r) noexcept;

struct ModMatrixState {
  std::vector<ModRoute> routes;
  std::uint32_t nextId = 1;
  bool operator==(const ModMatrixState& o) const noexcept { return routes == o.routes && nextId == o.nextId; }
};

// Validation structurelle complète (chargement, transactions) : capacité, ids, plages, cycles macro.
bool validateModMatrix(const ModMatrixState& m, std::string& error);
// Raison d'inactivité de chaque connexion d'un état valide, dans l'ordre des routes (budget audio-rate compris).
std::vector<ModInactive> modRouteStatus(const ModMatrixState& m);
bool paramModulatable(ParamId id) noexcept;

// Transactions de la matrice (hors RT) : atomiques.
struct ModTxResult {
  bool ok = false;
  std::string error;
  std::uint32_t id = 0;
};
ModTxResult modAdd(ModMatrixState& m, ModRoute r);             // r.id ignoré, nouvel identifiant attribué
ModTxResult modUpdate(ModMatrixState& m, const ModRoute& r);   // même id
ModTxResult modRemove(ModMatrixState& m, std::uint32_t id);
// Fusion (preset de matrice) : doublon exact ignoré, conflit d'identifiant refusé si contenu différent, capacité.
ModTxResult modMerge(ModMatrixState& m, const ModMatrixState& other, bool replace);

// Évaluation d'une courbe (RT-safe). x dans [0,1]. Forme brute partagée par la forme compilée du moteur.
float modCurveValue(ModCurve c, int numPoints, const float* px, const float* py, float x) noexcept;
float evalModCurve(const ModRoute& r, float x) noexcept;

}  // namespace sbd
