// SUBDESTRUCKT — ARCH-UI-04-R2-B1 / B2-03 — scène du comparatif 0.4.1 / R2 (planche 12), partagée par sbd_lab_compare et
// par VIS-01 (B2-03). À gauche : composants de PRODUCTION (0.4.1 jusqu'à B2-02, puis rendu R1 intégré famille par
// famille). À droite : composants R1 du laboratoire liés aux MÊMES paramètres réels par l'adaptateur ParamSource.
#pragma once
#include <memory>
#include <vector>

#include "app/Sheets.h"
#include "ui/Components.h"

namespace sbd::lab {

struct CompareScene {
  CompareScene();
  ~CompareScene();
  SubdestrucktProcessor proc;  // aucun traitement audio : instancié pour ses paramètres
  ui::UiContext ctx{proc, nullptr, RandomOptions{}, {}, {}, {}, {}, ParamId::WtX, {}};
  std::vector<std::unique_ptr<r2::ValueSource>> sources;  // déclarées avant la planche : détruites après ses composants
  std::unique_ptr<Sheet> sheet;
  std::vector<std::unique_ptr<juce::Component>> legacy;  // composants de production de la plaque gauche
  // plaque gauche (production), en coordonnées logiques : zone qui évolue avec l'intégration B2-03
  static juce::Rectangle<int> productionPlate() { return {16, 76, 596, 880}; }
};

void setParam(SubdestrucktProcessor& p, ParamId id, float value);
// onlySheet vide : toutes les planches ; b1Dir : captures B1 de la plateforme (historique de rendu de la planche 11)
int runVis01(const juce::File& outDir, const juce::String& onlySheet, const juce::File& b1Dir);  // VIS-01, étape 2 de CE-B2-03 (Vis01.cpp)

}  // namespace sbd::lab
