// SUBDESTRUCKT — L1 — état persistant versionné (preset + état DAW).
//
// Format texte v2 (UTF-8, une entrée par ligne, ARCH-G4) :
//   SUBDESTRUCKT_STATE 2
//   writer <identité de build>                        (informatif, ignoré)
//   scope preset|daw                                  (daw : inclut la section instance)
//   param <StableID> <bits IEEE-754 en hex> <lisible> (tous les paramètres de version <= 2)
//   topology next=<n>
//   route <id> <src> <port> <I|D>                     (triées par identifiant)
//   bank <assetId>                                    (banque WT référencée)
//   bankdata <nx> <ny> <nom hex> <float32 LE base64>  (banque utilisateur intégrée ; intégrité : id = SHA-256)
//   modnext <n>
//   mod <id> <source> <destStableID> <depthHex> <curve> <C|F> <inv> <offsetHex> <via> <viaHex> <smoothHex> <C|A>
//       <enabled> <locked> <n> <x0Hex> <y0Hex> ...    (une ligne par connexion de la matrice globale)
//   macroname <1..4> <utf8 hex>
//   lfocurve <1..4> <n> <x0Hex> <y0Hex> ...
//   seed <master 16 hex> <randomize> <mutate> <evolve>  (compteurs de flux)
//   instance quality=<Eco|Normal|High> drone=<0|1> drone_restore=<0|1>   (scope daw uniquement)
//   end
// Le format v1 (0.1.0-b5) reste lisible : migration explicite (paramètres v1 requis, les autres à leur défaut,
// banque usine, matrice vide). Lecture stricte et transactionnelle : toute anomalie refuse l'état entier ; la sortie
// n'est modifiée qu'en cas de succès. Une version future est refusée explicitement. Le Runtime State n'est pas persisté.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "ModMatrix.h"
#include "Parameters.h"
#include "Topology.h"

namespace sbd {

constexpr int kStateVersion = 2;


struct CurvePoints {
  std::uint8_t n = 2;
  std::array<float, kMaxCurvePoints> x{0.0f, 1.0f}, y{-1.0f, 1.0f};
  bool operator==(const CurvePoints& o) const noexcept;
};

// Application/instance state : préférence d'instance/projet, jamais dans un preset (P0-C13, Drone §9).
struct InstanceState {
  Quality quality = Quality::Normal;
  bool drone = false;
  bool droneRestore = false;  // autorisation locale de restaurer Drone au chargement du projet
  bool operator==(const InstanceState& o) const noexcept {
    return quality == o.quality && drone == o.drone && droneRestore == o.droneRestore;
  }
};

struct PluginState {
  ParamValues params = ParamValues::defaults();
  std::vector<Route> routes;
  std::uint32_t nextRouteId = 1;
  std::string bank = "factory:saw";
  // banque utilisateur intégrée (état autonome) : vide pour une banque usine
  std::vector<float> bankRaw;
  int bankNx = 0, bankNy = 0;
  std::string bankName;
  ModMatrixState mod;
  std::array<std::string, 4> macroNames{"Macro 1", "Macro 2", "Macro 3", "Macro 4"};
  std::array<CurvePoints, 4> lfoCurves{};
  std::uint64_t seed = 0x5B0D5EEDULL;
  std::array<std::uint64_t, 3> streamCounters{};  // Randomize, Mutate, Evolve
  InstanceState instance;
  int sourceVersion = kStateVersion;  // version lue (diagnostic de migration) ; non comparée
  bool hasInstance = true;            // faux pour un preset (section instance absente) ; non comparé
  bool operator==(const PluginState& o) const noexcept {
    return params == o.params && routes == o.routes && nextRouteId == o.nextRouteId && bank == o.bank && mod == o.mod &&
           bankRaw == o.bankRaw && bankNx == o.bankNx && bankNy == o.bankNy && bankName == o.bankName &&
           macroNames == o.macroNames && lfoCurves == o.lfoCurves && seed == o.seed &&
           streamCounters == o.streamCounters && instance == o.instance;
  }
  bool operator!=(const PluginState& o) const noexcept { return !(*this == o); }
};

// Preset minimal (état initial) : paramètres par défaut et topologie OP2>OP1 (I), WT>OP1 (I), OP1>WT (D).
PluginState defaultState();

enum class StateScope { Preset, Daw };
std::string encodeState(const PluginState& state, const std::string& writer, StateScope scope = StateScope::Daw);
bool decodeState(const std::string& text, PluginState& out, std::string& error);

// Validation sémantique complète (utilisée par le décodeur et les transactions).
bool validateState(const PluginState& s, std::string& error);

}  // namespace sbd
