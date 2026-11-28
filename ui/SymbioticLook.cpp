#include "SymbioticLook.h"

#include "ui/r2/R2Style.h"

namespace sbd::ui {

SymbioticLook::SymbioticLook() {
  setColour(juce::ResizableWindow::backgroundColourId, tok::chassis);
  setColour(juce::Label::textColourId, tok::text);
  setColour(juce::ComboBox::backgroundColourId, tok::carbon);
  setColour(juce::ComboBox::textColourId, tok::text);
  setColour(juce::ComboBox::outlineColourId, tok::glassEdge);
  setColour(juce::ComboBox::arrowColourId, tok::textDim);
  setColour(juce::PopupMenu::backgroundColourId, tok::panel);
  setColour(juce::PopupMenu::textColourId, tok::text);
  setColour(juce::PopupMenu::headerTextColourId, tok::textDim);
  setColour(juce::PopupMenu::highlightedBackgroundColourId, tok::titanium);
  setColour(juce::PopupMenu::highlightedTextColourId, tok::text);
  setColour(juce::TextButton::buttonColourId, tok::titanium);
  setColour(juce::TextButton::buttonOnColourId, tok::titanium.brighter(0.15f));
  setColour(juce::TextButton::textColourOffId, tok::text);
  setColour(juce::TextButton::textColourOnId, tok::text);
  setColour(juce::ToggleButton::textColourId, tok::text);
  setColour(juce::ToggleButton::tickColourId, tok::wt);
  setColour(juce::ToggleButton::tickDisabledColourId, tok::textMute);
  setColour(juce::TextEditor::backgroundColourId, tok::carbon);
  setColour(juce::TextEditor::textColourId, tok::text);
  setColour(juce::TextEditor::outlineColourId, tok::glassEdge);
  setColour(juce::TextEditor::focusedOutlineColourId, tok::focus);
  setColour(juce::TextEditor::highlightColourId, tok::mod.withAlpha(0.35f));
  setColour(juce::CaretComponent::caretColourId, tok::text);
  setColour(juce::ListBox::backgroundColourId, tok::carbon);
  setColour(juce::ListBox::outlineColourId, tok::glassEdge);
  setColour(juce::ScrollBar::thumbColourId, tok::titanium);
  setColour(juce::Slider::textBoxTextColourId, tok::text);
  setColour(juce::Slider::textBoxBackgroundColourId, tok::carbon);
  setColour(juce::Slider::textBoxOutlineColourId, tok::glassEdge);
  setColour(juce::TooltipWindow::backgroundColourId, tok::carbon);
  setColour(juce::TooltipWindow::textColourId, tok::text);
  setColour(juce::TooltipWindow::outlineColourId, tok::glassEdge);
  setColour(juce::AlertWindow::backgroundColourId, tok::panel);
  setColour(juce::AlertWindow::textColourId, tok::text);
  setColour(juce::FileBrowserComponent::currentPathBoxBackgroundColourId, tok::carbon);
  setColour(juce::FileBrowserComponent::filenameBoxBackgroundColourId, tok::carbon);
  setColour(juce::DirectoryContentsDisplayComponent::highlightColourId, tok::titanium);
  setColour(juce::DirectoryContentsDisplayComponent::textColourId, tok::text);
}

// ---------------------------------------------------------------- habillage R1 (DB2-06) : matières et primitives R1 validées
// (ui/r2/R2Style.h) appliquées aux widgets JUCE génériques ; aucune conception nouvelle, widgets et contrats conservés.
namespace {
namespace d = sbd::r2::draw;
namespace t2 = sbd::r2::tok2;
}  // namespace

void SymbioticLook::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) {
  auto r = b.getLocalBounds().toFloat().reduced(2.0f, 2.5f);
  const bool enabled = b.isEnabled();
  if (b.getToggleState()) {  // bouton basculé (choix d'une vue, d'un mode) : logement encastré, fond carbone (sélection)
    d::machinedGroove(g, r, 3.0f);
    g.setColour(tok::carbon);
    g.fillRoundedRectangle(r.reduced(2.0f), 2.0f);
    const auto bar = juce::Rectangle<float>(r.getX() + 6.0f, r.getBottom() - 4.0f, r.getWidth() - 12.0f, 1.6f);
    g.setColour(tok::textDim);
    g.fillRoundedRectangle(bar, 0.8f);
  } else if (!enabled) {  // indisponible : touche à plat, matière éteinte
    d::titaniumKey(g, r, t2::relief1, false, false, false);
  } else {  // action : touche en titane usiné, course physique à l'appui (NEURAL CELL, LAB-A02)
    d::titaniumKey(g, r, t2::titaniumFace, true, down, over);
  }
  if (b.hasKeyboardFocus(false)) d::focusRing(g, r.expanded(2.0f), tok::radiusSmall + 2.0f);
}

void SymbioticLook::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool down) {
  g.setFont(getTextButtonFont(b, b.getHeight()));
  g.setColour(b.isEnabled() ? tok::text : tok::textMute);
  const int travel = (down && !b.getToggleState()) ? 1 : 0;
  g.drawFittedText(b.getButtonText(), b.getLocalBounds().reduced(6, 1).translated(0, travel), juce::Justification::centred, 2, 1.0f);
}

void SymbioticLook::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool over, bool) {
  auto r = b.getLocalBounds().toFloat().reduced(1.0f);
  auto box = r.removeFromLeft(std::min(18.0f, r.getHeight()));
  // témoin encastré (NEURAL CELL R1) : éteint ou plein, jamais la couleur seule (le texte du bouton reste)
  d::recessedLed(g, box.getCentre(), 4.0f, b.getToggleState() ? 2 : 0, b.isEnabled() ? tok::mod : tok::textMute);
  if (over && b.isEnabled()) {
    g.setColour(tok::textDim.withAlpha(0.6f));
    g.drawEllipse(box.withSizeKeepingCentre(15.0f, 15.0f), 1.0f);
  }
  g.setColour(b.isEnabled() ? tok::text : tok::textMute);
  g.setFont(uiFont(tok::fontLabel));
  g.drawFittedText(b.getButtonText(), r.withTrimmedLeft(6.0f).toNearestInt(), juce::Justification::centredLeft, 2, 1.0f);
  if (b.hasKeyboardFocus(false)) d::focusRing(g, b.getLocalBounds().toFloat().reduced(0.5f), tok::radiusSmall);
}

void SymbioticLook::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& c) {
  // liste SYNAPTIC Machined Track (LAB-A03) : champ encastré, marqueur neutre, cran usiné avant le chevron
  auto r = juce::Rectangle<float>(0, 0, static_cast<float>(w), static_cast<float>(h)).reduced(1.0f);
  const bool enabled = c.isEnabled();
  d::machinedGroove(g, r, 3.0f);
  g.setColour(enabled ? tok::textDim : tok::panelEdge);
  g.fillRoundedRectangle(r.getX() + 3.0f, r.getY() + 5.0f, 2.0f, std::max(2.0f, r.getHeight() - 10.0f), 1.0f);
  const float cx = r.getRight() - 11.0f, cy = r.getCentreY();
  g.setColour(juce::Colours::black.withAlpha(0.65f));
  g.drawLine(cx - 9.0f, r.getY() + 5.0f, cx - 9.0f, r.getBottom() - 5.0f, 1.0f);
  g.setColour(t2::bevelLight.withAlpha(0.22f));
  g.drawLine(cx - 8.0f, r.getY() + 5.0f, cx - 8.0f, r.getBottom() - 5.0f, 1.0f);
  juce::Path tri;
  tri.addTriangle(cx - 3.5f, cy - 1.8f, cx + 3.5f, cy - 1.8f, cx, cy + 2.6f);
  g.setColour(enabled ? tok::textDim : tok::textMute);
  g.fillPath(tri);
  if (!enabled) d::hatch(g, r.reduced(2.0f), tok::panelEdge.withAlpha(0.5f), 6.0f);
  if (c.hasKeyboardFocus(true)) d::focusRing(g, r.expanded(1.5f), tok::radiusSmall + 1.5f);
}

void SymbioticLook::positionComboBoxText(juce::ComboBox& c, juce::Label& l) {
  l.setBounds(10, 1, std::max(0, c.getWidth() - 34), std::max(0, c.getHeight() - 2));
  l.setFont(getComboBoxFont(c));
}

void SymbioticLook::drawPopupMenuBackground(juce::Graphics& g, int w, int h) {
  g.fillAll(t2::relief1);
  g.setColour(tok::glassEdge);
  g.drawRect(0, 0, w, h, 1);
}

void SymbioticLook::drawScrollbar(juce::Graphics& g, juce::ScrollBar&, int x, int y, int w, int h, bool vertical, int ts, int tsz, bool over, bool down) {
  // gorge usinée et curseur en titane (matières R1)
  g.setColour(t2::groove);
  g.fillRect(x, y, w, h);
  auto th = (vertical ? juce::Rectangle<int>(x + 2, ts, w - 4, tsz) : juce::Rectangle<int>(ts, y + 2, tsz, h - 4)).toFloat();
  juce::ColourGradient grad(t2::relief3.brighter(down ? 0.0f : (over ? 0.25f : 0.1f)), th.getX(), th.getY(), t2::relief2.darker(0.2f),
                            vertical ? th.getRight() : th.getX(), vertical ? th.getY() : th.getBottom(), false);
  g.setGradientFill(grad);
  g.fillRoundedRectangle(th, 3.0f);
}

void SymbioticLook::fillTextEditorBackground(juce::Graphics& g, int w, int h, juce::TextEditor&) {
  g.setColour(tok::carbon);  // fenêtre de valeur R1 : carbone
  g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h), 2.5f);
}

void SymbioticLook::drawTextEditorOutline(juce::Graphics& g, int w, int h, juce::TextEditor& e) {
  const bool focus = e.hasKeyboardFocus(true);
  g.setColour(focus ? tok::focus : tok::glassEdge);
  g.drawRoundedRectangle(0.5f, 0.5f, static_cast<float>(w) - 1.0f, static_cast<float>(h) - 1.0f, 2.5f, focus ? 1.5f : 0.8f);
}

void SymbioticLook::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle st, juce::Slider& s) {
  // canal PLASMA H (LAB-A05) : gorge usinée, canal lumineux de la valeur, curseur en titane avec index
  const bool vert = st == juce::Slider::LinearVertical;
  const auto r = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));
  const auto slot = vert ? r.withSizeKeepingCentre(8.0f, std::max(0.0f, r.getHeight() - 8.0f)) : r.withSizeKeepingCentre(std::max(0.0f, r.getWidth() - 8.0f), 8.0f);
  g.setColour(t2::groove);
  g.fillRoundedRectangle(slot.expanded(2.0f), 3.0f);
  g.setColour(t2::bevelLight.withAlpha(0.3f));
  g.drawRoundedRectangle(slot.expanded(2.0f).translated(0.0f, 0.6f), 3.0f, 0.7f);
  const auto track = s.findColour(juce::Slider::trackColourId, true);
  const auto accent = track == juce::Colour() ? tok::mod : track;
  if (s.isEnabled()) {
    auto fill = vert ? slot.withTop(std::min(pos, slot.getBottom())) : slot.withRight(std::max(pos, slot.getX()));
    g.setColour(accent.withAlpha(t2::glowMax * 0.45f));
    g.fillRoundedRectangle(fill.expanded(2.5f), 3.0f);
    juce::ColourGradient plasma(accent.brighter(0.35f), fill.getX(), fill.getY(), accent.darker(0.25f), vert ? fill.getRight() : fill.getX(),
                                vert ? fill.getY() : fill.getBottom(), false);
    g.setGradientFill(plasma);
    g.fillRoundedRectangle(fill, 2.0f);
  }
  const auto c = vert ? juce::Point<float>(slot.getCentreX(), pos) : juce::Point<float>(pos, slot.getCentreY());
  const auto cur = vert ? juce::Rectangle<float>(0.0f, 0.0f, 24.0f, 10.0f).withCentre(c) : juce::Rectangle<float>(0.0f, 0.0f, 10.0f, 22.0f).withCentre(c);
  d::key(g, cur, t2::relief3, true, s.isMouseButtonDown(), s.isMouseOverOrDragging());
  g.setColour(s.isEnabled() ? tok::text : tok::textMute);
  if (vert) g.drawLine(cur.getX() + 2.0f, c.y, cur.getRight() - 2.0f, c.y, 1.4f);
  else g.drawLine(c.x, cur.getY() + 2.0f, c.x, cur.getBottom() - 2.0f, 1.4f);
  if (s.hasKeyboardFocus(false)) d::focusRing(g, cur.expanded(2.0f), 3.0f);
}

juce::Rectangle<int> SymbioticLook::getTooltipBounds(const juce::String& t, juce::Point<int> p, juce::Rectangle<int> area) {
  juce::TextLayout tl;
  juce::AttributedString s;
  s.append(t, uiFont(tok::fontLabel), tok::text);
  tl.createLayoutWithBalancedLineLengths(s, 360.0f);
  const int w = static_cast<int>(tl.getWidth()) + 16, h = static_cast<int>(tl.getHeight()) + 10;
  return juce::Rectangle<int>(p.x > area.getCentreX() ? p.x - (w + 12) : p.x + 24, p.y > area.getCentreY() ? p.y - (h + 6) : p.y + 6, w, h)
      .constrainedWithin(area);
}

void SymbioticLook::drawTooltip(juce::Graphics& g, const juce::String& t, int w, int h) {
  g.setColour(tok::carbon);
  g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h), tok::radiusSmall);
  g.setColour(tok::glassEdge);
  g.drawRoundedRectangle(0.5f, 0.5f, static_cast<float>(w) - 1.0f, static_cast<float>(h) - 1.0f, tok::radiusSmall, 1.0f);
  juce::AttributedString s;
  s.append(t, uiFont(tok::fontLabel), tok::text);
  juce::TextLayout tl;
  tl.createLayoutWithBalancedLineLengths(s, static_cast<float>(w) - 16.0f);
  tl.draw(g, juce::Rectangle<float>(8.0f, 5.0f, static_cast<float>(w) - 16.0f, static_cast<float>(h) - 10.0f));
}

void SymbioticLook::drawCornerResizer(juce::Graphics& g, int w, int h, bool over, bool) {
  g.setColour(over ? tok::textDim : tok::titanium);
  for (int k = 1; k <= 3; ++k)
    g.drawLine(static_cast<float>(w - k * 4), static_cast<float>(h - 1), static_cast<float>(w - 1), static_cast<float>(h - k * 4), 1.2f);
}

}  // namespace sbd::ui
