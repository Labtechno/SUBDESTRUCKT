// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — SYNAPTIC TRACK (implémentation).
#include "r2/R2SynapticTrack.h"

namespace sbd::r2 {

SynapticTrack::SynapticTrack(Mode mode, Domain domain, const juce::String& label, const juce::String& fullName, juce::StringArray options)
    : mode_(mode), domain_(domain), label_(label), options_(std::move(options)) {
  setTitle(fullName);
  setTooltip(fullName);
  setWantsKeyboardFocus(true);
}

void SynapticTrack::setIndex(int k) {
  index_ = juce::jlimit(0, std::max(0, options_.size() - 1), k);
  repaint();
}

juce::Rectangle<float> SynapticTrack::trackArea() const {
  auto r = getLocalBounds().toFloat().reduced(1.0f);
  if (mode_ != Mode::Tabs && label_.isNotEmpty()) r.removeFromTop(16.0f);
  return r;
}

juce::Rectangle<float> SynapticTrack::segment(int k) const {
  const auto r = trackArea();
  if (mode_ == Mode::Tabs) {  // onglets : largeur proportionnelle au libellé
    const auto f = labelFont(tok2::fontLabel, true);
    float total = 0.0f;
    for (auto& s : options_) total += draw::textWidth(f, s) + 24.0f;
    const float scale = total > r.getWidth() ? r.getWidth() / total : 1.0f;
    float x = r.getX();
    for (int i = 0; i < options_.size(); ++i) {
      const float w = (draw::textWidth(f, options_[i]) + 24.0f) * scale;
      if (i == k) return {x, r.getY(), w, r.getHeight()};
      x += w;
    }
    return {};
  }
  const float w = r.getWidth() / static_cast<float>(std::max(1, options_.size()));
  return {r.getX() + w * static_cast<float>(k), r.getY(), w, r.getHeight()};
}

int SynapticTrack::hitSegment(juce::Point<float> p) const {
  if (mode_ == Mode::List) return trackArea().contains(p) ? 0 : -1;
  for (int k = 0; k < options_.size(); ++k)
    if (segment(k).contains(p)) return k;
  return -1;
}

void SynapticTrack::paint(juce::Graphics& g) {
  if (rev_ == Rev::B1) paintB1(g);
  else paintR1(g);
}

// ---------------------------------------------------------------- B1 (Component Lab initial, inchangé)
void SynapticTrack::paintB1(juce::Graphics& g) {
  const bool disabled = forced_ == Forced::Disabled || !isEnabled();
  const bool focus = forced_ == Forced::Focus || (forced_ == Forced::None && hasKeyboardFocus(false));
  const int hov = forced_ == Forced::Hover ? forcedHover_ : hover_;
  const auto accent = domainColour(domain_);
  const auto bold = labelFont(tok2::fontLabel, true);
  const auto plain = labelFont(tok2::fontLabel);

  if (mode_ != Mode::Tabs && label_.isNotEmpty()) {
    g.setFont(plain);
    g.setColour(disabled ? t::textMute : t::text);
    g.drawText(label_, getLocalBounds().toFloat().reduced(3.0f, 0.0f).withHeight(16.0f), juce::Justification::centredLeft, false);
  }
  const auto tr = trackArea();

  if (mode_ == Mode::List) {  // liste : champ optique, valeur courante, rang n/N, chevron
    g.setColour(t::carbon);
    g.fillRoundedRectangle(tr, t::radiusSmall);
    g.setColour(hov == 0 && !disabled ? t::panelEdge.brighter(0.35f) : t::glassEdge);
    g.drawRoundedRectangle(tr.reduced(0.5f), t::radiusSmall, 1.0f);
    g.setColour(disabled ? t::panelEdge : accent);
    g.fillRect(tr.getX() + 1.0f, tr.getY() + 3.0f, 2.0f, tr.getHeight() - 6.0f);
    auto in = tr.reduced(9.0f, 0.0f);
    auto chev = in.removeFromRight(10.0f);
    juce::Path p;
    const auto c = chev.getCentre();
    p.addTriangle(c.x - 4.0f, c.y - 2.0f, c.x + 4.0f, c.y - 2.0f, c.x, c.y + 3.0f);
    g.setColour(disabled ? t::textMute : t::textDim);
    g.fillPath(p);
    const auto rank = juce::String(index_ + 1) + "/" + juce::String(options_.size());
    g.setFont(valueFont(9.5f));
    g.drawText(rank, in.removeFromRight(34.0f), juce::Justification::centredRight, false);
    g.setFont(plain);
    g.setColour(disabled ? t::textMute : t::text);
    g.drawText(options_[index_], in, juce::Justification::centredLeft, true);  // long : points de suspension
    if (focus) draw::focusRing(g, tr.expanded(1.5f), t::radiusSmall + 1.5f);
    return;
  }

  if (mode_ == Mode::Segments) {  // segments : gorge usinée, segment actif en relief avec soulignement
    g.setColour(tok2::groove);
    g.fillRoundedRectangle(tr, t::radiusSmall);
    g.setColour(t::panelEdge);
    g.drawRoundedRectangle(tr.reduced(0.5f), t::radiusSmall, 1.0f);
  }
  for (int k = 0; k < options_.size(); ++k) {
    auto s = segment(k);
    const bool sel = k == index_;
    if (mode_ == Mode::Segments) {
      if (sel) {
        draw::key(g, s.reduced(2.0f), disabled ? tok2::relief1 : tok2::relief3, false, false, false);
        if (!disabled) {
          g.setColour(accent);
          g.fillRect(s.getX() + 6.0f, s.getBottom() - 4.0f, s.getWidth() - 12.0f, 2.0f);
        }
      } else if (k == hov && !disabled) {
        g.setColour(tok2::relief1);
        g.fillRoundedRectangle(s.reduced(2.0f), t::radiusSmall);
      }
      if (k > 0 && !sel && k - 1 != index_) {
        g.setColour(t::panelEdge);
        g.drawVerticalLine(static_cast<int>(s.getX()), s.getY() + 5.0f, s.getBottom() - 5.0f);
      }
    } else {  // onglets : soulignement du domaine, inactif atténué
      if (sel) {
        g.setColour(t::carbon.withAlpha(0.6f));
        g.fillRect(s.reduced(1.0f, 0.0f));
        g.setColour(disabled ? t::textMute : accent);
        g.fillRect(s.getX() + 4.0f, s.getBottom() - 2.5f, s.getWidth() - 8.0f, 2.5f);
      } else if (k == hov && !disabled) {
        g.setColour(t::textMute.withAlpha(0.5f));
        g.fillRect(s.getX() + 4.0f, s.getBottom() - 1.0f, s.getWidth() - 8.0f, 1.0f);
      }
    }
    g.setFont(mode_ == Mode::Tabs || sel ? bold : plain);
    g.setColour(disabled ? t::textMute : (sel ? t::text : (k == hov ? t::text.withAlpha(0.85f) : t::textDim)));
    g.drawText(options_[k], s.reduced(3.0f, 0.0f).translated(0.0f, mode_ == Mode::Segments ? -0.5f : 0.0f), juce::Justification::centred, true);
  }
  if (focus) draw::focusRing(g, (mode_ == Mode::Tabs ? segment(index_) : tr).expanded(1.5f), t::radiusSmall + 1.5f);
}

// ---------------------------------------------------------------- R1 : Machined Track (LAB-A03), plus sobre que NEURAL CELL
void SynapticTrack::paintR1(juce::Graphics& g) {
  const bool disabled = forced_ == Forced::Disabled || !isEnabled();
  const bool focus = forced_ == Forced::Focus || (forced_ == Forced::None && hasKeyboardFocus(false));
  const int hov = forced_ == Forced::Hover ? forcedHover_ : hover_;
  const auto accent = domainColour(domain_);
  const auto bold = labelFont(tok2::fontLabel, true);
  const auto plain = labelFont(tok2::fontLabel);
  if (mode_ != Mode::Tabs && label_.isNotEmpty()) {
    g.setFont(plain);
    g.setColour(disabled ? t::textMute : t::text);
    g.drawText(label_, getLocalBounds().toFloat().reduced(3.0f, 0.0f).withHeight(16.0f), juce::Justification::centredLeft, false);
  }
  const auto tr = trackArea();
  const auto notch = [&g](float x, float y0, float y1) {  // séparation usinée : trait sombre et arête claire
    g.setColour(juce::Colours::black.withAlpha(0.65f));
    g.drawLine(x, y0, x, y1, 1.0f);
    g.setColour(tok2::bevelLight.withAlpha(0.22f));
    g.drawLine(x + 1.0f, y0, x + 1.0f, y1, 1.0f);
  };

  if (mode_ == Mode::List) {  // liste : champ encastré, marqueur du domaine, cran usiné avant le chevron
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
    g.drawText(juce::String(index_ + 1) + "/" + juce::String(options_.size()), in.removeFromRight(30.0f), juce::Justification::centredRight, false);
    g.setFont(plain);
    g.setColour(disabled ? t::textMute : t::text);
    g.drawText(options_[index_], in, juce::Justification::centredLeft, true);
    if (disabled) draw::hatch(g, tr.reduced(2.0f), t::panelEdge.withAlpha(0.5f), 6.0f);
    if (focus) draw::focusRing(g, tr.expanded(1.5f), t::radiusSmall + 1.5f);
    return;
  }
  if (mode_ == Mode::Segments) {  // segments : rail encastré, sélecteur en titane avec marqueur du domaine
    draw::machinedGroove(g, tr, 3.0f);
    for (int k = 0; k < options_.size(); ++k) {
      const auto s = segment(k);
      const bool sel = k == index_;
      if (sel) draw::trackSelector(g, s.reduced(2.5f, 2.5f), accent, !disabled);
      else if (k == hov && !disabled) {
        g.setColour(tok2::relief1);
        g.fillRoundedRectangle(s.reduced(2.5f), 2.5f);
      }
      if (k > 0 && !sel && k - 1 != index_) notch(s.getX(), s.getY() + 6.0f, s.getBottom() - 6.0f);
      g.setFont(sel ? bold : plain);
      g.setColour(disabled ? t::textMute : (sel ? t::text : (k == hov ? t::text.withAlpha(0.85f) : t::textDim)));
      g.drawText(options_[k], s.reduced(3.0f, 0.0f).translated(0.0f, sel ? -1.0f : -0.5f), juce::Justification::centred, true);
    }
    if (disabled) draw::hatch(g, tr.reduced(2.0f), t::panelEdge.withAlpha(0.45f), 6.0f);
    if (focus) draw::focusRing(g, tr.expanded(1.5f), t::radiusSmall + 1.5f);
    return;
  }
  // onglets : rail usiné en pied ; onglet actif = texte clair et marqueur du domaine encastré dans le rail
  const auto rail = tr.withTop(tr.getBottom() - 4.0f);
  draw::machinedGroove(g, rail, 1.5f);
  for (int k = 0; k < options_.size(); ++k) {
    const auto s = segment(k);
    const bool sel = k == index_;
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
    g.drawText(options_[k], s.withTrimmedBottom(4.0f).reduced(3.0f, 0.0f), juce::Justification::centred, true);
  }
  if (focus) draw::focusRing(g, segment(index_).expanded(1.5f), t::radiusSmall + 1.5f);
}

void SynapticTrack::mouseMove(const juce::MouseEvent& e) {
  const int h = hitSegment(e.position);
  if (h != hover_) {
    hover_ = h;
    repaint();
  }
}

void SynapticTrack::mouseExit(const juce::MouseEvent&) {
  hover_ = -1;
  repaint();
}

void SynapticTrack::mouseDown(const juce::MouseEvent& e) {
  if (!isEnabled()) return;
  grabKeyboardFocus();
  int k = hitSegment(e.position);
  if (mode_ == Mode::List) k = k == 0 ? (index_ + 1) % std::max(1, options_.size()) : -1;  // laboratoire : clic = option suivante
  if (k >= 0 && k != index_) {
    setIndex(k);
    if (onChange) onChange(index_);
  }
}

bool SynapticTrack::keyPressed(const juce::KeyPress& key) {
  const int code = key.getKeyCode();
  int k = index_;
  if (code == juce::KeyPress::rightKey || code == juce::KeyPress::downKey) k = std::min(options_.size() - 1, index_ + 1);
  else if (code == juce::KeyPress::leftKey || code == juce::KeyPress::upKey) k = std::max(0, index_ - 1);
  else if (code == juce::KeyPress::homeKey) k = 0;
  else if (code == juce::KeyPress::endKey) k = options_.size() - 1;
  else return false;
  if (k != index_) {
    setIndex(k);
    if (onChange) onChange(index_);
  }
  return true;
}

}  // namespace sbd::r2
