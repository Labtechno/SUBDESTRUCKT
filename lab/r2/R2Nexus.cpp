// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — NEXUS Hybrid (implémentation).
#include "r2/R2Nexus.h"

#include <cmath>

namespace sbd::r2 {

namespace {
constexpr double kDragPixelsFull = 220.0;                       // tok::dragPixelsFull (0.4.1)
constexpr double kFine = 0.1;                                    // tok::fineFactor
constexpr float kKeyStep = 0.01f, kPageStep = 0.10f;
constexpr int kWheelMs = 450;  // tok::wheelGestureMs

}  // namespace

Nexus::Nexus(Size size, Domain domain, const juce::String& label, const juce::String& fullName, ValueSource& src)
    : size_(size), domain_(domain), label_(label), full_(fullName), src_(src), guard_(&src) {
  setWantsKeyboardFocus(true);
  setTitle(fullName);  // titre accessible = nom complet (nom hôte en intégration)
  setTooltip(fullName);
  src_.onChange = [this] { repaint(); };
}

Nexus::~Nexus() {
  stopTimer();
  guard_.close();  // destruction pendant un geste : geste refermé (cas PB-02 pour les composants R2)
  src_.onChange = nullptr;
}

juce::Point<int> Nexus::cellSize(Size s) {
  switch (s) {
    case Size::XL: return {t::nexusXL.w, t::nexusXL.h};
    case Size::S: return {t::nexusS.w, t::nexusS.h};
    case Size::Micro: break;
  }
  return {t::nexusMicro.w, t::nexusMicro.h};
}

void Nexus::setInactive(bool inactive, const juce::String& reason) {
  inactive_ = inactive;
  inactiveWhy_ = reason;
  setTooltip(inactive ? full_ + "\nINACTIVE: " + reason : full_);
  repaint();
}

juce::Rectangle<float> Nexus::valueArea() const { return nexusValueArea(getLocalBounds(), size_); }

juce::Rectangle<float> Nexus::knobArea() const { return nexusKnobArea(getLocalBounds(), size_, label_, labelPx_); }

float Nexus::angleOf(float norm) const { return nexusAngle(norm); }

void Nexus::paint(juce::Graphics& g) {
  const bool disabled = forced_ == Forced::Disabled || !isEnabled();
  const bool hover = forced_ == Forced::Hover || (forced_ == Forced::None && isMouseOverOrDragging());
  const bool focus = forced_ == Forced::Focus || (forced_ == Forced::None && hasKeyboardFocus(false));
  const bool pressed = forced_ == Forced::Pressed || (forced_ == Forced::None && guard_.isOpen());
  NexusView v;  // vue du peintre partagé (ui/r2/R2Painters.h)
  v.size = size_;
  v.domain = domain_;
  v.label = label_;
  v.labelPx = labelPx_;
  v.norm = src_.norm();
  v.effective = src_.effectiveNorm();
  v.bipolar = src_.bipolar();
  v.routeDepths = src_.routeDepths();
  v.valueText = src_.text(v.norm);
  v.inactive = inactive_;
  v.entry = entry_;
  v.disabled = disabled;
  v.hover = hover;
  v.focus = focus;
  v.pressed = pressed;
  paintNexus(g, getLocalBounds(), v);
}

void Nexus::mouseDown(const juce::MouseEvent& e) {
  if (!isEnabled()) return;
  grabKeyboardFocus();
  if (e.mods.isPopupMenu()) return;
  stopTimer();
  if (wheel_) {
    wheel_ = false;
    guard_.close();
  }
  if (e.mods.isAltDown()) {
    src_.complete(src_.defaultNorm());
    return;
  }
  if (valueArea().contains(e.position)) return;  // zone de valeur : double-clic = saisie
  dragging_ = true;
  fineAtStart_ = e.mods.isShiftDown();
  dragStartY_ = e.position.y;
  dragStartNorm_ = src_.norm();
  guard_.open();
  repaint();
}

void Nexus::mouseDrag(const juce::MouseEvent& e) {
  if (!dragging_ || !guard_.isOpen()) return;
  const bool fine = e.mods.isShiftDown();
  if (fine != fineAtStart_) {  // changement de mode fin pendant le geste : repartir de la position (0.4.1)
    fineAtStart_ = fine;
    dragStartY_ = e.position.y;
    dragStartNorm_ = src_.norm();
  }
  const double f = fine ? kFine : 1.0;
  const double d = static_cast<double>(dragStartY_ - e.position.y) / kDragPixelsFull * f;
  src_.during(juce::jlimit(0.0f, 1.0f, dragStartNorm_ + static_cast<float>(d)));
}

void Nexus::mouseUp(const juce::MouseEvent&) {
  dragging_ = false;
  guard_.close();
  repaint();
}

void Nexus::mouseDoubleClick(const juce::MouseEvent& e) {
  if (!isEnabled()) return;
  if (valueArea().contains(e.position)) {
    entry_ = !entry_;  // saisie numérique : champ de 0.4.1 réutilisé en intégration (R2-B2)
    repaint();
    return;
  }
  // valeur par défaut DANS le geste ouvert par le second clic : aucun geste imbriqué (protocole PB-01)
  if (guard_.isOpen()) src_.during(src_.defaultNorm());
  else src_.complete(src_.defaultNorm());
}

void Nexus::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& w) {
  const float dy = (std::abs(w.deltaY) > std::abs(w.deltaX) ? w.deltaY : -w.deltaX) * (w.isReversed ? -1.0f : 1.0f);
  if (dy == 0.0f || dragging_ || !isEnabled()) return;
  if (!wheel_) wheel_ = guard_.open();
  const double step = (e.mods.isShiftDown() ? kFine : 1.0) * 0.05 * dy;
  src_.during(juce::jlimit(0.0f, 1.0f, src_.norm() + static_cast<float>(step)));
  startTimer(kWheelMs);
}

void Nexus::timerCallback() {
  stopTimer();
  if (wheel_) {
    wheel_ = false;
    guard_.close();
  }
}

void Nexus::stepNorm(float delta) {
  const int n = src_.steps();
  const float d = n > 1 ? (delta > 0.0f ? 1.0f : -1.0f) / static_cast<float>(n - 1) : delta;
  src_.complete(juce::jlimit(0.0f, 1.0f, src_.norm() + d));
}

bool Nexus::keyPressed(const juce::KeyPress& k) {
  if (!isEnabled()) return false;
  timerCallback();  // un geste molette ouvert est refermé avant une saisie clavier
  const float f = k.getModifiers().isShiftDown() ? static_cast<float>(kFine) : 1.0f;
  const int code = k.getKeyCode();
  if (code == juce::KeyPress::upKey || code == juce::KeyPress::rightKey) return stepNorm(kKeyStep * f), true;
  if (code == juce::KeyPress::downKey || code == juce::KeyPress::leftKey) return stepNorm(-kKeyStep * f), true;
  if (code == juce::KeyPress::pageUpKey) return stepNorm(kPageStep), true;
  if (code == juce::KeyPress::pageDownKey) return stepNorm(-kPageStep), true;
  if (code == juce::KeyPress::homeKey) return src_.complete(0.0f), true;
  if (code == juce::KeyPress::endKey) return src_.complete(1.0f), true;
  if (code == juce::KeyPress::deleteKey || code == juce::KeyPress::backspaceKey) return src_.complete(src_.defaultNorm()), true;
  if (code == juce::KeyPress::returnKey) return setEntryShown(!entry_), true;
  return false;
}

void Nexus::visibilityChanged() {
  if (!isShowing()) {  // masquage pendant un geste (changement de vue, repli de disposition) : geste refermé
    stopTimer();
    wheel_ = dragging_ = false;
    guard_.close();
  }
}

void Nexus::parentHierarchyChanged() {
  if (getParentComponent() == nullptr) visibilityChanged();
}

}  // namespace sbd::r2
