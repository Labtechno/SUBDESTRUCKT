// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — PLASMA CHANNEL Hybrid (implémentation).
#include "r2/R2PlasmaChannel.h"

namespace sbd::r2 {

PlasmaChannel::PlasmaChannel(Orientation o, Domain d, const juce::String& label, const juce::String& fullName, ValueSource& src,
                             juce::StringArray graduations)
    : o_(o), d_(d), label_(label), src_(src), grads_(std::move(graduations)), guard_(&src) {
  setTitle(fullName);
  setTooltip(fullName);
  setWantsKeyboardFocus(true);
  src_.onChange = [this] { repaint(); };
}

PlasmaChannel::~PlasmaChannel() {
  guard_.close();
  src_.onChange = nullptr;
}

juce::Rectangle<float> PlasmaChannel::slot() const { return plasmaSlot(getLocalBounds(), o_, compact()); }

juce::Point<float> PlasmaChannel::posOf(float n) const { return plasmaPosOf(slot(), o_, n); }

void PlasmaChannel::paint(juce::Graphics& g) {
  const bool disabled = forced_ == Forced::Disabled || !isEnabled();
  const bool hover = forced_ == Forced::Hover || (forced_ == Forced::None && isMouseOverOrDragging());
  const bool focus = forced_ == Forced::Focus || (forced_ == Forced::None && hasKeyboardFocus(false));
  const bool pressed = forced_ == Forced::Pressed || (forced_ == Forced::None && guard_.isOpen());
  PlasmaView v;  // vue du peintre partagé (ui/r2/R2Painters.h)
  v.orientation = o_;
  v.domain = d_;
  v.label = label_;
  v.graduations = grads_;
  v.compact = compact();
  v.norm = src_.norm();
  v.effective = src_.effectiveNorm();
  v.bipolar = src_.bipolar();
  v.valueText = src_.text(src_.norm());
  v.disabled = disabled;
  v.hover = hover;
  v.focus = focus;
  v.pressed = pressed;
  paintPlasma(g, getLocalBounds(), v);
}

void PlasmaChannel::mouseDown(const juce::MouseEvent& e) {
  if (!isEnabled() || e.mods.isPopupMenu()) return;
  grabKeyboardFocus();
  if (e.mods.isAltDown()) {
    src_.complete(src_.defaultNorm());
    return;
  }
  fine_ = e.mods.isShiftDown();
  startPos_ = o_ == Orientation::Vertical ? e.position.y : e.position.x;
  startNorm_ = src_.norm();
  guard_.open();
  repaint();
}

void PlasmaChannel::mouseDrag(const juce::MouseEvent& e) {
  if (!guard_.isOpen()) return;
  const float pos = o_ == Orientation::Vertical ? e.position.y : e.position.x;
  if (e.mods.isShiftDown() != fine_) {
    fine_ = e.mods.isShiftDown();
    startPos_ = pos;
    startNorm_ = src_.norm();
  }
  const float sign = o_ == Orientation::Vertical ? -1.0f : 1.0f;
  const float d = sign * (pos - startPos_) / 220.0f * (fine_ ? 0.1f : 1.0f);
  src_.during(juce::jlimit(0.0f, 1.0f, startNorm_ + d));
}

void PlasmaChannel::mouseUp(const juce::MouseEvent&) {
  guard_.close();
  repaint();
}

void PlasmaChannel::mouseDoubleClick(const juce::MouseEvent&) {
  if (guard_.isOpen()) src_.during(src_.defaultNorm());  // pas de geste imbriqué (protocole PB-01)
  else src_.complete(src_.defaultNorm());
}

bool PlasmaChannel::keyPressed(const juce::KeyPress& k) {
  const float f = k.getModifiers().isShiftDown() ? 0.1f : 1.0f;
  const int code = k.getKeyCode();
  const bool V = o_ == Orientation::Vertical;
  float d = 0.0f;
  if (code == (V ? juce::KeyPress::upKey : juce::KeyPress::rightKey)) d = 0.01f * f;
  else if (code == (V ? juce::KeyPress::downKey : juce::KeyPress::leftKey)) d = -0.01f * f;
  else if (code == juce::KeyPress::pageUpKey) d = 0.10f;
  else if (code == juce::KeyPress::pageDownKey) d = -0.10f;
  else if (code == juce::KeyPress::deleteKey || code == juce::KeyPress::backspaceKey) return src_.complete(src_.defaultNorm()), true;
  else return false;
  src_.complete(juce::jlimit(0.0f, 1.0f, src_.norm() + d));
  return true;
}

void PlasmaChannel::visibilityChanged() {
  if (!isShowing()) guard_.close();
}

}  // namespace sbd::r2
