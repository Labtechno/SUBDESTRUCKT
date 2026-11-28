// SUBDESTRUCKT — ARCH-UI-04-R2-B1-R1 — planches de la campagne de raffinement artistique final (LAB-A01 à LAB-A08).
// Les comparatifs avant / après rendent la révision B1 et la révision R1 dans la MÊME exécution, avec les mêmes
// dimensions, valeurs, contextes et DEMO DATA. NEXUS et PLASMA : comparaison pixel à pixel avec les captures B1 publiées.
#include <cmath>

#include "app/Measure.h"
#include "app/Sheets.h"

namespace sbd::lab {

namespace t = sbd::ui::tok;
using namespace sbd::r2;
using NX = Nexus::Size;
using NK = NeuralCell::Kind;
using ST = SynapticTrack::Mode;
using DV = CyberDisplay::Variant;
using PO = PlasmaChannel::Orientation;
using R = juce::Rectangle<int>;
using RF = juce::Rectangle<float>;

namespace {

juce::String u8(const char* s) { return juce::String::fromUTF8(s); }
const juce::String kArrow = u8("\xe2\x86\x92"), kDot = u8("\xc2\xb7"), kMinus = u8("\xe2\x88\x92"), kEll = u8("\xe2\x80\xa6");
juce::Colour dim() { return t::textDim; }
juce::Colour mute() { return t::textMute; }
juce::Colour txt() { return t::text; }

// ================================================================ plaques R1 (nomenclature LAB-A08)
void wtPlate(Sheet& s, int x, int y) {
  s.plate(RF(static_cast<float>(x), static_cast<float>(y), 440.0f, 400.0f), "WAVETABLE", Domain::Wt, "X Y Z");
  auto& vx = s.val(0, 1, 0, 0.486f, "", 2);
  vx.effective(0.6f);
  s.add<Nexus>(R(x + 12, y + 30, 132, 150), NX::XL, Domain::Wt, "X", "WT X", vx);
  auto& vy = s.val(0, 1, 0, 0.22f, "", 2);
  s.add<Nexus>(R(x + 154, y + 30, 132, 150), NX::XL, Domain::Wt, "Y", "WT Y", vy);
  auto& vz = s.val(0, 1, 0, 0.35f, "", 2);
  vz.routes({0.15f});
  s.add<Nexus>(R(x + 296, y + 30, 132, 150), NX::XL, Domain::Wt, "WT Z", "WT Z Master (Z transform depth)", vz);
  auto wd = demoDisplay(DV::Wavetable, 0.0f);
  wd.posBase = 0.486f;
  wd.posEffective = 0.6f;
  s.add<CyberDisplay>(R(x + 10, y + 188, 304, 202), DV::Wavetable, Domain::Wt, wd);
  s.add<NeuralCell>(R(x + 322, y + 194, 108, 42), NK::Toggle, Domain::Wt, "SYNC", "WT Sync");
  s.add<SynapticTrack>(R(x + 322, y + 244, 108, 46), ST::Segments, Domain::Wt, "PH MODE", "WT Phase Mode", juce::StringArray{"FREE", "RESET"}).setIndex(1);
  auto& lv = s.val(0, 1, 1, 0.8f, "", 2);
  s.add<Nexus>(R(x + 343, y + 300, 64, 74), NX::Micro, Domain::Wt, "LVL", "WT Level", lv);
}

void crossPlate(Sheet& s, int x, int y) {
  s.plate(RF(static_cast<float>(x), static_cast<float>(y), 460.0f, 400.0f), "CROSS", Domain::Cross, "engine interaction");
  auto& f = s.val(0, 1, 1, 0.72f, "", 2);
  f.effective(0.55f);
  s.add<Nexus>(R(x + 12, y + 34, 84, 96), NX::S, Domain::Cross, "WT" + kArrow + "FM", "CROSS WT>FM (forward)", f);
  auto& r = s.val(0, 1, 1, 0.4f, "", 2);
  s.add<Nexus>(R(x + 104, y + 34, 84, 96), NX::S, Domain::Cross, "FM" + kArrow + "WT", "CROSS FM>WT (reverse)", r);
  auto& m = s.val(0, 1, 1, 0.64f, "", 2);
  m.effective(0.78f);
  s.add<PlasmaChannel>(R(x + 210, y + 30, 64, 180), PO::Vertical, Domain::Cross, "CROSS LVL", "CROSS Master (CROSS level)", m, juce::StringArray{"0", ".5", "1"});
  s.add<NeuralCell>(R(x + 364, y + 34, 88, 42), NK::Action, Domain::Cross, "RESET", "Cross Reset (clear CROSS history)");
  // section CROSS AUDIO (post-chaînes) : AMOUNT et MODE sous leur en-tête (LAB-A08)
  const float sy = static_cast<float>(y) + 140.0f;
  s.caption("CROSS AUDIO", RF(static_cast<float>(x) + 12.0f, sy, 120.0f, 14.0f), 10.5f, txt(), juce::Justification::centredLeft, true);
  s.caption("post-chain", RF(static_cast<float>(x) + 104.0f, sy, 90.0f, 14.0f), 9.0f, mute());
  s.line({static_cast<float>(x) + 12.0f, sy + 16.0f}, {static_cast<float>(x) + 200.0f, sy + 16.0f});
  auto& a = s.val(0, 1, 0.5f, 0.5f, "", 2);
  s.add<Nexus>(R(x + 12, y + 160, 84, 96), NX::S, Domain::Cross, "AMOUNT", "Cross Audio Amount (post-chain)", a);
  s.add<SynapticTrack>(R(x + 104, y + 170, 100, 46), ST::List, Domain::Cross, "MODE", "Cross Audio Mode (post-chain)",
                       juce::StringArray{"OFF", "XFADE", "RING", "AM"})
      .setIndex(1);
  auto& b = s.val(-1, 1, 0, -0.2f, "", 2, true);
  s.add<PlasmaChannel>(R(x + 12, y + 268, 436, 44), PO::Horizontal, Domain::Cross, "BALANCE  PRE-CHAIN", "WT/FM Balance (audio, pre-chain)", b,
                       juce::StringArray{"WT", "0", "FM"});
  s.add<SynapticTrack>(R(x + 12, y + 324, 436, 32), ST::Tabs, Domain::Cross, "", "CROSS sections",
                       juce::StringArray{"DIRECT", "WT" + kArrow + "FM", "FM" + kArrow + "WT", "BUS / CROSS AUDIO"})
      .setIndex(3);
  s.caption(u8("BALANCE : avant les chaînes · CROSS AUDIO : après les chaînes · CROSS LVL : niveau global CROSS"),
            RF(static_cast<float>(x) + 12.0f, static_cast<float>(y) + 364.0f, 436.0f, 28.0f), 9.5f, dim(), juce::Justification::topLeft);
}

void fmPlate(Sheet& s, int x, int y) {
  s.plate(RF(static_cast<float>(x), static_cast<float>(y), 420.0f, 400.0f), "FM", Domain::Fm, "4 operators");
  for (int k = 0; k < 4; ++k) {
    auto& rv = s.val(0.5f, 16, 1, 1.0f + static_cast<float>(k), "x", 2);
    s.add<Nexus>(R(x + 12 + k * 100, y + 34, 84, 96), NX::S, Domain::Fm, "RATIO", "OP" + juce::String(k + 1) + " Ratio", rv);
    auto& lv = s.val(0, 1, 1, 0.9f - 0.2f * static_cast<float>(k), "", 2);
    s.add<Nexus>(R(x + 22 + k * 100, y + 138, 64, 74), NX::Micro, Domain::Fm, "LVL", "OP" + juce::String(k + 1) + " Level", lv);
  }
  auto& fi = s.val(0, 8, 1, 2.4f, "rad", 2);
  fi.effective(0.42f).routes({0.1f, -0.2f});
  s.add<Nexus>(R(x + 12, y + 222, 132, 150), NX::XL, Domain::Fm, "FM INDEX", "FM OP2>OP1 (contextual)", fi);
  s.add<SynapticTrack>(R(x + 158, y + 228, 250, 46), ST::Segments, Domain::Fm, "MOD TYPE", "OP1 Mod Mode", juce::StringArray{"TZFM", "LIN FM", "EXP FM", "PM"});
  s.add<NeuralCell>(R(x + 158, y + 286, 120, 42), NK::Toggle, Domain::Fm, "SYNC", "OP1 Sync").setToggleState(true, juce::dontSendNotification);
  s.add<SynapticTrack>(R(x + 158, y + 336, 250, 46), ST::List, Domain::Fm, "TOPOLOGY", "FM topology preset", juce::StringArray{"STACK 4", "2 PAIRS", "3+1", "PARALLEL"});
}

void barStrip(Sheet& s, int x, int y, int w) {
  s.plate(RF(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), 64.0f), "", Domain::Neutral);
  s.add<SynapticTrack>(R(x + 14, y + 16, 400, 32), ST::Tabs, Domain::Neutral, "", "Spaces", juce::StringArray{"PERFORMANCE", "SOUND DESIGN", "ENGINEERING"});
  s.add<NeuralCell>(R(x + 430, y + 11, 116, 42), NK::Action, Domain::Neutral, "PRESETS", "Open preset browser");
  s.add<NeuralCell>(R(x + 552, y + 11, 100, 42), NK::Action, Domain::Neutral, "SAVE" + kEll, "Save preset");
  s.add<NeuralCell>(R(x + 658, y + 11, 88, 42), NK::Action, Domain::Neutral, "UNDO", "Undo");
  s.add<NeuralCell>(R(x + 752, y + 11, 88, 42), NK::Action, Domain::Neutral, "REDO", "Redo").setEnabled(false);
  const int r0 = x + w - 840;
  s.add<NeuralCell>(R(r0, y + 11, 150, 42), NK::Arm, Domain::Neutral, "DRONE", "Drone").setArmState(NeuralCell::ArmState::Armed);
  auto& au = s.add<NeuralCell>(R(r0 + 156, y + 11, 146, 42), NK::Action, Domain::Neutral, "AUTHORIZE", "Authorize autonomous states (this instance)");
  au.setIcon(NeuralCell::Icon::Authorize);
  s.add<NeuralCell>(R(r0 + 308, y + 11, 136, 42), NK::Toggle, Domain::Mod, "INSPECTOR", "Inspector").setToggleState(true, juce::dontSendNotification);
  auto pk = demoDisplay(DV::Numeric, 0.0f);
  pk.title = "PEAK";
  pk.source = "measure";
  pk.value = kMinus + "3.2";
  pk.unit = "dBFS";
  pk.measure = true;
  s.add<CyberDisplay>(R(r0 + 452, y + 2, 250, 60), DV::Numeric, Domain::Neutral, pk);
  s.add<NeuralCell>(R(r0 + 712, y + 11, 116, 42), NK::Safety, Domain::Alert, "PANIC", "Panic");
}

}  // namespace

// ================================================================ 1. planche complète finale
std::unique_ptr<Sheet> buildBoard() {
  auto s = std::make_unique<Sheet>("SUBDESTRUCKT R2 Component Lab  -  final (R1)",
                                   u8("Planche complète après LAB-A01 à LAB-A08 · NEXUS et PLASMA validés · NEURAL CELL Machined Titanium · SYNAPTIC Machined Track · "
                                      "Cybernetic HUD · PANIC carbone · nomenclature P1 optimisée · aucune matière biomécanique hors Core"),
                                   1840, 1150);
  barStrip(*s, 16, 76, 1808);
  wtPlate(*s, 16, 152);
  crossPlate(*s, 466, 152);
  fmPlate(*s, 936, 152);
  s->add<CyberDisplay>(R(1368, 152, 456, 196), DV::Scope, Domain::Fm, demoDisplay(DV::Scope, 0.3f));
  s->add<CyberDisplay>(R(1368, 356, 456, 196), DV::Spectrum, Domain::Fm, demoDisplay(DV::Spectrum, 0.3f));
  // PROCESS (D2)
  s->plate(RF(16.0f, 566.0f, 600.0f, 566.0f), u8("PROCESS  ·  WT1 / D2 GLITCH"), Domain::Neutral, "pre Cross Audio");
  const juce::StringArray dtypes{"OFF", "N1 FOLD", "D1 CRUSH", "Y1 TRANSIENT", "S1 RING/SHIFT", "N2 FB SHAPER", "D2 GLITCH", "Y2 ENV SHAPER", "S2 SPECTRAL"};
  s->add<SynapticTrack>(R(30, 600, 200, 46), ST::List, Domain::Neutral, "TYPE", "DST WT1 Type", dtypes).setIndex(6);
  const char* d2[6] = {"SPEED", "REPEAT", "POS", "FDBK", "XFADE", "MIX"};
  for (int k = 0; k < 6; ++k) {
    auto& v = s->val(0, 1, 0, 0.2f + 0.12f * static_cast<float>(k), "", 2);
    s->add<Nexus>(R(30 + k * 96, 660, 84, 96), NX::S, Domain::Neutral, d2[k], juce::String("DST WT1 D2 ") + d2[k], v);
  }
  s->add<SynapticTrack>(R(30, 770, 220, 46), ST::Segments, Domain::Neutral, "FB MODE", "DST WT1 D2 Feedback Mode", juce::StringArray{"LEGACY", "REGEN"});
  auto& gv = s->val(-24, 24, 0, 0, "dB", 1, true);
  s->add<Nexus>(R(270, 760, 84, 96), NX::S, Domain::Neutral, "GAIN", "DST WT1 Output", gv);
  s->add<NeuralCell>(R(30, 868, 180, 42), NK::Toggle, Domain::Neutral, "FREEZE", "DST WT1 D2 Freeze").setToggleState(true, juce::dontSendNotification);
  s->add<NeuralCell>(R(220, 868, 180, 42), NK::Arm, Domain::Neutral, "ARM", "DST WT1 Autonomous Armed").setArmState(NeuralCell::ArmState::Armed);
  auto& cap = s->add<NeuralCell>(R(410, 868, 190, 42), NK::Action, Domain::Neutral, "CAPTURE", "Capture frozen D2/S2 content");
  cap.setIcon(NeuralCell::Icon::Capture);
  s->add<NeuralCell>(R(410, 918, 190, 42), NK::Action, Domain::Neutral, "EXCITE", "Manual excitation (F4)");
  s->caption(u8("FREEZE (bascule, automatisable) · ARM (configuration) · CAPTURE (action) : trois opérations, trois formes."),
             RF(30.0f, 966.0f, 570.0f, 20.0f), 10.0f, dim(), juce::Justification::topLeft);
  s->add<CyberDisplay>(R(30, 984, 570, 140), DV::Autonomy, Domain::Neutral, demoDisplay(DV::Autonomy, 0.0f));
  // MOD
  s->plate(RF(630.0f, 566.0f, 600.0f, 566.0f), u8("MOD  ·  ENVELOPES / LFO"), Domain::Mod);
  s->add<SynapticTrack>(R(644, 600, 400, 32), ST::Tabs, Domain::Mod, "", "Envelopes", juce::StringArray{"AMP", "WT", "FM", "AUX1", "AUX2"});
  s->add<CyberDisplay>(R(644, 640, 572, 190), DV::Envelope, Domain::Mod, demoDisplay(DV::Envelope, 0.0f));
  const char* env[4] = {"ATK", "DEC", "SUS", "REL"};
  for (int k = 0; k < 4; ++k) {
    auto& v = s->val(0, 1, 0, 0.15f + 0.2f * static_cast<float>(k), k == 2 ? "" : "s", 2);
    s->add<Nexus>(R(644 + k * 92, 840, 84, 96), NX::S, Domain::Mod, env[k], juce::String("Amp ") + env[k], v);
  }
  s->add<CyberDisplay>(R(1016, 840, 200, 160), DV::ModCurve, Domain::Mod, demoDisplay(DV::ModCurve, 0.0f));
  auto& rate = s->val(0.01f, 40, 1, 4.2f, "Hz", 2);
  rate.effective(0.3f);
  s->add<Nexus>(R(644, 950, 84, 96), NX::S, Domain::Mod, "RATE", "LFO1 Rate", rate);
  s->add<SynapticTrack>(R(740, 960, 260, 46), ST::Segments, Domain::Mod, "SHAPE", "LFO1 Shape", juce::StringArray{"SINE", "TRI", "SAW UP", "SQR"});
  s->add<NeuralCell>(R(740, 1020, 120, 42), NK::Toggle, Domain::Mod, "SYNC", "LFO1 Tempo Sync");
  // OUTPUT
  s->plate(RF(1244.0f, 566.0f, 580.0f, 566.0f), u8("OUTPUT  ·  SAFETY"), Domain::Neutral);
  auto& og = s->val(-48, 6, -6, -6, "dB", 1);
  s->add<PlasmaChannel>(R(1262, 600, 64, 180), PO::Vertical, Domain::Neutral, "OUT GAIN", "Output Gain", og, juce::StringArray{kMinus + "48", kMinus + "24", "0", "+6"});
  s->add<CyberDisplay>(R(1340, 600, 210, 210), DV::Meter, Domain::Neutral, demoDisplay(DV::Meter, 0.0f));
  s->add<NeuralCell>(R(1564, 604, 150, 42), NK::Toggle, Domain::Neutral, "LIMITER", "Musical Limiter").setToggleState(true, juce::dontSendNotification);
  auto& ce = s->val(-24, 0, -0.3f, -0.3f, "dB", 1);
  s->add<Nexus>(R(1564, 656, 84, 96), NX::S, Domain::Neutral, "CEILING", "Limiter Ceiling", ce);
  s->add<CyberDisplay>(R(1262, 824, 548, 294), DV::Diagnostic, Domain::Neutral, demoDisplay(DV::Diagnostic, 0.0f));
  return s;
}

namespace {

// ================================================================ comparatifs et non-régression
struct Diff {
  bool loaded = false, sameSize = false;
  juce::int64 differing = 0, total = 0;
  int maxDelta = 0;
};

Diff diffImages(const juce::Image& a, const juce::Image& b, int skipTop) {
  Diff d;
  d.loaded = a.isValid() && b.isValid();
  if (!d.loaded) return d;
  d.sameSize = a.getWidth() == b.getWidth() && a.getHeight() == b.getHeight();
  if (!d.sameSize) return d;
  const juce::Image::BitmapData ba(a, juce::Image::BitmapData::readOnly), bb(b, juce::Image::BitmapData::readOnly);
  for (int y = skipTop; y < a.getHeight(); ++y)
    for (int x = 0; x < a.getWidth(); ++x) {
      const auto p = ba.getPixelColour(x, y), q = bb.getPixelColour(x, y);
      const int dm = std::max({std::abs(p.getRed() - q.getRed()), std::abs(p.getGreen() - q.getGreen()), std::abs(p.getBlue() - q.getBlue())});
      ++d.total;
      if (dm > 0) {
        ++d.differing;
        d.maxDelta = std::max(d.maxDelta, dm);
      }
    }
  return d;
}

juce::String diffText(const Diff& d) {
  if (!d.loaded) return u8("référence B1 absente pour cette plateforme : comparaison non exécutée");
  if (!d.sameSize) return u8("dimensions différentes : comparaison impossible (ÉCHEC)");
  return juce::String(d.differing) + u8(" pixel(s) différent(s) sur ") + juce::String(d.total) + u8(" (hors bandeau d'en-tête), écart maximal ") +
         juce::String(d.maxDelta) + "/255" + (d.differing == 0 ? u8("  ->  IDENTIQUE") : u8("  ->  DIFFÉRENT"));
}

// avant (capture B1 publiée) | après (code R1) à mi-échelle, avec le résultat de la comparaison pixel à pixel
juce::Image composeBeforeAfter(const juce::Image& before, const juce::Image& after, const juce::String& title, const Diff& d) {
  const int half = after.getWidth() / 2, hh = after.getHeight() / 2, pad = 24;
  juce::Image out(juce::Image::RGB, pad * 3 + half * 2, 120 + hh + pad, true);
  juce::Graphics g(out);
  g.fillAll(t::chassis);
  g.setColour(t::text);
  g.setFont(labelFont(22.0f, true));
  g.drawText(title, pad, 14, out.getWidth() - 2 * pad, 28, juce::Justification::centredLeft);
  g.setFont(labelFont(13.0f, true));
  g.setColour(d.loaded && d.sameSize && d.differing == 0 ? t::wt : t::warn);
  g.drawText(diffText(d), pad, 46, out.getWidth() - 2 * pad, 18, juce::Justification::centredLeft);
  g.setFont(labelFont(10.0f));
  g.setColour(t::textMute);
  g.drawText(platformLine(), pad, 66, out.getWidth() - 2 * pad, 14, juce::Justification::centredLeft);
  g.setFont(labelFont(12.0f, true));
  g.setColour(t::textDim);
  g.drawText(u8("AVANT : capture B1 publiée (même plateforme)"), pad, 90, half, 18, juce::Justification::centredLeft);
  g.drawText(u8("APRÈS : code R1, même planche, mêmes valeurs"), pad * 2 + half, 90, half, 18, juce::Justification::centredLeft);
  if (before.isValid()) g.drawImage(before, pad, 112, half, hh, 0, 0, before.getWidth(), before.getHeight());
  g.drawImage(after, pad * 2 + half, 112, half, hh, 0, 0, after.getWidth(), after.getHeight());
  return out;
}

// colonne B1 / R1 : en-têtes et intitulés de lignes
void baColumns(Sheet& s, int xB1, int xR1, int w) {
  s.caption(u8("B1  —  AVANT"), RF(static_cast<float>(xB1), 72.0f, static_cast<float>(w), 16.0f), 12.0f, dim(), juce::Justification::centredLeft, true);
  s.caption(u8("R1  —  APRÈS"), RF(static_cast<float>(xR1), 72.0f, static_cast<float>(w), 16.0f), 12.0f, t::wt, juce::Justification::centredLeft, true);
  s.line({static_cast<float>(xR1) - 12.0f, 76.0f}, {static_cast<float>(xR1) - 12.0f, static_cast<float>(s.getHeight()) - 20.0f});
}

// ================================================================ 3. NEURAL CELL avant / après
std::unique_ptr<Sheet> sheetNeuralBA() {
  auto s = std::make_unique<Sheet>(u8("NEURAL CELL  —  Machined Titanium (LAB-A02), avant / après"),
                                   u8("Mêmes dimensions, mêmes états, même rendu · titane noir usiné, chanfreins, alvéole, témoins encastrés, course physique à l'appui · "
                                      "PANIC carbone (LAB-A07.1)"),
                                   1500, 980);
  const int xs[2] = {190, 860};
  baColumns(*s, xs[0], xs[1], 620);
  const juce::String rowNames[] = {"ACTION", "INTERRUPTEUR", "ARMEMENT", u8("SÉLECTION EXCLUSIVE"), u8("SÛRETÉ (PANIC)"), u8("DÉSACTIVÉE")};
  for (int c = 0; c < 2; ++c) {
    const Rev rev = c == 0 ? Rev::B1 : Rev::R1;
    const int x = xs[c];
    auto cell = [&](R b, NK k, Domain d, const juce::String& l, const juce::String& f) -> NeuralCell& {
      auto& n = s->add<NeuralCell>(b, k, d, l, f);
      n.setRevision(rev);
      return n;
    };
    cell(R(x, 100, 180, 42), NK::Action, Domain::Cross, "CROSS RESET", "Cross Reset");
    cell(R(x, 156, 180, 42), NK::Toggle, Domain::Neutral, "LIMITER", "Musical Limiter").setToggleState(true, juce::dontSendNotification);
    cell(R(x, 212, 180, 42), NK::Arm, Domain::Neutral, "ARM", "DST WT1 Autonomous Armed").setArmState(NeuralCell::ArmState::Armed);
    const char* sel[3] = {"LIN", "EXP", "LOG"};
    for (int k = 0; k < 3; ++k) {
      auto& n = cell(R(x + k * 112, 268, 106, 42), NK::Select, Domain::Mod, sel[k], juce::String("Curve ") + sel[k]);
      n.setRadioGroupId(10 + c);
      n.setToggleState(k == 1, juce::dontSendNotification);
    }
    cell(R(x, 324, 116, 42), NK::Safety, Domain::Alert, "PANIC", "Panic");
    auto& dis = cell(R(x, 380, 180, 42), NK::Action, Domain::Neutral, "CAPTURE", "Capture frozen D2/S2 content");
    dis.setIcon(NeuralCell::Icon::Capture);
    dis.setEnabled(false);
    // FREEZE / ARM / AUTHORIZE / CAPTURE
    const NK ks[4] = {NK::Toggle, NK::Arm, NK::Action, NK::Action};
    const char* ls[4] = {"FREEZE", "ARM", "AUTHORIZE", "CAPTURE"};
    for (int k = 0; k < 4; ++k) {
      auto& n = cell(R(x + k * 152, 470, 146, 42), ks[k], Domain::Neutral, ls[k], ls[k]);
      if (k == 0) n.setToggleState(true, juce::dontSendNotification);
      if (k == 1) n.setArmState(NeuralCell::ArmState::Armed);
      if (k == 2) n.setIcon(NeuralCell::Icon::Authorize);
      if (k == 3) n.setIcon(NeuralCell::Icon::Capture);
    }
    // états d'une action
    const Forced fs[5] = {Forced::Normal, Forced::Hover, Forced::Pressed, Forced::Focus, Forced::Disabled};
    for (int k = 0; k < 5; ++k) cell(R(x + k * 122, 580, 116, 42), NK::Action, Domain::Wt, "APPLY", "Apply drawn wave").setForced(fs[k]);
    // états d'une bascule
    for (int k = 0; k < 5; ++k) {
      auto& n = cell(R(x + k * 122, 660, 116, 42), NK::Toggle, Domain::Wt, "SYNC", "WT Sync");
      n.setForced(fs[k]);
      if (k == 1 || k == 3) n.setToggleState(true, juce::dontSendNotification);
    }
    // armement : OFF, ARMED, ACTIVE, DENIED
    const NeuralCell::ArmState as[4] = {NeuralCell::ArmState::Off, NeuralCell::ArmState::Armed, NeuralCell::ArmState::Active, NeuralCell::ArmState::Denied};
    for (int k = 0; k < 4; ++k) cell(R(x + k * 152, 740, 146, 42), NK::Arm, Domain::Neutral, "DRONE", "Drone").setArmState(as[k]);
    // PANIC : états
    for (int k = 0; k < 4; ++k) cell(R(x + k * 122, 830, 116, 42), NK::Safety, Domain::Alert, "PANIC", "Panic").setForced(fs[k]);
  }
  const int ry[] = {100, 156, 212, 268, 324, 380};
  for (int k = 0; k < 6; ++k) s->caption(rowNames[k], RF(24.0f, static_cast<float>(ry[k]) + 12.0f, 160.0f, 16.0f), 10.5f, txt(), juce::Justification::centredLeft, true);
  s->caption("FREEZE / ARM /\nAUTHORIZE / CAPTURE", RF(24.0f, 470.0f, 160.0f, 40.0f), 10.5f, txt(), juce::Justification::centredLeft, true);
  s->caption(u8("ACTION : normal,\nsurvol, appui,\nfocus, désactivé"), RF(24.0f, 572.0f, 160.0f, 56.0f), 10.0f, dim(), juce::Justification::centredLeft);
  s->caption(u8("BASCULE : mêmes\nétats (ON / OFF)"), RF(24.0f, 660.0f, 160.0f, 40.0f), 10.0f, dim(), juce::Justification::centredLeft);
  s->caption(u8("ARMEMENT : OFF,\nARMED, ACTIVE, DENIED"), RF(24.0f, 740.0f, 160.0f, 40.0f), 10.0f, dim(), juce::Justification::centredLeft);
  s->caption(u8("PANIC : normal,\nsurvol, appui, focus"), RF(24.0f, 830.0f, 160.0f, 40.0f), 10.0f, dim(), juce::Justification::centredLeft);
  s->caption(u8("Course physique : la face s'enfonce d'un pixel et l'ombre portée disparaît à l'appui. Les dimensions sont inchangées ; "
                "les gestes et la sémantique ne changent pas. FREEZE (bascule), ARM (liseré ambre, témoin), AUTHORIZE (action, icône ambre) et "
                "CAPTURE (action, icône d'instantané) restent distincts."),
             RF(24.0f, 900.0f, 1450.0f, 60.0f), 10.5f, dim(), juce::Justification::topLeft);
  return s;
}

// ================================================================ 4. SYNAPTIC TRACK avant / après
std::unique_ptr<Sheet> sheetTrackBA() {
  auto s = std::make_unique<Sheet>(u8("SYNAPTIC TRACK  —  Machined Track (LAB-A03), avant / après"),
                                   u8("Rail mécanique encastré, sélecteur en titane, marqueur du domaine · plus sobre que NEURAL CELL · dimensions, indices et "
                                      "comportements inchangés"),
                                   1500, 700);
  const int xs[2] = {190, 860};
  baColumns(*s, xs[0], xs[1], 620);
  for (int c = 0; c < 2; ++c) {
    const Rev rev = c == 0 ? Rev::B1 : Rev::R1;
    const int x = xs[c];
    auto tr = [&](R b, ST m, Domain d, const juce::String& l, juce::StringArray o) -> SynapticTrack& {
      auto& t2 = s->add<SynapticTrack>(b, m, d, l, l, std::move(o));
      t2.setRevision(rev);
      return t2;
    };
    tr(R(x, 100, 200, 46), ST::Segments, Domain::Neutral, "FB MODE", {"LEGACY", "REGEN"}).setIndex(0);
    tr(R(x + 216, 100, 240, 46), ST::Segments, Domain::Neutral, "FFT", {"512", "1024", "2048", "4096"}).setIndex(2);
    tr(R(x, 162, 300, 46), ST::Segments, Domain::Mod, "SHAPE", {"SINE", "TRI", "SAW UP", "SQR", "S&H"}).setIndex(2);
    tr(R(x + 316, 162, 290, 46), ST::Segments, Domain::Fm, "MOD TYPE", {"TZFM", "LIN FM", "EXP FM", "PM"}).setIndex(0);
    tr(R(x, 236, 168, 46), ST::List, Domain::Neutral, "TYPE", {"OFF", "N1 FOLD", "D1 CRUSH", "Y1 TRANSIENT", "S1 RING/SHIFT", "N2 FB SHAPER", "D2 GLITCH", "Y2 ENV SHAPER", "S2 SPECTRAL"})
        .setIndex(6);
    tr(R(x + 184, 236, 228, 46), ST::List, Domain::Neutral, "TYPE (host)", {"Off", "N1 Asymmetric Wavefolder", "D1 Variable Rate Crusher", "Y1 Transient Destroyer", "S1 Ring/Frequency Shifter"})
        .setIndex(4);
    tr(R(x + 428, 236, 168, 46), ST::List, Domain::Cross, "MODE", {"OFF", "XFADE", "RING", "AM"}).setIndex(1);
    tr(R(x, 310, 560, 32), ST::Tabs, Domain::Cross, "", {"NETWORK", "MODULATION", "BANK LAB", "STATE", "TOOLS"}).setIndex(0);
    tr(R(x, 356, 400, 32), ST::Tabs, Domain::Cross, "", {"DIRECT", "WT" + kArrow + "FM", "FM" + kArrow + "WT", "BUS / CROSS AUDIO"}).setIndex(3);
    auto& h = tr(R(x, 420, 200, 46), ST::Segments, Domain::Wt, "INTERP", {"LIN", "CUBIC", "SPECTRAL"});
    h.setIndex(0);
    h.setForced(Forced::Hover, 2);
    tr(R(x + 216, 420, 160, 46), ST::Segments, Domain::Wt, "SYNC", {"OFF", "ON"}).setForced(Forced::Focus);
    tr(R(x + 392, 420, 180, 46), ST::Segments, Domain::Cross, "DIR", {"FWD", "REV", "BOTH"}).setForced(Forced::Disabled);
    tr(R(x, 494, 200, 32), ST::Tabs, Domain::Cross, "", {"NEURAL", "TABLE"}).setIndex(0);
    auto& lf = tr(R(x + 216, 486, 168, 46), ST::List, Domain::Mod, "CURVE", {"LIN", "EXP", "LOG", "S-CURVE"});
    lf.setIndex(1);
    lf.setForced(Forced::Focus);
    tr(R(x + 400, 486, 168, 46), ST::List, Domain::Mod, "SOURCE", {"ENV AMP", "LFO1", "S&H", "M1"}).setForced(Forced::Disabled);
  }
  const juce::String rowNames[] = {"SEGMENTS", "SEGMENTS", "LISTES", "ONGLETS", "ONGLETS", u8("ÉTATS"), u8("DUAL VIEW,\nFOCUS, INDISPONIBLE")};
  const int ry[] = {100, 162, 236, 310, 356, 420, 486};
  for (int k = 0; k < 7; ++k) s->caption(rowNames[k], RF(24.0f, static_cast<float>(ry[k]) + 10.0f, 160.0f, 32.0f), 10.5f, txt(), juce::Justification::topLeft, true);
  s->caption(u8("États : survol sur SPECTRAL, focus clavier, indisponible (hachures, sans marqueur de domaine). Les libellés gardent toute leur surface utile ; "
                "aucun ornement ajouté dans la piste."),
             RF(24.0f, 560.0f, 1450.0f, 40.0f), 10.5f, dim(), juce::Justification::topLeft);
  return s;
}

// ================================================================ 5. Cybernetic HUD
std::unique_ptr<Sheet> sheetHud() {
  auto s = std::make_unique<Sheet>(u8("Cybernetic HUD  (LAB-A04)  —  neuf variantes"),
                                   u8("Verre optique profond, monture usinée, repères d'angle, rails gradués, réticules, typographie technique · nature de la donnée : "
                                      "BASE (carré), EFF (losange), MEAS (crochets), INDICATIVE, STATE · aucune donnée inventée, aucune animation"),
                                   1340, 830);
  const DV vs[9] = {DV::Numeric, DV::Scope, DV::Spectrum, DV::Wavetable, DV::ModCurve, DV::Envelope, DV::Meter, DV::Autonomy, DV::Diagnostic};
  const Domain ds[9] = {Domain::Neutral, Domain::Fm, Domain::Fm, Domain::Wt, Domain::Mod, Domain::Mod, Domain::Neutral, Domain::Neutral, Domain::Neutral};
  for (int k = 0; k < 9; ++k) s->add<CyberDisplay>(R(16 + (k % 3) * 440, 80 + (k / 3) * 248, 428, 236), vs[k], ds[k], demoDisplay(vs[k], 0.7f));
  return s;
}

std::unique_ptr<Sheet> sheetHudBA() {
  auto s = std::make_unique<Sheet>(u8("Cybernetic HUD  —  avant / après, neuf variantes"),
                                   u8("Mêmes dimensions (440 x 210), mêmes DEMO DATA, même rendu · B1 à gauche, R1 à droite"), 1120, 2090);
  baColumns(*s, 150, 620, 460);
  const DV vs[9] = {DV::Numeric, DV::Scope, DV::Spectrum, DV::Wavetable, DV::ModCurve, DV::Envelope, DV::Meter, DV::Autonomy, DV::Diagnostic};
  const Domain ds[9] = {Domain::Neutral, Domain::Fm, Domain::Fm, Domain::Wt, Domain::Mod, Domain::Mod, Domain::Neutral, Domain::Neutral, Domain::Neutral};
  const juce::String names[9] = {"1. VALEUR", "2. OSCILLO", "3. SPECTRE\nINDICATIF", "4. WAVETABLE", "5. COURBE", "6. ENVELOPPE", u8("7. CRÊTE"), "8. AUTONOMIE", "9. DIAGNOSTIC"};
  for (int k = 0; k < 9; ++k) {
    const int y = 96 + k * 222;
    s->caption(names[k], RF(20.0f, static_cast<float>(y) + 80.0f, 120.0f, 36.0f), 11.0f, txt(), juce::Justification::topLeft, true);
    s->add<CyberDisplay>(R(150, y, 440, 210), vs[k], ds[k], demoDisplay(vs[k], 0.7f)).setRevision(Rev::B1);
    s->add<CyberDisplay>(R(620, y, 440, 210), vs[k], ds[k], demoDisplay(vs[k], 0.7f)).setRevision(Rev::R1);
  }
  return s;
}

// ================================================================ 6. PLASMA STANDARD / COMPACT
struct PlasmaCase {
  const char *label, *full;
  float lo, hi, def, v;
  const char* unit;
  int dec;
  bool bip;
  float eff;
  Domain d;
};
const PlasmaCase kPlasma[5] = {{"OUT GAIN", "Output Gain", -48, 6, -6, -6, "dB", 1, false, -1, Domain::Neutral},
                               {"CROSS LVL", "CROSS Master (CROSS level)", 0, 1, 1, 0.64f, "", 2, false, 0.78f, Domain::Cross},
                               {"BALANCE", "WT/FM Balance (audio, pre-chain)", -1, 1, 0, -0.35f, "", 2, true, 0.6f, Domain::Cross},
                               {"M2", "Macro 2", 0, 1, 0, 0.0f, "", 2, false, -1, Domain::Mod},
                               {"LVL", "WT Level", 0, 1, 0, 1.0f, "", 2, false, -1, Domain::Wt}};
juce::StringArray plasmaGrads(int k) {
  if (k == 0) return {u8("\xe2\x88\x92") + "48", u8("\xe2\x88\x92") + "24", "0", "+6"};
  if (k == 1) return {"0", ".5", "1"};
  if (k == 2) return {"WT", "0", "FM"};
  return {"0", "1"};
}
void addPlasma(Sheet& s, R b, int k) {
  const auto& c = kPlasma[k];
  auto& v = s.val(c.lo, c.hi, c.def, c.v, c.unit, c.dec, c.bip);
  if (c.eff >= 0.0f) v.effective(c.eff);
  s.add<PlasmaChannel>(b, PO::Vertical, c.d, c.label, c.full, v, plasmaGrads(k));
}

std::unique_ptr<Sheet> sheetPlasmaSC() {
  auto s = std::make_unique<Sheet>(u8("PLASMA CHANNEL  —  STANDARD 64 x 180 et COMPACT 48 x 150 (LAB-A07.2)"),
                                   u8("Même composant, même contrat d'interaction · apparence validée (LAB-A05) · COMPACT : valeur, identification, BASE, EFFECTIVE, "
                                      "graduations essentielles, zone de manipulation · corps de texte inchangé"),
                                   1340, 640);
  s->plate(RF(16.0f, 76.0f, 1308.0f, 300.0f), u8("STANDARD  |  COMPACT  (paires, mêmes valeurs)"), Domain::Neutral);
  const juce::String cap[5] = {u8("dB, non modulé"), u8("modulé (EFFECTIVE)"), u8("bipolaire modulé"), "minimum", "maximum"};
  for (int k = 0; k < 5; ++k) {
    const int x = 40 + k * 256;
    addPlasma(*s, R(x, 112, 64, 180), k);
    addPlasma(*s, R(x + 96, 142, 48, 150), k);
    s->caption("64 x 180", RF(static_cast<float>(x), 298.0f, 80.0f, 12.0f), 9.0f, mute());
    s->caption("48 x 150", RF(static_cast<float>(x) + 96.0f, 298.0f, 80.0f, 12.0f), 9.0f, mute());
    s->caption(cap[k], RF(static_cast<float>(x), 318.0f, 220.0f, 14.0f), 10.0f, dim());
  }
  s->plate(RF(16.0f, 390.0f, 1308.0f, 230.0f), u8("COMPACT : états"), Domain::Neutral);
  const Forced fs[5] = {Forced::Normal, Forced::Hover, Forced::Focus, Forced::Pressed, Forced::Disabled};
  const char* fn[5] = {"NORMAL", "HOVER", "FOCUS", "PRESSED", "DISABLED"};
  for (int k = 0; k < 5; ++k) {
    auto& v = s->val(0, 1, 1, 0.64f, "", 2);
    if (k == 0) v.effective(0.8f);
    s->add<PlasmaChannel>(R(48 + k * 120, 424, 48, 150), PO::Vertical, Domain::Cross, "CROSS LVL", "CROSS Master", v, juce::StringArray{"0", ".5", "1"}).setForced(fs[k]);
    s->caption(fn[k], RF(48.0f + static_cast<float>(k) * 120.0f, 580.0f, 100.0f, 12.0f), 9.5f, mute());
  }
  s->caption(u8("COMPACT montre les bornes et le zéro de l'échelle ; STANDARD montre toutes les graduations. Le libellé passe sur deux lignes au "
                "même corps (11 px) plutôt que d'être réduit. Le choix de l'emplacement et des seuils de disposition appartient aux lots "
                "d'intégration et d'organisation spatiale (R2-B2, R2-C)."),
             RF(680.0f, 430.0f, 620.0f, 120.0f), 10.5f, dim(), juce::Justification::topLeft);
  return s;
}

// zooms 75 / 100 / 150 % en pixels écran réels (densité x1) d'un groupe de composants
juce::Image composeZoom(const juce::String& title, const juce::String& subtitle, const std::function<std::unique_ptr<Sheet>()>& make) {
  const float zs[3] = {0.75f, 1.0f, 1.5f};
  std::vector<juce::Image> imgs;
  for (float z : zs) {
    auto c = make();
    imgs.push_back(c->createComponentSnapshot(c->getLocalBounds(), true, z));
  }
  int w = 40, h = 0;
  for (auto& i : imgs) {
    w += i.getWidth() + 40;
    h = std::max(h, i.getHeight());
  }
  juce::Image out(juce::Image::RGB, w, h + 150, true);
  juce::Graphics g(out);
  g.fillAll(t::chassis);
  g.setFont(labelFont(20.0f, true));
  g.setColour(t::text);
  g.drawText(title, 24, 12, w - 48, 26, juce::Justification::centredLeft);
  g.setFont(labelFont(11.0f));
  g.setColour(t::textDim);
  g.drawText(subtitle, 24, 40, w - 48, 16, juce::Justification::centredLeft);
  g.setFont(labelFont(9.0f));
  g.setColour(t::textMute);
  g.drawText(platformLine(), 24, 58, w - 48, 12, juce::Justification::centredLeft);
  int x = 40;
  for (int k = 0; k < 3; ++k) {
    g.setFont(labelFont(12.0f, true));
    g.setColour(t::text);
    g.drawText(juce::String(static_cast<int>(zs[k] * 100.0f)) + " %", x, 84, 200, 16, juce::Justification::centredLeft);
    g.setFont(labelFont(10.0f));
    g.setColour(t::textDim);
    g.drawText(u8("libellé 11 → ") + juce::String(11.0f * zs[k], 2) + u8(" px · critique 12 → ") + juce::String(12.0f * zs[k], 2) + " px", x, 102, 400, 14,
               juce::Justification::centredLeft);
    g.drawImageAt(imgs[static_cast<size_t>(k)], x, 126);
    x += imgs[static_cast<size_t>(k)].getWidth() + 40;
  }
  return out;
}

std::unique_ptr<Sheet> plasmaZoomCluster() {
  auto s = std::make_unique<Sheet>("", "", 400, 210);
  addPlasma(*s, R(10, 14, 64, 180), 0);
  addPlasma(*s, R(90, 44, 48, 150), 0);
  addPlasma(*s, R(170, 14, 64, 180), 2);
  addPlasma(*s, R(250, 44, 48, 150), 2);
  s->caption("STD", RF(10.0f, 196.0f, 64.0f, 10.0f), 8.5f, mute());
  s->caption("CMP", RF(90.0f, 196.0f, 48.0f, 10.0f), 8.5f, mute());
  return s;
}

std::unique_ptr<Sheet> zoomCluster() {
  auto s = std::make_unique<Sheet>("", "", 640, 340);
  auto& z = s->val(0, 1, 0, 0.35f, "", 2);
  z.effective(0.5f);
  s->add<Nexus>(R(10, 10, 132, 150), NX::XL, Domain::Wt, "WT Z", "WT Z Master", z);
  auto& c = s->val(0, 1, 0.5f, 0.5f, "", 2);
  s->add<Nexus>(R(150, 20, 84, 96), NX::S, Domain::Cross, "AMOUNT", "Cross Audio Amount", c);
  auto& m = s->val(-1, 1, 0, -0.3f, "", 2, true);
  s->add<Nexus>(R(244, 30, 64, 74), NX::Micro, Domain::Cross, "DEPTH", "CROSS OP1>WT X", m);
  addPlasma(*s, R(318, 10, 64, 180), 1);
  addPlasma(*s, R(390, 40, 48, 150), 0);
  s->add<NeuralCell>(R(452, 12, 180, 42), NK::Toggle, Domain::Neutral, "FREEZE", "DST WT1 D2 Freeze").setToggleState(true, juce::dontSendNotification);
  s->add<NeuralCell>(R(452, 60, 180, 42), NK::Arm, Domain::Neutral, "ARM", "Autonomous Armed").setArmState(NeuralCell::ArmState::Armed);
  s->add<NeuralCell>(R(452, 108, 180, 42), NK::Safety, Domain::Alert, "PANIC", "Panic");
  s->add<SynapticTrack>(R(10, 196, 300, 46), ST::Segments, Domain::Neutral, "FB MODE", "Feedback Mode", juce::StringArray{"LEGACY", "REGEN"});
  s->add<SynapticTrack>(R(10, 256, 300, 46), ST::List, Domain::Cross, "MODE", "Cross Audio Mode", juce::StringArray{"OFF", "XFADE", "RING", "AM"}).setIndex(1);
  s->add<CyberDisplay>(R(452, 160, 180, 174), DV::Meter, Domain::Neutral, demoDisplay(DV::Meter, 0.0f));
  return s;
}

// ================================================================ 7. PANIC définitif
class PanicMeasures : public juce::Component {
 public:
  void paint(juce::Graphics& g) override {
    const char* st[4] = {"NORMAL", "HOVER", "PRESSED", "FOCUS"};
    for (int k = 0; k < 4; ++k) {
      const auto m1 = panicLabelContrast(k, 1.0f, true, true), m2 = panicLabelContrast(k, 2.0f, true, true);
      const double worst = std::min(m1.minRatio, m2.minRatio);
      const float x = 24.0f + static_cast<float>(k) * 150.0f;
      g.setFont(labelFont(10.0f, true));
      g.setColour(t::textDim);
      g.drawText(st[k], juce::Rectangle<float>(x, 0.0f, 140.0f, 14.0f), juce::Justification::centredLeft, false);
      g.setFont(valueFont(12.0f, true));
      g.setColour(worst >= 4.5 ? t::wt : t::alert);
      g.drawText("min " + juce::String(worst, 2) + ":1", juce::Rectangle<float>(x, 16.0f, 140.0f, 16.0f), juce::Justification::centredLeft, false);
    }
  }
};

std::unique_ptr<Sheet> sheetPanic() {
  auto s = std::make_unique<Sheet>(u8("PANIC  —  libellé carbone définitif (LAB-A07.1)"),
                                   u8("Rouge de sécurité inchangé · contraste du libellé mesuré sur la SURFACE RENDUE (dégradé réel), dans chaque état, aux échelles 1 et 2 · "
                                      "seuil 4,5:1"),
                                   1340, 600);
  s->plate(RF(16.0f, 76.0f, 640.0f, 200.0f), u8("R1 : états interactifs"), Domain::Alert);
  const Forced fs[4] = {Forced::Normal, Forced::Hover, Forced::Pressed, Forced::Focus};
  for (int k = 0; k < 4; ++k) s->add<NeuralCell>(R(40 + k * 150, 112, 116, 42), NK::Safety, Domain::Alert, "PANIC", "Panic").setForced(fs[k]);
  s->add<PanicMeasures>(R(16, 172, 640, 40));
  s->caption(u8("Contraste minimal mesuré pixel par pixel dans la zone du libellé (touche rendue sans libellé), pire cas des échelles 1 et 2."),
             RF(40.0f, 222.0f, 600.0f, 40.0f), 10.0f, dim(), juce::Justification::topLeft);
  s->plate(RF(670.0f, 76.0f, 654.0f, 200.0f), u8("Référence B1 (avant)"), Domain::Neutral);
  auto& w = s->add<NeuralCell>(R(700, 112, 116, 42), NK::Safety, Domain::Alert, "PANIC", "Panic");
  w.setRevision(Rev::B1);
  auto& cb = s->add<NeuralCell>(R(850, 112, 116, 42), NK::Safety, Domain::Alert, "PANIC", "Panic");
  cb.setRevision(Rev::B1);
  cb.setSafetyDarkLabel(true);
  s->caption(u8("B1 blanc (0.4.1) : 3,84:1 nominal, FAIL"), RF(700.0f, 160.0f, 150.0f, 30.0f), 9.5f, mute(), juce::Justification::topLeft);
  s->caption(u8("B1 carbone : variante soumise"), RF(850.0f, 160.0f, 150.0f, 30.0f), 9.5f, mute(), juce::Justification::topLeft);
  s->plate(RF(16.0f, 290.0f, 1308.0f, 290.0f), u8("En contexte : barre globale"), Domain::Neutral);
  barStrip(*s, 30, 330, 1280);
  s->caption(u8("PANIC reste immédiatement identifiable (seule touche rouge, isolée à droite), distinct de AUTHORIZE (action, icône ambre) et de "
                "DRONE (armement). Zone interactive et comportement inchangés."),
             RF(40.0f, 420.0f, 1260.0f, 40.0f), 10.5f, dim(), juce::Justification::topLeft);
  return s;
}

// ================================================================ 8. assemblage WT / CROSS / FM
std::unique_ptr<Sheet> sheetAssembly() {
  auto s = std::make_unique<Sheet>(u8("Assemblage WT / CROSS / FM"),
                                   u8("Composants validés (NEXUS, PLASMA) et perfectionnés (NEURAL CELL, SYNAPTIC, HUD) dans des plaques · disposition indicative, "
                                      "ne préjuge pas de R2-C, R2-D, R2-E"),
                                   1400, 500);
  wtPlate(*s, 16, 80);
  crossPlate(*s, 466, 80);
  fmPlate(*s, 936, 80);
  return s;
}

// ================================================================ 10. nomenclature critique
class TypoCasesR1 : public juce::Component {
 public:
  void paint(juce::Graphics& g) override {
    struct Case {
      juce::String label;
      juce::String family;
      float body, budget;  // budget < 0 : indéterminé (gabarit futur)
      bool bold, two;
      juce::String note;
    };
    const Case cs[] = {
        {"WT Z", "NEXUS XL (128)", 12.5f, 128.0f, true, true, u8("critique")},
        {"Z " + kDot + " CROSS LVL", "PLASMA V (70)", 12.0f, 70.0f, false, true, u8("critique · axe Z du Core")},
        {"CROSS LVL", "PLASMA V (70)", 12.0f, 70.0f, false, true, u8("critique")},
        {"X " + kDot + " WT" + kArrow + "FM", "NEXUS S (80)", 12.0f, 80.0f, false, true, u8("critique · axe X")},
        {"Y " + kDot + " FM" + kArrow + "WT", "NEXUS S (80)", 12.0f, 80.0f, false, true, u8("critique · axe Y")},
        {"AMOUNT", "NEXUS S (80)", 11.0f, 80.0f, false, true, u8("LAB-A08 : sous CROSS AUDIO (ex XAUDIO AMT)")},
        {"MODE", "liste (164)", 11.0f, 164.0f, false, false, u8("LAB-A08 : sous CROSS AUDIO (ex XAUDIO MODE)")},
        {"KEEP FUND", "NEURAL (99)", 11.0f, 99.0f, true, true, u8("LAB-A08 (ex KEEP F0)")},
        {"CROSS AUDIO", u8("en-tête de section"), 10.5f, -1.0f, true, false, u8("LAB-A08 : nouvel en-tête ; budget R2-B2 / R2-D")},
        {"BUS / CROSS AUDIO", "onglet", 11.0f, -1.0f, true, false, u8("LAB-A08 (ex BUS / XAUDIO) ; budget R2-D")},
        {"OUT GAIN", "NEXUS S (92)", 11.0f, 92.0f, false, true, u8("LAB-A08 : gain général")},
        {"PEAK " + kMinus + "3.2 dBFS", "barre (120)", 12.0f, 120.0f, true, false, u8("critique · mesure")},
        {"SUSPENDED", "barre (96)", 12.0f, 96.0f, true, false, u8("critique · autonomie")},
        {"SPECTRAL", "segment (48,7)", 11.0f, 48.67f, false, false, u8("marge faible")},
        {"WT>FM", "NEXUS S (80)", 12.0f, 80.0f, false, true, u8("repli ASCII (OD-11)")}};
    float y = 8.0f;
    g.setFont(labelFont(9.5f, true));
    g.setColour(t::textDim);
    const juce::String hs[5] = {u8("LIBELLÉ"), u8("FAMILLE (BUDGET) · CORPS"), u8("RENDU DANS LE BUDGET"), u8("MESURE (cette plateforme)"), u8("75 % écran")};
    const int hx[5] = {8, 200, 400, 640, 900};
    for (int k = 0; k < 5; ++k) g.drawText(hs[k], hx[k], static_cast<int>(y), 240, 12, juce::Justification::centredLeft);
    y += 20.0f;
    for (const auto& c : cs) {
      const auto f = labelFont(c.body, c.bold);
      const float w = draw::textWidth(f, c.label);
      const bool hasBudget = c.budget > 0.0f;
      const bool fits = hasBudget && w <= c.budget;
      bool twoOk = false;
      if (hasBudget && !fits && c.two && c.label.containsChar(' ')) {
        float best = 1.0e9f;
        for (int k = c.label.indexOfChar(' '); k >= 0; k = c.label.indexOfChar(k + 1, ' '))
          best = std::min(best, std::max(draw::textWidth(f, c.label.substring(0, k)), draw::textWidth(f, c.label.substring(k + 1))));
        twoOk = best <= c.budget;
      }
      const juce::String verdict = !hasBudget ? u8("BUDGET INDÉTERMINÉ") : fits ? (c.budget - w < 8.0f ? "OK, MARGIN < 8" : "OK") : (twoOk ? "2 LINES" : "OVERFLOW");
      const juce::Colour vc = !hasBudget ? t::warn : fits ? (c.budget - w < 8.0f ? t::warn : t::wt) : (twoOk ? t::mod : t::alert);
      const float rh = (!hasBudget || fits || !c.two) ? 26.0f : 40.0f;
      g.setFont(labelFont(11.0f, true));
      g.setColour(t::text);
      g.drawText(c.label, 8, static_cast<int>(y), 190, static_cast<int>(rh), juce::Justification::centredLeft);
      g.setFont(labelFont(10.0f));
      g.setColour(t::textDim);
      g.drawText(c.family + u8(" · ") + juce::String(c.body, 1), 200, static_cast<int>(y), 196, static_cast<int>(rh), juce::Justification::centredLeft);
      const auto box = RF(400.0f, y + 2.0f, hasBudget ? c.budget : w + 16.0f, rh - 4.0f);
      g.setColour(t::carbon);
      g.fillRect(box);
      g.setColour(hasBudget ? t::panelEdge : t::warn.withAlpha(0.5f));
      g.drawRect(box, 1.0f);
      {
        juce::Graphics::ScopedSaveState ss(g);
        g.reduceClipRegion(box.expanded(60.0f, 0.0f).toNearestInt());
        draw::fittedText(g, c.label, box, f, t::text, juce::Justification::centred, c.two);
      }
      g.setFont(valueFont(10.0f));
      g.setColour(t::text);
      g.drawText(juce::String(w, 1) + " / " + (hasBudget ? juce::String(c.budget, 1) : juce::String("?")) + " px", 640, static_cast<int>(y), 130, static_cast<int>(rh),
                 juce::Justification::centredLeft);
      g.setColour(vc);
      g.setFont(labelFont(10.0f, true));
      g.drawText(verdict, 770, static_cast<int>(y), 130, static_cast<int>(rh), juce::Justification::centredLeft);
      g.setFont(valueFont(10.0f));
      g.setColour(c.body * 0.75f >= 9.0f ? t::text : t::textMute);
      g.drawText(juce::String(c.body * 0.75f, 2) + " px", 900, static_cast<int>(y), 80, static_cast<int>(rh), juce::Justification::centredLeft);
      g.setFont(labelFont(9.5f));
      g.setColour(t::textMute);
      g.drawText(c.note, 980, static_cast<int>(y), 330, static_cast<int>(rh), juce::Justification::centredLeft);
      y += rh + 6.0f;
    }
    y += 8.0f;
    g.setFont(labelFont(12.0f, true));
    g.setColour(t::text);
    g.drawText(u8("GLYPHES (PT-05) · rendu réel dans les polices de l'interface"), 8, static_cast<int>(y), 700, 16, juce::Justification::centredLeft);
    y += 24.0f;
    const juce::String glyphs[4] = {kArrow, kDot, kEll, kMinus};
    const juce::String gn[4] = {u8("U+2192 →"), u8("U+00B7 ·"), u8("U+2026 …"), u8("U+2212 −")};
    const char* ascii[4] = {">", ".", "...", "-"};
    for (int k = 0; k < 4; ++k) {
      float x = 8.0f;
      g.setFont(labelFont(10.0f, true));
      g.setColour(t::textDim);
      g.drawText(gn[k], static_cast<int>(x), static_cast<int>(y), 90, 22, juce::Justification::centredLeft);
      x += 96.0f;
      for (float sz : {9.0f, 10.0f, 11.0f, 12.0f, 14.0f})
        for (int m = 0; m < 2; ++m) {
          const auto fo = m == 0 ? labelFont(sz) : valueFont(sz);
          const auto tf = fo.getTypefacePtr();
          const bool native = tf != nullptr && tf->getNominalGlyphForCodepoint(glyphs[k][0]).has_value();
          g.setFont(fo);
          g.setColour(native ? t::text : t::alert);
          g.drawText("A" + glyphs[k] + "B", static_cast<int>(x), static_cast<int>(y), 44, 22, juce::Justification::centredLeft);
          x += 46.0f;
        }
      g.setFont(labelFont(10.0f));
      g.setColour(t::textMute);
      g.drawText(u8("repli ASCII : « ") + ascii[k] + u8(" »"), static_cast<int>(x) + 10, static_cast<int>(y), 160, 22, juce::Justification::centredLeft);
      y += 26.0f;
    }
  }
};

std::unique_ptr<Sheet> sheetTypo() {
  auto s = std::make_unique<Sheet>(u8("Nomenclature critique (LAB-A08)"),
                                   u8("Mesures JUCE réelles sur la plateforme de rendu · nouveaux libellés AMOUNT, MODE, KEEP FUND, CROSS AUDIO · budgets Codex ; "
                                      "budget indéterminé = gabarit futur, jamais déclaré PASS"),
                                   1340, 860);
  s->add<TypoCasesR1>(R(16, 76, 1308, 770));
  return s;
}

// ================================================================ 11. matériaux et palette
class Swatches : public juce::Component {
 public:
  void paint(juce::Graphics& g) override {
    struct S {
      const char* name;
      juce::Colour c;
    };
    const S pal[8] = {{"Graphite", t::chassis}, {"Carbon", t::carbon}, {"Titanium", t::titanium}, {"Optical Text", t::text},
                      {"WT", t::wt},             {"FM", t::fm},         {"CROSS", t::cross},       {"MOD", t::mod}};
    const S der[8] = {{"relief1", tok2::relief1}, {"relief2", tok2::relief2}, {"relief3", tok2::relief3}, {"groove", tok2::groove},
                      {"hudGlassCore", tok2::hudGlassCore}, {"hudGlassDeep", tok2::hudGlassDeep}, {"warn", t::warn}, {"alert", t::alert}};
    for (int row = 0; row < 2; ++row)
      for (int k = 0; k < 8; ++k) {
        const auto& s = row == 0 ? pal[k] : der[k];
        const auto r = juce::Rectangle<float>(10.0f + static_cast<float>(k) * 160.0f, 10.0f + static_cast<float>(row) * 74.0f, 150.0f, 64.0f);
        g.setColour(s.c);
        g.fillRoundedRectangle(r.withHeight(40.0f), 3.0f);
        g.setColour(t::panelEdge);
        g.drawRoundedRectangle(r.withHeight(40.0f).reduced(0.5f), 3.0f, 1.0f);
        g.setFont(labelFont(10.0f, true));
        g.setColour(t::text);
        g.drawText(s.name, r.withTrimmedTop(42.0f).withHeight(11.0f), juce::Justification::centredLeft, false);
        g.setFont(valueFont(9.0f));
        g.setColour(t::textDim);
        g.drawText("#" + s.c.toDisplayString(false), r.withTrimmedTop(53.0f), juce::Justification::centredLeft, false);
      }
  }
};

std::unique_ptr<Sheet> sheetMaterials() {
  auto s = std::make_unique<Sheet>(u8("Cohérence des matériaux et de la palette (LAB-A06)"),
                                   u8("Châssis graphite mat · plaques en titane sombre · commandes usinées · afficheurs en verre optique cybernétique · couleurs "
                                      "fonctionnelles des domaines · matière biomécanique réservée au SYMBIOTIC CORE (absente)"),
                                   1340, 760);
  s->plate(RF(16.0f, 76.0f, 1308.0f, 200.0f), u8("PALETTE DE RÉFÉRENCE (inchangée) ET DÉRIVÉS"), Domain::Neutral);
  s->add<Swatches>(R(30, 104, 1290, 166));
  s->plate(RF(16.0f, 290.0f, 1308.0f, 446.0f), u8("MATÉRIAUX"), Domain::Neutral);
  const juce::String caps[6] = {u8("COMMANDE CONTINUE\nNEXUS (validé)"), u8("TOUCHE\ntitane usiné"), u8("RAIL\nusiné"), u8("CANAL\nplasma (validé)"),
                                u8("AFFICHEUR\nverre HUD"), u8("SÉCURITÉ\nPANIC carbone")};
  for (int k = 0; k < 6; ++k) s->caption(caps[k], RF(40.0f + static_cast<float>(k) * 214.0f, 322.0f, 200.0f, 30.0f), 10.0f, dim(), juce::Justification::topLeft, true);
  auto& v = s->val(0, 1, 0, 0.62f, "", 2);
  v.effective(0.42f);
  s->add<Nexus>(R(48, 366, 132, 150), NX::XL, Domain::Wt, "X", "WT X", v);
  s->add<NeuralCell>(R(254, 380, 180, 42), NK::Action, Domain::Neutral, "APPLY WAVE", "Apply drawn wave");
  s->add<NeuralCell>(R(254, 436, 180, 42), NK::Toggle, Domain::Wt, "SYNC", "WT Sync").setToggleState(true, juce::dontSendNotification);
  s->add<NeuralCell>(R(254, 492, 180, 42), NK::Arm, Domain::Neutral, "ARM", "Armed").setArmState(NeuralCell::ArmState::Armed);
  s->add<SynapticTrack>(R(468, 380, 190, 46), ST::Segments, Domain::Cross, "DIR", "CROSS Direction", juce::StringArray{"FWD", "REV", "BOTH"}).setIndex(1);
  s->add<SynapticTrack>(R(468, 446, 190, 46), ST::List, Domain::Fm, "TOPOLOGY", "FM topology", juce::StringArray{"STACK 4", "2 PAIRS"});
  addPlasma(*s, R(690, 366, 64, 180), 1);
  addPlasma(*s, R(780, 396, 48, 150), 2);
  s->add<CyberDisplay>(R(896, 366, 200, 150), DV::Numeric, Domain::Neutral, demoDisplay(DV::Numeric, 0.0f));
  s->add<NeuralCell>(R(1130, 380, 116, 42), NK::Safety, Domain::Alert, "PANIC", "Panic");
  s->caption(u8("Aucune vis décorative, aucune texture envahissante, aucun effet vintage : usinage suggéré par chanfreins, arêtes et stries de très faible "
                "opacité (2,2 %). Lueurs plafonnées à 0,35. La couleur d'un domaine signale une fonction, jamais un décor."),
             RF(40.0f, 580.0f, 1260.0f, 40.0f), 10.5f, dim(), juce::Justification::topLeft);
  return s;
}

}  // namespace

bool renderSheets(const juce::File& dir, float scale, juce::StringArray& log, const juce::File& b1Dir) {
  bool ok = true;
  const auto save = [&](const juce::Image& img, const juce::String& name) {
    const bool w = writePng(img, dir.getChildFile(name));
    log.add(name + "  " + juce::String(img.getWidth()) + "x" + juce::String(img.getHeight()) + (w ? "" : "  WRITE FAILED"));
    ok = ok && w;
  };
  const auto snap = [&](juce::Component& c, const juce::String& name, float sc) { save(c.createComponentSnapshot(c.getLocalBounds(), true, sc), name); };
  { auto b = buildBoard(); snap(*b, "01-final-board.png", scale); }
  // non-régression NEXUS et PLASMA : planche B1 re-rendue par le code R1, comparée pixel à pixel à la capture B1 publiée
  const struct {
    const char *b1Name, *outName, *title;
    std::function<std::unique_ptr<Sheet>()> make;
  } nonreg[2] = {{"02-nexus.png", "02-nexus-nonregression.png", "NEXUS Hybrid  -  non-regression (LAB-A01)", [] { return sheetNexusB1(); }},
                 {"06-plasma-channel.png", "06c-plasma-nonregression.png", "PLASMA CHANNEL  -  non-regression (LAB-A05)", [] { return sheetPlasmaB1(); }}};
  for (const auto& n : nonreg) {
    auto sh = n.make();
    const auto now = sh->createComponentSnapshot(sh->getLocalBounds(), true, scale);
    const auto before = b1Dir.getChildFile(n.b1Name).existsAsFile() ? juce::ImageFileFormat::loadFrom(b1Dir.getChildFile(n.b1Name)) : juce::Image();
    const auto d = diffImages(before, now, juce::roundToInt(66.0f * scale));
    log.add(juce::String("NONREG ") + n.b1Name + " : " + diffText(d));
    save(composeBeforeAfter(before, now, n.title, d), n.outName);
    ok = ok && d.loaded && d.sameSize && d.differing == 0;
  }
  { auto s = sheetNeuralBA(); snap(*s, "03-neural-cell-before-after.png", scale); }
  { auto s = sheetTrackBA(); snap(*s, "04-synaptic-track-before-after.png", scale); }
  { auto s = sheetHud(); snap(*s, "05a-cybernetic-hud.png", scale); }
  { auto s = sheetHudBA(); snap(*s, "05b-cybernetic-hud-before-after.png", scale); }
  { auto s = sheetPlasmaSC(); snap(*s, "06a-plasma-standard-compact.png", scale); }
  save(composeZoom(u8("PLASMA STANDARD / COMPACT  —  75 / 100 / 150 %"),
                   u8("Pixels écran réels (densité x1, cas le plus défavorable) · STD = 64 x 180, CMP = 48 x 150 · même composant"),
                   [] { return plasmaZoomCluster(); }),
       "06b-plasma-zoom-75-100-150.png");
  { auto s = sheetPanic(); snap(*s, "07-panic-final.png", scale); }
  { auto s = sheetAssembly(); snap(*s, "08-assembly-wt-cross-fm.png", scale); }
  save(composeZoom(u8("Comparatif 75 / 100 / 150 %  (R1)"),
                   u8("Pixels écran réels (densité x1) · repères critiques au corps logique 12 (XL 12,5) : 9 px écran à 75 % · PLASMA STANDARD et COMPACT"),
                   [] { return zoomCluster(); }),
       "09-zoom-75-100-150.png");
  { auto s = sheetTypo(); snap(*s, "10-critical-nomenclature.png", scale); }
  { auto s = sheetMaterials(); snap(*s, "11-materials-palette.png", scale); }
  return ok;
}

}  // namespace sbd::lab
