// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab — style R2 (implémentation des primitives).
#include "r2/R2Style.h"

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

void led(juce::Graphics& g, juce::Point<float> c, float r, int mode, juce::Colour col) {
  const auto circle = [c](float rr) { return juce::Rectangle<float>(c.x - rr, c.y - rr, 2.0f * rr, 2.0f * rr); };
  g.setColour(tok2::groove);
  g.fillEllipse(circle(r + 1.8f));
  g.setColour(tok2::bevelLight.withAlpha(0.35f));
  g.drawEllipse(circle(r + 1.8f).translated(0.0f, 0.5f), 0.7f);
  if (mode == 0) {
    g.setColour(tok2::relief1);
    g.fillEllipse(circle(r));
    return;
  }
  if (mode == 1) {
    g.setColour(col.withAlpha(tok2::glowMax * 0.4f));
    g.drawEllipse(circle(r + 0.6f), 3.0f);
    g.setColour(col);
    g.drawEllipse(circle(r - 0.6f), 1.6f);
    return;
  }
  g.setColour(col.withAlpha(tok2::glowMax * 0.5f));
  g.fillEllipse(circle(r + 2.6f));
  juce::ColourGradient core(col.brighter(0.5f), c.x - r * 0.3f, c.y - r * 0.3f, col, c.x + r, c.y + r, true);
  g.setGradientFill(core);
  g.fillEllipse(circle(r));
}

void opticalFrame(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour accent, bool primary) {
  juce::ColourGradient glass(t::glass.brighter(0.04f), r.getX(), r.getY(), t::carbon, r.getX(), r.getBottom(), false);
  g.setGradientFill(glass);
  g.fillRoundedRectangle(r, t::radiusSmall);
  g.setColour(t::glassEdge);
  g.drawRoundedRectangle(r.reduced(0.5f), t::radiusSmall, 1.0f);
  // repères d'angle optiques
  const float L = primary ? 9.0f : 6.0f, w = primary ? 1.4f : 1.0f, in = 3.0f;
  g.setColour(accent.withAlpha(primary ? 0.85f : 0.5f));
  const auto q = r.reduced(in);
  const juce::Point<float> corners[4] = {q.getTopLeft(), q.getTopRight(), q.getBottomLeft(), q.getBottomRight()};
  for (int k = 0; k < 4; ++k) {
    const float sx = (k % 2 == 0) ? 1.0f : -1.0f, sy = (k < 2) ? 1.0f : -1.0f;
    g.drawLine(corners[k].x, corners[k].y, corners[k].x + sx * L, corners[k].y, w);
    g.drawLine(corners[k].x, corners[k].y, corners[k].x, corners[k].y + sy * L, w);
  }
}

void demoTag(juce::Graphics& g, juce::Rectangle<float> area) {
  const auto f = labelFont(8.5f, true);
  const juce::String s("DEMO DATA");
  const float w = textWidth(f, s) + 8.0f;
  auto r = juce::Rectangle<float>(area.getRight() - w - 4.0f, area.getY() + 4.0f, w, 12.0f);
  g.setColour(t::carbon.withAlpha(0.85f));
  g.fillRoundedRectangle(r, 2.0f);
  g.setColour(t::warn.withAlpha(0.8f));
  g.drawRoundedRectangle(r.reduced(0.5f), 2.0f, 0.8f);
  g.setFont(f);
  g.drawText(s, r, juce::Justification::centred, false);
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

}  // namespace draw
}  // namespace sbd::r2
