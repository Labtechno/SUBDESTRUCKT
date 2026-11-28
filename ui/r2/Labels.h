// SUBDESTRUCKT — ARCH-UI-04-R2-B2 — table des alias d'affichage (nomenclature R1, LAB-A08 ; OD-18). Générée par
// tools/gen_ui_labels.py depuis docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv (ui/r2/Labels.inc, --check).
// Une entrée par occurrence. Seules les entrées « validées » (lexique P1, LAB-A08) peuvent être appliquées ; les P2 / P3
// sont des propositions soumises à validation groupée (DB2-03) et ne sont jamais appliquées silencieusement. Les noms
// hôtes du registre ne changent pas (INV-02) : ce sont des alias d'affichage.
#pragma once
#include <vector>

namespace sbd::ui::labels {

struct Entry {
  const char* ref;        // StableID ou référence hors registre (sans suffixe d'occurrence)
  int occurrence;         // rang d'occurrence de l'emplacement (1 pour la première)
  int option;             // indice d'option d'une liste (0..n-1), -1 pour le libellé d'une commande
  const char* location;   // emplacement (espace / plaque / contexte) tel que mesuré par PT-02
  const char* shortLabel; // sérigraphie courte proposée ou validée (UTF-8)
  const char* fullName;   // nom complet (Inspector, infobulle)
  const char* priority;   // P1, P2, P3
  bool validated;         // lexique validé (P1, LAB-A08)
};

inline const std::vector<Entry>& table() {
  static const std::vector<Entry> t{
#define SBD_UI_LABEL(ref, occ, opt, loc, lab, full, prio, val) Entry{ref, occ, opt, loc, lab, full, prio, val != 0},
#include "ui/r2/Labels.inc"
#undef SBD_UI_LABEL
  };
  return t;
}

}  // namespace sbd::ui::labels
