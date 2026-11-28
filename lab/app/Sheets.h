// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab — planches de captures (rendus JUCE réels, hors écran).
#pragma once
#include <memory>

#include "app/DemoData.h"
#include "r2/R2Nexus.h"
#include "r2/R2NeuralCell.h"
#include "r2/R2PlasmaChannel.h"
#include "r2/R2SynapticTrack.h"

namespace sbd::lab {

// Planche : fond châssis, en-tête (titre, sous-titre, plateforme), plaques, légendes ; possède ses composants et ses
// sources DEMO DATA.
class Sheet : public juce::Component {
 public:
  Sheet(const juce::String& title, const juce::String& subtitle, int w, int h);
  DemoValue& val(float lo, float hi, float def, float value, const juce::String& unit, int dec, bool bip = false, int steps = 0);
  template <class C, class... A>
  C& add(juce::Rectangle<int> r, A&&... a) {
    auto c = std::make_unique<C>(std::forward<A>(a)...);
    auto& ref = *c;
    addAndMakeVisible(ref);
    ref.setBounds(r);
    comps_.push_back(std::move(c));
    return ref;
  }
  void caption(const juce::String& s, juce::Rectangle<float> r, float size = 11.0f, juce::Colour c = juce::Colour(0xff98a5ae),
               juce::Justification j = juce::Justification::centredLeft, bool bold = false);
  void plate(juce::Rectangle<float> r, const juce::String& title, r2::Domain d, const juce::String& note = {});
  void line(juce::Point<float> a, juce::Point<float> b) { lines_.push_back({a, b}); }
  void paint(juce::Graphics&) override;

 private:
  void paintHeader(juce::Graphics&);
  void paintBody(juce::Graphics&);
  struct Cap {
    juce::String s;
    juce::Rectangle<float> r;
    float size;
    juce::Colour c;
    juce::Justification j;
    bool bold;
  };
  struct Pl {
    juce::Rectangle<float> r;
    juce::String title, note;
    r2::Domain d;
  };
  juce::String title_, subtitle_;
  std::vector<std::unique_ptr<DemoValue>> values_;
  std::vector<std::unique_ptr<juce::Component>> comps_;
  std::vector<Cap> caps_;
  std::vector<Pl> plates_;
  std::vector<std::pair<juce::Point<float>, juce::Point<float>>> lines_;
};

std::unique_ptr<Sheet> buildBoard();     // planche complète R1 (aussi la fenêtre interactive)
std::unique_ptr<Sheet> sheetNexusB1();   // planche NEXUS de B1, code inchangé (non-régression LAB-A01)
std::unique_ptr<Sheet> sheetPlasmaB1();  // planche PLASMA de B1, code inchangé (non-régression LAB-A05)
// Rend les planches R2-B1-R1 dans `dir` (PNG). scale = facteur des planches de revue (2 = équivalent Retina).
// b1Dir : captures B1 publiées de la MÊME plateforme, pour les comparaisons pixel à pixel de non-régression.
// stopBefore : arrêt avant la planche nommée (VIS-01 : rejouer l'historique de rendu d'une référence) ; vide : toutes.
bool renderSheets(const juce::File& dir, float scale, juce::StringArray& log, const juce::File& b1Dir, const juce::String& stopBefore = {});
// Planche R1 historique reconstruite par son code (VIS-01, B2-03) : "01" tableau final, "06a" PLASMA STANDARD / COMPACT,
// "09" groupe des zooms, "11" matériaux.
std::unique_ptr<Sheet> makeR1Sheet(const juce::String& id);
bool writePng(const juce::Image& img, const juce::File& f);

}  // namespace sbd::lab
