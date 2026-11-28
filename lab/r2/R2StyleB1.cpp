// SUBDESTRUCKT — Component Lab — primitives B1 et étiquette DEMO DATA (laboratoire uniquement).
#include "r2/R2Style.h"

namespace sbd::r2::draw {

namespace t = sbd::ui::tok;

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
}  // namespace sbd::r2::draw
