// SUBDESTRUCKT — UI — thème « Industrial Research Instrument » (palette candidate V1-UI-UX-DESIGN §2) et composants
// partagés : un seul composant de contrôle par paramètre du registre (aucune duplication d'état, P0-C23/C24).
// Les contrôles éditent la valeur Base (attachements JUCE -> paramètre hôte) ; l'anneau extérieur affiche la valeur
// Effective quand la destination est modulée (télémétrie décimée, jamais écrite dans l'automation).
// Graphisme provisoire : la validation artistique finale reste à faire (wireframes -> tâches -> prototype).
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include <functional>
#include <memory>
#include <vector>

#include "core/Parameters.h"
#include "plugin/PluginProcessor.h"

namespace sbd::ui {

namespace col {
const juce::Colour chassis(0xff121617), panel(0xff242a2d), text(0xffe1e5e6), dim(0xff8f999c), wt(0xff79c4ac), fm(0xffd98439),
    cross(0xff7899cb), display(0xff080e0c), control(0xff363d40), alert(0xffc65d54);
}
juce::Font mono(float h, bool bold = false);
juce::Font sans(float h, bool bold = false);
juce::Colour accentFor(ParamId id);  // couleur sémantique WT / FM / CROSS-mod / traitements

class Look : public juce::LookAndFeel_V4 {
 public:
  Look();
  void drawRotarySlider(juce::Graphics&, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider&) override;
  void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
  juce::Font getTextButtonFont(juce::TextButton&, int h) override { return sans(std::min(13.0f, h * 0.55f), true); }
  juce::Font getComboBoxFont(juce::ComboBox&) override { return sans(12.0f); }
  juce::Font getLabelFont(juce::Label&) override { return sans(12.0f); }
};

using SelectFn = std::function<void(ParamId)>;

// Contrôle d'un paramètre, selon son type : bouton rotatif (Float/Int), liste (Choice), interrupteur (Bool).
class ParamControl : public juce::Component {
 public:
  ParamControl(SubdestrucktProcessor& p, ParamId id, SelectFn onSelect, bool large = false);
  ~ParamControl() override;
  ParamId id() const noexcept { return id_; }
  void refreshTelemetry();  // anneau Effective (timer de l'éditeur)
  void resized() override;
  void paintOverChildren(juce::Graphics&) override;
  void setCaption(const juce::String& c);
  bool keyPressed(const juce::KeyPress&) override;

 private:
  struct Clicker : juce::MouseListener {
    ParamControl* owner = nullptr;
    void mouseDown(const juce::MouseEvent&) override { owner->select(); }
  } clicker_;
  void select();
  SubdestrucktProcessor& proc_;
  ParamId id_;
  SelectFn onSelect_;
  bool large_;
  juce::Label caption_;
  std::unique_ptr<juce::Slider> slider_;
  std::unique_ptr<juce::ComboBox> combo_;
  std::unique_ptr<juce::ToggleButton> toggle_;
  std::unique_ptr<juce::SliderParameterAttachment> sAtt_;
  std::unique_ptr<juce::ComboBoxParameterAttachment> cAtt_;
  std::unique_ptr<juce::ButtonParameterAttachment> bAtt_;
  float effNorm_ = -1.0f;  // position normalisée de l'Effective (-1 : non modulé)
};

// Grille auto-disposée de contrôles (colonnes de largeur fixe).
class ParamGrid : public juce::Component {
 public:
  ParamGrid(SubdestrucktProcessor& p, SelectFn onSelect) : proc_(p), onSelect_(std::move(onSelect)) {}
  void setParams(const std::vector<ParamId>& ids);
  void setCellSize(int w, int h) {
    cw_ = w;
    ch_ = h;
    resized();
  }
  int preferredHeight(int width) const;
  void resized() override;
  void refreshTelemetry();
  const std::vector<std::unique_ptr<ParamControl>>& controls() const { return controls_; }

 private:
  SubdestrucktProcessor& proc_;
  SelectFn onSelect_;
  std::vector<std::unique_ptr<ParamControl>> controls_;
  int cw_ = 78, ch_ = 88;
};

// Cadre titré à couleur sémantique.
class Section : public juce::Component {
 public:
  Section(juce::String title, juce::Colour accent) : title_(std::move(title)), accent_(accent) {}
  void paint(juce::Graphics&) override;
  juce::Rectangle<int> content() const { return getLocalBounds().reduced(6).withTrimmedTop(20); }
  void setTitle2(juce::String t) {
    title_ = std::move(t);
    repaint();
  }

 private:
  juce::String title_;
  juce::Colour accent_;
};

// Section contenant une grille, défilante si le contenu dépasse (scroll local, UI §6).
class GridSection : public Section {
 public:
  GridSection(SubdestrucktProcessor& p, SelectFn f, juce::String title, juce::Colour accent)
      : Section(std::move(title), accent), grid(p, std::move(f)) {
    vp_.setViewedComponent(&grid, false);
    vp_.setScrollBarsShown(true, false);
    vp_.setScrollBarThickness(8);
    addAndMakeVisible(vp_);
  }
  void resized() override {
    vp_.setBounds(content());
    const int w = vp_.getWidth() - 10;
    grid.setSize(w, std::max(vp_.getHeight(), grid.preferredHeight(w)));
  }
  ParamGrid grid;

 private:
  juce::Viewport vp_;
};

std::vector<ParamId> range(ParamId first, ParamId last);  // [first, last] inclus, ordre du registre
std::vector<ParamId> filterOffsets(ParamId base, const std::vector<int>& offsets);

}  // namespace sbd::ui
