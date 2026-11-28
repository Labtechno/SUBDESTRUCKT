// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Cybernetic Display (implémentation).
#include "r2/R2CyberDisplay.h"

#include <cmath>

namespace sbd::r2 {

namespace {
juce::String dbText(float db) {
  if (db <= -119.0f) return "-inf";
  return (db < 0.0f ? juce::String::fromUTF8("\xe2\x88\x92") : juce::String("+")) + juce::String(std::abs(db), 1);
}
float meterNorm(float db) { return juce::jlimit(0.0f, 1.0f, (db + 60.0f) / 66.0f); }  // -60 .. +6 dBFS
}  // namespace

CyberDisplay::CyberDisplay(Variant v, Domain d, DisplayData data) : v_(v), d_(d), data_(std::move(data)) {
  setTitle(data_.title);
}

void CyberDisplay::graticule(juce::Graphics& g, juce::Rectangle<float> r, int nx, int ny) const {
  g.setColour(tok2::grid);
  for (int i = 1; i < nx; ++i) g.drawVerticalLine(static_cast<int>(r.getX() + r.getWidth() * static_cast<float>(i) / static_cast<float>(nx)), r.getY(), r.getBottom());
  for (int j = 1; j < ny; ++j) g.drawHorizontalLine(static_cast<int>(r.getY() + r.getHeight() * static_cast<float>(j) / static_cast<float>(ny)), r.getX(), r.getRight());
  // graduations techniques sur les bords
  g.setColour(tok2::gridStrong);
  for (int i = 0; i <= nx * 4; ++i) {
    const float x = r.getX() + r.getWidth() * static_cast<float>(i) / static_cast<float>(nx * 4);
    g.drawLine(x, r.getBottom() - (i % 4 == 0 ? 4.0f : 2.0f), x, r.getBottom(), 1.0f);
  }
}

void CyberDisplay::tag(juce::Graphics& g, juce::Rectangle<float> area, const juce::String& s, juce::Colour c, bool left) const {
  const auto f = labelFont(8.5f, true);
  const float w = draw::textWidth(f, s) + 8.0f;
  const auto r = juce::Rectangle<float>(left ? area.getX() + 4.0f : area.getRight() - w - 4.0f, area.getY() + 4.0f, w, 12.0f);
  g.setColour(t::carbon.withAlpha(0.85f));
  g.fillRoundedRectangle(r, 2.0f);
  g.setColour(c.withAlpha(0.85f));
  g.drawRoundedRectangle(r.reduced(0.5f), 2.0f, 0.8f);
  g.setFont(f);
  g.drawText(s, r, juce::Justification::centred, false);
}

void CyberDisplay::paint(juce::Graphics& g) {
  if (rev_ == Rev::B1) paintB1(g);
  else paintR1(g);
}

// ---------------------------------------------------------------- B1 (Component Lab initial, inchangé)
void CyberDisplay::paintB1(juce::Graphics& g) {
  const auto accent = domainColour(d_);
  auto r = getLocalBounds().toFloat().reduced(1.0f);
  const bool primary = v_ == Variant::Scope || v_ == Variant::Meter || v_ == Variant::Wavetable || v_ == Variant::Numeric;
  draw::opticalFrame(g, r, accent, primary);
  auto in = r.reduced(8.0f, 6.0f);
  // en-tête : titre principal ; étiquette DEMO DATA ; pied : source (secondaire, obligatoire)
  auto head = in.removeFromTop(16.0f);
  g.setFont(labelFont(10.5f, true));
  g.setColour(t::text);
  g.drawText(data_.title, head.withTrimmedLeft(4.0f), juce::Justification::centredLeft, false);
  g.setColour(accent);
  g.fillRect(head.getX(), head.getY() + 4.0f, 2.0f, head.getHeight() - 8.0f);
  if (data_.demo) draw::demoTag(g, head.withTrimmedTop(-4.0f).withTrimmedRight(-4.0f));
  auto foot = in.removeFromBottom(13.0f);
  g.setFont(labelFont(9.0f));
  g.setColour(t::textDim);
  g.drawText(data_.source, foot, juce::Justification::centredLeft, true);
  in.removeFromTop(3.0f);
  in.removeFromBottom(2.0f);
  switch (v_) {
    case Variant::Numeric: paintNumeric(g, in); break;
    case Variant::Scope: paintScope(g, in); break;
    case Variant::Spectrum: paintSpectrum(g, in); break;
    case Variant::Wavetable: paintWavetable(g, in); break;
    case Variant::ModCurve: paintCurve(g, in); break;
    case Variant::Envelope: paintEnvelope(g, in); break;
    case Variant::Meter: paintMeter(g, in); break;
    case Variant::Autonomy:
    case Variant::Diagnostic: paintRows(g, in); break;
  }
}

void CyberDisplay::paintNumeric(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  const bool compact = r.getHeight() < 44.0f;  // afficheur de barre : valeur seule
  auto sec = compact ? juce::Rectangle<float>() : r.removeFromBottom(14.0f);
  const auto vf = valueFont(compact ? 16.0f : 24.0f);
  const float vw = draw::textWidth(vf, data_.value);
  const auto uf = labelFont(11.0f);
  const float uw = draw::textWidth(uf, data_.unit) + 6.0f;
  auto line = compact ? r.withTrimmedLeft(r.getWidth() - vw - uw - 4.0f) : r.withSizeKeepingCentre(std::min(r.getWidth(), vw + uw), 28.0f);
  const float underY = line.getBottom() + 3.0f;
  g.setFont(vf);
  g.setColour(t::text);
  g.drawText(data_.value, line.removeFromLeft(vw), juce::Justification::centredLeft, false);
  g.setFont(uf);
  g.setColour(t::textDim);
  g.drawText(data_.unit, line.withTrimmedLeft(6.0f), juce::Justification::centredLeft, false);
  if (compact) return;
  // repère de la valeur : trait lumineux maîtrisé sous la valeur
  g.setColour(accent.withAlpha(0.8f));
  g.fillRect(r.getCentreX() - 18.0f, underY, 36.0f, 1.5f);
  g.setFont(valueFont(9.5f));
  g.setColour(t::textDim);
  g.drawText(data_.secondary, sec, juce::Justification::centred, false);
}

void CyberDisplay::paintScope(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  graticule(g, r, 8, 4);
  g.setColour(tok2::gridStrong);
  g.drawHorizontalLine(static_cast<int>(r.getCentreY()), r.getX(), r.getRight());
  if (data_.samples.size() < 2) return;
  juce::Path p;
  const auto n = data_.samples.size();
  for (size_t k = 0; k < n; ++k) {
    const float x = r.getX() + r.getWidth() * static_cast<float>(k) / static_cast<float>(n - 1);
    const float y = r.getCentreY() - juce::jlimit(-1.0f, 1.0f, data_.samples[k]) * r.getHeight() * 0.46f;
    if (k == 0) p.startNewSubPath(x, y);
    else p.lineTo(x, y);
  }
  g.setColour(accent.withAlpha(tok2::glowMax * 0.45f));
  g.strokePath(p, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved));
  g.setColour(accent);
  g.strokePath(p, juce::PathStrokeType(1.3f, juce::PathStrokeType::curved));
}

void CyberDisplay::paintSpectrum(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  const float fMax = data_.decimatedRate * 0.5f, fMin = 20.0f;
  const auto xOf = [&](float f) { return r.getX() + r.getWidth() * std::log(f / fMin) / std::log(fMax / fMin); };
  const auto yOf = [&](float db) { return r.getY() + r.getHeight() * juce::jlimit(0.0f, 1.0f, -db / 100.0f); };
  g.setColour(tok2::grid);
  for (float db = -20.0f; db > -100.0f; db -= 20.0f) g.drawHorizontalLine(static_cast<int>(yOf(db)), r.getX(), r.getRight());
  g.setFont(labelFont(8.5f));
  for (float f : {100.0f, 1000.0f, 10000.0f}) {
    if (f >= fMax) continue;
    g.setColour(tok2::grid);
    g.drawVerticalLine(static_cast<int>(xOf(f)), r.getY(), r.getBottom());
    g.setColour(t::textMute);
    g.drawText(f >= 1000.0f ? juce::String(static_cast<int>(f / 1000.0f)) + "k" : juce::String(static_cast<int>(f)),
               juce::Rectangle<float>(xOf(f) + 2.0f, r.getBottom() - 11.0f, 24.0f, 10.0f), juce::Justification::centredLeft, false);
  }
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
    g.setColour(accent.withAlpha(0.12f));
    g.fillPath(fill);
    g.setColour(accent.withAlpha(0.9f));
    g.strokePath(line, juce::PathStrokeType(1.2f));
  }
  // la décimation sans filtre anti-repliement contamine aussi la bande affichée : spectre indicatif seulement
  tag(g, r, "INDICATIVE", t::warn, true);
  g.setFont(labelFont(8.5f));
  g.setColour(t::textDim);
  g.drawText("band 20 Hz - " + juce::String(fMax / 1000.0f, 1) + " kHz", r.withHeight(12.0f).translated(0.0f, 2.0f).withTrimmedRight(4.0f),
             juce::Justification::centredRight, false);
}

void CyberDisplay::paintWavetable(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  const auto nf = data_.frames.size();
  if (nf == 0) return;
  // pile de trames en perspective : profondeur = position X ; trame Base en couleur du moteur, Effective en losange
  const float dx = r.getWidth() * 0.28f / static_cast<float>(nf), dy = r.getHeight() * 0.42f / static_cast<float>(nf);
  const float w = r.getWidth() * 0.70f, h = r.getHeight() * 0.42f;
  const int baseIdx = juce::jlimit(0, static_cast<int>(nf) - 1, static_cast<int>(std::lround(data_.posBase * static_cast<float>(nf - 1))));
  const int effIdx = data_.posEffective ? juce::jlimit(0, static_cast<int>(nf) - 1, static_cast<int>(std::lround(*data_.posEffective * static_cast<float>(nf - 1)))) : -1;
  for (int f = static_cast<int>(nf) - 1; f >= 0; --f) {
    const auto& fr = data_.frames[static_cast<size_t>(f)];
    const float ox = r.getX() + dx * static_cast<float>(f), oy = r.getBottom() - h * 0.5f - 6.0f - dy * static_cast<float>(f);
    juce::Path p;
    for (size_t k = 0; k < fr.size(); ++k) {
      const float x = ox + w * static_cast<float>(k) / static_cast<float>(fr.size() - 1);
      const float y = oy - fr[k] * h * 0.5f;
      if (k == 0) p.startNewSubPath(x, y);
      else p.lineTo(x, y);
    }
    if (f == baseIdx) {
      g.setColour(accent.withAlpha(tok2::glowMax * 0.45f));
      g.strokePath(p, juce::PathStrokeType(4.0f));
      g.setColour(accent);
      g.strokePath(p, juce::PathStrokeType(1.5f));
    } else if (f == effIdx) {
      juce::Path dashed;
      const float dash[] = {4.0f, 3.0f};
      juce::PathStrokeType(1.2f).createDashedStroke(dashed, p, dash, 2);
      g.setColour(tok2::effective);
      g.fillPath(dashed);
    } else {
      g.setColour(t::textMute.withAlpha(0.22f + 0.25f * static_cast<float>(nf - 1 - static_cast<size_t>(f)) / static_cast<float>(nf)));
      g.strokePath(p, juce::PathStrokeType(0.8f));
    }
  }
  g.setFont(valueFont(9.5f));
  g.setColour(accent);
  g.drawText("X " + juce::String(data_.posBase, 2) + " BASE", r.withHeight(12.0f).withTrimmedLeft(2.0f), juce::Justification::centredLeft, false);
  if (data_.posEffective) {
    g.setColour(tok2::effective);
    g.drawText("EFF " + juce::String(*data_.posEffective, 2), r.withHeight(12.0f).translated(0.0f, 12.0f).withTrimmedLeft(2.0f),
               juce::Justification::centredLeft, false);
  }
}

void CyberDisplay::paintCurve(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  auto sq = r.withSizeKeepingCentre(std::min(r.getWidth(), r.getHeight() * 1.6f), r.getHeight());
  graticule(g, sq, 4, 4);
  g.setColour(tok2::gridStrong);
  g.drawLine(sq.getX(), sq.getBottom(), sq.getRight(), sq.getY(), 1.0f);
  if (data_.curve.size() < 2) return;
  juce::Path p;
  for (size_t k = 0; k < data_.curve.size(); ++k) {
    const auto& c = data_.curve[k];
    const float x = sq.getX() + c.x * sq.getWidth(), y = sq.getBottom() - c.y * sq.getHeight();
    if (k == 0) p.startNewSubPath(x, y);
    else p.lineTo(x, y);
  }
  g.setColour(accent.withAlpha(tok2::glowMax * 0.4f));
  g.strokePath(p, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved));
  g.setColour(accent);
  g.strokePath(p, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved));
  for (const auto& c : data_.curve) {
    const float x = sq.getX() + c.x * sq.getWidth(), y = sq.getBottom() - c.y * sq.getHeight();
    g.setColour(t::carbon);
    g.fillRect(x - 3.0f, y - 3.0f, 6.0f, 6.0f);
    g.setColour(t::text);
    g.drawRect(x - 3.0f, y - 3.0f, 6.0f, 6.0f, 1.0f);
  }
}

void CyberDisplay::paintEnvelope(juce::Graphics& g, juce::Rectangle<float> r) {
  const auto accent = domainColour(d_);
  auto labels = r.removeFromBottom(12.0f);
  auto level = r.removeFromRight(16.0f);
  r.removeFromRight(6.0f);
  graticule(g, r, 6, 4);
  const auto& e = data_.env;
  const float total = e[0] + e[1] + e[2] + e[3] + 0.35f + e[5];  // segment de maintien affiché à largeur fixe
  const auto X = [&](float t) { return r.getX() + r.getWidth() * t / total; };
  const auto Y = [&](float v) { return r.getBottom() - v * r.getHeight() * 0.92f; };
  float tt = 0.0f;
  std::array<float, 7> xs{};
  xs[0] = X(tt);
  tt += e[0]; xs[1] = X(tt);
  tt += e[1]; xs[2] = X(tt);
  tt += e[2]; xs[3] = X(tt);
  tt += e[3]; xs[4] = X(tt);
  tt += 0.35f; xs[5] = X(tt);
  tt += e[5]; xs[6] = X(tt);
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
  g.setColour(accent.withAlpha(0.10f));
  g.fillPath(fill);
  g.setColour(accent);
  g.strokePath(p, juce::PathStrokeType(1.5f));
  g.setColour(tok2::gridStrong);
  for (int k = 1; k < 6; ++k) g.drawVerticalLine(static_cast<int>(xs[static_cast<size_t>(k)]), r.getY(), r.getBottom());
  const char* names[6] = {"DLY", "ATK", "HOLD", "DEC", "SUS", "REL"};
  g.setFont(labelFont(8.5f, true));
  g.setColour(t::textDim);
  for (int k = 0; k < 6; ++k) {
    const float a = xs[static_cast<size_t>(k)], b = xs[static_cast<size_t>(k + 1)];
    g.drawText(names[k], juce::Rectangle<float>(a, labels.getY(), std::max(18.0f, b - a), labels.getHeight()), juce::Justification::centred, false);
  }
  // niveau instantané : publié pour AMP seulement ; l'étape courante n'est pas publiée (aucun curseur d'étape)
  g.setColour(tok2::groove);
  g.fillRoundedRectangle(level, 2.0f);
  if (data_.envLevel) {
    const float h = level.getHeight() * juce::jlimit(0.0f, 1.0f, *data_.envLevel);
    g.setColour(accent.withAlpha(0.85f));
    g.fillRoundedRectangle(level.withTop(level.getBottom() - h).reduced(3.0f, 1.0f), 1.5f);
  } else {
    g.setColour(t::textMute);
    g.drawLine(level.getX() + 3.0f, level.getBottom() - 3.0f, level.getRight() - 3.0f, level.getY() + 3.0f, 1.0f);
  }
}

void CyberDisplay::paintMeter(juce::Graphics& g, juce::Rectangle<float> r) {
  auto readout = r.removeFromBottom(20.0f);
  auto scale = r.removeFromLeft(26.0f);
  r.removeFromLeft(4.0f);
  const float bw = (r.getWidth() - 6.0f) * 0.5f;
  const juce::Rectangle<float> bars[2] = {r.withWidth(bw), r.withTrimmedLeft(bw + 6.0f)};
  const float peaks[2] = {data_.peakL, data_.peakR}, holds[2] = {data_.holdL, data_.holdR};
  g.setFont(labelFont(8.5f));
  for (float db : {6.0f, 0.0f, -6.0f, -12.0f, -24.0f, -48.0f}) {
    const float y = r.getBottom() - r.getHeight() * meterNorm(db);
    g.setColour(db == 0.0f ? t::text : t::textMute);
    g.drawText(db == 0.0f ? juce::String("0") : (db > 0 ? "+" : juce::String::fromUTF8("\xe2\x88\x92")) + juce::String(static_cast<int>(std::abs(db))),
               scale.withY(y - 5.0f).withHeight(10.0f), juce::Justification::centredRight, false);
    g.setColour(db == 0.0f ? t::textDim : tok2::grid);
    g.drawHorizontalLine(static_cast<int>(y), r.getX(), r.getRight());
  }
  for (int k = 0; k < 2; ++k) {
    const auto b = bars[k];
    g.setColour(tok2::groove);
    g.fillRect(b);
    // segments : mesure en blanc optique ; rouge uniquement au-delà de 0 dBFS (alerte)
    const float top = b.getBottom() - b.getHeight() * meterNorm(peaks[k]);
    const float zero = b.getBottom() - b.getHeight() * meterNorm(0.0f);
    for (float y = b.getBottom() - 2.0f; y > top; y -= 3.0f) {
      g.setColour(y < zero ? t::alert : tok2::measure.withAlpha(0.55f + 0.4f * (1.0f - (y - b.getY()) / b.getHeight())));
      g.fillRect(b.getX() + 2.0f, y - 2.0f, b.getWidth() - 4.0f, 2.0f);
    }
    const float hy = b.getBottom() - b.getHeight() * meterNorm(holds[k]);
    g.setColour(holds[k] > 0.0f ? t::alert : t::text);
    g.fillRect(b.getX() + 1.0f, hy - 1.0f, b.getWidth() - 2.0f, 2.0f);
    g.setFont(labelFont(8.5f, true));
    g.setColour(t::textDim);
    g.drawText(k == 0 ? "L" : "R", b.withHeight(11.0f).translated(0.0f, 2.0f), juce::Justification::centred, false);
  }
  const float pk = std::max(data_.peakL, data_.peakR);
  g.setFont(labelFont(9.0f, true));
  g.setColour(t::textDim);
  g.drawText("PEAK", readout.removeFromLeft(36.0f), juce::Justification::centredLeft, false);
  g.setFont(valueFont(13.0f, true));
  g.setColour(pk > 0.0f ? t::alert : tok2::measure);
  g.drawText(dbText(pk) + " dBFS", readout, juce::Justification::centredLeft, false);
}

void CyberDisplay::paintRows(juce::Graphics& g, juce::Rectangle<float> r) {
  const float rh = std::max(14.0f, std::min(19.0f, r.getHeight() / static_cast<float>(std::max<size_t>(1, data_.rows.size()))));
  for (const auto& row : data_.rows) {
    auto line = r.removeFromTop(rh);
    if (row.led >= 0) {
      draw::led(g, {line.getX() + 5.0f, line.getCentreY()}, 3.2f, row.led, row.colour);
      line.removeFromLeft(14.0f);
    }
    const bool primary = row.level == 0;
    g.setFont(labelFont(primary ? 10.5f : 9.5f, primary));
    g.setColour(primary ? t::textDim : t::textMute);
    g.drawText(row.key, line.removeFromLeft(line.getWidth() * 0.46f), juce::Justification::centredLeft, false);
    g.setFont(valueFont(primary ? 11.0f : 10.0f, primary));
    g.setColour(primary ? (row.colour.isTransparent() ? t::text : row.colour) : t::textDim);
    g.drawText(row.value, line, juce::Justification::centredLeft, true);
    g.setColour(tok2::grid);
    g.drawHorizontalLine(static_cast<int>(line.getBottom()), r.getX(), r.getRight());
  }
}

// R1 : peintre HUD partagé (ui/r2/R2Hud.cpp)
void CyberDisplay::paintR1(juce::Graphics& g) { paintHud(g, HudView{v_, d_, data_, getLocalBounds()}); }

}  // namespace sbd::r2
