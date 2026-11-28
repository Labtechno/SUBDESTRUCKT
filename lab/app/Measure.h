// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab — mesures (PT-02 typographie, PT-05 glyphes, contrastes).
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace sbd::lab {

// PT-02 : mesure JUCE réelle des libellés proposés (R2-CONTROL-LABELS-PROPOSALS.csv) aux zooms 75 / 100 / 150 %.
int runMeasure(const juce::File& csvIn, const juce::File& csvOut, juce::StringArray& log);
// PT-05 : présence native des glyphes → · … − dans les polices de l'interface, substitution éventuelle.
int runGlyphs(const juce::File& csvOut, juce::StringArray& log);
// Contrastes WCAG des paires de tokens (texte, éléments d'interface, niveaux de relief).
int runContrast(const juce::File& csvOut, juce::StringArray& log);

// PANIC (LAB-A07.1) : contraste du libellé mesuré sur la SURFACE RENDUE (dégradé réel, chaque état, chaque échelle).
// La touche est rendue sans libellé ; chaque pixel de la zone du libellé est comparé à la couleur du libellé.
struct PanicMeasure {
  double minRatio = 0.0, maxRatio = 0.0;
  int pixels = 0;
};
PanicMeasure panicLabelContrast(int state /* 0 normal, 1 hover, 2 pressed, 3 focus */, float scale, bool carbon, bool r1);
int runPanicContrast(const juce::File& csvOut, juce::StringArray& log);

// CSV minimal (RFC 4180 : guillemets, BOM UTF-8 accepté)
std::vector<juce::StringArray> readCsv(const juce::File& f);
bool writeCsv(const juce::File& f, const std::vector<juce::StringArray>& rows);

}  // namespace sbd::lab
