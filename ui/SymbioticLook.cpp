#include "SymbioticLook.h"

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

void SymbioticLook::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) {
  auto r = b.getLocalBounds().toFloat().reduced(1.0f);
  juce::ColourGradient grad(tok::titanium.brighter(down ? 0.0f : 0.1f), r.getX(), r.getY(), tok::titanium.darker(0.4f), r.getX(), r.getBottom(), false);
  g.setGradientFill(grad);
  g.fillRoundedRectangle(r, tok::radiusSmall);
  const bool on = b.getToggleState();
  g.setColour(b.hasKeyboardFocus(false) ? tok::focus : (on ? tok::text.withAlpha(0.7f) : (over ? tok::titanium.brighter(0.35f) : tok::panelEdge.brighter(0.2f))));
  g.drawRoundedRectangle(r.reduced(0.5f), tok::radiusSmall, on || b.hasKeyboardFocus(false) ? 1.4f : 1.0f);
}

void SymbioticLook::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) {
  g.setFont(getTextButtonFont(b, b.getHeight()));
  g.setColour(b.isEnabled() ? tok::text : tok::textMute);
  g.drawFittedText(b.getButtonText(), b.getLocalBounds().reduced(4, 1), juce::Justification::centred, 2, 1.0f);
}

void SymbioticLook::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool over, bool) {
  auto r = b.getLocalBounds().toFloat().reduced(1.0f);
  auto box = r.removeFromLeft(std::min(18.0f, r.getHeight())).withSizeKeepingCentre(14.0f, 14.0f);
  g.setColour(tok::carbon);
  g.fillRoundedRectangle(box, 2.0f);
  g.setColour(over ? tok::titanium.brighter(0.4f) : tok::glassEdge);
  g.drawRoundedRectangle(box, 2.0f, 1.0f);
  if (b.getToggleState()) {
    g.setColour(tok::text);
    g.fillRoundedRectangle(box.reduced(3.5f), 1.0f);
  }
  g.setColour(b.isEnabled() ? tok::text : tok::textMute);
  g.setFont(uiFont(tok::fontLabel));
  g.drawFittedText(b.getButtonText(), r.withTrimmedLeft(6.0f).toNearestInt(), juce::Justification::centredLeft, 2, 1.0f);
}

void SymbioticLook::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& c) {
  auto r = juce::Rectangle<float>(0, 0, static_cast<float>(w), static_cast<float>(h)).reduced(0.5f);
  g.setColour(tok::carbon);
  g.fillRoundedRectangle(r, tok::radiusSmall);
  g.setColour(c.hasKeyboardFocus(true) ? tok::focus : tok::glassEdge);
  g.drawRoundedRectangle(r, tok::radiusSmall, 1.0f);
  juce::Path tri;
  const float cx = static_cast<float>(w) - 10.0f, cy = static_cast<float>(h) * 0.5f;
  tri.addTriangle(cx - 4.0f, cy - 2.0f, cx + 4.0f, cy - 2.0f, cx, cy + 3.0f);
  g.setColour(tok::textDim);
  g.fillPath(tri);
}

void SymbioticLook::drawPopupMenuBackground(juce::Graphics& g, int w, int h) {
  g.fillAll(tok::panel);
  g.setColour(tok::glassEdge);
  g.drawRect(0, 0, w, h, 1);
}

void SymbioticLook::drawScrollbar(juce::Graphics& g, juce::ScrollBar&, int x, int y, int w, int h, bool vertical, int ts, int tsz, bool over, bool down) {
  g.setColour(tok::carbon.withAlpha(0.6f));
  g.fillRect(x, y, w, h);
  auto t = vertical ? juce::Rectangle<int>(x + 2, ts, w - 4, tsz) : juce::Rectangle<int>(ts, y + 2, tsz, h - 4);
  g.setColour(down ? tok::textDim : (over ? tok::titanium.brighter(0.3f) : tok::titanium));
  g.fillRoundedRectangle(t.toFloat(), 3.0f);
}

void SymbioticLook::fillTextEditorBackground(juce::Graphics& g, int w, int h, juce::TextEditor&) {
  g.setColour(tok::carbon);
  g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h), tok::radiusSmall);
}

void SymbioticLook::drawTextEditorOutline(juce::Graphics& g, int w, int h, juce::TextEditor& e) {
  g.setColour(e.hasKeyboardFocus(true) ? tok::focus : tok::glassEdge);
  g.drawRoundedRectangle(0.5f, 0.5f, static_cast<float>(w) - 1.0f, static_cast<float>(h) - 1.0f, tok::radiusSmall, 1.0f);
}

void SymbioticLook::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle st, juce::Slider& s) {
  const bool vert = st == juce::Slider::LinearVertical;
  auto r = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));
  auto rail = vert ? r.withSizeKeepingCentre(6.0f, r.getHeight()) : r.withSizeKeepingCentre(r.getWidth(), 6.0f);
  g.setColour(tok::carbon);
  g.fillRoundedRectangle(rail, 3.0f);
  g.setColour(s.findColour(juce::Slider::trackColourId, true) == juce::Colour() ? tok::mod : s.findColour(juce::Slider::trackColourId));
  auto fill = vert ? rail.withTop(pos) : rail.withRight(pos);
  g.fillRoundedRectangle(fill, 3.0f);
  g.setColour(tok::titanium.brighter(0.25f));
  auto th = vert ? juce::Rectangle<float>(r.getCentreX() - 9.0f, pos - 3.0f, 18.0f, 6.0f) : juce::Rectangle<float>(pos - 3.0f, r.getCentreY() - 9.0f, 6.0f, 18.0f);
  g.fillRoundedRectangle(th, 2.0f);
  if (s.hasKeyboardFocus(false)) {
    g.setColour(tok::focus);
    g.drawRoundedRectangle(th.expanded(1.5f), 2.0f, 1.0f);
  }
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
