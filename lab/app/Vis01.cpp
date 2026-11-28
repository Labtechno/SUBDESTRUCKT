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
  const char* sheet;           // "01", "09", "11", "12"
  juce::Rectangle<int> r;      // bornes logiques dans la planche
  ParamId id;
  Density d;
  const char* label;           // libellé de la référence (UTF-8)
  float norm;                  // valeur normalisée de la référence ; < 0 : valeur déjà fixée par la scène (planche 12)
  float eff;                   // Effective normalisée de la référence ; < 0 : aucune
  std::vector<float> routes;   // décalages normalisés des routes de la référence
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
    if (keep.isNotEmpty() && keep == paramSpec(k.id).stableId) continue;
    Substituted s{k};
    r2::Nexus* lab = nullptr;
    for (auto* ch : sheet.getChildren())
      if (ch->getBounds() == k.r)
        if (auto* n = dynamic_cast<r2::Nexus*>(ch)) lab = n;
    auto* prm = paramOf(proc, k.id);
    if (lab == nullptr || prm == nullptr) {
      s.error = lab == nullptr ? "NEXUS du laboratoire introuvable aux bornes indiquees" : "parametre introuvable";
      out.push_back(s);
      continue;
    }
    s.refText = lab->shownText();
    s.refNorm = lab->shownNorm();
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
    auto c = std::make_unique<ui::ContinuousControl>(ctx, k.id, ui::Form::Rotary, k.d);
    c->setCaption(juce::String::fromUTF8(k.label));
    // même rang de dessin que le composant remplacé : l'ordre des tracés est celui de la référence (sous Windows,
    // Direct2D met en cache la géométrie des tracés répétés, ce qui modifie l'anticrénelage des occurrences suivantes)
    sheet.addAndMakeVisible(*c, sheet.getIndexOfChildComponent(lab));
    // même donnée : la norme affichée par la production (aller-retour par le paramètre hôte) doit être exactement celle
    // de la référence ; sinon la valeur hôte est ajustée de quelques ulps (au plus 16), et l'ajustement est publié
    c->tick();
    if (c->shownNorm() != s.refNorm) {
      for (int u = 1; u <= 16 && c->shownNorm() != s.refNorm; ++u)
        for (float dir : {1.0f, -1.0f}) {
          float n = s.refNorm;
          for (int q = 0; q < u; ++q) n = std::nextafter(n, dir > 0.0f ? 2.0f : -1.0f);
          prm->setValueNotifyingHost(n);
          c->tick();
          if (c->shownNorm() == s.refNorm) {
            s.ulps = static_cast<int>(dir) * u;
            break;
          }
        }
    }
    s.prodNorm = c->shownNorm();
    c->setBounds(k.r);
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

}  // namespace

int runVis01(const juce::File& outDir, const juce::String& onlySheet, const juce::File& b1Dir) {
  outDir.createDirectory();
  const auto wanted = [&](const char* id) { return onlySheet.isEmpty() || onlySheet == id; };
  const auto cases = nexusCases();
  juce::StringArray rows;
  rows.add("famille,planche,image,origine_x,origine_y,echelle,cas,parametre,gabarit,x,y,w,h,valeur_x,valeur_y,valeur_w,valeur_h,"
           "texte_reference,texte_production,norme_reference,norme_production,ajustement_ulps,erreur");
  bool ok = true;
  const auto emit = [&](const juce::String& sheetId, const juce::String& png, int ox, int oy, float scale, const std::vector<Substituted>& subs) {
    for (const auto& s : subs) {
      const auto va = s.c != nullptr ? s.c->valueBox() : juce::Rectangle<float>();
      const juce::String prodText = s.c != nullptr ? s.c->valueText() : juce::String();
      rows.add("NEXUS," + sheetId + "," + png + "," + juce::String(ox) + "," + juce::String(oy) + "," + juce::String(scale, 2) + "," +
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
    save(scene.sheet->createComponentSnapshot(scene.sheet->getLocalBounds(), true, 2.0f), "vis01-12.png");
    emit("12-compare-041-r2.png", "vis01-12.png", 0, 0, 2.0f, subs);
    owned.clear();
  }
  for (const char* id : {"01", "11"}) {
    if (!wanted(id)) continue;
    SubdestrucktProcessor proc;
    ui::UiContext ctx{proc, nullptr, RandomOptions{}, {}, {}, {}, {}, ParamId::WtX, {}};
    ctx.learnCc.fill(-1);
    if (juce::String(id) == "11") {
      // la référence 11 a été rendue après les planches 01 à 10 du même processus : même historique du moteur graphique
      juce::StringArray lg;
      const auto tmp = juce::File::createTempFile("vis01-historique");
      tmp.createDirectory();
      renderSheets(tmp, 2.0f, lg, b1Dir);
      tmp.deleteRecursively();
    }
    auto sheet = makeR1Sheet(id);
    std::vector<std::unique_ptr<juce::Component>> owned;
    const auto subs = substitute(*sheet, ctx, only(id), owned);
    const juce::String png = juce::String("vis01-") + id + ".png";
    save(sheet->createComponentSnapshot(sheet->getLocalBounds(), true, 2.0f), png);
    emit(juce::String(id) == "01" ? "01-final-board.png" : "11-materials-palette.png", png, 0, 0, 2.0f, subs);
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
      const juce::String png = "vis01-09-" + juce::String(juce::roundToInt(z * 100.0f)) + ".png";
      const auto img = sheet->createComponentSnapshot(sheet->getLocalBounds(), true, z);
      save(img, png);
      emit("09-zoom-75-100-150.png", png, x, 126, z, subs);
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
