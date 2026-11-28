// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — NEURAL CELL Hybrid (implémentation).
#include "r2/R2NeuralCell.h"

namespace sbd::r2 {

NeuralCell::NeuralCell(Kind kind, Domain domain, const juce::String& label, const juce::String& fullName)
    : juce::Button(label), kind_(kind), domain_(domain), full_(fullName) {
  setTitle(fullName);
  setTooltip(fullName);
  setWantsKeyboardFocus(true);
  if (kind == Kind::Toggle || kind == Kind::Select) setClickingTogglesState(true);
  if (kind == Kind::Action) icon_ = Icon::Pulse;
}

void NeuralCell::setDisabledReason(const juce::String& why) {
  setTooltip(why.isEmpty() ? full_ : full_ + "\nUNAVAILABLE: " + why);
}

juce::String NeuralCell::stateText() const {
  if (!isEnabled() || forced_ == Forced::Disabled) return juce::String::fromUTF8("\xe2\x80\x94");
  switch (kind_) {
    case Kind::Toggle: return getToggleState() ? "ON" : "OFF";
    case Kind::Arm:
      switch (arm_) {
        case ArmState::Armed: return "ARMED";
        case ArmState::Active: return "ACTIVE";
        case ArmState::Denied: return "DENIED";
        case ArmState::Off: break;
      }
      return "OFF";
    case Kind::Action:
    case Kind::Select:
    case Kind::Safety: break;
  }
  return {};
}

void NeuralCell::paintIcon(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c) const {
  juce::Path p;
  const float x = r.getX(), y = r.getY(), w = r.getWidth(), h = r.getHeight();
  switch (icon_) {
    case Icon::Pulse:  // impulsion : créneau unique (action momentanée)
      p.startNewSubPath(x, y + h * 0.75f);
      p.lineTo(x + w * 0.3f, y + h * 0.75f);
      p.lineTo(x + w * 0.3f, y + h * 0.2f);
      p.lineTo(x + w * 0.7f, y + h * 0.2f);
      p.lineTo(x + w * 0.7f, y + h * 0.75f);
      p.lineTo(x + w, y + h * 0.75f);
      g.setColour(c);
      g.strokePath(p, juce::PathStrokeType(1.4f));
      return;
    case Icon::Capture: {  // capture : cadre d'instantané et point mémorisé
      const auto q = r.reduced(w * 0.05f, h * 0.12f);
      g.setColour(c);
      const float L = q.getWidth() * 0.32f;
      for (int k = 0; k < 4; ++k) {
        const float cx = (k % 2 == 0) ? q.getX() : q.getRight(), cy = (k < 2) ? q.getY() : q.getBottom();
        const float sx = (k % 2 == 0) ? 1.0f : -1.0f, sy = (k < 2) ? 1.0f : -1.0f;
        g.drawLine(cx, cy, cx + sx * L, cy, 1.3f);
        g.drawLine(cx, cy, cx, cy + sy * L, 1.3f);
      }
      g.fillEllipse(q.getCentreX() - 2.2f, q.getCentreY() - 2.2f, 4.4f, 4.4f);
      return;
    }
    case Icon::Authorize: {  // autorisation : anneau ouvert et barre (déverrouillage de l'exécution)
      const auto q = r.reduced(w * 0.12f, h * 0.08f);
      g.setColour(c);
      g.drawEllipse(q.getX(), q.getY(), q.getWidth() * 0.62f, q.getHeight() * 0.62f, 1.4f);
      g.drawLine(q.getX() + q.getWidth() * 0.52f, q.getY() + q.getHeight() * 0.52f, q.getRight(), q.getBottom(), 1.6f);
      g.drawLine(q.getRight() - q.getWidth() * 0.22f, q.getBottom() - q.getHeight() * 0.22f, q.getRight() - q.getWidth() * 0.05f,
                 q.getBottom() - q.getHeight() * 0.40f, 1.4f);
      return;
    }
    case Icon::None: break;
  }
}

void NeuralCell::paintButton(juce::Graphics& g, bool over, bool down) {
  if (rev_ == Rev::B1) paintB1(g, over, down);
  else paintR1(g, over, down);
}

// ---------------------------------------------------------------- B1 (Component Lab initial, inchangé)
void NeuralCell::paintB1(juce::Graphics& g, bool over, bool down) {
  const bool disabled = forced_ == Forced::Disabled || !isEnabled();
  const bool hover = !disabled && (forced_ == Forced::Hover || (forced_ == Forced::None && over));
  const bool pressed = !disabled && (forced_ == Forced::Pressed || (forced_ == Forced::None && down));
  const bool focus = forced_ == Forced::Focus || (forced_ == Forced::None && hasKeyboardFocus(false));
  const auto accent = domainColour(domain_);
  auto r = getLocalBounds().toFloat().reduced(1.5f, 2.0f);
  const auto lfont = labelFont(tok2::fontLabel, true);
  const auto sfont = labelFont(9.5f, true);

  if (disabled) {  // commande indisponible : contour seul, aucun témoin, tiret d'état
    g.setColour(t::panelEdge);
    g.drawRoundedRectangle(r, t::radiusSmall, 1.0f);
    draw::hatch(g, r.reduced(1.0f), t::panelEdge.withAlpha(0.5f), 6.0f);
    g.setFont(lfont);
    g.setColour(t::textMute);
    g.drawText(getButtonText(), r.reduced(10.0f, 0.0f), juce::Justification::centredLeft, false);
    g.setFont(sfont);
    g.drawText(stateText(), r.reduced(8.0f, 0.0f), juce::Justification::centredRight, false);
    if (focus) draw::focusRing(g, r.expanded(1.0f), t::radiusSmall + 1.0f);
    return;
  }

  switch (kind_) {
    case Kind::Action: {  // touche convexe, retour en creux à l'appui, icône d'impulsion
      draw::key(g, r, tok2::relief3, true, pressed, hover);
      auto in = r.reduced(9.0f, 0.0f);
      if (icon_ != Icon::None) {
        paintIcon(g, in.removeFromLeft(16.0f).withSizeKeepingCentre(16.0f, 14.0f).translated(0.0f, pressed ? 0.8f : 0.0f),
                  icon_ == Icon::Authorize ? t::warn : accent);
        in.removeFromLeft(7.0f);
      }
      g.setFont(lfont);
      g.setColour(t::text);
      g.drawText(getButtonText(), in.translated(0.0f, pressed ? 0.8f : 0.0f), juce::Justification::centredLeft, false);
      break;
    }
    case Kind::Toggle: {  // touche plate, témoin encastré, état ON/OFF
      draw::key(g, r, tok2::relief2, false, pressed, hover);
      const bool on = getToggleState();
      auto in = r.reduced(9.0f, 0.0f);
      draw::led(g, {in.getX() + 6.0f, r.getCentreY()}, 4.2f, on ? 2 : 0, accent);
      in.removeFromLeft(20.0f);
      g.setFont(sfont);
      g.setColour(on ? t::text : t::textDim);
      g.drawText(stateText(), in, juce::Justification::centredRight, false);
      g.setFont(lfont);
      g.setColour(t::text);
      g.drawText(getButtonText(), in.withTrimmedRight(30.0f), juce::Justification::centredLeft, false);
      break;
    }
    case Kind::Arm: {  // armement : liseré d'attente, témoin anneau (ARMED) ou plein (ACTIVE)
      draw::key(g, r, tok2::relief2, false, pressed, hover);
      g.setColour(t::warn.withAlpha(arm_ == ArmState::Off ? 0.35f : 0.85f));
      g.drawRoundedRectangle(r.reduced(1.5f), t::radiusSmall, arm_ == ArmState::Off ? 1.0f : 1.4f);
      auto in = r.reduced(9.0f, 0.0f);
      const int mode = arm_ == ArmState::Armed ? 1 : (arm_ == ArmState::Active ? 2 : 0);
      draw::led(g, {in.getX() + 6.0f, r.getCentreY()}, 4.2f, mode, t::warn);
      if (arm_ == ArmState::Denied) {
        const auto c = juce::Point<float>(in.getX() + 6.0f, r.getCentreY());
        g.setColour(t::textDim);
        g.drawLine(c.x - 3.0f, c.y - 3.0f, c.x + 3.0f, c.y + 3.0f, 1.3f);
        g.drawLine(c.x - 3.0f, c.y + 3.0f, c.x + 3.0f, c.y - 3.0f, 1.3f);
      }
      in.removeFromLeft(20.0f);
      g.setFont(sfont);
      g.setColour(arm_ == ArmState::Off ? t::textDim : t::warn);
      g.drawText(stateText(), in, juce::Justification::centredRight, false);
      g.setFont(lfont);
      g.setColour(t::text);
      g.drawText(getButtonText(), in.withTrimmedRight(50.0f), juce::Justification::centredLeft, false);
      break;
    }
    case Kind::Select: {  // sélection exclusive : fond carbone, losange et soulignement du domaine
      const bool sel = getToggleState() || forced_ == Forced::Pressed;
      g.setColour(sel ? t::carbon : tok2::relief1);
      g.fillRoundedRectangle(r, t::radiusSmall);
      g.setColour(hover ? t::panelEdge.brighter(0.3f) : t::panelEdge);
      g.drawRoundedRectangle(r.reduced(0.5f), t::radiusSmall, 1.0f);
      auto in = r.reduced(9.0f, 0.0f);
      const auto c = juce::Point<float>(in.getX() + 5.0f, r.getCentreY());
      juce::Path d;
      d.addQuadrilateral(c.x, c.y - 4.5f, c.x + 4.5f, c.y, c.x, c.y + 4.5f, c.x - 4.5f, c.y);
      g.setColour(sel ? accent : t::textMute);
      if (sel) g.fillPath(d);
      else g.strokePath(d, juce::PathStrokeType(1.1f));
      in.removeFromLeft(16.0f);
      g.setFont(lfont);
      g.setColour(sel ? t::text : t::textDim);
      g.drawText(getButtonText(), in, juce::Justification::centredLeft, false);
      if (sel) {
        g.setColour(accent);
        g.fillRect(r.getX() + 6.0f, r.getBottom() - 3.0f, r.getWidth() - 12.0f, 2.0f);
      }
      break;
    }
    case Kind::Safety: {  // PANIC : rouge réservé, touche convexe isolée, double biseau
      draw::key(g, r, t::alert, true, pressed, hover);
      g.setColour(t::alert.darker(0.6f));
      g.drawRoundedRectangle(r.reduced(2.5f), t::radiusSmall, 1.0f);
      g.setFont(labelFont(tok2::fontCritical + 1.0f, true));
      g.setColour(safetyDark_ ? t::carbon : juce::Colours::white);
      g.drawText(labelHidden_ ? juce::String() : getButtonText(), r.translated(0.0f, pressed ? 0.8f : 0.0f), juce::Justification::centred, false);
      break;
    }
  }
  if (hover && !focus) {  // survol : contour fin (forme), distinct de l'anneau de focus
    g.setColour(t::textDim.withAlpha(0.7f));
    g.drawRoundedRectangle(r.expanded(1.0f), t::radiusSmall + 1.0f, 1.0f);
  }
  if (focus) draw::focusRing(g, r.expanded(1.5f), t::radiusSmall + 1.5f);
}

// ---------------------------------------------------------------- R1 : Machined Titanium (LAB-A02)
void NeuralCell::paintR1(juce::Graphics& g, bool over, bool down) {
  const bool disabled = forced_ == Forced::Disabled || !isEnabled();
  const bool hover = !disabled && (forced_ == Forced::Hover || (forced_ == Forced::None && over));
  const bool pressed = !disabled && (forced_ == Forced::Pressed || (forced_ == Forced::None && down));
  const bool focus = forced_ == Forced::Focus || (forced_ == Forced::None && hasKeyboardFocus(false));
  const auto accent = domainColour(domain_);
  auto r = getLocalBounds().toFloat().reduced(3.0f, 4.0f);
  const float travel = pressed ? 1.0f : 0.0f;  // course physique : la face s'enfonce d'un pixel à l'appui
  const auto lfont = labelFont(tok2::fontLabel, true);
  const auto sfont = labelFont(9.5f, true);
  const auto label = labelHidden_ ? juce::String() : getButtonText();

  if (disabled) {  // commande indisponible : touche à plat, matière éteinte, hachures, aucun témoin
    draw::titaniumKey(g, r, tok2::relief1, false, false, false);
    draw::hatch(g, r.reduced(2.0f), t::panelEdge.withAlpha(0.6f), 6.0f);
    g.setFont(lfont);
    g.setColour(t::textMute);
    g.drawText(label, r.reduced(11.0f, 0.0f), juce::Justification::centredLeft, false);
    g.setFont(sfont);
    g.drawText(stateText(), r.reduced(10.0f, 0.0f), juce::Justification::centredRight, false);
    if (focus) draw::focusRing(g, r.expanded(3.0f), t::radiusSmall + 3.0f);
    return;
  }
  auto in = r.reduced(10.0f, 0.0f).translated(0.0f, travel);
  switch (kind_) {
    case Kind::Action: {  // touche convexe en titane, icône d'impulsion
      draw::titaniumKey(g, r, tok2::titaniumFace, true, pressed, hover);
      if (icon_ != Icon::None) {
        paintIcon(g, in.removeFromLeft(16.0f).withSizeKeepingCentre(16.0f, 14.0f), icon_ == Icon::Authorize ? t::warn : accent);
        in.removeFromLeft(7.0f);
      }
      g.setFont(lfont);
      g.setColour(t::text);
      g.drawText(label, in, juce::Justification::centredLeft, false);
      break;
    }
    case Kind::Toggle: {  // touche plate en titane, témoin encastré profond, état ON / OFF
      draw::titaniumKey(g, r, tok2::relief2, false, pressed, hover);
      const bool on = getToggleState();
      draw::recessedLed(g, {in.getX() + 5.0f, r.getCentreY() + travel}, 4.0f, on ? 2 : 0, accent);
      in.removeFromLeft(19.0f);
      g.setFont(sfont);
      g.setColour(on ? t::text : t::textDim);
      g.drawText(stateText(), in, juce::Justification::centredRight, false);
      g.setFont(lfont);
      g.setColour(t::text);
      g.drawText(label, in.withTrimmedRight(30.0f), juce::Justification::centredLeft, false);
      break;
    }
    case Kind::Arm: {  // armement : canal ambre incrusté dans le titane, témoin anneau (ARMED) ou plein (ACTIVE)
      draw::titaniumKey(g, r, tok2::relief2, false, pressed, hover);
      const auto face = (pressed ? r.translated(0.0f, 1.0f) : r).reduced(3.2f);
      g.setColour(juce::Colours::black.withAlpha(0.6f));
      g.drawRoundedRectangle(face.translated(0.0f, -0.4f), 2.0f, 1.6f);
      g.setColour(t::warn.withAlpha(arm_ == ArmState::Off ? 0.32f : 0.9f));
      g.drawRoundedRectangle(face, 2.0f, arm_ == ArmState::Off ? 0.9f : 1.3f);
      const int mode = arm_ == ArmState::Armed ? 1 : (arm_ == ArmState::Active ? 2 : 0);
      const auto c = juce::Point<float>(in.getX() + 5.0f, r.getCentreY() + travel);
      draw::recessedLed(g, c, 4.0f, mode, t::warn);
      if (arm_ == ArmState::Denied) {
        g.setColour(t::textDim);
        g.drawLine(c.x - 3.0f, c.y - 3.0f, c.x + 3.0f, c.y + 3.0f, 1.3f);
        g.drawLine(c.x - 3.0f, c.y + 3.0f, c.x + 3.0f, c.y - 3.0f, 1.3f);
      }
      in.removeFromLeft(19.0f);
      g.setFont(sfont);
      g.setColour(arm_ == ArmState::Off ? t::textDim : t::warn);
      g.drawText(stateText(), in, juce::Justification::centredRight, false);
      g.setFont(lfont);
      g.setColour(t::text);
      g.drawText(label, in.withTrimmedRight(50.0f), juce::Justification::centredLeft, false);
      break;
    }
    case Kind::Select: {  // sélection exclusive : logement encastré ; option active = fond carbone, losange plein, filet du domaine
      const bool sel = getToggleState() || forced_ == Forced::Pressed;
      draw::machinedGroove(g, r, 3.0f);
      if (sel) {
        g.setColour(t::carbon);
        g.fillRoundedRectangle(r.reduced(2.0f), 2.0f);
        const auto bar = juce::Rectangle<float>(r.getX() + 8.0f, r.getBottom() - 4.4f, r.getWidth() - 16.0f, 2.0f);
        g.setColour(accent.withAlpha(tok2::glowMax * 0.5f));
        g.fillRoundedRectangle(bar.expanded(1.4f), 1.6f);
        g.setColour(accent);
        g.fillRoundedRectangle(bar, 1.0f);
      } else if (hover) {
        g.setColour(tok2::relief1);
        g.fillRoundedRectangle(r.reduced(2.0f), 2.0f);
      }
      const auto c = juce::Point<float>(in.getX() + 4.0f, r.getCentreY() - 0.5f);
      juce::Path d;
      d.addQuadrilateral(c.x, c.y - 4.5f, c.x + 4.5f, c.y, c.x, c.y + 4.5f, c.x - 4.5f, c.y);
      g.setColour(sel ? accent : t::textMute);
      if (sel) g.fillPath(d);
      else g.strokePath(d, juce::PathStrokeType(1.1f));
      in.removeFromLeft(15.0f);
      g.setFont(lfont);
      g.setColour(sel ? t::text : t::textDim);
      g.drawText(label, in.translated(0.0f, -travel - 0.5f), juce::Justification::centredLeft, false);
      break;
    }
    case Kind::Safety: {  // PANIC : rouge de sécurité, chanfrein rouge sombre, libellé CARBONE (LAB-A07.1, >= 4,5:1)
      const float rad = 3.5f;
      g.setColour(tok2::groove);
      g.fillRoundedRectangle(r.expanded(1.6f), rad + 1.4f);
      if (!pressed) {
        g.setColour(juce::Colours::black.withAlpha(0.55f));
        g.fillRoundedRectangle(r.translated(0.0f, 1.8f), rad);
      }
      const auto key = pressed ? r.translated(0.0f, 1.0f).reduced(0.4f, 0.2f) : r;
      juce::ColourGradient cham(pressed ? t::alert.darker(1.2f) : t::alert.brighter(0.30f), key.getX(), key.getY(),
                                pressed ? t::alert.darker(0.2f) : t::alert.darker(1.3f), key.getRight(), key.getBottom(), false);
      g.setGradientFill(cham);
      g.fillRoundedRectangle(key, rad);
      // face bombée : dans la zone du libellé, jamais plus sombre que le rouge nominal (libellé carbone >= 4,5:1) ;
      // l'assombrissement de relief est confiné sous le libellé (au-delà de 72 % de la hauteur)
      const auto f = key.reduced(2.3f);
      juce::ColourGradient fg(t::alert.brighter(pressed ? 0.04f : (hover ? 0.30f : 0.22f)), f.getX(), f.getY(), pressed ? t::alert : t::alert.darker(0.30f),
                              f.getX(), f.getBottom(), false);
      fg.addColour(0.72, t::alert);
      g.setGradientFill(fg);
      g.fillRoundedRectangle(f, rad - 1.0f);
      if (!pressed) {
        juce::ColourGradient spec(juce::Colours::white.withAlpha(0.0f), f.getX(), f.getY(), juce::Colours::white.withAlpha(0.12f), f.getX(), f.getY() + f.getHeight() * 0.2f, false);
        spec.addColour(1.0, juce::Colours::white.withAlpha(0.0f));
        g.setGradientFill(spec);
        g.fillRoundedRectangle(f.withHeight(f.getHeight() * 0.42f), rad - 1.0f);
      }
      g.setColour(t::alert.darker(0.5f));
      g.drawRoundedRectangle(f.reduced(1.2f), rad - 1.5f, 0.9f);
      g.setColour(juce::Colours::white.withAlpha(pressed ? 0.08f : 0.22f));
      g.drawLine(f.getX() + rad, f.getY() + 0.5f, f.getRight() - rad, f.getY() + 0.5f, 1.0f);
      g.setFont(labelFont(tok2::fontCritical + 1.0f, true));
      g.setColour(t::carbon);
      g.drawText(label, key.translated(0.0f, 0.2f), juce::Justification::centred, false);
      break;
    }
  }
  if (hover && !focus) {
    g.setColour(t::textDim.withAlpha(0.6f));
    g.drawRoundedRectangle(r.expanded(2.6f), t::radiusSmall + 2.6f, 1.0f);
  }
  if (focus) draw::focusRing(g, r.expanded(3.0f), t::radiusSmall + 3.0f);
}

}  // namespace sbd::r2
