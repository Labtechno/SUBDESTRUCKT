// SUBDESTRUCKT — ARCH-UI-04-R2-B2 — identifiants stables de composants (OD-01, option B ; règles ID-01 à ID-08 de
// docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/INTERACTION-TEST-PLAN.md §1).
//  - param:<StableID> pour une commande liée à un paramètre (703, déduits du registre, jamais listés) ;
//  - action:<réf>, state:<réf>, nav:<espace>/<groupe>/<outil> : registre GÉNÉRÉ en ajout seul (ui/r2/ComponentIds.inc,
//    tools/gen_ui_labels.py --check).
// Un identifiant est ASCII et indépendant du texte visible, de la langue et de la sérigraphie. Le titre accessible n'est
// jamais un identifiant de test (ID-04).
#pragma once
#include <juce_core/juce_core.h>

#include <vector>

#include "core/Parameters.h"

namespace sbd::ui::ids {

inline juce::String param(ParamId id) { return juce::String("param:") + paramSpec(id).stableId; }

// registre non paramétrique, dans l'ordre de publication (immuable)
inline const std::vector<const char*>& registered() {
  static const std::vector<const char*> v{
#define SBD_UI_COMPONENT_ID(x) x,
#include "ui/r2/ComponentIds.inc"
#undef SBD_UI_COMPONENT_ID
  };
  return v;
}

inline bool isRegistered(const juce::String& id) {
  for (const char* r : registered())
    if (id == r) return true;
  return false;
}

// forme : <espèce>:<référence>, espèce dans {param, action, state, nav, view}, référence ASCII [a-z0-9._/-], non vide
inline bool isWellFormed(const juce::String& id) {
  const int c = id.indexOfChar(':');
  if (c <= 0 || c == id.length() - 1) return false;
  const auto kind = id.substring(0, c);
  if (kind != "param" && kind != "action" && kind != "state" && kind != "nav" && kind != "view") return false;
  for (auto ch : id.substring(c + 1)) {
    const bool ok = (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '.' || ch == '_' || ch == '/' || ch == '-';
    if (!ok) return false;
  }
  return true;
}

}  // namespace sbd::ui::ids
