// SUBDESTRUCKT — L1 — état persistant versionné (preset minimal + état DAW) de la tranche ARCH-G3.
//
// Format texte v1 (UTF-8, une entrée par ligne) :
//   SUBDESTRUCKT_STATE 1
//   writer <identité de build>                      (informatif, ignoré à la lecture)
//   param <StableID> <bits IEEE-754 float en hex> <valeur lisible>   (une ligne par paramètre v1, ordre du registre)
//   topology next=<nextRouteId>
//   route <id> <src> <dst> <I|D>                    (triées par identifiant)
//   end
// Les valeurs sont relues depuis les bits hexadécimaux : aucune dépendance à la locale de l'hôte, aller-retour exact.
// Lecture stricte et transactionnelle : toute anomalie refuse l'état entier ; la sortie n'est modifiée qu'en cas de
// succès. Une version future est refusée explicitement (aucune lecture de format plus récent). Le Runtime State
// (phases, historiques de retard) n'est pas persisté.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "Parameters.h"
#include "Topology.h"

namespace sbd {

constexpr int kStateVersion = 1;

struct PluginState {
  ParamValues params = ParamValues::defaults();
  std::vector<Route> routes;
  std::uint32_t nextRouteId = 1;
  bool operator==(const PluginState& o) const noexcept {
    return params == o.params && routes == o.routes && nextRouteId == o.nextRouteId;
  }
};

// Preset minimal (état initial) : paramètres par défaut et topologie OP2>OP1 (I), WT>OP1 (I), OP1>WT (D).
PluginState defaultState();

std::string encodeState(const PluginState& state, const std::string& writer);
bool decodeState(const std::string& text, PluginState& out, std::string& error);

}  // namespace sbd
