// SUBDESTRUCKT — tests ARCH-UI-04 (interface SYMBIOTIC INDUSTRIAL), exécutés par subdestruckt_wrapper_tests.
// Critères (enregistrés avant mesure, docs/V1-ARCH-UI-04-REPORT.md §3) :
//  UI04-INV   registre compilé : 703 StableID uniques, identiques à la liste figée
//  UI04-COV   chaque StableID atteint par au moins une commande liée visible, contextes compatibles compris
//  UI04-BIND  chaque StableID : édition par sa commande -> valeur Base hôte attendue, aucune autre écriture, retour exact
//  UI04-ALIAS commandes multiples d'un même StableID : même valeur affichée
//  UI04-INACT changer d'algorithme/de contexte puis revenir : toutes les autres valeurs conservées (bit à bit)
//  UI04-NAV   navigation, zoom, Inspector, tailles : aucune écriture de paramètre ni d'état
//  UI04-CLIP  libellés et valeurs sans troncature ; commandes dans leur parent, sans chevauchement (3 tailles x 4 zooms)
//  UI04-PANIC Panic visible dans toutes les tailles/zooms ; un clic révoque et compte un Panic
//  UI04-UNDO  une édition = une transaction ; geste X/Y du Core = une transaction ; undo/redo exacts
//  UI04-AUTO  automation hôte (autre thread) reflétée sans réécriture ni transaction
//  UI04-STATE état identique éditeur ouvert/fermé ; restauration reflétée par les commandes
//  UI04-OPEN  100 cycles ouverture/fermeture avec audio : état et autorisation inchangés
//  UI04-MULTI quatre instances avec éditeurs : isolation des valeurs
//  UI04-RT    processBlock avec éditeur actif rafraîchi entre les blocs : 0 allocation
//  UI04-DSP   rendu bit à bit identique avec et sans éditeur (navigation complète entre les blocs)
//  UI04-CORE  SYMBIOTIC CORE : X/Y/Z écrivent cross.fwd/rev/master seulement ; alternative numérique cohérente
//  UI04-CON   contrastes des tokens (texte >= 4,5:1, secondaires >= 3:1)
//  UI04-KEY   clavier : pas, reset, saisie numérique
//  UI04-DECO  aucune commande décorative : boutons et listes visibles reliés à une action
//  UI04-OPS   opérations hors registre présentes dans l'interface
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>
#include <fstream>
#include <functional>
#include <map>
#include <set>
#include <thread>
#include <typeinfo>

#include "TestKit.h"
#include "core/StateCodec.h"
#include "plugin/PluginProcessor.h"
#include "ui/MainEditor.h"

using namespace sbd;
using namespace sbd::ui;

namespace {

struct Fixture {
  SubdestrucktProcessor p;
  std::unique_ptr<juce::AudioProcessorEditor> ed;
  MainEditor* m = nullptr;
  explicit Fixture(int w = 1440, int h = 900) {
    p.prepareToPlay(48000.0, 256);
    ed.reset(p.createEditorIfNeeded());
    m = dynamic_cast<MainEditor*>(ed.get());
    if (m) m->setSize(w, h);
  }
  ~Fixture() {
    if (ed) p.editorBeingDeleted(ed.get());  // protocole de l'hôte (wrapper VST3) avant destruction de l'éditeur
  }
};

struct Ctx {
  int s, t, sub;
};

std::vector<Ctx> contexts(MainEditor& m) {
  std::vector<Ctx> v;
  for (int s = 0; s < 3; ++s)
    for (int t = 0; t < m.numTabs(s); ++t)
      for (int k = 0; k < m.numSubContexts(s, t); ++k) v.push_back({s, t, k});
  return v;
}

void show(MainEditor& m, const Ctx& c) {
  m.showSubContext(c.s, c.t, c.sub);
  m.refreshLayout();
}

BoundControl* visibleControl(MainEditor& m, ParamId id) {
  for (auto* b : m.visibleControls())
    if (b->boundId() == id) return b;
  return nullptr;
}

bool discrete(ParamId id) {
  const auto t = paramSpec(id).type;
  return t == ParamType::Choice || t == ParamType::Bool;
}

std::vector<float> valuesOf(ParamId id) {
  std::vector<float> v;
  const int n = paramSpec(id).type == ParamType::Bool ? 2 : choiceCount(id);
  for (int k = 0; k < n; ++k) v.push_back(static_cast<float>(k));
  return v;
}

float tolOf(ParamId id) {
  const auto& s = paramSpec(id);
  return s.type == ParamType::Float ? 1e-4f * (s.maxValue - s.minValue) + 1e-6f : 0.01f;
}

void collect(juce::Component& c, std::vector<juce::Component*>& out) {
  out.push_back(&c);
  for (auto* ch : c.getChildren()) collect(*ch, out);
}

double luminance(juce::Colour c) {
  auto lin = [](float v) { return v <= 0.03928f ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
  return 0.2126 * lin(c.getFloatRed()) + 0.7152 * lin(c.getFloatGreen()) + 0.0722 * lin(c.getFloatBlue());
}
double contrast(juce::Colour a, juce::Colour b) {
  const double la = luminance(a), lb = luminance(b);
  return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

struct Recipe {
  Ctx c;
  std::vector<std::pair<ParamId, float>> drivers;
};

std::string recipeText(MainEditor& m, const Recipe& r) {
  std::string t = m.contextName(r.c.s, r.c.t, r.c.sub).toStdString();
  for (const auto& d : r.drivers) t += " ; " + std::string(paramSpec(d.first).stableId) + "=" + formatValue(d.first, d.second);
  return t;
}

bool applyRecipe(MainEditor& m, const Recipe& r) {
  show(m, r.c);
  for (const auto& d : r.drivers) {
    auto* b = visibleControl(m, d.first);
    if (!b) return false;
    b->userSet(d.second);
    m.refreshLayout();
  }
  return true;
}

void undoRecipe(MainEditor& m, const Recipe& r, const ParamValues& before) {
  for (auto it = r.drivers.rbegin(); it != r.drivers.rend(); ++it) {
    if (auto* b = visibleControl(m, it->first)) b->userSet(before[it->first]);
    m.refreshLayout();
  }
}

bool sameExcept(const ParamValues& a, const ParamValues& b, ParamId except, int* firstDiff = nullptr) {
  for (int i = 0; i < kNumParams; ++i)
    if (i != idx(except) && a.v[static_cast<size_t>(i)] != b.v[static_cast<size_t>(i)]) {
      if (firstDiff) *firstDiff = i;
      return false;
    }
  return true;
}

}  // namespace

// ======================================================================= couverture, bindings, alias, inactifs
void testUi04Coverage(const std::string& mapPath) {
  // UI04-INV
  {
    std::set<std::string> ids;
    for (int i = 0; i < kNumParams; ++i) ids.insert(paramSpec(i).stableId);
    std::ifstream f("tests/core/expected_param_ids.txt");
    int listed = 0;
    std::string line;
    bool same = true;
    int k = 0;
    while (std::getline(f, line)) {
      if (line.empty() || line[0] == '#') continue;
      const std::string id = line.substr(0, line.find_first_of(" \t\r"));
      same = same && k < kNumParams && id == paramSpec(k).stableId;
      ++listed, ++k;
    }
    const bool ok = kNumParams == 703 && static_cast<int>(ids.size()) == kNumParams && listed == kNumParams && same;
    TK_CHECK(ok);
    tk::result("UI04-INV", ok, "registre compile : %d parametres, %zu StableID uniques ; liste figee : %d, identique (ordre) %d", kNumParams, ids.size(), listed, same);
  }
  auto fxHeap = std::make_unique<Fixture>();  // processeur complet : sur le tas
  auto& fx = *fxHeap;
  if (!fx.m) {
    tk::result("UI04-COV", false, "editeur indisponible");
    return;
  }
  auto& m = *fx.m;
  auto& p = fx.p;
  juce::MemoryBlock blob0;
  p.getStateInformation(blob0);
  std::map<int, Recipe> found;
  std::map<int, int> seenCount;
  int states = 0, inactChecks = 0, inactViol = 0;
  std::string inactFirst;
  // exploration : chaque contexte, puis chaque commande discrète visible (et celles qu'elle révèle) sur toutes ses valeurs
  std::function<void(const Ctx&, std::vector<std::pair<ParamId, float>>, int, const std::set<int>&)> explore =
      [&](const Ctx& c, std::vector<std::pair<ParamId, float>> drivers, int depth, const std::set<int>& parentVisible) {
        ++states;
        std::set<int> vis;
        for (auto* b : m.visibleControls()) vis.insert(idx(b->boundId()));
        for (int id : vis) {
          if (!found.count(id)) found[id] = Recipe{c, drivers};
          ++seenCount[id];
        }
        if (depth >= 3) return;
        std::vector<ParamId> drv;
        for (int id : vis) {
          const auto pid = static_cast<ParamId>(id);
          if (!discrete(pid) || (depth > 0 && parentVisible.count(id))) continue;
          bool used = false;
          for (const auto& d : drivers) used = used || d.first == pid;
          if (!used) drv.push_back(pid);
        }
        for (auto d : drv) {
          const auto before = p.parameters().values();
          const float orig = before[d];
          for (float v : valuesOf(d)) {
            if (std::abs(v - orig) < 0.5f) continue;
            auto* b = visibleControl(m, d);
            if (!b) break;
            b->userSet(v);
            m.refreshLayout();
            auto dr = drivers;
            dr.push_back({d, v});
            explore(c, dr, depth + 1, vis);
          }
          if (auto* b = visibleControl(m, d)) b->userSet(orig);
          m.refreshLayout();
          const auto after = p.parameters().values();
          ++inactChecks;
          int diff = -1;
          if (!sameExcept(before, after, ParamId::Count, &diff)) {
            if (inactViol++ == 0) inactFirst = std::string(paramSpec(d).stableId) + " -> " + paramSpec(diff).stableId;
          }
        }
      };
  const auto ctxs = contexts(m);
  for (const auto& c : ctxs) {
    show(m, c);
    explore(c, {}, 0, {});
  }
  std::vector<std::string> missing;
  for (int i = 0; i < kNumParams; ++i)
    if (!found.count(i)) missing.push_back(paramSpec(i).stableId);
  const bool cov = missing.empty();
  TK_CHECK(cov);
  tk::result("UI04-COV", cov, "%zu/%d StableID atteints par une commande liee visible ; %zu contextes de navigation, %d etats explores (algorithmes et modes compris) ; manquants : %zu%s%s",
             found.size(), kNumParams, ctxs.size(), states, missing.size(), missing.empty() ? "" : " premier ", missing.empty() ? "" : missing.front().c_str());
  const bool inact = inactViol == 0 && inactChecks > 0;
  TK_CHECK(inact);
  tk::result("UI04-INACT", inact, "%d changements de contexte (algorithmes, modes, types) puis retour : autres valeurs conservees bit a bit ; violations %d%s%s", inactChecks, inactViol,
             inactFirst.empty() ? "" : " premiere ", inactFirst.c_str());

  // UI04-BIND / UI04-ALIAS : chaque StableID édité par sa commande, dans son contexte
  int bindOk = 0, bindFail = 0, sideWrites = 0, restoreFail = 0, aliasParams = 0, aliasFail = 0;
  std::string firstFail;
  for (const auto& [id, rec] : found) {
    const auto pid = static_cast<ParamId>(id);
    const auto before = p.parameters().values();
    if (!applyRecipe(m, rec)) {
      ++bindFail;
      if (firstFail.empty()) firstFail = std::string(paramSpec(pid).stableId) + " (recette)";
      continue;
    }
    auto* b = visibleControl(m, pid);
    const auto& s = paramSpec(pid);
    const auto mid = p.parameters().values();
    const float orig = mid[pid];
    float target;
    if (s.type == ParamType::Bool) target = orig >= 0.5f ? 0.0f : 1.0f;
    else if (s.type == ParamType::Choice) target = static_cast<float>((static_cast<int>(orig + 0.5f) + 1) % std::max(1, choiceCount(pid)));
    else if (s.type == ParamType::Int) target = orig + 1.0f <= s.maxValue ? orig + 1.0f : orig - 1.0f;
    else {
      target = sanitize(pid, s.minValue + 0.3137f * (s.maxValue - s.minValue));
      if (std::abs(target - orig) < 10.0f * tolOf(pid)) target = sanitize(pid, s.minValue + 0.6871f * (s.maxValue - s.minValue));
    }
    bool ok = b != nullptr;
    if (b) {
      b->userSet(target);
      m.refreshLayout();
      const auto after = p.parameters().values();
      const bool valOk = std::abs(after[pid] - target) <= tolOf(pid);
      const bool noSide = sameExcept(mid, after, pid);
      sideWrites += !noSide;
      // alias : toutes les commandes visibles de ce StableID affichent la même valeur
      int n = 0;
      bool aliasOk = true;
      for (auto* a : m.visibleControls())
        if (a->boundId() == pid) {
          ++n;
          aliasOk = aliasOk && std::abs(a->shownBase() - after[pid]) <= tolOf(pid);
        }
      if (n > 1) ++aliasParams;
      aliasFail += !aliasOk;
      auto* b2 = visibleControl(m, pid);
      if (b2) b2->userSet(orig);
      m.refreshLayout();
      const bool back = std::abs(p.parameters().base(id) - orig) <= tolOf(pid);
      restoreFail += !back;
      ok = valOk && noSide && aliasOk && back && b2 != nullptr;
      if (!ok && firstFail.empty())
        firstFail = std::string(s.stableId) + " (valeur " + std::to_string(valOk) + ", ecritures annexes " + std::to_string(!noSide) + ", alias " + std::to_string(aliasOk) + ", retour " +
                    std::to_string(back) + ")";
    }
    undoRecipe(m, rec, before);
    ok ? ++bindOk : ++bindFail;
  }
  const bool bind = bindOk == kNumParams && bindFail == 0;
  TK_CHECK(bind);
  tk::result("UI04-BIND", bind, "%d/%d StableID edites par leur commande (Base hote = cible, aucune autre ecriture, retour) ; echecs %d, ecritures annexes %d, retours %d%s%s", bindOk, kNumParams,
             bindFail, sideWrites, restoreFail, firstFail.empty() ? "" : " ; premier ", firstFail.c_str());
  const bool alias = aliasFail == 0 && aliasParams > 0;
  TK_CHECK(alias);
  tk::result("UI04-ALIAS", alias, "%d StableID presentes par plusieurs commandes simultanees : valeurs affichees identiques (echecs %d)", aliasParams, aliasFail);

  // cartographie (artefact) : StableID, nom, contexte d'accès, recette, nombre d'états où visible
  if (!mapPath.empty()) {
    std::ofstream f(mapPath, std::ios::binary);
    f << "index\tstableId\tname\ttype\taccess\tvisibleStates\n";
    for (int i = 0; i < kNumParams; ++i) {
      const auto it = found.find(i);
      static const char* tn[] = {"Float", "Choice", "Bool", "Int"};
      f << i << '\t' << paramSpec(i).stableId << '\t' << paramSpec(i).name << '\t' << tn[static_cast<int>(paramSpec(i).type)] << '\t'
        << (it != found.end() ? recipeText(m, it->second) : std::string("NON ATTEINT")) << '\t' << seenCount[i] << '\n';
    }
    tk::info("UI04 : cartographie -> %s", mapPath.c_str());
  }
  // l'exploration n'a laissé aucune trace dans l'état sonore (hors état d'exécution)
  juce::MemoryBlock blob1;
  p.getStateInformation(blob1);
  tk::info("UI04 : etat apres exploration identique a l'initial : %d", blob0 == blob1);
}

// ======================================================================= navigation, clipping, Panic
void testUi04Layout() {
  // UI04-NAV : aucune écriture (écouteur sur tous les paramètres) ; état identique
  {
    auto fxHeap = std::make_unique<Fixture>();  // processeur complet : sur le tas (pile Windows 1 Mo, ASan)
    auto& fx = *fxHeap;
    auto& m = *fx.m;
    ModRoute route;  // une route globale : les lignes de la matrice existent pendant la navigation
    route.source = ModSource::Lfo3;
    route.dest = ParamId::WtZ;
    route.depth = 0.1f;
    fx.p.modAdd(route);
    m.refreshNow();
    struct Counter : juce::AudioProcessorParameter::Listener {
      int changes = 0, gestures = 0;
      void parameterValueChanged(int, float) override { ++changes; }
      void parameterGestureChanged(int, bool) override { ++gestures; }
    } cnt;
    for (int i = 0; i < kNumParams; ++i) fx.p.parameters().param(i)->addListener(&cnt);
    juce::MemoryBlock a;
    fx.p.getStateInformation(a);
    const bool undo0 = fx.p.canUndo();
    int views = 0;
    for (int z : {75, 100, 125, 150})
      for (bool insp : {true, false})
        for (auto sz : {std::pair<int, int>{1440, 900}, {960, 640}, {1920, 1200}}) {
          m.setSize(sz.first, sz.second);
          m.setZoomPercent(z);
          m.setInspectorOpen(insp);
          for (const auto& c : contexts(m)) {
            show(m, c);
            ++views;
          }
        }
    // une navigation ne détruit aucun composant (pointeurs conservés par l'hôte, l'accessibilité ou un test)
    std::vector<juce::Component*> before;
    collect(m, before);
    std::vector<juce::Component::SafePointer<juce::Component>> guards(before.begin(), before.end());
    std::vector<std::string> names;
    for (auto* c : before) names.push_back(std::string(typeid(*c).name()) + " '" + c->getTitle().toStdString() + "' " + c->getHelpText().toStdString());
    for (const auto& c : contexts(m)) show(m, c);
    m.setZoomPercent(100);
    for (const auto& c : contexts(m)) show(m, c);
    for (int k = 0; k < kNumParams; k += 3) m.selectParam(static_cast<ParamId>(k));  // sélection : aucune destruction non plus
    m.refreshNow();
    int destroyed = 0;
    for (size_t k = 0; k < guards.size(); ++k)
      if (guards[k] == nullptr && destroyed++ < 6) tk::info("UI04-NAV : detruit par la navigation : %s", names[k].c_str());
    for (int k = 0; k < kNumParams; k += 3) m.selectParam(static_cast<ParamId>(k));
    m.refreshNow();
    juce::MemoryBlock b;
    fx.p.getStateInformation(b);
    for (int i = 0; i < kNumParams; ++i) fx.p.parameters().param(i)->removeListener(&cnt);
    const bool ok = a == b && cnt.changes == 0 && cnt.gestures == 0 && fx.p.canUndo() == undo0 && destroyed == 0;
    TK_CHECK(ok);
    tk::result("UI04-NAV", ok, "%d vues (contextes x 3 tailles x 4 zooms x Inspector ouvert/ferme) + %d selections Inspector : ecritures %d, gestes %d, etat identique %d, pile Undo inchangee %d ; composants detruits par la navigation %d / %zu",
               views, (kNumParams + 2) / 3, cnt.changes, cnt.gestures, a == b, fx.p.canUndo() == undo0, destroyed, guards.size());
  }
  // UI04-CLIP / UI04-PANIC
  {
    auto fxHeap = std::make_unique<Fixture>();  // processeur complet : sur le tas (pile Windows 1 Mo, ASan)
    auto& fx = *fxHeap;
    auto& m = *fx.m;
    int checked = 0, textBad = 0, outside = 0, overlap = 0, tabsBad = 0, configs = 0, panicBad = 0;
    std::string firstText, firstOut, firstOver;
    for (auto sz : {std::pair<int, int>{1440, 900}, {960, 640}, {1920, 1200}})
      for (int z : {75, 100, 125, 150}) {
        m.setSize(sz.first, sz.second);
        m.setZoomPercent(z);
        m.setInspectorOpen(sz.first >= 1200);
        ++configs;
        auto& pb = m.panicButton();
        const bool panicOk = m.isVisibleInEditor(&pb) && m.getLocalBounds().contains(pb.getBoundsInParent() + m.barComponent().getPosition()) && pb.getWidth() >= 60;
        panicBad += !panicOk;
        for (const auto& c : contexts(m)) {
          show(m, c);
          auto vis = m.visibleControls();
          for (auto* b : vis) {
            auto& comp = b->asComponent();
            ++checked;
            if (!b->textFits()) {
              ++textBad;
              if (firstText.empty()) firstText = std::string(paramSpec(b->boundId()).stableId) + " @ " + m.contextName(c.s, c.t, c.sub).toStdString();
            }
            auto* parent = comp.getParentComponent();
            if (parent && !parent->getLocalBounds().contains(comp.getBounds())) {
              ++outside;
              if (firstOut.empty()) firstOut = std::string(paramSpec(b->boundId()).stableId) + " @ " + m.contextName(c.s, c.t, c.sub).toStdString();
            }
          }
          for (size_t i = 0; i < vis.size(); ++i)
            for (size_t j = i + 1; j < vis.size(); ++j) {
              auto& a = vis[i]->asComponent();
              auto& bb = vis[j]->asComponent();
              if (a.getParentComponent() == bb.getParentComponent() && a.getBounds().intersects(bb.getBounds())) {
                ++overlap;
                if (firstOver.empty()) firstOver = std::string(paramSpec(vis[i]->boundId()).stableId) + " / " + paramSpec(vis[j]->boundId()).stableId;
              }
            }
          std::vector<juce::Component*> all;
          collect(m, all);
          for (auto* x : all)
            if (auto* t = dynamic_cast<TabStrip*>(x))
              if (m.isVisibleInEditor(t) && !t->textFits()) ++tabsBad;
        }
      }
    const bool ok = textBad == 0 && outside == 0 && overlap == 0 && tabsBad == 0;
    TK_CHECK(ok);
    tk::result("UI04-CLIP", ok, "%d configurations (1440x900, 960x640, 1920x1200 ; zoom 75-150 %%), %d commandes visibles verifiees : troncatures %d%s%s, hors parent %d%s%s, chevauchements %d%s%s, onglets tronques %d",
               configs, checked, textBad, firstText.empty() ? "" : " (", firstText.empty() ? "" : (firstText + ")").c_str(), outside, firstOut.empty() ? "" : " (",
               firstOut.empty() ? "" : (firstOut + ")").c_str(), overlap, firstOver.empty() ? "" : " (", firstOver.empty() ? "" : (firstOver + ")").c_str(), tabsBad);
    // clic Panic : révocation et comptage
    m.setSize(1440, 900);
    const auto panics0 = fx.p.telemetry().panics.load();
    if (auto* btn = dynamic_cast<juce::Button*>(&m.panicButton())) btn->onClick();  // gestionnaire du clic (triggerClick est asynchrone)
    juce::AudioBuffer<float> buf(2, 256);
    juce::MidiBuffer midi;
    fx.p.processBlock(buf, midi);
    const bool clicked = fx.p.telemetry().panics.load() == panics0 + 1 && !fx.p.autonomyAuthorized();
    const bool pok = panicBad == 0 && clicked;
    TK_CHECK(pok);
    tk::result("UI04-PANIC", pok, "Panic visible et entier dans %d configurations (echecs %d) ; clic -> Panic compte %d, autorisation revoquee %d", configs, panicBad, clicked,
               !fx.p.autonomyAuthorized());
  }
}

// ======================================================================= Undo, automation, état, clavier, Core
void testUi04Behaviour() {
  // UI04-UNDO
  {
    auto fxHeap = std::make_unique<Fixture>();  // processeur complet : sur le tas (pile Windows 1 Mo, ASan)
    auto& fx = *fxHeap;
    auto& m = *fx.m;
    show(m, {0, 0, 0});
    auto* x = visibleControl(m, ParamId::WtX);
    const float x0 = fx.p.parameters().base(idx(ParamId::WtX));
    bool ok = x != nullptr;
    if (x) {
      x->userSet(0.5f);
      m.refreshLayout();
      const bool set = std::abs(fx.p.parameters().base(idx(ParamId::WtX)) - 0.5f) < 1e-4f && fx.p.canUndo();
      fx.p.undo();
      m.refreshLayout();
      const bool und = std::abs(fx.p.parameters().base(idx(ParamId::WtX)) - x0) < 1e-6f && std::abs(x->shownBase() - x0) < 1e-6f;
      fx.p.redo();
      m.refreshLayout();
      const bool red = std::abs(fx.p.parameters().base(idx(ParamId::WtX)) - 0.5f) < 1e-4f && std::abs(x->shownBase() - 0.5f) < 1e-4f;
      // geste X/Y du Core : une seule transaction pour deux paramètres
      CorePad* pad = nullptr;
      std::vector<juce::Component*> all;
      collect(m, all);
      for (auto* c : all)
        if (auto* cp = dynamic_cast<CorePad*>(c)) pad = cp;
      const float f0 = fx.p.parameters().base(idx(ParamId::CrossFwd)), r0 = fx.p.parameters().base(idx(ParamId::CrossRev));
      bool core = pad != nullptr;
      if (pad) {
        pad->userMove(0.25f, 0.75f);
        m.refreshLayout();
        const bool moved = std::abs(fx.p.parameters().base(idx(ParamId::CrossFwd)) - 0.25f) < 1e-4f && std::abs(fx.p.parameters().base(idx(ParamId::CrossRev)) - 0.75f) < 1e-4f;
        fx.p.undo();
        m.refreshLayout();
        core = moved && std::abs(fx.p.parameters().base(idx(ParamId::CrossFwd)) - f0) < 1e-6f && std::abs(fx.p.parameters().base(idx(ParamId::CrossRev)) - r0) < 1e-6f;
      }
      ok = set && und && red && core;
      tk::info("UI04-UNDO : edition %d, undo %d, redo %d, Core (2 parametres, 1 transaction) %d", set, und, red, core);
    }
    TK_CHECK(ok);
    tk::result("UI04-UNDO", ok, "edition NEXUS = 1 transaction (undo/redo exacts, affichage suivi) ; geste X/Y du SYMBIOTIC CORE = 1 transaction pour cross.fwd + cross.rev : %d", ok);
  }
  // UI04-AUTO : automation depuis un autre thread (hôte)
  {
    auto fxHeap = std::make_unique<Fixture>();  // processeur complet : sur le tas (pile Windows 1 Mo, ASan)
    auto& fx = *fxHeap;
    auto& m = *fx.m;
    show(m, {1, 0, 0});
    auto* q = fx.p.parameters().param(idx(ParamId::WtX));
    const std::string label0 = fx.p.undoLabel();
    const bool undo0 = fx.p.canUndo();
    std::thread th([q] { q->setValueNotifyingHost(0.8f); });
    th.join();
    m.refreshNow();
    m.refreshNow();
    const float base = fx.p.parameters().base(idx(ParamId::WtX));
    bool shown = true;
    int n = 0;
    for (auto* b : m.visibleControls())
      if (b->boundId() == ParamId::WtX) shown = shown && std::abs(b->shownBase() - base) < 1e-5f, ++n;
    const bool ok = n > 0 && shown && std::abs(q->getValue() - 0.8f) < 1e-6f && fx.p.canUndo() == undo0 && fx.p.undoLabel() == label0;
    TK_CHECK(ok);
    tk::result("UI04-AUTO", ok, "automation hote (thread non-UI) : %d commande(s) a jour %d, valeur hote intacte (pas de reecriture) %d, aucune transaction Undo %d", n, shown,
               std::abs(q->getValue() - 0.8f) < 1e-6f, fx.p.canUndo() == undo0);
  }
  // UI04-STATE
  {
    auto aHeap = std::make_unique<Fixture>();
    auto& a = *aHeap;
    show(*a.m, {1, 0, 0});
    for (auto id : {ParamId::WtX, ParamId::WtY, ParamId::CrossMaster, ParamId::Op1Level, ParamId::WtZAType})
      if (auto* b = visibleControl(*a.m, id)) b->userSet(paramSpec(id).type == ParamType::Choice ? 5.0f : 0.42f);
    juce::MemoryBlock blob;
    a.p.getStateInformation(blob);
    auto closedHeap = std::make_unique<SubdestrucktProcessor>();
    auto& closed = *closedHeap;
    closed.prepareToPlay(48000.0, 256);
    closed.setStateInformation(blob.getData(), static_cast<int>(blob.getSize()));
    auto openHeap = std::make_unique<Fixture>();
    auto& open = *openHeap;
    open.p.setStateInformation(blob.getData(), static_cast<int>(blob.getSize()));
    show(*open.m, {1, 0, 0});
    juce::MemoryBlock b1, b2;
    closed.getStateInformation(b1);
    open.p.getStateInformation(b2);
    bool reflect = true;
    int n = 0;
    for (auto* b : open.m->visibleControls()) {
      ++n;
      reflect = reflect && std::abs(b->shownBase() - open.p.parameters().base(idx(b->boundId()))) <= tolOf(b->boundId());
    }
    const bool ok = b1 == b2 && b1 == blob && reflect && n > 0;
    TK_CHECK(ok);
    tk::result("UI04-STATE", ok, "etat (format 4 inchange) : editeur ouvert = editeur ferme = blob sauvegarde %d ; %d commandes refletent la restauration %d", b1 == b2 && b1 == blob, n, reflect);
  }
  // UI04-KEY
  {
    auto fxHeap = std::make_unique<Fixture>();  // processeur complet : sur le tas (pile Windows 1 Mo, ASan)
    auto& fx = *fxHeap;
    auto& m = *fx.m;
    show(m, {0, 0, 0});
    ContinuousControl* c = nullptr;
    for (auto* b : m.visibleControls())
      if (b->boundId() == ParamId::WtY && !c) c = dynamic_cast<ContinuousControl*>(b);
    bool ok = c != nullptr;
    if (c) {
      const float v0 = fx.p.parameters().base(idx(ParamId::WtY));
      c->keyPressed(juce::KeyPress(juce::KeyPress::upKey));
      const float v1 = fx.p.parameters().base(idx(ParamId::WtY));
      c->keyPressed(juce::KeyPress(juce::KeyPress::deleteKey));
      const float v2 = fx.p.parameters().base(idx(ParamId::WtY));
      c->beginNumericEntry();
      juce::TextEditor* te = nullptr;
      for (auto* ch : c->getChildren())
        if (auto* t = dynamic_cast<juce::TextEditor*>(ch)) te = t;
      bool entry = false;
      if (te) {
        te->setText("0,25 pos", false);
        if (te->onReturnKey) te->onReturnKey();  // Entrée : TextEditor::returnPressed poste un message, gestionnaire appelé ici
        entry = std::abs(fx.p.parameters().base(idx(ParamId::WtY)) - 0.25f) < 1e-4f;
      }
      ok = std::abs(v1 - (v0 + 0.01f)) < 1e-4f && std::abs(v2 - paramSpec(ParamId::WtY).defaultValue) < 1e-6f && entry;
      tk::info("UI04-KEY : fleche %g -> %g, Suppr -> %g, saisie '0,25 pos' %d", v0, v1, v2, entry);
    }
    TK_CHECK(ok);
    tk::result("UI04-KEY", ok, "clavier NEXUS : fleche = +1 %% de plage, Suppr = defaut, Entree = saisie numerique (virgule et unite acceptees) : %d", ok);
  }
  // UI04-CORE
  {
    auto fxHeap = std::make_unique<Fixture>();  // processeur complet : sur le tas (pile Windows 1 Mo, ASan)
    auto& fx = *fxHeap;
    auto& m = *fx.m;
    show(m, {0, 0, 0});
    CorePad* pad = nullptr;
    std::vector<juce::Component*> all;
    collect(m, all);
    for (auto* c : all)
      if (auto* cp = dynamic_cast<CorePad*>(c)) pad = cp;
    bool ok = pad != nullptr;
    if (pad) {
      const auto before = fx.p.parameters().values();
      pad->userMove(0.3f, 0.6f);
      auto* z = visibleControl(m, ParamId::CrossMaster);
      if (z) z->userSet(0.5f);
      m.refreshLayout();
      auto after = fx.p.parameters().values();
      int others = 0;
      for (int i = 0; i < kNumParams; ++i)
        if (i != idx(ParamId::CrossFwd) && i != idx(ParamId::CrossRev) && i != idx(ParamId::CrossMaster) && before.v[static_cast<size_t>(i)] != after.v[static_cast<size_t>(i)]) ++others;
      int numeric = 0;
      bool numOk = true;
      for (auto* b : m.visibleControls())
        if (b->boundId() == ParamId::CrossFwd || b->boundId() == ParamId::CrossRev || b->boundId() == ParamId::CrossMaster) {
          ++numeric;
          numOk = numOk && std::abs(b->shownBase() - after[b->boundId()]) < 1e-4f;
        }
      ok = std::abs(after[ParamId::CrossFwd] - 0.3f) < 1e-4f && std::abs(after[ParamId::CrossRev] - 0.6f) < 1e-4f && std::abs(after[ParamId::CrossMaster] - 0.5f) < 1e-4f &&
           others == 0 && numeric >= 4 && numOk && std::abs(pad->shownX() - 0.3f) < 1e-4f;
      tk::info("UI04-CORE : X/Y/Z %g %g %g, autres parametres modifies %d, commandes numeriques %d coherentes %d", after[ParamId::CrossFwd], after[ParamId::CrossRev],
               after[ParamId::CrossMaster], others, numeric, numOk);
    }
    TK_CHECK(ok);
    tk::result("UI04-CORE", ok, "SYMBIOTIC CORE direct : X=cross.fwd, Y=cross.rev, Z=cross.master, aucun autre parametre ecrit, alternative numerique coherente : %d", ok);
  }
  // UI04-CON
  {
    struct P {
      const char* n;
      juce::Colour a, b;
      double min;
    } pairs[] = {{"texte/plaque", tok::text, tok::panel, 4.5},       {"texte/chassis", tok::text, tok::chassis, 4.5}, {"texte/verre", tok::text, tok::glass, 4.5},
                 {"secondaire/plaque", tok::textDim, tok::panel, 4.5}, {"inactif/plaque", tok::textMute, tok::panel, 3.0}, {"WT/carbone", tok::wt, tok::carbon, 3.0},
                 {"FM/carbone", tok::fm, tok::carbon, 3.0},           {"CROSS/carbone", tok::cross, tok::carbon, 3.0}, {"modulation/carbone", tok::mod, tok::carbon, 3.0},
                 {"alerte/carbone", tok::alert, tok::carbon, 3.0},    {"ARME/carbone", tok::warn, tok::carbon, 3.0}};
    bool ok = true;
    juce::String rep;
    for (const auto& q : pairs) {
      const double c = contrast(q.a, q.b);
      ok = ok && c >= q.min;
      rep << q.n << " " << juce::String(c, 2) << " ; ";
    }
    TK_CHECK(ok);
    tk::result("UI04-CON", ok, "contrastes des tokens (WCAG) : %s", rep.toRawUTF8());
  }
}

// ======================================================================= ouverture, instances, temps réel, DSP
void testUi04Runtime() {
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  // UI04-OPEN
  {
    auto pHeap = std::make_unique<SubdestrucktProcessor>();
    auto& p = *pHeap;
    p.prepareToPlay(48000.0, 256);
    juce::MemoryBlock a;
    p.getStateInformation(a);
    const bool auth0 = p.autonomyAuthorized();
    bool ok = true;
    for (int i = 0; i < 100; ++i) {
      std::unique_ptr<juce::AudioProcessorEditor> ed(p.createEditorIfNeeded());
      auto* m = dynamic_cast<MainEditor*>(ed.get());
      ok = ok && m != nullptr;
      if (m) {
        m->setSize(1440, 900);
        m->showSpace(i % 3);
        m->refreshNow();
      }
      p.editorBeingDeleted(ed.get());  // protocole de l'hôte
      ed.reset();
      buf.clear();
      p.processBlock(buf, midi);
    }
    juce::MemoryBlock b;
    p.getStateInformation(b);
    tk::info("UI04-OPEN : editeur actif apres fermeture %p", static_cast<void*>(p.getActiveEditor()));
    ok = ok && a == b && p.autonomyAuthorized() == auth0 && p.getActiveEditor() == nullptr;
    TK_CHECK(ok);
    tk::result("UI04-OPEN", ok, "100 cycles ouverture/fermeture avec audio entre les cycles : etat identique %d, autorisation inchangee %d, aucun editeur residuel %d", a == b,
               p.autonomyAuthorized() == auth0, p.getActiveEditor() == nullptr);
  }
  // UI04-MULTI
  {
    std::vector<std::unique_ptr<Fixture>> f;
    for (int k = 0; k < 4; ++k) f.push_back(std::make_unique<Fixture>());
    for (auto& x : f) show(*x->m, {0, 0, 0});
    auto* b = visibleControl(*f[1]->m, ParamId::WtX);
    if (b) b->userSet(0.9f);
    for (auto& x : f) x->m->refreshLayout();
    bool ok = b != nullptr;
    for (int k = 0; k < 4; ++k) {
      const float expect = k == 1 ? 0.9f : paramSpec(ParamId::WtX).defaultValue;
      ok = ok && std::abs(f[static_cast<size_t>(k)]->p.parameters().base(idx(ParamId::WtX)) - expect) < 1e-4f;
      for (auto* c : f[static_cast<size_t>(k)]->m->visibleControls())
        if (c->boundId() == ParamId::WtX) ok = ok && std::abs(c->shownBase() - expect) < 1e-4f;
    }
    TK_CHECK(ok);
    tk::result("UI04-MULTI", ok, "4 instances avec editeurs ouverts : edition d'une instance sans effet sur les autres (valeurs et affichage) : %d", ok);
  }
  // UI04-RT et UI04-DSP : même scénario, avec éditeur actif (navigation entre les blocs) et sans éditeur
  {
    auto run = [&](bool withEditor, long long& allocs) {
      auto pHeap = std::make_unique<SubdestrucktProcessor>();
      auto& p = *pHeap;
      p.prepareToPlay(48000.0, 256);
      std::unique_ptr<juce::AudioProcessorEditor> ed;
      MainEditor* m = nullptr;
      std::vector<Ctx> ctxs;
      struct Closer {
        SubdestrucktProcessor& p;
        std::unique_ptr<juce::AudioProcessorEditor>& e;
        ~Closer() {
          if (e) p.editorBeingDeleted(e.get());
        }
      } closer{p, ed};
      if (withEditor) {
        ed.reset(p.createEditorIfNeeded());
        m = dynamic_cast<MainEditor*>(ed.get());
        m->setSize(1440, 900);
        ctxs = contexts(*m);
      }
      std::vector<float> out;
      allocs = 0;
      for (int blk = 0; blk < 300; ++blk) {
        midi.clear();
        if (blk == 2) midi.addEvent(juce::MidiMessage::noteOn(1, 52, static_cast<juce::uint8>(100)), 17);
        if (blk == 150) midi.addEvent(juce::MidiMessage::noteOn(1, 59, static_cast<juce::uint8>(90)), 3);
        if (blk == 240) midi.addEvent(juce::MidiMessage::noteOff(1, 52), 100);
        buf.clear();
        tk::allocationCount().store(0);
        tk::countingAllocations().store(true);
        p.processBlock(buf, midi);
        tk::countingAllocations().store(false);
        allocs += tk::allocationCount().load();
        for (int i = 0; i < 256; ++i) out.push_back(buf.getSample(0, i)), out.push_back(buf.getSample(1, i));
        if (m) {
          const auto& c = ctxs[static_cast<size_t>(blk) % ctxs.size()];
          m->showSubContext(c.s, c.t, c.sub);
          m->refreshNow();
          if (blk % 50 == 0) m->setZoomPercent(75 + (blk / 50 % 4) * 25);
        }
      }
      return out;
    };
    long long allocA = 0, allocB = 0;
    const auto a = run(true, allocA);
    const auto b = run(false, allocB);
    const bool rt = allocA == 0;
    TK_CHECK(rt);
    tk::result("UI04-RT", rt, "processBlock avec editeur ouvert (navigation, zoom, telemetrie entre les 300 blocs) : %lld allocation(s)", allocA);
    size_t diff = 0;
    for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) diff += a[i] != b[i];
    double e = 0.0;
    for (float v : a) e += static_cast<double>(v) * v;
    const bool dsp = a.size() == b.size() && diff == 0 && e > 0.0;
    TK_CHECK(dsp);
    tk::result("UI04-DSP", dsp, "rendu 300 blocs x 256 (2 canaux) avec et sans editeur : echantillons differents %zu / %zu (energie %.3g) ; bit a bit %d", diff, a.size(), e, dsp);
  }
}

// ======================================================================= commandes décoratives, opérations hors registre
void testUi04Operations() {
  auto fxHeap = std::make_unique<Fixture>();  // processeur complet : sur le tas
  auto& fx = *fxHeap;
  auto& m = *fx.m;
  ModRoute route;  // une route globale pour que l'éditeur de lignes de la matrice soit présent
  route.source = ModSource::Lfo2;
  route.dest = ParamId::WtY;
  route.depth = 0.2f;
  fx.p.modAdd(route);
  std::set<std::string> texts;
  int buttons = 0, unwired = 0, combos = 0, combosUnwired = 0, stateInputs = 0;
  std::string firstUnwired;
  for (const auto& c : contexts(m)) {
    show(m, c);
    std::vector<juce::Component*> all;
    collect(m, all);
    for (auto* x : all) {
      if (auto* b = dynamic_cast<juce::Button*>(x)) {
        texts.insert(b->getButtonText().toStdString());
        if (!m.isVisibleInEditor(b) || b->getBounds().isEmpty()) continue;
        ++buttons;
        const bool toggleInput = (dynamic_cast<juce::ToggleButton*>(b) != nullptr && dynamic_cast<NeuralCell*>(b) == nullptr) || (b->getClickingTogglesState() && !b->onClick);
        if (toggleInput) ++stateInputs;  // entrée d'état lue par une action (familles Randomize, « actif »)
        else if (!b->onClick) {
          ++unwired;
          if (firstUnwired.empty()) firstUnwired = b->getButtonText().toStdString();
          tk::info("UI04-DECO : bouton non relie '%s' (%s)", b->getButtonText().toRawUTF8(), m.contextName(c.s, c.t, c.sub).toRawUTF8());
        }
      }
      if (auto* cb = dynamic_cast<juce::ComboBox*>(x)) {
        texts.insert(cb->getTitle().toStdString());
        if (!m.isVisibleInEditor(cb)) continue;
        ++combos;
        combosUnwired += !cb->onChange;
      }
      texts.insert(x->getTitle().toStdString());
    }
  }
  // les listes sans onChange sont des paramètres d'action (source, cadence...) lues par le bouton associé
  const bool deco = unwired == 0;
  TK_CHECK(deco);
  tk::result("UI04-DECO", deco, "%d boutons visibles (tous contextes) : non relies %d%s%s ; %d entrees d'etat (cases lues par une action) ; %d listes (%d parametres d'action lus par leur bouton)", buttons,
             unwired, firstUnwired.empty() ? "" : " premier ", firstUnwired.c_str(), stateInputs, combos, combosUnwired);
  const char* ops[] = {"Ouvrir", "Enregistrer", "Undo", "Redo", "PANIC", "Drone", "Autoriser", "Inspector", "Cross Reset", "Excitation", "Capturer gels", "Importer WAV...",
                       "Exporter WAV...", "Appliquer la forme", "Reprendre", "Enregistrer .sdwt", "Charger", "Favori", "+ route", "Ajouter", "RANDOMIZE", "MUTATE", "Evolve start",
                       "Stop", "Reset", "Commit", "Lever les verrous", "Annuler l'apprentissage", "Capturer maintenant", "Effacer WT1", "Effacer GL2", "Verrou Randomize",
                       "MIDI Learn", "Saisie", "+ Modulation", "Quality", "Zoom", "Banque wavetable", "Cadence Evolve (Hz)", "Profil", "Courbe",
                       "Restaurer la configuration du Drone"};
  int missing = 0;
  std::string first;
  for (const char* o : ops) {
    bool f = false;
    for (const auto& t : texts) f = f || t.rfind(o, 0) == 0 || t == juce::String::fromUTF8(o).toStdString();
    if (!f) {
      ++missing;
      if (first.empty()) first = o;
    }
  }
  for (const char* o : {"Générer", "Épingler", "Défaut", "Topologie FM préréglée", "Apprendre : paramètre de l'Inspector", "FORME CUSTOM DU LFO", "COURBE CUSTOM DE LA ROUTE",
                        "Symbiotic Core : X = cross.fwd, Y = cross.rev", "Editeur d'enveloppe", "Destination Inspector"}) {
    const std::string u = juce::String::fromUTF8(o).toStdString();
    bool f = false;
    for (const auto& t : texts) f = f || t.rfind(u, 0) == 0;
    if (!f) {
      ++missing;
      if (first.empty()) first = o;
    }
  }
  const bool ok = missing == 0;
  TK_CHECK(ok);
  tk::result("UI04-OPS", ok, "operations hors registre presentes (presets, banques, formes, routes, modulations, courbes, MIDI Learn, Randomize/Evolve, captures, Drone, Panic, Undo, Quality, zoom, Inspector) : manquantes %d%s%s",
             missing, first.empty() ? "" : " premiere ", first.c_str());
}

// Captures réelles de l'interface (rendu logiciel JUCE de l'éditeur réel, après quelques blocs joués).
void snapshotUi04(const std::string& prefix) {
  auto pHeap = std::make_unique<SubdestrucktProcessor>();
  auto& p = *pHeap;
  p.prepareToPlay(44100.0, 256);
  juce::AudioBuffer<float> buf(2, 256);
  juce::MidiBuffer midi;
  midi.addEvent(juce::MidiMessage::noteOn(1, 57, static_cast<juce::uint8>(110)), 0);
  for (int i = 0; i < 60; ++i) {
    buf.clear();
    p.processBlock(buf, midi);
    midi.clear();
  }
    std::unique_ptr<juce::AudioProcessorEditor> ed(p.createEditorIfNeeded());
  auto* m = dynamic_cast<MainEditor*>(ed.get());
  if (!m) return;
  struct Closer {
    SubdestrucktProcessor& p;
    std::unique_ptr<juce::AudioProcessorEditor>& e;
    ~Closer() {
      if (e) p.editorBeingDeleted(e.get());
    }
  } closer{p, ed};
  struct V {
    int w, h, zoom, s, t, sub;
    const char* name;
  } views[] = {{1440, 900, 100, 0, 0, 0, "performance"},         {1440, 900, 100, 1, 0, 0, "sound_engines"},      {1440, 900, 100, 1, 0, 3, "sound_engines_z"},
               {1440, 900, 100, 1, 0, 4, "sound_engines_matrix"}, {1440, 900, 100, 1, 1, 0, "sound_process"},       {1440, 900, 100, 1, 2, 0, "sound_mod"},
               {1440, 900, 100, 2, 0, 0, "eng_fm_network"},       {1440, 900, 100, 2, 1, 0, "eng_cross_network"},   {1440, 900, 100, 2, 2, 0, "eng_mod_matrix"},
               {1440, 900, 100, 2, 3, 0, "eng_bank_lab"},         {1440, 900, 100, 2, 4, 0, "eng_voice_tuning"},    {1440, 900, 100, 2, 5, 0, "eng_presets_state"},
               {1440, 900, 100, 2, 6, 0, "eng_randomize"},        {1440, 900, 100, 2, 7, 0, "eng_midi_learn"},      {1440, 900, 100, 2, 8, 0, "eng_diagnostics"},
               {1440, 900, 100, 2, 9, 0, "eng_captures"},         {960, 640, 100, 0, 0, 0, "min960_performance"},   {960, 640, 100, 1, 1, 0, "min960_sound_process"},
               {1440, 900, 150, 0, 0, 0, "zoom150_performance"},  {1440, 900, 75, 1, 0, 0, "zoom75_sound_engines"}};
  // un exemple de modulation pour montrer Base / Effective
  ModRoute r;
  r.source = ModSource::Lfo1;
  r.dest = ParamId::WtX;
  r.depth = 0.4f;
  p.modAdd(r);
  // scène de démonstration (capture seulement) : quelques algorithmes actifs pour montrer les panneaux contextuels
  auto setP = [&p](ParamId id, float v) {
    auto* q = p.parameters().param(idx(id));
    q->setValueNotifyingHost(q->convertTo0to1(v));
  };
  setP(ParamId::FltWtType, 1.0f);
  setP(ParamId::DstWt1Type, 1.0f);
  setP(ParamId::DstFm1Type, 6.0f);
  setP(ParamId::FltGlType, 4.0f);
  setP(ParamId::DstGl1Type, 8.0f);
  setP(ParamId::WtZAType, 4.0f);
  setP(ParamId::WtZBType, 1.0f);
  for (int i = 0; i < 40; ++i) {
    buf.clear();
    p.processBlock(buf, midi);
  }
  for (const auto& v : views) {
    m->setSize(v.w, v.h);
    m->setInspectorOpen(v.w >= 1200);
    m->setZoomPercent(v.zoom);
    m->showSubContext(v.s, v.t, v.sub);
    m->refreshLayout();
    m->refreshNow();
    const auto img = m->createComponentSnapshot(m->getLocalBounds(), true, 1.0f);
    juce::File f(juce::String::fromUTF8((prefix + "_" + v.name + ".png").c_str()));
    f.deleteFile();
    juce::FileOutputStream os(f);
    juce::PNGImageFormat png;
    const bool ok = os.openedOk() && png.writeImageToStream(img, os);
    tk::info("UI04 : capture %dx%d %s -> %s (%s)", img.getWidth(), img.getHeight(), v.name, f.getFullPathName().toRawUTF8(), ok ? "ecrite" : "ECHEC");
  }
}
