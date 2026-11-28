// SUBDESTRUCKT — Component Lab — style : le Design System R1 partagé (ui/r2/R2Style.h) et les éléments propres au
// laboratoire (révision B1 des comparatifs historiques, primitives B1, étiquette DEMO DATA). Non intégrés au plugin.
#pragma once
#include "ui/r2/R2Style.h"

namespace sbd::r2 {

// Révision de rendu : B1 (Component Lab initial, conservée pour les comparatifs avant/après) ou R1 (LAB-A02 à A04).
// NEXUS et PLASMA n'ont pas de révision : leur apparence est validée (LAB-A01, LAB-A05).
enum class Rev { B1, R1 };

namespace draw {
// témoin intégré : 0 éteint, 1 anneau (armé), 2 disque plein (actif)
void led(juce::Graphics&, juce::Point<float> c, float r, int mode, juce::Colour col);
// cadre optique cybernétique : verre, monture, repères d'angle
void opticalFrame(juce::Graphics&, juce::Rectangle<float> r, juce::Colour accent, bool primary);
// étiquette DEMO DATA (obligatoire sur toute donnée de démonstration)
void demoTag(juce::Graphics&, juce::Rectangle<float> area);
}  // namespace draw

}  // namespace sbd::r2
