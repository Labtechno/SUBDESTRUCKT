// SUBDESTRUCKT — ARCH-UI-04-R2-B2 — peintres R1 partagés (TB2-02) : NEXUS, PLASMA, NEURAL CELL, SYNAPTIC TRACK.
// Corps de rendu extraits sans modification du Component Lab R1 (seules les références aux membres deviennent des champs
// de vue). Fonctions pures : aucune écriture de paramètre, aucun geste hôte, aucun état partagé, aucun accès au DSP.
#include "ui/r2/R2Painters.h"

#include <cmath>

namespace sbd::r2 {

namespace {
constexpr float kA0 = -0.75f * juce::MathConstants<float>::pi;  // -135°
constexpr float kA1 = 0.75f * juce::MathConstants<float>::pi;   // +135°

struct Geo {
  float labelPx, valuePx, ring, knobInset;
  int knurls, ticks;
  float arcW, indexW;
};
Geo nexusGeo(NexusSize s) {
  switch (s) {
    case NexusSize::XL: return {tok2::fontTitle, tok2::fontValueXL, 8.0f, 7.0f, 56, 11, 4.0f, 2.6f};
    case NexusSize::S: return {tok2::fontLabel, tok2::fontLabel, 6.0f, 5.5f, 0, 5, 3.4f, 2.2f};
    case NexusSize::Micro: break;
  }
  return {tok2::fontMicro, tok2::fontMicro, 4.0f, 4.5f, 0, 0, 2.8f, 1.8f};
}
}  // namespace

// ================================================================ NEXUS Hybrid (LAB-A01, gelé)
float nexusAngle(float norm) { return kA0 + (kA1 - kA0) * juce::jlimit(0.0f, 1.0f, norm); }
float nexusFamilyLabelPx(NexusSize size) { return nexusGeo(size).labelPx; }
float nexusValuePx(NexusSize size) { return nexusGeo(size).valuePx; }

juce::Rectangle<float> nexusValueArea(juce::Rectangle<int> lb, NexusSize size) {
  const auto G = nexusGeo(size);
  auto r = lb.toFloat();
  return r.removeFromBottom(G.valuePx + 7.0f).reduced(size == NexusSize::Micro ? 3.0f : 6.0f, 1.0f);
}

juce::Rectangle<float> nexusKnobArea(juce::Rectangle<int> lb, NexusSize size, const juce::String& label, float labelPx) {
  const auto G = nexusGeo(size);
  const float lp = labelPx > 0.0f ? labelPx : G.labelPx;
  auto r = lb.toFloat();
  r.removeFromBottom(G.valuePx + 9.0f);
  const bool two = draw::textWidth(labelFont(lp, size == NexusSize::XL), label) > r.getWidth() - 4.0f;
  r.removeFromTop((two ? 2.0f : 1.0f) * std::round(lp * 1.2f) + 4.0f);
  return r;
}

void paintNexus(juce::Graphics& g, juce::Rectangle<int> lb, const NexusView& v) {
  const auto G = nexusGeo(v.size);
  const bool disabled = v.disabled, hover = v.hover, focus = v.focus, pressed = v.pressed;
  const auto accent = domainColour(v.domain);
  const float lp = v.labelPx > 0.0f ? v.labelPx : G.labelPx;

  // sérigraphie
  auto all = lb.toFloat();
  const auto lf = labelFont(lp, v.size == NexusSize::XL);
  const bool two = draw::textWidth(lf, v.label) > all.getWidth() - 4.0f;
  auto labelR = all.withHeight((two ? 2.0f : 1.0f) * std::round(lp * 1.2f) + 4.0f);
  draw::fittedText(g, v.label, labelR.reduced(2.0f, 0.0f), lf, disabled ? t::textMute : (v.inactive ? tok2::inactive : t::text),
                   juce::Justification::centred, true);

  // géométrie du bouton
  const auto ka = nexusKnobArea(lb, v.size, v.label, v.labelPx);
  const auto c = ka.getCentre();
  const float arcR = std::min(ka.getWidth(), ka.getHeight()) * 0.5f - G.ring;
  const float knobR = arcR - G.knobInset;
  const float norm = v.norm;
  const float aV = nexusAngle(norm);
  const float aZero = v.bipolar ? nexusAngle(v.zeroNorm) : kA0;

  if (!disabled) {
    // graduations techniques (XL : 11, S : 5, Micro : aucune) ; centre marqué en bipolaire
    if (G.ticks > 0) draw::ticks(g, c, arcR + 3.0f, arcR + (v.size == NexusSize::XL ? 6.5f : 5.0f), kA0, kA1, G.ticks, t::textMute.withAlpha(0.7f), 1.0f);
    if (v.bipolar) draw::ticks(g, c, arcR + 2.5f, arcR + (v.size == NexusSize::Micro ? 5.0f : 8.0f), aZero, aZero, 2, t::text, 1.4f);
    // piste et arc de Base
    draw::arc(g, c, arcR, kA0, kA1, G.arcW, t::track);
    if (v.size == NexusSize::Micro) draw::arc(g, c, arcR, aZero, aV, G.arcW, accent);
    else draw::glowArc(g, c, arcR, aZero, aV, G.arcW, accent);
    // marques de route (au plus quatre, puis +n ; aucune en Micro)
    const auto depths = v.routeDepths;
    if (v.size != NexusSize::Micro && !depths.empty()) {
      const float rr = arcR + G.ring - 1.0f;
      for (size_t k = 0; k < std::min<size_t>(4, depths.size()); ++k) {
        const float a = nexusAngle(norm + depths[k]);
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
    if (const auto eff = v.effective) {
      const float aE = nexusAngle(*eff);
      const float re = arcR + G.ring - 1.5f;
      draw::arc(g, c, re, std::min(aV, aE), std::max(aV, aE), 1.4f, tok2::effective.withAlpha(0.55f));
      const float s = std::sin(aE), co = -std::cos(aE);
      const float d = v.size == NexusSize::XL ? 4.2f : 3.2f;
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
    const float r0 = knobR * (v.size == NexusSize::XL ? 0.30f : 0.18f), r1 = knobR * (G.knurls > 0 ? 0.80f : 0.86f);
    g.setColour(t::carbon.withAlpha(0.7f));
    g.drawLine(c.x + r0 * s + 0.6f, c.y + r0 * co + 0.8f, c.x + r1 * s + 0.6f, c.y + r1 * co + 0.8f, G.indexW + (pressed ? 0.8f : 0.0f));
    g.setColour(t::text);
    g.drawLine(c.x + r0 * s, c.y + r0 * co, c.x + r1 * s, c.y + r1 * co, G.indexW + (pressed ? 0.8f : 0.0f));
    if (v.size == NexusSize::XL) g.fillEllipse(c.x + r1 * s - 2.2f, c.y + r1 * co - 2.2f, 4.4f, 4.4f);
  }
  if (v.inactive && !disabled) {
    draw::hatch(g, ka.reduced(2.0f), tok2::inactive.withAlpha(0.28f), 4.0f);
    g.setFont(labelFont(8.5f, true));
    g.setColour(tok2::inactive);
    g.drawText("INACTIVE", ka.withHeight(11.0f).withTrimmedRight(2.0f), juce::Justification::centredRight, false);
  }

  // affichage numérique (Base) dans une fenêtre optique
  const auto va = nexusValueArea(lb, v.size);
  g.setColour(t::carbon);
  g.fillRoundedRectangle(va, 2.5f);
  g.setColour(pressed && !disabled ? accent.withAlpha(0.8f) : t::glassEdge);
  g.drawRoundedRectangle(va.reduced(0.5f), 2.5f, pressed ? 1.2f : 0.8f);
  g.setFont(valueFont(G.valuePx));
  g.setColour(disabled ? t::textMute : (pressed ? t::text.brighter(0.2f) : t::text));
  g.drawText(disabled ? juce::String::fromUTF8("\xe2\x80\x94") : (v.entry ? v.valueText + "_" : v.valueText), va,
             juce::Justification::centred, false);
  if (focus) draw::focusRing(g, ka.withSizeKeepingCentre(knobR * 2.0f + 2.0f * G.ring + 6.0f, knobR * 2.0f + 2.0f * G.ring + 6.0f), 6.0f);
}

// ================================================================ PLASMA CHANNEL (LAB-A05, LAB-A07.2)
juce::Rectangle<float> plasmaSlot(juce::Rectangle<int> lb, PlasmaOrientation o, bool compact) {
  auto r = lb.toFloat();
  if (o == PlasmaOrientation::Vertical) {
    r.removeFromTop(30.0f);     // sérigraphie (deux lignes au plus)
    r.removeFromBottom(18.0f);  // valeur
    return juce::Rectangle<float>(r.getX() + (compact ? 11.0f : 10.0f), r.getY() + 6.0f, 8.0f, r.getHeight() - 12.0f);
  }
  r.removeFromLeft(std::round(r.getWidth() * 0.30f));  // légende
  r.removeFromRight(54.0f);                            // valeur
  return juce::Rectangle<float>(r.getX() + 8.0f, r.getY() + 9.0f, r.getWidth() - 16.0f, 8.0f);
}

juce::Point<float> plasmaPosOf(juce::Rectangle<float> s, PlasmaOrientation o, float n) {
  n = juce::jlimit(0.0f, 1.0f, n);
  if (o == PlasmaOrientation::Vertical) return {s.getCentreX(), s.getBottom() - n * s.getHeight()};
  return {s.getX() + n * s.getWidth(), s.getCentreY()};
}

void paintPlasma(juce::Graphics& g, juce::Rectangle<int> lb, const PlasmaView& v) {
  const bool disabled = v.disabled, hover = v.hover, focus = v.focus, pressed = v.pressed;
  const auto accent = domainColour(v.domain);
  const bool V = v.orientation == PlasmaOrientation::Vertical;
  const auto s = plasmaSlot(lb, v.orientation, v.compact);
  auto all = lb.toFloat();

  // sérigraphie et valeur
  const auto lf = labelFont(tok2::fontLabel);
  if (V) draw::fittedText(g, v.label, all.withHeight(30.0f).reduced(1.0f, 0.0f), lf, disabled ? t::textMute : t::text, juce::Justification::centred, true);
  else draw::fittedText(g, v.label, all.withWidth(std::round(all.getWidth() * 0.30f)).reduced(3.0f, 0.0f), lf, disabled ? t::textMute : t::text,
                        juce::Justification::centredLeft, true);
  auto va = V ? all.withTop(all.getBottom() - 17.0f).reduced(2.0f, 1.0f) : all.withLeft(all.getRight() - 52.0f).reduced(1.0f, 10.0f);
  g.setColour(t::carbon);
  g.fillRoundedRectangle(va, 2.0f);
  g.setColour(pressed && !disabled ? accent.withAlpha(0.8f) : t::glassEdge);
  g.drawRoundedRectangle(va.reduced(0.5f), 2.0f, 0.8f);
  g.setFont(valueFont(10.0f));
  g.setColour(disabled ? t::textMute : t::text);
  g.drawText(disabled ? juce::String::fromUTF8("\xe2\x80\x94") : v.valueText, va, juce::Justification::centred, false);

  // gorge usinée et graduations
  g.setColour(tok2::groove);
  g.fillRoundedRectangle(s.expanded(2.0f), 3.0f);
  g.setColour(tok2::bevelLight.withAlpha(0.3f));
  g.drawRoundedRectangle(s.expanded(2.0f).translated(0.0f, 0.6f), 3.0f, 0.7f);
  if (hover && !disabled) {  // survol : contour de la gorge
    g.setColour(t::textDim.withAlpha(0.75f));
    g.drawRoundedRectangle(s.expanded(4.5f), 4.0f, 1.0f);
  }
  const int ng = v.graduations.size();
  g.setFont(labelFont(8.5f));
  for (int k = 0; k < ng; ++k) {
    const float n = ng > 1 ? static_cast<float>(k) / static_cast<float>(ng - 1) : 0.0f;
    const auto p = plasmaPosOf(s, v.orientation, n);
    g.setColour(t::textMute);
    if (V) {
      g.drawLine(s.getRight() + 4.0f, p.y, s.getRight() + 8.0f, p.y, 1.0f);
      // COMPACT : graduations essentielles seulement (bornes et zéro), même corps de texte
      if (v.compact && k != 0 && k != ng - 1 && v.graduations[k] != "0") continue;
      g.drawText(v.graduations[k], juce::Rectangle<float>(s.getRight() + 9.0f, p.y - 5.0f, all.getRight() - s.getRight() - 9.0f, 10.0f),
                 juce::Justification::centredLeft, false);
    } else {
      g.drawLine(p.x, s.getBottom() + 4.0f, p.x, s.getBottom() + 8.0f, 1.0f);
      const float w = 30.0f;
      const float x = k == 0 ? p.x - 2.0f : (k == ng - 1 ? p.x - w + 2.0f : p.x - w * 0.5f);
      g.drawText(v.graduations[k], juce::Rectangle<float>(x, s.getBottom() + 8.0f, w, 10.0f),
                 k == 0 ? juce::Justification::centredLeft : (k == ng - 1 ? juce::Justification::centredRight : juce::Justification::centred), false);
    }
  }
  if (disabled) return;

  // canal lumineux de Base (depuis le minimum, ou depuis le centre en bipolaire)
  const float norm = v.norm;
  const auto p0 = plasmaPosOf(s, v.orientation, v.bipolar ? v.zeroNorm : 0.0f), pv = plasmaPosOf(s, v.orientation, norm);
  auto fill = V ? juce::Rectangle<float>(s.getX(), std::min(p0.y, pv.y), s.getWidth(), std::abs(p0.y - pv.y))
                : juce::Rectangle<float>(std::min(p0.x, pv.x), s.getY(), std::abs(p0.x - pv.x), s.getHeight());
  g.setColour(accent.withAlpha(tok2::glowMax * 0.45f));
  g.fillRoundedRectangle(fill.expanded(2.5f), 3.0f);
  juce::ColourGradient plasma(accent.brighter(0.35f), fill.getX(), fill.getY(), accent.darker(0.25f), V ? fill.getRight() : fill.getX(),
                              V ? fill.getY() : fill.getBottom(), false);
  g.setGradientFill(plasma);
  g.fillRoundedRectangle(fill, 2.0f);
  if (v.bipolar) {
    g.setColour(t::text);
    if (V) g.drawLine(s.getX() - 3.0f, p0.y, s.getRight() + 3.0f, p0.y, 1.2f);
    else g.drawLine(p0.x, s.getY() - 3.0f, p0.x, s.getBottom() + 3.0f, 1.2f);
  }

  // marqueur Effective indépendant (côté opposé aux graduations)
  if (const auto eff = v.effective) {
    const auto pe = plasmaPosOf(s, v.orientation, *eff);
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

// ================================================================ NEURAL CELL Machined Titanium (LAB-A02), PANIC (LAB-A07.1)
void paintCellIcon(juce::Graphics& g, CellIcon icon, juce::Rectangle<float> r, juce::Colour c) {
  juce::Path p;
  const float x = r.getX(), y = r.getY(), w = r.getWidth(), h = r.getHeight();
  switch (icon) {
    case CellIcon::Pulse:  // impulsion : créneau unique (action momentanée)
      p.startNewSubPath(x, y + h * 0.75f);
      p.lineTo(x + w * 0.3f, y + h * 0.75f);
      p.lineTo(x + w * 0.3f, y + h * 0.2f);
      p.lineTo(x + w * 0.7f, y + h * 0.2f);
      p.lineTo(x + w * 0.7f, y + h * 0.75f);
      p.lineTo(x + w, y + h * 0.75f);
      g.setColour(c);
      g.strokePath(p, juce::PathStrokeType(1.4f));
      return;
    case CellIcon::Capture: {  // capture : cadre d'instantané et point mémorisé
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
    case CellIcon::Authorize: {  // autorisation : anneau ouvert et barre (déverrouillage de l'exécution)
      const auto q = r.reduced(w * 0.12f, h * 0.08f);
      g.setColour(c);
      g.drawEllipse(q.getX(), q.getY(), q.getWidth() * 0.62f, q.getHeight() * 0.62f, 1.4f);
      g.drawLine(q.getX() + q.getWidth() * 0.52f, q.getY() + q.getHeight() * 0.52f, q.getRight(), q.getBottom(), 1.6f);
      g.drawLine(q.getRight() - q.getWidth() * 0.22f, q.getBottom() - q.getHeight() * 0.22f, q.getRight() - q.getWidth() * 0.05f,
                 q.getBottom() - q.getHeight() * 0.40f, 1.4f);
      return;
    }
    case CellIcon::None: break;
  }
}

void paintCell(juce::Graphics& g, juce::Rectangle<int> lb, const CellView& v) {
  const bool disabled = v.disabled, hover = v.hover, focus = v.focus, pressed = v.pressed;
  const auto accent = domainColour(v.domain);
  auto r = lb.toFloat().reduced(3.0f, 4.0f);
  const float travel = pressed ? 1.0f : 0.0f;  // course physique : la face s'enfonce d'un pixel à l'appui
  const auto lfont = labelFont(tok2::fontLabel, true);
  const auto sfont = labelFont(9.5f, true);
  const auto label = v.label;

  if (disabled) {  // commande indisponible : touche à plat, matière éteinte, hachures, aucun témoin
    draw::titaniumKey(g, r, tok2::relief1, false, false, false);
    draw::hatch(g, r.reduced(2.0f), t::panelEdge.withAlpha(0.6f), 6.0f);
    g.setFont(lfont);
    g.setColour(t::textMute);
    g.drawText(label, r.reduced(11.0f, 0.0f), juce::Justification::centredLeft, false);
    g.setFont(sfont);
    g.drawText(v.stateText, r.reduced(10.0f, 0.0f), juce::Justification::centredRight, false);
    if (focus) draw::focusRing(g, r.expanded(3.0f), t::radiusSmall + 3.0f);
    return;
  }
  auto in = r.reduced(10.0f, 0.0f).translated(0.0f, travel);
  switch (v.kind) {
    case CellKind::Action: {  // touche convexe en titane, icône d'impulsion
      draw::titaniumKey(g, r, tok2::titaniumFace, true, pressed, hover);
      if (v.icon != CellIcon::None) {
        paintCellIcon(g, v.icon, in.removeFromLeft(16.0f).withSizeKeepingCentre(16.0f, 14.0f), v.icon == CellIcon::Authorize ? t::warn : accent);
        in.removeFromLeft(7.0f);
      }
      g.setFont(lfont);
      g.setColour(t::text);
      g.drawText(label, in, juce::Justification::centredLeft, false);
      break;
    }
    case CellKind::Toggle: {  // touche plate en titane, témoin encastré profond, état ON / OFF
      draw::titaniumKey(g, r, tok2::relief2, false, pressed, hover);
      const bool on = v.on;
      draw::recessedLed(g, {in.getX() + 5.0f, r.getCentreY() + travel}, 4.0f, on ? 2 : 0, accent);
      in.removeFromLeft(19.0f);
      g.setFont(sfont);
      g.setColour(on ? t::text : t::textDim);
      g.drawText(v.stateText, in, juce::Justification::centredRight, false);
      g.setFont(lfont);
      g.setColour(t::text);
      g.drawText(label, in.withTrimmedRight(30.0f), juce::Justification::centredLeft, false);
      break;
    }
    case CellKind::Arm: {  // armement : canal ambre incrusté dans le titane, témoin anneau (ARMED) ou plein (ACTIVE)
      draw::titaniumKey(g, r, tok2::relief2, false, pressed, hover);
      const auto face = (pressed ? r.translated(0.0f, 1.0f) : r).reduced(3.2f);
      g.setColour(juce::Colours::black.withAlpha(0.6f));
      g.drawRoundedRectangle(face.translated(0.0f, -0.4f), 2.0f, 1.6f);
      g.setColour(t::warn.withAlpha(v.arm == CellArm::Off ? 0.32f : 0.9f));
      g.drawRoundedRectangle(face, 2.0f, v.arm == CellArm::Off ? 0.9f : 1.3f);
      const int mode = v.arm == CellArm::Armed ? 1 : (v.arm == CellArm::Active ? 2 : 0);
      const auto c = juce::Point<float>(in.getX() + 5.0f, r.getCentreY() + travel);
      draw::recessedLed(g, c, 4.0f, mode, t::warn);
      if (v.arm == CellArm::Denied) {
        g.setColour(t::textDim);
        g.drawLine(c.x - 3.0f, c.y - 3.0f, c.x + 3.0f, c.y + 3.0f, 1.3f);
        g.drawLine(c.x - 3.0f, c.y + 3.0f, c.x + 3.0f, c.y - 3.0f, 1.3f);
      }
      in.removeFromLeft(19.0f);
      g.setFont(sfont);
      g.setColour(v.arm == CellArm::Off ? t::textDim : t::warn);
      g.drawText(v.stateText, in, juce::Justification::centredRight, false);
      g.setFont(lfont);
      g.setColour(t::text);
      g.drawText(label, in.withTrimmedRight(50.0f), juce::Justification::centredLeft, false);
      break;
    }
    case CellKind::Select: {  // sélection exclusive : logement encastré ; option active = fond carbone, losange plein, filet du domaine
      const bool sel = v.selected;
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
    case CellKind::Safety: {  // PANIC : rouge de sécurité, chanfrein rouge sombre, libellé CARBONE (LAB-A07.1, >= 4,5:1)
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

// ================================================================ SYNAPTIC Machined Track (LAB-A03)
juce::Rectangle<float> trackArea(juce::Rectangle<int> lb, TrackMode mode, bool hasLabel) {
  auto r = lb.toFloat().reduced(1.0f);
  if (mode != TrackMode::Tabs && hasLabel) r.removeFromTop(16.0f);
  return r;
}

juce::Rectangle<float> trackSegment(juce::Rectangle<int> lb, const TrackView& v, int k) {
  const auto r = trackArea(lb, v.mode, v.label.isNotEmpty());
  if (v.mode == TrackMode::Tabs) {  // onglets : largeur proportionnelle au libellé
    const auto f = labelFont(tok2::fontLabel, true);
    float total = 0.0f;
    for (auto& s : v.options) total += draw::textWidth(f, s) + 24.0f;
    const float scale = total > r.getWidth() ? r.getWidth() / total : 1.0f;
    float x = r.getX();
    for (int i = 0; i < v.options.size(); ++i) {
      const float w = (draw::textWidth(f, v.options[i]) + 24.0f) * scale;
      if (i == k) return {x, r.getY(), w, r.getHeight()};
      x += w;
    }
    return {};
  }
  const float w = r.getWidth() / static_cast<float>(std::max(1, v.options.size()));
  return {r.getX() + w * static_cast<float>(k), r.getY(), w, r.getHeight()};
}

void paintTrack(juce::Graphics& g, juce::Rectangle<int> lb, const TrackView& v) {
  const bool disabled = v.disabled, focus = v.focus;
  const int hov = v.hover;
  const auto accent = domainColour(v.domain);
  const auto bold = labelFont(tok2::fontLabel, true);
  const auto plain = labelFont(tok2::fontLabel);
  if (v.mode != TrackMode::Tabs && v.label.isNotEmpty()) {
    g.setFont(plain);
    g.setColour(disabled ? t::textMute : t::text);
    g.drawText(v.label, lb.toFloat().reduced(3.0f, 0.0f).withHeight(16.0f), juce::Justification::centredLeft, false);
  }
  const auto tr = trackArea(lb, v.mode, v.label.isNotEmpty());
  const auto notch = [&g](float x, float y0, float y1) {  // séparation usinée : trait sombre et arête claire
    g.setColour(juce::Colours::black.withAlpha(0.65f));
    g.drawLine(x, y0, x, y1, 1.0f);
    g.setColour(tok2::bevelLight.withAlpha(0.22f));
    g.drawLine(x + 1.0f, y0, x + 1.0f, y1, 1.0f);
  };

  if (v.mode == TrackMode::List) {  // liste : champ encastré, marqueur du domaine, cran usiné avant le chevron
    draw::machinedGroove(g, tr, 3.0f);
    g.setColour(disabled ? t::panelEdge : accent);
    g.fillRoundedRectangle(tr.getX() + 3.0f, tr.getY() + 5.0f, 2.0f, tr.getHeight() - 10.0f, 1.0f);
    if (hov == 0 && !disabled) {
      g.setColour(tok2::relief1.withAlpha(0.6f));
      g.fillRoundedRectangle(tr.reduced(2.0f), 2.0f);
    }
    auto in = tr.reduced(10.0f, 0.0f);
    auto chev = in.removeFromRight(12.0f);
    notch(chev.getX() - 3.0f, tr.getY() + 5.0f, tr.getBottom() - 5.0f);
    juce::Path p;
    const auto c = chev.getCentre().translated(1.0f, 0.0f);
    p.addTriangle(c.x - 3.5f, c.y - 1.8f, c.x + 3.5f, c.y - 1.8f, c.x, c.y + 2.6f);
    g.setColour(disabled ? t::textMute : t::textDim);
    g.fillPath(p);
    in.removeFromRight(6.0f);
    g.setFont(valueFont(9.5f));
    g.drawText(juce::String(v.index + 1) + "/" + juce::String(v.options.size()), in.removeFromRight(30.0f), juce::Justification::centredRight, false);
    g.setFont(plain);
    g.setColour(disabled ? t::textMute : t::text);
    g.drawText(v.options[v.index], in, juce::Justification::centredLeft, true);
    if (disabled) draw::hatch(g, tr.reduced(2.0f), t::panelEdge.withAlpha(0.5f), 6.0f);
    if (focus) draw::focusRing(g, tr.expanded(1.5f), t::radiusSmall + 1.5f);
    return;
  }
  if (v.mode == TrackMode::Segments) {  // segments : rail encastré, sélecteur en titane avec marqueur du domaine
    draw::machinedGroove(g, tr, 3.0f);
    for (int k = 0; k < v.options.size(); ++k) {
      const auto s = trackSegment(lb, v, k);
      const bool sel = k == v.index;
      if (sel) draw::trackSelector(g, s.reduced(2.5f, 2.5f), accent, !disabled);
      else if (k == hov && !disabled) {
        g.setColour(tok2::relief1);
        g.fillRoundedRectangle(s.reduced(2.5f), 2.5f);
      }
      if (k > 0 && !sel && k - 1 != v.index) notch(s.getX(), s.getY() + 6.0f, s.getBottom() - 6.0f);
      g.setFont(sel ? bold : plain);
      g.setColour(disabled ? t::textMute : (sel ? t::text : (k == hov ? t::text.withAlpha(0.85f) : t::textDim)));
      g.drawText(v.options[k], s.reduced(3.0f, 0.0f).translated(0.0f, sel ? -1.0f : -0.5f), juce::Justification::centred, true);
    }
    if (disabled) draw::hatch(g, tr.reduced(2.0f), t::panelEdge.withAlpha(0.45f), 6.0f);
    if (focus) draw::focusRing(g, tr.expanded(1.5f), t::radiusSmall + 1.5f);
    return;
  }
  // onglets : rail usiné en pied ; onglet actif = texte clair et marqueur du domaine encastré dans le rail
  const auto rail = tr.withTop(tr.getBottom() - 4.0f);
  draw::machinedGroove(g, rail, 1.5f);
  for (int k = 0; k < v.options.size(); ++k) {
    const auto s = trackSegment(lb, v, k);
    const bool sel = k == v.index;
    if (sel) {
      g.setColour(t::carbon.withAlpha(0.45f));
      g.fillRoundedRectangle(s.withTrimmedBottom(4.0f).reduced(1.0f, 0.0f), 2.0f);
      const auto m = juce::Rectangle<float>(s.getX() + 5.0f, rail.getY() + 1.0f, s.getWidth() - 10.0f, 2.0f);
      g.setColour((disabled ? t::textMute : accent).withAlpha(tok2::glowMax * 0.5f));
      g.fillRoundedRectangle(m.expanded(1.0f), 1.5f);
      g.setColour(disabled ? t::textMute : accent);
      g.fillRoundedRectangle(m, 1.0f);
    } else if (k == hov && !disabled) {
      g.setColour(t::textMute.withAlpha(0.6f));
      g.fillRect(s.getX() + 8.0f, rail.getY() + 1.5f, s.getWidth() - 16.0f, 1.0f);
    }
    g.setFont(bold);
    g.setColour(disabled ? t::textMute : (sel ? t::text : (k == hov ? t::text.withAlpha(0.85f) : t::textDim)));
    g.drawText(v.options[k], s.withTrimmedBottom(4.0f).reduced(3.0f, 0.0f), juce::Justification::centred, true);
  }
  if (focus) draw::focusRing(g, trackSegment(lb, v, v.index).expanded(1.5f), t::radiusSmall + 1.5f);
}

}  // namespace sbd::r2
