// SUBDESTRUCKT — ARCH-UI-04-R2-B2-03 — VIS-01 : étape 2 de la chaîne de preuve visuelle CE-B2-03
// (référence R1 HISTORIQUE -> composant intégré en PRODUCTION).
//
// Méthode. Chaque planche historique est reconstruite par son code de laboratoire, prouvé identique aux références à
// l'étape 1 (B2-02). Le composant R1 du laboratoire visé est masqué et remplacé, aux mêmes bornes, par le composant de
// PRODUCTION lié au paramètre réel correspondant : même valeur normalisée, même libellé, même valeur Effective, mêmes
// routes. La planche est rendue à l'échelle de la référence, avec la chaîne de SHA de la référence (--sha), puis
// lab/tools/vis01_compare.py la compare à la capture historique publiée :
//   - dans les bornes substituées : conformité du composant de production, décomposée en fenêtre de valeur et reste ;
//   - hors de ces bornes : contrôle, qui doit être identique, sinon la comparaison est invalide.
// Les références ne sont jamais écrites. La valeur Effective est injectée dans la télémétrie publiée du processeur
// (moteur arrêté), comme le moteur la publie : c'est un état représentatif, documenté comme tel. Les routes passent par la
// transaction de production (modAdd) puis par la copie publiée de l'éditeur (UiContext::mod).
#include <cmath>
#include <iostream>

#include "app/CompareScene.h"

namespace sbd::lab {

namespace {

using ui::Density;

struct NexusCase {
  const char* sheet;           // "01", "06a", "09", "11", "12"
  juce::Rectangle<int> r;      // bornes logiques dans la planche
  ParamId id;
  Density d;
  const char* label;           // libellé de la référence (UTF-8)
  float norm;                  // valeur normalisée de la référence ; < 0 : valeur déjà fixée par la scène (planche 12)
  float eff;                   // Effective normalisée de la référence ; < 0 : aucune
  std::vector<float> routes;   // décalages normalisés des routes de la référence
  ui::Form form = ui::Form::Rotary;  // Rotary : NEXUS ; LinearV : PLASMA vertical
  bool disabled = false;
};

const char* kArrow = "\xe2\x86\x92";

std::vector<NexusCase> nexusCases() {
  std::vector<NexusCase> v;
  // planche 12 : comparatif, paramètres réels, texte hôte (aucune donnée de démonstration)
  v.push_back({"12", {652, 112, 132, 150}, ParamId::WtX, Density::XL, "X", -1, -1, {}});
  v.push_back({"12", {802, 112, 132, 150}, ParamId::WtFine, Density::XL, "FINE", -1, -1, {}});
  v.push_back({"12", {652, 282, 84, 96}, ParamId::CrossAudioAmount, Density::S, "AMOUNT", -1, -1, {}});
  v.push_back({"12", {762, 292, 64, 74}, ParamId::DepthOp2Op1, Density::Micro, "DEPTH", -1, -1, {}});
  // planche 01 : tableau final R1 (données de démonstration ; paramètre réel de même nature, même valeur normalisée)
  v.push_back({"01", {28, 182, 132, 150}, ParamId::WtX, Density::XL, "X", 0.486f, 0.6f, {}});
  v.push_back({"01", {170, 182, 132, 150}, ParamId::WtY, Density::XL, "Y", 0.22f, -1, {}});
  v.push_back({"01", {312, 182, 132, 150}, ParamId::WtZ, Density::XL, "WT Z", 0.35f, -1, {0.15f}});
  v.push_back({"01", {359, 452, 64, 74}, ParamId::WtLevel, Density::Micro, "LVL", 0.8f, -1, {}});
  static const std::string fwd = std::string("WT") + kArrow + "FM", rev = std::string("FM") + kArrow + "WT";
  v.push_back({"01", {478, 186, 84, 96}, ParamId::CrossFwd, Density::S, fwd.c_str(), 0.72f, 0.55f, {}});
  v.push_back({"01", {570, 186, 84, 96}, ParamId::CrossRev, Density::S, rev.c_str(), 0.4f, -1, {}});
  v.push_back({"01", {478, 312, 84, 96}, ParamId::CrossAudioAmount, Density::S, "AMOUNT", 0.5f, -1, {}});
  const ParamId ratios[4] = {ParamId::Op1Ratio, ParamId::Op2Ratio, ParamId::Op3Ratio, ParamId::Op4Ratio};
  const ParamId levels[4] = {ParamId::Op1Level, ParamId::Op2Level, ParamId::Op3Level, ParamId::Op4Level};
  for (int k = 0; k < 4; ++k) {
    v.push_back({"01", {948 + 100 * k, 186, 84, 96}, ratios[k], Density::S, "RATIO", (1.0f + static_cast<float>(k) - 0.5f) / 15.5f, -1, {}});
    v.push_back({"01", {958 + 100 * k, 290, 64, 74}, levels[k], Density::Micro, "LVL", 0.9f - 0.2f * static_cast<float>(k), -1, {}});
  }
  v.push_back({"01", {948, 374, 132, 150}, ParamId::DepthOp2Op1, Density::XL, "FM INDEX", 2.4f / 8.0f, 0.42f, {0.1f, -0.2f}});
  // D2 : SPEED exclu (paramètre réel bipolaire -4..4, démonstration unipolaire : données non équivalentes)
  const ParamId d2[5] = {ParamId::DstWt1D2Repeat, ParamId::DstWt1D2Pos, ParamId::DstWt1D2Fb, ParamId::DstWt1D2Xfade, ParamId::DstWt1Mix};
  const char* d2l[5] = {"REPEAT", "POS", "FDBK", "XFADE", "MIX"};
  for (int k = 0; k < 5; ++k) v.push_back({"01", {30 + (k + 1) * 96, 660, 84, 96}, d2[k], Density::S, d2l[k], 0.2f + 0.12f * static_cast<float>(k + 1), -1, {}});
  v.push_back({"01", {270, 760, 84, 96}, ParamId::DstWt1Out, Density::S, "GAIN", 0.5f, -1, {}});
  const ParamId env[4] = {ParamId::AmpAttack, ParamId::AmpDecay, ParamId::AmpSustain, ParamId::AmpRelease};
  const char* envl[4] = {"ATK", "DEC", "SUS", "REL"};
  for (int k = 0; k < 4; ++k) v.push_back({"01", {644 + 92 * k, 840, 84, 96}, env[k], Density::S, envl[k], 0.15f + 0.2f * static_cast<float>(k), -1, {}});
  v.push_back({"01", {644, 950, 84, 96}, ParamId::Lfo1Rate, Density::S, "RATE", (4.2f - 0.01f) / 39.99f, 0.3f, {}});
  v.push_back({"01", {1564, 656, 84, 96}, ParamId::OutCeiling, Density::S, "CEILING", (24.0f - 0.3f) / 24.0f, -1, {}});
  // planche 09 : zooms 75 / 100 / 150 % (densité x1)
  v.push_back({"09", {10, 10, 132, 150}, ParamId::WtZ, Density::XL, "WT Z", 0.35f, 0.5f, {}});
  v.push_back({"09", {150, 20, 84, 96}, ParamId::CrossAudioAmount, Density::S, "AMOUNT", 0.5f, -1, {}});
  v.push_back({"09", {244, 30, 64, 74}, ParamId::DepthOp1WtX, Density::Micro, "DEPTH", 0.35f, -1, {}});
  // planche 11 : matériaux
  v.push_back({"11", {48, 366, 132, 150}, ParamId::WtX, Density::XL, "X", 0.62f, 0.42f, {}});
  // ---- PLASMA (B2-03.2) : CROSS LVL = cross.master, seul paramètre des planches dont les données sont équivalentes en
  // STANDARD (graduations 0 / .5 / 1 exactes pour la plage réelle). Exclus : OUT GAIN (graduations de démonstration
  // −48 / −24 / 0 / +6 inexactes pour la plage linéaire réelle), BALANCE vertical et tout COMPACT (gabarit bloqué en
  // production : la valeur ne tient pas, CE-B2-02), M2 et LVL (graduations de démonstration 0 / 1).
  v.push_back({"12", {652, 418, 64, 180}, ParamId::CrossMaster, Density::S, "CROSS LVL", -1, -1, {}, ui::Form::LinearV});
  v.push_back({"01", {676, 182, 64, 180}, ParamId::CrossMaster, Density::S, "CROSS LVL", 0.64f, 0.78f, {}, ui::Form::LinearV});
  v.push_back({"06a", {296, 112, 64, 180}, ParamId::CrossMaster, Density::S, "CROSS LVL", 0.64f, 0.78f, {}, ui::Form::LinearV});
  v.push_back({"09", {318, 10, 64, 180}, ParamId::CrossMaster, Density::S, "CROSS LVL", 0.64f, 0.78f, {}, ui::Form::LinearV});
  return v;
}

// ---- NEURAL CELL (B2-03.3) : NeuralCell de production (paramètre Bool ou état hors registre) ou ActionCell
struct CellCase {
  const char* sheet;
  juce::Rectangle<int> r;
  int type;              // 1 : NeuralCell liée à un paramètre ; 2 : ActionCell ; 3 : NeuralCell hors registre
  ParamId id;            // type 1
  const char* label;     // libellé de la référence
  int state;             // état de production imposé (CellState) après lecture du processeur ; -1 : lu du processeur
  int icon;              // ActionCell : 0 aucune, 1 capture, 2 autorisation
  juce::Colour accent;   // ActionCell et état hors registre
  bool on = false;       // bascule hors registre
};

std::vector<CellCase> cellCases() {
  using CS = ui::CellState;
  std::vector<CellCase> v;
  // planche 12 : comparatif (paramètres réels : Freeze ON, ARM ON et instance autorisée = ACTIF)
  v.push_back({"12", {652, 620, 168, 40}, 1, ParamId::DstWt1D2Freeze, "FREEZE", -1, 0, {}});
  v.push_back({"12", {832, 620, 168, 40}, 1, ParamId::DstWt1Arm, "ARM", -1, 0, {}});
  v.push_back({"12", {652, 700, 168, 40}, 2, ParamId::Count, "CAPTURE", -1, 1, ui::tok::text});
  v.push_back({"12", {832, 700, 140, 40}, 2, ParamId::Count, "AUTHORIZE", -1, 2, ui::tok::text});
  // planche 01 : tableau final (états de la démonstration : Freeze ON ; ARM armé, non autorisé)
  v.push_back({"01", {30, 868, 180, 42}, 1, ParamId::DstWt1D2Freeze, "FREEZE", static_cast<int>(CS::Active), 0, {}});
  v.push_back({"01", {220, 868, 180, 42}, 1, ParamId::DstWt1Arm, "ARM", static_cast<int>(CS::Armed), 0, {}});
  v.push_back({"01", {410, 868, 190, 42}, 2, ParamId::Count, "CAPTURE", -1, 1, ui::tok::text});
  v.push_back({"01", {410, 918, 190, 42}, 2, ParamId::Count, "EXCITE", -1, 0, ui::tok::text});
  v.push_back({"01", {830, 186, 88, 42}, 2, ParamId::Count, "RESET", -1, 0, ui::tok::cross});
  // planche 11 : matériaux (action, bascule du domaine WT, armement)
  v.push_back({"11", {254, 380, 180, 42}, 2, ParamId::Count, "APPLY WAVE", -1, 0, ui::tok::text});
  v.push_back({"11", {254, 436, 180, 42}, 3, ParamId::Count, "SYNC", static_cast<int>(CS::Active), 0, ui::tok::wt, true});
  v.push_back({"11", {254, 492, 180, 42}, 1, ParamId::DstWt1Arm, "ARM", static_cast<int>(CS::Armed), 0, {}});
  return v;
}

juce::RangedAudioParameter* paramOf(SubdestrucktProcessor& p, ParamId id) {
  for (auto* prm : p.getParameters())
    if (auto* r = dynamic_cast<juce::RangedAudioParameter*>(prm))
      if (r->getParameterID() == juce::String(paramSpec(id).stableId)) return r;
  return nullptr;
}

const char* densityName(Density d) { return d == Density::XL ? "XL" : (d == Density::Micro ? "Micro" : "S"); }

struct Substituted {
  NexusCase k;
  ui::ContinuousControl* c = nullptr;
  juce::String refText;  // texte de valeur affiché par la référence (composant du laboratoire)
  float refNorm = 0.0f, prodNorm = 0.0f;  // valeurs normalisées affichées (référence, production)
  int ulps = 0;                           // ajustement de la valeur hôte, en ulps, pour la même norme affichée
  juce::String error;
};

// Remplace, dans `sheet`, les NEXUS du laboratoire des cas `cases` par les composants de production. Les composants
// créés sont confiés à `owned` (détruits avant la planche par l'appelant).
std::vector<Substituted> substitute(juce::Component& sheet, ui::UiContext& ctx, const std::vector<NexusCase>& cases,
                                    std::vector<std::unique_ptr<juce::Component>>& owned) {
  std::vector<Substituted> out;
  auto& proc = ctx.proc;
  const auto keep = juce::SystemStats::getEnvironmentVariable("SBD_VIS01_KEEP", {});  // diagnostic : composant du laboratoire conservé
  for (const auto& k : cases) {
    if (keep.isNotEmpty() && keep == juce::String(k.r.getX()) + ":" + juce::String(k.r.getY())) continue;
    Substituted s{k};
    juce::Component* lab = nullptr;
    for (auto* ch : sheet.getChildren())
      if (ch->getBounds() == k.r) {
        if (auto* n = dynamic_cast<r2::Nexus*>(ch); n != nullptr && k.form == ui::Form::Rotary) {
          lab = n;
          s.refText = n->shownText();
          s.refNorm = n->shownNorm();
        }
        if (auto* pl = dynamic_cast<r2::PlasmaChannel*>(ch); pl != nullptr && k.form != ui::Form::Rotary) {
          lab = pl;
          s.refText = pl->shownText();
          s.refNorm = pl->shownNorm();
        }
      }
    auto* prm = paramOf(proc, k.id);
    if (lab == nullptr || prm == nullptr) {
      s.error = lab == nullptr ? "composant du laboratoire introuvable aux bornes indiquees" : "parametre introuvable";
      out.push_back(s);
      continue;
    }
    lab->setVisible(false);
    if (k.norm >= 0.0f) prm->setValueNotifyingHost(k.norm);
    const auto i = static_cast<size_t>(idx(k.id));
    if (k.eff >= 0.0f) {  // télémétrie publiée (moteur arrêté) : état représentatif
      proc.telemetry().effective[i].store(prm->convertFrom0to1(k.eff));
      proc.telemetry().modulated[i].store(1);
    }
    const auto& spec = paramSpec(k.id);
    for (size_t ri = 0; ri < k.routes.size(); ++ri) {  // transaction de production : décalage normalisé -> profondeur (x modScale)
      const float rn = k.routes[ri];
      const float base = prm->convertFrom0to1(prm->getValue());
      const float off = prm->convertFrom0to1(juce::jlimit(0.0f, 1.0f, prm->getValue() + rn)) - base;
      ModRoute r;
      r.source = static_cast<ModSource>(static_cast<int>(ModSource::Lfo1) + static_cast<int>(ri));  // une source par route
      r.dest = k.id;
      r.depth = off / spec.modScale;
      const auto res = proc.modAdd(r);
      if (!res.ok) s.error = "route refusee : " + juce::String(res.error);
    }
    auto c = std::make_unique<ui::ContinuousControl>(ctx, k.id, k.form, k.d);
    c->setCaption(juce::String::fromUTF8(k.label));
    c->setEnabled(!k.disabled);
    // même rang de dessin que le composant remplacé : l'ordre des tracés est celui de la référence (sous Windows,
    // Direct2D met en cache la géométrie des tracés répétés, ce qui modifie l'anticrénelage des occurrences suivantes)
    sheet.addAndMakeVisible(*c, sheet.getIndexOfChildComponent(lab));
    // même donnée : la norme affichée par la production (aller-retour par le paramètre hôte) doit être exactement celle
    // de la référence ; sinon la valeur hôte est ajustée (au plus 4 096 ulps), et l'ajustement est publié. Si l'égalité
    // reste impossible (correspondance non injective), le cas est publié comme DONNÉE NON ÉQUIVALENTE (hors comparaison).
    c->tick();
    if (c->shownNorm() != s.refNorm) {
      float up = s.refNorm, down = s.refNorm;
      for (int u = 1; u <= 4096 && c->shownNorm() != s.refNorm; ++u) {
        up = std::nextafter(up, 2.0f);
        prm->setValueNotifyingHost(up);
        c->tick();
        if (c->shownNorm() == s.refNorm) {
          s.ulps = u;
          break;
        }
        down = std::nextafter(down, -1.0f);
        prm->setValueNotifyingHost(down);
        c->tick();
        if (c->shownNorm() == s.refNorm) {
          s.ulps = -u;
          break;
        }
      }
      if (c->shownNorm() != s.refNorm) prm->setValueNotifyingHost(s.refNorm);
    }
    s.prodNorm = c->shownNorm();
    c->setBounds(k.r);
    if (k.form != ui::Form::Rotary && !c->plasmaR1()) s.error = "PLASMA de production bloque a ces bornes : " + c->plasmaBlockReason();
    s.c = c.get();
    owned.push_back(std::move(c));
    out.push_back(s);
  }
  ctx.mod = proc.currentMod();  // copie publiée, comme l'éditeur à 10 Hz
  ++ctx.modRevision;
  for (auto& s : out)
    if (s.c != nullptr) s.c->tick();
  return out;
}

juce::String csvField(const juce::String& s) { return "\"" + s.replace("\"", "\"\"") + "\""; }

// ---- SYNAPTIC TRACK (B2-03.4) : SynapticTrack de production (paramètre Choice, mêmes options et même mode que la
// référence ; libellés d'options posés par l'alias d'affichage) ou TabStrip (onglets de navigation)
struct TrackCase {
  const char* sheet;
  juce::Rectangle<int> r;
  int type;  // 1 : SynapticTrack ; 2 : TabStrip
  ParamId id;
  const char* label;
  std::vector<const char*> options;
  int index;
  juce::Colour accent;  // onglets
};

std::vector<TrackCase> trackCases() {
  std::vector<TrackCase> v;
  const std::vector<const char*> fb{"LEGACY", "REGEN"};
  const std::vector<const char*> types{"OFF", "N1 FOLD", "D1 CRUSH", "Y1 TRANSIENT", "S1 RING/SHIFT", "N2 FB SHAPER", "D2 GLITCH", "Y2 ENV SHAPER", "S2 SPECTRAL"};
  // planche 12 : FB MODE = valeur réelle (défaut 1, REGEN) ; TYPE = valeur réelle (0)
  v.push_back({"12", {652, 790, 168, 46}, 1, ParamId::DstWt1D2FbMode, "FB MODE", fb, 1, {}});
  v.push_back({"12", {832, 790, 200, 46}, 1, ParamId::DstWt1Type, "TYPE", types, 0, {}});
  // planche 01 : TYPE (index 6), FB MODE (index 0), onglets des enveloppes
  v.push_back({"01", {30, 600, 200, 46}, 1, ParamId::DstWt1Type, "TYPE", types, 6, {}});
  v.push_back({"01", {30, 770, 220, 46}, 1, ParamId::DstWt1D2FbMode, "FB MODE", fb, 0, {}});
  v.push_back({"01", {644, 600, 400, 32}, 2, ParamId::Count, "", {"AMP", "WT", "FM", "AUX1", "AUX2"}, 0, ui::tok::mod});
  // planche 11 : DIR (CROSS, index 1)
  v.push_back({"11", {468, 380, 190, 46}, 1, ParamId::CrossDir, "DIR", {"FWD", "REV", "BOTH"}, 1, {}});
  // planche 09 : FB MODE aux trois zooms (index 0)
  v.push_back({"09", {10, 196, 300, 46}, 1, ParamId::DstWt1D2FbMode, "FB MODE", fb, 0, {}});
  return v;
}

struct CellSub {
  CellCase k;
  juce::Component* c = nullptr;
  juce::String refText, prodText, error;
  juce::Rectangle<float> textZone;  // zone du texte d'état (donnée) : à droite de la face
};

struct TrackSub {
  TrackCase k;
  juce::Component* c = nullptr;
  juce::String error;
};

std::vector<TrackSub> substituteTracks(juce::Component& sheet, ui::UiContext& ctx, const std::vector<TrackCase>& cases,
                                       std::vector<std::unique_ptr<juce::Component>>& owned) {
  std::vector<TrackSub> out;
  const auto keep = juce::SystemStats::getEnvironmentVariable("SBD_VIS01_KEEP", {});
  for (const auto& k : cases) {
    if (keep.isNotEmpty() && keep == juce::String(k.r.getX()) + ":" + juce::String(k.r.getY())) continue;
    TrackSub s{k};
    juce::Component* lab = nullptr;
    for (auto* ch : sheet.getChildren())
      if (ch->getBounds() == k.r && dynamic_cast<r2::SynapticTrack*>(ch) != nullptr) lab = ch;
    if (lab == nullptr) {
      s.error = "SYNAPTIC TRACK du laboratoire introuvable";
      out.push_back(s);
      continue;
    }
    lab->setVisible(false);
    juce::StringArray opts;
    for (auto* o : k.options) opts.add(juce::String::fromUTF8(o));
    std::unique_ptr<juce::Component> comp;
    if (k.type == 1) {
      if (auto* prm = dynamic_cast<juce::RangedAudioParameter*>(ctx.proc.getParameters()[idx(k.id)]))
        prm->setValueNotifyingHost(prm->convertTo0to1(static_cast<float>(k.index)));
      auto t = std::make_unique<ui::SynapticTrack>(ctx, k.id);
      t->setOptionLabels(opts);
      t->setCaption(juce::String::fromUTF8(k.label));
      t->setBounds(k.r);
      if (!t->r1()) s.error = "SynapticTrack de production bloquee : " + t->r1BlockReason();
      comp = std::move(t);
    } else {
      auto t = std::make_unique<ui::TabStrip>(k.accent);
      t->setTabs(opts);
      t->setCurrent(k.index);
      t->setBounds(k.r);
      if (!t->r1()) s.error = "TabStrip de production bloquee";
      comp = std::move(t);
    }
    sheet.addAndMakeVisible(*comp, sheet.getIndexOfChildComponent(lab));
    s.c = comp.get();
    owned.push_back(std::move(comp));
    out.push_back(s);
  }
  return out;
}

std::vector<CellSub> substituteCells(juce::Component& sheet, ui::UiContext& ctx, const std::vector<CellCase>& cases,
                                     std::vector<std::unique_ptr<juce::Component>>& owned) {
  std::vector<CellSub> out;
  const auto keep = juce::SystemStats::getEnvironmentVariable("SBD_VIS01_KEEP", {});  // diagnostic : composant du laboratoire conservé
  for (const auto& k : cases) {
    if (keep.isNotEmpty() && keep == juce::String(k.r.getX()) + ":" + juce::String(k.r.getY())) continue;
    CellSub s{k};
    r2::NeuralCell* lab = nullptr;
    for (auto* ch : sheet.getChildren())
      if (ch->getBounds() == k.r)
        if (auto* n = dynamic_cast<r2::NeuralCell*>(ch)) lab = n;
    if (lab == nullptr) {
      s.error = "NEURAL CELL du laboratoire introuvable";
      out.push_back(s);
      continue;
    }
    s.refText = lab->stateText();
    lab->setVisible(false);
    std::unique_ptr<juce::Component> comp;
    if (k.type == 1 || k.type == 3) {
      auto n = k.type == 1 ? std::make_unique<ui::NeuralCell>(ctx, k.id) : std::make_unique<ui::NeuralCell>(ctx, juce::String::fromUTF8(k.label), k.accent);
      n->setCaption(juce::String::fromUTF8(k.label));
      if (k.type == 3) n->setToggleState(k.on, juce::dontSendNotification);
      n->tick();
      if (k.state >= 0) n->setCellState(static_cast<ui::CellState>(k.state));  // état représentatif de la référence
      n->setBounds(k.r);
      if (!n->r1()) s.error = "NeuralCell de production bloquee : " + n->r1BlockReason();
      s.prodText = n->stateText();
      comp = std::move(n);
    } else {
      auto a = std::make_unique<ui::ActionCell>(juce::String::fromUTF8(k.label), k.accent);
      if (k.icon != 0) a->setIcon(k.icon == 1 ? ui::ActionCell::Icon::Capture : ui::ActionCell::Icon::Authorize);  // sinon : impulsion (défaut)
      a->setBounds(k.r);
      if (!a->r1()) s.error = "ActionCell de production bloquee : " + a->r1BlockReason();
      comp = std::move(a);
    }
    // zone du texte d'état du peintre : r = bornes réduites (3, 4), in = r réduite (10, 0), texte aligné à droite
    const float w = static_cast<float>(k.r.getWidth()), h = static_cast<float>(k.r.getHeight());
    if (k.type != 2) s.textZone = {w - 13.0f - 50.0f, 4.0f, 50.0f, h - 8.0f};
    sheet.addAndMakeVisible(*comp, sheet.getIndexOfChildComponent(lab));
    s.c = comp.get();
    owned.push_back(std::move(comp));
    out.push_back(s);
  }
  return out;
}

}  // namespace

int runVis01(const juce::File& outDir, const juce::String& onlySheet, const juce::File& b1Dir) {
  outDir.createDirectory();
  const auto wanted = [&](const char* id) { return onlySheet.isEmpty() || onlySheet == id; };
  const auto cases = nexusCases();
  juce::StringArray rows;
  rows.add("famille,feuille,planche,image,origine_x,origine_y,echelle,cas,parametre,gabarit,x,y,w,h,valeur_x,valeur_y,valeur_w,valeur_h,"
           "texte_reference,texte_production,norme_reference,norme_production,ajustement_ulps,erreur");
  bool ok = true;
  const auto emitCells = [&](const juce::String& sheetId, const juce::String& png, int ox, int oy, float scale, const std::vector<CellSub>& subs) {
    for (const auto& s : subs) {
      rows.add(juce::String("NEURAL CELL,") + s.k.sheet + "," + sheetId + "," + png + "," + juce::String(ox) + "," + juce::String(oy) + "," + juce::String(scale, 2) +
               "," + csvField(juce::String::fromUTF8(s.k.label)) + "," +
               (s.k.type == 1 ? juce::String(paramSpec(s.k.id).stableId) : juce::String(s.k.type == 2 ? "action" : "etat-hors-registre")) + "," +
               (s.k.type == 2 ? "ActionCell" : "NeuralCell") + "," + juce::String(s.k.r.getX()) + "," + juce::String(s.k.r.getY()) + "," +
               juce::String(s.k.r.getWidth()) + "," + juce::String(s.k.r.getHeight()) + "," + juce::String(s.textZone.getX(), 2) + "," +
               juce::String(s.textZone.getY(), 2) + "," + juce::String(s.textZone.getWidth(), 2) + "," + juce::String(s.textZone.getHeight(), 2) + "," +
               csvField(s.refText) + "," + csvField(s.prodText) + ",,,0," + csvField(s.error));
      ok = ok && s.error.isEmpty();
    }
  };
  const auto emitTracks = [&](const juce::String& sheetId, const juce::String& png, int ox, int oy, float scale, const std::vector<TrackSub>& subs) {
    for (const auto& s : subs) {
      rows.add(juce::String("SYNAPTIC,") + s.k.sheet + "," + sheetId + "," + png + "," + juce::String(ox) + "," + juce::String(oy) + "," + juce::String(scale, 2) +
               "," + csvField(juce::String::fromUTF8(s.k.label)) + "," + (s.k.type == 1 ? juce::String(paramSpec(s.k.id).stableId) : juce::String("onglets")) + "," +
               (s.k.type == 1 ? "SynapticTrack" : "TabStrip") + "," + juce::String(s.k.r.getX()) + "," + juce::String(s.k.r.getY()) + "," +
               juce::String(s.k.r.getWidth()) + "," + juce::String(s.k.r.getHeight()) + ",0,0,0,0,,,,,0," + csvField(s.error));
      ok = ok && s.error.isEmpty();
    }
  };
  const auto emit = [&](const juce::String& sheetId, const juce::String& png, int ox, int oy, float scale, const std::vector<Substituted>& subs) {
    for (const auto& s : subs) {
      const auto va = s.c != nullptr ? s.c->valueBox() : juce::Rectangle<float>();
      const juce::String prodText = s.c != nullptr ? s.c->valueText() : juce::String();
      rows.add(juce::String(s.k.form == ui::Form::Rotary ? "NEXUS," : "PLASMA,") + s.k.sheet + "," + sheetId + "," + png + "," + juce::String(ox) + "," + juce::String(oy) + "," + juce::String(scale, 2) + "," +
               csvField(juce::String::fromUTF8(s.k.label)) + "," + paramSpec(s.k.id).stableId + "," + densityName(s.k.d) + "," + juce::String(s.k.r.getX()) +
               "," + juce::String(s.k.r.getY()) + "," + juce::String(s.k.r.getWidth()) + "," + juce::String(s.k.r.getHeight()) + "," +
               juce::String(va.getX(), 2) + "," + juce::String(va.getY(), 2) + "," + juce::String(va.getWidth(), 2) + "," + juce::String(va.getHeight(), 2) + "," +
               csvField(s.refText) + "," + csvField(prodText) + "," + juce::String(s.refNorm, 9) + "," + juce::String(s.prodNorm, 9) + "," +
               juce::String(s.ulps) + "," + csvField(s.error));
      ok = ok && s.error.isEmpty();
    }
  };
  const auto save = [&](const juce::Image& img, const juce::String& name) {
    const bool w = writePng(img, outDir.getChildFile(name));
    std::cout << "vis01 " << name << " " << img.getWidth() << "x" << img.getHeight() << (w ? "" : " WRITE FAILED") << std::endl;
    ok = ok && w;
  };
  const auto cells = cellCases();
  const auto tracks = trackCases();
  const auto onlyTracks = [&](const char* sheet) {
    std::vector<TrackCase> v;
    for (const auto& k : tracks)
      if (juce::String(k.sheet) == sheet) v.push_back(k);
    return v;
  };
  const auto onlyCells = [&](const char* sheet) {
    std::vector<CellCase> v;
    for (const auto& k : cells)
      if (juce::String(k.sheet) == sheet) v.push_back(k);
    return v;
  };
  const auto only = [&](const char* sheet) {
    std::vector<NexusCase> v;
    for (const auto& k : cases)
      if (juce::String(k.sheet) == sheet) v.push_back(k);
    return v;
  };
  if (wanted("12")) {  // planche 12 : scène du comparatif (processeur de la scène, contexte de production : aucun CC MIDI Learn)
    CompareScene scene;
    ui::UiContext ctx{scene.proc, nullptr, RandomOptions{}, {}, {}, {}, {}, ParamId::WtX, {}};
    ctx.learnCc.fill(-1);
    std::vector<std::unique_ptr<juce::Component>> owned;
    const auto subs = substitute(*scene.sheet, ctx, only("12"), owned);
    const auto csubs = substituteCells(*scene.sheet, ctx, onlyCells("12"), owned);
    const auto tsubs = substituteTracks(*scene.sheet, ctx, onlyTracks("12"), owned);
    save(scene.sheet->createComponentSnapshot(scene.sheet->getLocalBounds(), true, 2.0f), "vis01-12.png");
    emit("12-compare-041-r2.png", "vis01-12.png", 0, 0, 2.0f, subs);
    emitCells("12-compare-041-r2.png", "vis01-12.png", 0, 0, 2.0f, csubs);
    emitTracks("12-compare-041-r2.png", "vis01-12.png", 0, 0, 2.0f, tsubs);
    owned.clear();
  }
  for (const char* id : {"01", "06a", "11"}) {
    if (!wanted(id)) continue;
    SubdestrucktProcessor proc;
    ui::UiContext ctx{proc, nullptr, RandomOptions{}, {}, {}, {}, {}, ParamId::WtX, {}};
    ctx.learnCc.fill(-1);
    const juce::String refName = juce::String(id) == "01" ? "01-final-board.png" : (juce::String(id) == "06a" ? "06a-plasma-standard-compact.png" : "11-materials-palette.png");
    if (juce::String(id) != "01") {
      // la référence a été rendue après les planches précédentes du même processus : même historique du moteur graphique
      juce::StringArray lg;
      const auto tmp = juce::File::createTempFile("vis01-historique");
      tmp.createDirectory();
      renderSheets(tmp, 2.0f, lg, b1Dir, refName);
      tmp.deleteRecursively();
    }
    auto sheet = makeR1Sheet(id);
    std::vector<std::unique_ptr<juce::Component>> owned;
    const auto subs = substitute(*sheet, ctx, only(id), owned);
    const auto csubs = substituteCells(*sheet, ctx, onlyCells(id), owned);
    const auto tsubs = substituteTracks(*sheet, ctx, onlyTracks(id), owned);
    const juce::String png = juce::String("vis01-") + id + ".png";
    save(sheet->createComponentSnapshot(sheet->getLocalBounds(), true, 2.0f), png);
    emit(refName, png, 0, 0, 2.0f, subs);
    emitCells(refName, png, 0, 0, 2.0f, csubs);
    emitTracks(refName, png, 0, 0, 2.0f, tsubs);
    owned.clear();
  }
  if (wanted("09")) {  // planche 09 : chaque zoom est une image à part, incrustée dans la référence à x = 40 + ..., y = 126 (composeZoom)
    const float zs[3] = {0.75f, 1.0f, 1.5f};
    int x = 40;
    for (float z : zs) {
      SubdestrucktProcessor proc;
      ui::UiContext ctx{proc, nullptr, RandomOptions{}, {}, {}, {}, {}, ParamId::WtX, {}};
      ctx.learnCc.fill(-1);
      auto sheet = makeR1Sheet("09");
      std::vector<std::unique_ptr<juce::Component>> owned;
      const auto subs = substitute(*sheet, ctx, only("09"), owned);
      const auto tsubs = substituteTracks(*sheet, ctx, onlyTracks("09"), owned);
      const juce::String png = "vis01-09-" + juce::String(juce::roundToInt(z * 100.0f)) + ".png";
      const auto img = sheet->createComponentSnapshot(sheet->getLocalBounds(), true, z);
      save(img, png);
      emit("09-zoom-75-100-150.png", png, x, 126, z, subs);
      emitTracks("09-zoom-75-100-150.png", png, x, 126, z, tsubs);
      x += img.getWidth() + 40;
      owned.clear();
    }
  }
  // manifeste complété planche par planche (une planche par processus : rendu indépendant de l'historique du moteur graphique)
  const auto manifest = outDir.getChildFile("vis01-cases.csv");
  const int n = rows.size() - 1;
  if (manifest.existsAsFile()) rows.remove(0);
  ok = manifest.appendText(rows.joinIntoString("\n") + "\n", false, false, "\n") && ok;
  std::cout << "vis01 " << n << " cas, " << (ok ? "OK" : "ECHEC") << std::endl;
  return ok ? 0 : 1;
}

}  // namespace sbd::lab
