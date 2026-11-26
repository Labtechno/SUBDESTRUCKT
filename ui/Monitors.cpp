#include "Monitors.h"

#include <cmath>
#include <complex>

#include "Widgets.h"
#include "dsp/Fft.h"

namespace sbd::ui {

void ScopeView::refresh() {
  if (!isVisible()) return;
  auto& ring = proc_.telemetry().scope;
  const std::uint32_t w = ring.write.load(std::memory_order_acquire);
  const int n = static_cast<int>(buf_.size());
  for (int i = 0; i < n; ++i) buf_[static_cast<size_t>(i)] = ring.data[(w - static_cast<std::uint32_t>(n) + static_cast<std::uint32_t>(i)) % ScopeRing::kSize].load(std::memory_order_relaxed);
  if (spectrum_) {
    static Fft fft(2048);
    std::vector<std::complex<double>> c(2048);
    for (int i = 0; i < 2048; ++i) c[static_cast<size_t>(i)] = buf_[static_cast<size_t>(i)] * (0.5 - 0.5 * std::cos(2.0 * 3.14159265358979 * i / 2048.0));
    fft.transform(c.data(), false);
    for (int k = 0; k < 1024; ++k) {
      const double m = 20.0 * std::log10(std::abs(c[static_cast<size_t>(k)]) * 4.0 / 2048.0 + 1e-9);
      mag_[static_cast<size_t>(k)] = static_cast<float>(0.7 * mag_[static_cast<size_t>(k)] + 0.3 * m);
    }
  }
  repaint();
}

void ScopeView::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat();
  g.setColour(col::display);
  g.fillRoundedRectangle(r, 3.0f);
  g.setColour(col::control.withAlpha(0.6f));
  for (int k = 1; k < 4; ++k) g.drawHorizontalLine(static_cast<int>(r.getY() + r.getHeight() * k / 4), r.getX(), r.getRight());
  const double fsHost = proc_.telemetry().hostRate.load();
  juce::Path p;
  if (!spectrum_) {
    // déclenchement sur front montant pour une image stable
    int start = 0;
    for (int i = 1; i < 1024; ++i)
      if (buf_[static_cast<size_t>(i - 1)] < 0.0f && buf_[static_cast<size_t>(i)] >= 0.0f) {
        start = i;
        break;
      }
    for (int i = 0; i < 1024; ++i) {
      const float x = r.getX() + r.getWidth() * i / 1023.0f;
      const float y = r.getCentreY() - juce::jlimit(-1.0f, 1.0f, buf_[static_cast<size_t>(start + i)]) * r.getHeight() * 0.45f;
      if (i == 0) p.startNewSubPath(x, y); else p.lineTo(x, y);
    }
    g.setColour(col::wt);
    g.strokePath(p, juce::PathStrokeType(1.3f));
    g.setColour(col::dim);
    g.setFont(mono(10.0f));
    g.drawText("SCOPE  sortie moteur (avant gain), +-1 = 0 dBFS", r.reduced(4.0f), juce::Justification::topLeft);
  } else {
    const double fs = fsHost > 0 ? fsHost / 2.0 : 24000.0;  // capture décimée par 2
    for (int k = 1; k < 1024; ++k) {
      const double f = k * fs / 2048.0;
      const float x = r.getX() + r.getWidth() * static_cast<float>(std::log(f / 20.0) / std::log(fs / 2.0 / 20.0));
      const float y = r.getY() + r.getHeight() * juce::jlimit(0.0f, 1.0f, -mag_[static_cast<size_t>(k)] / 100.0f);
      if (k == 1) p.startNewSubPath(x, y); else p.lineTo(x, y);
    }
    g.setColour(col::fm);
    g.strokePath(p, juce::PathStrokeType(1.3f));
    g.setColour(col::dim);
    g.setFont(mono(10.0f));
    g.drawText("SPECTRE  sortie moteur, 20 Hz - " + juce::String(fs / 2000.0, 1) + " kHz, 0 a -100 dB", r.reduced(4.0f), juce::Justification::topLeft);
  }
}

void XYView::refresh() {
  if (!isVisible()) return;
  auto& t = proc_.telemetry();
  bx_ = proc_.parameters().base(idx(ParamId::WtX));
  by_ = proc_.parameters().base(idx(ParamId::WtY));
  ex_ = t.wtX.load();
  ey_ = t.wtY.load();
  z_ = t.wtZ.load();
  if (auto b = FactoryBanks::get(proc_.currentBank())) {
    nx_ = b->nx();
    ny_ = b->ny();
  }
  repaint();
}

void XYView::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat();
  g.setColour(col::display);
  g.fillRoundedRectangle(r, 3.0f);
  auto a = r.reduced(14.0f).withTrimmedRight(22.0f);
  g.setColour(col::control);
  for (int i = 0; i < nx_; ++i) {
    const float x = a.getX() + (nx_ > 1 ? a.getWidth() * i / (nx_ - 1.0f) : a.getWidth() * 0.5f);
    g.drawVerticalLine(static_cast<int>(x), a.getY(), a.getBottom());
  }
  for (int j = 0; j < ny_; ++j) {
    const float y = a.getBottom() - (ny_ > 1 ? a.getHeight() * j / (ny_ - 1.0f) : a.getHeight() * 0.5f);
    g.drawHorizontalLine(static_cast<int>(y), a.getX(), a.getRight());
  }
  auto pos = [&](float x, float y) {
    x = juce::jlimit(0.0f, 1.0f, x);
    y = juce::jlimit(0.0f, 1.0f, y);
    return juce::Point<float>(a.getX() + a.getWidth() * x, a.getBottom() - a.getHeight() * y);
  };
  const auto pb = pos(bx_, by_), pe = pos(ex_, ey_);
  g.setColour(col::wt.withAlpha(0.5f));
  g.drawEllipse(pb.x - 6, pb.y - 6, 12, 12, 1.5f);
  g.setColour(col::wt);
  g.fillEllipse(pe.x - 4, pe.y - 4, 8, 8);
  // Z : barre séparée (transformation, non stockée)
  auto zb = r.reduced(6.0f).removeFromRight(12.0f);
  g.setColour(col::control);
  g.fillRect(zb);
  g.setColour(col::cross);
  g.fillRect(zb.withTrimmedTop(zb.getHeight() * (1.0f - juce::jlimit(0.0f, 1.0f, z_))));
  g.setColour(col::dim);
  g.setFont(mono(10.0f));
  g.drawText("XY  banque " + juce::String(nx_) + "x" + juce::String(ny_) + "  o Base  * Effective  |  Z", r.reduced(4.0f), juce::Justification::topLeft);
}

void NetworkView::refresh() {
  if (!isVisible()) return;
  routes_ = proc_.currentRoutes();
  auto& t = proc_.telemetry();
  for (int s = 0; s < kMaxRoutes; ++s) {
    depth_[static_cast<size_t>(s)] = t.effectiveDepth[static_cast<size_t>(s)].load();
    gain_[static_cast<size_t>(s)] = t.edgeGain[static_cast<size_t>(s)].load();
  }
  repaint();
}

void NetworkView::paint(juce::Graphics& g) {
  auto r = getLocalBounds().toFloat();
  g.setColour(col::display);
  g.fillRoundedRectangle(r, 3.0f);
  auto a = r.reduced(26.0f);
  // WT à gauche, BUS à droite, OP1..OP4 en colonne centrale (CROSS au centre de la lecture)
  std::array<juce::Point<float>, kNumNodes> P;
  P[kNodeWT] = {a.getX(), a.getCentreY()};
  P[kNodeBUS] = {a.getRight(), a.getCentreY()};
  for (int k = 0; k < 4; ++k) P[static_cast<size_t>(1 + k)] = {a.getCentreX(), a.getY() + a.getHeight() * k / 3.0f};
  for (const auto& rt : routes_) {
    const int slot = routeSlot(rt.src, rt.dst);
    const auto from = P[static_cast<size_t>(rt.src)], to = P[static_cast<size_t>(portNode(rt.dst))];
    const float d = std::fabs(slot >= 0 ? depth_[static_cast<size_t>(slot)] : 0.0f), gn = slot >= 0 ? gain_[static_cast<size_t>(slot)] : 0.0f;
    const auto kind = slot >= 0 ? routeSlotInfo(slot).kind : RouteKind::Internal;
    const juce::Colour c = kind == RouteKind::Internal ? col::fm : col::cross;
    g.setColour(c.withAlpha(0.25f + 0.75f * juce::jlimit(0.0f, 1.0f, gn)));
    if (rt.src == portNode(rt.dst)) {
      g.drawEllipse(from.x + 8, from.y - 16, 16, 16, 1.0f + std::min(d, 4.0f));
    } else {
      juce::Path p;
      const auto mid = (from + to) * 0.5f + juce::Point<float>(0.0f, rt.src < portNode(rt.dst) ? -14.0f : 14.0f);
      p.startNewSubPath(from);
      p.quadraticTo(mid, to);
      const float th = 1.0f + std::min(d, 4.0f);
      if (rt.cls == CausalClass::Delayed) {
        const float dash[2] = {5.0f, 4.0f};
        juce::Path dashed;
        juce::PathStrokeType(th).createDashedStroke(dashed, p, dash, 2);
        g.fillPath(dashed);
      } else {
        g.strokePath(p, juce::PathStrokeType(th));
      }
      g.setFont(mono(9.0f));
      g.drawText(juce::String(portName(rt.dst)) + (rt.cls == CausalClass::Delayed ? " D" : " I"), juce::Rectangle<float>(mid.x - 30, mid.y - 6, 60, 12), juce::Justification::centred);
    }
  }
  for (int n = 0; n < kNumNodes; ++n) {
    const auto p = P[static_cast<size_t>(n)];
    g.setColour(n == kNodeWT ? col::wt : (n == kNodeBUS ? col::cross : col::fm));
    g.fillEllipse(p.x - 13, p.y - 13, 26, 26);
    g.setColour(col::display);
    g.setFont(sans(10.0f, true));
    g.drawText(nodeName(n), juce::Rectangle<float>(p.x - 13, p.y - 7, 26, 14), juce::Justification::centred);
  }
  g.setColour(col::dim);
  g.setFont(mono(10.0f));
  g.drawText("RESEAU FM/CROSS reel  trait plein = Immediate, tirets = Delayed (T2)", r.reduced(4.0f), juce::Justification::topLeft);
}

MonitorPanel::MonitorPanel(SubdestrucktProcessor& p) : scope_(p), xy_(p), net_(p) {
  const char* names[4] = {"SCOPE", "SPECTRE", "XY", "RESEAU"};
  for (int k = 0; k < 4; ++k) {
    b_[k].setButtonText(names[k]);
    b_[k].setClickingTogglesState(false);
    b_[k].onClick = [this, k] { show(k); };
    addAndMakeVisible(b_[k]);
  }
  addAndMakeVisible(scope_);
  addChildComponent(xy_);
  addChildComponent(net_);
  show(0);
}

void MonitorPanel::show(int k) {
  mode_ = k;
  for (int i = 0; i < 4; ++i) b_[i].setToggleState(i == k, juce::dontSendNotification);
  scope_.setVisible(k <= 1);
  scope_.setSpectrum(k == 1);
  xy_.setVisible(k == 2);
  net_.setVisible(k == 3);
}

void MonitorPanel::refresh() {
  if (mode_ <= 1) scope_.refresh();
  else if (mode_ == 2) xy_.refresh();
  else net_.refresh();
}

void MonitorPanel::resized() {
  auto r = getLocalBounds().reduced(4);
  auto top = r.removeFromTop(24);
  for (int k = 0; k < 4; ++k) b_[k].setBounds(top.removeFromLeft(top.getWidth() / (4 - k)).reduced(2, 0));
  r.removeFromTop(4);
  scope_.setBounds(r);
  xy_.setBounds(r);
  net_.setBounds(r);
}

void MonitorPanel::paint(juce::Graphics& g) {
  g.setColour(col::panel);
  g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
}

}  // namespace sbd::ui
