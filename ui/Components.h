// SUBDESTRUCKT — UI — Design System SYMBIOTIC INDUSTRIAL (ARCH-UI-04) : composants réutilisables.
//
//   NEXUS (XL / S / Micro) ... commande continue rotative : arc Base, indicateur Effective distinct, valeur + unité,
//                               réglage fin (Maj), saisie numérique, reset explicite, clavier, molette, geste hôte.
//   PLASMA CHANNEL ............ commande continue linéaire : membrane Base, marqueur Effective, mesure réelle optionnelle.
//   SYNAPTIC TRACK ............ sélection discrète (segments ou liste) ; jamais d'interpolation d'énumération.
//   NEURAL CELL ............... interrupteur à états textuels OFF / ARMÉ / ACTIF (armements autonomes) ; ActionCell :
//                               action momentanée distincte d'un interrupteur.
//   OPTICAL DISPLAY / Plate ... verre optique fumé / plaque usinée titrée à accent sémantique.
//   ParamPanel ................ disposition en flux de commandes liées, sans troncature (libellés sur deux lignes).
//
// Règles : une commande liée édite la valeur Base du paramètre hôte (juce::ParameterAttachment : begin/end de geste
// cohérents, une transaction Undo par geste) ; elle n'écrit jamais une valeur Effective ; elle affiche l'Effective
// (télémétrie décimée du moteur) quand la destination est modulée ; elle ne connaît pas l'état DSP. Titre accessible =
// libellé du registre ; HelpText = StableID (inventaire de couverture).
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "DesignTokens.h"
#include "core/Parameters.h"
#include "plugin/PluginProcessor.h"
#include "ui/r2/R2Painters.h"

namespace sbd::ui {

using SelectFn = std::function<void(ParamId)>;
class BoundControl;

// Contexte partagé par toutes les vues de l'éditeur (un par éditeur ; jamais de source de vérité DSP).
struct UiContext {
  SubdestrucktProcessor& proc;
  SelectFn select;
  RandomOptions random;                         // réglages Randomizer de la session (verrous compris)
  std::array<std::uint8_t, kNumParams> modCount{};  // routes globales par destination (rafraîchi à 10 Hz)
  std::array<std::int16_t, kNumParams> learnCc{};   // CC MIDI Learn par paramètre (-1 : aucun)
  std::vector<BoundControl*> registry;          // commandes liées vivantes (télémétrie, couverture)
  std::function<void(ParamId)> requestLearn;    // MIDI Learn depuis le menu contextuel
  ParamId inspected = ParamId::WtX;             // paramètre affiché par l'Inspector
  std::function<void(int space, int tab)> navigate;  // navigation (Ouvrir/Enregistrer, liens d'aide)
  ModMatrixState mod;            // configuration publiée de la matrice (copie rafraîchie à 10 Hz par l'éditeur, fil message)
  std::uint64_t modRevision = 0;  // incrémenté à chaque changement de `mod` (marques de routes des NEXUS)
};

// ---------------------------------------------------------------- utilitaires du registre
Domain3 domainOf(ParamId id);
std::string shortLabel(ParamId id);   // libellé contextuel (préfixes de groupe retirés), jamais tronqué
std::vector<ParamId> range(ParamId first, ParamId last);  // [first, last] inclus, ordre du registre
std::vector<ParamId> filterOffsets(ParamId base, const std::vector<int>& offsets);
// Analyse d'une saisie numérique (unité facultative, virgule décimale, suffixe k) ; false si illisible.
bool parseValueText(ParamId id, const juce::String& text, float& out);

// Libellé sur une à deux lignes, police réduite jusqu'au plancher tok::fontMicro ; renvoie false si le texte ne tient
// pas (défaut ergonomique mesuré par le test UI04-CLIP), sans jamais insérer d'ellipse.
bool layoutLabel(const juce::String& text, float width, int maxLines, float preferredSize, juce::StringArray& lines, float& size);
void drawLabel(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area, int maxLines, float size,
               juce::Colour c, juce::Justification j = juce::Justification::centredTop);

// ---------------------------------------------------------------- interface commune des commandes liées
class BoundControl {
 public:
  explicit BoundControl(UiContext& ctx);
  virtual ~BoundControl();
  virtual ParamId boundId() const = 0;
  virtual juce::Component& asComponent() = 0;
  // Geste utilisateur complet (même chemin que la saisie numérique et le clavier) : une transaction hôte.
  virtual void userSet(float base) = 0;
  virtual float shownBase() const = 0;  // valeur Base actuellement affichée par la commande
  virtual void tick() {}                // télémétrie (cadence de l'éditeur), uniquement si visible
  virtual bool textFits() const { return true; }  // libellés et valeurs extrêmes sans troncature

 protected:
  UiContext& ctx_;
};

// Liaison à un paramètre hôte (attachement générique JUCE + valeur Base exacte du pont).
class ParamLink {
 public:
  ParamLink(SubdestrucktProcessor& p, ParamId id, std::function<void()> onChange);
  float base() const;
  float norm() const { return param_.getValue(); }
  float fromNorm(float n) const { return param_.convertFrom0to1(juce::jlimit(0.0f, 1.0f, n)); }
  float toNorm(float v) const { return param_.convertTo0to1(v); }
  void begin() { att_->beginGesture(); }
  void during(float v) { att_->setValueAsPartOfGesture(v); }
  void end() { att_->endGesture(); }
  void complete(float v) { att_->setValueAsCompleteGesture(v); }
  juce::RangedAudioParameter& param() { return param_; }

 private:
  SubdestrucktProcessor& proc_;
  ParamId id_;
  juce::RangedAudioParameter& param_;
  std::unique_ptr<juce::ParameterAttachment> att_;
};

// ---------------------------------------------------------------- NEXUS / PLASMA CHANNEL
enum class Density { XL, S, Micro };
enum class Form { Rotary, LinearV, LinearH };

class ContinuousControl : public juce::Component, public juce::SettableTooltipClient, public BoundControl, private juce::Timer {
 public:
  ContinuousControl(UiContext& ctx, ParamId id, Form form, Density d);
  ~ContinuousControl() override;
  ParamId boundId() const override { return id_; }
  juce::Component& asComponent() override { return *this; }
  void userSet(float base) override {
    closeWheelGesture();  // PB-01e : aucun geste imbriqué dans un geste molette encore ouvert
    link_.complete(sanitize(id_, base));
  }
  float shownBase() const override { return shown_; }
  juce::Rectangle<float> valueBox() const { return valueArea(); }  // fenêtre de valeur (preuves VIS-01)
  juce::Rectangle<float> knobBox() const { return knobArea(); }    // zone de manipulation (tests NX-*, PLAS-*)
  juce::Rectangle<float> labelBox() const { return labelArea(); }  // sérigraphie (tests PLAS-*)
  juce::Rectangle<float> channelBox() const { return channelArea(); }  // canal : correspondance valeur <-> position (PLASMA)
  juce::Rectangle<float> manipulationBox() const { return manipulationArea(); }  // zone active des gestes (DB2-08)
  // PLASMA R1 (B2-03.2, CE-B2-02) : gabarit choisi par la surface utile ; false si aucun gabarit R1 ne tient réellement
  // (géométrie, libellé ou valeur) : rendu 0.4.1 conservé, contrat d'interaction DB2-08 / DB2-09 appliqué quand même
  bool plasmaR1() const { return plasma_.r1; }
  bool plasmaCompact() const { return plasma_.compact; }
  juce::Rectangle<int> plasmaTemplate() const { return plasma_.tpl; }
  juce::String plasmaBlockReason() const { return plasma_.why; }
  // CE-B2-08 : zone de l'indicateur MIDI Learn dans la sérigraphie (vide si aucun CC appris, en Micro, ou si la place
  // n'est pas libre à côté du libellé : l'indicateur ne recouvre jamais un élément du composant R1)
  juce::Rectangle<float> ccTagBox() const;
  juce::String valueText() const { return juce::String(formatValue(id_, shown_)); }
  float shownNorm() const { return link_.toNorm(shown_); }
  void tick() override;
  bool textFits() const override;
  void setCaption(const juce::String& c);  // libellé contextuel imposé (ex. « FM INDEX »)
  void setMeter(std::function<float()> dbfs) { meter_ = std::move(dbfs); }  // mesure réelle (PLASMA de sortie)
  Density density() const { return density_; }
  Form form() const { return form_; }
  // valeur conservée mais sans effet dans le contexte courant (ex. route absente) : reste éditable, marquée INACTIF
  void setInactive(bool inactive, const juce::String& why = {});
  bool inactive() const { return inactive_; }

  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseDrag(const juce::MouseEvent&) override;
  void mouseUp(const juce::MouseEvent&) override;
  void mouseDoubleClick(const juce::MouseEvent&) override;
  void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
  bool keyPressed(const juce::KeyPress&) override;
  void focusGained(FocusChangeType) override { repaint(); }
  void focusLost(FocusChangeType) override { repaint(); }
  void resized() override;
  std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;
  void beginNumericEntry();     // saisie numérique (Entrée, double-clic sur la valeur, menu)
  void resetToDefault();        // reset explicite (double-clic, Alt-clic, Suppr, menu)

 private:
  friend class NexusAccessibleValue;
  void timerCallback() override;  // fin de geste molette
  void closeWheelGesture();       // ferme un geste molette ouvert (avant tout geste incompatible : PB-01e)
  float nexusLabelPx() const;     // corps de sérigraphie R1 tenant sur deux lignes (famille, puis réduction jusqu'au plancher)
  struct NexusLayout {             // mise en page R1 du NEXUS (B2-03.1), recalculée au redimensionnement et au changement de libellé
    float px = 0.0f;
    int lines = 1, split = -1;
    juce::Rectangle<float> knob;
    juce::Rectangle<int> bounds;
    juce::String caption;
  };
  NexusLayout nexus_;
  void layoutNexus();
  void refreshRouteDepths();      // décalages des routes globales vers ce paramètre (configuration publiée)
  void showMenu();
  void stepNorm(double delta);
  juce::Rectangle<float> knobArea() const;
  juce::Rectangle<float> valueArea() const;
  juce::Rectangle<float> labelArea() const;
  juce::Rectangle<float> channelArea() const;
  juce::Rectangle<float> manipulationArea() const;
  struct PlasmaGeo {
    bool r1 = false, compact = false;
    juce::Rectangle<int> tpl;  // gabarit R1 dans la cellule (STANDARD 64 x 180, COMPACT 48 x 150, horizontal : hauteur 44)
    juce::String why;          // raison du blocage CE-B2-02 (vide si R1)
  };
  PlasmaGeo plasma_;
  void layoutPlasma();
  juce::StringArray plasmaGraduations() const;
  void paintRotary(juce::Graphics&, juce::Rectangle<float>);
  void paintLinear(juce::Graphics&, juce::Rectangle<float>);
  float labelSize() const;
  int labelLines() const;

  ParamId id_;
  Form form_;
  Density density_;
  ParamLink link_;
  juce::String caption_;
  float shown_ = 0.0f;
  float effNorm_ = -1.0f;   // Effective normalisée (-1 : non modulée)
  juce::String baseTooltip_;  // infobulle 0.4.1 (nom, StableID, aide ou raison d'inactivité)
  int learnShown_ = -1;       // CC MIDI Learn reflété dans l'infobulle (CE-B2-08)
  void refreshTooltip();
  float meterDb_ = -120.0f;
  std::function<float()> meter_;
  bool dragging_ = false, wheelGesture_ = false, inactive_ = false;
  float dragStartNorm_ = 0.0f;
  juce::Point<float> dragStart_;
  std::unique_ptr<juce::TextEditor> entry_;
  std::vector<float> routeOffsets_;          // décalages en unités du domaine (profondeur x modScale), signés
  std::uint64_t routeRevision_ = ~std::uint64_t(0);
};

// ---------------------------------------------------------------- SYNAPTIC TRACK
class SynapticTrack : public juce::Component, public juce::SettableTooltipClient, public BoundControl {
 public:
  SynapticTrack(UiContext& ctx, ParamId id);
  ~SynapticTrack() override;
  ParamId boundId() const override { return id_; }
  juce::Component& asComponent() override { return *this; }
  void userSet(float base) override { link_.complete(static_cast<float>(juce::jlimit(0, count_ - 1, static_cast<int>(base + 0.5f)))); }
  float shownBase() const override { return static_cast<float>(index_); }
  bool textFits() const override;
  void setCaption(const juce::String& c) {
    caption_ = c;
    repaint();
  }
  // libellés d'affichage des options (alias de nomenclature ; B2-04), même nombre d'options que le registre ; la valeur
  // transmise à l'hôte reste l'index
  void setOptionLabels(const juce::StringArray& labels) {
    if (labels.size() == count_) labels_ = labels;
    repaint();
  }
  bool segmented() const;
  // SYNAPTIC TRACK R1 (B2-03.4) : même mode que 0.4.1 (segments ou liste), rendu par le peintre partagé si tous les
  // textes tiennent sans troncature dans le gabarit R1 ; sinon rendu 0.4.1 conservé (raison publiée)
  bool r1() const;
  juce::String r1BlockReason() const;
  juce::Rectangle<float> segmentBox(int k) const;  // zone de clic du segment k (géométrie du rendu)
  int count() const { return count_; }
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseMove(const juce::MouseEvent&) override;
  void mouseExit(const juce::MouseEvent&) override;
  bool keyPressed(const juce::KeyPress&) override;
  void focusGained(FocusChangeType) override { repaint(); }
  void focusLost(FocusChangeType) override { repaint(); }
  std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

 private:
  sbd::r2::TrackView r1View() const;
  int hover_ = -1;
  juce::Rectangle<float> bodyArea() const;
  void openList();
  ParamId id_;
  ParamLink link_;
  juce::String caption_;
  juce::StringArray labels_;
  int count_ = 1, index_ = 0;
};

// ---------------------------------------------------------------- NEURAL CELL
// Interrupteur (ToggleButton : intention explicite de l'utilisateur). Pour les 9 armements autonomes : OFF / ARMÉ
// (configuré, exécution suspendue faute d'autorisation) / ACTIF (configuré et autorisé). Un clic de mise en marche d'un
// armement vaut autorisation explicite (ARCH-G4.3, TA-07w) ; automation et restauration ne passent jamais par ici.
class NeuralCell : public juce::ToggleButton, public BoundControl {
 public:
  NeuralCell(UiContext& ctx, ParamId id);  // lié à un paramètre Bool du registre
  NeuralCell(UiContext& ctx, const juce::String& text, juce::Colour accent);  // état hors registre (Drone…)
  ~NeuralCell() override;
  ParamId boundId() const override { return id_; }
  juce::Component& asComponent() override { return *this; }
  void userSet(float base) override { setToggleState(base >= 0.5f, juce::sendNotification); }
  float shownBase() const override { return getToggleState() ? 1.0f : 0.0f; }
  void tick() override;
  bool textFits() const override;
  void setCellState(CellState s);
  CellState cellState() const { return state_; }
  void setCaption(const juce::String& c) {
    caption_ = c;
    repaint();
  }
  void paintButton(juce::Graphics&, bool over, bool down) override;
  std::function<void(bool)> onUserToggle;  // état hors registre
  // NEURAL CELL R1 (B2-03.3) : rendu par le peintre partagé si le libellé tient sur la ligne du gabarit R1, sinon rendu
  // 0.4.1 conservé (raison publiée) ; le comportement est le même dans les deux cas
  bool r1() const;
  juce::String r1BlockReason() const;
  juce::String stateText() const;  // texte d'état affiché (OFF, ON, ARMÉ, ACTIF, REFUS, —)

 private:
  ParamId id_ = ParamId::Count;
  std::unique_ptr<ParamLink> link_;
  juce::String caption_;
  juce::Colour accent_;
  CellState state_ = CellState::Off;
  bool updating_ = false;
};

// Action momentanée (jamais un état persistant) : Panic, Cross Reset, Excitation, captures, Randomize…
class ActionCell : public juce::TextButton {
 public:
  // icônes R1 (LAB-A02) : impulsion (toute action momentanée, par défaut), capture, autorisation
  enum class Icon { None, Pulse, Capture, Authorize };
  ActionCell(const juce::String& text, juce::Colour accent, bool danger = false);
  void paintButton(juce::Graphics&, bool over, bool down) override;
  void setIcon(Icon i) {
    icon_ = i;
    repaint();
  }
  // NEURAL CELL R1 (B2-03.3) : action = touche Action, bascule = Toggle ; PANIC (danger) : B2-03.6
  bool r1() const;
  juce::String r1BlockReason() const;
  bool textFits() const;
  void setRefused(bool r) {
    refused_ = r;
    repaint();
  }

 private:
  juce::Colour accent_;
  bool danger_, refused_ = false;
  Icon icon_ = Icon::Pulse;  // repère R1 du geste momentané (remplace le repère 0.4.1 en bas à gauche)
};

// Onglets (navigation) : jamais une écriture d'état sonore.
class TabStrip : public juce::Component {
 public:
  explicit TabStrip(juce::Colour accent) : accent_(accent) {}
  void setTabs(const juce::StringArray& names);
  void setCurrent(int k);
  int current() const { return current_; }
  int count() const { return names_.size(); }
  juce::String name(int k) const { return names_[k]; }
  std::function<void(int)> onChange;
  void paint(juce::Graphics&) override;
  void mouseDown(const juce::MouseEvent&) override;
  void mouseMove(const juce::MouseEvent&) override;
  void mouseExit(const juce::MouseEvent&) override;
  bool keyPressed(const juce::KeyPress&) override;
  int preferredWidth() const;
  bool textFits() const;
  // SYNAPTIC TRACK R1, onglets (B2-03.4) : si les onglets tiennent à leur largeur R1 (libellé + 24), sinon rendu 0.4.1
  bool r1() const;
  juce::Rectangle<float> tabBox(int k) const { return tabRect(k); }

 private:
  sbd::r2::TrackView r1View() const;
  juce::Rectangle<float> tabRect(int k) const;
  juce::StringArray names_;
  int current_ = 0, hover_ = -1;
  juce::Colour accent_;
};

// ---------------------------------------------------------------- plaques et afficheurs
class Plate : public juce::Component {
 public:
  Plate(juce::String title, juce::Colour accent) : title_(std::move(title)), accent_(accent) {}
  void setTitle2(juce::String t, juce::String note = {}) {
    title_ = std::move(t);
    note_ = std::move(note);
    repaint();
  }
  void setAccent(juce::Colour c) {
    accent_ = c;
    repaint();
  }
  juce::Rectangle<int> content() const { return getLocalBounds().reduced(tok::pad).withTrimmedTop(tok::sectionHead); }
  void paint(juce::Graphics&) override;
  const juce::String& title() const { return title_; }

 private:
  juce::String title_, note_;
  juce::Colour accent_;
};

// Verre optique fumé : données réelles uniquement ; le point de prélèvement est nommé.
class OpticalDisplay : public juce::Component {
 public:
  OpticalDisplay(juce::String title, juce::String tap) : title_(std::move(title)), tap_(std::move(tap)) {}
  void setTap(juce::String t) {
    tap_ = std::move(t);
    repaint();
  }
  juce::Rectangle<int> content() const { return getLocalBounds().reduced(6).withTrimmedTop(16); }
  void paint(juce::Graphics&) override;
  void paintOverChildren(juce::Graphics&) override;

 private:
  juce::String title_, tap_;
};

// ---------------------------------------------------------------- disposition
// Fabrique : commande conforme au type du paramètre (Float/Int -> NEXUS, Choice -> SYNAPTIC TRACK, Bool -> NEURAL CELL).
std::unique_ptr<juce::Component> makeControl(UiContext& ctx, ParamId id, Density d = Density::S);
BoundControl* asBound(juce::Component* c);
juce::Rectangle<int> preferredCell(ParamId id, Density d);

// Flux de commandes liées (lignes de hauteur variable, sans chevauchement ni troncature), éventuellement en groupes
// titrés. Hauteur préférée pour une largeur donnée (défilement local par l'appelant).
class ParamPanel : public juce::Component {
 public:
  ParamPanel(UiContext& ctx, Density d = Density::S) : ctx_(ctx), density_(d) {}
  void setParams(const std::vector<ParamId>& ids);                 // un groupe sans titre
  void setGroups(const std::vector<std::pair<juce::String, std::vector<ParamId>>>& groups);
  int preferredHeight(int width) const;
  void resized() override;
  void paint(juce::Graphics&) override;
  const std::vector<std::unique_ptr<juce::Component>>& controls() const { return controls_; }
  void setAccent(juce::Colour c) { accent_ = c; }

 private:
  int layout(int width, bool apply);
  UiContext& ctx_;
  Density density_;
  juce::Colour accent_ = tok::textDim;
  std::vector<std::pair<juce::String, std::vector<ParamId>>> groups_;
  std::vector<std::unique_ptr<juce::Component>> controls_;
  std::vector<std::pair<juce::String, juce::Rectangle<int>>> heads_;
};

// Panneau défilant (scroll local) contenant un ParamPanel.
class ScrollPanel : public juce::Component {
 public:
  ScrollPanel(UiContext& ctx, Density d = Density::S) : panel(ctx, d) {
    vp_.setViewedComponent(&panel, false);
    vp_.setScrollBarsShown(true, false);
    vp_.setScrollBarThickness(8);
    addAndMakeVisible(vp_);
  }
  void resized() override {
    vp_.setBounds(getLocalBounds());
    const int w = std::max(60, vp_.getWidth() - 10);
    panel.setSize(w, std::max(vp_.getHeight(), panel.preferredHeight(w)));
  }
  ParamPanel panel;
  juce::Viewport& viewport() { return vp_; }

 private:
  juce::Viewport vp_;
};

}  // namespace sbd::ui
