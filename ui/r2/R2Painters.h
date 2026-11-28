// SUBDESTRUCKT — ARCH-UI-04-R2-B2 — peintres R1 partagés (TB2-02) : NEXUS Hybrid, PLASMA CHANNEL, NEURAL CELL Machined
// Titanium (PANIC compris), SYNAPTIC Machined Track. Consommés par le plugin et par le Component Lab.
//
// Contrat (B2-00-ADDENDUM.md §6) : un peintre reçoit une VUE (valeurs, textes, états visuels) et une géométrie, et dessine.
// Il n'écrit aucun paramètre, n'ouvre aucun geste hôte, ne lit ni l'automation ni le DSP, et ne garde aucun état mutable
// partagé entre instances. Les gestionnaires d'événements restent propres à chaque consommateur. Les fonctions de
// géométrie (zone de manipulation, zone de valeur, canal, segments) sont publiques pour que le rendu et les interactions
// d'un consommateur utilisent la même définition (DB2-08).
#pragma once
#include <optional>
#include <vector>

#include "ui/r2/R2Style.h"

namespace sbd::r2 {

// ---------------------------------------------------------------- NEXUS Hybrid (LAB-A01, gelé)
enum class NexusSize { XL, S, Micro };

struct NexusView {
  NexusSize size = NexusSize::S;
  Domain domain = Domain::Neutral;
  juce::String label;          // sérigraphie
  float labelPx = 0.0f;        // 0 = corps de la famille
  float norm = 0.0f;           // valeur Base normalisée
  std::optional<float> effective;  // valeur Effective normalisée, présente seulement si la destination est modulée
  bool bipolar = false;
  float zeroNorm = 0.5f;  // position normalisée du zéro d'un paramètre bipolaire (0,5 si la plage est symétrique)
  std::vector<float> routeDepths;  // profondeurs signées des routes vers la destination (au plus quatre marques, puis +n)
  juce::String valueText;      // valeur Base formatée avec unité
  bool inactive = false, entry = false, disabled = false, hover = false, focus = false, pressed = false;
  // mise en page du libellé déjà calculée par le consommateur (même règle : nexusLabelLayout) ; 0 : calculée par le peintre
  int labelLines = 0;   // 1 ou 2
  int labelSplit = -1;  // fittedSplit (si labelLines != 0)
};

float nexusAngle(float norm);
float nexusFamilyLabelPx(NexusSize size);  // corps de sérigraphie de la famille (XL 12,5 gras, S 11, Micro 10)
float nexusValuePx(NexusSize size);        // corps de la valeur (XL 14, S 11, Micro 10)
juce::Rectangle<float> nexusValueArea(juce::Rectangle<int> localBounds, NexusSize size);
juce::Rectangle<float> nexusKnobArea(juce::Rectangle<int> localBounds, NexusSize size, const juce::String& label, float labelPx);
juce::Rectangle<float> nexusKnobAreaLines(juce::Rectangle<int> localBounds, NexusSize size, bool twoLabelLines, float labelPx);
// mise en page du libellé selon la règle du peintre : lines = 1 ou 2 (hauteur de la sérigraphie), split = fittedSplit
void nexusLabelLayout(juce::Rectangle<int> localBounds, NexusSize size, const juce::String& label, float labelPx, int& lines, int& split);
void paintNexus(juce::Graphics& g, juce::Rectangle<int> localBounds, const NexusView& v);

// ---------------------------------------------------------------- PLASMA CHANNEL (LAB-A05, LAB-A07.2)
enum class PlasmaOrientation { Vertical, Horizontal };

struct PlasmaView {
  PlasmaOrientation orientation = PlasmaOrientation::Vertical;
  Domain domain = Domain::Neutral;
  juce::String label;
  juce::StringArray graduations;  // du minimum au maximum, réparties régulièrement
  bool compact = false;           // gabarit vertical COMPACT (choisi par le consommateur, CE-B2-02)
  float norm = 0.0f;
  std::optional<float> effective;
  bool bipolar = false;
  float zeroNorm = 0.5f;  // position normalisée du zéro d'un paramètre bipolaire
  juce::String valueText;
  bool disabled = false, hover = false, focus = false, pressed = false;
};

juce::Rectangle<float> plasmaSlot(juce::Rectangle<int> localBounds, PlasmaOrientation o, bool compact);  // canal (zone de manipulation)
juce::Point<float> plasmaPosOf(juce::Rectangle<float> slot, PlasmaOrientation o, float norm);
void paintPlasma(juce::Graphics& g, juce::Rectangle<int> localBounds, const PlasmaView& v);

// ---------------------------------------------------------------- NEURAL CELL Machined Titanium (LAB-A02), PANIC (LAB-A07.1)
enum class CellKind { Action, Toggle, Arm, Select, Safety };
enum class CellArm { Off, Armed, Active, Denied };
enum class CellIcon { Pulse, Capture, Authorize, None };

struct CellView {
  CellKind kind = CellKind::Action;
  Domain domain = Domain::Neutral;
  juce::String label;      // vide : touche dessinée sans libellé (mesure de contraste sur la surface rendue)
  juce::String stateText;  // OFF, ON, ARMED, ACTIVE, DENIED, — (texte d'état, jamais la couleur seule)
  CellArm arm = CellArm::Off;
  CellIcon icon = CellIcon::None;
  bool on = false;        // bascule
  bool selected = false;  // sélection exclusive
  bool disabled = false, hover = false, focus = false, pressed = false;
};

void paintCellIcon(juce::Graphics& g, CellIcon icon, juce::Rectangle<float> r, juce::Colour c);
void paintCell(juce::Graphics& g, juce::Rectangle<int> localBounds, const CellView& v);

// ---------------------------------------------------------------- SYNAPTIC Machined Track (LAB-A03)
enum class TrackMode { Segments, List, Tabs };

struct TrackView {
  TrackMode mode = TrackMode::Segments;
  Domain domain = Domain::Neutral;
  juce::String label;  // légende (segments et liste) ; vide pour les onglets
  juce::StringArray options;
  int index = 0;
  int hover = -1;  // segment survolé (-1 : aucun)
  bool disabled = false, focus = false;
};

juce::Rectangle<float> trackArea(juce::Rectangle<int> localBounds, TrackMode mode, bool hasLabel);
juce::Rectangle<float> trackSegment(juce::Rectangle<int> localBounds, const TrackView& v, int k);
void paintTrack(juce::Graphics& g, juce::Rectangle<int> localBounds, const TrackView& v);

}  // namespace sbd::r2
