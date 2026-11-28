// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab — DEMO DATA (implémentation).
#include "app/DemoData.h"

#include <cmath>

namespace sbd::lab {

namespace t = sbd::ui::tok;
using r2::CyberDisplay;
using r2::DisplayData;

DemoValue::DemoValue(float lo, float hi, float def, float value, juce::String unit, int decimals, bool bipolar, int steps)
    : lo_(lo), hi_(hi), def_(def), n_(0.0f), unit_(std::move(unit)), dec_(decimals), steps_(steps), bip_(bipolar) {
  n_ = toNorm(value);
}

juce::String DemoValue::text(float norm) const {
  const float v = lo_ + (hi_ - lo_) * norm;
  if (unit_ == "dB" && v <= -47.99f) return "-inf dB";
  juce::String s = juce::String(std::abs(v), dec_);
  if (v < 0.0f && std::abs(v) >= 0.5f * std::pow(10.0f, static_cast<float>(-dec_))) s = juce::String::fromUTF8("\xe2\x88\x92") + s;  // U+2212
  else if (bip_ && v > 0.0f) s = "+" + s;
  return unit_.isEmpty() ? s : s + " " + unit_;
}

void DemoValue::set(float n) {
  n_ = juce::jlimit(0.0f, 1.0f, n);
  ++writes;
  if (onChange) onChange();
}
void DemoValue::begin() {
  ++begins;
  if (depth > 0) ++nested;
  ++depth;
  maxDepth = std::max(maxDepth, depth);
}
void DemoValue::during(float n) {
  if (depth == 0) ++writesOutside;
  set(n);
}
void DemoValue::end() {
  ++ends;
  depth = std::max(0, depth - 1);
}
void DemoValue::complete(float n) {
  begin();
  set(n);
  end();
}

std::vector<float> demoScope(int n, float phase) {
  std::vector<float> s(static_cast<size_t>(n));
  for (int k = 0; k < n; ++k) {
    const float x = static_cast<float>(k) / static_cast<float>(n) * juce::MathConstants<float>::twoPi * 3.0f + phase;
    // forme FM de démonstration : porteuse modulée en phase, légère asymétrie
    s[static_cast<size_t>(k)] = 0.78f * std::sin(x + 1.6f * std::sin(2.0f * x)) + 0.08f * std::sin(7.0f * x);
  }
  return s;
}

std::vector<float> demoSpectrumDb(const std::vector<float>& sig) {
  const int n = static_cast<int>(sig.size());
  const int bins = n / 2;
  std::vector<float> db(static_cast<size_t>(bins), -100.0f);
  for (int b = 1; b < bins; ++b) {
    double re = 0.0, im = 0.0;
    for (int k = 0; k < n; ++k) {
      const double w = 0.5 - 0.5 * std::cos(2.0 * juce::MathConstants<double>::pi * k / (n - 1));  // Hann
      const double a = 2.0 * juce::MathConstants<double>::pi * b * k / n;
      re += sig[static_cast<size_t>(k)] * w * std::cos(a);
      im -= sig[static_cast<size_t>(k)] * w * std::sin(a);
    }
    const double mag = std::sqrt(re * re + im * im) / (n * 0.25);
    db[static_cast<size_t>(b)] = static_cast<float>(juce::jlimit(-100.0, 0.0, 20.0 * std::log10(mag + 1.0e-9)));
  }
  return db;
}

std::vector<std::vector<float>> demoWavetable(int frames, int points) {
  std::vector<std::vector<float>> t(static_cast<size_t>(frames), std::vector<float>(static_cast<size_t>(points)));
  for (int f = 0; f < frames; ++f) {
    const float m = static_cast<float>(f) / static_cast<float>(frames - 1);
    for (int k = 0; k < points; ++k) {
      const float x = juce::MathConstants<float>::twoPi * static_cast<float>(k) / static_cast<float>(points - 1);
      float v = std::sin(x) * (1.0f - m);
      for (int h = 2; h <= 7; ++h) v += m * std::sin(static_cast<float>(h) * x) / static_cast<float>(h) * 0.9f;
      t[static_cast<size_t>(f)][static_cast<size_t>(k)] = juce::jlimit(-1.0f, 1.0f, v);
    }
  }
  return t;
}

std::vector<juce::Point<float>> demoCurve() { return {{0.0f, 0.0f}, {0.22f, 0.08f}, {0.45f, 0.35f}, {0.62f, 0.62f}, {0.8f, 0.78f}, {1.0f, 1.0f}}; }

DisplayData demoDisplay(CyberDisplay::Variant v, float phase) {
  DisplayData d;
  d.demo = true;
  using V = CyberDisplay::Variant;
  const auto rowColour = [](int k) { return k == 0 ? t::text : juce::Colour(); };
  switch (v) {
    case V::Numeric:
      d.title = "CUTOFF";
      d.source = "BASE value (parameter) / EFFECTIVE when modulated";
      d.value = "2.40";
      d.unit = "kHz";
      d.secondary = "BASE 2.40 kHz  /  EFF 3.12 kHz";
      break;
    case V::Scope:
      d.title = "SCOPE";
      d.source = "plugin output, post OUT GAIN and limiter, mono, 1 sample in 2";
      d.samples = demoScope(512, phase);
      break;
    case V::Spectrum: {
      d.title = "SPECTRUM";
      d.source = "computed from SCOPE ring: indicative, aliasing may affect the displayed band";
      // signal de démonstration : 220 Hz et harmoniques décroissantes, au taux décimé (fs hôte / 2 = 22 050 Hz)
      std::vector<float> sig(2048);
      for (size_t k = 0; k < sig.size(); ++k) {
        float v = 0.0f;
        for (int h = 1; h <= 40; ++h) {
          const float f = 220.0f * static_cast<float>(h) * (1.0f + 0.002f * static_cast<float>(h));
          if (f >= 11025.0f) break;
          v += 0.6f / static_cast<float>(h) * std::sin(juce::MathConstants<float>::twoPi * f * static_cast<float>(k) / 22050.0f + phase * static_cast<float>(h));
        }
        sig[k] = v;
      }
      d.spectrumDb = demoSpectrumDb(sig);
      d.decimatedRate = 22050.0f;
      break;
    }
    case V::Wavetable:
      d.title = "WAVETABLE";
      d.source = "bank frames (control thread) / X BASE and EFFECTIVE";
      d.frames = demoWavetable(14, 96);
      d.posBase = 0.38f;
      d.posEffective = 0.62f;
      break;
    case V::ModCurve:
      d.title = "MOD CURVE";
      d.source = "route curve (custom, 6 points)";
      d.curve = demoCurve();
      break;
    case V::Envelope:
      d.title = "ENV AMP";
      d.source = "shape from BASE parameters; level published for AMP only; stage not published";
      d.envLevel = 0.62f;
      break;
    case V::Meter:
      d.title = "OUTPUT PEAK";
      d.source = "MEASURE: block peak at plugin output (not a parameter)";
      d.peakL = -6.4f;
      d.peakR = -8.1f;
      d.holdL = -3.2f;
      d.holdR = -4.0f;
      break;
    case V::Autonomy:
      d.title = "AUTONOMY";
      d.source = "transport, authorization and output level are independent states";
      d.rows = {{"TRANSPORT", "STOP", 0, 0, juce::Colour()},
                {"AUTONOMY", "SUSPENDED", 0, 1, t::warn},
                {"ARMED", "2  D2 WT1, S2 GL1", 0, -1, t::warn},
                {"OUTPUT", juce::String::fromUTF8("SIGNAL  \xe2\x88\x92") + "18.4 dBFS", 0, 2, t::text},
                {"SOURCE", "explicit (last grant)", 1, -1, juce::Colour()}};
      break;
    case V::Diagnostic:
      d.title = "DIAGNOSTICS";
      d.source = "safety counters (cumulative), CPU of the last block only";
      d.rows = {{"CPU", "3.1 % last block", 0, -1, juce::Colour()},
                {"PANIC", "1", 0, -1, juce::Colour()},
                {"LIMITER", "12", 1, -1, juce::Colour()},
                {"ELP +24 dBFS", "0", 1, -1, juce::Colour()},
                {"NON-FINITE", "0", 1, -1, juce::Colour()},
                {"DROPPED EVENTS", "0", 1, -1, juce::Colour()}};
      for (size_t k = 0; k < d.rows.size(); ++k) d.rows[k].colour = rowColour(static_cast<int>(k));
      break;
  }
  return d;
}

juce::String platformLine() {
  const auto sans = juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 12.0f, juce::Font::plain));
  const auto mono = juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain));
  const auto dot = juce::String::fromUTF8("  \xc2\xb7  ");
  return juce::SystemStats::getOperatingSystemName() + dot + juce::SystemStats::getJUCEVersion() + dot + "JUCE software renderer, offscreen" + dot +
         "fonts: " + sans.getTypefacePtr()->getName() + " / " + mono.getTypefacePtr()->getName();
}

}  // namespace sbd::lab
