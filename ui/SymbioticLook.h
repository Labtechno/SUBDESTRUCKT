// SUBDESTRUCKT — UI — LookAndFeel SYMBIOTIC INDUSTRIAL (ARCH-UI-04 ; habillage R1 en ARCH-UI-04-R2-B2, DB2-06) : tous les éléments
// JUCE génériques résiduels
// (menus, listes, champs de saisie, barres de défilement, info-bulles, boutons secondaires) sont redessinés avec les
// design tokens. Aucune commande musicale principale n'utilise un widget JUCE générique (composants : Components.h).
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

#include "DesignTokens.h"

namespace sbd::ui {

class SymbioticLook : public juce::LookAndFeel_V4 {
 public:
  SymbioticLook();
  void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
  void drawButtonText(juce::Graphics&, juce::TextButton&, bool over, bool down) override;
  void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;
  void drawComboBox(juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
  void positionComboBoxText(juce::ComboBox&, juce::Label&) override;
  juce::Font getComboBoxFont(juce::ComboBox&) override { return uiFont(tok::fontLabel); }
  juce::Font getTextButtonFont(juce::TextButton&, int h) override { return uiFont(std::min(tok::fontTitle, h * 0.5f), true); }
  juce::Font getLabelFont(juce::Label&) override { return uiFont(tok::fontLabel); }
  juce::Font getPopupMenuFont() override { return uiFont(tok::fontTitle); }
  void drawPopupMenuBackground(juce::Graphics&, int w, int h) override;
  void drawScrollbar(juce::Graphics&, juce::ScrollBar&, int x, int y, int w, int h, bool vertical, int thumbStart, int thumbSize,
                     bool over, bool down) override;
  void fillTextEditorBackground(juce::Graphics&, int w, int h, juce::TextEditor&) override;
  void drawTextEditorOutline(juce::Graphics&, int w, int h, juce::TextEditor&) override;
  void drawLinearSlider(juce::Graphics&, int x, int y, int w, int h, float pos, float minPos, float maxPos, juce::Slider::SliderStyle,
                        juce::Slider&) override;
  juce::Rectangle<int> getTooltipBounds(const juce::String&, juce::Point<int>, juce::Rectangle<int>) override;
  void drawTooltip(juce::Graphics&, const juce::String&, int w, int h) override;
  void drawCornerResizer(juce::Graphics&, int w, int h, bool over, bool down) override;
};

}  // namespace sbd::ui
