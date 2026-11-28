// SUBDESTRUCKT — ARCH-UI-04-R2-B2 — Design System R1 (gelé, AR-15) : tokens dérivés, primitives de dessin, source de valeur
// et garde de geste, partagés par le plugin et le Component Lab (TB2-02). Extrait sans modification du laboratoire R1.
// Palette de référence : ui/DesignTokens.h (0.4.1, lue sans modification). Spécification :
// docs/ARCH-UI-04-R2/R2-DESIGN-SYSTEM-SPEC.md et docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/DESIGN-TOKENS-R2.md.
// Ces composants ne dépendent ni du registre ni du processeur : une source de valeur abstraite (ValueSource) les
// relie aux données de démonstration dans le laboratoire, et à ParamLink lors de l'intégration (R2-B2).
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <optional>
#include <vector>

#include "ui/DesignTokens.h"

namespace sbd::r2 {

namespace t = sbd::ui::tok;

// ---------------------------------------------------------------- tokens R2 (dérivés ; DESIGN-TOKENS-R2.md)
namespace tok2 {
// niveaux de relief (matière mécanique) : châssis < plaque < corps < surface d'action
inline const juce::Colour relief0{0xff161a1d};  // = chassis
inline const juce::Colour relief1{0xff22282d};  // plaque usinée
inline const juce::Colour relief2{0xff2c343a};  // corps de commande (titane sombre)
inline const juce::Colour relief3{0xff3a434b};  // surface d'action (titane clair)
inline const juce::Colour bevelLight{0xff56616a};  // arête éclairée
inline const juce::Colour bevelDark{0xff07090b};   // arête d'ombre
inline const juce::Colour groove{0xff0b0f12};      // gorge / rainure
inline const juce::Colour effective = t::mod;      // anneau et marqueur Effective
inline const juce::Colour measure = t::text;       // valeurs mesurées
inline const juce::Colour inactive = t::textMute;  // conservé sans effet
inline const juce::Colour grid{0xff1d2a33};        // graticule des afficheurs
inline const juce::Colour gridStrong{0xff2b3d4a};
constexpr float glowMax = 0.35f;                    // opacité maximale d'une lueur
// typographie R2 (px logiques)
constexpr float fontMicro = 10.0f;
constexpr float fontLabel = 11.0f;
constexpr float fontCritical = 12.0f;  // repères critiques : >= 9 px écran à 75 % (OD-06)
constexpr float fontTitle = 12.5f;
constexpr float fontValueXL = 14.0f;
// ---- R2-B1-R1 (LAB-A02 à LAB-A04) : dérivés supplémentaires, palette de référence inchangée
inline const juce::Colour hudGlassDeep{0xff070b0e};    // fond profond du verre HUD (bords)
inline const juce::Colour hudGlassCore{0xff0e161b};    // cœur du verre HUD (centre)
inline const juce::Colour hudRail{0xff22313b};         // rails et graduations d'encadrement
inline const juce::Colour titaniumFace = t::titanium;  // face de touche d'action (titane noir #343C43)
}  // namespace tok2

juce::Font labelFont(float h, bool bold = false);  // police d'interface 0.4.1 (sans-serif système)
juce::Font valueFont(float h, bool bold = false);  // chiffres tabulaires (monospace système)

enum class Domain { Wt, Fm, Cross, Mod, Neutral, Alert };
juce::Colour domainColour(Domain d);
const char* domainName(Domain d);

// État visuel imposé (planches de captures) ; None = état réel issu de la souris et du clavier.
enum class Forced { None, Normal, Hover, Focus, Pressed, Disabled };

// ---------------------------------------------------------------- source de valeur (modèle)
// Valeurs normalisées 0..1. Contrat de geste identique à ParamLink (0.4.1) : begin / during / end, ou complete.
class ValueSource {
 public:
  virtual ~ValueSource() = default;
  virtual float norm() const = 0;
  virtual float defaultNorm() const = 0;
  virtual std::optional<float> effectiveNorm() const = 0;  // présent seulement si la destination est modulée
  virtual juce::String text(float norm) const = 0;         // valeur formatée avec unité
  virtual bool bipolar() const = 0;
  virtual int steps() const { return 0; }  // 0 = continu
  virtual void begin() = 0;
  virtual void during(float norm) = 0;
  virtual void end() = 0;
  virtual void complete(float norm) = 0;
  virtual std::vector<float> routeDepths() const { return {}; }  // profondeurs signées des routes vers ce paramètre
  std::function<void()> onChange;
};

// Garde de geste : ferme le geste ouvert à la destruction, au masquage et au détachement (traitement R2 de PB-01/PB-02
// dans les composants R2 du laboratoire uniquement ; le code de production n'est pas modifié en B1).
class GestureGuard {
 public:
  explicit GestureGuard(ValueSource* s = nullptr) : src_(s) {}
  ~GestureGuard() { close(); }
  void setSource(ValueSource* s) {
    close();
    src_ = s;
  }
  bool open() {
    if (src_ == nullptr || open_) return false;
    src_->begin();
    open_ = true;
    return true;
  }
  void close() {
    if (src_ != nullptr && open_) src_->end();
    open_ = false;
  }
  bool isOpen() const { return open_; }

 private:
  ValueSource* src_ = nullptr;
  bool open_ = false;
};

// ---------------------------------------------------------------- primitives de dessin
namespace draw {
// corps usiné circulaire : dégradé radial, arête biseautée ; moletage optionnel (nombre de stries)
void machinedKnob(juce::Graphics&, juce::Point<float> c, float r, int knurls, bool hover, bool pressed, bool disabled);
// arc de piste et de valeur (angles en radians, convention JUCE : 0 = midi, sens horaire)
void arc(juce::Graphics&, juce::Point<float> c, float r, float a0, float a1, float width, juce::Colour col);
void glowArc(juce::Graphics&, juce::Point<float> c, float r, float a0, float a1, float width, juce::Colour col);
// graduations radiales
void ticks(juce::Graphics&, juce::Point<float> c, float r0, float r1, float a0, float a1, int n, juce::Colour col, float w);
// touche mécanique (rectangle à arêtes usinées) : convexe (action) ou plate (bascule)
void key(juce::Graphics&, juce::Rectangle<float> r, juce::Colour face, bool convex, bool pressed, bool hover);
// ---- primitives R1 (n'altèrent pas les primitives partagées par NEXUS et PLASMA)
// touche en titane noir usiné : alvéole dans la plaque, chanfrein, face, course physique (appui = face enfoncée)
void titaniumKey(juce::Graphics&, juce::Rectangle<float> r, juce::Colour face, bool convex, bool pressed, bool hover);
// témoin encastré profond : 0 éteint, 1 anneau, 2 plein
void recessedLed(juce::Graphics&, juce::Point<float> c, float r, int mode, juce::Colour col);
// rail mécanique encastré (gorge) et sélecteur en titane (SYNAPTIC Machined Track)
void machinedGroove(juce::Graphics&, juce::Rectangle<float> r, float radius);
void trackSelector(juce::Graphics&, juce::Rectangle<float> r, juce::Colour marker, bool enabled);
// verre optique cybernétique profond, monture usinée, repères d'angle et rails gradués (Cybernetic HUD)
void hudFrame(juce::Graphics&, juce::Rectangle<float> r, juce::Colour accent, bool primary);
// pastille de nature de donnée : BASE (carré plein), EFF (losange), MEAS (crochets), INDICATIVE, STATE ; renvoie la largeur
float hudChip(juce::Graphics&, float x, float y, const juce::String& text, juce::Colour c, int shape);
juce::Font hudTitleFont(float h);  // capitales techniques espacées
// motif hachuré (état INACTIVE)
void hatch(juce::Graphics&, juce::Rectangle<float> r, juce::Colour col, float spacing);
// anneau de focus clavier (non chromatique)
void focusRing(juce::Graphics&, juce::Rectangle<float> r, float radius);
// texte ajusté : une ligne, sinon deux lignes coupées à l'espace le plus équilibré (jamais sous le corps indiqué)
void fittedText(juce::Graphics&, const juce::String& s, juce::Rectangle<float> r, const juce::Font& f, juce::Colour col,
                juce::Justification j, bool allowTwoLines);
// mise en page de fittedText calculée à part : -1 = une ligne ; k >= 0 = deux lignes coupées à l'espace d'indice k
int fittedSplit(const juce::Font& f, const juce::String& s, float width, bool allowTwoLines);
// fittedText avec une mise en page déjà calculée (fittedSplit) : mêmes appels de dessin, sans mesure du texte
void fittedTextSplit(juce::Graphics&, const juce::String& s, juce::Rectangle<float> r, const juce::Font& f, juce::Colour col,
                     juce::Justification j, int split);
float textWidth(const juce::Font& f, const juce::String& s);
// le texte tient-il dans la largeur, selon la règle de fittedText (une ligne, sinon deux lignes coupées à l'espace équilibré) ?
bool fitsText(const juce::Font& f, const juce::String& s, float width, bool allowTwoLines);
}  // namespace draw

}  // namespace sbd::r2
