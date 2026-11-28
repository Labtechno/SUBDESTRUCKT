// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab — auto-tests des composants R2 (laboratoire uniquement).
// Ils vérifient la discipline de geste des composants R2 (comptage begin/end, aucun geste imbriqué, fermeture à la
// destruction, au masquage et au détachement) et la stratégie d'identification OD-01. Ils ne testent PAS le plugin de
// production : PB-01 et PB-02 y restent non corrigés (INTERACTION-TEST-PLAN.md).
#include "app/SelfTest.h"

#include "app/Sheets.h"

namespace sbd::lab {

using namespace sbd::r2;

namespace {

struct Results {
  int pass = 0, fail = 0;
  juce::StringArray lines;
  void add(const juce::String& id, bool ok, const juce::String& detail) {
    (ok ? pass : fail)++;
    lines.add("RESULT " + id + (ok ? " PASS " : " FAIL ") + detail);
  }
};

juce::MouseEvent ev(juce::Component& c, juce::Point<float> p, juce::ModifierKeys mods, int clicks = 1, bool dragged = false) {
  auto src = juce::Desktop::getInstance().getMainMouseSource();
  const auto now = juce::Time::getCurrentTime();
  return juce::MouseEvent(src, p, mods, juce::MouseInputSource::defaultPressure, 0.0f, 0.0f, 0.0f, 0.0f, &c, &c, now, p, now, clicks, dragged);
}
const juce::ModifierKeys kLeft(juce::ModifierKeys::leftButtonModifier);
const juce::ModifierKeys kShift(juce::ModifierKeys::leftButtonModifier | juce::ModifierKeys::shiftModifier);
const juce::ModifierKeys kAlt(juce::ModifierKeys::leftButtonModifier | juce::ModifierKeys::altModifier);

juce::String counts(const DemoValue& v) {
  return "begin " + juce::String(v.begins) + " end " + juce::String(v.ends) + " nested " + juce::String(v.nested) + " outside " + juce::String(v.writesOutside);
}

juce::Component* findById(juce::Component& root, const juce::String& id) {
  if (root.getComponentID() == id) return &root;
  for (auto* c : root.getChildren())
    if (auto* f = findById(*c, id)) return f;
  return nullptr;
}

}  // namespace

int runSelfTest(juce::StringArray& out) {
  Results R;
  juce::Component parent;
  parent.setSize(400, 400);
  const juce::Point<float> knob(42.0f, 50.0f);  // centre du bouton d'un NEXUS S (84 x 96)

  // LAB-G01 glisser : un geste, refermé au relâchement
  {
    DemoValue v(0, 1, 0, 0.5f, "", 2);
    Nexus n(Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
    parent.addAndMakeVisible(n);
    n.setBounds(0, 0, 84, 96);
    n.mouseDown(ev(n, knob, kLeft));
    for (int k = 1; k <= 10; ++k) n.mouseDrag(ev(n, knob.translated(0.0f, -2.2f * static_cast<float>(k)), kLeft, 1, true));
    n.mouseUp(ev(n, knob.translated(0.0f, -22.0f), juce::ModifierKeys()));
    const bool ok = v.begins == 1 && v.ends == 1 && v.nested == 0 && v.writesOutside == 0 && std::abs(v.norm() - 0.6f) < 1.0e-3f;
    R.add("LAB-G01", ok, "drag 22 px = +0.10 (" + juce::String(v.norm(), 3) + "), " + counts(v));
  }
  // LAB-G02 double-clic hors valeur : défaut dans le geste ouvert, aucun geste imbriqué (protocole PB-01)
  {
    DemoValue v(0, 1, 0.25f, 0.8f, "", 2);
    Nexus n(Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
    parent.addAndMakeVisible(n);
    n.setBounds(0, 0, 84, 96);
    n.mouseDown(ev(n, knob, kLeft));
    n.mouseUp(ev(n, knob, juce::ModifierKeys()));
    n.mouseDown(ev(n, knob, kLeft, 2));
    n.mouseDoubleClick(ev(n, knob, kLeft, 2));
    n.mouseUp(ev(n, knob, juce::ModifierKeys(), 2));
    const bool ok = v.balanced() && v.nested == 0 && v.maxDepth == 1 && std::abs(v.norm() - 0.25f) < 1.0e-4f;
    R.add("LAB-G02", ok, "double click -> default " + juce::String(v.norm(), 2) + ", max depth " + juce::String(v.maxDepth) + ", " + counts(v));
  }
  // LAB-G03 molette : crans groupés en un geste ; une touche referme le geste molette avant sa propre transaction
  {
    DemoValue v(0, 1, 0, 0.5f, "", 2);
    Nexus n(Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
    parent.addAndMakeVisible(n);
    n.setBounds(0, 0, 84, 96);
    juce::MouseWheelDetails w{};
    w.deltaY = 0.1f;
    for (int k = 0; k < 3; ++k) n.mouseWheelMove(ev(n, knob, juce::ModifierKeys()), w);
    const bool grouped = v.begins == 1 && v.depth == 1;
    n.keyPressed(juce::KeyPress(juce::KeyPress::upKey));
    const bool ok = grouped && v.begins == 2 && v.balanced() && v.nested == 0;
    R.add("LAB-G03", ok, "3 wheel notches = 1 gesture (" + juce::String(grouped ? "yes" : "no") + "), then key; " + counts(v));
  }
  // LAB-G04 clavier : flèches, Page, Début/Fin, Suppr ; une transaction complète par appui
  {
    DemoValue v(0, 1, 0.5f, 0.5f, "", 2);
    Nexus n(Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
    const int keys[] = {juce::KeyPress::upKey, juce::KeyPress::pageUpKey, juce::KeyPress::endKey, juce::KeyPress::homeKey, juce::KeyPress::deleteKey};
    for (int k : keys) n.keyPressed(juce::KeyPress(k));
    const bool ok = v.begins == 5 && v.balanced() && v.nested == 0 && std::abs(v.norm() - 0.5f) < 1.0e-4f;
    R.add("LAB-G04", ok, "5 keys = 5 complete gestures, final default; " + counts(v));
  }
  // LAB-G05 Alt+clic : défaut en une transaction, sans glisser
  {
    DemoValue v(0, 1, 0.1f, 0.9f, "", 2);
    Nexus n(Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
    parent.addAndMakeVisible(n);
    n.setBounds(0, 0, 84, 96);
    n.mouseDown(ev(n, knob, kAlt));
    n.mouseUp(ev(n, knob, juce::ModifierKeys()));
    R.add("LAB-G05", v.begins == 1 && v.balanced() && std::abs(v.norm() - 0.1f) < 1.0e-4f, "alt-click default; " + counts(v));
  }
  // LAB-G06 destruction pendant un glisser : geste refermé (cas PB-02 pour les composants R2)
  {
    DemoValue v(0, 1, 0, 0.5f, "", 2);
    {
      auto n = std::make_unique<Nexus>(Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
      parent.addAndMakeVisible(*n);
      n->setBounds(0, 0, 84, 96);
      n->mouseDown(ev(*n, knob, kLeft));
      n->mouseDrag(ev(*n, knob.translated(0.0f, -10.0f), kLeft, 1, true));
    }  // destruction sans mouseUp
    R.add("LAB-G06", v.begins == 1 && v.balanced(), "editor destroyed mid-drag; " + counts(v));
  }
  // LAB-G07 masquage pendant un glisser
  {
    DemoValue v(0, 1, 0, 0.5f, "", 2);
    Nexus n(Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
    parent.addAndMakeVisible(n);
    n.setBounds(0, 0, 84, 96);
    n.mouseDown(ev(n, knob, kLeft));
    n.setVisible(false);
    const bool closed = v.balanced();
    n.mouseDrag(ev(n, knob.translated(0.0f, -10.0f), kLeft, 1, true));  // événement tardif : ignoré
    n.mouseUp(ev(n, knob, juce::ModifierKeys()));
    R.add("LAB-G07", closed && v.balanced() && v.writesOutside == 0, "hidden mid-drag, late events ignored; " + counts(v));
  }
  // LAB-G08 détachement pendant un glisser (changement de vue)
  {
    DemoValue v(0, 1, 0, 0.5f, "", 2);
    Nexus n(Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
    parent.addAndMakeVisible(n);
    n.setBounds(0, 0, 84, 96);
    n.mouseDown(ev(n, knob, kLeft));
    parent.removeChildComponent(&n);
    R.add("LAB-G08", v.balanced(), "removed from parent mid-drag; " + counts(v));
  }
  // LAB-G09 changement de mode fin pendant le glisser : pas de saut de valeur
  {
    DemoValue v(0, 1, 0, 0.5f, "", 2);
    Nexus n(Nexus::Size::S, Domain::Wt, "CHAR", "WT Character", v);
    parent.addAndMakeVisible(n);
    n.setBounds(0, 0, 84, 96);
    n.mouseDown(ev(n, knob, kLeft));
    n.mouseDrag(ev(n, knob.translated(0.0f, -22.0f), kLeft, 1, true));
    const float before = v.norm();
    n.mouseDrag(ev(n, knob.translated(0.0f, -22.0f), kShift, 1, true));
    const float atSwitch = v.norm();
    n.mouseDrag(ev(n, knob.translated(0.0f, -44.0f), kShift, 1, true));
    n.mouseUp(ev(n, knob, juce::ModifierKeys()));
    const bool ok = std::abs(atSwitch - before) < 1.0e-4f && std::abs(v.norm() - (before + 0.01f)) < 1.0e-3f && v.balanced();
    R.add("LAB-G09", ok, "fine mode re-anchors: " + juce::String(before, 3) + " -> " + juce::String(v.norm(), 3) + "; " + counts(v));
  }
  // LAB-G10 PLASMA : glisser, double-clic, destruction
  {
    DemoValue v(0, 1, 0.3f, 0.7f, "", 2);
    {
      auto p = std::make_unique<PlasmaChannel>(PlasmaChannel::Orientation::Vertical, Domain::Cross, "LVL", "CROSS level", v, juce::StringArray{"0", "1"});
      parent.addAndMakeVisible(*p);
      p->setBounds(0, 0, 64, 180);
      const juce::Point<float> c(14.0f, 90.0f);
      p->mouseDown(ev(*p, c, kLeft));
      p->mouseUp(ev(*p, c, juce::ModifierKeys()));
      p->mouseDown(ev(*p, c, kLeft, 2));
      p->mouseDoubleClick(ev(*p, c, kLeft, 2));
      p->mouseUp(ev(*p, c, juce::ModifierKeys(), 2));
      p->mouseDown(ev(*p, c, kLeft));
      p->mouseDrag(ev(*p, c.translated(0.0f, -11.0f), kLeft, 1, true));
    }
    R.add("LAB-G10", v.balanced() && v.nested == 0 && v.begins == 3, "plasma click, double click (default), drag then destroy; " + counts(v));
  }
  // LAB-G11 OD-01 : identification par identifiant stable, indépendante du texte visible et de la traduction
  {
    auto board = buildBoard();
    int tagged = 0;
    std::function<void(juce::Component&)> tag = [&](juce::Component& c) {
      if (auto* b = dynamic_cast<juce::Button*>(&c)) {
        if (b->getButtonText() == "PANIC") b->setComponentID("sbd.action.panic"), ++tagged;
        if (b->getButtonText() == "AUTHORIZE") b->setComponentID("sbd.action.authorize"), ++tagged;
      }
      for (auto* ch : c.getChildren()) tag(*ch);
    };
    tag(*board);
    auto* panic = dynamic_cast<juce::Button*>(findById(*board, "sbd.action.panic"));
    bool ok = panic != nullptr && tagged == 2;
    if (panic != nullptr) {
      const auto title = panic->getTitle();
      panic->setButtonText(juce::String::fromUTF8("PANIQUE"));  // traduction ou renommage de la sérigraphie
      ok = ok && findById(*board, "sbd.action.panic") == panic && panic->getTitle() == title;
    }
    R.add("LAB-G11", ok, "component ID finds PANIC after visible text change; accessible title unchanged");
  }
  // LAB-G12 rendu et navigation sans écriture : peindre la planche complète n'écrit aucune valeur
  {
    auto board = buildBoard();
    const auto img = board->createComponentSnapshot(board->getLocalBounds(), true, 1.0f);
    int writes = 0;
    // les sources DEMO DATA de la planche sont internes : on vérifie l'absence d'écriture par un NEXUS témoin
    DemoValue v(0, 1, 0, 0.4f, "", 2);
    Nexus n(Nexus::Size::XL, Domain::Wt, "WT Z", "WT Z Master", v);
    n.setBounds(0, 0, 132, 150);
    for (auto f : {Forced::Normal, Forced::Hover, Forced::Focus, Forced::Pressed, Forced::Disabled}) {
      n.setForced(f);
      (void)n.createComponentSnapshot(n.getLocalBounds(), true, 1.0f);
    }
    writes = v.writes + v.begins;
    R.add("LAB-G12", img.isValid() && writes == 0, "board rendered " + juce::String(img.getWidth()) + "x" + juce::String(img.getHeight()) +
                                                        ", forced states painted, writes " + juce::String(writes));
  }
  // LAB-G13 NEURAL CELL : bascule et sélection exclusive
  {
    NeuralCell tg(NeuralCell::Kind::Toggle, Domain::Wt, "SYNC", "WT Sync");
    tg.setToggleState(true, juce::sendNotificationSync);
    const bool on = tg.getToggleState() && tg.stateText() == "ON";
    juce::Component group;
    NeuralCell a(NeuralCell::Kind::Select, Domain::Mod, "LIN", "Curve LIN"), b(NeuralCell::Kind::Select, Domain::Mod, "EXP", "Curve EXP");
    group.addAndMakeVisible(a);
    group.addAndMakeVisible(b);
    a.setRadioGroupId(3);
    b.setRadioGroupId(3);
    a.setToggleState(true, juce::sendNotification);
    b.setToggleState(true, juce::sendNotification);
    const bool excl = b.getToggleState() && !a.getToggleState();
    NeuralCell arm(NeuralCell::Kind::Arm, Domain::Neutral, "ARM", "Armed");
    arm.setArmState(NeuralCell::ArmState::Armed);
    const bool armTxt = arm.stateText() == "ARMED";
    arm.setEnabled(false);
    const bool dis = arm.stateText() == juce::String::fromUTF8("\xe2\x80\x94");
    R.add("LAB-G13", on && excl && armTxt && dis, "toggle ON, exclusive selection, ARMED text, disabled shows dash");
  }
  out.addArray(R.lines);
  out.add("SELFTEST " + juce::String(R.pass) + " PASS, " + juce::String(R.fail) + " FAIL");
  return R.fail == 0 ? 0 : 1;
}

}  // namespace sbd::lab
