// SUBDESTRUCKT — UI — vues des trois espaces (ARCH-UI-04) sur un état unique.
//   PERFORMANCE   : WT X/Y/Z, deux commandes FM contextuelles (paramètres existants), SYMBIOTIC CORE (X = cross.fwd,
//                   Y = cross.rev, Z = cross.master, alternative numérique complète), macros, balance, sortie, mètres,
//                   états autonomes, moniteur réel.
//   SOUND DESIGN  : MOTEURS (WT | CROSS central | FM), TRAITEMENTS (chaînes WT, FM, Global : filtre Pre/Post + deux
//                   emplacements Destruction, contextuels à l'algorithme), MODULATION (5 enveloppes, 4 LFO, aléatoire,
//                   chaos, suiveurs).
//   ENGINEERING   : FM Network, Cross Network, matrice globale, Bank Laboratory, Voice/Tuning, Presets/State,
//                   Randomize/Mutate/Evolve, MIDI Learn, Diagnostics, Captures/Assets.
// Naviguer (espaces, onglets, sous-contextes) ne modifie jamais l'état sonore ; les valeurs inactives sont conservées
// (une commande masquée n'efface rien). Les sous-contextes sont énumérables (test de couverture UI04-COV).
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>

#include "Components.h"
#include "CurveEditors.h"
#include "Monitors.h"

namespace sbd::ui {

// Vue d'un espace : onglets, sous-contextes (opérateur, enveloppe, LFO, volet local…), rafraîchissements.
class SpaceView : public juce::Component {
 public:
  explicit SpaceView(UiContext& c) : ctx_(c) {}
  virtual int numTabs() const = 0;
  virtual juce::String tabName(int k) const = 0;
  virtual void showTab(int k) = 0;
  virtual int currentTab() const = 0;
  virtual int numSubContexts(int tab) const { juce::ignoreUnused(tab); return 1; }
  virtual void showSubContext(int tab, int sub) { juce::ignoreUnused(tab, sub); }
  virtual juce::String subContextName(int tab, int sub) const { juce::ignoreUnused(tab, sub); return {}; }
  virtual void refreshState() {}   // changement d'état non-hôte ou d'algorithme (disposition contextuelle)
  virtual void tick() {}           // télémétrie et textes (cadence éditeur), vue visible seulement
  virtual juce::Point<int> minimumSize() const { return {1080, 700}; }

 protected:
  UiContext& ctx_;
};

// SYMBIOTIC CORE : accès de test au geste direct X/Y (même chemin que le glisser).
struct CorePad {
  virtual ~CorePad() = default;
  virtual void userMove(float x, float y) = 0;
  virtual float shownX() const = 0;
  virtual float shownY() const = 0;
};

std::unique_ptr<SpaceView> makePerformanceView(UiContext& c);
std::unique_ptr<SpaceView> makeSoundDesignView(UiContext& c);
std::unique_ptr<SpaceView> makeEngineeringView(UiContext& c);

// Destination Inspector : contexte du paramètre sélectionné (Base / Effective / unités / plage / défaut / automation /
// MIDI / modulations), alias de réglage, ajout de modulation, verrou Randomize, MIDI Learn ; rétractable et épinglable.
class InspectorPanel : public juce::Component {
 public:
  explicit InspectorPanel(UiContext& c);
  ~InspectorPanel() override;
  void show(ParamId id);
  void refresh();
  ParamId current() const { return id_; }
  bool pinned() const;
  void resized() override;
  void paint(juce::Graphics&) override;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
  UiContext& ctx_;
  ParamId id_ = ParamId::WtX;
};

// Paramètres d'un emplacement de destruction présentés pour son algorithme (exposé pour les tests de couverture).
std::vector<ParamId> destructionParamsFor(ParamId typeParam, int type);
std::vector<ParamId> filterParamsFor(ParamId typeParam, int type);

}  // namespace sbd::ui
