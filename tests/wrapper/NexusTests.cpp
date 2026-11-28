// SUBDESTRUCKT — ARCH-UI-04-R2-B2-03 (1) : tests d'intégration des NEXUS R1 de production, exécutés par
// subdestruckt_wrapper_tests. Critères enregistrés avant mesure (B2-ACCEPTANCE-CRITERIA.md, addendum B2-03 ; addendum
// normatif B2-03 §3 CE-B2-08) :
//  NX-CC     CE-B2-08 / MIDI-CC : l'indicateur MIDI Learn (donnée 0.4.1 : CC appris) est dessiné en XL et en S, masqué
//            en Micro comme en 0.4.1, aux échelles 0,75 / 1 / 1,5 / 2, pour un paramètre unipolaire et un bipolaire, Base,
//            Effective et cinq routes affichées (quatre marques et « +1 »), états actif et INACTIVE, libellés courts et longs.
//            Placé dans la sérigraphie à gauche du libellé quand la place est libre, sinon absent (infobulle seulement,
//            arbitrage DB2-10). Il ne modifie AUCUN pixel hors de sa zone et ne recouvre AUCUN pixel déjà dessiné (valeur,
//            marques, Effective, libellé, états) ; l'infobulle porte le CC dans tous les cas. Contraste sur plaque >= 4,5:1.
//  NX-ROUTES les marques de route suivent la configuration publiée par l'éditeur (UiContext::mod, révision) : ajout,
//            inversion et retrait reproduisent exactement le rendu attendu ; aucune lecture du fil audio.
//  NX-GEOM   géométrie partagée : fenêtre de valeur et zone du bouton du composant = géométrie du peintre R1 pour les
//            trois gabarits ; la saisie numérique (double-clic sur la valeur, dispatch réel) s'ouvre exactement sur la
//            fenêtre de valeur, sans écriture préalable ; un double-clic sur le bouton revient au défaut (contrat NEXUS
//            inchangé, DB2-09 ne concerne que les PLASMA).
#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>

#include "TestKit.h"
#include "core/Parameters.h"
#include "support/HeadlessPeer.h"
#include "ui/Components.h"
#include "ui/r2/R2Painters.h"

using namespace sbd;

namespace {

juce::Image renderComp(juce::Component& c, float scale, juce::Colour bg) {
  juce::Image img(juce::Image::ARGB, juce::roundToInt(static_cast<float>(c.getWidth()) * scale), juce::roundToInt(static_cast<float>(c.getHeight()) * scale), true,
                  juce::SoftwareImageType());
  {
    juce::Graphics g(img);
    g.fillAll(bg);
    g.addTransform(juce::AffineTransform::scale(scale));
    c.paintEntireComponent(g, false);
  }
  return img;
}

bool samePixel(const juce::Image& a, const juce::Image& b, int x, int y) { return a.getPixelAt(x, y) == b.getPixelAt(x, y); }

double luminance(juce::Colour c) {
  auto lin = [](float v) { return v <= 0.03928f ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
  return 0.2126 * lin(c.getFloatRed()) + 0.7152 * lin(c.getFloatGreen()) + 0.0722 * lin(c.getFloatBlue());
}
double contrast(juce::Colour a, juce::Colour b) {
  const double la = luminance(a), lb = luminance(b);
  return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

juce::RangedAudioParameter* paramOf(SubdestrucktProcessor& p, ParamId id) {
  for (auto* prm : p.getParameters())
    if (auto* r = dynamic_cast<juce::RangedAudioParameter*>(prm))
      if (r->getParameterID() == juce::String(paramSpec(id).stableId)) return r;
  return nullptr;
}

struct Fixture {
  SubdestrucktProcessor proc;
  ui::UiContext ctx{proc, nullptr, RandomOptions{}, {}, {}, {}, {}, ParamId::WtX, {}};
  Fixture() { ctx.learnCc.fill(-1); }
  void routes(ParamId id, std::vector<float> depths, bool invertFirst = false) {
    ctx.mod.routes.clear();
    std::uint32_t rid = 1;
    for (float d : depths) {
      ModRoute r;
      r.id = rid++;
      r.source = ModSource::Lfo1;
      r.dest = id;
      r.depth = d;
      r.invert = invertFirst && rid == 2;
      ctx.mod.routes.push_back(r);
    }
    ++ctx.modRevision;
  }
};

// hôte sans écran avec pair de test : événements souris injectés par le dispatch JUCE réel (comme GestureTests)
struct MiniHost {
  sbd::testsupport::TestHost host;
  juce::ComponentPeer* peer = nullptr;
  juce::int64 t = juce::Time::currentTimeMillis();
  explicit MiniHost(juce::Component& c) {
    host.setBounds(40, 40, 400, 300);
    c.setTopLeftPosition(20, 20);
    host.addAndMakeVisible(c);
    host.setVisible(true);
    host.addToDesktop(0);
    peer = host.getPeer();
  }
  ~MiniHost() {
    host.removeAllChildren();
    host.removeFromDesktop();
  }
  void at(juce::Component& c, juce::Point<float> l, juce::ModifierKeys m) {
    t += 40;
    juce::ModifierKeys::currentModifiers = m.withoutMouseButtons();
    peer->handleMouseEvent(juce::MouseInputSource::InputSourceType::mouse, peer->globalToLocal(c.localPointToGlobal(l)), m,
                           juce::MouseInputSource::defaultPressure, juce::MouseInputSource::defaultOrientation, t);
  }
  void doubleClick(juce::Component& c, juce::Point<float> l) {
    at(c, l, {});
    for (int k = 0; k < 2; ++k) {
      at(c, l, juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier));
      at(c, l, {});
    }
    t += 800;  // au-delà du délai de double-clic
  }
};

juce::TextEditor* entryOf(juce::Component& c) {
  for (auto* ch : c.getChildren())
    if (auto* e = dynamic_cast<juce::TextEditor*>(ch)) return e;
  return nullptr;
}

const ui::Density kDens[3] = {ui::Density::XL, ui::Density::S, ui::Density::Micro};
const juce::Rectangle<int> kBounds[3] = {{0, 0, 132, 150}, {0, 0, 84, 96}, {0, 0, 64, 74}};
const char* kDensName[3] = {"XL", "S", "Micro"};

sbd::r2::NexusSize r2Size(int k) { return k == 0 ? sbd::r2::NexusSize::XL : (k == 1 ? sbd::r2::NexusSize::S : sbd::r2::NexusSize::Micro); }

void testNexusCc() {
  Fixture f;
  const juce::Colour bg = ui::tok::panel;
  int configs = 0, drawn = 0, notDrawn = 0, outside = 0, masked = 0, hiddenChanged = 0, tooltipMissing = 0, microDrawn = 0;
  // libellés : courts (place libre) et longs (pas de place : infobulle seulement)
  const struct {
    ParamId id;
    const char* caption;
  } cases[4] = {{ParamId::WtX, "X"}, {ParamId::DepthOp1WtX, "DEPTH"}, {ParamId::CrossAudioAmount, "AMOUNT"}, {ParamId::DepthOp2Op1, "FM INDEX OP2"}};
  for (const auto& cs : cases) {
    const ParamId id = cs.id;
    auto* prm = paramOf(f.proc, id);
    prm->setValueNotifyingHost(0.62f);
    const auto i = static_cast<size_t>(idx(id));
    f.proc.telemetry().effective[i].store(prm->convertFrom0to1(0.8f));
    f.proc.telemetry().modulated[i].store(1);
    f.routes(id, {0.1f, -0.15f, 0.2f, 0.05f, -0.05f});  // cinq routes : quatre marques et « +1 »
    for (int k = 0; k < 3; ++k)
      for (bool inactive : {false, true}) {
        ui::ContinuousControl c(f.ctx, id, ui::Form::Rotary, kDens[k]);
        c.setBounds(kBounds[k]);
        c.setCaption(cs.caption);
        c.setInactive(inactive, "test");
        f.ctx.learnCc[i] = 127;
        c.tick();
        const auto tag = c.ccTagBox();
        tooltipMissing += c.getTooltip().contains("MIDI Learn : CC127") ? 0 : 1;
        f.ctx.learnCc[i] = -1;
        c.tick();
        tooltipMissing += c.getTooltip().contains("MIDI Learn") ? 1 : 0;  // retiré quand le CC est oublié
        for (float s : {0.75f, 1.0f, 1.5f, 2.0f}) {
          ++configs;
          f.ctx.learnCc[i] = -1;
          const auto a = renderComp(c, s, bg);
          f.ctx.learnCc[i] = 127;
          const auto b = renderComp(c, s, bg);
          f.ctx.learnCc[i] = -1;
          if (s == 2.0f) {  // planches de preuve (SBD_B2_OUT)
            const auto dir = juce::SystemStats::getEnvironmentVariable("SBD_B2_OUT", {});
            if (dir.isNotEmpty()) {
              juce::PNGImageFormat png;
              juce::FileOutputStream os(juce::File(dir).getChildFile(juce::String("nx-cc-") + kDensName[k] + "-" + juce::String(cs.caption).replace(" ", "_") +
                                                                     (inactive ? "-inactive" : "") + ".png"));
              if (os.openedOk()) png.writeImageToStream(b, os);
            }
          }
          int diff = 0, out = 0, msk = 0;
          const auto tp = (tag * s).getSmallestIntegerContainer().expanded(1);
          for (int y = 0; y < a.getHeight(); ++y)
            for (int x = 0; x < a.getWidth(); ++x) {
              const bool d = !samePixel(a, b, x, y);
              diff += d ? 1 : 0;
              if (d && (tag.isEmpty() || !tp.contains(x, y))) ++out;
              if (d && a.getPixelAt(x, y) != bg) ++msk;  // l'indicateur a modifié un pixel déjà dessiné (recouvrement)
            }
          if (kDens[k] == ui::Density::Micro) {
            microDrawn += diff > 0 ? 1 : 0;
          } else if (tag.isEmpty()) {
            ++notDrawn;
            hiddenChanged += diff > 0 ? 1 : 0;
          } else {
            drawn += diff > 0 ? 1 : 0;
            outside += out;
            masked += msk;
          }
        }
      }
  }
  // couverture (information pour DB2-10) : part des paramètres continus dont le libellé court laisse la place en XL et en S
  int nParams = 0, fitXL = 0, fitS = 0;
  for (int p = 0; p < kNumParams; ++p) {
    const auto id = static_cast<ParamId>(p);
    if (paramSpec(id).type != ParamType::Float) continue;
    ++nParams;
    const auto i = static_cast<size_t>(p);
    f.ctx.learnCc[i] = 127;
    for (int k = 0; k < 2; ++k) {
      ui::ContinuousControl c(f.ctx, id, ui::Form::Rotary, kDens[k]);
      c.setBounds(kBounds[k]);
      (k == 0 ? fitXL : fitS) += c.ccTagBox().isEmpty() ? 0 : 1;
    }
    f.ctx.learnCc[i] = -1;
  }
  const double ratio = contrast(ui::tok::textDim, ui::tok::panel), ratioChassis = contrast(ui::tok::textDim, ui::tok::chassis);
  const bool ok = configs == 4 * 3 * 2 * 4 && drawn > 0 && notDrawn > 0 && outside == 0 && masked == 0 && hiddenChanged == 0 && microDrawn == 0 &&
                  tooltipMissing == 0 && ratio >= 4.5 && ratioChassis >= 4.5;
  TK_CHECK(ok);
  tk::result("NX-CC", ok,
             "CE-B2-08 indicateur MIDI Learn : %d configurations (XL/S/Micro, libelles courts et longs, uni/bipolaire, actif/INACTIVE, x0.75/1/1.5/2, "
             "Base+Effective+5 routes) ; dessine %d (pixels modifies hors zone %d, pixels deja dessines recouverts %d), sans place %d (aucun pixel "
             "modifie : %d ecarts), Micro dessine %d (masque comme en 0.4.1) ; infobulle sans CC %d ; contraste %.2f:1 plaque, %.2f:1 chassis ; "
             "couverture (libelles courts des %d parametres continus) XL %d, S %d",
             configs, drawn, outside, masked, notDrawn, hiddenChanged, microDrawn, tooltipMissing, ratio, ratioChassis, nParams, fitXL, fitS);
}

void testNexusRoutes() {
  Fixture f;
  const ParamId id = ParamId::WtZ;
  paramOf(f.proc, id)->setValueNotifyingHost(0.4f);
  ui::ContinuousControl c(f.ctx, id, ui::Form::Rotary, ui::Density::XL);
  c.setBounds(kBounds[0]);
  c.tick();
  const auto bg = ui::tok::panel;
  const auto none = renderComp(c, 2.0f, bg);
  f.routes(id, {0.25f});
  c.tick();
  const auto one = renderComp(c, 2.0f, bg);
  f.routes(id, {0.25f}, true);  // même profondeur, inversée : marque de l'autre côté
  c.tick();
  const auto inv = renderComp(c, 2.0f, bg);
  f.routes(id, {});
  c.tick();
  const auto back = renderComp(c, 2.0f, bg);
  // sans changement de révision, une modification de la copie publiée n'est pas lue (aucune lecture hors révision)
  f.ctx.mod.routes.push_back(ModRoute{});
  f.ctx.mod.routes.back().dest = id;
  f.ctx.mod.routes.back().depth = 0.25f;
  c.tick();
  const auto stale = renderComp(c, 2.0f, bg);
  f.ctx.mod.routes.clear();
  int dOne = 0, dInv = 0, dOneInv = 0, dBack = 0, dStale = 0;
  for (int y = 0; y < none.getHeight(); ++y)
    for (int x = 0; x < none.getWidth(); ++x) {
      dOne += samePixel(none, one, x, y) ? 0 : 1;
      dInv += samePixel(none, inv, x, y) ? 0 : 1;
      dOneInv += samePixel(one, inv, x, y) ? 0 : 1;
      dBack += samePixel(none, back, x, y) ? 0 : 1;
      dStale += samePixel(none, stale, x, y) ? 0 : 1;
    }
  const bool ok = dOne > 0 && dInv > 0 && dOneInv > 0 && dBack == 0 && dStale == 0;
  TK_CHECK(ok);
  tk::result("NX-ROUTES", ok,
             "marques de route depuis la configuration publiee (revision) : ajout %d px, inversion %d px (distincte de l'ajout : %d px), retrait -> rendu "
             "initial (%d px d'ecart), copie modifiee sans revision ignoree (%d px)",
             dOne, dInv, dOneInv, dBack, dStale);
}

void testNexusGeometry() {
  int geomBad = 0, entryBad = 0, writesBeforeEntry = 0, resetBad = 0, checked = 0;
  for (int k = 0; k < 3; ++k) {
    Fixture f;
    auto* prm = paramOf(f.proc, ParamId::WtX);
    prm->setValueNotifyingHost(0.7f);
    ui::ContinuousControl c(f.ctx, ParamId::WtX, ui::Form::Rotary, kDens[k]);
    c.setBounds(kBounds[k]);
    MiniHost host(c);
    const auto va = sbd::r2::nexusValueArea(c.getLocalBounds(), r2Size(k));
    geomBad += c.valueBox() == va ? 0 : 1;
    ++checked;
    // double-clic sur la fenêtre de valeur : saisie, sans écriture
    const float before = prm->getValue();
    host.doubleClick(c, va.getCentre());
    auto* ed = entryOf(c);
    entryBad += (ed != nullptr && ed->getBounds() == va.toNearestInt().expanded(0, 1)) ? 0 : 1;
    writesBeforeEntry += prm->getValue() == before ? 0 : 1;
    if (ed != nullptr && ed->onEscapeKey) ed->onEscapeKey();  // abandon de la saisie (aucune écriture)
    // double-clic sur le bouton : retour au défaut (contrat NEXUS 0.4.1 inchangé)
    geomBad += c.knobBox() == sbd::r2::nexusKnobArea(c.getLocalBounds(), r2Size(k), juce::String(ui::shortLabel(ParamId::WtX)), 0.0f) ? 0 : 1;
    host.doubleClick(c, c.knobBox().getCentre());
    resetBad += std::abs(prm->getValue() - prm->getDefaultValue()) < 1.0e-6f ? 0 : 1;
  }
  const bool ok = checked == 3 && geomBad == 0 && entryBad == 0 && writesBeforeEntry == 0 && resetBad == 0;
  TK_CHECK(ok);
  tk::result("NX-GEOM", ok,
             "geometrie du peintre R1 (XL, S, Micro) : fenetre de valeur differente %d ; saisie hors fenetre %d, ecriture avant saisie %d ; double-clic "
             "bouton sans retour au defaut %d",
             geomBad, entryBad, writesBeforeEntry, resetBad);
}

// Mesure (information, dossier d'arbitrage CE-B2-08) : union des pixels dessinés par le peintre R1 sur un balayage d'états
// (Base 0..1, uni/bipolaire, Effective, 5 routes, survol/focus/pression), et recouvrement de chaque emplacement candidat
// de l'indicateur « CC127 » (corps 8,5 gras, comme les marqueurs « +n » et « INACTIVE » du peintre).
void measureCcPlacements() {
  juce::StringArray rows{"gabarit,libelle,emplacement,x,y,w,h,pixels_recouverts_actif,pixels_recouverts_inactive"};
  juce::String summary;
  const struct {
    sbd::r2::NexusSize size;
    juce::Rectangle<int> b;
    const char* label;
  } cfgs[4] = {{sbd::r2::NexusSize::XL, {0, 0, 132, 150}, "X"}, {sbd::r2::NexusSize::XL, {0, 0, 132, 150}, "FM INDEX"},
               {sbd::r2::NexusSize::S, {0, 0, 84, 96}, "AMOUNT"}, {sbd::r2::NexusSize::S, {0, 0, 84, 96}, "RATIO"}};
  const float sc = 2.0f;
  const auto tagFont = sbd::r2::labelFont(8.5f, true);
  const float tw = sbd::r2::draw::textWidth(tagFont, "CC127") + 2.0f;
  for (const auto& cf : cfgs) {
    const int W = cf.b.getWidth() * 2, H = cf.b.getHeight() * 2;
    std::vector<std::uint8_t> act(static_cast<size_t>(W * H), 0), ina(static_cast<size_t>(W * H), 0);
    const auto bg = ui::tok::panel;
    for (int inactive = 0; inactive < 2; ++inactive)
      for (int bip = 0; bip < 2; ++bip)
        for (int ni = 0; ni <= 10; ++ni)
          for (int ei = -1; ei <= 4; ++ei)
            for (int ri = 0; ri < 2; ++ri)
              for (int st = 0; st < 2; ++st) {
                sbd::r2::NexusView v;
                v.size = cf.size;
                v.domain = sbd::r2::Domain::Wt;
                v.label = cf.label;
                v.norm = static_cast<float>(ni) / 10.0f;
                v.bipolar = bip == 1;
                v.zeroNorm = 0.5f;
                if (ei >= 0) v.effective = static_cast<float>(ei) / 4.0f;
                if (ri == 1) v.routeDepths = {0.2f, -0.2f, 0.1f, -0.1f, 0.05f};
                v.valueText = "0.620 pos";
                v.inactive = inactive == 1;
                v.hover = v.focus = v.pressed = st == 1;
                juce::Image img(juce::Image::ARGB, W, H, true, juce::SoftwareImageType());
                {
                  juce::Graphics g(img);
                  g.fillAll(bg);
                  g.addTransform(juce::AffineTransform::scale(sc));
                  sbd::r2::paintNexus(g, cf.b, v);
                }
                auto& m = inactive ? ina : act;
                for (int y = 0; y < H; ++y)
                  for (int x = 0; x < W; ++x)
                    if (img.getPixelAt(x, y) != bg) m[static_cast<size_t>(y * W + x)] = 1;
              }
    const auto ka = sbd::r2::nexusKnobArea(cf.b, cf.size, cf.label, 0.0f);
    const float lh = ka.getY();
    const struct {
      const char* name;
      juce::Rectangle<float> r;
    } cand[6] = {{"bouton-haut-gauche", {ka.getX() + 2.0f, ka.getY(), tw, 11.0f}},
                 {"bouton-haut-droite", {ka.getRight() - tw - 2.0f, ka.getY(), tw, 11.0f}},
                 {"bouton-bas-gauche", {ka.getX() + 2.0f, ka.getBottom() - 11.0f, tw, 11.0f}},
                 {"bouton-bas-droite", {ka.getRight() - tw - 2.0f, ka.getBottom() - 11.0f, tw, 11.0f}},
                 {"serigraphie-gauche", {2.0f, 0.0f, tw, lh}},
                 {"serigraphie-droite", {static_cast<float>(cf.b.getWidth()) - tw - 2.0f, 0.0f, tw, lh}}};
    for (const auto& c : cand) {
      const auto p = (c.r * sc).getSmallestIntegerContainer().getIntersection({0, 0, W, H});
      int na = 0, ni = 0;
      for (int y = p.getY(); y < p.getBottom(); ++y)
        for (int x = p.getX(); x < p.getRight(); ++x) {
          na += act[static_cast<size_t>(y * W + x)];
          ni += ina[static_cast<size_t>(y * W + x)];
        }
      rows.add(juce::String(cf.size == sbd::r2::NexusSize::XL ? "XL" : "S") + "," + cf.label + "," + c.name + "," + juce::String(c.r.getX(), 1) + "," +
               juce::String(c.r.getY(), 1) + "," + juce::String(c.r.getWidth(), 1) + "," + juce::String(c.r.getHeight(), 1) + "," + juce::String(na) + "," +
               juce::String(ni));
      if (na == 0) summary << (cf.size == sbd::r2::NexusSize::XL ? "XL" : "S") << "/" << cf.label << "/" << c.name << " libre ; ";
    }
  }
  const auto dir = juce::SystemStats::getEnvironmentVariable("SBD_B2_OUT", {});
  if (dir.isNotEmpty()) juce::File(dir).getChildFile("nx-cc-placements.csv").replaceWithText(rows.joinIntoString("\n") + "\n", false, false, "\n");
  tk::result("NX-CC-PLACE", true, "[INFORMATION] emplacements libres de l'indicateur CC127 (%.1f px) sur tout le balayage d'etats actifs : %s", tw,
             summary.isEmpty() ? "aucun" : summary.toRawUTF8());
}

}  // namespace

void testB2NexusIntegration() {
  std::fprintf(stderr, "[ARCH-UI-04-R2-B2-03 (1) : NEXUS R1 de production]\n");
  testNexusCc();
  testNexusRoutes();
  testNexusGeometry();
  measureCcPlacements();
}
