// SUBDESTRUCKT — ARCH-UI-04-R2-B1-R1 — Cybernetic HUD (LAB-A04), révision R1 des neuf variantes de CyberDisplay.
// Verre optique profond, monture usinée, repères d'angle, rails gradués, réticules fonctionnels, typographie technique,
// hiérarchie stricte. Nature de chaque donnée signalée par une pastille : BASE (valeur configurée, carré plein du domaine),
// EFF (valeur effective, losange MOD), MEAS (mesure, crochets), INDICATIVE (dérivé non fiable), STATE (état publié).
// Aucune donnée inventée, aucune animation, aucune nouvelle télémétrie ; DEMO DATA dans le laboratoire.
#include <cmath>

#include "r2/R2CyberDisplay.h"

namespace sbd::r2 {

namespace {
juce::String minusSign() { return juce::String::fromUTF8("\xe2\x88\x92"); }
juce::String dbTextR1(float db) {
  if (db <= -119.0f) return "-inf";
  return (db < 0.0f ? minusSign() : juce::String("+")) + juce::String(std::abs(db), 1);
}
float meterNormR1(float db) { return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 66.0f); }

struct Chip {
  juce::String text;
  juce::Colour colour;
  int shape;
};

float chipWidth(const Chip& c) {
  if (c.text.isEmpty()) return 13.0f;  // pastille réduite à sa forme
  const auto f = labelFont(8.5f, true).withExtraKerningFactor(0.06f);
  return draw::textWidth(f, c.text) + (c.shape == 3 ? 8.0f : 15.0f);
}

// graticule instrumental : grille mineure discrète, axes majeurs, graduations sur les axes
void hudGrid(juce::Graphics& g, juce::Rectangle<float> r, int nx, int ny, bool centreAxes) {
  g.setColour(tok2::grid.withAlpha(0.7f));
  for (int i = 1; i < nx; ++i) g.drawVerticalLine(juce::roundToInt(r.getX() + r.getWidth() * static_cast<float>(i) / static_cast<float>(nx)), r.getY(), r.getBottom());
  for (int j = 1; j < ny; ++j) g.drawHorizontalLine(juce::roundToInt(r.getY() + r.getHeight() * static_cast<float>(j) / static_cast<float>(ny)), r.getX(), r.getRight());
  g.setColour(tok2::gridStrong);
  g.drawRect(r, 1.0f);
  if (centreAxes) {
    const float cx = r.getCentreX(), cy = r.getCentreY();
    g.drawHorizontalLine(juce::roundToInt(cy), r.getX(), r.getRight());
    g.drawVerticalLine(juce::roundToInt(cx), r.getY(), r.getBottom());
    for (int i = 0; i <= nx * 5; ++i) {  // graduations mineures sur l'axe horizontal
      const float x = r.getX() + r.getWidth() * static_cast<float>(i) / static_cast<float>(nx * 5);
      g.drawLine(x, cy - (i % 5 == 0 ? 3.0f : 1.5f), x, cy + (i % 5 == 0 ? 3.0f : 1.5f), 0.8f);
    }
    for (int j = 0; j <= ny * 5; ++j) {
      const float y = r.getY() + r.getHeight() * static_cast<float>(j) / static_cast<float>(ny * 5);
      g.drawLine(cx - (j % 5 == 0 ? 3.0f : 1.5f), y, cx + (j % 5 == 0 ? 3.0f : 1.5f), y, 0.8f);
    }
  }
}

void glowPath(juce::Graphics& g, const juce::Path& p, juce::Colour c, float w) {
  g.setColour(c.withAlpha(tok2::glowMax * 0.35f));
  g.strokePath(p, juce::PathStrokeType(w * 3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
  g.setColour(c);
  g.strokePath(p, juce::PathStrokeType(w, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void diamond(juce::Graphics& g, juce::Point<float> p, float d, juce::Colour c) {
  juce::Path dia;
  dia.addQuadrilateral(p.x, p.y - d, p.x + d, p.y, p.x, p.y + d, p.x - d, p.y);
  g.setColour(c);
  g.fillPath(dia);
  g.setColour(t::carbon);
  g.strokePath(dia, juce::PathStrokeType(0.8f));
}

void scaleText(juce::Graphics& g, const juce::String& s, juce::Rectangle<float> r, juce::Justification j) {
  g.setFont(valueFont(8.5f));
  g.setColour(t::textMute);
  g.drawText(s, r, j, false);
}
}  // namespace

namespace {
// BASE d'un paramètre neutre : gris titane clair, jamais le blanc réservé à la mesure (MEAS)
juce::Colour baseColour(Domain d) { return d == Domain::Neutral ? t::textDim : domainColour(d); }
}  // namespace

void CyberDisplay::paintR1(juce::Graphics& g) {
  const auto accent = domainColour(d_);
  auto r = getLocalBounds().toFloat().reduced(1.0f);
  const bool primary = v_ == Variant::Scope || v_ == Variant::Meter || v_ == Variant::Wavetable || v_ == Variant::Numeric;
  draw::hudFrame(g, r, accent, primary);
  auto in = r.reduced(4.0f).reduced(11.0f, 8.0f);
  if (v_ == Variant::Numeric && r.getHeight() < 70.0f) {  // afficheur de barre : titre, valeur, nature, DEMO DATA sur une ligne
    const auto f = draw::hudTitleFont(9.5f);
    g.setFont(f);
    g.setColour(t::textDim);
    g.drawText(data_.title, in.withWidth(48.0f), juce::Justification::centredLeft, false);
    float x = in.getRight();
    if (data_.demo) {
      const Chip c{"DEMO", t::warn, 3};
      x -= chipWidth(c);
      draw::hudChip(g, x, in.getCentreY() - 6.0f, c.text, c.colour, c.shape);
      x -= 4.0f;
    }
    const Chip n = data_.measure ? Chip{"MEAS", tok2::measure, 2} : Chip{"BASE", baseColour(d_), 0};
    x -= chipWidth(n);
    draw::hudChip(g, x, in.getCentreY() - 6.0f, n.text, n.colour, n.shape);
    auto v = juce::Rectangle<float>(in.getX() + 48.0f, in.getY(), x - in.getX() - 52.0f, in.getHeight());
    g.setFont(labelFont(10.0f));
    g.setColour(t::textDim);
    g.drawText(data_.unit, v.removeFromRight(30.0f), juce::Justification::centredLeft, false);
    g.setFont(valueFont(17.0f, true));
    g.setColour(data_.measure ? tok2::measure : t::text);
    g.drawText(data_.value, v, juce::Justification::centredRight, false);
    return;
  }

  // en-tête : onglet du domaine, titre en capitales techniques, pastilles de nature de donnée, DEMO DATA
  auto head = in.removeFromTop(16.0f);
  juce::Path tabP;
  tabP.addQuadrilateral(head.getX(), head.getY() + 3.0f, head.getX() + 7.0f, head.getY() + 3.0f, head.getX() + 4.0f, head.getBottom() - 3.0f,
                        head.getX() - 3.0f, head.getBottom() - 3.0f);
  g.setColour(accent);
  g.fillPath(tabP);
  g.setFont(draw::hudTitleFont(10.5f));
  g.setColour(t::text);
  g.drawText(data_.title, head.withTrimmedLeft(12.0f), juce::Justification::centredLeft, false);
  std::vector<Chip> chips;
  switch (v_) {
    case Variant::Numeric:
      if (data_.measure) chips = {{"MEAS", tok2::measure, 2}};
      else chips = {{"BASE", baseColour(d_), 0}, {"EFF", tok2::effective, 1}};
      break;
    case Variant::Wavetable: chips = {{"BASE", baseColour(d_), 0}, {"EFF", tok2::effective, 1}}; break;
    case Variant::Scope:
    case Variant::Meter: chips = {{"MEAS", tok2::measure, 2}}; break;
    case Variant::Spectrum: chips = {{"INDICATIVE", t::warn, 3}}; break;
    case Variant::ModCurve: chips = {{"BASE", baseColour(d_), 0}, {"EFF", tok2::effective, 1}}; break;
    case Variant::Envelope: chips = {{"BASE", baseColour(d_), 0}, {"LVL", tok2::measure, 4}}; break;
    case Variant::Autonomy:
    case Variant::Diagnostic: chips = {{"STATE", t::textDim, 4}}; break;
  }
  if (data_.demo) chips.push_back({"DEMO DATA", t::warn, 3});
  // place insuffisante : pastilles de nature réduites à leur forme, DEMO DATA abrégé (jamais supprimé)
  {
    float need = draw::textWidth(draw::hudTitleFont(10.5f), data_.title) + 20.0f;
    for (const auto& c : chips) need += chipWidth(c) + 4.0f;
    if (need > head.getWidth())
      for (auto& c : chips) c.text = c.text == "DEMO DATA" ? juce::String("DEMO") : (c.shape == 3 ? c.text : juce::String());
  }
  float x = head.getRight();
  for (auto it = chips.rbegin(); it != chips.rend(); ++it) {
    x -= chipWidth(*it);
    draw::hudChip(g, x, head.getY() + 2.0f, it->text, it->colour, it->shape);
    x -= 4.0f;
  }
  // rail sous l'en-tête
  in.removeFromTop(4.0f);
  g.setColour(tok2::hudRail);
  g.drawHorizontalLine(juce::roundToInt(in.getY()), in.getX(), in.getRight());
  g.fillRect(in.getX(), in.getY() - 1.0f, 18.0f, 2.0f);
  in.removeFromTop(6.0f);
  // pied : source (secondaire, obligatoire)
  auto foot = in.removeFromBottom(12.0f);
  g.setColour(tok2::hudRail);
  g.drawHorizontalLine(juce::roundToInt(foot.getY() - 3.0f), foot.getX(), foot.getRight());
  g.setFont(valueFont(8.5f, true));
  g.setColour(t::textMute);
  g.drawText("SRC", foot.withWidth(24.0f), juce::Justification::centredLeft, false);
  g.setFont(labelFont(9.0f));
  g.setColour(t::textDim);
  g.drawText(data_.source, foot.withTrimmedLeft(26.0f), juce::Justification::centredLeft, true);
  in.removeFromBottom(6.0f);
  switch (v_) {
    case Variant::Numeric: r1Numeric(g, in); break;
    case Variant::Scope: r1Scope(g, in); break;
    case Variant::Spectrum: r1Spectrum(g, in); break;
    case Variant::Wavetable: r1Wavetable(g, in); break;
    case Variant::ModCurve: r1Curve(g, in); break;
    case Variant::Envelope: r1Envelope(g, in); break;
    case Variant::Meter: r1Meter(g, in); break;
    case Variant::Autonomy:
    case Variant::Diagnostic: r1Rows(g, in); break;
  }
}

// 1. valeur numérique : lecture principale, échelle instrumentale, repères BASE (triangle) et EFF (losange)
void CyberDisplay::r1Numeric(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = baseColour(d_);
  const bool compact = r.getHeight() < 44.0f;
  const auto vf = valueFont(compact ? 17.0f : 27.0f, true);
  const float vw = draw::textWidth(vf, data_.value);
  if (compact) {
    g.setFont(vf);
    g.setColour(tok2::measure);
    g.drawText(data_.value, r.withTrimmedRight(34.0f), juce::Justification::centredRight, false);
    g.setFont(labelFont(10.0f));
    g.setColour(t::textDim);
    g.drawText(data_.unit, r.withTrimmedLeft(r.getWidth() - 32.0f), juce::Justification::centredLeft, false);
    return;
  }
  auto top = r.removeFromTop(r.getHeight() * 0.52f);
  auto line = top.withSizeKeepingCentre(vw + 50.0f, 32.0f);
  g.setFont(vf);
  g.setColour(t::text);
  g.drawText(data_.value, line.removeFromLeft(vw), juce::Justification::centredLeft, false);
  g.setFont(labelFont(12.0f));
  g.setColour(t::textDim);
  g.drawText(data_.unit, line.withTrimmedLeft(6.0f), juce::Justification::centredLeft, false);
  // échelle : rail encastré, graduations, bornes
  auto scale = r.removeFromTop(30.0f).reduced(8.0f, 0.0f);
  const auto rail = juce::Rectangle<float>(scale.getX(), scale.getY() + 10.0f, scale.getWidth(), 5.0f);
  g.setColour(tok2::groove);
  g.fillRoundedRectangle(rail, 2.0f);
  g.setColour(tok2::hudRail);
  for (int k = 0; k <= 20; ++k) {
    const float xx = rail.getX() + rail.getWidth() * static_cast<float>(k) / 20.0f;
    g.drawLine(xx, rail.getBottom() + 1.0f, xx, rail.getBottom() + (k % 5 == 0 ? 5.0f : 2.5f), 0.8f);
  }
  if (data_.baseNorm >= 0.0f) {
    const float bx = rail.getX() + rail.getWidth() * data_.baseNorm;
    g.setColour(accent.withAlpha(0.85f));
    g.fillRoundedRectangle(rail.withRight(bx).reduced(0.0f, 1.0f), 1.5f);
    juce::Path tri;
    tri.addTriangle(bx - 4.0f, rail.getY() - 7.0f, bx + 4.0f, rail.getY() - 7.0f, bx, rail.getY() - 1.0f);
    g.setColour(accent);
    g.fillPath(tri);
  }
  if (data_.effNorm >= 0.0f) {
    const float ex = rail.getX() + rail.getWidth() * data_.effNorm;
    diamond(g, {ex, rail.getBottom() + 8.0f}, 3.6f, tok2::effective);
    g.setColour(tok2::effective.withAlpha(0.6f));
    g.drawLine(ex, rail.getY(), ex, rail.getBottom() + 4.0f, 1.0f);
  }
  scaleText(g, data_.minText, scale.withTrimmedTop(18.0f).withWidth(60.0f), juce::Justification::centredLeft);
  scaleText(g, data_.maxText, scale.withTrimmedTop(18.0f).withTrimmedLeft(scale.getWidth() - 60.0f), juce::Justification::centredRight);
  // lecture secondaire : BASE et EFF, chacun avec sa forme
  auto sec = r.withHeight(14.0f).translated(0.0f, 4.0f);
  float x = sec.getCentreX() - 110.0f;
  g.setColour(accent);
  g.fillRect(x, sec.getCentreY() - 2.5f, 5.0f, 5.0f);
  g.setFont(valueFont(10.0f));
  g.setColour(t::textDim);
  g.drawText("BASE " + data_.value + " " + data_.unit, juce::Rectangle<float>(x + 9.0f, sec.getY(), 110.0f, 14.0f), juce::Justification::centredLeft, false);
  if (data_.effText.isNotEmpty()) {
    x += 124.0f;
    diamond(g, {x + 2.5f, sec.getCentreY()}, 3.2f, tok2::effective);
    g.setColour(t::textDim);
    g.drawText("EFF " + data_.effText, juce::Rectangle<float>(x + 9.0f, sec.getY(), 110.0f, 14.0f), juce::Justification::centredLeft, false);
  }
}

// 2. oscilloscope : graticule instrumental, axes gradués, réticule central, trace mesurée
void CyberDisplay::r1Scope(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  auto lab = r.removeFromLeft(20.0f);
  hudGrid(g, r, 10, 4, true);
  scaleText(g, "+1", lab.withHeight(10.0f).translated(0.0f, r.getY() - lab.getY()), juce::Justification::centredRight);
  scaleText(g, "0", lab.withHeight(10.0f).withCentre({lab.getCentreX(), r.getCentreY()}), juce::Justification::centredRight);
  scaleText(g, minusSign() + "1", lab.withHeight(10.0f).withY(r.getBottom() - 10.0f), juce::Justification::centredRight);
  // réticule central
  g.setColour(t::textDim.withAlpha(0.7f));
  g.drawEllipse(r.getCentreX() - 5.0f, r.getCentreY() - 5.0f, 10.0f, 10.0f, 0.9f);
  if (data_.samples.size() < 2) return;
  juce::Path p;
  const auto n = data_.samples.size();
  for (size_t k = 0; k < n; ++k) {
    const float x = r.getX() + r.getWidth() * static_cast<float>(k) / static_cast<float>(n - 1);
    const float y = r.getCentreY() - juce::jlimit(-1.0f, 1.0f, data_.samples[k]) * r.getHeight() * 0.46f;
    if (k == 0) p.startNewSubPath(x, y);
    else p.lineTo(x, y);
  }
  glowPath(g, p, accent, 1.3f);
}

// 3. spectre : INDICATIVE dans toute la bande affichée (décimation sans filtre anti-repliement, C03)
void CyberDisplay::r1Spectrum(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  auto lab = r.removeFromLeft(24.0f);
  auto fl = r.removeFromBottom(11.0f);
  const float fMax = data_.decimatedRate * 0.5f, fMin = 20.0f;
  const auto xOf = [&](float f) { return r.getX() + r.getWidth() * std::log(f / fMin) / std::log(fMax / fMin); };
  const auto yOf = [&](float db) { return r.getY() + r.getHeight() * juce::jlimit(0.0f, 1.0f, -db / 100.0f); };
  g.setColour(tok2::gridStrong);
  g.drawRect(r, 1.0f);
  for (float db = 0.0f; db >= -80.0f; db -= 20.0f) {
    g.setColour(tok2::grid.withAlpha(0.8f));
    g.drawHorizontalLine(juce::roundToInt(yOf(db)), r.getX(), r.getRight());
    scaleText(g, db == 0.0f ? juce::String("0") : minusSign() + juce::String(static_cast<int>(-db)), lab.withHeight(10.0f).withCentre({lab.getCentreX(), yOf(db)}),
              juce::Justification::centredRight);
  }
  for (float f : {50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f}) {
    if (f >= fMax) continue;
    const bool major = f == 100.0f || f == 1000.0f || f == 10000.0f;
    g.setColour(major ? tok2::gridStrong : tok2::grid.withAlpha(0.6f));
    g.drawVerticalLine(juce::roundToInt(xOf(f)), r.getY(), r.getBottom());
    if (major)
      scaleText(g, f >= 1000.0f ? juce::String(static_cast<int>(f / 1000.0f)) + "k" : juce::String(static_cast<int>(f)),
                fl.withWidth(30.0f).withCentre({xOf(f), fl.getCentreY()}), juce::Justification::centred);
  }
  if (fMax >= 10000.0f && xOf(fMax) - xOf(10000.0f) < 40.0f) {
  } else scaleText(g, juce::String(fMax / 1000.0f, 1) + "k", fl.withTrimmedLeft(fl.getWidth() - 34.0f), juce::Justification::centredRight);
  const auto n = data_.spectrumDb.size();
  if (n > 2) {
    juce::Path line, fill;
    bool started = false;
    for (size_t k = 1; k < n; ++k) {
      const float f = fMax * static_cast<float>(k) / static_cast<float>(n - 1);
      if (f < fMin) continue;
      const float x = xOf(f), y = yOf(data_.spectrumDb[k]);
      if (!started) {
        line.startNewSubPath(x, y);
        fill.startNewSubPath(x, r.getBottom());
        fill.lineTo(x, y);
        started = true;
      } else {
        line.lineTo(x, y);
        fill.lineTo(x, y);
      }
    }
    fill.lineTo(r.getRight(), r.getBottom());
    fill.closeSubPath();
    g.setColour(accent.withAlpha(0.10f));
    g.fillPath(fill);
    g.setColour(accent.withAlpha(0.85f));
    g.strokePath(line, juce::PathStrokeType(1.1f));
  }
}

// 4. wavetable : trames en perspective, rail de position X avec repères BASE (triangle) et EFF (losange)
void CyberDisplay::r1Wavetable(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  auto rail = r.removeFromBottom(22.0f).reduced(4.0f, 0.0f);
  const auto nf = data_.frames.size();
  if (nf > 0) {
    const float dx = r.getWidth() * 0.26f / static_cast<float>(nf), dy = r.getHeight() * 0.40f / static_cast<float>(nf);
    const float w = r.getWidth() * 0.70f, h = r.getHeight() * 0.40f;
    const int baseIdx = juce::jlimit(0, static_cast<int>(nf) - 1, static_cast<int>(std::lround(data_.posBase * static_cast<float>(nf - 1))));
    const int effIdx = data_.posEffective ? juce::jlimit(0, static_cast<int>(nf) - 1, static_cast<int>(std::lround(*data_.posEffective * static_cast<float>(nf - 1)))) : -1;
    for (int f = static_cast<int>(nf) - 1; f >= 0; --f) {
      const auto& fr = data_.frames[static_cast<size_t>(f)];
      const float ox = r.getX() + 6.0f + dx * static_cast<float>(f), oy = r.getBottom() - h * 0.5f - 4.0f - dy * static_cast<float>(f);
      juce::Path p;
      for (size_t k = 0; k < fr.size(); ++k) {
        const float px = ox + w * static_cast<float>(k) / static_cast<float>(fr.size() - 1), py = oy - fr[k] * h * 0.5f;
        if (k == 0) p.startNewSubPath(px, py);
        else p.lineTo(px, py);
      }
      if (f == baseIdx) glowPath(g, p, accent, 1.5f);
      else if (f == effIdx) {
        juce::Path dashed;
        const float dash[] = {4.0f, 3.0f};
        juce::PathStrokeType(1.2f).createDashedStroke(dashed, p, dash, 2);
        g.setColour(tok2::effective);
        g.fillPath(dashed);
      } else {
        g.setColour(t::textMute.withAlpha(0.16f + 0.24f * static_cast<float>(nf - 1 - static_cast<size_t>(f)) / static_cast<float>(nf)));
        g.strokePath(p, juce::PathStrokeType(0.75f));
      }
    }
  }
  const auto bar = juce::Rectangle<float>(rail.getX() + 26.0f, rail.getY() + 9.0f, rail.getWidth() - 30.0f, 4.0f);
  scaleText(g, "X", rail.withWidth(20.0f), juce::Justification::centredLeft);
  g.setColour(tok2::groove);
  g.fillRoundedRectangle(bar.expanded(1.0f), 2.0f);
  g.setColour(tok2::gridStrong);
  g.drawRoundedRectangle(bar.expanded(1.0f), 2.0f, 0.8f);
  g.setColour(tok2::hudRail);
  for (int k = 0; k <= 10; ++k) {
    const float xx = bar.getX() + bar.getWidth() * static_cast<float>(k) / 10.0f;
    g.drawLine(xx, bar.getBottom() + 1.0f, xx, bar.getBottom() + (k % 5 == 0 ? 4.5f : 2.5f), 0.8f);
  }
  const float bx = bar.getX() + bar.getWidth() * data_.posBase;
  juce::Path tri;
  tri.addTriangle(bx - 4.0f, bar.getY() - 7.0f, bx + 4.0f, bar.getY() - 7.0f, bx, bar.getY() - 1.0f);
  g.setColour(accent);
  g.fillPath(tri);
  g.setFont(valueFont(9.5f));
  g.drawText(juce::String(data_.posBase, 2), juce::Rectangle<float>(bx + 6.0f, bar.getY() - 12.0f, 40.0f, 10.0f), juce::Justification::centredLeft, false);
  if (data_.posEffective) {
    const float ex = bar.getX() + bar.getWidth() * *data_.posEffective;
    diamond(g, {ex, bar.getCentreY()}, 3.6f, tok2::effective);
    g.setColour(tok2::effective);
    g.drawText(juce::String(*data_.posEffective, 2), juce::Rectangle<float>(ex + 6.0f, bar.getBottom() - 1.0f, 40.0f, 10.0f), juce::Justification::centredLeft, false);
  }
}

// 5. courbe de modulation : axes IN / OUT gradués, référence linéaire, points, réticule d'entrée (EFF)
void CyberDisplay::r1Curve(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  auto sq = r.withSizeKeepingCentre(std::min(r.getWidth() - 30.0f, (r.getHeight() - 12.0f) * 1.5f), r.getHeight() - 12.0f).translated(8.0f, -4.0f);
  hudGrid(g, sq, 4, 4, false);
  for (int k = 0; k <= 20; ++k) {
    const float xx = sq.getX() + sq.getWidth() * static_cast<float>(k) / 20.0f, yy = sq.getBottom() - sq.getHeight() * static_cast<float>(k) / 20.0f;
    g.setColour(tok2::hudRail);
    g.drawLine(xx, sq.getBottom(), xx, sq.getBottom() + (k % 5 == 0 ? 4.0f : 2.0f), 0.8f);
    g.drawLine(sq.getX() - (k % 5 == 0 ? 4.0f : 2.0f), yy, sq.getX(), yy, 0.8f);
  }
  scaleText(g, "IN", juce::Rectangle<float>(sq.getRight() - 20.0f, sq.getBottom() + 3.0f, 20.0f, 10.0f), juce::Justification::centredRight);
  scaleText(g, "OUT", juce::Rectangle<float>(sq.getX() - 26.0f, sq.getY() - 2.0f, 20.0f, 10.0f), juce::Justification::centredRight);
  {
    juce::Path ref, dashed;
    ref.startNewSubPath(sq.getX(), sq.getBottom());
    ref.lineTo(sq.getRight(), sq.getY());
    const float dash[] = {3.0f, 3.0f};
    juce::PathStrokeType(0.8f).createDashedStroke(dashed, ref, dash, 2);
    g.setColour(tok2::gridStrong);
    g.fillPath(dashed);
  }
  if (data_.curve.size() < 2) return;
  juce::Path p;
  for (size_t k = 0; k < data_.curve.size(); ++k) {
    const auto& c = data_.curve[k];
    const float x = sq.getX() + c.x * sq.getWidth(), y = sq.getBottom() - c.y * sq.getHeight();
    if (k == 0) p.startNewSubPath(x, y);
    else p.lineTo(x, y);
  }
  glowPath(g, p, accent, 1.5f);
  for (const auto& c : data_.curve) {
    const float x = sq.getX() + c.x * sq.getWidth(), y = sq.getBottom() - c.y * sq.getHeight();
    g.setColour(t::carbon);
    g.fillRect(x - 3.0f, y - 3.0f, 6.0f, 6.0f);
    g.setColour(t::text);
    g.drawRect(x - 3.0f, y - 3.0f, 6.0f, 6.0f, 1.0f);
  }
  if (data_.curveInput >= 0.0f) {  // entrée courante de la source : réticule vers la sortie effective
    float yv = data_.curve.front().y;
    for (size_t k = 1; k < data_.curve.size(); ++k)
      if (data_.curveInput <= data_.curve[k].x) {
        const auto a = data_.curve[k - 1], b = data_.curve[k];
        yv = a.y + (b.y - a.y) * (data_.curveInput - a.x) / std::max(1.0e-6f, b.x - a.x);
        break;
      }
    const float x = sq.getX() + data_.curveInput * sq.getWidth(), y = sq.getBottom() - yv * sq.getHeight();
    g.setColour(tok2::effective.withAlpha(0.55f));
    g.drawLine(x, sq.getBottom(), x, y, 1.0f);
    g.drawLine(sq.getX(), y, x, y, 1.0f);
    diamond(g, {x, y}, 3.8f, tok2::effective);
  }
}

// 6. enveloppe : forme configurée (BASE), séparateurs d'étapes, niveau publié pour AMP seulement (aucun curseur d'étape)
void CyberDisplay::r1Envelope(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  auto labels = r.removeFromTop(13.0f);
  auto level = r.removeFromRight(18.0f);
  r.removeFromRight(8.0f);
  hudGrid(g, r, 6, 4, false);
  const auto& e = data_.env;
  const float total = e[0] + e[1] + e[2] + e[3] + 0.35f + e[5];
  const auto X = [&](float tt) { return r.getX() + r.getWidth() * tt / total; };
  const auto Y = [&](float v) { return r.getBottom() - v * r.getHeight() * 0.9f; };
  std::array<float, 7> xs{};
  float tt = 0.0f;
  xs[0] = X(tt);
  const float seg[6] = {e[0], e[1], e[2], e[3], 0.35f, e[5]};
  for (int k = 0; k < 6; ++k) {
    tt += seg[k];
    xs[static_cast<size_t>(k + 1)] = X(tt);
  }
  juce::Path p;
  p.startNewSubPath(xs[0], Y(0.0f));
  p.lineTo(xs[1], Y(0.0f));
  p.quadraticTo((xs[1] + xs[2]) * 0.5f, Y(0.85f), xs[2], Y(1.0f));
  p.lineTo(xs[3], Y(1.0f));
  p.quadraticTo((xs[3] + xs[4]) * 0.5f, Y(e[4] * 0.9f), xs[4], Y(e[4]));
  p.lineTo(xs[5], Y(e[4]));
  p.quadraticTo((xs[5] + xs[6]) * 0.5f, Y(e[4] * 0.08f), xs[6], Y(0.0f));
  juce::Path fill(p);
  fill.lineTo(xs[0], Y(0.0f));
  fill.closeSubPath();
  g.setColour(accent.withAlpha(0.09f));
  g.fillPath(fill);
  glowPath(g, p, accent, 1.4f);
  const char* names[6] = {"DLY", "ATK", "HOLD", "DEC", "SUS", "REL"};
  for (int k = 0; k < 6; ++k) {
    const float a = xs[static_cast<size_t>(k)], b = xs[static_cast<size_t>(k + 1)];
    g.setColour(tok2::gridStrong);
    if (k > 0) g.drawVerticalLine(juce::roundToInt(a), labels.getY(), r.getBottom());
    g.setFont(valueFont(8.5f, true));
    g.setColour(t::textDim);
    g.drawText(names[k], juce::Rectangle<float>(a, labels.getY(), std::max(20.0f, b - a), labels.getHeight()), juce::Justification::centred, false);
  }
  {  // niveau de maintien configuré
    juce::Path sus, dashed;
    sus.startNewSubPath(r.getX(), Y(e[4]));
    sus.lineTo(r.getRight(), Y(e[4]));
    const float dash[] = {2.0f, 3.0f};
    juce::PathStrokeType(0.8f).createDashedStroke(dashed, sus, dash, 2);
    g.setColour(accent.withAlpha(0.45f));
    g.fillPath(dashed);
  }
  // niveau instantané publié (AMP) ; sinon logement hachuré
  g.setColour(tok2::groove);
  g.fillRoundedRectangle(level, 2.0f);
  for (int k = 0; k <= 4; ++k) {
    const float yy = level.getBottom() - level.getHeight() * static_cast<float>(k) / 4.0f;
    g.setColour(tok2::hudRail);
    g.drawLine(level.getX() - 3.0f, yy, level.getX(), yy, 0.8f);
  }
  if (data_.envLevel) {
    const float h = level.getHeight() * juce::jlimit(0.0f, 1.0f, *data_.envLevel);
    g.setColour(tok2::measure.withAlpha(0.85f));
    g.fillRoundedRectangle(level.withTop(level.getBottom() - h).reduced(4.0f, 1.0f), 1.5f);
  } else draw::hatch(g, level.reduced(2.0f), t::panelEdge, 4.0f);
}

// 7. crête et vumètre : MESURE, segments neutres, rouge seulement au-delà de 0 dBFS, maintien, lecture principale
void CyberDisplay::r1Meter(juce::Graphics& g, juce::Rectangle<float> r) {
  auto readout = r.removeFromBottom(24.0f);
  auto scale = r.removeFromLeft(26.0f);
  r.removeFromLeft(5.0f);
  const float bw = (r.getWidth() - 8.0f) * 0.5f;
  const juce::Rectangle<float> bars[2] = {r.withWidth(bw), r.withTrimmedLeft(bw + 8.0f)};
  const float peaks[2] = {data_.peakL, data_.peakR}, holds[2] = {data_.holdL, data_.holdR};
  for (float db : {6.0f, 0.0f, -6.0f, -12.0f, -18.0f, -24.0f, -36.0f, -48.0f}) {
    const float y = r.getBottom() - r.getHeight() * meterNormR1(db);
    scaleText(g, db == 0.0f ? juce::String("0") : (db > 0 ? "+" : minusSign()) + juce::String(static_cast<int>(std::abs(db))), scale.withY(y - 5.0f).withHeight(10.0f),
              juce::Justification::centredRight);
    g.setColour(db == 0.0f ? t::textDim : tok2::hudRail);
    g.drawLine(r.getX() - 3.0f, y, db == 0.0f ? r.getRight() : r.getX(), y, db == 0.0f ? 1.0f : 0.8f);
  }
  for (int k = 0; k < 2; ++k) {
    const auto b = bars[k];
    g.setColour(tok2::groove);
    g.fillRoundedRectangle(b, 1.5f);
    const float top = b.getBottom() - b.getHeight() * meterNormR1(peaks[k]);
    const float zero = b.getBottom() - b.getHeight() * meterNormR1(0.0f);
    for (float y = b.getBottom() - 2.0f; y > top; y -= 3.0f) {
      g.setColour(y < zero ? t::alert : tok2::measure.withAlpha(0.62f + 0.33f * (1.0f - (y - b.getY()) / b.getHeight())));
      g.fillRect(b.getX() + 2.0f, y - 2.0f, b.getWidth() - 4.0f, 2.0f);
    }
    const float hy = b.getBottom() - b.getHeight() * meterNormR1(holds[k]);
    g.setColour(holds[k] > 0.0f ? t::alert : t::text);
    g.fillRect(b.getX() + 1.0f, hy - 1.0f, b.getWidth() - 2.0f, 2.0f);
    g.setFont(valueFont(8.5f, true));
    g.setColour(t::textDim);
    g.drawText(k == 0 ? "L" : "R", b.withHeight(11.0f).translated(0.0f, 2.0f), juce::Justification::centred, false);
  }
  const float pk = std::max(data_.peakL, data_.peakR);
  g.setFont(draw::hudTitleFont(9.0f));
  g.setColour(t::textDim);
  g.drawText("PEAK", readout.removeFromLeft(40.0f), juce::Justification::centredLeft, false);
  g.setFont(valueFont(17.0f, true));
  g.setColour(pk > 0.0f ? t::alert : tok2::measure);
  g.drawText(dbTextR1(pk), readout.removeFromLeft(64.0f), juce::Justification::centredLeft, false);
  g.setFont(labelFont(10.0f));
  g.setColour(t::textDim);
  g.drawText("dBFS", readout, juce::Justification::centredLeft, false);
}

// 8 et 9. autonomie et diagnostic : matrice d'états (clé, témoin, valeur) ; STOP, SUSPENDED et signal sur des lignes séparées
void CyberDisplay::r1Rows(juce::Graphics& g, juce::Rectangle<float> r) {
  const float rh = std::max(15.0f, std::min(20.0f, r.getHeight() / static_cast<float>(std::max<size_t>(1, data_.rows.size()))));
  for (const auto& row : data_.rows) {
    auto line = r.removeFromTop(rh);
    const bool primary = row.level == 0;
    auto key = line.removeFromLeft(line.getWidth() * 0.40f);
    if (row.led >= 0) draw::recessedLed(g, {key.getX() + 5.0f, key.getCentreY()}, 3.0f, row.led, row.colour);
    key.removeFromLeft(16.0f);
    g.setFont(primary ? draw::hudTitleFont(9.5f) : labelFont(9.5f));
    g.setColour(primary ? t::textDim : t::textMute);
    g.drawText(row.key, key, juce::Justification::centredLeft, false);
    g.setFont(valueFont(primary ? 11.0f : 10.0f, primary));
    g.setColour(primary ? (row.colour.isTransparent() ? t::text : row.colour) : t::textDim);
    g.drawText(row.value, line, juce::Justification::centredLeft, true);
    g.setColour(tok2::hudRail.withAlpha(0.7f));
    g.drawHorizontalLine(juce::roundToInt(line.getBottom()), r.getX(), r.getRight());
  }
}

}  // namespace sbd::r2
