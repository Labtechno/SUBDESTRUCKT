#include "Widgets.h"

#include <cstring>

namespace sbd::ui {

juce::Font mono(float h, bool bold) {
  return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), h, bold ? juce::Font::bold : juce::Font::plain));
}
juce::Font sans(float h, bool bold) {
  return juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), h, bold ? juce::Font::bold : juce::Font::plain));
}

juce::Colour accentFor(ParamId id) {
  const char* g = paramSpec(id).group;
  if (std::strcmp(g, "wt") == 0) return col::wt;
  if (std::strncmp(g, "op", 2) == 0 || std::strncmp(g, "fm", 2) == 0) return col::fm;
  if (std::strcmp(g, "cross") == 0 || std::strncmp(g, "lfo", 3) == 0 || std::strcmp(g, "macro") == 0 ||
      std::strncmp(g, "env", 3) == 0 || std::strcmp(g, "random") == 0 || std::strcmp(g, "follower") == 0)
    return col::cross;
  return col::text.withAlpha(0.75f);
}

Look::Look() {
  setColour(juce::ResizableWindow::backgroundColourId, col::chassis);
  setColour(juce::Label::textColourId, col::text);
  setColour(juce::Slider::textBoxTextColourId, col::text);
  setColour(juce::Slider::textBoxBackgroundColourId, col::display);
  setColour(juce::Slider::textBoxOutlineColourId, col::control);
  setColour(juce::ComboBox::backgroundColourId, col::display);
  setColour(juce::ComboBox::textColourId, col::text);
  setColour(juce::ComboBox::outlineColourId, col::control);
  setColour(juce::ComboBox::arrowColourId, col::dim);
  setColour(juce::PopupMenu::backgroundColourId, col::panel);
  setColour(juce::PopupMenu::textColourId, col::text);
  setColour(juce::PopupMenu::highlightedBackgroundColourId, col::control);
  setColour(juce::TextButton::buttonColourId, col::control);
  setColour(juce::TextButton::buttonOnColourId, col::cross.darker(0.4f));
  setColour(juce::TextButton::textColourOffId, col::text);
  setColour(juce::TextButton::textColourOnId, col::text);
  setColour(juce::ToggleButton::textColourId, col::text);
  setColour(juce::ToggleButton::tickColourId, col::wt);
  setColour(juce::ToggleButton::tickDisabledColourId, col::dim);
  setColour(juce::TextEditor::backgroundColourId, col::display);
  setColour(juce::TextEditor::textColourId, col::text);
  setColour(juce::TextEditor::outlineColourId, col::control);
  setColour(juce::ListBox::backgroundColourId, col::display);
  setColour(juce::ScrollBar::thumbColourId, col::control);
  setColour(juce::TabbedButtonBar::tabTextColourId, col::dim);
  setColour(juce::TabbedButtonBar::frontTextColourId, col::text);
}

void Look::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float a0, float a1, juce::Slider& s) {
  const auto b = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)).reduced(3.0f);
  const float r = std::min(b.getWidth(), b.getHeight()) * 0.5f;
  const auto c = b.getCentre();
  const auto accent = s.findColour(juce::Slider::rotarySliderFillColourId);
  // corps (aspect matériel discret)
  g.setColour(col::control);
  g.fillEllipse(c.x - r * 0.72f, c.y - r * 0.72f, r * 1.44f, r * 1.44f);
  g.setColour(col::chassis.brighter(0.15f));
  g.drawEllipse(c.x - r * 0.72f, c.y - r * 0.72f, r * 1.44f, r * 1.44f, 1.0f);
  // piste et valeur Base
  juce::Path track, val;
  track.addCentredArc(c.x, c.y, r * 0.9f, r * 0.9f, 0.0f, a0, a1, true);
  g.setColour(col::display);
  g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
  const float a = a0 + pos * (a1 - a0);
  val.addCentredArc(c.x, c.y, r * 0.9f, r * 0.9f, 0.0f, a0, a, true);
  g.setColour(s.isEnabled() ? accent : col::dim);
  g.strokePath(val, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
  // index
  const juce::Point<float> p(c.x + r * 0.62f * std::sin(a), c.y - r * 0.62f * std::cos(a));
  g.setColour(col::text);
  g.drawLine(c.x + r * 0.2f * std::sin(a), c.y - r * 0.2f * std::cos(a), p.x, p.y, 2.0f);
  if (s.hasKeyboardFocus(true)) {
    g.setColour(col::cross);
    g.drawEllipse(b.reduced(1.0f), 1.0f);  // focus clavier visible
  }
}

void Look::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour& bg, bool over, bool down) {
  auto r = b.getLocalBounds().toFloat().reduced(0.5f);
  auto c = b.getToggleState() ? b.findColour(juce::TextButton::buttonOnColourId) : bg;
  if (down) c = c.brighter(0.2f);
  else if (over) c = c.brighter(0.08f);
  g.setColour(c);
  g.fillRoundedRectangle(r, 3.0f);
  g.setColour(b.hasKeyboardFocus(false) ? col::cross : col::chassis.brighter(0.2f));
  g.drawRoundedRectangle(r, 3.0f, 1.0f);
}

// ---------------------------------------------------------------- ParamControl
ParamControl::ParamControl(SubdestrucktProcessor& p, ParamId id, SelectFn onSelect, bool large)
    : proc_(p), id_(id), onSelect_(std::move(onSelect)), large_(large) {
  const auto& s = paramSpec(id);
  auto* param = proc_.parameters().param(idx(id));
  clicker_.owner = this;
  caption_.setText(s.name, juce::dontSendNotification);
  caption_.setFont(sans(large ? 13.0f : 11.0f, large));
  caption_.setJustificationType(juce::Justification::centred);
  caption_.setColour(juce::Label::textColourId, col::text);
  caption_.setInterceptsMouseClicks(false, false);
  addAndMakeVisible(caption_);
  const juce::Colour accent = accentFor(id);
  if (s.type == ParamType::Choice) {
    combo_ = std::make_unique<juce::ComboBox>();
    for (int k = 0; k < choiceCount(id); ++k) combo_->addItem(juce::String(choiceLabel(id, k)), k + 1);  // libellés du registre
    combo_->setTitle(s.name);
    combo_->setHelpText(s.stableId);
    cAtt_ = std::make_unique<juce::ComboBoxParameterAttachment>(*param, *combo_, nullptr);
    combo_->addMouseListener(&clicker_, true);
    addAndMakeVisible(*combo_);
  } else if (s.type == ParamType::Bool) {
    toggle_ = std::make_unique<juce::ToggleButton>(s.name);
    toggle_->setTitle(s.name);
    toggle_->setHelpText(s.stableId);
    bAtt_ = std::make_unique<juce::ButtonParameterAttachment>(*param, *toggle_, nullptr);
    toggle_->addMouseListener(&clicker_, true);
    addAndMakeVisible(*toggle_);
    caption_.setVisible(false);
  } else {
    slider_ = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
    slider_->setTitle(s.name);
    slider_->setHelpText(s.stableId);
    slider_->setColour(juce::Slider::rotarySliderFillColourId, accent);
    slider_->setTextBoxStyle(juce::Slider::TextBoxBelow, false, large ? 86 : 70, 16);
    slider_->setDoubleClickReturnValue(true, s.defaultValue);
    slider_->setWantsKeyboardFocus(true);
    sAtt_ = std::make_unique<juce::SliderParameterAttachment>(*param, *slider_, nullptr);
    slider_->setDoubleClickReturnValue(true, s.defaultValue);  // remise au défaut explicite
    slider_->addMouseListener(&clicker_, true);
    addAndMakeVisible(*slider_);
  }
  setTitle(s.name);
}

ParamControl::~ParamControl() {
  if (slider_) slider_->removeMouseListener(&clicker_);
  if (combo_) combo_->removeMouseListener(&clicker_);
  if (toggle_) toggle_->removeMouseListener(&clicker_);
}

void ParamControl::setCaption(const juce::String& c) { caption_.setText(c, juce::dontSendNotification); }

void ParamControl::select() {
  if (onSelect_) onSelect_(id_);
}

bool ParamControl::keyPressed(const juce::KeyPress& k) {
  if (k == juce::KeyPress::returnKey) {
    select();
    return true;
  }
  return false;
}

void ParamControl::resized() {
  auto r = getLocalBounds();
  if (toggle_) {
    toggle_->setBounds(r.reduced(2));
    return;
  }
  caption_.setBounds(r.removeFromTop(large_ ? 18 : 15));
  if (combo_) combo_->setBounds(r.reduced(2).withHeight(24));
  if (slider_) slider_->setBounds(r);
}

void ParamControl::refreshTelemetry() {
  if (!slider_) return;
  auto& t = proc_.telemetry();
  const auto i = static_cast<size_t>(idx(id_));
  float n = -1.0f;
  if (t.modulated[i].load(std::memory_order_relaxed)) {
    auto* p = proc_.parameters().param(idx(id_));
    n = p->convertTo0to1(t.effective[i].load(std::memory_order_relaxed));
  }
  if (n != effNorm_) {
    effNorm_ = n;
    repaint();
  }
}

void ParamControl::paintOverChildren(juce::Graphics& g) {
  if (effNorm_ < 0.0f || !slider_) return;
  // anneau extérieur : valeur Effective (modulation), distincte de la Base (arc intérieur)
  const auto sb = slider_->getBounds().toFloat();
  const auto tb = sb.withTrimmedBottom(18.0f).reduced(1.0f);
  const float r = std::min(tb.getWidth(), tb.getHeight()) * 0.5f;
  const auto c = tb.getCentre();
  const auto rp = slider_->getRotaryParameters();
  const float a = rp.startAngleRadians + effNorm_ * (rp.endAngleRadians - rp.startAngleRadians);
  juce::Path arc;
  arc.addCentredArc(c.x, c.y, r, r, 0.0f, rp.startAngleRadians, a, true);
  g.setColour(col::cross.withAlpha(0.9f));
  g.strokePath(arc, juce::PathStrokeType(1.6f));
  g.fillEllipse(c.x + r * std::sin(a) - 2.5f, c.y - r * std::cos(a) - 2.5f, 5.0f, 5.0f);
}

// ---------------------------------------------------------------- ParamGrid
void ParamGrid::setParams(const std::vector<ParamId>& ids) {
  bool same = ids.size() == controls_.size();
  for (size_t k = 0; same && k < ids.size(); ++k) same = controls_[k]->id() == ids[k];
  if (same) return;
  controls_.clear();
  for (auto id : ids) {
    controls_.push_back(std::make_unique<ParamControl>(proc_, id, onSelect_));
    addAndMakeVisible(*controls_.back());
  }
  resized();
}

int ParamGrid::preferredHeight(int width) const {
  const int cols = std::max(1, width / cw_);
  const int rows = (static_cast<int>(controls_.size()) + cols - 1) / cols;
  return rows * ch_;
}

void ParamGrid::resized() {
  const int cols = std::max(1, getWidth() / cw_);
  for (size_t k = 0; k < controls_.size(); ++k) {
    const int cx = static_cast<int>(k) % cols, cy = static_cast<int>(k) / cols;
    controls_[k]->setBounds(cx * cw_, cy * ch_, cw_ - 4, ch_ - 4);
  }
}

void ParamGrid::refreshTelemetry() {
  for (auto& c : controls_) c->refreshTelemetry();
}

// ---------------------------------------------------------------- Section
void Section::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat().reduced(1.0f);
  g.setColour(col::panel);
  g.fillRoundedRectangle(r, 4.0f);
  g.setColour(accent_.withAlpha(0.8f));
  g.fillRect(r.getX() + 6.0f, r.getY() + 4.0f, 3.0f, 13.0f);
  g.setColour(col::text);
  g.setFont(sans(12.0f, true));
  g.drawText(title_, juce::Rectangle<float>(r.getX() + 14.0f, r.getY() + 2.0f, r.getWidth() - 20.0f, 17.0f), juce::Justification::centredLeft);
}

std::vector<ParamId> range(ParamId first, ParamId last) {
  std::vector<ParamId> v;
  for (int i = idx(first); i <= idx(last); ++i) v.push_back(static_cast<ParamId>(i));
  return v;
}

std::vector<ParamId> filterOffsets(ParamId base, const std::vector<int>& offsets) {
  std::vector<ParamId> v;
  for (int o : offsets) v.push_back(static_cast<ParamId>(idx(base) + o));
  return v;
}

}  // namespace sbd::ui
