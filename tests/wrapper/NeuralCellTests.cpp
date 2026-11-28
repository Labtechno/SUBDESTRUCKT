// SUBDESTRUCKT — ARCH-UI-04-R2-B2-03 (3) : tests d'intégration des NEURAL CELL R1 de production (subdestruckt_wrapper_tests).
// Critères enregistrés avant mesure (B2-ACCEPTANCE-CRITERIA.md, addendum B2-03.3) :
//  NC-G      dans les 12 configurations d'UI04-CLIP et tous les contextes : chaque NeuralCell et chaque ActionCell visible
//            (hors PANIC, B2-03.6) est rendue par le peintre R1 si son libellé tient sur la ligne du gabarit, sinon garde le
//            rendu 0.4.1 avec une raison publiée ; aucun libellé tronqué (textFits) ; rapport des comptes et des raisons.
//  NC-STATES les états de production restent distincts à l'écran et portés par le texte (jamais la couleur seule) :
//            OFF, ON, ARMÉ, ACTIF, REFUS, indisponible ; survol, pression, focus ; action, bascule, armement.
#include <juce_gui_basics/juce_gui_basics.h>

#include <map>
#include <set>

#include "TestKit.h"
#include "core/Parameters.h"
#include "ui/Components.h"
#include "ui/MainEditor.h"

using namespace sbd;

namespace {

struct NcFixture {
  SubdestrucktProcessor p;
  std::unique_ptr<juce::AudioProcessorEditor> ed;
  ui::MainEditor* m = nullptr;
  NcFixture() {
    p.prepareToPlay(48000.0, 256);
    ed.reset(p.createEditorIfNeeded());
    m = dynamic_cast<ui::MainEditor*>(ed.get());
  }
  ~NcFixture() {
    if (ed) p.editorBeingDeleted(ed.get());
  }
};

void collectAll(juce::Component& c, std::vector<juce::Component*>& out) {
  for (auto* ch : c.getChildren()) {
    out.push_back(ch);
    collectAll(*ch, out);
  }
}

template <class B>  // paintButton est public dans NeuralCell et ActionCell (protégé dans juce::Button)
juce::Image render(B& b, bool over, bool down) {
  juce::Image img(juce::Image::ARGB, b.getWidth() * 2, b.getHeight() * 2, true, juce::SoftwareImageType());
  {
    juce::Graphics g(img);
    g.fillAll(ui::tok::panel);
    g.addTransform(juce::AffineTransform::scale(2.0f));
    b.paintButton(g, over, down);
  }
  return img;
}

int differing(const juce::Image& a, const juce::Image& b) {
  int n = 0;
  for (int y = 0; y < a.getHeight(); ++y)
    for (int x = 0; x < a.getWidth(); ++x) n += a.getPixelAt(x, y) != b.getPixelAt(x, y) ? 1 : 0;
  return n;
}

}  // namespace

void testNeuralCellIntegration() {
  std::fprintf(stderr, "[ARCH-UI-04-R2-B2-03 (3) : NEURAL CELL R1 de production]\n");
  // ---- NC-G
  {
    auto fx = std::make_unique<NcFixture>();
    auto& m = *fx->m;
    int seenCells = 0, r1Cells = 0, seenActions = 0, r1Actions = 0, noReason = 0, clipped = 0;
    std::map<std::string, int> blocked;
    for (auto sz : {std::pair<int, int>{1440, 900}, {960, 640}, {1920, 1200}})
      for (int z : {75, 100, 125, 150}) {
        m.setSize(sz.first, sz.second);
        m.setZoomPercent(z);
        m.setInspectorOpen(sz.first >= 1200);
        for (int s = 0; s < 3; ++s)
          for (int t = 0; t < m.numTabs(s); ++t)
            for (int k = 0; k < m.numSubContexts(s, t); ++k) {
              m.showSubContext(s, t, k);
              m.refreshLayout();
              std::vector<juce::Component*> all;
              collectAll(m, all);
              for (auto* c : all) {
                if (!m.isVisibleInEditor(c)) continue;
                if (auto* n = dynamic_cast<ui::NeuralCell*>(c)) {
                  ++seenCells;
                  if (n->r1()) ++r1Cells;
                  else {
                    noReason += n->r1BlockReason().isEmpty() ? 1 : 0;
                    ++blocked[n->r1BlockReason().toStdString()];
                  }
                  clipped += n->textFits() ? 0 : 1;
                } else if (auto* a = dynamic_cast<ui::ActionCell*>(c)) {
                  if (a->r1BlockReason().startsWith("PANIC")) continue;
                  ++seenActions;
                  if (a->r1()) ++r1Actions;
                  else {
                    noReason += a->r1BlockReason().isEmpty() ? 1 : 0;
                    ++blocked[a->r1BlockReason().toStdString()];
                  }
                  clipped += a->textFits() ? 0 : 1;
                }
              }
            }
      }
    juce::String detail;
    int shown = 0;
    for (const auto& [why, n] : blocked)
      if (shown++ < 12) detail << juce::String::fromUTF8(why.c_str()) << " x" << n << " ; ";
    const bool ok = seenCells > 0 && seenActions > 0 && r1Cells + r1Actions > 0 && noReason == 0 && clipped == 0;
    TK_CHECK(ok);
    const auto dir = juce::SystemStats::getEnvironmentVariable("SBD_B2_OUT", {});
    if (dir.isNotEmpty()) {
      juce::StringArray rows{"raison;occurrences"};
      for (const auto& [why, n] : blocked) rows.add(juce::String::fromUTF8(why.c_str()) + ";" + juce::String(n));
      juce::File(dir).getChildFile("nc-g-blocages.csv").replaceWithText(rows.joinIntoString("\n") + "\n", false, false, "\n");
    }
    tk::result("NC-G", ok,
               "NEURAL CELL : %d cellules visibles (12 configurations, tous contextes), R1 %d ; %d actions (hors PANIC), R1 %d ; bloquees sans raison %d, "
               "libelles tronques %d ; %d raisons distinctes : %s",
               seenCells, r1Cells, seenActions, r1Actions, noReason, clipped, static_cast<int>(blocked.size()), detail.toRawUTF8());
  }
  // ---- NC-STATES
  {
    SubdestrucktProcessor proc;
    ui::UiContext ctx{proc, nullptr, RandomOptions{}, {}, {}, {}, {}, ParamId::WtX, {}};
    ctx.learnCc.fill(-1);
    ui::NeuralCell cell(ctx, ParamId::DstWt1D2Freeze);
    cell.setCaption("FREEZE");  // libellés courts du gabarit R1 (les libellés de production longs gardent le rendu 0.4.1 : NC-G)
    cell.setBounds(0, 0, 168, 40);
    ui::NeuralCell arm(ctx, ParamId::DstWt1Arm);
    arm.setCaption("ARM");
    arm.setBounds(0, 0, 168, 40);
    ui::ActionCell act("Cross Reset", ui::tok::cross);
    act.setBounds(0, 0, 168, 40);
    ui::ActionCell tog("Inspector", ui::tok::mod);
    tog.setClickingTogglesState(true);
    tog.setBounds(0, 0, 168, 40);
    int pairs = 0, same = 0;
    std::string firstSame;
    const auto distinct = [&](const juce::Image& a, const juce::Image& b, const std::string& what) {
      ++pairs;
      if (differing(a, b) == 0) {
        ++same;
        if (firstSame.empty()) firstSame = what;
      }
    };
    using CS = ui::CellState;
    std::map<int, juce::Image> st;
    for (CS s : {CS::Off, CS::Active, CS::Refused, CS::Disabled}) {
      cell.setCellState(s);
      st[static_cast<int>(s)] = render(cell, false, false);
    }
    distinct(st[0], st[2], "toggle OFF / ON");
    distinct(st[2], st[3], "toggle ON / REFUS");
    distinct(st[0], st[4], "toggle OFF / indisponible");
    std::map<int, juce::Image> sa;
    for (CS s : {CS::Off, CS::Armed, CS::Active, CS::Refused}) {
      arm.setCellState(s);
      sa[static_cast<int>(s)] = render(arm, false, false);
    }
    distinct(sa[0], sa[1], "armement OFF / ARME");
    distinct(sa[1], sa[2], "armement ARME / ACTIF");
    distinct(sa[2], sa[3], "armement ACTIF / REFUS");
    cell.setCellState(CS::Off);
    const auto n0 = render(cell, false, false), nh = render(cell, true, false), np = render(cell, true, true);
    distinct(n0, nh, "survol");
    distinct(nh, np, "pression");
    const auto a0 = render(act, false, false), ap = render(act, true, true);
    distinct(a0, ap, "action pressee");
    tog.setToggleState(false, juce::dontSendNotification);
    const auto t0 = render(tog, false, false);
    tog.setToggleState(true, juce::dontSendNotification);
    const auto t1 = render(tog, false, false);
    distinct(t0, t1, "bascule OFF / ON");
    act.setEnabled(false);
    distinct(a0, render(act, false, false), "action indisponible");
    const bool allR1 = cell.r1() && arm.r1() && act.r1() && tog.r1();
    const bool ok = allR1 && same == 0;
    TK_CHECK(ok);
    tk::result("NC-STATES", ok, "etats NEURAL CELL distincts au rendu R1 : %d paires comparees, identiques %d%s%s ; quatre cellules en R1 : %d", pairs, same,
               firstSame.empty() ? "" : " : ", firstSame.c_str(), allR1);
  }
}
