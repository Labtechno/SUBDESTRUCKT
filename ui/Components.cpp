#include "Components.h"

#include <cmath>
#include <cstring>

#include "core/StateCodec.h"

namespace sbd::ui {

// ================================================================ tokens et typographie
juce::Colour accentOf(Domain3 d) {
  switch (d) {
    case Domain3::Wt: return tok::wt;
    case Domain3::Fm: return tok::fm;
    case Domain3::Cross: return tok::cross;
    case Domain3::Mod: return tok::mod;
    case Domain3::Alert: return tok::alert;
    case Domain3::Neutral: break;
  }
  return tok::text;
}

juce::Font uiFont(float h, bool bold) {
  return juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), h, bold ? juce::Font::bold : juce::Font::plain));
}
juce::Font numFont(float h, bool bold) {
  return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), h, bold ? juce::Font::bold : juce::Font::plain));
}

// ================================================================ registre
Domain3 domainOf(ParamId id) {
  const char* g = paramSpec(id).group;
  auto starts = [g](const char* p) { return std::strncmp(g, p, std::strlen(p)) == 0; };
  if (std::strcmp(g, "wt") == 0) return Domain3::Wt;
  if (starts("op") || starts("fm")) return Domain3::Fm;
  if (std::strcmp(g, "cross") == 0 || std::strcmp(g, "perf") == 0) return Domain3::Cross;
  if (starts("env") || starts("lfo") || std::strcmp(g, "random") == 0 || std::strcmp(g, "follower") == 0 || std::strcmp(g, "macro") == 0)
    return Domain3::Mod;
  return Domain3::Neutral;
}

std::string shortLabel(ParamId id) {
  static const std::set<std::string> prefixes{"Amp", "ENV", "WT", "FM", "AUX1", "AUX2", "CROSS", "FLT", "GLOBAL", "DST", "WT1", "WT2",
                                              "FM1", "FM2", "GL1", "GL2", "N1", "D1", "Y1", "S1", "N2", "D2", "Y2", "S2", "OP1", "OP2",
                                              "OP3", "OP4", "LFO1", "LFO2", "LFO3", "LFO4", "Follower1", "Follower2", "Follower3"};
  juce::StringArray t;
  t.addTokens(paramSpec(id).name, " ", "");
  while (t.size() > 1 && prefixes.count(t[0].toStdString()) > 0) t.remove(0);
  return t.joinIntoString(" ").toStdString();
}

std::vector<ParamId> range(ParamId first, ParamId last) {
  std::vector<ParamId> v;
  for (int i = idx(first); i <= idx(last); ++i) v.push_back(static_cast<ParamId>(i));
  return v;
}

std::vector<ParamId> filterOffsets(ParamId base, const std::vector<int>& offsets) {
  std::vector<ParamId> v;
  for (int o : offsets) v.push_back(static_cast<ParamId>(idx(base) + o));
  return v;
}

bool parseValueText(ParamId id, const juce::String& text, float& out) {
  const auto& s = paramSpec(id);
  juce::String t = text.trim().replaceCharacter(',', '.');
  if (t.isEmpty()) return false;
  if (s.type == ParamType::Choice) {
    for (int k = 0; k < choiceCount(id); ++k)
      if (t.equalsIgnoreCase(juce::String(choiceLabel(id, k)))) {
        out = static_cast<float>(k);
        return true;
      }
  }
  if (s.type == ParamType::Bool) {
    if (t.equalsIgnoreCase("on") || t == "1") return out = 1.0f, true;
    if (t.equalsIgnoreCase("off") || t == "0") return out = 0.0f, true;
    return false;
  }
  const juce::String num = t.initialSectionContainingOnly("+-0123456789.eE");
  if (num.isEmpty() || num == "-" || num == "+" || num == ".") return false;
  double v = num.getDoubleValue();
  const juce::String rest = t.substring(num.length()).trim();
  if (rest.startsWithIgnoreCase("k") && !juce::String(s.unit).startsWithIgnoreCase("k")) v *= 1000.0;
  if (std::strcmp(s.unit, "s") == 0 && rest.startsWithIgnoreCase("ms")) v *= 0.001;
  if (std::strcmp(s.unit, "ms") == 0 && rest.equalsIgnoreCase("s")) v *= 1000.0;
  if (!std::isfinite(v)) return false;
  out = sanitize(id, static_cast<float>(v));
  return true;
}

// ================================================================ libellés sans troncature
bool layoutLabel(const juce::String& text, float width, int maxLines, float preferredSize, juce::StringArray& lines, float& size) {
  juce::StringArray words;
  words.addTokens(text, " ", "");
  for (float sz = preferredSize; sz >= tok::fontMicro - 0.01f; sz -= 0.5f) {
    const auto f = uiFont(sz);
    lines.clear();
    juce::String cur;
    bool wordTooWide = false;
    for (const auto& w : words) {
      if (juce::GlyphArrangement::getStringWidth(f, w) > width) wordTooWide = true;
      const juce::String cand = cur.isEmpty() ? w : cur + " " + w;
      if (cur.isNotEmpty() && juce::GlyphArrangement::getStringWidth(f, cand) > width) {
        lines.add(cur);
        cur = w;
      } else {
        cur = cand;
      }
    }
    if (cur.isNotEmpty()) lines.add(cur);
    size = sz;
    if (!wordTooWide && lines.size() <= maxLines) return true;
  }
  return false;
}

void drawLabel(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area, int maxLines, float size, juce::Colour c,
               juce::Justification j) {
  juce::StringArray lines;
  float sz = size;
  layoutLabel(text, area.getWidth(), maxLines, size, lines, sz);
  g.setColour(c);
  g.setFont(uiFont(sz));
  const float lh = sz * 1.18f;
  float y = area.getY();
  if (j.testFlags(juce::Justification::verticallyCentred)) y = area.getCentreY() - lh * static_cast<float>(lines.size()) * 0.5f;
  const auto hj = j.getOnlyHorizontalFlags() | juce::Justification::verticallyCentred;
  for (int i = 0; i < lines.size() && i < maxLines; ++i) {
    g.drawText(lines[i], juce::Rectangle<float>(area.getX(), y, area.getWidth(), lh), hj, false);
    y += lh;
  }
}

// ================================================================ liaison
BoundControl::BoundControl(UiContext& ctx) : ctx_(ctx) { ctx_.registry.push_back(this); }
BoundControl::~BoundControl() {
  auto& r = ctx_.registry;
  r.erase(std::remove(r.begin(), r.end(), this), r.end());
}

ParamLink::ParamLink(SubdestrucktProcessor& p, ParamId id, std::function<void()> onChange)
    : proc_(p), id_(id), param_(*p.parameters().param(idx(id))) {
  att_ = std::make_unique<juce::ParameterAttachment>(param_, [cb = std::move(onChange)](float) {
    if (cb) cb();
  });
}

float ParamLink::base() const { return proc_.parameters().base(idx(id_)); }

// ================================================================ NEXUS / PLASMA
namespace {
bool bipolar(ParamId id) {
  const auto& s = paramSpec(id);
  return s.minValue < 0.0f && s.maxValue > 0.0f;
}
float valueFont(Density d) { return d == Density::XL ? tok::fontXL : (d == Density::Micro ? 10.0f : tok::fontValue); }
}  // namespace

class NexusAccessibleValue : public juce::AccessibilityRangedNumericValueInterface {
 public:
  explicit NexusAccessibleValue(ContinuousControl& c) : c_(c) {}
  bool isReadOnly() const override { return false; }
  double getCurrentValue() const override { return c_.shown_; }
  void setValue(double v) override { c_.userSet(static_cast<float>(v)); }
  AccessibleValueRange getRange() const override {
    const auto& s = paramSpec(c_.id_);
    return {{s.minValue, s.maxValue}, s.type == ParamType::Int ? 1.0 : (s.maxValue - s.minValue) * tok::keyStep};
  }

 private:
  ContinuousControl& c_;
};

ContinuousControl::ContinuousControl(UiContext& ctx, ParamId id, Form form, Density d)
    : BoundControl(ctx), id_(id), form_(form), density_(d), link_(ctx.proc, id, [this] {
        shown_ = link_.base();
        repaint();
      }) {
  const auto& s = paramSpec(id);
  caption_ = shortLabel(id);
  shown_ = link_.base();
  setTitle(s.name);
  setHelpText(s.stableId);
  setDescription(juce::String(s.name) + " (" + s.stableId + ")");
  setWantsKeyboardFocus(true);
  setRepaintsOnMouseActivity(true);
  setTooltip(juce::String(s.name) + "\n" + s.stableId + juce::String::fromUTF8("  |  glisser : réglage, Maj : fin, double-clic : défaut, Entrée : saisie"));
}

ContinuousControl::~ContinuousControl() {
  if (dragging_ || wheelGesture_) link_.end();
}

void ContinuousControl::setInactive(bool inactive, const juce::String& why) {
  if (inactive == inactive_) return;
  inactive_ = inactive;
  const auto& s = paramSpec(id_);
  const juce::String nl = juce::newLine;
  setTooltip(juce::String(s.name) + nl + s.stableId + (inactive ? nl + "INACTIF : " + why : juce::String()));
  repaint();
}

void ContinuousControl::setCaption(const juce::String& c) {
  caption_ = c;
  repaint();
}

int ContinuousControl::labelLines() const { return form_ == Form::LinearH ? 1 : 2; }
float ContinuousControl::labelSize() const { return density_ == Density::XL ? tok::fontTitle : (density_ == Density::Micro ? 10.0f : tok::fontLabel); }

juce::Rectangle<float> ContinuousControl::labelArea() const {
  auto r = getLocalBounds().toFloat().reduced(2.0f);
  if (form_ == Form::LinearH) return r.removeFromLeft(r.getWidth() * 0.34f);
  const float lh = labelSize() * 1.18f * static_cast<float>(labelLines());
  return r.removeFromTop(lh);
}
juce::Rectangle<float> ContinuousControl::valueArea() const {
  auto r = getLocalBounds().toFloat().reduced(2.0f);
  const float vh = valueFont(density_) + 5.0f;
  if (form_ == Form::LinearH) {
    r.removeFromLeft(r.getWidth() * 0.34f);
    return r.removeFromRight(r.getWidth() * 0.36f);
  }
  return r.removeFromBottom(vh);
}
juce::Rectangle<float> ContinuousControl::knobArea() const {
  auto r = getLocalBounds().toFloat().reduced(2.0f);
  if (form_ == Form::LinearH) {
    r.removeFromLeft(r.getWidth() * 0.34f);
    r.removeFromRight(r.getWidth() * 0.36f);
    return r.reduced(4.0f, 0.0f);
  }
  r.removeFromTop(labelArea().getHeight() + 1.0f);
  r.removeFromBottom(valueArea().getHeight() + 1.0f);
  return r;
}

bool ContinuousControl::textFits() const {
  juce::StringArray lines;
  float sz = 0.0f;
  const auto la = labelArea();
  if (!layoutLabel(caption_, la.getWidth(), labelLines(), labelSize(), lines, sz)) return false;
  const auto& s = paramSpec(id_);
  const auto f = numFont(valueFont(density_));
  const float vw = valueArea().getWidth();
  for (float v : {s.minValue, s.maxValue, s.defaultValue, shown_})
    if (juce::GlyphArrangement::getStringWidth(f, juce::String(formatValue(id_, v))) > vw) return false;
  return true;
}

void ContinuousControl::paint(juce::Graphics& g) {
  const auto accent = accentOf(domainOf(id_));
  const bool on = isEnabled() && !inactive_;
  drawLabel(g, caption_, labelArea(), labelLines(), labelSize(), on ? tok::text : tok::textMute,
            form_ == Form::LinearH ? juce::Justification::centredLeft : juce::Justification::centredTop);
  if (form_ == Form::Rotary) paintRotary(g, knobArea());
  else paintLinear(g, knobArea());
  // valeur Base (texte + unité) ; Effective en second texte si modulée
  const auto va = valueArea();
  g.setColour(tok::glass);
  g.fillRoundedRectangle(va, tok::radiusSmall);
  g.setColour(hasKeyboardFocus(false) ? tok::focus : tok::glassEdge);
  g.drawRoundedRectangle(va.reduced(0.5f), tok::radiusSmall, hasKeyboardFocus(false) ? 1.4f : 1.0f);
  g.setColour(on ? tok::text : tok::textMute);
  g.setFont(numFont(valueFont(density_)));
  g.drawText(juce::String(formatValue(id_, shown_)), va, juce::Justification::centred, false);
  // marqueurs : modulations globales, MIDI Learn (texte, jamais seulement la couleur)
  const auto i = static_cast<size_t>(idx(id_));
  juce::String tag;
  if (ctx_.modCount[i] > 0) tag << "M" << static_cast<int>(ctx_.modCount[i]);
  if (ctx_.learnCc[i] >= 0) tag << (tag.isEmpty() ? "" : " ") << "CC" << ctx_.learnCc[i];
  if (inactive_) tag = tag.isEmpty() ? juce::String("INACTIF") : tag + " INACTIF";
  if (tag.isNotEmpty() && (density_ != Density::Micro || inactive_)) {
    g.setFont(uiFont(9.5f, true));
    g.setColour(inactive_ ? tok::textDim : tok::mod);
    auto kr = knobArea();
    g.drawText(tag, kr.removeFromTop(12.0f).removeFromRight(std::min(kr.getWidth(), 64.0f)), juce::Justification::topRight, false);
  }
  juce::ignoreUnused(accent);
}

void ContinuousControl::paintRotary(juce::Graphics& g, juce::Rectangle<float> a) {
  const auto accent = accentOf(domainOf(id_));
  const float r = std::max(6.0f, std::min(a.getWidth(), a.getHeight()) * 0.5f - 1.0f);
  const auto c = a.getCentre();
  const float a0 = juce::MathConstants<float>::pi * -0.75f, a1 = juce::MathConstants<float>::pi * 0.75f;
  const float n = link_.toNorm(shown_);
  const float ang = a0 + n * (a1 - a0);
  // cavité carbone + anneau de piste
  g.setColour(tok::carbon);
  g.fillEllipse(c.x - r, c.y - r, 2 * r, 2 * r);
  juce::Path trackP;
  trackP.addCentredArc(c.x, c.y, r * 0.86f, r * 0.86f, 0.0f, a0, a1, true);
  g.setColour(tok::track.brighter(0.25f));
  g.strokePath(trackP, juce::PathStrokeType(r * 0.13f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
  // arc Base (depuis le centre pour un paramètre bipolaire)
  const float from = bipolar(id_) ? a0 + link_.toNorm(0.0f) * (a1 - a0) : a0;
  juce::Path base;
  base.addCentredArc(c.x, c.y, r * 0.86f, r * 0.86f, 0.0f, std::min(from, ang), std::max(from, ang), true);
  g.setColour(isEnabled() && !inactive_ ? accent : tok::textMute);
  g.strokePath(base, juce::PathStrokeType(r * 0.13f, juce::PathStrokeType::curved, juce::PathStrokeType::butt));
  // corps titane usiné
  const float br = r * 0.64f;
  juce::ColourGradient grad(tok::titanium.brighter(0.18f), c.x, c.y - br, tok::titanium.darker(0.45f), c.x, c.y + br, false);
  g.setGradientFill(grad);
  g.fillEllipse(c.x - br, c.y - br, 2 * br, 2 * br);
  g.setColour(tok::panelEdge.brighter(0.3f));
  g.drawEllipse(c.x - br, c.y - br, 2 * br, 2 * br, 1.0f);
  // index
  g.setColour(tok::text);
  g.drawLine(c.x + br * 0.25f * std::sin(ang), c.y - br * 0.25f * std::cos(ang), c.x + br * 0.92f * std::sin(ang),
             c.y - br * 0.92f * std::cos(ang), density_ == Density::Micro ? 1.6f : 2.2f);
  // Effective : anneau extérieur fin + point, couleur modulation (distinct de la Base)
  if (effNorm_ >= 0.0f) {
    const float ea = a0 + effNorm_ * (a1 - a0);
    juce::Path e;
    e.addCentredArc(c.x, c.y, r, r, 0.0f, std::min(ang, ea), std::max(ang, ea), true);
    g.setColour(tok::mod.withAlpha(0.9f));
    g.strokePath(e, juce::PathStrokeType(1.6f));
    g.fillEllipse(c.x + r * std::sin(ea) - 2.6f, c.y - r * std::cos(ea) - 2.6f, 5.2f, 5.2f);
  }
  if (hasKeyboardFocus(false)) {
    g.setColour(tok::focus);
    g.drawEllipse(c.x - r - 1.5f, c.y - r - 1.5f, 2 * r + 3, 2 * r + 3, 1.0f);
  }
}

void ContinuousControl::paintLinear(juce::Graphics& g, juce::Rectangle<float> a) {
  const auto accent = accentOf(domainOf(id_));
  const bool vert = form_ == Form::LinearV;
  const float n = link_.toNorm(shown_);
  auto rail = vert ? a.withSizeKeepingCentre(std::min(16.0f, a.getWidth()), a.getHeight()) : a.withSizeKeepingCentre(a.getWidth(), std::min(14.0f, a.getHeight()));
  g.setColour(tok::carbon);
  g.fillRoundedRectangle(rail, 3.0f);
  g.setColour(tok::glassEdge);
  g.drawRoundedRectangle(rail.reduced(0.5f), 3.0f, 1.0f);
  // mesure réelle (sortie) : colonne fine dans le rail, distincte de la membrane Base
  if (meter_) {
    const float m = juce::jlimit(0.0f, 1.0f, (meterDb_ + 60.0f) / 66.0f);
    auto mr = rail.reduced(3.0f);
    mr = vert ? mr.removeFromBottom(mr.getHeight() * m).removeFromLeft(3.0f) : mr.removeFromLeft(mr.getWidth() * m).removeFromTop(3.0f);
    g.setColour(meterDb_ > 0.0f ? tok::alert : tok::text.withAlpha(0.7f));
    g.fillRect(mr);
  }
  // membrane Base
  const float from = bipolar(id_) ? link_.toNorm(0.0f) : 0.0f;
  auto inner = rail.reduced(vert ? 5.0f : 2.0f, vert ? 2.0f : 4.0f);
  juce::Rectangle<float> mem;
  if (vert) {
    const float y0 = inner.getBottom() - inner.getHeight() * std::max(from, n), y1 = inner.getBottom() - inner.getHeight() * std::min(from, n);
    mem = {inner.getX(), y0, inner.getWidth(), y1 - y0};
  } else {
    const float x0 = inner.getX() + inner.getWidth() * std::min(from, n), x1 = inner.getX() + inner.getWidth() * std::max(from, n);
    mem = {x0, inner.getY(), x1 - x0, inner.getHeight()};
  }
  g.setColour((isEnabled() ? accent : tok::textMute).withAlpha(0.85f));
  g.fillRoundedRectangle(mem, 2.0f);
  // poignée titane
  const float p = vert ? inner.getBottom() - inner.getHeight() * n : inner.getX() + inner.getWidth() * n;
  auto h = vert ? juce::Rectangle<float>(a.getCentreX() - 14.0f, p - 4.0f, 28.0f, 8.0f) : juce::Rectangle<float>(p - 4.0f, a.getCentreY() - 10.0f, 8.0f, 20.0f);
  g.setColour(tok::titanium.brighter(0.2f));
  g.fillRoundedRectangle(h, 2.0f);
  g.setColour(tok::text);
  if (vert) g.drawLine(h.getX() + 3, p, h.getRight() - 3, p, 1.4f);
  else g.drawLine(p, h.getY() + 3, p, h.getBottom() - 3, 1.4f);
  if (effNorm_ >= 0.0f) {
    const float e = vert ? inner.getBottom() - inner.getHeight() * effNorm_ : inner.getX() + inner.getWidth() * effNorm_;
    g.setColour(tok::mod);
    if (vert) g.fillRect(rail.getRight() + 2.0f, e - 1.5f, 6.0f, 3.0f);
    else g.fillRect(e - 1.5f, rail.getBottom() + 2.0f, 3.0f, 6.0f);
  }
  if (hasKeyboardFocus(false)) {
    g.setColour(tok::focus);
    g.drawRoundedRectangle(rail.expanded(2.0f), 3.0f, 1.0f);
  }
}

void ContinuousControl::tick() {
  auto& t = ctx_.proc.telemetry();
  const auto i = static_cast<size_t>(idx(id_));
  float n = -1.0f;
  if (t.modulated[i].load(std::memory_order_relaxed)) n = link_.toNorm(t.effective[i].load(std::memory_order_relaxed));
  bool dirty = std::abs(n - effNorm_) > 1e-4f;
  effNorm_ = n;
  if (meter_) {
    const float db = meter_();
    if (std::abs(db - meterDb_) > 0.2f) dirty = true;
    meterDb_ = db;
  }
  const float b = link_.base();
  if (b != shown_) {
    shown_ = b;
    dirty = true;
  }
  if (dirty) repaint();
}

void ContinuousControl::mouseDown(const juce::MouseEvent& e) {
  if (ctx_.select) ctx_.select(id_);
  if (e.mods.isPopupMenu()) {
    showMenu();
    return;
  }
  if (e.mods.isAltDown()) {
    resetToDefault();
    return;
  }
  if (valueArea().contains(e.position) && form_ != Form::LinearH) return;  // double-clic sur la valeur = saisie
  dragging_ = true;
  dragStart_ = e.position;
  dragStartNorm_ = link_.norm();
  link_.begin();
  if (form_ != Form::Rotary) {  // linéaire : clic = position absolue
    const auto a = knobArea();
    const float n = form_ == Form::LinearV ? (a.getBottom() - e.position.y) / a.getHeight() : (e.position.x - a.getX()) / a.getWidth();
    dragStartNorm_ = juce::jlimit(0.0f, 1.0f, n);
    link_.during(link_.fromNorm(dragStartNorm_));
  }
}

void ContinuousControl::mouseDrag(const juce::MouseEvent& e) {
  if (!dragging_) return;
  const double fine = e.mods.isShiftDown() ? tok::fineFactor : 1.0;
  const auto d = e.position - dragStart_;
  double delta = 0.0;
  if (form_ == Form::Rotary) delta = (-d.y + d.x * 0.5) / tok::dragPixelsFull;
  else if (form_ == Form::LinearV) delta = -d.y / std::max(20.0, static_cast<double>(knobArea().getHeight()));
  else delta = d.x / std::max(20.0, static_cast<double>(knobArea().getWidth()));
  const float n = juce::jlimit(0.0f, 1.0f, static_cast<float>(dragStartNorm_ + delta * fine));
  link_.during(link_.fromNorm(n));
  if (e.mods.isShiftDown() != (fine < 1.0)) {  // changement de mode fin en cours de geste : repartir de la position
    dragStart_ = e.position;
    dragStartNorm_ = n;
  }
}

void ContinuousControl::mouseUp(const juce::MouseEvent&) {
  if (!dragging_) return;
  dragging_ = false;
  link_.end();
}

void ContinuousControl::mouseDoubleClick(const juce::MouseEvent& e) {
  if (valueArea().contains(e.position)) beginNumericEntry();
  else resetToDefault();
}

void ContinuousControl::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& w) {
  const float dy = (std::abs(w.deltaY) > std::abs(w.deltaX) ? w.deltaY : -w.deltaX) * (w.isReversed ? -1.0f : 1.0f);
  if (dy == 0.0f || dragging_) return;
  if (!wheelGesture_) {
    wheelGesture_ = true;
    link_.begin();
  }
  const double step = (e.mods.isShiftDown() ? tok::fineFactor : 1.0) * 0.05 * dy;
  const auto& s = paramSpec(id_);
  float n = juce::jlimit(0.0f, 1.0f, link_.norm() + static_cast<float>(step));
  if (s.type == ParamType::Int) n = link_.toNorm(std::round(link_.fromNorm(n)) + (dy > 0 ? 0.0f : 0.0f));
  link_.during(link_.fromNorm(n));
  startTimer(tok::wheelGestureMs);
}

void ContinuousControl::timerCallback() {
  stopTimer();
  if (wheelGesture_) {
    wheelGesture_ = false;
    link_.end();
  }
}

void ContinuousControl::stepNorm(double delta) {
  const auto& s = paramSpec(id_);
  if (s.type == ParamType::Int) {
    const float v = std::round(shown_) + (delta > 0 ? 1.0f : -1.0f) * (std::abs(delta) >= tok::pageStep ? 10.0f : 1.0f);
    userSet(v);
    return;
  }
  userSet(link_.fromNorm(juce::jlimit(0.0f, 1.0f, link_.norm() + static_cast<float>(delta))));
}

bool ContinuousControl::keyPressed(const juce::KeyPress& k) {
  const double f = k.getModifiers().isShiftDown() ? tok::fineFactor : 1.0;
  const int code = k.getKeyCode();
  if (code == juce::KeyPress::upKey || code == juce::KeyPress::rightKey) return stepNorm(tok::keyStep * f), true;
  if (code == juce::KeyPress::downKey || code == juce::KeyPress::leftKey) return stepNorm(-tok::keyStep * f), true;
  if (code == juce::KeyPress::pageUpKey) return stepNorm(tok::pageStep), true;
  if (code == juce::KeyPress::pageDownKey) return stepNorm(-tok::pageStep), true;
  if (code == juce::KeyPress::homeKey) return userSet(paramSpec(id_).minValue), true;
  if (code == juce::KeyPress::endKey) return userSet(paramSpec(id_).maxValue), true;
  if (code == juce::KeyPress::deleteKey || code == juce::KeyPress::backspaceKey) return resetToDefault(), true;
  if (code == juce::KeyPress::returnKey) {
    if (ctx_.select) ctx_.select(id_);
    beginNumericEntry();
    return true;
  }
  return false;
}

void ContinuousControl::resetToDefault() { userSet(paramSpec(id_).defaultValue); }

void ContinuousControl::beginNumericEntry() {
  entry_ = std::make_unique<juce::TextEditor>();
  auto* ed = entry_.get();
  ed->setFont(numFont(valueFont(density_)));
  ed->setJustification(juce::Justification::centred);
  ed->setText(juce::String(formatValue(id_, shown_)).upToFirstOccurrenceOf(" ", false, false), false);
  ed->setTitle(juce::String("Saisie : ") + paramSpec(id_).name);
  addAndMakeVisible(*ed);
  auto va = valueArea().toNearestInt();
  if (form_ == Form::LinearH) va = va.withLeft(static_cast<int>(knobArea().getX()));
  ed->setBounds(va.expanded(0, 1));
  ed->selectAll();
  ed->grabKeyboardFocus();
  juce::Component::SafePointer<ContinuousControl> safe(this);
  auto finish = [safe](bool commit) {
    if (safe == nullptr || safe->entry_ == nullptr) return;
    float v = 0.0f;
    if (commit && parseValueText(safe->id_, safe->entry_->getText(), v)) safe->userSet(v);
    juce::MessageManager::callAsync([safe] {
      if (safe != nullptr) {
        safe->entry_.reset();
        safe->grabKeyboardFocus();
      }
    });
  };
  ed->onReturnKey = [finish] { finish(true); };
  ed->onEscapeKey = [finish] { finish(false); };
  ed->onFocusLost = [finish] { finish(true); };
}

void ContinuousControl::showMenu() {
  const auto& s = paramSpec(id_);
  juce::PopupMenu m;
  m.addSectionHeader(juce::String(s.name) + "  (" + s.stableId + ")");
  m.addItem(1, "Saisir une valeur...");
  m.addItem(2, juce::String::fromUTF8("Réinitialiser (défaut ") + juce::String(formatValue(id_, s.defaultValue)) + ")");
  if (MidiLearnMap::learnableParam(idx(id_))) m.addItem(3, "MIDI Learn");
  m.addItem(4, "Inspecter / moduler");
  juce::Component::SafePointer<ContinuousControl> safe(this);
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this), [safe](int r) {
    if (safe == nullptr) return;
    if (r == 1) safe->beginNumericEntry();
    if (r == 2) safe->resetToDefault();
    if (r == 3 && safe->ctx_.requestLearn) safe->ctx_.requestLearn(safe->id_);
    if (r == 4 && safe->ctx_.select) safe->ctx_.select(safe->id_);
  });
}

std::unique_ptr<juce::AccessibilityHandler> ContinuousControl::createAccessibilityHandler() {
  return std::make_unique<juce::AccessibilityHandler>(*this, juce::AccessibilityRole::slider, juce::AccessibilityActions(),
                                                      juce::AccessibilityHandler::Interfaces{std::make_unique<NexusAccessibleValue>(*this)});
}

// ================================================================ SYNAPTIC TRACK
namespace {
class TrackAccessibleValue : public juce::AccessibilityTextValueInterface {
 public:
  TrackAccessibleValue(SynapticTrack& t, ParamId id) : t_(t), id_(id) {}
  bool isReadOnly() const override { return false; }
  juce::String getCurrentValueAsString() const override { return juce::String(formatValue(id_, t_.shownBase())); }
  void setValueAsString(const juce::String& s) override {
    float v = 0.0f;
    if (parseValueText(id_, s, v)) t_.userSet(v);
  }

 private:
  SynapticTrack& t_;
  ParamId id_;
};
}  // namespace

SynapticTrack::SynapticTrack(UiContext& ctx, ParamId id) : BoundControl(ctx), id_(id), link_(ctx.proc, id, [this] {
  index_ = static_cast<int>(link_.base() + 0.5f);
  repaint();
}) {
  const auto& s = paramSpec(id);
  count_ = std::max(1, choiceCount(id));
  for (int k = 0; k < count_; ++k) labels_.add(juce::String(choiceLabel(id, k)));
  caption_ = shortLabel(id);
  index_ = static_cast<int>(link_.base() + 0.5f);
  setTitle(s.name);
  setHelpText(s.stableId);
  setWantsKeyboardFocus(true);
  setTooltip(juce::String(s.name) + "\n" + s.stableId + (s.automatable ? "" : "  |  structurel : non automatisable"));
}
SynapticTrack::~SynapticTrack() = default;

bool SynapticTrack::segmented() const {
  if (count_ > 5) return false;
  const auto f = uiFont(tok::fontLabel);
  const float w = bodyArea().getWidth() / static_cast<float>(count_);
  for (const auto& l : labels_)
    if (juce::GlyphArrangement::getStringWidth(f, l) > w - 6.0f) return false;
  return true;
}

juce::Rectangle<float> SynapticTrack::bodyArea() const { return getLocalBounds().toFloat().reduced(2.0f).withTrimmedTop(tok::fontLabel * 1.25f + 2.0f); }

bool SynapticTrack::textFits() const {
  juce::StringArray lines;
  float sz = 0.0f;
  if (!layoutLabel(caption_, getWidth() - 4.0f, 1, tok::fontLabel, lines, sz)) return false;
  if (segmented()) return true;
  const auto f = uiFont(tok::fontLabel);
  for (const auto& l : labels_)
    if (juce::GlyphArrangement::getStringWidth(f, l) > bodyArea().getWidth() - 28.0f) return false;
  return true;
}

void SynapticTrack::paint(juce::Graphics& g) {
  const auto accent = accentOf(domainOf(id_));
  auto top = getLocalBounds().toFloat().reduced(2.0f).removeFromTop(tok::fontLabel * 1.25f);
  drawLabel(g, caption_, top, 1, tok::fontLabel, isEnabled() ? tok::text : tok::textMute, juce::Justification::centredLeft);
  const auto b = bodyArea();
  g.setColour(tok::carbon);
  g.fillRoundedRectangle(b, tok::radiusSmall);
  g.setColour(hasKeyboardFocus(false) ? tok::focus : tok::glassEdge);
  g.drawRoundedRectangle(b.reduced(0.5f), tok::radiusSmall, hasKeyboardFocus(false) ? 1.4f : 1.0f);
  g.setFont(uiFont(tok::fontLabel, false));
  if (segmented()) {
    const float w = b.getWidth() / static_cast<float>(count_);
    for (int k = 0; k < count_; ++k) {
      auto r = juce::Rectangle<float>(b.getX() + w * k, b.getY(), w, b.getHeight()).reduced(2.0f);
      if (k == index_) {
        g.setColour(accent.withAlpha(0.22f));
        g.fillRoundedRectangle(r, 2.0f);
        g.setColour(accent);
        g.fillRect(r.getX() + 3.0f, r.getBottom() - 2.0f, r.getWidth() - 6.0f, 2.0f);  // trait synaptique (forme + couleur)
      }
      g.setColour(k == index_ ? tok::text : tok::textDim);
      g.drawText(labels_[k], r, juce::Justification::centred, false);
      if (k > 0) {
        g.setColour(tok::glassEdge);
        g.drawVerticalLine(static_cast<int>(b.getX() + w * k), b.getY() + 4.0f, b.getBottom() - 4.0f);
      }
    }
  } else {
    auto r = b.reduced(6.0f, 0.0f);
    g.setColour(accent);
    g.fillRect(r.getX() - 2.0f, b.getCentreY() - 6.0f, 2.0f, 12.0f);
    g.setColour(tok::text);
    g.drawText(index_ >= 0 && index_ < count_ ? labels_[index_] : juce::String("?"), r.withTrimmedRight(16.0f), juce::Justification::centredLeft, false);
    g.setColour(tok::textDim);
    juce::Path tri;
    const float cx = r.getRight() - 6.0f, cy = b.getCentreY();
    tri.addTriangle(cx - 4.0f, cy - 2.0f, cx + 4.0f, cy - 2.0f, cx, cy + 3.0f);
    g.fillPath(tri);
    g.setFont(numFont(9.5f));
    g.drawText(juce::String(index_ + 1) + "/" + juce::String(count_), r.withTrimmedRight(18.0f), juce::Justification::centredRight, false);
  }
}

void SynapticTrack::mouseDown(const juce::MouseEvent& e) {
  if (ctx_.select) ctx_.select(id_);
  grabKeyboardFocus();
  if (e.mods.isPopupMenu() || !segmented()) {
    openList();
    return;
  }
  const auto b = bodyArea();
  if (!b.contains(e.position)) return;
  const int k = juce::jlimit(0, count_ - 1, static_cast<int>((e.position.x - b.getX()) / (b.getWidth() / count_)));
  userSet(static_cast<float>(k));
}

void SynapticTrack::openList() {
  juce::PopupMenu m;
  m.addSectionHeader(juce::String(paramSpec(id_).name));
  for (int k = 0; k < count_; ++k) m.addItem(k + 1, labels_[k], true, k == index_);
  juce::Component::SafePointer<SynapticTrack> safe(this);
  m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this).withMinimumWidth(getWidth()), [safe](int r) {
    if (safe != nullptr && r > 0) safe->userSet(static_cast<float>(r - 1));
  });
}

bool SynapticTrack::keyPressed(const juce::KeyPress& k) {
  const int code = k.getKeyCode();
  if (code == juce::KeyPress::rightKey || code == juce::KeyPress::downKey) return userSet(static_cast<float>(std::min(count_ - 1, index_ + 1))), true;
  if (code == juce::KeyPress::leftKey || code == juce::KeyPress::upKey) return userSet(static_cast<float>(std::max(0, index_ - 1))), true;
  if (code == juce::KeyPress::homeKey) return userSet(0.0f), true;
  if (code == juce::KeyPress::endKey) return userSet(static_cast<float>(count_ - 1)), true;
  if (code == juce::KeyPress::returnKey || code == juce::KeyPress::spaceKey) return openList(), true;
  if (code == juce::KeyPress::deleteKey || code == juce::KeyPress::backspaceKey) return userSet(paramSpec(id_).defaultValue), true;
  return false;
}

std::unique_ptr<juce::AccessibilityHandler> SynapticTrack::createAccessibilityHandler() {
  return std::make_unique<juce::AccessibilityHandler>(*this, juce::AccessibilityRole::comboBox, juce::AccessibilityActions(),
                                                      juce::AccessibilityHandler::Interfaces{std::make_unique<TrackAccessibleValue>(*this, id_)});
}

// ================================================================ NEURAL CELL
NeuralCell::NeuralCell(UiContext& ctx, ParamId id) : juce::ToggleButton(paramSpec(id).name), BoundControl(ctx), id_(id) {
  const auto& s = paramSpec(id);
  caption_ = shortLabel(id);
  accent_ = isAutonomyArm(id) ? tok::warn : accentOf(domainOf(id));
  link_ = std::make_unique<ParamLink>(ctx.proc, id, [this] {
    updating_ = true;
    setToggleState(link_->base() >= 0.5f, juce::dontSendNotification);
    updating_ = false;
    tick();
  });
  setToggleState(link_->base() >= 0.5f, juce::dontSendNotification);
  setTitle(s.name);
  setHelpText(s.stableId);
  setWantsKeyboardFocus(true);
  setTooltip(juce::String(s.name) + "\n" + s.stableId +
             (isAutonomyArm(id) ? "  |  armement autonome : la mise en marche par clic vaut autorisation explicite" : ""));
  onClick = [this] {
    if (updating_) return;
    if (ctx_.select) ctx_.select(id_);
    const bool on = getToggleState();
    link_->complete(on ? 1.0f : 0.0f);
    // armement mis en marche par un clic : intention explicite, qui vaut autorisation (ARCH-G4.3, TA-07w)
    if (on && isAutonomyArm(id_)) ctx_.proc.authorizeAutonomy();
    tick();
  };
  tick();
}

NeuralCell::NeuralCell(UiContext& ctx, const juce::String& text, juce::Colour accent) : juce::ToggleButton(text), BoundControl(ctx) {
  caption_ = text;
  accent_ = accent;
  setTitle(text);
  setWantsKeyboardFocus(true);
  onClick = [this] {
    if (onUserToggle) onUserToggle(getToggleState());
  };
  // un état hors registre n'est pas une commande de paramètre : retiré de l'inventaire de couverture
  auto& r = ctx_.registry;
  r.erase(std::remove(r.begin(), r.end(), static_cast<BoundControl*>(this)), r.end());
}

NeuralCell::~NeuralCell() = default;

void NeuralCell::setCellState(CellState s) {
  if (s != state_) {
    state_ = s;
    repaint();
  }
}

void NeuralCell::tick() {
  if (id_ == ParamId::Count) return;
  const bool on = link_->base() >= 0.5f;
  if (on != getToggleState()) {
    updating_ = true;
    setToggleState(on, juce::dontSendNotification);
    updating_ = false;
  }
  if (!on) setCellState(CellState::Off);
  else if (isAutonomyArm(id_) && !ctx_.proc.autonomyAuthorized()) setCellState(CellState::Armed);
  else setCellState(CellState::Active);
}

// zone du libellé : partagée par le dessin et par la mesure de troncature (UI04-CLIP)
static juce::Rectangle<float> cellTextArea(juce::Rectangle<int> b) {
  return b.toFloat().reduced(1.5f).withTrimmedLeft(22.0f).reduced(2.0f, 1.0f).withTrimmedRight(40.0f);
}

bool NeuralCell::textFits() const {
  juce::StringArray lines;
  float sz = 0.0f;
  return layoutLabel(caption_, cellTextArea(getLocalBounds()).getWidth(), 2, tok::fontLabel, lines, sz);
}

void NeuralCell::paintButton(juce::Graphics& g, bool over, bool down) {
  auto r = getLocalBounds().toFloat().reduced(1.5f);
  g.setColour(down ? tok::titanium : tok::carbon);
  g.fillRoundedRectangle(r, tok::radiusSmall);
  // contour partiellement lumineux selon l'état (forme + texte : jamais la seule couleur)
  const bool lit = state_ == CellState::Active || state_ == CellState::Armed;
  const juce::Colour c = state_ == CellState::Refused ? tok::alert : (state_ == CellState::Armed ? tok::warn : accent_);
  g.setColour(lit ? c : (over ? tok::titanium.brighter(0.3f) : tok::glassEdge));
  g.drawRoundedRectangle(r.reduced(0.5f), tok::radiusSmall, lit ? 1.6f : 1.0f);
  if (lit) {
    g.setColour(c.withAlpha(0.18f));
    g.fillRoundedRectangle(r.reduced(2.0f), tok::radiusSmall);
  }
  // diode d'état
  const float d = 8.0f;
  auto led = juce::Rectangle<float>(r.getX() + 8.0f, r.getCentreY() - d * 0.5f, d, d);
  g.setColour(lit ? c : tok::titanium);
  if (state_ == CellState::Armed) g.drawEllipse(led, 1.6f);  // ARMÉ : anneau ; ACTIF : disque plein
  else g.fillEllipse(led);
  auto text = cellTextArea(getLocalBounds());
  const char* st = state_ == CellState::Off ? "OFF" : state_ == CellState::Armed ? "ARMÉ" : state_ == CellState::Refused ? "REFUS" : state_ == CellState::Disabled ? "—" : "ACTIF";
  if (id_ != ParamId::Count && !isAutonomyArm(id_) && state_ == CellState::Active) st = "ON";
  const float sw = 40.0f;
  g.setFont(numFont(9.5f, true));
  g.setColour(lit ? tok::text : tok::textDim);
  g.drawText(juce::String::fromUTF8(st), juce::Rectangle<float>(text.getRight(), text.getY(), sw, text.getHeight()), juce::Justification::centredRight, false);
  drawLabel(g, caption_, text, 2, tok::fontLabel, isEnabled() ? tok::text : tok::textMute, juce::Justification::centredLeft);
  if (hasKeyboardFocus(false)) {
    g.setColour(tok::focus);
    g.drawRoundedRectangle(r.expanded(1.0f), tok::radiusSmall, 1.0f);
  }
}

// ================================================================ ActionCell
ActionCell::ActionCell(const juce::String& text, juce::Colour accent, bool danger) : juce::TextButton(text), accent_(accent), danger_(danger) {
  setTitle(text);
  setWantsKeyboardFocus(true);
}

void ActionCell::paintButton(juce::Graphics& g, bool over, bool down) {
  auto r = getLocalBounds().toFloat().reduced(1.5f);
  const juce::Colour c = danger_ ? tok::alert : accent_;
  if (danger_) {
    g.setColour(down ? tok::alert.brighter(0.2f) : tok::alert.withAlpha(over ? 0.95f : 0.82f));
    g.fillRoundedRectangle(r, tok::radiusSmall);
  } else {
    juce::ColourGradient grad(tok::titanium.brighter(down ? 0.0f : 0.12f), r.getX(), r.getY(), tok::titanium.darker(0.35f), r.getX(), r.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(r, tok::radiusSmall);
    g.setColour(getToggleState() ? c : (over ? c.withAlpha(0.7f) : tok::panelEdge.brighter(0.25f)));
    g.drawRoundedRectangle(r.reduced(0.5f), tok::radiusSmall, getToggleState() ? 1.6f : 1.0f);
    g.setColour(c.withAlpha(isEnabled() ? 0.95f : 0.35f));
    g.fillRect(r.getX() + 5.0f, r.getBottom() - 3.0f, 10.0f, 1.6f);  // repère d'action (momentané)
  }
  if (hasKeyboardFocus(false)) {
    g.setColour(tok::focus);
    g.drawRoundedRectangle(r.expanded(1.0f), tok::radiusSmall, 1.0f);
  }
  g.setColour(danger_ ? juce::Colours::white : (isEnabled() ? (refused_ ? tok::alert : tok::text) : tok::textMute));
  juce::StringArray lines;
  float sz = 0.0f;
  layoutLabel(getButtonText(), r.getWidth() - 8.0f, 2, tok::fontLabel + (danger_ ? 1.0f : 0.0f), lines, sz);
  g.setFont(uiFont(sz, true));
  g.drawFittedText(lines.joinIntoString("\n"), r.reduced(4.0f, 1.0f).toNearestInt(), juce::Justification::centred, 2, 1.0f);
}

// ================================================================ TabStrip
void TabStrip::setTabs(const juce::StringArray& names) {
  names_ = names;
  setWantsKeyboardFocus(true);
  repaint();
}
void TabStrip::setCurrent(int k) {
  current_ = juce::jlimit(0, std::max(0, names_.size() - 1), k);
  repaint();
}
int TabStrip::preferredWidth() const {
  float w = 0.0f;
  for (const auto& n : names_) w += juce::GlyphArrangement::getStringWidth(uiFont(tok::fontLabel, true), n) + 22.0f;
  return static_cast<int>(std::ceil(w));
}
juce::Rectangle<float> TabStrip::tabRect(int k) const {
  const float total = static_cast<float>(preferredWidth());
  const float scale = total > getWidth() ? getWidth() / total : 1.0f;
  float x = 0.0f;
  for (int i = 0; i < names_.size(); ++i) {
    const float w = (juce::GlyphArrangement::getStringWidth(uiFont(tok::fontLabel, true), names_[i]) + 22.0f) * scale;
    if (i == k) return {x, 0.0f, w, static_cast<float>(getHeight())};
    x += w;
  }
  return {};
}
bool TabStrip::textFits() const { return preferredWidth() <= getWidth(); }
void TabStrip::paint(juce::Graphics& g) {
  g.setColour(tok::carbon.withAlpha(0.6f));
  g.fillRoundedRectangle(getLocalBounds().toFloat(), tok::radiusSmall);
  for (int k = 0; k < names_.size(); ++k) {
    const auto r = tabRect(k).reduced(1.5f, 2.0f);
    const bool cur = k == current_;
    if (cur) {
      g.setColour(tok::panel.brighter(0.08f));
      g.fillRoundedRectangle(r, tok::radiusSmall);
      g.setColour(accent_);
      g.fillRect(r.getX() + 6.0f, r.getBottom() - 2.0f, r.getWidth() - 12.0f, 2.0f);
    }
    g.setColour(cur ? tok::text : tok::textDim);
    g.setFont(uiFont(tok::fontLabel, true));
    g.drawText(names_[k], r, juce::Justification::centred, false);
  }
  if (hasKeyboardFocus(false)) {
    g.setColour(tok::focus);
    g.drawRoundedRectangle(tabRect(current_).reduced(0.5f), tok::radiusSmall, 1.0f);
  }
}
void TabStrip::mouseDown(const juce::MouseEvent& e) {
  for (int k = 0; k < names_.size(); ++k)
    if (tabRect(k).contains(e.position)) {
      setCurrent(k);
      if (onChange) onChange(k);
    }
}
bool TabStrip::keyPressed(const juce::KeyPress& k) {
  int d = 0;
  if (k.getKeyCode() == juce::KeyPress::rightKey) d = 1;
  if (k.getKeyCode() == juce::KeyPress::leftKey) d = -1;
  if (d == 0) return false;
  setCurrent((current_ + d + names_.size()) % std::max(1, names_.size()));
  if (onChange) onChange(current_);
  return true;
}

// ================================================================ plaques / afficheurs
void Plate::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat().reduced(1.0f);
  juce::ColourGradient grad(tok::panel.brighter(0.04f), r.getX(), r.getY(), tok::panel.darker(0.12f), r.getX(), r.getBottom(), false);
  g.setGradientFill(grad);
  g.fillRoundedRectangle(r, tok::radius);
  g.setColour(tok::panelEdge);
  g.drawRoundedRectangle(r.reduced(0.5f), tok::radius, 1.0f);
  // vis de plaque (usinage) et bandeau de titre
  g.setColour(tok::titanium);
  for (auto p : {r.getTopLeft() + juce::Point<float>(6, 6), r.getTopRight() + juce::Point<float>(-6, 6)}) g.fillEllipse(p.x - 1.5f, p.y - 1.5f, 3, 3);
  g.setColour(accent_);
  g.fillRoundedRectangle(r.getX() + 12.0f, r.getY() + 7.0f, 3.0f, 11.0f, 1.0f);
  g.setColour(tok::text);
  g.setFont(uiFont(tok::fontTitle, true));
  auto head = juce::Rectangle<float>(r.getX() + 20.0f, r.getY() + 3.0f, r.getWidth() - 34.0f, 18.0f);
  g.drawText(title_, head, juce::Justification::centredLeft, false);
  if (note_.isNotEmpty()) {
    g.setColour(tok::textDim);
    g.setFont(uiFont(tok::fontLabel));
    g.drawText(note_, head, juce::Justification::centredRight, false);
  }
}

void OpticalDisplay::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat().reduced(1.0f);
  g.setColour(tok::glass);
  g.fillRoundedRectangle(r, tok::radius);
  g.setColour(tok::glassEdge);
  g.drawRoundedRectangle(r.reduced(0.5f), tok::radius, 1.2f);
  g.setFont(uiFont(10.0f, true));
  g.setColour(tok::textDim);
  g.drawText(title_, r.reduced(8.0f, 2.0f).removeFromTop(14.0f), juce::Justification::centredLeft, false);
  g.setFont(numFont(9.5f));
  g.drawText(tap_, r.reduced(8.0f, 2.0f).removeFromTop(14.0f), juce::Justification::centredRight, false);
}
void OpticalDisplay::paintOverChildren(juce::Graphics& g) {
  // reflet du verre fumé : dégradé discret, aucune trame CRT
  auto r = getLocalBounds().toFloat().reduced(2.0f);
  juce::ColourGradient sheen(juce::Colours::white.withAlpha(0.035f), r.getX(), r.getY(), juce::Colours::transparentWhite, r.getX(), r.getY() + r.getHeight() * 0.45f, false);
  g.setGradientFill(sheen);
  g.fillRoundedRectangle(r, tok::radius);
}

// ================================================================ fabrique et disposition
BoundControl* asBound(juce::Component* c) { return dynamic_cast<BoundControl*>(c); }

juce::Rectangle<int> preferredCell(ParamId id, Density d) {
  const auto& s = paramSpec(id);
  if (s.type == ParamType::Choice) return {0, 0, d == Density::Micro ? 140 : tok::track1.w, tok::track1.h};
  if (s.type == ParamType::Bool) return {0, 0, tok::cell1.w + (isAutonomyArm(id) ? 40 : 0), tok::cell1.h};
  const auto c = d == Density::XL ? tok::nexusXL : (d == Density::Micro ? tok::nexusMicro : tok::nexusS);
  return {0, 0, c.w, c.h};
}

std::unique_ptr<juce::Component> makeControl(UiContext& ctx, ParamId id, Density d) {
  const auto& s = paramSpec(id);
  if (s.type == ParamType::Choice) return std::make_unique<SynapticTrack>(ctx, id);
  if (s.type == ParamType::Bool) return std::make_unique<NeuralCell>(ctx, id);
  return std::make_unique<ContinuousControl>(ctx, id, Form::Rotary, d);
}

void ParamPanel::setParams(const std::vector<ParamId>& ids) { setGroups({{juce::String(), ids}}); }

void ParamPanel::setGroups(const std::vector<std::pair<juce::String, std::vector<ParamId>>>& groups) {
  bool same = groups.size() == groups_.size();
  for (size_t k = 0; same && k < groups.size(); ++k) same = groups[k].first == groups_[k].first && groups[k].second == groups_[k].second;
  if (same) return;
  groups_ = groups;
  controls_.clear();
  for (const auto& gr : groups_)
    for (auto id : gr.second) {
      controls_.push_back(makeControl(ctx_, id, density_));
      addAndMakeVisible(*controls_.back());
    }
  resized();
}

int ParamPanel::layout(int width, bool apply) {
  int y = 0;
  size_t k = 0;
  if (apply) heads_.clear();
  for (const auto& gr : groups_) {
    if (gr.first.isNotEmpty()) {
      if (apply) heads_.push_back({gr.first, {0, y, width, 18}});
      y += 20;
    }
    int x = 0, rowH = 0;
    for (size_t n = 0; n < gr.second.size(); ++n, ++k) {
      auto cell = preferredCell(gr.second[n], density_);
      if (x > 0 && x + cell.getWidth() > width) {
        x = 0;
        y += rowH + tok::gap / 2;
        rowH = 0;
      }
      if (apply && k < controls_.size()) controls_[k]->setBounds(x, y, std::min(cell.getWidth(), width), cell.getHeight());
      x += cell.getWidth() + tok::gap / 2;
      rowH = std::max(rowH, cell.getHeight());
    }
    y += rowH + tok::gap;
  }
  return y;
}

int ParamPanel::preferredHeight(int width) const { return const_cast<ParamPanel*>(this)->layout(width, false); }
void ParamPanel::resized() { layout(getWidth(), true); }
void ParamPanel::paint(juce::Graphics& g) {
  for (const auto& h : heads_) {
    g.setColour(accent_.withAlpha(0.7f));
    g.fillRect(h.second.getX(), h.second.getY() + 8, 8, 2);
    g.setColour(tok::textDim);
    g.setFont(uiFont(tok::fontLabel, true));
    g.drawText(h.first, h.second.withTrimmedLeft(12), juce::Justification::centredLeft, false);
    g.setColour(tok::panelEdge);
    const int tw = static_cast<int>(juce::GlyphArrangement::getStringWidth(uiFont(tok::fontLabel, true), h.first));
    g.drawHorizontalLine(h.second.getY() + 9, static_cast<float>(h.second.getX() + 20 + tw), static_cast<float>(h.second.getRight()));
  }
}

}  // namespace sbd::ui
