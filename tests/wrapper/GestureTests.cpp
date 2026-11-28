// SUBDESTRUCKT — ARCH-UI-04-R2-B2-01 : reproduction des défauts de geste PB-01 / PB-02 (OD-10) et contrôle de prise en main
// des PLASMA (DB2-02), exécutés par subdestruckt_wrapper_tests. Tests PERMANENTS (CE-B2-01) : ils ne sont jamais retirés ;
// après une correction autorisée, l'assertion du scénario devient le critère BE (docs/ARCH-UI-04-R2/B2-PRODUCTION-INTEGRATION).
//
// Méthode (enregistrée avant mesure, B2-TEST-PLAN.md §2-3 ; banc précisé dans PB/README.md) :
//  - l'éditeur réel est placé dans un composant hôte de test, posé sur le bureau avec un PAIR DE TEST (TestPeer) qui remplace
//    seulement la couche fenêtre de l'OS (empilement des fenêtres, rendu à l'écran). Les événements souris sont injectés par
//    ComponentPeer::handleMouseEvent / handleMouseWheel : tout le dispatch est celui de JUCE 9.0.6 (MouseInputSource :
//    comptage des clics, ordre mouseDown / mouseUp / mouseDoubleClick, ciblage, contrôles modaux) ;
//  - un AudioProcessorListener compte, par paramètre, les begin/end de geste, la profondeur courante et maximale, et les
//    écritures hors geste ; un MouseListener journalise l'ordre des événements effectivement reçus par le composant ;
//  - les transactions Undo sont comptées par l'API publique (undo() jusqu'à épuisement), sur un processeur neuf ; les valeurs
//    de départ sont posées par l'hôte (setValueNotifyingHost, hors geste : aucune transaction Undo) ;
//  - aucun scénario ne vaut sans preuve que l'action a atteint le composant (événements « ui » au journal) : sinon BLOCKED.
// Statuts : REPRODUIT, NON REPRODUIT, BLOCKED (jamais un PASS sur lecture statique). Critère de chaque RESULT :
//  le scénario est exécuté par le dispatch JUCE réel sans plantage et ses observations sont consignées ; pour les scénarios où
//  0.4.1 doit déjà satisfaire BE-01..BE-05, l'assertion BE est exigée (non-régression).
// Journaux : si la variable d'environnement SBD_B2_OUT désigne un dossier, pb-events.csv, pb-summary.csv et pb-platform.txt
// y sont écrits.
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "TestKit.h"
#include "support/HeadlessPeer.h"
#include "plugin/PluginProcessor.h"
#include "ui/CurveEditors.h"
#include "ui/MainEditor.h"

using namespace sbd;
using namespace sbd::ui;

namespace {

// ---------------------------------------------------------------- journal commun
struct Journal {
  std::vector<std::string> events;   // scenario;seq;source;event;param;value;depth
  std::vector<std::string> summary;  // une ligne par scénario
  std::string scenario;
  int seq = 0;
  void add(const std::string& src, const std::string& ev, const std::string& param, double value, int depth) {
    char b[512];
    std::snprintf(b, sizeof b, "%s;%d;%s;%s;%s;%.6f;%d", scenario.c_str(), ++seq, src.c_str(), ev.c_str(), param.c_str(), value, depth);
    events.push_back(b);
  }
  int uiEvents(const std::string& scen) const {
    int n = 0;
    for (auto& e : events)
      if (e.rfind(scen + ";", 0) == 0 && e.find(";ui;") != std::string::npos) ++n;
    return n;
  }
} gJournal;

using sbd::testsupport::TestHost;  // pair de test commun : tests/support/HeadlessPeer.h

// ---------------------------------------------------------------- sonde de geste (thread message)
struct Probe : juce::AudioProcessorListener {
  juce::AudioProcessor& p;
  std::map<int, int> depth, maxDepth, begins, ends, writes, outside;
  bool userPhase = true;  // les écritures hors geste ne comptent que pendant les actions utilisateur simulées
  explicit Probe(juce::AudioProcessor& proc) : p(proc) { p.addListener(this); }
  ~Probe() override { p.removeListener(this); }
  std::string id(int i) const {
    if (auto* w = dynamic_cast<juce::AudioProcessorParameterWithID*>(p.getParameters()[i])) return w->paramID.toStdString();
    return std::to_string(i);
  }
  void audioProcessorParameterChanged(juce::AudioProcessor*, int i, float v) override {
    ++writes[i];
    if (depth[i] == 0 && userPhase) ++outside[i];
    gJournal.add("host", "write", id(i), v, depth[i]);
  }
  void audioProcessorChanged(juce::AudioProcessor*, const ChangeDetails&) override {}
  void audioProcessorParameterChangeGestureBegin(juce::AudioProcessor*, int i) override {
    ++begins[i];
    maxDepth[i] = std::max(maxDepth[i], ++depth[i]);
    gJournal.add("host", "begin", id(i), 0.0, depth[i]);
  }
  void audioProcessorParameterChangeGestureEnd(juce::AudioProcessor*, int i) override {
    ++ends[i];
    --depth[i];
    gJournal.add("host", "end", id(i), 0.0, depth[i]);
  }
  int maxD() const {
    int m = 0;
    for (auto& kv : maxDepth) m = std::max(m, kv.second);
    return m;
  }
  int open() const {  // gestes résiduels (begin non refermés), tous paramètres
    int n = 0;
    for (auto& kv : begins) n += std::max(0, kv.second - (ends.count(kv.first) ? ends.at(kv.first) : 0));
    return n;
  }
  static int total(const std::map<int, int>& m) {
    int n = 0;
    for (auto& kv : m) n += kv.second;
    return n;
  }
};

// ---------------------------------------------------------------- journal des événements reçus par un composant
struct UiLog : juce::MouseListener {
  std::string name;
  explicit UiLog(std::string n) : name(std::move(n)) {}
  void mouseDown(const juce::MouseEvent& e) override { gJournal.add("ui", "mouseDown clicks=" + std::to_string(e.getNumberOfClicks()), name, 0.0, 0); }
  void mouseUp(const juce::MouseEvent& e) override { gJournal.add("ui", "mouseUp clicks=" + std::to_string(e.getNumberOfClicks()), name, 0.0, 0); }
  void mouseDoubleClick(const juce::MouseEvent&) override { gJournal.add("ui", "mouseDoubleClick", name, 0.0, 0); }
  void mouseDrag(const juce::MouseEvent&) override { gJournal.add("ui", "mouseDrag", name, 0.0, 0); }
  void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override { gJournal.add("ui", "mouseWheelMove", name, 0.0, 0); }
};

// ---------------------------------------------------------------- éditeur réel dans un hôte de test + injection
struct Rig {
  std::unique_ptr<SubdestrucktProcessor> p = std::make_unique<SubdestrucktProcessor>();
  std::unique_ptr<TestHost> host = std::make_unique<TestHost>();
  std::unique_ptr<juce::AudioProcessorEditor> ed;
  MainEditor* m = nullptr;
  juce::ComponentPeer* peer = nullptr;
  juce::int64 t = 0;
  explicit Rig(int x = 40, int w = 1440, int h = 900) {
    p->prepareToPlay(48000.0, 256);
    ed.reset(p->createEditorIfNeeded());
    m = dynamic_cast<MainEditor*>(ed.get());
    if (m == nullptr) return;
    host->setBounds(x, 40, w, h);
    m->setBounds(0, 0, w, h);
    host->addAndMakeVisible(*m);
    host->setVisible(true);
    host->addToDesktop(0);
    peer = host->getPeer();
    t = juce::Time::currentTimeMillis();
  }
  ~Rig() {
    destroyEditor();
    host->removeFromDesktop();
  }
  void destroyEditor() {
    if (ed) {
      p->editorBeingDeleted(ed.get());  // protocole de l'hôte (wrapper VST3)
      ed.reset();
      m = nullptr;
    }
  }
  bool ok() const { return m != nullptr && peer != nullptr; }
  juce::Point<float> toPeer(juce::Component& c, juce::Point<float> local) const { return peer->globalToLocal(c.localPointToGlobal(local)); }
  void at(juce::Point<float> peerPos, juce::ModifierKeys mods, int dtMs = 40) {
    t += dtMs;
    // touches clavier : JUCE les lit dans ModifierKeys::currentModifiers (tenu à jour par les pairs natifs depuis le clavier
    // de l'OS) et ne garde que les boutons dans l'événement souris injecté (MouseInputSource::handleEvent)
    juce::ModifierKeys::currentModifiers = mods.withoutMouseButtons();
    peer->handleMouseEvent(juce::MouseInputSource::InputSourceType::mouse, peerPos, mods, juce::MouseInputSource::defaultPressure,
                           juce::MouseInputSource::defaultOrientation, t);
  }
  void mouse(juce::Component& c, juce::Point<float> local, juce::ModifierKeys mods, int dtMs = 40) { at(toPeer(c, local), mods, dtMs); }
  void move(juce::Component& c, juce::Point<float> l) { mouse(c, l, juce::ModifierKeys()); }
  void down(juce::Component& c, juce::Point<float> l, int extra = 0) { mouse(c, l, juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier | extra)); }
  void drag(juce::Component& c, juce::Point<float> l, int extra = 0) { down(c, l, extra); }
  void up(juce::Component& c, juce::Point<float> l, int extra = 0) { mouse(c, l, juce::ModifierKeys(extra)); }
  void wheel(juce::Component& c, juce::Point<float> l, float dy) {
    t += 20;
    juce::MouseWheelDetails w{};
    w.deltaY = dy;
    peer->handleMouseWheel(juce::MouseInputSource::InputSourceType::mouse, toPeer(c, l), t, w);
  }
  void pause() { t += 800; }  // au-delà du délai de double-clic : aucun regroupement avec l'action suivante
  // dispatch JUCE réel d'un double-clic : down, up, down (2 clics), up -> mouseDoubleClick émis dans internalMouseUp
  void doubleClick(juce::Component& c, juce::Point<float> l) {
    move(c, l);
    down(c, l);
    up(c, l);
    down(c, l);
    up(c, l);
  }
  float base(ParamId id) const { return p->parameters().base(idx(id)); }
  void hostSet(ParamId id, float plain) {  // valeur de départ posée par l'hôte (aucune transaction Undo)
    if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(p->getParameters()[idx(id)])) rp->setValueNotifyingHost(rp->convertTo0to1(plain));
    if (m != nullptr) m->refreshNow();
  }
  int countUndo() {  // transactions Undo produites depuis la création
    int n = 0;
    while (n < 400 && p->undo()) ++n;
    return n;
  }
};

void collect(juce::Component& c, std::vector<juce::Component*>& out) {
  for (auto* ch : c.getChildren()) {
    out.push_back(ch);
    collect(*ch, out);
  }
}

ContinuousControl* findControl(MainEditor& m, ParamId id) {
  for (auto* b : m.visibleControls())
    if (b->boundId() == id)
      if (auto* cc = dynamic_cast<ContinuousControl*>(&b->asComponent())) return cc;
  return nullptr;
}

template <class T>
T* findShowing(juce::Component& root) {
  std::vector<juce::Component*> all;
  collect(root, all);
  for (auto* c : all)
    if (auto* x = dynamic_cast<T*>(c))
      if (c->isShowing()) return x;
  return nullptr;
}

juce::Point<float> centre(juce::Component& c) { return {c.getWidth() * 0.5f, c.getHeight() * 0.5f}; }

// Conséquence d'un geste resté ouvert : après destruction de l'éditeur, un nouvel éditeur effectue UNE édition complète de
// wt.x ; renvoie le nombre de transactions Undo produites par cette seule édition (1 attendu sans défaut).
int undoAfterReopen(SubdestrucktProcessor& p) {
  int before = 0;
  {
    std::unique_ptr<juce::AudioProcessorEditor> e2(p.createEditorIfNeeded());
    auto* m2 = dynamic_cast<MainEditor*>(e2.get());
    if (m2 == nullptr) return -1;
    m2->setSize(1440, 900);
    m2->showSpace(0);
    m2->refreshLayout();
    ContinuousControl* c = nullptr;
    for (auto* b : m2->visibleControls())
      if (b->boundId() == ParamId::WtX) c = dynamic_cast<ContinuousControl*>(&b->asComponent());
    if (c == nullptr) {
      p.editorBeingDeleted(e2.get());
      return -1;
    }
    while (before < 400 && p.undo()) ++before;  // historique vidé jusqu'à l'origine
    for (int k = 0; k < before; ++k) p.redo();
    c->userSet(c->shownBase() > 0.5f ? 0.2f : 0.9f);
    p.editorBeingDeleted(e2.get());
  }
  int after = 0;
  while (after < 400 && p.undo()) ++after;
  return after - before;
}

void summarize(const std::string& id, const Probe* pr, const std::string& status, const std::string& detail, int undo, double value, double expected) {
  char b[1400];
  std::snprintf(b, sizeof b, "%s;%s;%d;%d;%d;%d;%d;%d;%.6f;%.6f;%s", id.c_str(), status.c_str(), pr ? Probe::total(pr->begins) : -1,
                pr ? Probe::total(pr->ends) : -1, pr ? pr->maxD() : -1, pr ? pr->open() : -1, pr ? Probe::total(pr->outside) : -1, undo, value,
                expected, detail.c_str());
  gJournal.summary.push_back(b);
}

void blocked(const char* id, const std::string& why) {
  summarize(id, nullptr, "BLOCKED", why, -1, 0, 0);
  tk::result(id, false, "[REPRODUCTION] BLOCKED : %s", why.c_str());
}

bool near(double a, double b, double span) { return std::abs(a - b) <= 1e-4 * std::max(1.0, span); }

// ---------------------------------------------------------------- PB-01 : double-clic hors zone de valeur
// PB-01a NEXUS rotatif wt.x ; PB-01b PLASMA vertical out.gain. Départ hors défaut (sinon le retour au défaut ne fait rien).
// valeurs écrites pour un paramètre pendant un scénario (journal de la sonde)
std::vector<double> writesOf(const std::string& scen, const std::string& param) {
  std::vector<double> v;
  for (auto& e : gJournal.events)
    if (e.rfind(scen + ";", 0) == 0 && e.find(";host;write;" + param + ";") != std::string::npos) {
      // scenario;seq;host;write;param;value;depth : 6e champ
      size_t pos = 0;
      for (int k = 0; k < 5; ++k) pos = e.find(';', pos) + 1;
      v.push_back(std::atof(e.substr(pos, e.find(';', pos) - pos).c_str()));
    }
  return v;
}

void runDoubleClick(const char* id, ParamId pid, float start) {
  gJournal.scenario = id;
  Rig r;
  if (!r.ok()) return blocked(id, "pair de test indisponible");
  r.m->showSpace(0);
  r.m->refreshLayout();
  auto* c = findControl(*r.m, pid);
  if (c == nullptr) return blocked(id, std::string("commande introuvable : ") + paramSpec(pid).stableId);
  r.hostSet(pid, start);
  UiLog log(paramSpec(pid).stableId);
  c->addMouseListener(&log, false);
  Probe pr(*r.p);
  r.doubleClick(*c, centre(*c));
  c->removeMouseListener(&log);
  pr.userPhase = false;
  const auto& s = paramSpec(pid);
  const double v = r.base(pid);
  const bool atDefault = near(v, s.defaultValue, s.maxValue - s.minValue);
  const int ui = gJournal.uiEvents(id), maxd = pr.maxD(), open = pr.open();
  const auto w = writesOf(id, s.stableId);  // écritures de la phase utilisateur (avant les Undo du comptage)
  r.destroyEditor();
  const int undo = r.countUndo();
  if (ui == 0) return blocked(id, "aucun evenement recu par le composant");
  const bool nested = maxd > 1;
  const std::string status = nested || open > 0 ? "REPRODUIT" : "NON REPRODUIT";
  const std::string detail = std::string("double-clic dispatche par JUCE ; ") + std::to_string(ui) + " evenements ui ; valeur finale " +
                             (atDefault ? "= defaut" : "!= defaut");
  summarize(id, &pr, status, detail, undo, v, s.defaultValue);
  const bool be = !nested && open == 0 && Probe::total(pr.outside) == 0 && Probe::total(pr.begins) == Probe::total(pr.ends);
  if (c->form() == Form::Rotary) {  // NEXUS : double-clic = défaut (contrat inchangé)
    tk::result(id, be && atDefault,
               "[PERMANENT, dispatch JUCE reel] %s ; double-clic hors valeur sur %s (depart %.3f) : %d evenements ui, begin %d / end %d, "
               "profondeur max %d, residuel %d, hors geste %d, transactions Undo %d, valeur finale = defaut : %d",
               status.c_str(), s.stableId, start, ui, Probe::total(pr.begins), Probe::total(pr.ends), maxd, open, Probe::total(pr.outside), undo,
               atDefault);
    return;
  }
  // PLASMA, DB2-09 (B) : scénario historique conservé, assertion corrigée : position absolue au premier clic, aucun retour
  // au défaut, aucune valeur intermédiaire, une seule écriture, une seule transaction Undo (preuve avant : PB/*/pb-summary.csv)
  bool defWritten = false;
  for (double x : w) defWritten = defWritten || near(x * (s.maxValue - s.minValue) + s.minValue, s.defaultValue, s.maxValue - s.minValue);
  const bool ok = be && !atDefault && w.size() == 1 && !defWritten && undo == 1;
  tk::result(id, ok,
             "[PERMANENT DB2-09, dispatch JUCE reel] %s ; double-clic dans le canal de %s (depart %.3f) : %d evenements ui, begin %d / end %d, "
             "profondeur max %d, residuel %d, hors geste %d, ecritures %zu, defaut ecrit %d, transactions Undo %d (attendu : 1 ecriture, 0, 1)",
             status.c_str(), s.stableId, start, ui, Probe::total(pr.begins), Probe::total(pr.ends), maxd, open, Probe::total(pr.outside), w.size(),
             defWritten, undo);
}

// Séquence HYPOTHÉTIQUE de l'audit statique H-04 (down, up, down, mouseDoubleClick, up) par appel direct des gestionnaires :
// documente ce que donnerait cet ordre. Ce n'est PAS une reproduction : JUCE 9.0.6 n'émet pas cet ordre. Information seule.
void hypotheticalH04() {
  gJournal.scenario = "H04-HYPOTHESE";
  Rig r;
  if (!r.ok()) return;
  r.m->showSpace(0);
  r.m->refreshLayout();
  auto* c = findControl(*r.m, ParamId::WtX);
  if (c == nullptr) return;
  r.hostSet(ParamId::WtX, 0.8f);
  Probe pr(*r.p);
  auto src = juce::Desktop::getInstance().getMainMouseSource();
  const auto pt = centre(*c);
  const auto now = juce::Time::getCurrentTime();
  auto ev = [&](int clicks, juce::ModifierKeys mods) {
    return juce::MouseEvent(src, pt, mods, juce::MouseInputSource::defaultPressure, juce::MouseInputSource::defaultOrientation,
                            juce::MouseInputSource::defaultRotation, juce::MouseInputSource::defaultTiltX, juce::MouseInputSource::defaultTiltY, c, c,
                            now, pt, now, clicks, false);
  };
  const juce::ModifierKeys btn(juce::ModifierKeys::leftButtonModifier);
  c->mouseDown(ev(1, btn));
  c->mouseUp(ev(1, {}));
  c->mouseDown(ev(2, btn));
  c->mouseDoubleClick(ev(2, btn));
  c->mouseUp(ev(2, {}));
  pr.userPhase = false;
  summarize("H04-HYPOTHESE", &pr, "INFORMATION", "ordre suppose par H-04 (appel direct des gestionnaires), non emis par JUCE 9.0.6", -1,
            r.base(ParamId::WtX), paramSpec(ParamId::WtX).defaultValue);
  tk::info("H04-HYPOTHESE (appel direct, ordre non emis par JUCE) : profondeur max %d, begin %d, end %d", pr.maxD(), Probe::total(pr.begins),
           Probe::total(pr.ends));
}

// PB-01c : PLASMA horizontal perf.balance, double-clic sur la zone de valeur (ouverture de la saisie numérique)
void runLinearHValueDoubleClick() {
  const char* id = "PB-01c";
  gJournal.scenario = id;
  Rig r;
  if (!r.ok()) return blocked(id, "pair de test indisponible");
  r.m->showSpace(0);
  r.m->refreshLayout();
  auto* c = findControl(*r.m, ParamId::PerfBalance);
  if (c == nullptr) return blocked(id, "perf.balance introuvable");
  r.hostSet(ParamId::PerfBalance, 0.5f);
  const double v0 = r.base(ParamId::PerfBalance);
  UiLog log("perf.balance");
  c->addMouseListener(&log, false);
  Probe pr(*r.p);
  // zone de valeur d'un PLASMA horizontal : tiers droit de la commande (ContinuousControl::valueArea)
  r.doubleClick(*c, {c->getWidth() * 0.86f, c->getHeight() * 0.5f});
  c->removeMouseListener(&log);
  const double v = r.base(ParamId::PerfBalance);
  bool entry = false;
  for (auto* ch : c->getChildren()) entry = entry || dynamic_cast<juce::TextEditor*>(ch) != nullptr;
  pr.userPhase = false;
  const int ui = gJournal.uiEvents(id), maxd = pr.maxD(), open = pr.open();
  r.destroyEditor();
  const int undo = r.countUndo();
  if (ui == 0) return blocked(id, "aucun evenement recu par le composant");
  const bool jumped = std::abs(v - v0) > 1e-4;
  const std::string status = (maxd > 1 || open > 0 || jumped) ? "REPRODUIT" : "NON REPRODUIT";
  summarize(id, &pr, status, std::string(jumped ? "valeur modifiee par les clics avant la saisie (saut)" : "valeur inchangee") +
                                 (entry ? " ; saisie ouverte" : " ; saisie absente"),
            undo, v, v0);
  // DB2-08 : assertion corrigée (saut reproduit avant correction : PB/*/pb-summary.csv)
  const bool ok = !jumped && entry && Probe::total(pr.begins) == 0 && Probe::total(pr.writes) == 0 && open == 0 && undo == 0;
  tk::result(id, ok,
             "[PERMANENT DB2-08, dispatch JUCE reel] %s ; double-clic sur la valeur de perf.balance : %.4f -> %.4f, %d evenements ui, gestes %d, ecritures %d, "
             "residuel %d, saisie ouverte %d, transactions Undo %d (attendu : aucun saut, saisie, 0 geste, 0 ecriture)",
             status.c_str(), v0, v, ui, Probe::total(pr.begins), Probe::total(pr.writes), open, entry, undo);
}

// PB-01d : autres retours au défaut et saisie numérique (NEXUS wt.x)
void runDefaultPaths() {
  const char* id = "PB-01d";
  gJournal.scenario = id;
  Rig r;
  if (!r.ok()) return blocked(id, "pair de test indisponible");
  r.m->showSpace(0);
  r.m->refreshLayout();
  auto* c = findControl(*r.m, ParamId::WtX);
  if (c == nullptr) return blocked(id, "wt.x introuvable");
  const float def = paramSpec(ParamId::WtX).defaultValue;
  UiLog log("wt.x");
  c->addMouseListener(&log, false);
  Probe pr(*r.p);
  pr.userPhase = false;
  r.hostSet(ParamId::WtX, 0.8f);
  pr.userPhase = true;
  r.move(*c, centre(*c));
  r.down(*c, centre(*c), juce::ModifierKeys::altModifier);  // Alt+clic, dispatch réel
  r.up(*c, centre(*c), juce::ModifierKeys::altModifier);
  const bool alt = near(r.base(ParamId::WtX), def, 1.0);
  pr.userPhase = false;
  r.hostSet(ParamId::WtX, 0.8f);
  pr.userPhase = true;
  c->keyPressed(juce::KeyPress(juce::KeyPress::deleteKey));  // Suppr : gestionnaire clavier
  const bool del = near(r.base(ParamId::WtX), def, 1.0);
  pr.userPhase = false;
  r.hostSet(ParamId::WtX, 0.8f);
  pr.userPhase = true;
  c->resetToDefault();  // chemin du menu contextuel « Réinitialiser » (rappel asynchrone du menu)
  const bool menu = near(r.base(ParamId::WtX), def, 1.0);
  c->beginNumericEntry();
  bool entry = false;
  for (auto* ch : c->getChildren())
    if (auto* te = dynamic_cast<juce::TextEditor*>(ch)) {
      te->setText("0.3", false);
      if (te->onReturnKey) te->onReturnKey();  // la touche Entrée de JUCE passe par un message asynchrone : rappel direct
      entry = true;
    }
  const bool typed = near(r.base(ParamId::WtX), 0.3, 1.0);
  c->removeMouseListener(&log);
  pr.userPhase = false;
  const int ui = gJournal.uiEvents(id);
  const bool be = pr.maxD() <= 1 && pr.open() == 0 && Probe::total(pr.begins) == Probe::total(pr.ends) && Probe::total(pr.outside) == 0;
  const std::string status = be ? "NON REPRODUIT" : "REPRODUIT";
  r.destroyEditor();
  const int undo = r.countUndo();
  summarize(id, &pr, status, "Alt+clic " + std::to_string(alt) + ", Suppr " + std::to_string(del) + ", menu " + std::to_string(menu) +
                                 ", saisie " + std::to_string(entry && typed) + ", " + std::to_string(ui) + " evenements ui",
            undo, r.base(ParamId::WtX), 0.3);
  tk::result(id, be && alt && del && menu && entry && typed && ui > 0,
             "[REPRODUCTION] %s ; wt.x : Alt+clic (dispatch reel) %d, Suppr %d, menu defaut %d, saisie numerique %d ; begin %d / end %d, profondeur "
             "max %d, residuel %d, hors geste %d, transactions Undo %d (attendu 4)",
             status.c_str(), alt, del, menu, entry && typed, Probe::total(pr.begins), Probe::total(pr.ends), pr.maxD(), pr.open(),
             Probe::total(pr.outside), undo);
}

// PB-01e : molette (geste ouvert, fin différée de 450 ms) puis clic avant l'échéance
void runWheelThenClick() {
  const char* id = "PB-01e";
  gJournal.scenario = id;
  Rig r;
  if (!r.ok()) return blocked(id, "pair de test indisponible");
  r.m->showSpace(0);
  r.m->refreshLayout();
  auto* c = findControl(*r.m, ParamId::WtX);
  if (c == nullptr) return blocked(id, "wt.x introuvable");
  UiLog log("wt.x");
  c->addMouseListener(&log, false);
  Probe pr(*r.p);
  const auto pt = centre(*c);
  r.move(*c, pt);
  r.wheel(*c, pt, 0.2f);
  r.down(*c, pt);  // moins de 450 ms après la molette : la fin du geste molette n'a pas eu lieu
  const int depthAtClick = pr.maxD();
  r.drag(*c, pt + juce::Point<float>(0.0f, -11.0f));
  r.up(*c, pt + juce::Point<float>(0.0f, -11.0f));
  const int openAfterUp = pr.open();
  c->removeMouseListener(&log);
  // le minuteur de fin de geste molette exige la boucle de messages (absente du banc) : fermeture par destruction de l'éditeur
  r.destroyEditor();
  const int openAfterDestroy = pr.open();
  pr.userPhase = false;
  const int ui = gJournal.uiEvents(id);
  const int undo = r.countUndo();
  if (ui < 3) return blocked(id, "evenements ui insuffisants");
  const std::string status = depthAtClick > 1 ? "REPRODUIT" : "NON REPRODUIT";
  summarize(id, &pr, status, "profondeur au clic " + std::to_string(depthAtClick) + ", ouverts apres relachement " + std::to_string(openAfterUp) +
                                 ", apres destruction " + std::to_string(openAfterDestroy) + " (minuteur 450 ms remplace par la destruction)",
            undo, 0, 0);
  // CE-B2-01 : scénario permanent ; depuis la correction B2-03.1, assertion corrigée (le geste molette est refermé au clic) :
  // profondeur 1, aucun geste résiduel, deux transactions (molette, glisser). Preuve historique du défaut : PB/*/pb-summary.csv.
  const bool fixedOk = depthAtClick <= 1 && openAfterUp == 0 && openAfterDestroy == 0 && undo == 2;
  tk::result(id, fixedOk, "[PERMANENT, dispatch JUCE reel] %s ; molette puis clic avant 450 ms sur wt.x : %d evenements ui, profondeur max %d, ouverts apres "
             "relachement %d, apres destruction %d, transactions Undo %d (attendu : profondeur 1, 0, 0, 2)",
             status.c_str(), ui, depthAtClick, openAfterUp, openAfterDestroy, undo);
}

// PB-01f : molette puis touche clavier avant l'échéance (autre geste incompatible couvert par la correction de PB-01e)
void runWheelThenKey() {
  const char* id = "PB-01f";
  gJournal.scenario = id;
  Rig r;
  if (!r.ok()) return blocked(id, "pair de test indisponible");
  r.m->showSpace(0);
  r.m->refreshLayout();
  auto* c = findControl(*r.m, ParamId::WtX);
  if (c == nullptr) return blocked(id, "wt.x introuvable");
  UiLog log("wt.x");
  c->addMouseListener(&log, false);
  Probe pr(*r.p);
  const auto pt = centre(*c);
  r.move(*c, pt);
  r.wheel(*c, pt, 0.2f);
  c->keyPressed(juce::KeyPress(juce::KeyPress::upKey));  // touche pendant le geste molette (gestionnaire clavier)
  const int depth = pr.maxD();
  const int openAfterKey = pr.open();
  c->removeMouseListener(&log);
  r.destroyEditor();
  const int openAfterDestroy = pr.open();
  pr.userPhase = false;
  const int ui = gJournal.uiEvents(id);
  const int undo = r.countUndo();
  if (ui == 0) return blocked(id, "molette non recue");
  const bool ok = depth <= 1 && openAfterKey == 0 && openAfterDestroy == 0 && undo == 2;
  summarize(id, &pr, depth > 1 ? "REPRODUIT" : "NON REPRODUIT", "molette puis fleche haut : profondeur " + std::to_string(depth) + ", ouverts apres touche " +
                                                                    std::to_string(openAfterKey) + ", apres destruction " + std::to_string(openAfterDestroy),
            undo, 0, 0);
  tk::result(id, ok, "[PERMANENT, dispatch JUCE reel] molette puis touche avant 450 ms sur wt.x : profondeur max %d, ouverts apres touche %d, apres destruction %d, "
             "transactions Undo %d (attendu : 1, 0, 0, 2)", depth, openAfterKey, openAfterDestroy, undo);
}

// PB-01m : deux instances simultanées (CE-B2-01) : double-clic dans chacune, sondes indépendantes
void runMultiInstance() {
  const char* id = "PB-01m";
  gJournal.scenario = id;
  Rig a(40), b(1600);
  if (!a.ok() || !b.ok()) return blocked(id, "pair de test indisponible");
  for (Rig* r : {&a, &b}) {
    r->m->showSpace(0);
    r->m->refreshLayout();
  }
  auto* ca = findControl(*a.m, ParamId::WtX);
  auto* cb = findControl(*b.m, ParamId::WtX);
  if (!ca || !cb) return blocked(id, "wt.x introuvable");
  a.hostSet(ParamId::WtX, 0.8f);
  b.hostSet(ParamId::WtX, 0.7f);
  UiLog la("A wt.x"), lb("B wt.x");
  ca->addMouseListener(&la, false);
  cb->addMouseListener(&lb, false);
  Probe pa(*a.p), pb(*b.p);
  a.doubleClick(*ca, centre(*ca));
  const bool isolated = Probe::total(pb.begins) == 0 && near(b.base(ParamId::WtX), 0.7, 1.0);
  b.t = a.t + 1000;
  b.doubleClick(*cb, centre(*cb));
  ca->removeMouseListener(&la);
  cb->removeMouseListener(&lb);
  pa.userPhase = pb.userPhase = false;
  const float def = paramSpec(ParamId::WtX).defaultValue;
  const bool both = near(a.base(ParamId::WtX), def, 1.0) && near(b.base(ParamId::WtX), def, 1.0);
  const int ui = gJournal.uiEvents(id);
  if (ui == 0) return blocked(id, "aucun evenement recu");
  const bool ok = isolated && both && pa.open() == 0 && pb.open() == 0 && pa.maxD() <= 1 && pb.maxD() <= 1;
  summarize(id, &pa, ok ? "NON REPRODUIT" : "REPRODUIT",
            std::string("instances ") + (isolated ? "isolees" : "NON isolees") + ", A profondeur " + std::to_string(pa.maxD()) + ", B profondeur " +
                std::to_string(pb.maxD()) + ", defauts " + std::to_string(both),
            -1, a.base(ParamId::WtX), def);
  tk::result(id, ok, "[REPRODUCTION, dispatch JUCE reel] %s ; deux instances : isolement %d, defaut atteint dans les deux %d, A profondeur %d residuel %d, B profondeur %d residuel %d",
             ok ? "NON REPRODUIT" : "REPRODUIT", isolated, both, pa.maxD(), pa.open(), pb.maxD(), pb.open());
}

// ---------------------------------------------------------------- PB-02 : SYMBIOTIC CORE (a, b, c, e) et enveloppe (d)
juce::Component* findCore(MainEditor& m) {
  std::vector<juce::Component*> all;
  collect(m, all);
  for (auto* c : all)
    if (dynamic_cast<CorePad*>(c) != nullptr && c->isShowing()) return c;
  return nullptr;
}
// centre du pavé : au-dessus de la rangée numérique (NEXUS S + 4), sous la marge de 30 px, à gauche du canal Z (ui/PerformanceView.cpp)
juce::Point<float> padCentre(juce::Component& core) {
  const float h = static_cast<float>(core.getHeight()) - 30.0f - static_cast<float>(tok::nexusS.h + 4);
  const float w = static_cast<float>(core.getWidth()) - static_cast<float>(tok::plasmaV.w + 26);
  return {w * 0.5f, 30.0f + h * 0.5f};
}

enum class Cut { Destroy, HideSpace, Modal, Resize };

void runCore(const char* id, Cut cut) {
  gJournal.scenario = id;
  Rig r;
  if (!r.ok()) return blocked(id, "pair de test indisponible");
  r.m->showSpace(0);
  r.m->refreshLayout();
  auto* core = findCore(*r.m);
  if (core == nullptr) return blocked(id, "SYMBIOTIC CORE introuvable");
  UiLog log("core");
  core->addMouseListener(&log, false);
  Probe pr(*r.p);
  const auto pc = padCentre(*core);
  const auto releasePeer = r.toPeer(*core, pc + juce::Point<float>(12.0f, -8.0f));
  r.move(*core, pc);
  r.down(*core, pc);
  r.drag(*core, pc + juce::Point<float>(12.0f, -8.0f));
  const int began = Probe::total(pr.begins);
  std::string what;
  int openAfterCut = 0, openAfterRelease = -1;
  std::unique_ptr<juce::Component> dlg;
  switch (cut) {
    case Cut::Destroy:
      what = "destruction de l'editeur sans relachement";
      core->removeMouseListener(&log);
      r.destroyEditor();
      openAfterCut = pr.open();
      break;
    case Cut::HideSpace:
      what = "changement d'espace (vue masquee), puis relachement";
      r.m->showSpace(1);
      r.m->refreshLayout();
      openAfterCut = pr.open();
      r.at(releasePeer, juce::ModifierKeys());
      openAfterRelease = pr.open();
      core->removeMouseListener(&log);
      break;
    case Cut::Modal:
      what = "composant modal ouvert pendant le glisser, puis relachement";
      dlg = std::make_unique<juce::Component>();
      r.host->addAndMakeVisible(*dlg);
      dlg->setBounds(600, 300, 200, 120);
      dlg->enterModalState(false);
      openAfterCut = pr.open();
      r.at(releasePeer, juce::ModifierKeys());
      openAfterRelease = pr.open();
      dlg->exitModalState(0);
      core->removeMouseListener(&log);
      break;
    case Cut::Resize:
      what = "redimensionnement et zoom 75 % pendant le glisser, puis relachement";
      r.m->setSize(1200, 800);
      r.m->setZoomPercent(75);
      r.m->refreshLayout();
      openAfterCut = pr.open();
      r.at(releasePeer, juce::ModifierKeys());
      openAfterRelease = pr.open();
      core->removeMouseListener(&log);
      break;
  }
  r.destroyEditor();
  const int openAfterDestroy = pr.open();
  pr.userPhase = false;
  const int ui = gJournal.uiEvents(id);
  const int undo = r.countUndo();
  const int undoNext = undoAfterReopen(*r.p);  // conséquence sur l'historique de la session
  if (ui == 0 || began == 0) return blocked(id, "glisser non recu par le Core");
  // défaut PB-02 : un geste reste ouvert côté hôte après la coupure, et n'est refermé ni par le relâchement ni par la fin de vie
  const bool defect = (cut == Cut::Destroy) ? openAfterCut > 0 : (openAfterRelease > 0 || openAfterDestroy > 0);
  const std::string status = defect ? "REPRODUIT" : "NON REPRODUIT";
  const std::string detail = what + " : begin au glisser " + std::to_string(began) + ", ouverts apres coupure " + std::to_string(openAfterCut) +
                             ", apres relachement " + std::to_string(openAfterRelease) + ", apres destruction finale " + std::to_string(openAfterDestroy);
  summarize(id, &pr, status, detail + " ; edition suivante (nouvel editeur) : " + std::to_string(undoNext) + " transaction(s) Undo (1 attendue) ; correction reservee a R2-C", undo, 0, 0);
  tk::result(id, true, "[REPRODUCTION, dispatch JUCE reel] %s ; Core (cross.fwd, cross.rev), %s ; edition suivante : %d transaction(s) Undo (1 attendue) (correction : R2-C)",
             status.c_str(), detail.c_str(), undoNext);
}

void runEnvelope() {
  const char* id = "PB-02d";
  gJournal.scenario = id;
  // repérage d'une poignée par balayage, sur une instance d'exploration (aucune mesure retenue)
  juce::Point<float> handle{-1.0f, -1.0f};
  {
    Rig x;
    if (!x.ok()) return blocked(id, "pair de test indisponible");
    x.m->showSubTab(1, 2);
    x.m->refreshLayout();
    if (auto* env = findShowing<EnvelopeEditor>(*x.m)) {
      Probe px(*x.p);
      for (int gy = 1; gy < 40 && handle.x < 0; ++gy)
        for (int gx = 1; gx < 120 && handle.x < 0; ++gx) {
          const juce::Point<float> pt{env->getWidth() * gx / 120.0f, env->getHeight() * gy / 40.0f};
          const int b0 = Probe::total(px.begins);
          x.move(*env, pt);
          x.down(*env, pt);
          x.up(*env, pt);
          x.pause();
          if (Probe::total(px.begins) > b0) handle = pt;
        }
    }
  }
  gJournal.events.erase(std::remove_if(gJournal.events.begin(), gJournal.events.end(), [&](const std::string& e) { return e.rfind(std::string(id) + ";", 0) == 0; }),
                        gJournal.events.end());  // le balayage n'est pas une mesure
  Rig r;
  if (!r.ok()) return blocked(id, "pair de test indisponible");
  if (handle.x < 0) return blocked(id, "poignee d'enveloppe introuvable par balayage");
  r.m->showSubTab(1, 2);
  r.m->refreshLayout();
  auto* env = findShowing<EnvelopeEditor>(*r.m);
  if (env == nullptr) return blocked(id, "EnvelopeEditor introuvable");
  UiLog log("envelope");
  env->addMouseListener(&log, false);
  Probe pr(*r.p);
  r.move(*env, handle);
  r.down(*env, handle);
  r.drag(*env, handle + juce::Point<float>(6.0f, -4.0f));
  const int began = Probe::total(pr.begins);
  env->removeMouseListener(&log);
  r.destroyEditor();
  const int open = pr.open();
  pr.userPhase = false;
  const int ui = gJournal.uiEvents(id);
  const int undo = r.countUndo();
  const int undoNext = undoAfterReopen(*r.p);
  if (ui == 0 || began == 0) return blocked(id, "glisser non recu par l'editeur d'enveloppe");
  const std::string status = open > 0 ? "REPRODUIT" : "NON REPRODUIT";
  summarize(id, &pr, status, "poignee (" + std::to_string(handle.x) + ", " + std::to_string(handle.y) + ") glissee puis destruction de l'editeur : begin " +
                                 std::to_string(began) + ", gestes ouverts " + std::to_string(open) + " ; edition suivante (nouvel editeur) : " +
                                 std::to_string(undoNext) + " transaction(s) Undo (1 attendue) ; correction reservee a R2-D",
            undo, 0, 0);
  tk::result(id, true, "[REPRODUCTION, dispatch JUCE reel] %s ; editeur d'enveloppe : begin %d, gestes ouverts apres destruction %d ; edition suivante : %d transaction(s) Undo (1 attendue) (correction : R2-D)",
             status.c_str(), began, open, undoNext);
}

// ---------------------------------------------------------------- PB-N : NEXUS pendant un glisser
void runNexusCut(const char* id, int mode) {
  gJournal.scenario = id;
  Rig r;
  if (!r.ok()) return blocked(id, "pair de test indisponible");
  ParamId pid = ParamId::WtX;
  ContinuousControl* c = nullptr;
  if (mode == 3) {  // NEXUS d'un panneau contextuel : DST WT1 D2 Playback (type D2 GLITCH)
    pid = ParamId::DstWt1D2Speed;
    r.hostSet(ParamId::DstWt1Type, 6.0f);
    for (int t = 0; t < r.m->numTabs(1) && c == nullptr; ++t)
      for (int k = 0; k < r.m->numSubContexts(1, t) && c == nullptr; ++k) {
        r.m->showSubContext(1, t, k);
        r.m->refreshLayout();
        c = findControl(*r.m, pid);
      }
  } else {
    r.m->showSpace(0);
    r.m->refreshLayout();
    c = findControl(*r.m, pid);
  }
  if (c == nullptr) return blocked(id, std::string("commande introuvable : ") + paramSpec(pid).stableId);
  juce::Component::SafePointer<juce::Component> alive(c);
  UiLog log(paramSpec(pid).stableId);
  c->addMouseListener(&log, false);
  Probe pr(*r.p);
  const auto pt = centre(*c);
  const auto releasePeer = r.toPeer(*c, pt + juce::Point<float>(0.0f, -10.0f));
  r.move(*c, pt);
  r.down(*c, pt);
  r.drag(*c, pt + juce::Point<float>(0.0f, -10.0f));
  const int began = Probe::total(pr.begins);
  std::string what;
  int openAfterCut = 0, openAfterRelease = -1;
  bool destroyed = false;
  if (mode == 1) {
    what = "destruction de l'editeur pendant le glisser";
    c->removeMouseListener(&log);
    r.destroyEditor();
    openAfterCut = pr.open();
  } else if (mode == 2) {
    what = "changement d'espace pendant le glisser, puis relachement";
    r.m->showSpace(1);
    r.m->refreshLayout();
    openAfterCut = pr.open();
    r.at(releasePeer, juce::ModifierKeys());
    openAfterRelease = pr.open();
    if (alive != nullptr) c->removeMouseListener(&log);
  } else {
    what = "type DST WT1 change par l'hote pendant le glisser (panneau contextuel), puis relachement";
    pr.userPhase = false;
    r.hostSet(ParamId::DstWt1Type, 1.0f);  // N1 FOLD
    r.m->refreshLayout();
    pr.userPhase = true;
    destroyed = alive == nullptr;
    openAfterCut = pr.open();
    r.at(releasePeer, juce::ModifierKeys());
    openAfterRelease = pr.open();
    if (alive != nullptr) c->removeMouseListener(&log);
  }
  r.destroyEditor();
  const int openAfterDestroy = pr.open();
  pr.userPhase = false;
  const int ui = gJournal.uiEvents(id);
  const int undo = r.countUndo();
  if (ui == 0 || began == 0) return blocked(id, "glisser non recu par la commande");
  const bool defect = pr.maxD() > 1 || (mode == 1 ? openAfterCut > 0 : (openAfterRelease > 0 || openAfterDestroy > 0));
  const std::string status = defect ? "REPRODUIT" : "NON REPRODUIT";
  const std::string detail = what + (mode == 3 ? std::string(destroyed ? " (commande detruite par la reconstruction)" : " (commande conservee)") : std::string()) +
                             " : begin " + std::to_string(began) + ", ouverts apres coupure " + std::to_string(openAfterCut) + ", apres relachement " +
                             std::to_string(openAfterRelease) + ", apres destruction finale " + std::to_string(openAfterDestroy);
  summarize(id, &pr, status, detail + " ; lot : B2 (gestes NEXUS)", undo, 0, 0);
  // PB-N1 : 0.4.1 referme le geste dans le destructeur (BE-05 attendu) : assertion de non-régression ; N2, N3 : caractérisation
  tk::result(id, mode == 1 ? !defect : true, "[REPRODUCTION, dispatch JUCE reel] %s ; %s : %s, profondeur max %d, transactions Undo %d", status.c_str(),
             paramSpec(pid).stableId, detail.c_str(), pr.maxD(), undo);
}

// ---------------------------------------------------------------- PB-J : prise en main des PLASMA (DB2-02 : aucun saut involontaire)
void runJump(const char* id, ParamId pid, float start, const char* where, juce::Point<float> (*at)(juce::Component&), bool outsideManipulation) {
  gJournal.scenario = id;
  Rig r;
  if (!r.ok()) return blocked(id, "pair de test indisponible");
  r.m->showSpace(0);
  r.m->refreshLayout();
  auto* c = findControl(*r.m, pid);
  if (c == nullptr) return blocked(id, std::string("commande introuvable : ") + paramSpec(pid).stableId);
  r.hostSet(pid, start);
  UiLog log(paramSpec(pid).stableId);
  c->addMouseListener(&log, false);
  Probe pr(*r.p);
  const double v0 = r.base(pid);
  const auto pt = at(*c);
  r.move(*c, pt);
  r.down(*c, pt);
  r.up(*c, pt);
  const double v1 = r.base(pid);
  r.pause();
  r.down(*c, pt);  // seconde prise au même point
  r.up(*c, pt);
  const double v2 = r.base(pid);
  c->removeMouseListener(&log);
  pr.userPhase = false;
  const auto& s = paramSpec(pid);
  const double span = s.maxValue - s.minValue;
  const int ui = gJournal.uiEvents(id);
  r.destroyEditor();
  const int undo = r.countUndo();
  if (ui == 0) return blocked(id, "aucun evenement recu par le composant");
  const bool jump1 = std::abs(v1 - v0) > 0.02 * span, regrab = std::abs(v2 - v1) > 0.01 * span;
  // hors zone de manipulation : aucun changement attendu ; dans le canal : position absolue au premier clic (DB2-02), stable ensuite
  const bool defect = outsideManipulation ? (jump1 || regrab) : regrab;
  const std::string status = defect ? "REPRODUIT" : "NON REPRODUIT";
  char d[300];
  std::snprintf(d, sizeof d, "%s : %.4f -> %.4f (2e prise %.4f) ; %s", where, v0, v1, v2,
                outsideManipulation ? "aucun changement attendu (hors zone de manipulation)" : "position absolue attendue au 1er clic, stable au 2e");
  summarize(id, &pr, status, d, undo, v1, outsideManipulation ? v0 : v1);
  // DB2-08 : assertions corrigées (sauts reproduits avant correction : PB/*/pb-summary.csv). Hors zone : aucun changement,
  // aucun geste, aucune écriture, aucune transaction ; dans le canal : stable à la reprise, gestes appariés.
  const bool ok = outsideManipulation ? (!defect && Probe::total(pr.begins) == 0 && Probe::total(pr.writes) == 0 && undo == 0)
                                      : (!defect && pr.open() == 0 && Probe::total(pr.begins) == Probe::total(pr.ends) && pr.maxD() <= 1);
  tk::result(id, ok, "[PERMANENT DB2-08, dispatch JUCE reel] %s ; %s %s ; gestes %d, ecritures %d, profondeur max %d, residuel %d, transactions Undo %d", status.c_str(),
             s.stableId, d, Probe::total(pr.begins), Probe::total(pr.writes), pr.maxD(), pr.open(), undo);
}


// ================================================================ B2-03.2 : contrat PLASMA (DB2-02, DB2-08, DB2-09)
// Critères enregistrés avant mesure : PLAS-Z (B2-ACCEPTANCE-CRITERIA.md) et EC19-FIX (B2-00-ADDENDUM-B2-03.md §4).
// Les trois PLASMA de production : out.gain (vertical, rendu 0.4.1 : gabarit R1 bloqué, CE-B2-02), cross.master (Z du Core,
// vertical, R1 STANDARD), perf.balance (horizontal, rendu 0.4.1 : bloqué). Le contrat d'interaction est le même pour tous.

ContinuousControl* findPlasma(MainEditor& m, ParamId id) {
  for (auto* b : m.visibleControls())
    if (b->boundId() == id)
      if (auto* cc = dynamic_cast<ContinuousControl*>(&b->asComponent()); cc != nullptr && cc->form() != Form::Rotary) return cc;
  return nullptr;
}

// fenêtre du banc : 1440 x 900 à 75 et 100 % (configurations UI04 : surface logique 1920 x 1200 et 1440 x 900) ;
// à 150 %, fenêtre agrandie pour garder une surface logique de 1440 x 900 (la vue PERFORMANCE entière, sans défilement)
int winW(int z) { return z > 100 ? 1440 * z / 100 : 1440; }
int winH(int z) { return z > 100 ? 900 * z / 100 : 900; }

struct PlCase {
  ParamId id;
  float start;  // valeur de départ posée par l'hôte (différente du défaut)
};
const PlCase kPl[3] = {{ParamId::OutGain, 0.0f}, {ParamId::CrossMaster, 0.4f}, {ParamId::PerfBalance, 0.5f}};

// point de la zone de manipulation à la position normalisée n (le long du canal)
juce::Point<float> chanAt(ContinuousControl& c, float n) {
  const auto ch = c.channelBox();
  if (c.form() == Form::LinearV) return {ch.getCentreX(), ch.getBottom() - n * ch.getHeight()};
  return {ch.getX() + n * ch.getWidth(), ch.getCentreY()};
}
float normOf(Rig& r, ParamId id) {
  auto* rp = dynamic_cast<juce::RangedAudioParameter*>(r.p->getParameters()[idx(id)]);
  return rp != nullptr ? rp->getValue() : -1.0f;
}

// PLAS-Z : zones (libellé, valeur, hors canal, frontières), glisser commencé hors canal et dans le canal, clic absolu
// aux extrémités, prise en main sans saut, focus ; trois PLASMA ; zooms 75, 100 et 150 %
void runPlasmaZones() {
  const char* id = "PLAS-Z";
  gJournal.scenario = id;
  int checks = 0, fails = 0;
  std::string first;
  const auto fail = [&](bool ok, const std::string& what) {
    ++checks;
    if (!ok) {
      ++fails;
      if (fails <= 14) first += (first.empty() ? "" : " | ") + what;
    }
  };
  for (int z : {75, 100, 150})
    for (const auto& pc : kPl) {
      Rig r(40, winW(z), winH(z));
      if (!r.ok()) return blocked(id, "pair de test indisponible");
      r.m->setZoomPercent(z);
      r.m->showSpace(0);
      r.m->refreshLayout();
      auto* c = findPlasma(*r.m, pc.id);
      const std::string tag = std::string(paramSpec(pc.id).stableId) + " @" + std::to_string(z) + "%";
      if (c == nullptr) return blocked(id, "PLASMA introuvable : " + tag);
      const bool V = c->form() == Form::LinearV;
      r.hostSet(pc.id, pc.start);
      const float n0 = normOf(r, pc.id);
      Probe pr(*r.p);
      const auto noEffect = [&](juce::Point<float> pt, const std::string& what) {
        const int b0 = Probe::total(pr.begins), w0 = Probe::total(pr.writes);
        r.move(*c, pt);
        r.down(*c, pt);
        r.up(*c, pt);
        r.pause();
        fail(Probe::total(pr.begins) == b0 && Probe::total(pr.writes) == w0 && std::abs(normOf(r, pc.id) - n0) < 1e-6f, tag + " " + what);
      };
      // libellé, valeur, hors canal (côté graduations ou loin du rail), juste hors de la zone de manipulation
      noEffect(c->labelBox().getCentre(), "clic libelle");
      noEffect(c->valueBox().getCentre(), "clic valeur");
      const auto mz = c->manipulationBox();
      const auto lb = c->getLocalBounds().toFloat();
      if (V) {
        if (mz.getRight() + 3.0f < lb.getRight()) noEffect({mz.getRight() + 2.0f, mz.getCentreY()}, "clic hors canal (droite)");
        if (mz.getX() - 3.0f > lb.getX()) noEffect({mz.getX() - 2.0f, mz.getCentreY()}, "clic hors canal (gauche)");
      } else {
        if (mz.getY() - 3.0f > lb.getY()) noEffect({mz.getCentreX(), mz.getY() - 2.0f}, "clic hors canal (haut)");
        if (mz.getBottom() + 3.0f < lb.getBottom()) noEffect({mz.getCentreX(), mz.getBottom() + 2.0f}, "clic hors canal (bas)");
      }
      // focus et sélection au clic sur le libellé (aucune écriture : vérifié ci-dessus)
      fail(c->hasKeyboardFocus(false), tag + " focus apres clic libelle");
      // glisser commencé hors canal (libellé) puis amené dans le canal : jamais un geste de modification
      {
        const int b0 = Probe::total(pr.begins), w0 = Probe::total(pr.writes);
        const auto from = c->labelBox().getCentre(), to = chanAt(*c, 0.9f);
        r.move(*c, from);
        r.down(*c, from);
        for (int k = 1; k <= 6; ++k) r.drag(*c, from + (to - from) * (static_cast<float>(k) / 6.0f));
        r.up(*c, to);
        r.pause();
        fail(Probe::total(pr.begins) == b0 && Probe::total(pr.writes) == w0 && std::abs(normOf(r, pc.id) - n0) < 1e-6f, tag + " glisser depuis le libelle");
      }
      // frontière : juste à l'intérieur de la zone de manipulation -> geste et position absolue
      {
        const int b0 = Probe::total(pr.begins);
        const auto in = V ? juce::Point<float>(mz.getX() + 1.0f, chanAt(*c, 0.25f).y) : juce::Point<float>(chanAt(*c, 0.25f).x, mz.getY() + 1.0f);
        r.move(*c, in);
        r.down(*c, in);
        const float after = normOf(r, pc.id);
        r.up(*c, in);
        r.pause();
        fail(Probe::total(pr.begins) == b0 + 1 && std::abs(after - 0.25f) < 0.02f, tag + " clic en bordure interieure");
      }
      // extrémités : clic au bord extrême de la zone de manipulation -> borne (correspondance bornée ; en R1 la zone
      // dépasse le canal de la demi-hauteur du curseur, donc la borne est exacte ; en 0.4.1, à une ligne près)
      for (float target : {1.0f, 0.0f}) {
        const auto zz = c->manipulationBox();
        const auto pt = V ? juce::Point<float>(zz.getCentreX(), target > 0.5f ? zz.getY() + 0.5f : zz.getBottom() - 0.5f)
                          : juce::Point<float>(target > 0.5f ? zz.getRight() - 0.5f : zz.getX() + 0.5f, zz.getCentreY());
        r.move(*c, pt);
        r.down(*c, pt);
        r.up(*c, pt);
        r.pause();
        fail(std::abs(normOf(r, pc.id) - target) < (c->plasmaR1() ? 1e-4f : 0.02f), tag + (target > 0.5f ? " extremite haute" : " extremite basse"));
      }
      // glisser dans le canal : prise en main sans saut, puis suivi rapporté à la longueur utile (DB2-02)
      {
        const auto p0 = chanAt(*c, 0.5f);
        r.move(*c, p0);
        r.down(*c, p0);
        const float a = normOf(r, pc.id);
        r.drag(*c, p0);  // premier déplacement nul : aucun saut
        const float b = normOf(r, pc.id);
        const auto p1 = chanAt(*c, 0.75f);
        for (int k = 1; k <= 5; ++k) r.drag(*c, p0 + (p1 - p0) * (static_cast<float>(k) / 5.0f));
        const float d = normOf(r, pc.id);
        r.up(*c, p1);
        r.pause();
        fail(std::abs(a - 0.5f) < 0.02f && std::abs(b - a) < 1e-6f && std::abs(d - 0.75f) < 0.02f, tag + " glisser dans le canal");
      }
      pr.userPhase = false;
      fail(pr.open() == 0 && pr.maxD() <= 1 && Probe::total(pr.begins) == Probe::total(pr.ends) && Probe::total(pr.outside) == 0, tag + " gestes apparies");
    }
  tk::result(id, fails == 0, "DB2-08 : out.gain (0.4.1, bloque), cross.master (R1 STANDARD), perf.balance (0.4.1, bloque) x zooms 75/100/150 : %d verifications "
             "(libelle, valeur, hors canal, glisser depuis le libelle, bordure, extremites, prise en main, suivi, focus, appariement) ; echecs %d%s%s",
             checks, fails, first.empty() ? "" : " : ", first.c_str());
}

// EC19-FIX : DB2-09 (B), liste de B2-00-ADDENDUM-B2-03.md §4
void runPlasmaDoubleClickContract() {
  const char* id = "EC19-FIX";
  gJournal.scenario = id;
  int checks = 0, fails = 0;
  std::string first;
  const auto fail = [&](bool ok, const std::string& what) {
    ++checks;
    if (!ok) {
      ++fails;
      if (fails <= 14) first += (first.empty() ? "" : " | ") + what;
    }
  };
  for (int z : {75, 100, 150})
    for (const auto& pc : kPl) {
      const std::string tag = std::string(paramSpec(pc.id).stableId) + " @" + std::to_string(z) + "%";
      const auto& spec = paramSpec(pc.id);
      int sub = 0;
      auto defNorm = [&](Rig& r) {
        auto* rp = dynamic_cast<juce::RangedAudioParameter*>(r.p->getParameters()[idx(pc.id)]);
        return rp->convertTo0to1(spec.defaultValue);
      };
      auto setup = [&](Rig& r) -> ContinuousControl* {
        gJournal.scenario = std::string(id) + "/" + tag + "/" + std::to_string(++sub);  // journal propre à chaque sous-scénario
        r.m->setZoomPercent(z);
        r.m->showSpace(0);
        r.m->refreshLayout();
        auto* c = findPlasma(*r.m, pc.id);
        if (c != nullptr) r.hostSet(pc.id, pc.start);
        return c;
      };
      // (1) double-clic dans le canal : réponse immédiate, aucun retour au défaut, aucune valeur intermédiaire, une écriture,
      //     une transaction, gestes appariés, aucun geste résiduel
      {
        Rig r(40, winW(z), winH(z));
        auto* c = r.ok() ? setup(r) : nullptr;
        if (c == nullptr) return blocked(id, "PLASMA introuvable : " + tag);
        Probe pr(*r.p);
        const auto pt = chanAt(*c, 0.3f);
        r.move(*c, pt);
        r.down(*c, pt);
        const float firstResp = normOf(r, pc.id);
        r.up(*c, pt);
        r.down(*c, pt);
        r.up(*c, pt);
        r.pause();
        pr.userPhase = false;
        const auto w = writesOf(gJournal.scenario, spec.stableId);
        const float dn = defNorm(r);
        bool def = false;
        for (double x : w) def = def || std::abs(x - dn) < 1e-4;
        const float fin = normOf(r, pc.id);
        const int maxd = pr.maxD(), open = pr.open(), beg = Probe::total(pr.begins), end = Probe::total(pr.ends);
        gJournal.scenario += "-undo";  // les Undo du comptage ne sont pas comptés comme écritures du scénario
        r.destroyEditor();
        const int undo = r.countUndo();
        fail(std::abs(firstResp - 0.3f) < 0.02f, tag + " reponse immediate au 1er clic");
        fail(std::abs(fin - firstResp) < 1e-6f && !def, tag + " aucun retour au defaut / valeur intermediaire");
        fail(w.size() == 1, tag + " une seule ecriture (" + std::to_string(w.size()) + ")");
        fail(undo == 1, tag + " une transaction Undo (" + std::to_string(undo) + ")");
        fail(maxd <= 1 && open == 0 && beg == end, tag + " gestes apparies, aucun residuel");
      }
      // (2) deux clics successifs à des positions différentes (dans le délai du double-clic)
      {
        Rig r(40, winW(z), winH(z));
        auto* c = setup(r);
        Probe pr(*r.p);
        const auto a = chanAt(*c, 0.2f), b = chanAt(*c, 0.8f);
        r.move(*c, a);
        r.down(*c, a);
        r.up(*c, a);
        r.move(*c, b);
        r.down(*c, b);
        r.up(*c, b);
        r.pause();
        const float fin = normOf(r, pc.id);
        pr.userPhase = false;
        r.destroyEditor();
        fail(std::abs(fin - 0.8f) < 0.02f && pr.open() == 0 && std::abs(fin - defNorm(r)) > 1e-3f, tag + " deux clics a deux positions");
      }
      // (3) double-clic suivi d'un glisser : le glisser part de la position cliquée, aucun retour au défaut
      {
        Rig r(40, winW(z), winH(z));
        auto* c = setup(r);
        Probe pr(*r.p);
        const auto a = chanAt(*c, 0.4f), b = chanAt(*c, 0.6f);
        r.move(*c, a);
        r.down(*c, a);
        r.up(*c, a);
        r.down(*c, a);
        for (int k = 1; k <= 4; ++k) r.drag(*c, a + (b - a) * (static_cast<float>(k) / 4.0f));
        r.up(*c, b);
        r.pause();
        const float fin = normOf(r, pc.id);
        pr.userPhase = false;
        fail(std::abs(fin - 0.6f) < 0.02f && pr.open() == 0 && pr.maxD() <= 1, tag + " double-clic puis glisser");
      }
      // (4) double-clic aux extrémités du canal : bornes, aucun retour au défaut
      for (float target : {0.0f, 1.0f}) {
        Rig r(40, winW(z), winH(z));
        auto* c = setup(r);
        const auto pt = chanAt(*c, target);
        r.doubleClick(*c, pt);
        fail(std::abs(normOf(r, pc.id) - target) < 1e-3f, tag + " double-clic extremite " + std::to_string(static_cast<int>(target)));
      }
      // (5) automation active : l'hôte écrit entre les clics ; gestes appariés, valeur finale = position cliquée
      {
        Rig r(40, winW(z), winH(z));
        auto* c = setup(r);
        Probe pr(*r.p);
        const auto pt = chanAt(*c, 0.7f);
        r.move(*c, pt);
        r.down(*c, pt);
        r.up(*c, pt);
        pr.userPhase = false;
        r.hostSet(pc.id, spec.minValue + 0.1f * (spec.maxValue - spec.minValue));  // lecture d'automation hôte
        pr.userPhase = true;
        r.down(*c, pt);
        r.up(*c, pt);
        r.pause();
        const float fin = normOf(r, pc.id);
        pr.userPhase = false;
        fail(std::abs(fin - 0.7f) < 0.02f && pr.open() == 0 && Probe::total(pr.begins) == Probe::total(pr.ends) && pr.maxD() <= 1,
             tag + " automation active");
      }
      // (6) retours au défaut autorisés : Alt+clic dans le canal, Suppr, menu ; saisie numérique sans écriture préalable
      {
        Rig r(40, winW(z), winH(z));
        auto* c = setup(r);
        const float dn = defNorm(r);
        const auto pt = chanAt(*c, 0.5f);
        r.move(*c, pt);
        r.down(*c, pt, juce::ModifierKeys::altModifier);
        r.up(*c, pt, juce::ModifierKeys::altModifier);
        r.pause();
        const bool alt = std::abs(normOf(r, pc.id) - dn) < 1e-4f;
        r.hostSet(pc.id, pc.start);
        c->keyPressed(juce::KeyPress(juce::KeyPress::deleteKey));
        const bool del = std::abs(normOf(r, pc.id) - dn) < 1e-4f;
        r.hostSet(pc.id, pc.start);
        c->resetToDefault();
        const bool menu = std::abs(normOf(r, pc.id) - dn) < 1e-4f;
        r.hostSet(pc.id, pc.start);
        const float before = normOf(r, pc.id);
        Probe pr(*r.p);
        r.doubleClick(*c, c->valueBox().getCentre());
        bool entry = false;
        for (auto* ch : c->getChildren()) entry = entry || dynamic_cast<juce::TextEditor*>(ch) != nullptr;
        fail(alt && del && menu, tag + " defaut par Alt+clic, Suppr, menu");
        fail(entry && Probe::total(pr.writes) == 0 && Probe::total(pr.begins) == 0 && std::abs(normOf(r, pc.id) - before) < 1e-6f,
             tag + " saisie sans ecriture prealable (saisie " + std::to_string(entry) + ", ecritures " + std::to_string(Probe::total(pr.writes)) + ", gestes " +
                 std::to_string(Probe::total(pr.begins)) + ", " + std::to_string(before) + " -> " + std::to_string(normOf(r, pc.id)) + ", valeur " +
                 c->valueBox().toString().toStdString() + ", zone " + c->manipulationBox().toString().toStdString() + ")");
      }
      // infobulle : interactions réellement disponibles (DB2-09)
      {
        Rig r(40, winW(z), winH(z));
        auto* c = setup(r);
        const auto t = c->getTooltip();
        fail(t.contains(juce::String::fromUTF8("Alt+clic, Suppr ou menu : défaut")) && !t.contains(juce::String::fromUTF8("double-clic : défaut")), tag + " infobulle");
      }
    }
  // (7) instances multiples : un double-clic dans chaque instance, sondes indépendantes, aucune interférence
  {
    Rig a(40), b(1520);
    if (!a.ok() || !b.ok()) return blocked(id, "instances multiples indisponibles");
    for (auto* rg : {&a, &b}) {
      rg->m->showSpace(0);
      rg->m->refreshLayout();
      rg->hostSet(ParamId::OutGain, 0.0f);
    }
    Probe pa(*a.p), pb(*b.p);
    auto* ca = findPlasma(*a.m, ParamId::OutGain);
    auto* cb = findPlasma(*b.m, ParamId::OutGain);
    a.doubleClick(*ca, chanAt(*ca, 0.3f));
    const float bUntouched = normOf(b, ParamId::OutGain);
    b.doubleClick(*cb, chanAt(*cb, 0.6f));
    pa.userPhase = pb.userPhase = false;
    fail(std::abs(normOf(a, ParamId::OutGain) - 0.3f) < 0.02f && std::abs(normOf(b, ParamId::OutGain) - 0.6f) < 0.02f && pa.open() == 0 && pb.open() == 0 &&
             std::abs(bUntouched - normOf(b, ParamId::OutGain)) > 0.1f,
         "instances multiples");
  }
  tk::result(id, fails == 0,
             "DB2-09 (B) : 3 PLASMA x zooms 75/100/150 + instances multiples : %d verifications (reponse immediate, aucun retour au defaut, aucune valeur "
             "intermediaire, une ecriture, une transaction, gestes apparies, deux positions, double-clic puis glisser, extremites, automation active, "
             "Alt+clic / Suppr / menu, saisie sans ecriture, infobulle) ; echecs %d%s%s",
             checks, fails, first.empty() ? "" : " : ", first.c_str());
}

juce::Point<float> labelTop(juce::Component& c) { return {c.getWidth() * 0.5f, 4.0f}; }
juce::Point<float> labelLeft(juce::Component& c) { return {c.getWidth() * 0.12f, c.getHeight() * 0.5f}; }
juce::Point<float> valueRight(juce::Component& c) { return {c.getWidth() * 0.86f, c.getHeight() * 0.5f}; }
juce::Point<float> channelCentre(juce::Component& c) { return {c.getWidth() * 0.5f, c.getHeight() * 0.5f}; }

void writeJournal() {
  const char* dir = std::getenv("SBD_B2_OUT");
  if (dir == nullptr || *dir == 0) return;
  const juce::File d(juce::String::fromUTF8(dir));
  d.createDirectory();
  std::ofstream ev(d.getChildFile("pb-events.csv").getFullPathName().toStdString(), std::ios::binary);
  ev << "scenario;seq;source;event;param;value;depth\n";
  for (auto& l : gJournal.events) ev << l << "\n";
  std::ofstream su(d.getChildFile("pb-summary.csv").getFullPathName().toStdString(), std::ios::binary);
  su << "scenario;status;begins;ends;max_depth;residual;outside_writes;undo_transactions;value;expected;detail\n";
  for (auto& l : gJournal.summary) su << l << "\n";
  std::ofstream pl(d.getChildFile("pb-platform.txt").getFullPathName().toStdString(), std::ios::binary);
  pl << "os: " << juce::SystemStats::getOperatingSystemName().toStdString() << "\njuce: " << juce::SystemStats::getJUCEVersion().toStdString()
     << "\ndouble-click timeout ms: " << juce::MouseEvent::getDoubleClickTimeout()
     << "\nbanc: editeur reel dans un hote de test (TestPeer : couche fenetre de l'OS remplacee), evenements injectes par ComponentPeer::handleMouseEvent\n";
}

}  // namespace

void testGestureReproduction() {
  std::fprintf(stderr, "[ARCH-UI-04-R2-B2-01 : reproductions PB-01 / PB-02 / PB-N / PB-J]\n");
  hypotheticalH04();
  runDoubleClick("PB-01a", ParamId::WtX, 0.8f);
  runDoubleClick("PB-01b", ParamId::OutGain, 0.0f);
  runLinearHValueDoubleClick();
  runDefaultPaths();
  runWheelThenClick();
  runWheelThenKey();
  runMultiInstance();
  runCore("PB-02a", Cut::Destroy);
  runCore("PB-02b", Cut::HideSpace);
  runCore("PB-02c", Cut::Modal);
  runEnvelope();
  runCore("PB-02e", Cut::Resize);
  runNexusCut("PB-N1", 1);
  runNexusCut("PB-N2", 2);
  runNexusCut("PB-N3", 3);
  runJump("PB-J1", ParamId::OutGain, 0.0f, "clic sur le libelle du PLASMA vertical", labelTop, true);
  runJump("PB-J2", ParamId::PerfBalance, 0.5f, "clic sur le libelle du PLASMA horizontal", labelLeft, true);
  runJump("PB-J3", ParamId::PerfBalance, 0.5f, "clic sur la valeur du PLASMA horizontal", valueRight, true);
  runJump("PB-J4", ParamId::OutGain, 0.0f, "clic dans le canal puis reprise au meme point", channelCentre, false);
  runPlasmaZones();
  runPlasmaDoubleClickContract();
  juce::ModifierKeys::currentModifiers = juce::ModifierKeys();  // aucun état clavier simulé laissé aux tests suivants
  writeJournal();
}
