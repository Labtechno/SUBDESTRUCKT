#include "CurveEditors.h"

namespace sbd::ui {

// ================================================================ EnvelopeEditor
std::vector<ParamId> EnvelopeEditor::paramsOf(int env) {
  if (env == 0) return {ParamId::AmpDelay, ParamId::AmpAttack, ParamId::AmpHold, ParamId::AmpDecay, ParamId::AmpSustain, ParamId::AmpRelease};
  const int base = idx(ParamId::EnvWtDelay) + (env - 1) * (idx(ParamId::EnvFmDelay) - idx(ParamId::EnvWtDelay));
  std::vector<ParamId> v;
  for (int o = 0; o < 6; ++o) v.push_back(static_cast<ParamId>(base + o));
  return v;
}

EnvelopeEditor::EnvelopeEditor(UiContext& ctx) : ctx_(ctx) {
  setEnvelope(0);
  setTitle("Editeur d'enveloppe");
  setTooltip(juce::String::fromUTF8("Glisser les poignées : Attack, Decay/Sustain, Release, Delay, Hold (gestes hôte, Undo par geste)"));
}

void EnvelopeEditor::setEnvelope(int k) {
  env_ = juce::jlimit(0, 4, k);
  const auto ids = paramsOf(env_);
  for (size_t i = 0; i < 6; ++i) links_[i] = std::make_unique<ParamLink>(ctx_.proc, ids[i], [this] { repaint(); });
  repaint();
}

void EnvelopeEditor::geometry(std::array<juce::Point<float>, 6>& p) const {
  auto a = getLocalBounds().toFloat().reduced(10.0f).withTrimmedTop(14.0f);
  const float seg = a.getWidth() / 5.6f;
  const float n[6] = {links_[0]->norm(), links_[1]->norm(), links_[2]->norm(), links_[3]->norm(), links_[4]->norm(), links_[5]->norm()};
  const float sus = n[4];
  float x = a.getX();
  p[0] = {x += seg * 0.6f * n[0], a.getBottom()};      // fin du Delay
  p[1] = {x += seg * (0.15f + n[1]), a.getY()};         // crête (Attack)
  p[2] = {x += seg * 0.6f * n[2], a.getY()};            // fin du Hold
  p[3] = {x += seg * (0.15f + n[3]), a.getBottom() - sus * a.getHeight()};  // Decay -> Sustain
  p[4] = {x += seg * 0.9f, p[3].y};                     // tenue (illustrative, durée de note)
  p[5] = {x += seg * (0.15f + n[5]), a.getBottom()};    // Release
}

void EnvelopeEditor::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat().reduced(1.0f);
  g.setColour(tok::glass);
  g.fillRoundedRectangle(r, tok::radius);
  g.setColour(tok::glassEdge);
  g.drawRoundedRectangle(r.reduced(0.5f), tok::radius, 1.0f);
  static const char* names[5] = {"AMP", "WT", "FM", "AUX1", "AUX2"};
  g.setFont(uiFont(10.0f, true));
  g.setColour(tok::textDim);
  g.drawText(juce::String("ENVELOPPE ") + names[env_] + juce::String::fromUTF8("  //  DAHDSR (positions = plage du paramètre)"), r.reduced(8.0f, 2.0f).removeFromTop(14.0f),
             juce::Justification::centredLeft, false);
  std::array<juce::Point<float>, 6> p;
  geometry(p);
  auto a = getLocalBounds().toFloat().reduced(10.0f).withTrimmedTop(14.0f);
  juce::Path path;
  path.startNewSubPath(a.getX(), a.getBottom());
  for (auto& q : p) path.lineTo(q);
  juce::Path fill(path);
  fill.lineTo(p[5].x, a.getBottom());
  fill.closeSubPath();
  g.setColour(tok::mod.withAlpha(0.12f));
  g.fillPath(fill);
  g.setColour(tok::mod);
  g.strokePath(path, juce::PathStrokeType(2.0f));
  static const char* hn[6] = {"D", "A", "H", "D/S", "", "R"};
  for (int i = 0; i < 6; ++i) {
    if (i == 4) continue;
    g.setColour(drag_ == i ? tok::text : tok::mod);
    g.fillEllipse(p[static_cast<size_t>(i)].x - 4.5f, p[static_cast<size_t>(i)].y - 4.5f, 9.0f, 9.0f);
    g.setColour(tok::textDim);
    g.setFont(numFont(9.5f));
    g.drawText(hn[i], juce::Rectangle<float>(p[static_cast<size_t>(i)].x - 14.0f, p[static_cast<size_t>(i)].y - 18.0f, 28.0f, 12.0f), juce::Justification::centred, false);
  }
}

void EnvelopeEditor::mouseDown(const juce::MouseEvent& e) {
  std::array<juce::Point<float>, 6> p;
  geometry(p);
  drag_ = -1;
  float best = 14.0f;
  for (int i = 0; i < 6; ++i) {
    if (i == 4) continue;
    const float d = p[static_cast<size_t>(i)].getDistanceFrom(e.position);
    if (d < best) best = d, drag_ = i;
  }
  if (drag_ < 0) return;
  start_ = e.position;
  const int lx = drag_ == 0 ? 0 : drag_ == 1 ? 1 : drag_ == 2 ? 2 : drag_ == 3 ? 3 : 5;
  startNx_ = links_[static_cast<size_t>(lx)]->norm();
  startNy_ = links_[4]->norm();
  links_[static_cast<size_t>(lx)]->begin();
  if (drag_ == 3) links_[4]->begin();
  if (ctx_.select) ctx_.select(paramsOf(env_)[static_cast<size_t>(lx)]);
  repaint();
}

void EnvelopeEditor::mouseDrag(const juce::MouseEvent& e) {
  if (drag_ < 0) return;
  const auto a = getLocalBounds().toFloat().reduced(10.0f).withTrimmedTop(14.0f);
  const float seg = a.getWidth() / 5.6f * (drag_ == 0 || drag_ == 2 ? 0.6f : 1.0f);
  const float fine = e.mods.isShiftDown() ? 0.1f : 1.0f;
  const int lx = drag_ == 0 ? 0 : drag_ == 1 ? 1 : drag_ == 2 ? 2 : drag_ == 3 ? 3 : 5;
  auto& L = *links_[static_cast<size_t>(lx)];
  L.during(L.fromNorm(startNx_ + (e.position.x - start_.x) / seg * fine));
  if (drag_ == 3) links_[4]->during(links_[4]->fromNorm(startNy_ - (e.position.y - start_.y) / a.getHeight() * fine));
}

void EnvelopeEditor::mouseUp(const juce::MouseEvent&) {
  if (drag_ < 0) return;
  const int lx = drag_ == 0 ? 0 : drag_ == 1 ? 1 : drag_ == 2 ? 2 : drag_ == 3 ? 3 : 5;
  links_[static_cast<size_t>(lx)]->end();
  if (drag_ == 3) links_[4]->end();
  drag_ = -1;
  repaint();
}

// ================================================================ PointCurveEditor
PointCurveEditor::PointCurveEditor(juce::String title, juce::Colour accent, bool bipolarY) : title_(std::move(title)), accent_(accent), bipolar_(bipolarY) {
  setTitle(title_);
  setTooltip(juce::String::fromUTF8("Glisser un point ; double-clic : ajouter (vide) ou retirer (sur un point) ; validé au relâchement (Undo)"));
}

void PointCurveEditor::setPoints(int n, const float* x, const float* y) {
  if (drag_ >= 0) return;
  n_ = juce::jlimit(2, 8, n);
  for (int i = 0; i < n_; ++i) x_[static_cast<size_t>(i)] = x[i], y_[static_cast<size_t>(i)] = y[i];
  repaint();
}

juce::Point<float> PointCurveEditor::toScreen(float x, float y) const {
  const auto a = area();
  const float ny = bipolar_ ? (y + 1.0f) * 0.5f : y;
  return {a.getX() + x * a.getWidth(), a.getBottom() - ny * a.getHeight()};
}

void PointCurveEditor::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat().reduced(1.0f);
  g.setColour(tok::glass);
  g.fillRoundedRectangle(r, tok::radius);
  g.setColour(tok::glassEdge);
  g.drawRoundedRectangle(r.reduced(0.5f), tok::radius, 1.0f);
  g.setFont(uiFont(10.0f, true));
  g.setColour(tok::textDim);
  g.drawText(title_ + "  (" + juce::String(n_) + " points)", r.reduced(8.0f, 2.0f).removeFromTop(14.0f), juce::Justification::centredLeft, false);
  const auto a = area();
  g.setColour(tok::glassEdge);
  if (bipolar_) g.drawHorizontalLine(static_cast<int>(a.getCentreY()), a.getX(), a.getRight());
  juce::Path p;
  for (int i = 0; i < n_; ++i) {
    const auto q = toScreen(x_[static_cast<size_t>(i)], y_[static_cast<size_t>(i)]);
    i == 0 ? p.startNewSubPath(q) : p.lineTo(q);
  }
  g.setColour(accent_);
  g.strokePath(p, juce::PathStrokeType(2.0f));
  for (int i = 0; i < n_; ++i) {
    const auto q = toScreen(x_[static_cast<size_t>(i)], y_[static_cast<size_t>(i)]);
    g.setColour(i == drag_ ? tok::text : accent_);
    g.fillEllipse(q.x - 4.0f, q.y - 4.0f, 8.0f, 8.0f);
  }
}

void PointCurveEditor::mouseDown(const juce::MouseEvent& e) {
  drag_ = -1;
  float best = 12.0f;
  for (int i = 0; i < n_; ++i) {
    const float d = toScreen(x_[static_cast<size_t>(i)], y_[static_cast<size_t>(i)]).getDistanceFrom(e.position);
    if (d < best) best = d, drag_ = i;
  }
}

void PointCurveEditor::mouseDrag(const juce::MouseEvent& e) {
  if (drag_ < 0) return;
  const auto a = area();
  const auto i = static_cast<size_t>(drag_);
  float x = (e.position.x - a.getX()) / a.getWidth();
  float ny = (a.getBottom() - e.position.y) / a.getHeight();
  ny = juce::jlimit(0.0f, 1.0f, ny);
  // extrémités fixes en x ; points intérieurs strictement entre leurs voisins (x croissants)
  if (drag_ == 0) x = 0.0f;
  else if (drag_ == n_ - 1) x = 1.0f;
  else x = juce::jlimit(x_[i - 1] + 0.01f, x_[i + 1] - 0.01f, x);
  x_[i] = x;
  y_[i] = bipolar_ ? ny * 2.0f - 1.0f : ny;
  repaint();
}

void PointCurveEditor::mouseUp(const juce::MouseEvent&) {
  if (drag_ < 0) return;
  drag_ = -1;
  if (onCommit) onCommit(n_, x_.data(), y_.data());
  repaint();
}

void PointCurveEditor::mouseDoubleClick(const juce::MouseEvent& e) {
  for (int i = 1; i < n_ - 1; ++i)
    if (toScreen(x_[static_cast<size_t>(i)], y_[static_cast<size_t>(i)]).getDistanceFrom(e.position) < 10.0f) {
      for (int k = i; k < n_ - 1; ++k) x_[static_cast<size_t>(k)] = x_[static_cast<size_t>(k + 1)], y_[static_cast<size_t>(k)] = y_[static_cast<size_t>(k + 1)];
      --n_;
      if (onCommit) onCommit(n_, x_.data(), y_.data());
      repaint();
      return;
    }
  if (n_ >= 8) return;
  const auto a = area();
  const float x = juce::jlimit(0.01f, 0.99f, (e.position.x - a.getX()) / a.getWidth());
  float ny = juce::jlimit(0.0f, 1.0f, (a.getBottom() - e.position.y) / a.getHeight());
  int at = 1;
  while (at < n_ && x_[static_cast<size_t>(at)] < x) ++at;
  if (x - x_[static_cast<size_t>(at - 1)] < 0.01f || x_[static_cast<size_t>(at)] - x < 0.01f) return;
  for (int k = n_; k > at; --k) x_[static_cast<size_t>(k)] = x_[static_cast<size_t>(k - 1)], y_[static_cast<size_t>(k)] = y_[static_cast<size_t>(k - 1)];
  x_[static_cast<size_t>(at)] = x;
  y_[static_cast<size_t>(at)] = bipolar_ ? ny * 2.0f - 1.0f : ny;
  ++n_;
  if (onCommit) onCommit(n_, x_.data(), y_.data());
  repaint();
}

}  // namespace sbd::ui
