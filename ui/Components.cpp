#include "Components.h"

#include <cmath>
#include <cstring>

#include "core/StateCodec.h"
#include "ui/r2/R2Painters.h"

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
// correspondances vers le Design System R1 partagé (ui/r2)
sbd::r2::Domain r2Domain(Domain3 d) {
  switch (d) {
    case Domain3::Wt: return sbd::r2::Domain::Wt;
    case Domain3::Fm: return sbd::r2::Domain::Fm;
    case Domain3::Cross: return sbd::r2::Domain::Cross;
    case Domain3::Mod: return sbd::r2::Domain::Mod;
    case Domain3::Alert: return sbd::r2::Domain::Alert;
    case Domain3::Neutral: break;
  }
  return sbd::r2::Domain::Neutral;
}
sbd::r2::NexusSize nexusSize(Density d) {
  return d == Density::XL ? sbd::r2::NexusSize::XL : (d == Density::Micro ? sbd::r2::NexusSize::Micro : sbd::r2::NexusSize::S);
}
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
  baseTooltip_ = juce::String(s.name) + "\n" + s.stableId +
                 (form == Form::Rotary ? juce::String::fromUTF8("  |  glisser : réglage, Maj : fin, double-clic : défaut, Entrée : saisie")
                                       // PLASMA (DB2-08, DB2-09) : interactions réellement disponibles
                                       : juce::String::fromUTF8("  |  clic ou glisser dans le canal : réglage, Maj : fin, Alt+clic, Suppr ou menu : défaut, "
                                                                "double-clic sur la valeur ou Entrée : saisie"));
  setTooltip(baseTooltip_);
}

ContinuousControl::~ContinuousControl() {
  if (dragging_ || wheelGesture_) link_.end();
}

void ContinuousControl::setInactive(bool inactive, const juce::String& why) {
  if (inactive == inactive_) return;
  inactive_ = inactive;
  const auto& s = paramSpec(id_);
  const juce::String nl = juce::newLine;
  baseTooltip_ = juce::String(s.name) + nl + s.stableId + (inactive ? nl + "INACTIF : " + why : juce::String());
  refreshTooltip();
  repaint();
}

void ContinuousControl::refreshTooltip() {
  // CE-B2-08 : le CC appris (donnée MIDI Learn de 0.4.1) est toujours lisible dans l'infobulle, quel que soit le gabarit
  setTooltip(learnShown_ >= 0 ? baseTooltip_ + juce::newLine + "MIDI Learn : CC" + juce::String(learnShown_) : baseTooltip_);
}

juce::Rectangle<float> ContinuousControl::ccTagBox() const {
  const auto cc = ctx_.learnCc[static_cast<size_t>(idx(id_))];
  if (form_ != Form::Rotary || cc < 0 || density_ == Density::Micro) return {};
  // place libre à gauche du libellé, mesurée selon la mise en page du peintre (une ligne, sinon deux lignes équilibrées)
  const auto size = nexusSize(density_);
  const auto lf = sbd::r2::labelFont(nexusLabelPx(), size == sbd::r2::NexusSize::XL);
  const float W = static_cast<float>(getWidth());
  float wl = sbd::r2::draw::textWidth(lf, caption_);
  if (wl > W - 4.0f && caption_.containsChar(' ')) {
    float best = 1.0e9f;
    for (int k = caption_.indexOfChar(' '); k >= 0; k = caption_.indexOfChar(k + 1, ' '))
      best = std::min(best, std::max(sbd::r2::draw::textWidth(lf, caption_.substring(0, k)), sbd::r2::draw::textWidth(lf, caption_.substring(k + 1))));
    wl = best;
  }
  const float tw = sbd::r2::draw::textWidth(sbd::r2::labelFont(8.5f, true), "CC" + juce::String(static_cast<int>(cc))) + 2.0f;
  const float freeLeft = (W - wl) * 0.5f - 3.0f;  // marge de 3 px avant le libellé centré
  if (2.0f + tw > freeLeft) return {};
  return {2.0f, 0.0f, tw, knobArea().getY()};
}

void ContinuousControl::setCaption(const juce::String& c) {
  caption_ = c;
  if (form_ != Form::Rotary) layoutPlasma();
  else layoutNexus();
  repaint();
}

int ContinuousControl::labelLines() const { return form_ == Form::LinearH ? 1 : 2; }

// ---------------------------------------------------------------- PLASMA R1 (B2-03.2)
juce::Rectangle<float> ContinuousControl::channelArea() const {
  if (plasma_.r1)
    return sbd::r2::plasmaSlot(plasma_.tpl, form_ == Form::LinearV ? sbd::r2::PlasmaOrientation::Vertical : sbd::r2::PlasmaOrientation::Horizontal,
                               plasma_.compact);
  return knobArea();  // rendu 0.4.1 : correspondance historique sur la zone du rail
}

juce::Rectangle<float> ContinuousControl::manipulationArea() const {
  // DB2-08 : la zone active des gestes est la zone de manipulation du rendu
  if (form_ == Form::Rotary || plasma_.r1) return knobArea();  // R1 : canal élargi à l'empreinte du curseur
  // rendu 0.4.1 (gabarit R1 bloqué, CE-B2-02) : rail et empreinte de la poignée (28 x 8 vertical, 8 x 20 horizontal)
  // ... bornée par la sérigraphie et la fenêtre de valeur (DB2-08 : aucun recouvrement de ces zones)
  const auto a = knobArea();
  if (form_ == Form::LinearV) {
    auto z = a.withSizeKeepingCentre(std::min(28.0f, a.getWidth()), a.getHeight() + 8.0f);
    return z.withTop(std::max(z.getY(), labelArea().getBottom())).withBottom(std::min(z.getBottom(), valueArea().getY()));
  }
  auto z = a.withSizeKeepingCentre(a.getWidth() + 8.0f, std::min(20.0f, a.getHeight()));
  return z.withLeft(std::max(z.getX(), labelArea().getRight())).withRight(std::min(z.getRight(), valueArea().getX()));
}

void ContinuousControl::resized() {
  if (form_ != Form::Rotary) layoutPlasma();
  else layoutNexus();
}

void ContinuousControl::layoutNexus() {
  // mise en page du libellé et zone du bouton : mesures de texte faites ici, une fois, et non à chaque peinture (GC-01)
  nexus_.bounds = getLocalBounds();
  nexus_.caption = caption_;
  const auto size = nexusSize(density_);
  const float family = sbd::r2::nexusFamilyLabelPx(size);
  const float w = static_cast<float>(getWidth()) - 4.0f;
  nexus_.px = tok::fontMicro;
  for (float px = family; px >= tok::fontMicro - 1.0e-3f; px -= 0.5f)
    if (sbd::r2::draw::fitsText(sbd::r2::labelFont(px, size == sbd::r2::NexusSize::XL), caption_, w, true)) {
      nexus_.px = px;
      break;
    }
  sbd::r2::nexusLabelLayout(nexus_.bounds, size, caption_, nexus_.px, nexus_.lines, nexus_.split);
  nexus_.knob = sbd::r2::nexusKnobAreaLines(nexus_.bounds, size, nexus_.lines == 2, nexus_.px);
}

void ContinuousControl::layoutPlasma() {
  // CE-B2-02 : gabarit choisi par la surface utile réelle (la cellule de production), aux dimensions normatives R1, sans
  // déformation ; STANDARD si 64 x 180 tient, sinon COMPACT si 48 x 150 tient ; horizontal : hauteur 44, largeur de la
  // cellule. Le libellé et TOUTES les valeurs représentables doivent tenir dans les zones du gabarit, au même corps.
  plasma_ = {};
  const auto lb = getLocalBounds();
  if (form_ == Form::LinearV) {
    const bool standard = lb.getWidth() >= 64 && lb.getHeight() >= 180;
    const bool compact = !standard && lb.getWidth() >= 48 && lb.getHeight() >= 150;
    if (!standard && !compact) {
      plasma_.why = "cellule " + juce::String(lb.getWidth()) + " x " + juce::String(lb.getHeight()) + " : ni STANDARD 64 x 180 ni COMPACT 48 x 150";
      return;
    }
    plasma_.compact = compact;
    plasma_.tpl = lb.withSizeKeepingCentre(compact ? 48 : 64, compact ? 150 : 180);
  } else {
    if (lb.getHeight() < 44) {
      plasma_.why = "cellule de hauteur " + juce::String(lb.getHeight()) + " < 44";
      return;
    }
    plasma_.tpl = lb.withSizeKeepingCentre(lb.getWidth(), 44);
  }
  const auto vf = sbd::r2::valueFont(10.0f);
  const float vw = valueArea().getWidth();
  const auto& s = paramSpec(id_);
  float worst = 0.0f;
  juce::String worstText;
  for (int k = -3; k <= 400; ++k) {  // bornes, défaut, puis 401 valeurs régulières : toutes les formes d'affichage
    const float v = k == -3 ? s.minValue : (k == -2 ? s.maxValue : (k == -1 ? s.defaultValue : link_.fromNorm(static_cast<float>(k) / 400.0f)));
    const juce::String t(formatValue(id_, v));
    const float w = sbd::r2::draw::textWidth(vf, t);
    if (w > worst) {
      worst = w;
      worstText = t;
    }
  }
  const auto lf = sbd::r2::labelFont(tok::fontLabel);
  const float lw = form_ == Form::LinearV ? static_cast<float>(plasma_.tpl.getWidth()) - 2.0f : std::round(static_cast<float>(plasma_.tpl.getWidth()) * 0.30f) - 6.0f;
  const juce::String tplName = plasma_.compact ? "COMPACT 48 x 150" : (form_ == Form::LinearV ? "STANDARD 64 x 180" : "horizontal h44");
  if (worst > vw) {
    plasma_.tpl = {};
    plasma_.compact = false;
    plasma_.why = tplName + juce::String::fromUTF8(" : valeur « ") + worstText + juce::String::fromUTF8(" » (") + juce::String(worst, 1) +
                  juce::String::fromUTF8(" px) > fenêtre de valeur (") + juce::String(vw, 1) + " px)";
    return;
  }
  if (!sbd::r2::draw::fitsText(lf, caption_, lw, true)) {
    plasma_.tpl = {};
    plasma_.compact = false;
    plasma_.why = tplName + juce::String::fromUTF8(" : libellé « ") + caption_ + juce::String::fromUTF8(" » > sérigraphie du gabarit");
    return;
  }
  plasma_.r1 = true;
}

juce::StringArray ContinuousControl::plasmaGraduations() const {
  // Graduations réparties régulièrement de la borne basse à la borne haute (contrat du peintre) : leur texte doit être la
  // valeur réelle à cette position. Contenu R1 des planches gelées quand il est exact pour la plage réelle (BALANCE :
  // WT / 0 / FM ; plages 0..1 : 0 / .5 / 1) ; sinon bornes et milieu réels, format compact.
  const juce::String sid(paramSpec(id_).stableId);
  if (sid == "perf.balance") return {"WT", "0", "FM"};
  juce::StringArray g;
  for (float n : {0.0f, 0.5f, 1.0f}) {
    const float v = link_.fromNorm(n);
    juce::String t = std::abs(v - std::round(v)) < 1.0e-4f ? juce::String(static_cast<int>(std::lround(v))) : juce::String(v, 2).trimCharactersAtEnd("0");
    if (t.startsWith("0.")) t = t.substring(1);
    if (t.startsWith("-0.")) t = "-" + t.substring(2);
    if (t.startsWith("-")) t = juce::String::fromUTF8("\xe2\x88\x92") + t.substring(1);
    else if (bipolar(id_) && v > 0.0f) t = "+" + t;
    g.add(t);
  }
  return g;
}
float ContinuousControl::labelSize() const { return density_ == Density::XL ? tok::fontTitle : (density_ == Density::Micro ? 10.0f : tok::fontLabel); }

juce::Rectangle<float> ContinuousControl::labelArea() const {
  if (form_ == Form::Rotary) return getLocalBounds().toFloat().withBottom(knobArea().getY());
  if (plasma_.r1) {  // sérigraphie du gabarit R1 (peintre : 30 px en vertical, 30 % de la largeur en horizontal)
    const auto t = plasma_.tpl.toFloat();
    return form_ == Form::LinearV ? t.withHeight(30.0f) : t.withWidth(std::round(t.getWidth() * 0.30f));
  }
  auto r = getLocalBounds().toFloat().reduced(2.0f);
  if (form_ == Form::LinearH) return r.removeFromLeft(r.getWidth() * 0.34f);
  const float lh = labelSize() * 1.18f * static_cast<float>(labelLines());
  return r.removeFromTop(lh);
}
juce::Rectangle<float> ContinuousControl::valueArea() const {
  if (form_ == Form::Rotary) return sbd::r2::nexusValueArea(getLocalBounds(), nexusSize(density_));
  if (!plasma_.tpl.isEmpty()) {  // fenêtre de valeur du peintre PLASMA R1 (aussi pour vérifier si la valeur y tient)
    const auto t = plasma_.tpl.toFloat();
    if (form_ == Form::LinearV) return t.withTop(t.getBottom() - 17.0f).reduced(2.0f, 1.0f);
    return t.withLeft(t.getRight() - 52.0f).reduced(1.0f, 10.0f);
  }
  auto r = getLocalBounds().toFloat().reduced(2.0f);
  const float vh = valueFont(density_) + 5.0f;
  if (form_ == Form::LinearH) {
    r.removeFromLeft(r.getWidth() * 0.34f);
    return r.removeFromRight(r.getWidth() * 0.36f);
  }
  return r.removeFromBottom(vh);
}
juce::Rectangle<float> ContinuousControl::knobArea() const {
  if (form_ == Form::Rotary) {
    (void)nexusLabelPx();  // mise en page à jour
    return nexus_.knob;
  }
  if (plasma_.r1) {  // zone de manipulation (DB2-08) : le canal élargi à l'empreinte du curseur R1 (24 x 10)
    const auto s = channelArea();
    return form_ == Form::LinearV ? s.withSizeKeepingCentre(24.0f, s.getHeight() + 10.0f) : s.withSizeKeepingCentre(s.getWidth() + 10.0f, 24.0f);
  }
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

float ContinuousControl::nexusLabelPx() const {
  if (nexus_.bounds != getLocalBounds() || nexus_.caption != caption_) const_cast<ContinuousControl*>(this)->layoutNexus();  // sûreté
  return nexus_.px;
}

bool ContinuousControl::textFits() const {
  if (plasma_.r1) {  // PLASMA R1 : libellé et valeurs vérifiés au choix du gabarit ; valeur courante revérifiée
    return sbd::r2::draw::textWidth(sbd::r2::valueFont(10.0f), juce::String(formatValue(id_, shown_))) <= valueArea().getWidth();
  }
  if (form_ == Form::Rotary) {  // NEXUS R1 : sérigraphie sur deux lignes au plus, au-dessus du plancher ; valeurs extrêmes dans la fenêtre
    const auto size = nexusSize(density_);
    const float px = nexusLabelPx();
    if (!sbd::r2::draw::fitsText(sbd::r2::labelFont(px, size == sbd::r2::NexusSize::XL), caption_, static_cast<float>(getWidth()) - 4.0f, true))
      return false;
    const auto& s = paramSpec(id_);
    const auto f = sbd::r2::valueFont(sbd::r2::nexusValuePx(size));
    const float vw = valueArea().getWidth();
    for (float v : {s.minValue, s.maxValue, s.defaultValue, shown_})
      if (sbd::r2::draw::textWidth(f, juce::String(formatValue(id_, v))) > vw) return false;
    return true;
  }
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
  if (plasma_.r1) {  // PLASMA R1 (B2-03.2) : peintre partagé, gabarit normatif dans la cellule, données publiées seulement
    sbd::r2::PlasmaView v;
    v.orientation = form_ == Form::LinearV ? sbd::r2::PlasmaOrientation::Vertical : sbd::r2::PlasmaOrientation::Horizontal;
    v.domain = r2Domain(domainOf(id_));
    v.label = caption_;
    v.graduations = plasmaGraduations();
    v.compact = plasma_.compact;
    v.norm = link_.toNorm(shown_);
    if (effNorm_ >= 0.0f) v.effective = effNorm_;
    v.bipolar = bipolar(id_);
    v.zeroNorm = v.bipolar ? link_.toNorm(0.0f) : 0.0f;
    v.valueText = juce::String(formatValue(id_, shown_));
    v.disabled = !isEnabled();
    v.hover = isMouseOverOrDragging();
    v.focus = hasKeyboardFocus(false);
    v.pressed = dragging_ || wheelGesture_;
    sbd::r2::paintPlasma(g, plasma_.tpl, v);
    return;
  }
  if (form_ == Form::Rotary) {
    sbd::r2::NexusView v;  // vue du peintre partagé : données publiées seulement (Base, Effective, routes configurées)
    v.size = nexusSize(density_);
    v.domain = r2Domain(domainOf(id_));
    v.label = caption_;
    v.labelPx = nexusLabelPx();
    v.labelLines = nexus_.lines;  // mise en page calculée au redimensionnement (même règle que le peintre)
    v.labelSplit = nexus_.split;
    v.norm = link_.toNorm(shown_);
    if (effNorm_ >= 0.0f) v.effective = effNorm_;
    v.bipolar = bipolar(id_);
    v.zeroNorm = v.bipolar ? link_.toNorm(0.0f) : 0.0f;
    const auto& spec = paramSpec(id_);
    for (float off : routeOffsets_) v.routeDepths.push_back(link_.toNorm(juce::jlimit(spec.minValue, spec.maxValue, shown_ + off)) - v.norm);
    v.valueText = juce::String(formatValue(id_, shown_));
    v.inactive = inactive_;
    v.inactiveText = "INACTIF";  // texte 0.4.1 : INACTIVE est une proposition P2 non validée (state.inactive, B2-04)
    v.entry = false;  // la saisie numérique est un champ de texte posé sur la fenêtre de valeur
    v.disabled = !isEnabled();
    v.hover = isMouseOverOrDragging();
    v.focus = hasKeyboardFocus(false);
    v.pressed = dragging_ || wheelGesture_;
    sbd::r2::paintNexus(g, getLocalBounds(), v);
    // CE-B2-08 : indicateur MIDI Learn conservé (donnée de 0.4.1), corps 8,5 gras des marqueurs R1 (« +n », « INACTIVE »),
    // dans la sérigraphie à gauche du libellé quand la place est libre ; sinon infobulle seulement (arbitrage DB2-10) ;
    // masqué en Micro comme en 0.4.1 ; écart d'intégration mineur EC-21
    const auto tag = ccTagBox();
    if (!tag.isEmpty()) {
      g.setFont(sbd::r2::labelFont(8.5f, true));
      g.setColour(isEnabled() ? tok::textDim : tok::textMute);
      g.drawText("CC" + juce::String(static_cast<int>(ctx_.learnCc[static_cast<size_t>(idx(id_))])), tag, juce::Justification::centredLeft, false);
    }
    return;
  }
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

void ContinuousControl::refreshRouteDepths() {
  routeOffsets_.clear();
  const auto& spec = paramSpec(id_);
  for (const auto& r : ctx_.mod.routes)
    if (r.dest == id_ && r.enabled) routeOffsets_.push_back((r.invert ? -r.depth : r.depth) * spec.modScale);
}

void ContinuousControl::tick() {
  auto& t = ctx_.proc.telemetry();
  if (form_ == Form::Rotary && ctx_.modRevision != routeRevision_) {  // configuration publiée, fil message (aucune lecture audio)
    routeRevision_ = ctx_.modRevision;
    refreshRouteDepths();
    repaint();
  }
  const auto i = static_cast<size_t>(idx(id_));
  if (ctx_.learnCc[i] != learnShown_) {  // CC MIDI Learn publié par l'éditeur (10 Hz) : infobulle
    learnShown_ = ctx_.learnCc[i];
    refreshTooltip();
  }
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

void ContinuousControl::closeWheelGesture() {
  if (!wheelGesture_) return;
  stopTimer();
  wheelGesture_ = false;
  link_.end();
}

void ContinuousControl::mouseDown(const juce::MouseEvent& e) {
  closeWheelGesture();  // PB-01e
  if (ctx_.select) ctx_.select(id_);
  if (e.mods.isPopupMenu()) {
    showMenu();
    return;
  }
  if (e.mods.isAltDown()) {
    resetToDefault();
    return;
  }
  if (form_ != Form::Rotary) {
    // PLASMA (DB2-08) : seul un clic dans la zone de manipulation (canal et empreinte du curseur) devient un geste ;
    // libellé, valeur et hors canal : sélection et focus seulement, aucune écriture, aucun geste hôte
    if (!manipulationArea().contains(e.position)) return;
    dragging_ = true;
    dragStart_ = e.position;
    link_.begin();
    // clic = position absolue (DB2-02), rapportée au canal ; aucune écriture si la valeur ne change pas
    const auto a = channelArea();
    const float n = form_ == Form::LinearV ? (a.getBottom() - e.position.y) / a.getHeight() : (e.position.x - a.getX()) / a.getWidth();
    dragStartNorm_ = juce::jlimit(0.0f, 1.0f, n);
    if (std::abs(link_.norm() - dragStartNorm_) > 1.0e-6f) link_.during(link_.fromNorm(dragStartNorm_));
    return;
  }
  if (valueArea().contains(e.position)) return;  // double-clic sur la valeur = saisie
  dragging_ = true;
  dragStart_ = e.position;
  dragStartNorm_ = link_.norm();
  link_.begin();
}

void ContinuousControl::mouseDrag(const juce::MouseEvent& e) {
  if (!dragging_) return;
  const double fine = e.mods.isShiftDown() ? tok::fineFactor : 1.0;
  const auto d = e.position - dragStart_;
  double delta = 0.0;
  if (form_ == Form::Rotary) delta = (-d.y + d.x * 0.5) / tok::dragPixelsFull;
  else if (form_ == Form::LinearV) delta = -d.y / std::max(20.0, static_cast<double>(channelArea().getHeight()));  // longueur utile du canal
  else delta = d.x / std::max(20.0, static_cast<double>(channelArea().getWidth()));
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
  else if (form_ == Form::Rotary) resetToDefault();
  // PLASMA (DB2-09) : aucun retour au défaut par double-clic ; dans le canal, les deux clics ont déjà posé la position
  // absolue (une seule transaction) ; sur le libellé ou hors canal : rien. Défaut : Alt+clic, Suppr, menu.
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
  closeWheelGesture();  // PB-01e
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
  // NEXUS R1 : le champ prend la typographie de la fenêtre de valeur R1 qu'il recouvre
  ed->setFont(form_ == Form::Rotary ? sbd::r2::valueFont(sbd::r2::nexusValuePx(nexusSize(density_)))
                                    : (plasma_.r1 ? sbd::r2::valueFont(10.0f) : numFont(valueFont(density_))));
  ed->setJustification(juce::Justification::centred);
  ed->setText(juce::String(formatValue(id_, shown_)).upToFirstOccurrenceOf(" ", false, false), false);
  ed->setTitle(juce::String("Saisie : ") + paramSpec(id_).name);
  addAndMakeVisible(*ed);
  auto va = valueArea().toNearestInt();
  if (form_ == Form::LinearH && !plasma_.r1) va = va.withLeft(static_cast<int>(knobArea().getX()));
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
  if (r1()) return true;  // vérifié par le choix du rendu (une ligne dans la zone R1)
  juce::StringArray lines;
  float sz = 0.0f;
  return layoutLabel(caption_, cellTextArea(getLocalBounds()).getWidth(), 2, tok::fontLabel, lines, sz);
}

namespace {
// correspondance d'un accent 0.4.1 vers le domaine R1 (états et actions hors registre)
sbd::r2::Domain r2DomainOfColour(juce::Colour c) {
  if (c == tok::wt) return sbd::r2::Domain::Wt;
  if (c == tok::fm) return sbd::r2::Domain::Fm;
  if (c == tok::cross) return sbd::r2::Domain::Cross;
  if (c == tok::mod) return sbd::r2::Domain::Mod;
  if (c == tok::alert) return sbd::r2::Domain::Alert;
  return sbd::r2::Domain::Neutral;
}
// zones de texte du peintre NEURAL CELL (une ligne au corps 11 gras) : libellé et état
float r1CellLabelWidth(int w, sbd::r2::CellKind k, bool icon) {
  const float in = static_cast<float>(w) - 6.0f - 20.0f;  // r = lb.reduced(3, 4) ; in = r.reduced(10, 0)
  switch (k) {
    case sbd::r2::CellKind::Toggle: return in - 19.0f - 30.0f;
    case sbd::r2::CellKind::Arm: return in - 19.0f - 50.0f;
    case sbd::r2::CellKind::Action: return icon ? in - 23.0f : in;
    default: return in;
  }
}
juce::String cellStateText(CellState s, bool autonomyArm) {
  // texte d'état de production (0.4.1) : les termes R1 ARMED / ACTIVE sont des libellés P1 appliqués en B2-04
  switch (s) {
    case CellState::Off: return "OFF";
    case CellState::Armed: return juce::String::fromUTF8("ARMÉ");
    case CellState::Refused: return "REFUS";
    case CellState::Disabled: return juce::String::fromUTF8("\xe2\x80\x94");
    case CellState::Active: break;
  }
  return autonomyArm ? "ACTIF" : "ON";
}
}  // namespace

juce::String NeuralCell::r1BlockReason() const {
  const bool arm = id_ != ParamId::Count && isAutonomyArm(id_);
  const float w = r1CellLabelWidth(getWidth(), arm ? sbd::r2::CellKind::Arm : sbd::r2::CellKind::Toggle, false);
  const float lw = sbd::r2::draw::textWidth(sbd::r2::labelFont(sbd::r2::tok2::fontLabel, true), caption_);
  // même règle que les actions ; le témoin encastré (rayon 4) tient dans la face
  if (getHeight() < 24) return "hauteur " + juce::String(getHeight()) + " < 24";
  if (lw > w) return juce::String::fromUTF8("libellé « ") + caption_ + juce::String::fromUTF8(" » (") + juce::String(lw, 1) + " px) > zone R1 (" + juce::String(w, 1) + " px)";
  return {};
}
bool NeuralCell::r1() const { return r1BlockReason().isEmpty(); }
juce::String NeuralCell::stateText() const { return cellStateText(state_, id_ != ParamId::Count && isAutonomyArm(id_)); }

void NeuralCell::paintButton(juce::Graphics& g, bool over, bool down) {
  if (r1()) {  // NEURAL CELL Machined Titanium (peintre partagé) : Toggle, ou Arm pour les armements autonomes
    const bool arm = id_ != ParamId::Count && isAutonomyArm(id_);
    sbd::r2::CellView v;
    v.kind = arm ? sbd::r2::CellKind::Arm : sbd::r2::CellKind::Toggle;
    v.domain = id_ != ParamId::Count ? r2Domain(domainOf(id_)) : r2DomainOfColour(accent_);
    v.label = caption_;
    v.stateText = stateText();
    v.on = state_ == CellState::Active || (id_ == ParamId::Count && getToggleState());
    v.arm = state_ == CellState::Armed ? sbd::r2::CellArm::Armed
                                       : (state_ == CellState::Active ? sbd::r2::CellArm::Active
                                                                      : (state_ == CellState::Refused ? sbd::r2::CellArm::Denied : sbd::r2::CellArm::Off));
    v.disabled = !isEnabled() || state_ == CellState::Disabled;
    v.hover = over;
    v.pressed = down;
    v.focus = hasKeyboardFocus(false);
    sbd::r2::paintCell(g, getLocalBounds(), v);
    return;
  }
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

juce::String ActionCell::r1BlockReason() const {
  if (danger_) return "PANIC : B2-03.6";
  const auto kind = getClickingTogglesState() ? sbd::r2::CellKind::Toggle : sbd::r2::CellKind::Action;
  const float w = r1CellLabelWidth(getWidth(), kind, kind == sbd::r2::CellKind::Action && icon_ != Icon::None);
  const float lw = sbd::r2::draw::textWidth(sbd::r2::labelFont(sbd::r2::tok2::fontLabel, true), getButtonText());
  // face du gabarit (hauteur − 8) au moins égale à une ligne du libellé (11 gras × 1,2) + 2 : hauteur >= 24
  if (getHeight() < 24) return "hauteur " + juce::String(getHeight()) + " < 24";
  if (lw > w) return juce::String::fromUTF8("libellé « ") + getButtonText() + juce::String::fromUTF8(" » (") + juce::String(lw, 1) + " px) > zone R1 (" + juce::String(w, 1) + " px)";
  return {};
}
bool ActionCell::r1() const { return r1BlockReason().isEmpty(); }
bool ActionCell::textFits() const {
  if (r1()) return true;
  juce::StringArray lines;
  float sz = 0.0f;
  return layoutLabel(getButtonText(), getLocalBounds().toFloat().reduced(1.5f).getWidth() - 8.0f, 2, tok::fontLabel + (danger_ ? 1.0f : 0.0f), lines, sz);
}

void ActionCell::paintButton(juce::Graphics& g, bool over, bool down) {
  if (r1()) {  // NEURAL CELL R1 : action momentanée = touche Action ; bascule = Toggle (témoin et ON / OFF)
    sbd::r2::CellView v;
    const bool toggle = getClickingTogglesState();
    v.kind = toggle ? sbd::r2::CellKind::Toggle : sbd::r2::CellKind::Action;
    v.domain = r2DomainOfColour(accent_);
    v.label = getButtonText();
    v.on = getToggleState();
    v.stateText = toggle ? (getToggleState() ? "ON" : "OFF") : juce::String();
    v.icon = toggle ? sbd::r2::CellIcon::None
                    : (icon_ == Icon::Capture ? sbd::r2::CellIcon::Capture
                                              : (icon_ == Icon::Authorize ? sbd::r2::CellIcon::Authorize
                                                                          : (icon_ == Icon::Pulse ? sbd::r2::CellIcon::Pulse : sbd::r2::CellIcon::None)));
    v.disabled = !isEnabled();
    v.hover = over;
    v.pressed = down;
    v.focus = hasKeyboardFocus(false);
    sbd::r2::paintCell(g, getLocalBounds(), v);
    return;
  }
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
