// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab — planches (implémentation).
// Légendes de planche en français (documentation) ; sérigraphie des composants en anglais technique compact (DA-A01).
#include "app/Sheets.h"

#include <cmath>

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

// ================================================================ Sheet
Sheet::Sheet(const juce::String& title, const juce::String& subtitle, int w, int h) : title_(title), subtitle_(subtitle) { setSize(w, h); }

DemoValue& Sheet::val(float lo, float hi, float def, float value, const juce::String& unit, int dec, bool bip, int steps) {
  values_.push_back(std::make_unique<DemoValue>(lo, hi, def, value, unit, dec, bip, steps));
  return *values_.back();
}

void Sheet::caption(const juce::String& s, RF r, float size, juce::Colour c, juce::Justification j, bool bold) {
  caps_.push_back({s, r, size, c, j, bold});
}

void Sheet::plate(RF r, const juce::String& title, Domain d, const juce::String& note) { plates_.push_back({r, title, note, d}); }

void Sheet::paint(juce::Graphics& g) {
  g.fillAll(t::chassis);
  if (title_.isNotEmpty()) paintHeader(g);
  paintBody(g);
}

void Sheet::paintHeader(juce::Graphics& g) {
  auto head = getLocalBounds().toFloat().removeFromTop(64.0f).reduced(24.0f, 10.0f);
  g.setFont(labelFont(20.0f, true));
  g.setColour(t::text);
  g.drawText(title_, head.withHeight(24.0f), juce::Justification::centredLeft, false);
  g.setFont(labelFont(11.0f));
  g.setColour(t::textDim);
  g.drawText(subtitle_, head.withTrimmedTop(28.0f).withHeight(16.0f), juce::Justification::centredLeft, true);
  g.setFont(labelFont(9.0f));
  g.setColour(t::textMute);
  g.drawText(platformLine(), head.withHeight(12.0f), juce::Justification::centredRight, false);
  g.setColour(t::warn.withAlpha(0.85f));
  g.drawText(bannerLine(), head.withTrimmedTop(14.0f).withHeight(12.0f),
             juce::Justification::centredRight, false);
  g.setColour(t::panelEdge);
  g.drawHorizontalLine(64, 16.0f, static_cast<float>(getWidth()) - 16.0f);
}

void Sheet::paintBody(juce::Graphics& g) {
  for (const auto& p : plates_) {
    g.setColour(tok2::relief1);
    g.fillRoundedRectangle(p.r, t::radius);
    g.setColour(t::panelEdge);
    g.drawRoundedRectangle(p.r.reduced(0.5f), t::radius, 1.0f);
    auto h = p.r.reduced(10.0f, 0.0f).withHeight(24.0f);
    g.setColour(domainColour(p.d));
    g.fillRect(h.getX(), h.getY() + 7.0f, 3.0f, 11.0f);
    g.setFont(labelFont(tok2::fontTitle, true));
    g.setColour(t::text);
    g.drawText(p.title, h.withTrimmedLeft(9.0f), juce::Justification::centredLeft, false);
    if (p.note.isNotEmpty()) {
      g.setFont(labelFont(9.5f));
      g.setColour(t::textDim);
      g.drawText(p.note, h, juce::Justification::centredRight, false);
    }
  }
  g.setColour(t::textMute.withAlpha(0.8f));
  for (const auto& l : lines_) g.drawLine(l.first.x, l.first.y, l.second.x, l.second.y, 0.8f);
  for (const auto& c : caps_) {
    g.setFont(labelFont(c.size, c.bold));
    g.setColour(c.c);
    g.drawFittedText(c.s, c.r.toNearestInt(), c.j, 6, 1.0f);
  }
}

bool writePng(const juce::Image& img, const juce::File& f) {
  f.getParentDirectory().createDirectory();
  f.deleteFile();
  juce::FileOutputStream os(f);
  if (!os.openedOk()) return false;
  juce::PNGImageFormat png;
  return png.writeImageToStream(img, os);
}

namespace {

#define kDim t::textDim
#define kMute t::textMute
#define kText t::text
juce::String u8(const char* s) { return juce::String::fromUTF8(s); }
const juce::String kArrow = u8("\xe2\x86\x92"), kDot = u8("\xc2\xb7"), kMinus = u8("\xe2\x88\x92"), kEll = u8("\xe2\x80\xa6");

// ================================================================ 2. NEXUS
std::unique_ptr<Sheet> sheetNexus() {
  auto s = std::make_unique<Sheet>(u8("NEXUS Hybrid  —  XL · S · Micro"),
                                   u8("DA-A03 · titane noir, périphérie usinée (moletage XL), index précis, arc BASE, marqueur EFFECTIVE (losange), "
                                      "affichage numérique · unipolaire et bipolaire · WT / FM / CROSS / MOD"),
                                   1340, 830);
  struct Sp {
    const char *lbl, *full;
    float lo, hi, def, v;
    const char* unit;
    int dec;
    bool bip;
    float eff;  // < 0 : non modulé
    std::vector<float> routes;
  };
  const Domain doms[4] = {Domain::Wt, Domain::Fm, Domain::Cross, Domain::Mod};
  const Sp specs[4][6] = {
      {{"X", "WT X", 0, 1, 0, 0.486f, "", 2, false, 0.62f, {0.2f, -0.1f}},
       {"FINE", "WT Fine", -100, 100, 0, -32, "ct", 0, true, -1, {}},
       {"WT Z", "WT Z Master (Z transform depth)", 0, 1, 0, 0.35f, "", 2, false, 0.5f, {0.15f}},
       {"A DEPTH", "WT ZA Depth", -1, 1, 0, 0.4f, "", 2, true, -1, {}},
       {"LVL", "WT Level", 0, 1, 1, 0.8f, "", 2, false, -1, {}},
       {"SHIFT", "WT Z Shift (demo)", -1, 1, 0, -0.3f, "", 2, true, -1, {}}},
      {{"FM INDEX", "FM OP2>OP1 (contextual)", 0, 8, 1, 2.4f, "rad", 2, false, 0.42f, {0.1f, 0.25f, -0.2f}},
       {"FINE", "OP1 Fine", -100, 100, 0, 12, "ct", 0, true, -1, {}},
       {"RATIO", "OP2 Ratio", 0.5f, 16, 1, 2, "x", 2, false, -1, {}},
       {"OFFSET", "OP3 Offset", -50, 50, 0, -7.5f, "Hz", 1, true, 0.35f, {}},
       {"OP2" "\xe2\x86\x92" "OP1", "FM OP2>OP1", 0, 8, 0, 3.2f, "rad", 1, false, -1, {}},
       {"PAN", "OP4 Pan", -1, 1, 0, 0.25f, "", 2, true, -1, {}}},
      {{"WT" "\xe2\x86\x92" "FM", "CROSS WT>FM (forward)", 0, 1, 1, 0.72f, "", 2, false, 0.55f, {-0.15f}},
       {"BALANCE", "WT/FM Balance (audio, pre-chain)", -1, 1, 0, -0.2f, "", 2, true, 0.1f, {}},
       {"CROSS LVL", "CROSS Master (CROSS level)", 0, 1, 1, 0.64f, "", 2, false, -1, {0.2f}},
       {"Z", "CROSS OP1>WT Z", -1, 1, 0, 0.55f, "", 2, true, -1, {}},
       {"DEPTH", "CROSS OP1>WT X", -1, 1, 0, 0.1f, "", 2, false, -1, {}},
       {"DEPTH", "CROSS BUS>WT Y", -1, 1, 0, -0.6f, "", 2, true, -1, {}}},
      {{"M1", "Macro 1", 0, 1, 0, 0.58f, "", 2, false, -1, {}},
       {"DEPTH", "Mod link depth", -1, 1, 0, -0.45f, "", 2, true, -1, {}},
       {"RATE", "LFO1 Rate", 0.01f, 40, 1, 4.2f, "Hz", 2, false, 0.3f, {0.3f}},
       {"OFFSET", "Mod link offset", -1, 1, 0, 0.3f, "", 2, true, -1, {}},
       {"ATK", "Amp Attack", 0, 10000, 5, 120, "ms", 0, false, -1, {}},
       {"AMT", "Via amount", -1, 1, 0, -0.8f, "", 2, true, -1, {}}}};
  const NX sizes[6] = {NX::XL, NX::XL, NX::S, NX::S, NX::Micro, NX::Micro};
  const int colX[6] = {120, 266, 420, 516, 618, 694};
  const char* heads[6] = {"XL · unipolar", "XL · bipolar", "S · unipolar", "S · bipolar", "Micro · uni", "Micro · bi"};  // utf8-ok
  for (int c = 0; c < 6; ++c) s->caption(u8(heads[c]), RF(static_cast<float>(colX[c]) - 6.0f, 72.0f, 150.0f, 14.0f), 10.0f, kDim);
  for (int r = 0; r < 4; ++r) {
    const int y = 92 + r * 178;
    s->caption(domainName(doms[r]), RF(24.0f, static_cast<float>(y) + 50.0f, 80.0f, 18.0f), 14.0f, domainColour(doms[r]), juce::Justification::centredLeft, true);
    for (int c = 0; c < 6; ++c) {
      const auto& sp = specs[r][c];
      auto& v = s->val(sp.lo, sp.hi, sp.def, sp.v, sp.unit, sp.dec, sp.bip);
      if (sp.eff >= 0.0f) v.effective(sp.eff);
      v.routes(sp.routes);
      const auto cs = Nexus::cellSize(sizes[c]);
      const int yy = y + (150 - cs.y) / 2;
      s->add<Nexus>(R(colX[c], yy, cs.x, cs.y), sizes[c], doms[r], u8(sp.lbl), u8(sp.full), v).setForced(Forced::Normal);
    }
  }
  // anatomie
  s->plate(RF(800.0f, 84.0f, 520.0f, 360.0f), u8("ANATOMIE · NEXUS XL"), Domain::Neutral, "DEMO DATA");
  auto& v = s->val(0, 1, 0, 0.42f, "", 2);
  v.effective(0.66f).routes({0.18f, -0.12f, 0.3f, 0.05f, -0.2f});
  s->add<Nexus>(R(900, 150, 132, 150), NX::XL, Domain::Wt, "WT Z", "WT Z Master (Z transform depth)", v).setForced(Forced::Normal);
  const float cx = 966.0f, cy = 150.0f + 19.0f + 54.0f;
  struct Call {
    float px, py;
    juce::String txt;
  };
  const Call calls[] = {{cx, 160.0f, u8("Sérigraphie : libellé court anglais (nom complet au focus et dans l'Inspector)")},
                        {cx - 30.0f, cy - 40.0f, u8("Arc BASE : couleur du moteur, depuis le minimum (ou le centre en bipolaire)")},
                        {cx + 44.0f, cy - 30.0f, u8("EFFECTIVE : anneau extérieur et losange, indépendants de la BASE")},
                        {cx + 50.0f, cy + 10.0f, u8("Marques de route : 4 au plus, puis +n (détail dans l'Inspector)")},
                        {cx + 30.0f, cy + 28.0f, u8("Moletage usiné (XL seulement) ; biseau éclairé en haut à gauche")},
                        {cx, cy, u8("Index précis (trait optique) sur face en titane noir")},
                        {cx, 290.0f, u8("Valeur BASE en chiffres tabulaires, fenêtre optique")}};
  float ty = 112.0f;
  for (const auto& c : calls) {
    s->caption(c.txt, RF(1060.0f, ty, 250.0f, 26.0f), 9.5f, kDim);
    s->line({c.px, c.py}, {1056.0f, ty + 8.0f});
    ty += 46.0f;
  }
  s->plate(RF(800.0f, 456.0f, 520.0f, 352.0f), u8("RÈGLES"), Domain::Neutral);
  s->caption(u8("XL : richesse visuelle maximale (moletage, graduations x11, lueur maîtrisée ≤ 0,35).\n"
                "S : traitement mécanique simplifié (biseau usiné sans moletage, graduations x5).\n"
                "Micro : priorité absolue à la lisibilité (index, arc, valeur ; pas de marque de route).\n\n"
                "Gestes 0.4.1 conservés : glisser 220 px = plage complète, Maj x0,1, molette groupée 450 ms, flèches 1 %, "
                "Page 10 %, Début/Fin, Suppr et Alt+clic = défaut, double-clic valeur = saisie.\n\n"
                "Discipline de geste R2 : un begin/end par geste ; double-clic hors valeur = défaut DANS le geste ouvert "
                "(aucun geste imbriqué, cf. PB-01) ; geste refermé à la destruction et au masquage (cf. PB-02). "
                "Vérifié par l'auto-test du laboratoire (INTERACTION-TEST-PLAN.md).\n\n"
                "Valeurs, libellés et plages de cette planche : DEMO DATA."),
             RF(816.0f, 486.0f, 490.0f, 310.0f), 10.5f, kDim, juce::Justification::topLeft);
  return s;
}

// ================================================================ 6. PLASMA CHANNEL
std::unique_ptr<Sheet> sheetPlasma() {
  auto s = std::make_unique<Sheet>("PLASMA CHANNEL Hybrid", u8("DA-A06 · curseur mécanique, canal lumineux BASE, marqueur EFFECTIVE indépendant (triangle), "
                                                                "graduations et unité · vertical / horizontal · unipolaire / bipolaire"),
                                   1340, 640);
  s->plate(RF(16.0f, 76.0f, 520.0f, 290.0f), "VERTICAL", Domain::Neutral);
  const juce::StringArray dbG{u8("\xe2\x88\x92") + "48", u8("\xe2\x88\x92") + "24", "0", "+6"};
  auto& g1 = s->val(-48, 6, -6, -6, "dB", 1);
  s->add<PlasmaChannel>(R(40, 112, 64, 180), PO::Vertical, Domain::Neutral, "OUT GAIN", "Output Gain", g1, dbG);
  auto& g2 = s->val(0, 1, 1, 0.64f, "", 2);
  g2.effective(0.78f);
  s->add<PlasmaChannel>(R(130, 112, 64, 180), PO::Vertical, Domain::Cross, "CROSS LVL", "CROSS Master (CROSS level)", g2, juce::StringArray{"0", ".5", "1"});
  auto& g3 = s->val(-1, 1, 0, -0.35f, "", 2, true);
  g3.effective(0.6f);
  s->add<PlasmaChannel>(R(220, 112, 64, 180), PO::Vertical, Domain::Cross, "BALANCE", "WT/FM Balance (audio, pre-chain)", g3, juce::StringArray{"WT", "0", "FM"});
  auto& g4 = s->val(0, 1, 0, 0.0f, "", 2);
  s->add<PlasmaChannel>(R(310, 112, 64, 180), PO::Vertical, Domain::Mod, "M2", "Macro 2", g4, juce::StringArray{"0", "1"});
  auto& g5 = s->val(0, 1, 0, 1.0f, "", 2);
  s->add<PlasmaChannel>(R(400, 112, 64, 180), PO::Vertical, Domain::Wt, "LVL", "WT Level", g5, juce::StringArray{"0", "1"}).setForced(Forced::Focus);
  s->caption(u8("unipolaire dB · modulé · bipolaire modulé · minimum · maximum + focus"), RF(36.0f, 300.0f, 490.0f, 14.0f), 9.5f, kMute);
  s->caption(u8("Gabarit 0.4.1 : 48 x 150 ; R2 propose 64 x 180 pour loger graduations et unité lisibles (écart à valider)."), RF(36.0f, 320.0f, 490.0f, 30.0f), 9.5f, kDim,
             juce::Justification::topLeft);

  s->plate(RF(550.0f, 76.0f, 774.0f, 290.0f), "HORIZONTAL", Domain::Neutral);
  auto& h1 = s->val(0, 1, 1, 0.64f, "", 2);
  h1.effective(0.48f);
  s->add<PlasmaChannel>(R(570, 112, 360, 44), PO::Horizontal, Domain::Cross, "CROSS LVL", "CROSS Master", h1, juce::StringArray{"0", ".25", ".5", ".75", "1"});
  auto& h2 = s->val(-1, 1, 0, 0.3f, "", 2, true);
  h2.effective(-0.1f);
  s->add<PlasmaChannel>(R(570, 172, 360, 44), PO::Horizontal, Domain::Cross, "BALANCE", "WT/FM Balance (audio, pre-chain)", h2, juce::StringArray{"WT", "0", "FM"});
  auto& h3 = s->val(0, 1, 0.5f, 0.5f, "", 2);
  s->add<PlasmaChannel>(R(570, 232, 360, 44), PO::Horizontal, Domain::Cross, "XAUDIO AMT", "Cross Audio Amount (post-chain)", h3, juce::StringArray{"0", ".5", "1"});
  auto& h4 = s->val(-1, 1, 0, -0.8f, "", 2, true);
  s->add<PlasmaChannel>(R(950, 112, 360, 44), PO::Horizontal, Domain::Mod, "DEPTH", "Mod link depth", h4, juce::StringArray{"-1", "0", "+1"});
  auto& h5 = s->val(0, 1, 0, 0.5f, "", 2);
  s->add<PlasmaChannel>(R(950, 172, 360, 44), PO::Horizontal, Domain::Fm, "FM CHAR", "FM Character Master", h5, juce::StringArray{"0", "1"}).setForced(Forced::Pressed);
  auto& h6 = s->val(0, 1, 0, 0.5f, "", 2);
  s->add<PlasmaChannel>(R(950, 232, 360, 44), PO::Horizontal, Domain::Wt, "LVL", "WT Level", h6, juce::StringArray{"0", "1"}).setForced(Forced::Disabled);
  s->caption(u8("pressé (curseur en creux, valeur soulignée) · désactivé (contour, sans canal)"), RF(950.0f, 284.0f, 360.0f, 14.0f), 9.5f, kMute);

  s->plate(RF(16.0f, 380.0f, 1308.0f, 236.0f), u8("PARAMÈTRE ≠ MESURE"), Domain::Alert);
  auto& og = s->val(-48, 6, -6, -6, "dB", 1);
  s->add<PlasmaChannel>(R(40, 414, 64, 180), PO::Vertical, Domain::Neutral, "OUT GAIN", "Output Gain", og, dbG);
  auto md = demoDisplay(DV::Meter, 0.0f);
  s->add<CyberDisplay>(R(120, 414, 190, 190), DV::Meter, Domain::Neutral, md).setRevision(Rev::B1);  // référence B1 fidèle
  s->caption(u8("OUT GAIN est un PARAMÈTRE (réglage, automatisable, plasma). PEAK est une MESURE (crête de bloc en sortie finale, "
                "afficheur dédié, unité dBFS). La mesure n'est jamais dessinée dans le canal du paramètre. Rouge seulement au-delà de 0 dBFS."),
             RF(330.0f, 430.0f, 960.0f, 60.0f), 11.0f, kText, juce::Justification::topLeft);
  return s;
}


}  // namespace

// Planches B1 conservées à l'identique (référence de non-régression NEXUS / PLASMA, LAB-A01 et LAB-A05)
std::unique_ptr<Sheet> sheetNexusB1() { return sheetNexus(); }
std::unique_ptr<Sheet> sheetPlasmaB1() { return sheetPlasma(); }

}  // namespace sbd::lab
