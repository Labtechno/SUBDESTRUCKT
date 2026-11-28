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

juce::Rectangle<float> PlasmaChannel::slot() const {
  auto r = getLocalBounds().toFloat();
  if (o_ == Orientation::Vertical) {
    r.removeFromTop(30.0f);     // sérigraphie (deux lignes au plus)
    r.removeFromBottom(18.0f);  // valeur
    return juce::Rectangle<float>(r.getX() + 10.0f, r.getY() + 6.0f, 8.0f, r.getHeight() - 12.0f);
  }
  r.removeFromLeft(std::round(r.getWidth() * 0.30f));  // légende
  r.removeFromRight(54.0f);                            // valeur
  return juce::Rectangle<float>(r.getX() + 8.0f, r.getY() + 9.0f, r.getWidth() - 16.0f, 8.0f);
}

juce::Point<float> PlasmaChannel::posOf(float n) const {
  const auto s = slot();
  n = juce::jlimit(0.0f, 1.0f, n);
  if (o_ == Orientation::Vertical) return {s.getCentreX(), s.getBottom() - n * s.getHeight()};
  return {s.getX() + n * s.getWidth(), s.getCentreY()};
}

void PlasmaChannel::paint(juce::Graphics& g) {
  const bool disabled = forced_ == Forced::Disabled || !isEnabled();
  const bool hover = forced_ == Forced::Hover || (forced_ == Forced::None && isMouseOverOrDragging());
  const bool focus = forced_ == Forced::Focus || (forced_ == Forced::None && hasKeyboardFocus(false));
  const bool pressed = forced_ == Forced::Pressed || (forced_ == Forced::None && guard_.isOpen());
  const auto accent = domainColour(d_);
  const bool V = o_ == Orientation::Vertical;
  const auto s = slot();
  auto all = getLocalBounds().toFloat();

  // sérigraphie et valeur
  const auto lf = labelFont(tok2::fontLabel);
  if (V) draw::fittedText(g, label_, all.withHeight(30.0f).reduced(1.0f, 0.0f), lf, disabled ? t::textMute : t::text, juce::Justification::centred, true);
  else draw::fittedText(g, label_, all.withWidth(std::round(all.getWidth() * 0.30f)).reduced(3.0f, 0.0f), lf, disabled ? t::textMute : t::text,
                        juce::Justification::centredLeft, true);
  auto va = V ? all.withTop(all.getBottom() - 17.0f).reduced(2.0f, 1.0f) : all.withLeft(all.getRight() - 52.0f).reduced(1.0f, 10.0f);
  g.setColour(t::carbon);
  g.fillRoundedRectangle(va, 2.0f);
  g.setColour(pressed && !disabled ? accent.withAlpha(0.8f) : t::glassEdge);
  g.drawRoundedRectangle(va.reduced(0.5f), 2.0f, 0.8f);
  g.setFont(valueFont(10.0f));
  g.setColour(disabled ? t::textMute : t::text);
  g.drawText(disabled ? juce::String::fromUTF8("\xe2\x80\x94") : src_.text(src_.norm()), va, juce::Justification::centred, false);

  // gorge usinée et graduations
  g.setColour(tok2::groove);
  g.fillRoundedRectangle(s.expanded(2.0f), 3.0f);
  g.setColour(tok2::bevelLight.withAlpha(0.3f));
  g.drawRoundedRectangle(s.expanded(2.0f).translated(0.0f, 0.6f), 3.0f, 0.7f);
  if (hover && !disabled) {  // survol : contour de la gorge
    g.setColour(t::textDim.withAlpha(0.75f));
    g.drawRoundedRectangle(s.expanded(4.5f), 4.0f, 1.0f);
  }
  const int ng = grads_.size();
  g.setFont(labelFont(8.5f));
  for (int k = 0; k < ng; ++k) {
    const float n = ng > 1 ? static_cast<float>(k) / static_cast<float>(ng - 1) : 0.0f;
    const auto p = posOf(n);
    g.setColour(t::textMute);
    if (V) {
      g.drawLine(s.getRight() + 4.0f, p.y, s.getRight() + 8.0f, p.y, 1.0f);
      g.drawText(grads_[k], juce::Rectangle<float>(s.getRight() + 9.0f, p.y - 5.0f, all.getRight() - s.getRight() - 9.0f, 10.0f),
                 juce::Justification::centredLeft, false);
    } else {
      g.drawLine(p.x, s.getBottom() + 4.0f, p.x, s.getBottom() + 8.0f, 1.0f);
      const float w = 30.0f;
      const float x = k == 0 ? p.x - 2.0f : (k == ng - 1 ? p.x - w + 2.0f : p.x - w * 0.5f);
      g.drawText(grads_[k], juce::Rectangle<float>(x, s.getBottom() + 8.0f, w, 10.0f),
                 k == 0 ? juce::Justification::centredLeft : (k == ng - 1 ? juce::Justification::centredRight : juce::Justification::centred), false);
    }
  }
  if (disabled) return;

  // canal lumineux de Base (depuis le minimum, ou depuis le centre en bipolaire)
  const float norm = src_.norm();
  const auto p0 = posOf(src_.bipolar() ? 0.5f : 0.0f), pv = posOf(norm);
  auto fill = V ? juce::Rectangle<float>(s.getX(), std::min(p0.y, pv.y), s.getWidth(), std::abs(p0.y - pv.y))
                : juce::Rectangle<float>(std::min(p0.x, pv.x), s.getY(), std::abs(p0.x - pv.x), s.getHeight());
  g.setColour(accent.withAlpha(tok2::glowMax * 0.45f));
  g.fillRoundedRectangle(fill.expanded(2.5f), 3.0f);
  juce::ColourGradient plasma(accent.brighter(0.35f), fill.getX(), fill.getY(), accent.darker(0.25f), V ? fill.getRight() : fill.getX(),
                              V ? fill.getY() : fill.getBottom(), false);
  g.setGradientFill(plasma);
  g.fillRoundedRectangle(fill, 2.0f);
  if (src_.bipolar()) {
    g.setColour(t::text);
    if (V) g.drawLine(s.getX() - 3.0f, p0.y, s.getRight() + 3.0f, p0.y, 1.2f);
    else g.drawLine(p0.x, s.getY() - 3.0f, p0.x, s.getBottom() + 3.0f, 1.2f);
  }

  // marqueur Effective indépendant (côté opposé aux graduations)
  if (const auto eff = src_.effectiveNorm()) {
    const auto pe = posOf(*eff);
    juce::Path tri;
    if (V) tri.addTriangle(s.getX() - 9.0f, pe.y - 4.5f, s.getX() - 9.0f, pe.y + 4.5f, s.getX() - 3.0f, pe.y);
    else tri.addTriangle(pe.x - 4.5f, s.getY() - 9.0f, pe.x + 4.5f, s.getY() - 9.0f, pe.x, s.getY() - 3.0f);
    g.setColour(tok2::effective);
    g.fillPath(tri);
    g.setColour(tok2::effective.withAlpha(0.6f));
    if (V) g.drawLine(s.getX() - 3.0f, pe.y, s.getRight(), pe.y, 1.0f);
    else g.drawLine(pe.x, s.getY() - 3.0f, pe.x, s.getBottom(), 1.0f);
  }

  // curseur mécanique : titane, stries de préhension, index
  auto cur = V ? juce::Rectangle<float>(0.0f, 0.0f, 24.0f, 10.0f).withCentre(pv) : juce::Rectangle<float>(0.0f, 0.0f, 10.0f, 24.0f).withCentre(pv);
  draw::key(g, cur, tok2::relief3, true, pressed, hover);
  g.setColour(tok2::bevelDark.withAlpha(0.8f));
  for (int k = -1; k <= 1; k += 2) {
    if (V) g.drawLine(cur.getX() + 4.0f, pv.y + 2.5f * static_cast<float>(k), cur.getRight() - 4.0f, pv.y + 2.5f * static_cast<float>(k), 0.8f);
    else g.drawLine(pv.x + 2.5f * static_cast<float>(k), cur.getY() + 4.0f, pv.x + 2.5f * static_cast<float>(k), cur.getBottom() - 4.0f, 0.8f);
  }
  g.setColour(t::text);
  if (V) g.drawLine(cur.getX() + 2.0f, pv.y, cur.getRight() - 2.0f, pv.y, 1.4f);
  else g.drawLine(pv.x, cur.getY() + 2.0f, pv.x, cur.getBottom() - 2.0f, 1.4f);
  if (focus) draw::focusRing(g, s.expanded(V ? 9.0f : 4.0f, V ? 4.0f : 9.0f), 4.0f);
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
