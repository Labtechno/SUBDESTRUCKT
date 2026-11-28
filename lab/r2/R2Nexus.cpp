// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — NEXUS Hybrid (implémentation).
#include "r2/R2Nexus.h"

#include <cmath>

namespace sbd::r2 {

namespace {
constexpr float kA0 = -0.75f * juce::MathConstants<float>::pi;  // -135°
constexpr float kA1 = 0.75f * juce::MathConstants<float>::pi;   // +135°
constexpr double kDragPixelsFull = 220.0;                       // tok::dragPixelsFull (0.4.1)
constexpr double kFine = 0.1;                                    // tok::fineFactor
constexpr float kKeyStep = 0.01f, kPageStep = 0.10f;
constexpr int kWheelMs = 450;  // tok::wheelGestureMs

struct Geo {
  float labelPx, valuePx, ring, knobInset;
  int knurls, ticks;
  float arcW, indexW;
};
Geo geo(Nexus::Size s) {
  switch (s) {
    case Nexus::Size::XL: return {tok2::fontTitle, tok2::fontValueXL, 8.0f, 7.0f, 56, 11, 4.0f, 2.6f};
    case Nexus::Size::S: return {tok2::fontLabel, tok2::fontLabel, 6.0f, 5.5f, 0, 5, 3.4f, 2.2f};
    case Nexus::Size::Micro: break;
  }
  return {tok2::fontMicro, tok2::fontMicro, 4.0f, 4.5f, 0, 0, 2.8f, 1.8f};
}
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

juce::Rectangle<float> Nexus::valueArea() const {
  const auto G = geo(size_);
  auto r = getLocalBounds().toFloat();
  return r.removeFromBottom(G.valuePx + 7.0f).reduced(size_ == Size::Micro ? 3.0f : 6.0f, 1.0f);
}

juce::Rectangle<float> Nexus::knobArea() const {
  const auto G = geo(size_);
  const float lp = labelPx_ > 0.0f ? labelPx_ : G.labelPx;
  auto r = getLocalBounds().toFloat();
  r.removeFromBottom(G.valuePx + 9.0f);
  const bool two = draw::textWidth(labelFont(lp, size_ == Size::XL), label_) > r.getWidth() - 4.0f;
  r.removeFromTop((two ? 2.0f : 1.0f) * std::round(lp * 1.2f) + 4.0f);
  return r;
}

float Nexus::angleOf(float norm) const { return kA0 + (kA1 - kA0) * juce::jlimit(0.0f, 1.0f, norm); }

void Nexus::paint(juce::Graphics& g) {
  const auto G = geo(size_);
  const bool disabled = forced_ == Forced::Disabled || !isEnabled();
  const bool hover = forced_ == Forced::Hover || (forced_ == Forced::None && isMouseOverOrDragging());
  const bool focus = forced_ == Forced::Focus || (forced_ == Forced::None && hasKeyboardFocus(false));
  const bool pressed = forced_ == Forced::Pressed || (forced_ == Forced::None && guard_.isOpen());
  const auto accent = domainColour(domain_);
  const float lp = labelPx_ > 0.0f ? labelPx_ : G.labelPx;

  // sérigraphie
  auto all = getLocalBounds().toFloat();
  const auto lf = labelFont(lp, size_ == Size::XL);
  const bool two = draw::textWidth(lf, label_) > all.getWidth() - 4.0f;
  auto labelR = all.withHeight((two ? 2.0f : 1.0f) * std::round(lp * 1.2f) + 4.0f);
  draw::fittedText(g, label_, labelR.reduced(2.0f, 0.0f), lf, disabled ? t::textMute : (inactive_ ? tok2::inactive : t::text),
                   juce::Justification::centred, true);

  // géométrie du bouton
  const auto ka = knobArea();
  const auto c = ka.getCentre();
  const float arcR = std::min(ka.getWidth(), ka.getHeight()) * 0.5f - G.ring;
  const float knobR = arcR - G.knobInset;
  const float norm = src_.norm();
  const float aV = angleOf(norm);
  const float aZero = src_.bipolar() ? 0.0f : kA0;

  if (!disabled) {
    // graduations techniques (XL : 11, S : 5, Micro : aucune) ; centre marqué en bipolaire
    if (G.ticks > 0) draw::ticks(g, c, arcR + 3.0f, arcR + (size_ == Size::XL ? 6.5f : 5.0f), kA0, kA1, G.ticks, t::textMute.withAlpha(0.7f), 1.0f);
    if (src_.bipolar()) draw::ticks(g, c, arcR + 2.5f, arcR + (size_ == Size::Micro ? 5.0f : 8.0f), 0.0f, 0.0f, 2, t::text, 1.4f);
    // piste et arc de Base
    draw::arc(g, c, arcR, kA0, kA1, G.arcW, t::track);
    if (size_ == Size::Micro) draw::arc(g, c, arcR, aZero, aV, G.arcW, accent);
    else draw::glowArc(g, c, arcR, aZero, aV, G.arcW, accent);
    // marques de route (au plus quatre, puis +n ; aucune en Micro)
    const auto depths = src_.routeDepths();
    if (size_ != Size::Micro && !depths.empty()) {
      const float rr = arcR + G.ring - 1.0f;
      for (size_t k = 0; k < std::min<size_t>(4, depths.size()); ++k) {
        const float a = angleOf(norm + depths[k]);
        const float s = std::sin(a), co = -std::cos(a);
        g.setColour(tok2::effective.withAlpha(0.75f));
        g.drawLine(c.x + (rr - 3.0f) * s, c.y + (rr - 3.0f) * co, c.x + (rr + 2.0f) * s, c.y + (rr + 2.0f) * co, 1.2f);
      }
      if (depths.size() > 4) {
        g.setFont(labelFont(8.5f, true));
        g.setColour(tok2::effective);
        g.drawText("+" + juce::String(static_cast<int>(depths.size()) - 4), juce::Rectangle<float>(ka.getRight() - 22.0f, ka.getY(), 20.0f, 11.0f),
                   juce::Justification::centredRight, false);
      }
    }
    // Effective : anneau extérieur indépendant + marqueur losange (jamais confondu avec la Base)
    if (const auto eff = src_.effectiveNorm()) {
      const float aE = angleOf(*eff);
      const float re = arcR + G.ring - 1.5f;
      draw::arc(g, c, re, std::min(aV, aE), std::max(aV, aE), 1.4f, tok2::effective.withAlpha(0.55f));
      const float s = std::sin(aE), co = -std::cos(aE);
      const float d = size_ == Size::XL ? 4.2f : 3.2f;
      juce::Path dia;
      const juce::Point<float> p(c.x + re * s, c.y + re * co);
      dia.addQuadrilateral(p.x, p.y - d, p.x + d, p.y, p.x, p.y + d, p.x - d, p.y);
      g.setColour(tok2::effective);
      g.fillPath(dia);
      g.setColour(t::carbon);
      g.strokePath(dia, juce::PathStrokeType(0.8f));
    }
  }

  // corps usiné et index ; survol : halo fin autour du corps (forme, pas seulement couleur)
  if (hover && !disabled) {
    g.setColour(t::textDim.withAlpha(0.75f));
    g.drawEllipse(c.x - knobR - 3.0f, c.y - knobR - 3.0f, 2.0f * knobR + 6.0f, 2.0f * knobR + 6.0f, 1.0f);
  }
  draw::machinedKnob(g, c, knobR, G.knurls, hover, pressed, disabled);
  if (!disabled) {
    const float s = std::sin(aV), co = -std::cos(aV);
    const float r0 = knobR * (size_ == Size::XL ? 0.30f : 0.18f), r1 = knobR * (G.knurls > 0 ? 0.80f : 0.86f);
    g.setColour(t::carbon.withAlpha(0.7f));
    g.drawLine(c.x + r0 * s + 0.6f, c.y + r0 * co + 0.8f, c.x + r1 * s + 0.6f, c.y + r1 * co + 0.8f, G.indexW + (pressed ? 0.8f : 0.0f));
    g.setColour(t::text);
    g.drawLine(c.x + r0 * s, c.y + r0 * co, c.x + r1 * s, c.y + r1 * co, G.indexW + (pressed ? 0.8f : 0.0f));
    if (size_ == Size::XL) g.fillEllipse(c.x + r1 * s - 2.2f, c.y + r1 * co - 2.2f, 4.4f, 4.4f);
  }
  if (inactive_ && !disabled) {
    draw::hatch(g, ka.reduced(2.0f), tok2::inactive.withAlpha(0.28f), 4.0f);
    g.setFont(labelFont(8.5f, true));
    g.setColour(tok2::inactive);
    g.drawText("INACTIVE", ka.withHeight(11.0f).withTrimmedRight(2.0f), juce::Justification::centredRight, false);
  }

  // affichage numérique (Base) dans une fenêtre optique
  const auto va = valueArea();
  g.setColour(t::carbon);
  g.fillRoundedRectangle(va, 2.5f);
  g.setColour(pressed && !disabled ? accent.withAlpha(0.8f) : t::glassEdge);
  g.drawRoundedRectangle(va.reduced(0.5f), 2.5f, pressed ? 1.2f : 0.8f);
  g.setFont(valueFont(G.valuePx));
  g.setColour(disabled ? t::textMute : (pressed ? t::text.brighter(0.2f) : t::text));
  g.drawText(disabled ? juce::String::fromUTF8("\xe2\x80\x94") : (entry_ ? src_.text(norm) + "_" : src_.text(norm)), va,
             juce::Justification::centred, false);
  if (focus) draw::focusRing(g, ka.withSizeKeepingCentre(knobR * 2.0f + 2.0f * G.ring + 6.0f, knobR * 2.0f + 2.0f * G.ring + 6.0f), 6.0f);
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
