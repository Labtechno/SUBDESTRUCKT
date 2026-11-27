// SUBDESTRUCKT — UI — design tokens SYMBIOTIC INDUSTRIAL (ARCH-UI-04, contrat docs/V1-ARCH-UI-03-SPEC.md §2).
// Source unique des couleurs, dimensions, espacements, rayons, typographie et états des composants. Aucun composant ne
// définit de couleur ou de dimension en dur : tout passe par ce fichier (documentation : docs/V1-ARCH-UI-04-DESIGN-SYSTEM.md).
// Graphite minéral, titane noir, verre optique fumé ; accents lumineux maîtrisés ; rouge réservé aux alertes et au Panic.
// Aucun style CRT, rétro ou vintage ; aucune accumulation de néons.
#pragma once
#include <juce_graphics/juce_graphics.h>

namespace sbd::ui {

namespace tok {
// ---------------------------------------------------------------- palette de référence (propriétaire)
inline const juce::Colour chassis{0xff161a1d};   // châssis graphite minéral
inline const juce::Colour carbon{0xff0a0d10};    // carbone : fonds de verre optique, cavités
inline const juce::Colour titanium{0xff343c43};  // titane noir : corps des commandes, filets
inline const juce::Colour text{0xffdce4e9};      // texte principal
inline const juce::Colour wt{0xff5ce2cb};        // moteur Wavetable
inline const juce::Colour fm{0xfff2a05e};        // moteur FM / PM
inline const juce::Colour cross{0xffb095ff};     // CROSS (interaction des moteurs)
inline const juce::Colour mod{0xff7fa9e8};       // modulation (Effective, sources, routes globales)
inline const juce::Colour alert{0xffe0524a};     // rouge : alertes et Panic uniquement

// ---------------------------------------------------------------- dérivés (calculés une fois, jamais ad hoc)
inline const juce::Colour panel{0xff1c2125};      // plaque de section (graphite plus clair que le châssis)
inline const juce::Colour panelEdge{0xff2a3137};  // arête usinée des plaques
inline const juce::Colour glass{0xff0d1215};      // verre optique fumé (afficheurs)
inline const juce::Colour glassEdge{0xff26303a};  // monture du verre
inline const juce::Colour textDim{0xff98a5ae};    // texte secondaire (contraste ≥ 4,5:1 sur panel, mesuré par UI04-CON)
inline const juce::Colour textMute{0xff7d8a93};   // libellés inactifs (≥ 3:1 sur panel)
inline const juce::Colour track{0xff10151a};      // piste des arcs et rails
inline const juce::Colour focus{0xffdce4e9};      // anneau de focus clavier (texte, non chromatique)
inline const juce::Colour warn{0xffe6c35a};       // état ARMED (attente d'autorisation) — distinct du rouge

// ---------------------------------------------------------------- grille et espacements (px logiques, zoom 100 %)
constexpr int unit = 4;           // module de base
constexpr int gap = 2 * unit;     // espacement entre commandes
constexpr int pad = 2 * unit;     // marge interne des plaques
constexpr int barH = 44;          // barre globale
constexpr int stripH = 92;        // bandeau macros / balance / sortie (Performance)
constexpr int tabH = 28;          // onglets
constexpr int sectionHead = 22;   // bandeau de titre d'une plaque
constexpr int inspectorW = 300;   // Inspector (rétractable / épinglable)
constexpr float radius = 5.0f;    // rayon des plaques
constexpr float radiusSmall = 3.0f;

// ---------------------------------------------------------------- dimensions des composants (cellules)
struct Cell {
  int w, h;
};
constexpr Cell nexusXL{132, 150};    // NEXUS XL : commandes Performance majeures
constexpr Cell nexusS{84, 96};       // NEXUS S : édition courante
constexpr Cell nexusMicro{64, 74};   // NEXUS Micro : matrices, réseaux, listes denses
constexpr Cell track1{168, 46};      // SYNAPTIC TRACK (sélection discrète)
constexpr Cell cell1{120, 40};       // NEURAL CELL (interrupteur / action)
constexpr Cell plasmaV{48, 150};     // PLASMA CHANNEL vertical
constexpr Cell plasmaH{220, 40};     // PLASMA CHANNEL horizontal

// ---------------------------------------------------------------- typographie (sans-serif technique, chiffres tabulaires)
constexpr float fontMicro = 9.5f;   // plancher absolu (aucun libellé plus petit ; sinon retour à la ligne)
constexpr float fontLabel = 11.0f;
constexpr float fontValue = 11.0f;
constexpr float fontTitle = 12.5f;
constexpr float fontHead = 15.0f;
constexpr float fontXL = 14.0f;

// ---------------------------------------------------------------- interaction
constexpr double dragPixelsFull = 220.0;  // course verticale pour la plage complète
constexpr double fineFactor = 0.1;        // Maj : réglage fin
constexpr double keyStep = 0.01;          // flèches : 1 % de la plage normalisée (Int/Choice : 1 pas)
constexpr double pageStep = 0.10;         // Page Haut/Bas
constexpr int wheelGestureMs = 450;       // molette : un geste (une transaction Undo) jusqu'à ce délai d'inactivité
constexpr int refreshHz = 30;             // cadence de l'éditeur (télémétrie décimée)
}  // namespace tok

// États visuels d'un composant (contrat ARCH-UI-03 §2) : texte et forme, jamais la seule couleur.
enum class CellState { Off, Armed, Active, Refused, Disabled };

// Domaine sémantique d'un paramètre (couleur d'accent).
enum class Domain3 { Wt, Fm, Cross, Mod, Neutral, Alert };
juce::Colour accentOf(Domain3 d);

juce::Font uiFont(float h, bool bold = false);
juce::Font numFont(float h, bool bold = false);  // chiffres tabulaires (police monospace système)

// Compatibilité : moniteurs et vues historiques utilisent ces alias, tous définis par les tokens.
namespace col {
inline const juce::Colour chassis = tok::chassis, panel = tok::panel, text = tok::text, dim = tok::textDim, wt = tok::wt,
                          fm = tok::fm, cross = tok::cross, mod = tok::mod, display = tok::glass, control = tok::titanium,
                          alert = tok::alert;
}
inline juce::Font mono(float h, bool bold = false) { return numFont(h, bold); }
inline juce::Font sans(float h, bool bold = false) { return uiFont(h, bold); }

}  // namespace sbd::ui
