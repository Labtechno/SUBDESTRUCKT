// SUBDESTRUCKT — ARCH-UI-04-R2-B2 : tests de la fondation R1 (B2-02), exécutés par subdestruckt_wrapper_tests.
// Critères (enregistrés avant mesure, docs/ARCH-UI-04-R2/B2-PRODUCTION-INTEGRATION/B2-TEST-PLAN.md §4) :
//  ID-UNIQ    registre des identifiants de composants (OD-01) : 703 identifiants param:<StableID> et registre généré
//             action: / state: / nav: (ajout seul) uniques, bien formés (ASCII, schéma ID-02), sans collision d'espèce
//  LBL-TABLE  table des alias d'affichage générée depuis la nomenclature R1 : 2 214 occurrences (dont 537 options de liste
//             indexées 0..n-1), 324 validées (P1, LAB-A08),
//             aucune entrée validée hors P1, libellés non vides, chaque référence paramétrique désigne un StableID du registre
#include <juce_core/juce_core.h>

#include <set>
#include <string>

#include "TestKit.h"
#include "core/Parameters.h"
#include "ui/r2/ComponentIds.h"
#include "ui/r2/Labels.h"

using namespace sbd;

void testR2B2Foundation() {
  std::fprintf(stderr, "[ARCH-UI-04-R2-B2-02 : fondation R1 (identifiants, libelles)]\n");
  // ID-UNIQ
  {
    std::set<std::string> all;
    int bad = 0, dup = 0, params = 0;
    for (int i = 0; i < kNumParams; ++i) {
      const auto id = ui::ids::param(static_cast<ParamId>(i));
      bad += ui::ids::isWellFormed(id) ? 0 : 1;
      dup += all.insert(id.toStdString()).second ? 0 : 1;
      ++params;
    }
    int reg = 0;
    for (const char* r : ui::ids::registered()) {
      const juce::String id(r);
      bad += (ui::ids::isWellFormed(id) && !id.startsWith("param:")) ? 0 : 1;
      dup += all.insert(id.toStdString()).second ? 0 : 1;
      ++reg;
    }
    const bool ok = params == 703 && reg > 0 && bad == 0 && dup == 0 && ui::ids::isRegistered("action:bar.panic") &&
                    ui::ids::isRegistered("action:bar.authorize") && ui::ids::isRegistered("action:bar.drone") &&
                    !ui::ids::isWellFormed("PANIC") && !ui::ids::isWellFormed("action:Bar.Panic");
    TK_CHECK(ok);
    tk::result("ID-UNIQ", ok, "identifiants : %d param + %d registre, mal formes %d, doublons %d", params, reg, bad, dup);
  }
  // LBL-TABLE
  {
    std::set<std::string> stable;
    for (int i = 0; i < kNumParams; ++i) stable.insert(paramSpec(static_cast<ParamId>(i)).stableId);
    int n = 0, validated = 0, validatedNotP1 = 0, empty = 0, paramRefs = 0, unknownParam = 0, options = 0;
    for (const auto& e : ui::labels::table()) {
      ++n;
      validated += e.validated ? 1 : 0;
      validatedNotP1 += (e.validated && std::string(e.priority) != "P1") ? 1 : 0;
      empty += (juce::String::fromUTF8(e.shortLabel).isEmpty() || e.occurrence < 1 || e.option < -1) ? 1 : 0;
      options += e.option >= 0 ? 1 : 0;
      const std::string ref(e.ref);
      const bool looksParam = stable.count(ref) > 0;
      paramRefs += looksParam ? 1 : 0;
      // une référence d'emplacement de paramètre (contenant un point et sans espace de nommage hors registre) doit exister
      const bool nonParamNamespace = ref.rfind("bar.", 0) == 0 || ref.rfind("perf.", 0) == 0 || ref.rfind("sound.", 0) == 0 ||
                                     ref.rfind("eng.", 0) == 0 || ref.rfind("inspector.", 0) == 0 || ref.rfind("state.", 0) == 0 ||
                                     ref.rfind("units.", 0) == 0;
      unknownParam += (!looksParam && !nonParamNamespace && ref.find(':') == std::string::npos && ref.find("choice") == std::string::npos &&
                       ref.find('=') == std::string::npos && ref.find('/') == std::string::npos && ref.find('[') == std::string::npos)
                          ? 1
                          : 0;
    }
    const bool ok = n == 2214 && validated == 324 && validatedNotP1 == 0 && empty == 0 && paramRefs > 0 && options == 537;
    TK_CHECK(ok);
    tk::result("LBL-TABLE", ok, "table des libelles : %d occurrences dont %d options de liste, %d validees (P1), validees hors P1 %d, vides ou mal indexees %d, references parametriques %d, references non resolues (information) %d",
               n, options, validated, validatedNotP1, empty, paramRefs, unknownParam);
  }
}
