// SUBDESTRUCKT — ARCH-UI-04-R2-B2 — Design System R1 : primitives de dessin (extraites du Component Lab R1).
#include "ui/r2/R2Style.h"

#include <cmath>

namespace sbd::r2 {

juce::Font labelFont(float h, bool bold) {
  return juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), h, bold ? juce::Font::bold : juce::Font::plain));
}
juce::Font valueFont(float h, bool bold) {
  return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), h, bold ? juce::Font::bold : juce::Font::plain));
}

juce::Colour domainColour(Domain d) {
  switch (d) {
    case Domain::Wt: return t::wt;
    case Domain::Fm: return t::fm;
    case Domain::Cross: return t::cross;
    case Domain::Mod: return t::mod;
    case Domain::Alert: return t::alert;
    case Domain::Neutral: break;
  }
  return t::text;
}
const char* domainName(Domain d) {
  switch (d) {
    case Domain::Wt: return "WT";
    case Domain::Fm: return "FM";
    case Domain::Cross: return "CROSS";
    case Domain::Mod: return "MOD";
    case Domain::Alert: return "ALERT";
    case Domain::Neutral: break;
  }
  return "NEUTRAL";
}

namespace draw {

void arc(juce::Graphics& g, juce::Point<float> c, float r, float a0, float a1, float width, juce::Colour col) {
  if (std::abs(a1 - a0) < 1.0e-4f) return;
  juce::Path p;
  p.addCentredArc(c.x, c.y, r, r, 0.0f, std::min(a0, a1), std::max(a0, a1), true);
  g.setColour(col);
  g.strokePath(p, juce::PathStrokeType(width, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
}

void glowArc(juce::Graphics& g, juce::Point<float> c, float r, float a0, float a1, float width, juce::Colour col) {
  if (std::abs(a1 - a0) < 1.0e-4f) return;
  // lueur maîtrisée : deux passes larges à faible opacité (<= glowMax), puis le trait plein
  arc(g, c, r, a0, a1, width * 3.2f, col.withAlpha(tok2::glowMax * 0.28f));
  arc(g, c, r, a0, a1, width * 1.9f, col.withAlpha(tok2::glowMax * 0.55f));
  arc(g, c, r, a0, a1, width, col);
}

void ticks(juce::Graphics& g, juce::Point<float> c, float r0, float r1, float a0, float a1, int n, juce::Colour col, float w) {
  if (n < 2) return;
  g.setColour(col);
  for (int k = 0; k < n; ++k) {
    const float a = a0 + (a1 - a0) * static_cast<float>(k) / static_cast<float>(n - 1);
    const float s = std::sin(a), co = -std::cos(a);
    g.drawLine(c.x + r0 * s, c.y + r0 * co, c.x + r1 * s, c.y + r1 * co, w);
  }
}

void machinedKnob(juce::Graphics& g, juce::Point<float> c, float r, int knurls, bool hover, bool pressed, bool disabled) {
  const auto circle = [c](float rr) { return juce::Rectangle<float>(c.x - rr, c.y - rr, 2.0f * rr, 2.0f * rr); };
  // ombre portée (relief)
  g.setColour(tok2::bevelDark.withAlpha(0.85f));
  g.fillEllipse(circle(r + 1.6f).translated(0.0f, 1.2f));
  if (disabled) {
    g.setColour(tok2::relief1);
    g.fillEllipse(circle(r));
    g.setColour(t::panelEdge);
    g.drawEllipse(circle(r - 0.5f), 1.0f);
    return;
  }
  // périphérie usinée : biseau éclairé en haut à gauche
  juce::ColourGradient rim(tok2::bevelLight.brighter(hover ? 0.25f : 0.0f), c.x - r, c.y - r, tok2::bevelDark, c.x + r, c.y + r, false);
  g.setGradientFill(rim);
  g.fillEllipse(circle(r));
  // moletage (XL : stries fines ; S : aucune strie, biseau seul)
  if (knurls > 0) {
    for (int k = 0; k < knurls; ++k) {
      const float a = juce::MathConstants<float>::twoPi * static_cast<float>(k) / static_cast<float>(knurls);
      const float s = std::sin(a), co = -std::cos(a);
      const float lit = 0.5f + 0.5f * std::cos(a + juce::MathConstants<float>::pi * 0.25f);  // éclairage haut-gauche
      g.setColour((k % 2 == 0 ? tok2::bevelLight : tok2::bevelDark).withAlpha(0.25f + 0.45f * lit));
      g.drawLine(c.x + (r - 0.6f) * s, c.y + (r - 0.6f) * co, c.x + (r * 0.86f) * s, c.y + (r * 0.86f) * co, 1.0f);
    }
  }
  // face du bouton : titane noir, légère concavité
  const float rf = knurls > 0 ? r * 0.84f : r * 0.88f;
  juce::ColourGradient face(pressed ? tok2::relief2.darker(0.25f) : tok2::relief3, c.x - rf * 0.45f, c.y - rf * 0.55f,
                            pressed ? tok2::relief0 : tok2::relief2.darker(0.2f), c.x + rf * 0.6f, c.y + rf * 0.7f, true);
  g.setGradientFill(face);
  g.fillEllipse(circle(rf));
  g.setColour(tok2::bevelDark.withAlpha(0.9f));
  g.drawEllipse(circle(rf), 1.0f);
  g.setColour(tok2::bevelLight.withAlpha(0.35f));
  g.drawEllipse(circle(rf - 1.2f).translated(-0.3f, -0.4f), 0.8f);
}

void key(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour face, bool convex, bool pressed, bool hover) {
  const float rad = t::radiusSmall;
  g.setColour(tok2::bevelDark.withAlpha(0.9f));
  g.fillRoundedRectangle(r.translated(0.0f, pressed ? 0.4f : 1.2f), rad);
  const auto top = convex ? face.brighter(hover ? 0.38f : 0.26f) : face.brighter(hover ? 0.12f : 0.04f);
  const auto bottom = convex ? face.darker(0.45f) : face.darker(0.18f);
  juce::ColourGradient grad(pressed ? bottom : top, r.getX(), r.getY(), pressed ? top : bottom, r.getX(), r.getBottom(), false);
  g.setGradientFill(grad);
  g.fillRoundedRectangle(r, rad);
  // arêtes usinées
  g.setColour((pressed ? tok2::bevelDark : tok2::bevelLight).withAlpha(convex ? 0.8f : 0.45f));
  g.drawLine(r.getX() + rad, r.getY() + 0.6f, r.getRight() - rad, r.getY() + 0.6f, 1.0f);
  g.setColour(tok2::bevelDark.withAlpha(0.9f));
  g.drawRoundedRectangle(r.reduced(0.5f), rad, 1.0f);
}

void hatch(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour col, float spacing) {
  juce::Graphics::ScopedSaveState s(g);
  g.reduceClipRegion(r.toNearestInt());
  g.setColour(col);
  for (float x = r.getX() - r.getHeight(); x < r.getRight(); x += spacing)
    g.drawLine(x, r.getBottom(), x + r.getHeight(), r.getY(), 0.8f);
}

void focusRing(juce::Graphics& g, juce::Rectangle<float> r, float radius) {
  g.setColour(t::focus);
  g.drawRoundedRectangle(r, radius, 1.5f);
}

float textWidth(const juce::Font& f, const juce::String& s) { return juce::GlyphArrangement::getStringWidth(f, s); }

void fittedText(juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r, const juce::Font& f, juce::Colour col,
                juce::Justification j, bool allowTwoLines) {
  g.setFont(f);
  g.setColour(col);
  if (textWidth(f, s) <= r.getWidth() || !allowTwoLines || !s.containsChar(' ')) {
    g.drawText(s, r, j, false);
    return;
  }
  // coupure à l'espace qui minimise la ligne la plus longue
  int best = -1;
  float bestW = 1.0e9f;
  for (int k = s.indexOfChar(' '); k >= 0; k = s.indexOfChar(k + 1, ' ')) {
    const float w = std::max(textWidth(f, s.substring(0, k)), textWidth(f, s.substring(k + 1)));
    if (w < bestW) {
      bestW = w;
      best = k;
    }
  }
  const float lh = f.getHeight();
  auto top = r.withHeight(lh).withCentre({r.getCentreX(), r.getCentreY() - lh * 0.5f});
  g.drawText(s.substring(0, best), top, j, false);
  g.drawText(s.substring(best + 1), top.translated(0.0f, lh), j, false);
}

// ================================================================ primitives R1 (R2-B1-R1)
void titaniumKey(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour face, bool convex, bool pressed, bool hover) {
  const float rad = 3.5f, ch = 2.3f;  // rayon, largeur du chanfrein usiné
  // alvéole usinée dans la plaque : gorge sombre, lèvre basse éclairée
  g.setColour(tok2::groove);
  g.fillRoundedRectangle(r.expanded(2.0f), rad + 1.8f);
  g.setColour(tok2::bevelLight.withAlpha(0.30f));
  g.drawLine(r.getX() + rad, r.getBottom() + 1.7f, r.getRight() - rad, r.getBottom() + 1.7f, 0.8f);
  // course physique : ombre portée au repos, supprimée à l'appui (la touche descend d'un pixel)
  if (!pressed) {
    g.setColour(juce::Colours::black.withAlpha(0.65f));
    g.fillRoundedRectangle(r.translated(0.6f, 2.2f), rad);
  }
  const auto key = pressed ? r.translated(0.0f, 1.0f).reduced(0.4f, 0.2f) : r;
  // chanfrein : éclairage haut-gauche (diagonal), inversé à l'appui
  const auto lit = tok2::bevelLight.brighter(hover ? 0.55f : 0.35f), shade = juce::Colour(0xff030405);
  juce::ColourGradient cham(pressed ? shade : lit, key.getX(), key.getY(), pressed ? tok2::bevelLight.withAlpha(0.7f) : shade, key.getRight(), key.getBottom(), false);
  cham.addColour(0.5, tok2::relief2.darker(0.2f));
  g.setGradientFill(cham);
  g.fillRoundedRectangle(key, rad);
  // face en titane noir : bombée (action) ou en cuvette (bascule, armement)
  const auto f = key.reduced(ch);
  juce::ColourGradient fg(face, f.getX(), f.getY(), face, f.getX(), f.getBottom(), false);
  if (convex) {
    fg = juce::ColourGradient(face.brighter(pressed ? 0.06f : 0.34f), f.getX(), f.getY(), face.darker(pressed ? 0.12f : 0.38f), f.getX(), f.getBottom(), false);
    fg.addColour(0.45, face.brighter(pressed ? 0.0f : 0.06f));
  } else {
    fg = juce::ColourGradient(face.darker(pressed ? 0.30f : 0.22f), f.getX(), f.getY(), face.brighter(pressed ? 0.0f : 0.10f), f.getX(), f.getBottom(), false);
  }
  g.setGradientFill(fg);
  g.fillRoundedRectangle(f, rad - 1.0f);
  {
    juce::Graphics::ScopedSaveState ss(g);
    g.reduceClipRegion(f.toNearestInt());
    g.setColour(juce::Colours::white.withAlpha(0.03f));  // stries d'usinage, très discrètes
    for (float y = f.getY() + 1.5f; y < f.getBottom(); y += 2.0f) g.drawHorizontalLine(static_cast<int>(y), f.getX(), f.getRight());
    if (convex && !pressed) {  // bande spéculaire maîtrisée
      juce::ColourGradient spec(juce::Colours::white.withAlpha(0.0f), f.getX(), f.getY(), juce::Colours::white.withAlpha(0.09f), f.getX(), f.getY() + f.getHeight() * 0.22f, false);
      spec.addColour(1.0, juce::Colours::white.withAlpha(0.0f));
      g.setGradientFill(spec);
      g.fillRect(f.withHeight(f.getHeight() * 0.5f));
    }
    if (!convex) {  // cuvette : ombre interne haute
      g.setColour(juce::Colours::black.withAlpha(0.45f));
      g.drawLine(f.getX() + rad, f.getY() + 0.8f, f.getRight() - rad, f.getY() + 0.8f, 1.4f);
    }
  }
  if (convex) {
    g.setColour((pressed ? tok2::bevelDark : tok2::bevelLight.brighter(0.2f)).withAlpha(0.7f));
    g.drawLine(f.getX() + rad, f.getY() + 0.5f, f.getRight() - rad, f.getY() + 0.5f, 1.0f);
  }
  g.setColour(juce::Colours::black.withAlpha(0.75f));
  g.drawRoundedRectangle(key.reduced(0.5f), rad, 1.0f);
}

void recessedLed(juce::Graphics& g, juce::Point<float> c, float r, int mode, juce::Colour col) {
  const auto circle = [c](float rr) { return juce::Rectangle<float>(c.x - rr, c.y - rr, 2.0f * rr, 2.0f * rr); };
  // logement percé : arête sombre en haut, claire en bas (creux)
  juce::ColourGradient well(tok2::bevelDark, c.x, c.y - r - 2.4f, tok2::bevelLight.withAlpha(0.55f), c.x, c.y + r + 2.4f, false);
  g.setGradientFill(well);
  g.fillEllipse(circle(r + 2.4f));
  g.setColour(tok2::groove);
  g.fillEllipse(circle(r + 1.2f));
  if (mode == 2) {
    g.setColour(col.withAlpha(tok2::glowMax * 0.45f));
    g.fillEllipse(circle(r + 2.2f));
    juce::ColourGradient lens(col.brighter(0.55f), c.x - r * 0.35f, c.y - r * 0.4f, col.darker(0.15f), c.x + r, c.y + r, true);
    g.setGradientFill(lens);
    g.fillEllipse(circle(r));
  } else if (mode == 1) {
    g.setColour(tok2::relief0);
    g.fillEllipse(circle(r));
    g.setColour(col.withAlpha(tok2::glowMax * 0.4f));
    g.drawEllipse(circle(r + 0.4f), 2.6f);
    g.setColour(col);
    g.drawEllipse(circle(r - 0.7f), 1.5f);
  } else {
    juce::ColourGradient lens(tok2::relief2, c.x - r * 0.3f, c.y - r * 0.3f, tok2::groove, c.x + r, c.y + r, true);
    g.setGradientFill(lens);
    g.fillEllipse(circle(r));
  }
  g.setColour(juce::Colours::white.withAlpha(mode == 2 ? 0.45f : 0.12f));  // reflet de lentille
  g.fillEllipse(c.x - r * 0.45f, c.y - r * 0.55f, r * 0.55f, r * 0.4f);
}

void machinedGroove(juce::Graphics& g, juce::Rectangle<float> r, float radius) {
  g.setColour(tok2::groove);
  g.fillRoundedRectangle(r, radius);
  g.setColour(juce::Colours::black.withAlpha(0.7f));  // ombre interne haute : rail encastré
  g.drawLine(r.getX() + radius, r.getY() + 0.8f, r.getRight() - radius, r.getY() + 0.8f, 1.2f);
  g.setColour(tok2::bevelLight.withAlpha(0.28f));  // arête basse éclairée
  g.drawLine(r.getX() + radius, r.getBottom() - 0.5f, r.getRight() - radius, r.getBottom() - 0.5f, 0.8f);
  g.setColour(juce::Colours::black.withAlpha(0.5f));
  g.drawRoundedRectangle(r.reduced(0.5f), radius, 0.8f);
}

void trackSelector(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour marker, bool enabled) {
  // sélecteur en titane : biseau discret (plus sobre qu'une NEURAL CELL), marqueur du domaine encastré en pied
  juce::ColourGradient body(enabled ? tok2::relief3.brighter(0.06f) : tok2::relief1, r.getX(), r.getY(), enabled ? tok2::relief2 : tok2::relief1.darker(0.1f),
                            r.getX(), r.getBottom(), false);
  g.setGradientFill(body);
  g.fillRoundedRectangle(r, 2.5f);
  g.setColour(tok2::bevelLight.withAlpha(enabled ? 0.5f : 0.2f));
  g.drawLine(r.getX() + 2.5f, r.getY() + 0.5f, r.getRight() - 2.5f, r.getY() + 0.5f, 1.0f);
  g.setColour(juce::Colours::black.withAlpha(0.55f));
  g.drawRoundedRectangle(r.reduced(0.4f), 2.5f, 0.8f);
  if (enabled) {
    const auto m = juce::Rectangle<float>(r.getCentreX() - r.getWidth() * 0.22f, r.getBottom() - 3.4f, r.getWidth() * 0.44f, 1.8f);
    g.setColour(marker.withAlpha(tok2::glowMax * 0.5f));
    g.fillRoundedRectangle(m.expanded(1.2f), 1.5f);
    g.setColour(marker);
    g.fillRoundedRectangle(m, 0.9f);
  }
}

juce::Font hudTitleFont(float h) { return labelFont(h, true).withExtraKerningFactor(0.12f); }

void hudFrame(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour accent, bool primary) {
  // monture en titane usiné (chanfrein), joint carbone, verre profond (cœur plus clair), reflet haut, vignettage
  juce::ColourGradient bez(tok2::bevelLight, r.getX(), r.getY(), tok2::bevelDark, r.getX(), r.getBottom(), false);
  g.setGradientFill(bez);
  g.fillRoundedRectangle(r, 5.0f);
  g.setColour(tok2::relief2);
  g.fillRoundedRectangle(r.reduced(1.4f), 4.2f);
  const auto glass = r.reduced(4.0f);
  g.setColour(t::carbon);
  g.fillRoundedRectangle(glass.expanded(1.0f), 3.2f);
  juce::ColourGradient deep(tok2::hudGlassCore, glass.getCentreX(), glass.getCentreY() - glass.getHeight() * 0.1f, tok2::hudGlassDeep,
                            glass.getX(), glass.getY(), true);
  g.setGradientFill(deep);
  g.fillRoundedRectangle(glass, 2.6f);
  {
    juce::Graphics::ScopedSaveState ss(g);
    g.reduceClipRegion(glass.toNearestInt());
    juce::ColourGradient refl(juce::Colours::white.withAlpha(0.045f), glass.getX(), glass.getY(), juce::Colours::white.withAlpha(0.0f), glass.getX(),
                              glass.getY() + glass.getHeight() * 0.32f, false);
    g.setGradientFill(refl);
    g.fillRect(glass.withHeight(glass.getHeight() * 0.32f));
    for (int k = 0; k < 3; ++k) {  // vignettage interne : profondeur optique
      g.setColour(juce::Colours::black.withAlpha(0.18f - 0.05f * static_cast<float>(k)));
      g.drawRoundedRectangle(glass.reduced(0.5f + static_cast<float>(k)), 2.6f, 1.0f);
    }
  }
  // rails gradués d'encadrement (haut et bas) : graduations mineures / majeures
  g.setColour(tok2::hudRail);
  const float x0 = glass.getX() + 14.0f, x1 = glass.getRight() - 14.0f;
  int k = 0;
  for (float x = x0; x <= x1; x += 6.0f, ++k) {
    const float h = (k % 5 == 0) ? 3.0f : 1.6f;
    g.drawLine(x, glass.getY() + 1.0f, x, glass.getY() + 1.0f + h, 0.8f);
    g.drawLine(x, glass.getBottom() - 1.0f - h, x, glass.getBottom() - 1.0f, 0.8f);
  }
  // encadrement technique intérieur (côtés et pied, interrompu au centre) : secondaire
  {
    const auto f = glass.reduced(6.0f);
    g.setColour(tok2::hudRail.withAlpha(0.55f));
    g.drawLine(f.getX(), f.getY() + 18.0f, f.getX(), f.getBottom(), 0.8f);
    g.drawLine(f.getRight(), f.getY() + 18.0f, f.getRight(), f.getBottom(), 0.8f);
    g.drawLine(f.getX(), f.getBottom(), f.getCentreX() - 30.0f, f.getBottom(), 0.8f);
    g.drawLine(f.getCentreX() + 30.0f, f.getBottom(), f.getRight(), f.getBottom(), 0.8f);
  }
  // repères d'angle (secondaires, couleur du domaine)
  const float L = primary ? 11.0f : 8.0f, w = primary ? 1.6f : 1.2f, in = 3.0f;
  const auto q = glass.reduced(in);
  const juce::Point<float> corners[4] = {q.getTopLeft(), q.getTopRight(), q.getBottomLeft(), q.getBottomRight()};
  g.setColour(accent.withAlpha(primary ? 0.9f : 0.6f));
  for (int c = 0; c < 4; ++c) {
    const float sx = (c % 2 == 0) ? 1.0f : -1.0f, sy = (c < 2) ? 1.0f : -1.0f;
    g.drawLine(corners[c].x, corners[c].y, corners[c].x + sx * L, corners[c].y, w);
    g.drawLine(corners[c].x, corners[c].y, corners[c].x, corners[c].y + sy * L, w);
    g.fillRect(corners[c].x + sx * (L + 2.0f) - (sx < 0 ? 2.0f : 0.0f), corners[c].y - (sy < 0 ? 1.0f : 0.0f), 2.0f, 1.0f);
  }
}

float hudChip(juce::Graphics& g, float x, float y, const juce::String& text, juce::Colour c, int shape) {
  // shape : 0 carré plein (BASE), 1 losange (EFF), 2 crochets (MEAS), 3 contour (INDICATIVE, DEMO), 4 point (STATE)
  const auto f = labelFont(8.5f, true).withExtraKerningFactor(0.06f);
  const float tw = textWidth(f, text), w = tw + (shape == 3 ? 8.0f : 15.0f), h = 12.0f;
  const auto r = juce::Rectangle<float>(x, y, w, h);
  g.setColour(t::carbon.withAlpha(0.8f));
  g.fillRoundedRectangle(r, 2.0f);
  g.setColour(c.withAlpha(0.55f));
  g.drawRoundedRectangle(r.reduced(0.5f), 2.0f, 0.8f);
  const auto m = juce::Point<float>(x + 6.0f, y + h * 0.5f);
  g.setColour(c);
  juce::Path p;
  switch (shape) {
    case 0: g.fillRect(m.x - 2.5f, m.y - 2.5f, 5.0f, 5.0f); break;
    case 1:
      p.addQuadrilateral(m.x, m.y - 3.2f, m.x + 3.2f, m.y, m.x, m.y + 3.2f, m.x - 3.2f, m.y);
      g.fillPath(p);
      break;
    case 2:
      g.drawLine(m.x - 2.5f, m.y - 3.0f, m.x - 2.5f, m.y + 3.0f, 1.0f);
      g.drawLine(m.x - 2.5f, m.y - 3.0f, m.x - 1.0f, m.y - 3.0f, 1.0f);
      g.drawLine(m.x - 2.5f, m.y + 3.0f, m.x - 1.0f, m.y + 3.0f, 1.0f);
      g.drawLine(m.x + 2.5f, m.y - 3.0f, m.x + 2.5f, m.y + 3.0f, 1.0f);
      g.drawLine(m.x + 2.5f, m.y - 3.0f, m.x + 1.0f, m.y - 3.0f, 1.0f);
      g.drawLine(m.x + 2.5f, m.y + 3.0f, m.x + 1.0f, m.y + 3.0f, 1.0f);
      break;
    case 4: g.fillEllipse(m.x - 2.2f, m.y - 2.2f, 4.4f, 4.4f); break;
    default: break;
  }
  g.setFont(f);
  g.drawText(text, juce::Rectangle<float>(x + (shape == 3 ? 4.0f : 11.0f), y, tw + 2.0f, h), juce::Justification::centredLeft, false);
  return w;
}

}  // namespace draw
}  // namespace sbd::r2
