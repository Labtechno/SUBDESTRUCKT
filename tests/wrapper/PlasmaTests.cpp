// SUBDESTRUCKT — ARCH-UI-04-R2-B2-03 (2) : tests d'intégration des PLASMA R1 de production (subdestruckt_wrapper_tests).
#include <juce_gui_basics/juce_gui_basics.h>

#include <map>
#include <set>

#include "TestKit.h"
#include "core/Parameters.h"
#include "ui/Components.h"
#include "ui/MainEditor.h"

using namespace sbd;

namespace {

struct PlFixture {
  SubdestrucktProcessor p;
  std::unique_ptr<juce::AudioProcessorEditor> ed;
  ui::MainEditor* m = nullptr;
  PlFixture() {
    p.prepareToPlay(48000.0, 256);
    ed.reset(p.createEditorIfNeeded());
    m = dynamic_cast<ui::MainEditor*>(ed.get());
  }
  ~PlFixture() {
    if (ed) p.editorBeingDeleted(ed.get());
  }
};

}  // namespace

// PLAS-G (CE-B2-02) : gabarit PLASMA choisi par la surface utile réelle, dans les 12 configurations d'UI04-CLIP
// (1440x900, 960x640, 1920x1200 ; zooms 75, 100, 125, 150 %), pour tous les PLASMA visibles de tous les contextes.
// Invariants d'un PLASMA R1 : gabarit aux dimensions normatives exactes (STANDARD 64 x 180, COMPACT 48 x 150, horizontal
// hauteur 44), contenu dans la cellule ; canal et zone de manipulation dans le gabarit ; libellé et valeur courante tenant
// dans leurs zones. Un PLASMA bloqué porte une raison documentée et garde le rendu 0.4.1 (CE-B2-02 : ni déformation, ni
// réduction typographique, ni anticipation de R2-C). Le rapport liste le gabarit de chaque paramètre.
void testPlasmaGeometry() {
  auto fx = std::make_unique<PlFixture>();
  auto& m = *fx->m;
  int seen = 0, r1 = 0, bad = 0, blockedNoReason = 0;
  std::map<std::string, std::set<std::string>> verdicts;
  std::string firstBad;
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
            for (auto* b : m.visibleControls()) {
              auto* c = dynamic_cast<ui::ContinuousControl*>(b);
              if (c == nullptr || c->form() == ui::Form::Rotary) continue;
              ++seen;
              const std::string id = paramSpec(c->boundId()).stableId;
              const auto lb = c->getLocalBounds();
              if (!c->plasmaR1()) {
                blockedNoReason += c->plasmaBlockReason().isEmpty() ? 1 : 0;
                verdicts[id].insert("BLOQUE (" + c->plasmaBlockReason().toStdString() + ")");
                continue;
              }
              ++r1;
              const auto tpl = c->plasmaTemplate();
              const bool V = c->form() == ui::Form::LinearV;
              const bool sizeOk = V ? (c->plasmaCompact() ? tpl.getWidth() == 48 && tpl.getHeight() == 150 : tpl.getWidth() == 64 && tpl.getHeight() == 180)
                                    : tpl.getHeight() == 44 && tpl.getWidth() == lb.getWidth();
              const auto tf = tpl.toFloat();
              const bool inside = lb.contains(tpl) && tf.contains(c->channelBox()) && lb.toFloat().contains(c->manipulationBox()) &&
                                  tf.contains(c->valueBox()) && tf.contains(c->labelBox());
              const bool ok = sizeOk && inside && c->textFits();
              if (!ok) {
                ++bad;
                if (firstBad.empty()) firstBad = id + " " + std::to_string(lb.getWidth()) + "x" + std::to_string(lb.getHeight());
              }
              verdicts[id].insert(V ? (c->plasmaCompact() ? "COMPACT 48x150" : "STANDARD 64x180") : "HORIZONTAL h44");
            }
          }
    }
  juce::String detail;
  for (const auto& [id, vs] : verdicts) {
    detail << id.c_str() << " :";
    for (const auto& v : vs) detail << " " << juce::String::fromUTF8(v.c_str()) << ";";
    detail << " ";
  }
  const bool ok = seen > 0 && r1 > 0 && bad == 0 && blockedNoReason == 0;
  TK_CHECK(ok);
  tk::result("PLAS-G", ok, "CE-B2-02 : %d PLASMA visibles (12 configurations, tous contextes), R1 %d, invariants violes %d%s%s, bloques sans raison %d ; %s", seen,
             r1, bad, firstBad.empty() ? "" : " (", firstBad.empty() ? "" : (firstBad + ")").c_str(), blockedNoReason, detail.toRawUTF8());
}
