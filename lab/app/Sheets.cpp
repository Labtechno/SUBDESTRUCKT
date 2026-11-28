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
  g.drawText(juce::String::fromUTF8("DEMO DATA  \xc2\xb7  ARCH-UI-04-R2-B1 Component Lab  \xc2\xb7  isolated lab, not the production plugin"), head.withTrimmedTop(14.0f).withHeight(12.0f),
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

// ================================================================ 3. NEURAL CELL
std::unique_ptr<Sheet> sheetNeural() {
  auto s = std::make_unique<Sheet>("NEURAL CELL Hybrid", u8("DA-A04 · touches mécaniques en titane noir, témoins intégrés · six traitements distincts · "
                                                             "l'état n'est jamais porté par la seule couleur · PANIC inchangé"),
                                   1340, 750);
  s->plate(RF(16.0f, 76.0f, 1308.0f, 150.0f), u8("1 · SIX TRAITEMENTS"), Domain::Neutral);
  const char* heads[6] = {"ACTION INSTANTANÉE", "INTERRUPTEUR", "ARMEMENT", "SÉLECTION EXCLUSIVE", "SÛRETÉ (PANIC)", "DÉSACTIVÉE"};  // utf8-ok
  const int xs[6] = {36, 236, 436, 636, 980, 1120};
  for (int k = 0; k < 6; ++k) s->caption(u8(heads[k]), RF(static_cast<float>(xs[k]), 108.0f, 300.0f, 14.0f), 10.0f, kDim, juce::Justification::centredLeft, true);
  s->add<NeuralCell>(R(36, 128, 168, 40), NK::Action, Domain::Cross, "CROSS RESET", "Cross Reset (clear CROSS history)");
  s->add<NeuralCell>(R(236, 128, 168, 40), NK::Toggle, Domain::Neutral, "LIMITER", "Musical Limiter").setToggleState(true, juce::dontSendNotification);
  s->add<NeuralCell>(R(436, 128, 168, 40), NK::Arm, Domain::Neutral, "ARM", "DST WT1 Autonomous Armed").setArmState(NeuralCell::ArmState::Armed);
  const char* sel[3] = {"LIN", "EXP", "LOG"};
  for (int k = 0; k < 3; ++k) {
    auto& c = s->add<NeuralCell>(R(636 + k * 108, 128, 104, 40), NK::Select, Domain::Mod, sel[k], juce::String("Curve ") + sel[k]);
    c.setRadioGroupId(7);
    c.setToggleState(k == 1, juce::dontSendNotification);
  }
  s->add<NeuralCell>(R(980, 128, 112, 40), NK::Safety, Domain::Alert, "PANIC", "Panic (All Sound Off, revoke)");
  auto& dis = s->add<NeuralCell>(R(1120, 128, 168, 40), NK::Action, Domain::Neutral, "CAPTURE", "Capture frozen D2/S2 content");
  dis.setIcon(NeuralCell::Icon::Capture);
  dis.setEnabled(false);
  dis.setDisabledReason("nothing frozen");
  s->caption(u8("Convexe, icône d'impulsion,\nretour en creux à l'appui"), RF(36.0f, 174.0f, 190.0f, 40.0f), 9.5f, kMute, juce::Justification::topLeft);
  s->caption(u8("Plate, témoin encastré,\ntexte OFF / ON"), RF(236.0f, 174.0f, 190.0f, 40.0f), 9.5f, kMute, juce::Justification::topLeft);
  s->caption(u8("Liseré d'attente ambre, témoin\nanneau (ARMED) ou plein (ACTIVE)"), RF(436.0f, 174.0f, 190.0f, 40.0f), 9.5f, kMute, juce::Justification::topLeft);
  s->caption(u8("Fond carbone, losange plein et soulignement\ndu domaine sur l'option active"), RF(572.0f, 174.0f, 330.0f, 40.0f), 9.5f, kMute, juce::Justification::topLeft);
  s->caption(u8("Rouge réservé, isolée,\ndouble biseau"), RF(980.0f, 174.0f, 140.0f, 40.0f), 9.5f, kMute, juce::Justification::topLeft);
  s->caption(u8("Contour seul, hachures,\n« — » et raison en infobulle"), RF(1120.0f, 174.0f, 190.0f, 40.0f), 9.5f, kMute, juce::Justification::topLeft);

  s->plate(RF(16.0f, 240.0f, 1308.0f, 236.0f), u8("2 · FREEZE · ARM · AUTHORIZE · CAPTURE : QUATRE OPÉRATIONS DISTINCTES"), Domain::Neutral);
  struct Op {
    NK kind;
    const char *lbl, *full, *type;
    juce::String desc;
  };
  const Op ops[4] = {
      {NK::Toggle, "FREEZE", "DST WT1 D2 Freeze", "INTERRUPTEUR", u8("Paramètre du registre (*.d2.freeze, *.s2.freeze)\nPersisté (BASE) · automatisable\nPérimètre : un étage D2 ou S2\nEmplacement : plaque de l'étage")},
      {NK::Arm, "ARM", "DST WT1 Autonomous Armed", "ARMEMENT", u8("Paramètre du registre (*.arm)\nPersisté (configuration) · non automatisable\nPérimètre : un module autonome\nEmplacement : plaque du module")},
      {NK::Action, "AUTHORIZE", "Authorize autonomous states (this instance)", "ACTION", u8("Hors registre · non persistée\nAutorisation de CETTE instance\nVisible si un module est armé et suspendu\nEmplacement : barre globale")},
      {NK::Action, "CAPTURE", "Capture frozen D2/S2 content", "ACTION", u8("Hors registre · mémorise le gel D2/S2\nEnregistré avec le preset et le projet\nPérimètre : WT1 … GL2\nEmplacement : PROCESS et STATE")}};
  for (int k = 0; k < 4; ++k) {
    const int x = 36 + k * 322;
    s->caption(u8(ops[k].type), RF(static_cast<float>(x), 272.0f, 300.0f, 14.0f), 10.0f, kDim, juce::Justification::centredLeft, true);
    auto& c = s->add<NeuralCell>(R(x, 292, 200, 40), ops[k].kind, Domain::Neutral, ops[k].lbl, ops[k].full);
    if (k == 0) c.setToggleState(true, juce::dontSendNotification);
    if (k == 1) c.setArmState(NeuralCell::ArmState::Armed);
    if (k == 2) c.setIcon(NeuralCell::Icon::Authorize);
    if (k == 3) c.setIcon(NeuralCell::Icon::Capture);
    s->caption(ops[k].desc, RF(static_cast<float>(x), 344.0f, 300.0f, 70.0f), 10.0f, kDim, juce::Justification::topLeft);
  }
  s->caption(u8("Différences portées par le TYPE (forme : plate / liseré / convexe), le TÉMOIN (disque, anneau, aucun), l'ICÔNE "
                "(impulsion, autorisation, instantané), le TEXTE d'état et l'EMPLACEMENT. Contrat d'autonomie 0.4.1 inchangé : "
                "le rendu hors ligne affiche RENDER, jamais AUTHORIZED."),
             RF(36.0f, 430.0f, 1270.0f, 40.0f), 10.5f, kText, juce::Justification::topLeft);

  s->plate(RF(16.0f, 490.0f, 1308.0f, 240.0f), u8("3 · ÉTATS"), Domain::Neutral);
  const NeuralCell::ArmState arms[4] = {NeuralCell::ArmState::Off, NeuralCell::ArmState::Armed, NeuralCell::ArmState::Active, NeuralCell::ArmState::Denied};
  for (int k = 0; k < 4; ++k) s->add<NeuralCell>(R(36 + k * 180, 526, 168, 40), NK::Arm, Domain::Neutral, "DRONE", "Drone").setArmState(arms[k]);
  s->caption(u8("ARMEMENT : OFF · ARMED (configuré, exécution suspendue) · ACTIVE (armé et autorisé) · DENIED (demande refusée)"),
             RF(36.0f, 572.0f, 720.0f, 14.0f), 9.5f, kMute);
  for (int k = 0; k < 2; ++k)
    s->add<NeuralCell>(R(780 + k * 180, 526, 168, 40), NK::Toggle, Domain::Wt, "SYNC", "WT Sync").setToggleState(k == 1, juce::dontSendNotification);
  s->caption(u8("INTERRUPTEUR : OFF · ON"), RF(780.0f, 572.0f, 340.0f, 14.0f), 9.5f, kMute);
  const Forced fs[4] = {Forced::Normal, Forced::Hover, Forced::Pressed, Forced::Focus};
  const char* fn[4] = {"NORMAL", "HOVER", "PRESSED", "FOCUS"};
  for (int k = 0; k < 4; ++k) {
    auto& c = s->add<NeuralCell>(R(36 + k * 180, 618, 168, 40), NK::Action, Domain::Wt, "APPLY WAVE", "Apply drawn wave");
    c.setForced(fs[k]);
    s->caption(fn[k], RF(36.0f + static_cast<float>(k) * 180.0f, 662.0f, 168.0f, 14.0f), 9.5f, kMute);
  }
  for (int k = 0; k < 2; ++k) {
    auto& c = s->add<NeuralCell>(R(780 + k * 180, 618, 168, 40), NK::Select, Domain::Fm, k == 0 ? "LINEAR FM" : "TZFM", "OP Mod Mode");
    c.setToggleState(k == 0, juce::dontSendNotification);
  }
  s->caption(u8("SÉLECTION : active · inactive"), RF(780.0f, 662.0f, 340.0f, 14.0f), 9.5f, kMute);
  return s;
}

// ================================================================ 4. SYNAPTIC TRACK
std::unique_ptr<Sheet> sheetTrack() {
  auto s = std::make_unique<Sheet>("SYNAPTIC TRACK", u8("Sélection discrète · segments, liste, onglets · indices de choix du registre inchangés, "
                                                         "affichage compact R2 · états actif / inactif / survol / focus / désactivé"),
                                   1340, 700);
  s->plate(RF(16.0f, 76.0f, 650.0f, 280.0f), "SEGMENTS", Domain::Neutral);
  s->add<SynapticTrack>(R(36, 110, 200, 46), ST::Segments, Domain::Neutral, "FB MODE", "DST WT1 D2 Feedback Mode", juce::StringArray{"LEGACY", "REGEN"}).setIndex(0);
  s->add<SynapticTrack>(R(256, 110, 240, 46), ST::Segments, Domain::Neutral, "FFT", "DST GL1 S2 FFT Size", juce::StringArray{"512", "1024", "2048", "4096"}).setIndex(2);
  s->add<SynapticTrack>(R(36, 180, 300, 46), ST::Segments, Domain::Mod, "SHAPE", "LFO1 Shape", juce::StringArray{"SINE", "TRI", "SAW UP", "SQR", "S&H"}).setIndex(2);
  s->add<SynapticTrack>(R(356, 180, 290, 46), ST::Segments, Domain::Fm, "MOD TYPE", "OP2 Mod Mode", juce::StringArray{"TZFM", "LIN FM", "EXP FM", "PM"}).setIndex(0);
  auto& hov = s->add<SynapticTrack>(R(36, 256, 240, 46), ST::Segments, Domain::Wt, "INTERP", "WT Interpolation", juce::StringArray{"LIN", "CUBIC", "SPECTRAL"});
  hov.setIndex(0);
  hov.setForced(Forced::Hover, 2);
  s->add<SynapticTrack>(R(296, 256, 160, 46), ST::Segments, Domain::Wt, "SYNC", "WT Sync", juce::StringArray{"OFF", "ON"}).setForced(Forced::Focus);
  s->add<SynapticTrack>(R(476, 256, 170, 46), ST::Segments, Domain::Cross, "DIR", "CROSS Direction", juce::StringArray{"FWD", "REV", "BOTH"}).setForced(Forced::Disabled);
  s->caption(u8("survol sur SPECTRAL"), RF(36.0f, 306.0f, 200.0f, 12.0f), 9.5f, kMute);
  s->caption(u8("focus clavier"), RF(296.0f, 306.0f, 160.0f, 12.0f), 9.5f, kMute);
  s->caption(u8("désactivé"), RF(476.0f, 306.0f, 160.0f, 12.0f), 9.5f, kMute);

  s->plate(RF(680.0f, 76.0f, 644.0f, 280.0f), u8("LISTES"), Domain::Neutral);
  const juce::StringArray dtypes{"OFF", "N1 FOLD", "D1 CRUSH", "Y1 TRANSIENT", "S1 RING/SHIFT", "N2 FB SHAPER", "D2 GLITCH", "Y2 ENV SHAPER", "S2 SPECTRAL"};
  s->add<SynapticTrack>(R(700, 110, 168, 46), ST::List, Domain::Neutral, "TYPE", "DST WT1 Type", dtypes).setIndex(6);
  s->add<SynapticTrack>(R(888, 110, 168, 46), ST::List, Domain::Neutral, "TYPE", "DST GL1 Type", dtypes).setIndex(4);
  const juce::StringArray host{"Off", "N1 Asymmetric Wavefolder", "D1 Variable Rate Crusher", "Y1 Transient Destroyer", "S1 Ring/Frequency Shifter"};
  s->add<SynapticTrack>(R(1076, 110, 228, 46), ST::List, Domain::Neutral, "TYPE (host labels)", "DST WT2 Type", host).setIndex(4);
  s->add<SynapticTrack>(R(700, 180, 168, 46), ST::List, Domain::Mod, "SOURCE", "Mod link source", juce::StringArray{"ENV AMP", "LFO1", "S&H", "M1"}).setIndex(1);
  auto& lf = s->add<SynapticTrack>(R(888, 180, 168, 46), ST::List, Domain::Mod, "CURVE", "Mod link curve", juce::StringArray{"LIN", "EXP", "LOG", "S-CURVE"});
  lf.setIndex(1);
  lf.setForced(Forced::Focus);
  s->caption(u8("Affichage compact R2 (S1 RING/SHIFT) et libellé hôte complet (S1 Ring/Frequency Shifter) : indice 4 identique. "
                "Une liste trop étroite tronque avec « … » ; le libellé hôte complet reste dans la liste déroulante et l'Inspector."),
             RF(700.0f, 240.0f, 600.0f, 60.0f), 10.0f, kDim, juce::Justification::topLeft);

  s->plate(RF(16.0f, 370.0f, 1308.0f, 300.0f), "ONGLETS", Domain::Neutral);
  s->add<SynapticTrack>(R(36, 410, 360, 30), ST::Tabs, Domain::Neutral, "", "Spaces", juce::StringArray{"PERFORMANCE", "SOUND DESIGN", "ENGINEERING"}).setIndex(0);
  s->add<SynapticTrack>(R(36, 470, 300, 30), ST::Tabs, Domain::Wt, "", "Sound Design tabs", juce::StringArray{"ENGINES", "PROCESS", "MOD"}).setIndex(1);
  s->add<SynapticTrack>(R(36, 530, 520, 30), ST::Tabs, Domain::Cross, "", "Engineering groups", juce::StringArray{"NETWORK", "MODULATION", "BANK LAB", "STATE", "TOOLS"}).setIndex(0);
  auto& sub = s->add<SynapticTrack>(R(36, 580, 240, 26), ST::Tabs, Domain::Cross, "", "Network tools", juce::StringArray{"FM", "CROSS"});
  sub.setIndex(1);
  auto& vt = s->add<SynapticTrack>(R(296, 580, 200, 26), ST::Tabs, Domain::Cross, "", "Network view", juce::StringArray{"NEURAL", "TABLE"});
  vt.setIndex(0);
  vt.setForced(Forced::Hover, 1);
  s->caption(u8("Espaces (3) · onglets SOUND DESIGN · groupes ENGINEERING (DA-A10 : 5 groupes, 10 outils) · outils du groupe NETWORK · "
                "Dual View NEURAL / TABLE (DA-A14 ; survol sur TABLE). Onglet actif : fond carbone et soulignement du domaine ; inactif : texte atténué."),
             RF(600.0f, 410.0f, 700.0f, 80.0f), 10.5f, kDim, juce::Justification::topLeft);
  return s;
}

// ================================================================ 5. CYBERNETIC DISPLAY
std::unique_ptr<Sheet> sheetDisplays() {
  auto s = std::make_unique<Sheet>("Cybernetic Display", u8("DA-A05 option D (et non Optical Glass Hybrid) · graduations techniques, marqueurs lumineux maîtrisés, "
                                                             "repères optiques, hiérarchie stricte · source nommée sur chaque afficheur · aucune mesure inventée"),
                                   1340, 830);
  const DV vs[9] = {DV::Numeric, DV::Scope, DV::Spectrum, DV::Wavetable, DV::ModCurve, DV::Envelope, DV::Meter, DV::Autonomy, DV::Diagnostic};
  const Domain ds[9] = {Domain::Neutral, Domain::Fm, Domain::Fm, Domain::Wt, Domain::Mod, Domain::Mod, Domain::Neutral, Domain::Neutral, Domain::Neutral};
  for (int k = 0; k < 9; ++k) {
    const int x = 16 + (k % 3) * 440, y = 80 + (k / 3) * 248;
    s->add<CyberDisplay>(R(x, y, 428, 236), vs[k], ds[k], demoDisplay(vs[k], 0.7f));
  }
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
  s->add<CyberDisplay>(R(120, 414, 190, 190), DV::Meter, Domain::Neutral, md);
  s->caption(u8("OUT GAIN est un PARAMÈTRE (réglage, automatisable, plasma). PEAK est une MESURE (crête de bloc en sortie finale, "
                "afficheur dédié, unité dBFS). La mesure n'est jamais dessinée dans le canal du paramètre. Rouge seulement au-delà de 0 dBFS."),
             RF(330.0f, 430.0f, 960.0f, 60.0f), 11.0f, kText, juce::Justification::topLeft);
  return s;
}

// ================================================================ 7. ÉTATS
std::unique_ptr<Sheet> sheetStates() {
  auto s = std::make_unique<Sheet>(u8("États interactifs"), u8("Normal · Hover · Focus · Pressed · Selected · Disabled · Modulated · Min · Mid · Max · Négatif — "
                                                                 "différenciés par forme, témoin et texte, pas seulement par la couleur"),
                                   1420, 660);
  const char* cols[11] = {"NORMAL", "HOVER", "FOCUS", "PRESSED", "SELECTED", "DISABLED", "MODULATED", "MIN", "MID", "MAX", "NEGATIVE"};
  const int x0 = 130, cw = 116;
  for (int c = 0; c < 11; ++c) s->caption(cols[c], RF(static_cast<float>(x0 + c * cw), 74.0f, static_cast<float>(cw), 14.0f), 9.5f, kDim, juce::Justification::centred, true);
  const char* rows[5] = {"NEXUS S", "PLASMA V", "NEURAL\ntoggle", "NEURAL\naction", "SYNAPTIC\nsegments"};
  const int ry[5] = {96, 214, 410, 476, 548};
  for (int r = 0; r < 5; ++r) s->caption(rows[r], RF(20.0f, static_cast<float>(ry[r]) + 10.0f, 100.0f, 30.0f), 11.0f, kText, juce::Justification::topLeft, true);
  const auto na = [&](int r, int c) {
    s->caption(u8("n/a"), RF(static_cast<float>(x0 + c * cw), static_cast<float>(ry[r]) + 14.0f, static_cast<float>(cw), 14.0f), 9.5f, kMute,
               juce::Justification::centred);
  };
  for (int c = 0; c < 11; ++c) {
    const int cx = x0 + c * cw;
    // NEXUS S
    {
      const bool bip = c == 10;
      auto& v = s->val(bip ? -1.0f : 0.0f, 1, 0, 0.5f, "", 2, bip);
      if (c == 7) v.normValue(0.0f);
      if (c == 8) v.normValue(0.5f);
      if (c == 9) v.normValue(1.0f);
      if (c == 10) v.value(-0.6f);
      if (c == 6) v.effective(0.78f);
      if (c == 4) na(0, c);
      else {
        auto& n = s->add<Nexus>(R(cx + 16, ry[0], 84, 96), Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
        const Forced f[6] = {Forced::Normal, Forced::Hover, Forced::Focus, Forced::Pressed, Forced::Normal, Forced::Disabled};
        n.setForced(c < 6 ? f[c] : Forced::Normal);
      }
    }
    // PLASMA V
    {
      const bool bip = c == 10;
      auto& v = s->val(bip ? -1.0f : 0.0f, 1, 0, 0.5f, "", 2, bip);
      if (c == 7) v.normValue(0.0f);
      if (c == 9) v.normValue(1.0f);
      if (c == 10) v.value(-0.6f);
      if (c == 6) v.effective(0.8f);
      if (c == 4) na(1, c);
      else {
        auto& p = s->add<PlasmaChannel>(R(cx + 26, ry[1], 64, 180), PO::Vertical, Domain::Cross, "LVL", "CROSS level", v, juce::StringArray{"0", "1"});
        const Forced f[6] = {Forced::Normal, Forced::Hover, Forced::Focus, Forced::Pressed, Forced::Normal, Forced::Disabled};
        p.setForced(c < 6 ? f[c] : Forced::Normal);
      }
    }
    // NEURAL toggle et action
    if (c <= 5) {
      auto& tg = s->add<NeuralCell>(R(cx + 4, ry[2], 108, 40), NK::Toggle, Domain::Wt, "SYNC", "WT Sync");
      const Forced f[6] = {Forced::Normal, Forced::Hover, Forced::Focus, Forced::Pressed, Forced::Normal, Forced::Disabled};
      tg.setForced(f[c]);
      if (c == 4) tg.setToggleState(true, juce::dontSendNotification);
      if (c != 4) {
        auto& ac = s->add<NeuralCell>(R(cx + 4, ry[3], 108, 40), NK::Action, Domain::Wt, "APPLY", "Apply drawn wave");
        ac.setForced(f[c]);
      } else na(3, c);
    } else {
      na(2, c);
      na(3, c);
    }
    // SYNAPTIC
    if (c <= 5 && c != 3) {
      auto& st = s->add<SynapticTrack>(R(cx + 2, ry[4], 112, 46), ST::Segments, Domain::Fm, "SYNC", "OP Sync", juce::StringArray{"OFF", "ON"});
      st.setIndex(c == 4 ? 1 : 0);
      if (c == 1) st.setForced(Forced::Hover, 1);
      if (c == 2) st.setForced(Forced::Focus);
      if (c == 5) st.setForced(Forced::Disabled);
    } else na(4, c);
  }
  s->caption(u8("MODULATED : anneau/triangle EFFECTIVE en plus de la BASE. MIN/MID/MAX : valeurs normalisées 0 / 0,5 / 1. NEGATIVE : paramètre bipolaire, arc depuis le centre. "
                "HOVER, FOCUS, PRESSED et DISABLED sont imposés pour la capture (aucune souris réelle). « n/a » : état sans objet pour la famille."),
             RF(20.0f, 620.0f, 1380.0f, 50.0f), 10.5f, kDim, juce::Justification::topLeft);
  return s;
}

// ================================================================ 8. COULEURS PAR MOTEUR
std::unique_ptr<Sheet> sheetDomains() {
  auto s = std::make_unique<Sheet>("Couleurs par moteur", u8("WT #5CE2CB · FM #F2A05E · CROSS #B095FF · MOD #7FA9E8 · mêmes composants, seule la couleur "
                                                              "de domaine change · EFFECTIVE toujours en MOD"),
                                   1340, 640);
  const Domain ds[4] = {Domain::Wt, Domain::Fm, Domain::Cross, Domain::Mod};
  for (int k = 0; k < 4; ++k) {
    const float x = 16.0f + static_cast<float>(k) * 330.0f;
    s->plate(RF(x, 76.0f, 318.0f, 540.0f), domainName(ds[k]), ds[k]);
    const int xi = static_cast<int>(x);
    auto& a = s->val(0, 1, 0, 0.62f, "", 2);
    a.effective(0.4f).routes({0.2f});
    s->add<Nexus>(R(xi + 20, 110, 84, 96), NX::S, ds[k], "AMOUNT", "Amount", a);
    auto& b = s->val(-1, 1, 0, -0.35f, "", 2, true);
    s->add<Nexus>(R(xi + 116, 110, 84, 96), NX::S, ds[k], "OFFSET", "Offset", b);
    auto& c = s->val(0, 1, 0, 0.3f, "", 2);
    s->add<Nexus>(R(xi + 222, 120, 64, 74), NX::Micro, ds[k], "DEPTH", "Depth", c);
    auto& p = s->val(0, 1, 0, 0.7f, "", 2);
    p.effective(0.85f);
    s->add<PlasmaChannel>(R(xi + 14, 220, 290, 44), PO::Horizontal, ds[k], "LEVEL", "Level", p, juce::StringArray{"0", "1"});
    s->add<NeuralCell>(R(xi + 14, 280, 140, 40), NK::Toggle, ds[k], "SYNC", "Sync").setToggleState(true, juce::dontSendNotification);
    auto& sel = s->add<NeuralCell>(R(xi + 164, 280, 140, 40), NK::Select, ds[k], "SELECTED", "Selected");
    sel.setToggleState(true, juce::dontSendNotification);
    s->add<SynapticTrack>(R(xi + 14, 330, 290, 46), ST::Segments, ds[k], "MODE", "Mode", juce::StringArray{"A", "B", "C"}).setIndex(1);
    auto d = demoDisplay(DV::Scope, 0.4f * static_cast<float>(k));
    d.title = juce::String(domainName(ds[k])) + " SCOPE";
    s->add<CyberDisplay>(R(xi + 14, 392, 290, 206), DV::Scope, ds[k], d);
  }
  return s;
}

// ================================================================ 12. PLAQUES (WT et CROSS)
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
  s.add<CyberDisplay>(R(x + 12, y + 190, 300, 196), DV::Wavetable, Domain::Wt, wd);
  s.add<NeuralCell>(R(x + 322, y + 194, 106, 40), NK::Toggle, Domain::Wt, "SYNC", "WT Sync");
  s.add<SynapticTrack>(R(x + 322, y + 244, 106, 46), ST::Segments, Domain::Wt, "PH MODE", "WT Phase Mode", juce::StringArray{"FREE", "RESET"}).setIndex(1);
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
  auto& a = s.val(0, 1, 0.5f, 0.5f, "", 2);
  s.add<Nexus>(R(x + 196, y + 34, 84, 96), NX::S, Domain::Cross, "XAUDIO AMT", "Cross Audio Amount (post-chain)", a);
  auto& m = s.val(0, 1, 1, 0.64f, "", 2);
  m.effective(0.78f);
  s.add<PlasmaChannel>(R(x + 294, y + 30, 64, 180), PO::Vertical, Domain::Cross, "CROSS LVL", "CROSS Master (CROSS level)", m, juce::StringArray{"0", ".5", "1"});
  s.add<NeuralCell>(R(x + 370, y + 40, 84, 40), NK::Action, Domain::Cross, "RESET", "Cross Reset (clear CROSS history)");
  s.add<SynapticTrack>(R(x + 12, y + 140, 276, 46), ST::Segments, Domain::Cross, "XAUDIO MODE", "Cross Audio Mode (post-chain)",
                       juce::StringArray{"OFF", "XFADE", "RING", "AM"})
      .setIndex(1);
  auto& b = s.val(-1, 1, 0, -0.2f, "", 2, true);
  s.add<PlasmaChannel>(R(x + 12, y + 224, 436, 44), PO::Horizontal, Domain::Cross, "BALANCE  PRE-CHAIN", "WT/FM Balance (audio, pre-chain)", b,
                       juce::StringArray{"WT", "0", "FM"});
  s.add<SynapticTrack>(R(x + 12, y + 284, 436, 30), ST::Tabs, Domain::Cross, "", "CROSS sections",
                       juce::StringArray{"DIRECT", "WT" + kArrow + "FM", "FM" + kArrow + "WT", "BUS / XAUDIO"})
      .setIndex(0);
  s.caption(u8("BALANCE : balance audio avant les chaînes (pré-chaînes), distincte de XAUDIO AMT (après les chaînes) et de CROSS LVL."),
            RF(static_cast<float>(x) + 12.0f, static_cast<float>(y) + 330.0f, 436.0f, 50.0f), 10.0f, kDim, juce::Justification::topLeft);
}

std::unique_ptr<Sheet> sheetPlates() {
  auto s = std::make_unique<Sheet>("Regroupement en plaques", u8("Exemple d'assemblage WT et CROSS avec les composants R2 (disposition indicative, pas la "
                                                                  "disposition R2-C) · sérigraphie R2 proposée, non validée"),
                                   960, 500);
  wtPlate(*s, 16, 80);
  crossPlate(*s, 476, 80);
  return s;
}

// ================================================================ 1. PLANCHE COMPLÈTE
}  // namespace

std::unique_ptr<Sheet> buildBoard() {
  auto s = std::make_unique<Sheet>("SUBDESTRUCKT R2 Component Lab", u8("Planche complète · NEXUS · NEURAL CELL · SYNAPTIC TRACK · Cybernetic Display · "
                                                                        "PLASMA CHANNEL · SYMBIOTIC INDUSTRIAL Hybrid · aucune matière biomécanique hors Core"),
                                   1840, 1150);
  // barre globale (repères permanents)
  s->plate(RF(16.0f, 76.0f, 1808.0f, 64.0f), "", Domain::Neutral);
  s->add<SynapticTrack>(R(30, 92, 400, 30), ST::Tabs, Domain::Neutral, "", "Spaces", juce::StringArray{"PERFORMANCE", "SOUND DESIGN", "ENGINEERING"});
  s->add<NeuralCell>(R(450, 88, 112, 40), NK::Action, Domain::Neutral, "PRESETS", "Open preset browser");
  s->add<NeuralCell>(R(568, 88, 96, 40), NK::Action, Domain::Neutral, "SAVE" + kEll, "Save preset");
  s->add<NeuralCell>(R(670, 88, 84, 40), NK::Action, Domain::Neutral, "UNDO", "Undo");
  s->add<NeuralCell>(R(760, 88, 84, 40), NK::Action, Domain::Neutral, "REDO", "Redo").setEnabled(false);
  s->add<NeuralCell>(R(990, 88, 150, 40), NK::Arm, Domain::Neutral, "DRONE", "Drone").setArmState(NeuralCell::ArmState::Armed);
  auto& au = s->add<NeuralCell>(R(1146, 88, 144, 40), NK::Action, Domain::Neutral, "AUTHORIZE", "Authorize autonomous states (this instance)");
  au.setIcon(NeuralCell::Icon::Authorize);
  auto pk = demoDisplay(DV::Numeric, 0.0f);
  pk.title = "PEAK";
  pk.source = "measure";
  pk.value = kMinus + "3.2";
  pk.unit = "dBFS";
  pk.secondary = "";
  s->add<NeuralCell>(R(1296, 88, 134, 40), NK::Toggle, Domain::Mod, "INSPECTOR", "Inspector").setToggleState(true, juce::dontSendNotification);
  s->add<CyberDisplay>(R(1436, 80, 250, 56), DV::Numeric, Domain::Neutral, pk);
  s->add<NeuralCell>(R(1700, 88, 112, 40), NK::Safety, Domain::Alert, "PANIC", "Panic");

  wtPlate(*s, 16, 152);
  crossPlate(*s, 466, 152);
  // FM
  s->plate(RF(936.0f, 152.0f, 420.0f, 400.0f), "FM", Domain::Fm, "4 operators");
  for (int k = 0; k < 4; ++k) {
    auto& rv = s->val(0.5f, 16, 1, 1.0f + static_cast<float>(k), "x", 2);
    s->add<Nexus>(R(948 + k * 100, 186, 84, 96), NX::S, Domain::Fm, "RATIO", "OP" + juce::String(k + 1) + " Ratio", rv);
    auto& lv = s->val(0, 1, 1, 0.9f - 0.2f * static_cast<float>(k), "", 2);
    s->add<Nexus>(R(958 + k * 100, 290, 64, 74), NX::Micro, Domain::Fm, "LVL", "OP" + juce::String(k + 1) + " Level", lv);
  }
  auto& fi = s->val(0, 8, 1, 2.4f, "rad", 2);
  fi.effective(0.42f).routes({0.1f, -0.2f});
  s->add<Nexus>(R(948, 374, 132, 150), NX::XL, Domain::Fm, "FM INDEX", "FM OP2>OP1 (contextual)", fi);
  s->add<SynapticTrack>(R(1094, 380, 250, 46), ST::Segments, Domain::Fm, "MOD TYPE", "OP1 Mod Mode", juce::StringArray{"TZFM", "LIN FM", "EXP FM", "PM"});
  s->add<NeuralCell>(R(1094, 440, 120, 40), NK::Toggle, Domain::Fm, "SYNC", "OP1 Sync").setToggleState(true, juce::dontSendNotification);
  s->add<SynapticTrack>(R(1094, 490, 250, 46), ST::List, Domain::Fm, "TOPOLOGY", "FM topology preset", juce::StringArray{"STACK 4", "2 PAIRS", "3+1", "PARALLEL"});
  // afficheurs
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
  s->add<NeuralCell>(R(30, 870, 180, 40), NK::Toggle, Domain::Neutral, "FREEZE", "DST WT1 D2 Freeze").setToggleState(true, juce::dontSendNotification);
  s->add<NeuralCell>(R(220, 870, 180, 40), NK::Arm, Domain::Neutral, "ARM", "DST WT1 Autonomous Armed").setArmState(NeuralCell::ArmState::Armed);
  auto& cap = s->add<NeuralCell>(R(410, 870, 190, 40), NK::Action, Domain::Neutral, "CAPTURE", "Capture frozen D2/S2 content");
  cap.setIcon(NeuralCell::Icon::Capture);
  s->add<NeuralCell>(R(410, 920, 190, 40), NK::Action, Domain::Neutral, "EXCITE", "Manual excitation (F4)");
  s->caption(u8("FREEZE (bascule, automatisable) · ARM (configuration) · CAPTURE (action) : trois opérations, trois formes."),
             RF(30.0f, 966.0f, 570.0f, 20.0f), 10.0f, kDim, juce::Justification::topLeft);
  s->add<CyberDisplay>(R(30, 984, 570, 140), DV::Autonomy, Domain::Neutral, demoDisplay(DV::Autonomy, 0.0f));
  // MOD
  s->plate(RF(630.0f, 566.0f, 600.0f, 566.0f), u8("MOD  ·  ENVELOPES / LFO"), Domain::Mod);
  s->add<SynapticTrack>(R(644, 600, 400, 30), ST::Tabs, Domain::Mod, "", "Envelopes", juce::StringArray{"AMP", "WT", "FM", "AUX1", "AUX2"});
  s->add<CyberDisplay>(R(644, 640, 572, 190), DV::Envelope, Domain::Mod, demoDisplay(DV::Envelope, 0.0f));
  const char* env[4] = {"ATK", "DEC", "SUS", "REL"};
  for (int k = 0; k < 4; ++k) {
    auto& v = s->val(0, 1, 0, 0.15f + 0.2f * static_cast<float>(k), k == 2 ? "" : "s", 2);
    s->add<Nexus>(R(644 + k * 92, 840, 84, 96), NX::S, Domain::Mod, env[k], juce::String("Amp ") + env[k], v);
  }
  s->add<CyberDisplay>(R(1016, 840, 200, 150), DV::ModCurve, Domain::Mod, demoDisplay(DV::ModCurve, 0.0f));
  auto& rate = s->val(0.01f, 40, 1, 4.2f, "Hz", 2);
  rate.effective(0.3f);
  s->add<Nexus>(R(644, 950, 84, 96), NX::S, Domain::Mod, "RATE", "LFO1 Rate", rate);
  s->add<SynapticTrack>(R(740, 960, 260, 46), ST::Segments, Domain::Mod, "SHAPE", "LFO1 Shape", juce::StringArray{"SINE", "TRI", "SAW UP", "SQR"});
  s->add<NeuralCell>(R(740, 1020, 120, 40), NK::Toggle, Domain::Mod, "SYNC", "LFO1 Tempo Sync");
  // SORTIE
  s->plate(RF(1244.0f, 566.0f, 580.0f, 566.0f), u8("OUTPUT  ·  SAFETY"), Domain::Neutral);
  auto& og = s->val(-48, 6, -6, -6, "dB", 1);
  s->add<PlasmaChannel>(R(1262, 600, 64, 180), PO::Vertical, Domain::Neutral, "OUT GAIN", "Output Gain", og,
                        juce::StringArray{kMinus + "48", kMinus + "24", "0", "+6"});
  s->add<CyberDisplay>(R(1340, 600, 200, 200), DV::Meter, Domain::Neutral, demoDisplay(DV::Meter, 0.0f));
  s->add<NeuralCell>(R(1556, 604, 150, 40), NK::Toggle, Domain::Neutral, "LIMITER", "Musical Limiter").setToggleState(true, juce::dontSendNotification);
  auto& ce = s->val(-24, 0, -0.3f, -0.3f, "dB", 1);
  s->add<Nexus>(R(1556, 654, 84, 96), NX::S, Domain::Neutral, "CEILING", "Limiter Ceiling", ce);
  s->add<CyberDisplay>(R(1262, 816, 548, 300), DV::Diagnostic, Domain::Neutral, demoDisplay(DV::Diagnostic, 0.0f));
  return s;
}

namespace {

// ================================================================ 9. ZOOMS 75 / 100 / 150 %
std::unique_ptr<Sheet> zoomCluster() {
  auto s = std::make_unique<Sheet>("", "", 560, 330);
  auto& z = s->val(0, 1, 0, 0.35f, "", 2);
  z.effective(0.5f);
  s->add<Nexus>(R(10, 10, 132, 150), NX::XL, Domain::Wt, "WT Z", "WT Z Master", z).setLabelFontSize(tok2::fontCritical + 0.5f);
  auto& c = s->val(0, 1, 1, 0.64f, "", 2);
  s->add<Nexus>(R(150, 20, 84, 96), NX::S, Domain::Cross, "XAUDIO AMT", "Cross Audio Amount", c);
  auto& m = s->val(-1, 1, 0, -0.3f, "", 2, true);
  s->add<Nexus>(R(244, 30, 64, 74), NX::Micro, Domain::Cross, "DEPTH", "CROSS OP1>WT X", m);
  auto& g = s->val(0, 1, 1, 0.64f, "", 2);
  s->add<PlasmaChannel>(R(318, 10, 64, 180), PO::Vertical, Domain::Cross, "CROSS LVL", "CROSS Master", g, juce::StringArray{"0", ".5", "1"});
  s->add<NeuralCell>(R(392, 14, 160, 40), NK::Toggle, Domain::Neutral, "FREEZE", "DST WT1 D2 Freeze").setToggleState(true, juce::dontSendNotification);
  s->add<NeuralCell>(R(392, 62, 160, 40), NK::Arm, Domain::Neutral, "ARM", "Autonomous Armed").setArmState(NeuralCell::ArmState::Armed);
  s->add<NeuralCell>(R(392, 110, 160, 40), NK::Safety, Domain::Alert, "PANIC", "Panic");
  s->add<SynapticTrack>(R(10, 180, 300, 46), ST::Segments, Domain::Neutral, "FB MODE", "Feedback Mode", juce::StringArray{"LEGACY", "REGEN"});
  auto pk = demoDisplay(DV::Meter, 0.0f);
  s->add<CyberDisplay>(R(392, 160, 160, 160), DV::Meter, Domain::Neutral, pk);
  s->add<SynapticTrack>(R(10, 240, 370, 46), ST::List, Domain::Neutral, "TYPE", "DST Type", juce::StringArray{"S1 RING/SHIFT"});
  return s;
}

juce::Image composeZoom(juce::StringArray& log) {
  const float zs[3] = {0.75f, 1.0f, 1.5f};
  std::vector<juce::Image> imgs;
  for (float z : zs) {
    auto c = zoomCluster();
    imgs.push_back(c->createComponentSnapshot(c->getLocalBounds(), true, z));
  }
  int w = 40, h = 0;
  for (auto& i : imgs) {
    w += i.getWidth() + 40;
    h = std::max(h, i.getHeight());
  }
  juce::Image out(juce::Image::ARGB, w, h + 150, true);
  juce::Graphics g(out);
  g.fillAll(t::chassis);
  g.setFont(labelFont(20.0f, true));
  g.setColour(t::text);
  g.drawText(u8("Comparatif 75 / 100 / 150 %"), 24, 12, w - 48, 26, juce::Justification::centredLeft);
  g.setFont(labelFont(11.0f));
  g.setColour(t::textDim);
  g.drawText(u8("Même groupe de composants rendu au facteur du zoom, en PIXELS ÉCRAN RÉELS (densité x1, cas le plus défavorable ; sur Retina, x2). "
                "Libellés critiques au corps logique 12 (XL 12,5) : 9 px écran à 75 %."),
             24, 40, w - 48, 16, juce::Justification::centredLeft);
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
  log.add("zoom sheet composed");
  return out;
}

// ================================================================ 10. CAS TYPOGRAPHIQUES CRITIQUES
class TypoCases : public juce::Component {
 public:
  void paint(juce::Graphics& g) override {
    struct Case {
      juce::String label;
      const char* family;
      float body, budget;
      bool bold, two;
      juce::String note;
    };
    const Case cs[] = {
        {"WT Z", "NEXUS XL", 12.5f, 128.0f, true, true, u8("critique · homonymie Z Master levée")},
        {"Z " + kDot + " CROSS LVL", "PLASMA V (Codex 70 px)", 12.0f, 70.0f, false, true, u8("critique · deux lignes")},
        {"CROSS LVL", "PLASMA V (Codex 70 px)", 12.0f, 70.0f, false, true, "critique"},
        {"X " + kDot + " WT" + kArrow + "FM", "NEXUS S (80 px)", 12.0f, 80.0f, false, true, u8("critique · axe Core")},
        {"Y " + kDot + " FM" + kArrow + "WT", "NEXUS S (80 px)", 12.0f, 80.0f, false, true, u8("critique · axe Core")},
        {"XAUDIO AMT", "NEXUS S (80 px)", 11.0f, 80.0f, false, true, u8("abréviation à valider")},
        {"OUT GAIN", "NEXUS S (92 px)", 11.0f, 92.0f, false, true, "recommandation architecte (vs GAIN)"},
        {"PEAK " + kMinus + "3.2 dBFS", "barre (120 px)", 12.0f, 120.0f, true, false, u8("critique · mesure")},
        {"SUSPENDED", "barre (96 px)", 12.0f, 96.0f, true, false, u8("critique · état d'autonomie")},
        {"S1 RING/SHIFT", "liste (136 px)", 11.0f, 136.0f, false, false, u8("option compacte")},
        {"S1 Ring/Frequency Shifter", "liste (136 px)", 11.0f, 136.0f, false, false, u8("libellé hôte : tronqué « … »")},
        {"SPECTRAL", "segment (48,7 px)", 11.0f, 48.67f, false, false, "marge faible"},
        {"WT>FM", "NEXUS S (80 px)", 12.0f, 80.0f, false, true, u8("repli ASCII de WT→FM")}};
    float y = 8.0f;
    g.setFont(labelFont(9.5f, true));
    g.setColour(t::textDim);
    g.drawText(u8("LIBELLÉ"), 8, static_cast<int>(y), 220, 12, juce::Justification::centredLeft);
    g.drawText(u8("FAMILLE · BUDGET"), 236, static_cast<int>(y), 200, 12, juce::Justification::centredLeft);
    g.drawText(u8("RENDU DANS LE BUDGET (100 %)"), 440, static_cast<int>(y), 220, 12, juce::Justification::centredLeft);
    g.drawText(u8("MESURE (cette plateforme)"), 690, static_cast<int>(y), 220, 12, juce::Justification::centredLeft);
    g.drawText(u8("75 % écran"), 930, static_cast<int>(y), 120, 12, juce::Justification::centredLeft);
    y += 20.0f;
    for (const auto& c : cs) {
      const auto f = labelFont(c.body, c.bold);
      const float w = draw::textWidth(f, c.label);
      const bool fits = w <= c.budget;
      bool twoOk = false;
      if (!fits && c.two && c.label.containsChar(' ')) {
        float best = 1.0e9f;
        for (int k = c.label.indexOfChar(' '); k >= 0; k = c.label.indexOfChar(k + 1, ' '))
          best = std::min(best, std::max(draw::textWidth(f, c.label.substring(0, k)), draw::textWidth(f, c.label.substring(k + 1))));
        twoOk = best <= c.budget;
      }
      const juce::String verdict = fits ? (c.budget - w < 8.0f ? "OK, MARGIN < 8" : "OK") : (twoOk ? "2 LINES" : "OVERFLOW");
      const juce::Colour vc = fits ? (c.budget - w < 8.0f ? t::warn : t::wt) : (twoOk ? t::mod : t::alert);
      const float rh = (fits || !c.two) ? 26.0f : 40.0f;
      g.setFont(labelFont(11.0f, true));
      g.setColour(t::text);
      g.drawText(c.label, 8, static_cast<int>(y), 220, static_cast<int>(rh), juce::Justification::centredLeft);
      g.setFont(labelFont(10.0f));
      g.setColour(t::textDim);
      g.drawText(juce::String(c.family) + u8(" · corps ") + juce::String(c.body, 1), 236, static_cast<int>(y), 200, static_cast<int>(rh),
                 juce::Justification::centredLeft);
      auto box = RF(440.0f, y + 2.0f, c.budget, rh - 4.0f);
      g.setColour(t::carbon);
      g.fillRect(box);
      g.setColour(t::panelEdge);
      g.drawRect(box, 1.0f);
      {
        juce::Graphics::ScopedSaveState ss(g);
        g.reduceClipRegion(box.expanded(60.0f, 0.0f).toNearestInt());
        draw::fittedText(g, c.label, box, f, t::text, juce::Justification::centred, c.two);
      }
      g.setFont(valueFont(10.0f));
      g.setColour(t::text);
      g.drawText(juce::String(w, 1) + " / " + juce::String(c.budget, 1) + " px", 690, static_cast<int>(y), 140, static_cast<int>(rh), juce::Justification::centredLeft);
      g.setColour(vc);
      g.setFont(labelFont(10.0f, true));
      g.drawText(verdict, 830, static_cast<int>(y), 100, static_cast<int>(rh), juce::Justification::centredLeft);
      g.setFont(valueFont(10.0f));
      g.setColour(c.body * 0.75f >= 9.0f ? t::text : t::textMute);
      g.drawText(juce::String(c.body * 0.75f, 2) + " px", 930, static_cast<int>(y), 80, static_cast<int>(rh), juce::Justification::centredLeft);
      g.setFont(labelFont(9.5f));
      g.setColour(t::textMute);
      g.drawText(c.note, 1010, static_cast<int>(y), 300, static_cast<int>(rh), juce::Justification::centredLeft);
      y += rh + 6.0f;
    }
    // glyphes Unicode et repli ASCII (PT-05)
    y += 10.0f;
    g.setFont(labelFont(12.0f, true));
    g.setColour(t::text);
    g.drawText(u8("GLYPHES (PT-05) · rendu réel dans les polices de l'interface"), 8, static_cast<int>(y), 700, 16, juce::Justification::centredLeft);
    y += 24.0f;
    const juce::String glyphs[4] = {kArrow, kDot, kEll, kMinus};
    const char* gn[4] = {"U+2192 →", "U+00B7 ·", "U+2026 …", "U+2212 −"};  // utf8-ok
    const char* ascii[4] = {">", ".", "...", "-"};
    for (int k = 0; k < 4; ++k) {
      float x = 8.0f;
      g.setFont(labelFont(10.0f, true));
      g.setColour(t::textDim);
      g.drawText(u8(gn[k]), static_cast<int>(x), static_cast<int>(y), 90, 22, juce::Justification::centredLeft);
      x += 96.0f;
      for (float sz : {9.0f, 10.0f, 11.0f, 12.0f, 14.0f}) {
        for (int m = 0; m < 2; ++m) {
          const auto f = m == 0 ? labelFont(sz) : valueFont(sz);
          const auto tf = f.getTypefacePtr();
          const auto cp = glyphs[k][0];
          const bool native = tf != nullptr && tf->getNominalGlyphForCodepoint(cp).has_value();
          g.setFont(f);
          g.setColour(native ? t::text : t::alert);
          g.drawText("A" + glyphs[k] + "B", static_cast<int>(x), static_cast<int>(y), 44, 22, juce::Justification::centredLeft);
          x += 46.0f;
        }
      }
      g.setFont(labelFont(10.0f));
      g.setColour(t::textMute);
      g.drawText(juce::String(u8("repli ASCII : « ")) + ascii[k] + u8(" »"), static_cast<int>(x) + 10, static_cast<int>(y), 160, 22, juce::Justification::centredLeft);
      y += 26.0f;
    }
    g.setFont(labelFont(9.5f));
    g.setColour(t::textMute);
    g.drawText(u8("Colonnes : corps 9 / 10 / 11 / 12 / 14 px, police d'interface puis police des valeurs. Rouge = glyphe absent de la police (substitution)."), 8,
               static_cast<int>(y) + 4, 1200, 14, juce::Justification::centredLeft);
  }
};

std::unique_ptr<Sheet> sheetTypo() {
  auto s = std::make_unique<Sheet>("Cas typographiques critiques",
                                   u8("PT-02 (extrait) et PT-05 · mesures JUCE réelles sur la plateforme de rendu · budgets Codex · verdict : OK / marge < 8 / 2 lignes / dépassement"),
                                   1340, 820);
  s->add<TypoCases>(R(16, 76, 1308, 730));
  return s;
}

}  // namespace

bool renderSheets(const juce::File& dir, float scale, juce::StringArray& log) {
  bool ok = true;
  const auto snap = [&](juce::Component& c, const juce::String& name, float sc) {
    const auto img = c.createComponentSnapshot(c.getLocalBounds(), true, sc);
    const auto f = dir.getChildFile(name);
    const bool w = writePng(img, f);
    log.add(name + "  " + juce::String(img.getWidth()) + "x" + juce::String(img.getHeight()) + (w ? "" : "  WRITE FAILED"));
    ok = ok && w;
  };
  { auto b = buildBoard(); snap(*b, "01-component-lab-board.png", scale); }
  { auto s = sheetNexus(); snap(*s, "02-nexus.png", scale); }
  { auto s = sheetNeural(); snap(*s, "03-neural-cell.png", scale); }
  { auto s = sheetTrack(); snap(*s, "04-synaptic-track.png", scale); }
  { auto s = sheetDisplays(); snap(*s, "05-cybernetic-display.png", scale); }
  { auto s = sheetPlasma(); snap(*s, "06-plasma-channel.png", scale); }
  { auto s = sheetStates(); snap(*s, "07-interactive-states.png", scale); }
  { auto s = sheetDomains(); snap(*s, "08-engine-colours.png", scale); }
  {
    const auto img = composeZoom(log);
    const bool w = writePng(img, dir.getChildFile("09-zoom-75-100-150.png"));
    log.add("09-zoom-75-100-150.png  " + juce::String(img.getWidth()) + "x" + juce::String(img.getHeight()) + (w ? "" : "  WRITE FAILED"));
    ok = ok && w;
  }
  { auto s = sheetTypo(); snap(*s, "10-critical-typography.png", scale); }
  { auto s = sheetPlates(); snap(*s, "12-plate-grouping.png", scale); }
  return ok;
}

}  // namespace sbd::lab
