#include "MainEditor.h"

#include "BuildInfo.h"
#include "core/StateCodec.h"

namespace sbd::ui {

namespace {
bool visibleIn(const juce::Component* c, const juce::Component* root) {
  for (; c != nullptr; c = c->getParentComponent()) {
    if (!c->isVisible()) return false;
    if (c == root) return true;
  }
  return false;
}
juce::String fmtV(ParamId id, float v) { return juce::String(formatValue(id, v)); }
}  // namespace

// ================================================================ Destination Inspector
struct InspectorPanel::Impl {
  explicit Impl(UiContext& c) : pin(juce::String::fromUTF8("Épingler"), tok::mod), lock("Verrou Randomize", tok::mod), reset(juce::String::fromUTF8("Défaut"), tok::text), learn("MIDI Learn", tok::mod), entry("Saisie", tok::text), add("+ Modulation", tok::mod) {
    juce::ignoreUnused(c);
  }
  juce::Label title, info, mods;
  ActionCell pin, lock, reset, learn, entry, add;
  juce::ComboBox src, rate;
  juce::Slider depth;
  std::unique_ptr<juce::Component> alias;
  std::vector<std::unique_ptr<ActionCell>> removes;
  std::vector<std::uint32_t> removeIds;
};

InspectorPanel::InspectorPanel(UiContext& c) : impl_(std::make_unique<Impl>(c)), ctx_(c) {
  auto& m = *impl_;
  m.pin.setButtonText(juce::String::fromUTF8("Épingler"));
  m.reset.setButtonText(juce::String::fromUTF8("Défaut"));
  m.title.setFont(uiFont(tok::fontHead, true));
  m.title.setColour(juce::Label::textColourId, tok::text);
  for (auto* l : {&m.info, &m.mods}) {
    l->setFont(numFont(10.5f));
    l->setColour(juce::Label::textColourId, tok::text);
    l->setJustificationType(juce::Justification::topLeft);
  }
  m.pin.setClickingTogglesState(true);
  m.pin.setTooltip(juce::String::fromUTF8("Épinglé : la sélection d'une autre commande ne remplace pas ce contexte"));
  m.lock.setClickingTogglesState(true);
  m.lock.onClick = [this] {
    if (impl_->lock.getToggleState()) ctx_.random.locks.insert(id_);
    else ctx_.random.locks.erase(id_);
  };
  m.reset.setTooltip(juce::String::fromUTF8("Remet la valeur Base au défaut du registre (geste hôte, Undo)"));
  m.reset.onClick = [this] {
    if (auto* b = asBound(impl_->alias.get())) b->userSet(paramSpec(id_).defaultValue);
  };
  m.entry.onClick = [this] {
    if (auto* cc = dynamic_cast<ContinuousControl*>(impl_->alias.get())) cc->beginNumericEntry();
  };
  m.learn.onClick = [this] { ctx_.proc.setMidiLearnTarget(idx(id_)); };
  for (int s = 1; s < kNumModSources; ++s) m.src.addItem(modSourceLabel(static_cast<ModSource>(s)), s);
  m.src.setSelectedId(static_cast<int>(ModSource::Lfo1));
  m.src.setTitle("Source de modulation");
  m.rate.addItem("Control", 1);
  m.rate.addItem("Audio", 2);
  m.rate.setSelectedId(1);
  m.rate.setTitle("Cadence");
  m.depth.setSliderStyle(juce::Slider::LinearHorizontal);
  m.depth.setRange(-1.0, 1.0, 0.001);
  m.depth.setValue(0.25);
  m.depth.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
  m.depth.setTitle("Profondeur");
  m.add.onClick = [this] {
    ModRoute r;
    r.source = static_cast<ModSource>(impl_->src.getSelectedId());
    r.dest = id_;
    r.depth = static_cast<float>(impl_->depth.getValue());
    r.rate = impl_->rate.getSelectedId() == 2 ? ModRate::Audio : ModRate::Control;
    ctx_.proc.modAdd(r);
    refresh();
  };
  for (auto* x : std::initializer_list<juce::Component*>{&m.title, &m.info, &m.mods, &m.pin, &m.lock, &m.reset, &m.learn, &m.entry, &m.add, &m.src, &m.rate, &m.depth})
    addAndMakeVisible(*x);
  setTitle("Destination Inspector");
  show(ParamId::WtX);
}

InspectorPanel::~InspectorPanel() = default;
bool InspectorPanel::pinned() const { return impl_->pin.getToggleState(); }

void InspectorPanel::show(ParamId id) {
  auto& m = *impl_;
  if (pinned() && m.alias) return;  // contexte épinglé : la sélection ne le remplace pas
  if (id == id_ && m.alias) return;
  id_ = id;
  ctx_.inspected = id;
  m.alias = makeControl(ctx_, id, Density::S);
  addAndMakeVisible(*m.alias);
  m.lock.setToggleState(ctx_.random.locks.count(id) > 0, juce::dontSendNotification);
  const bool mod = paramModulatable(id);
  for (auto* c : std::initializer_list<juce::Component*>{&m.src, &m.rate, &m.depth, &m.add}) c->setEnabled(mod);
  m.learn.setEnabled(MidiLearnMap::learnableParam(idx(id)));
  m.entry.setEnabled(dynamic_cast<ContinuousControl*>(m.alias.get()) != nullptr);
  refresh();
  resized();
}

void InspectorPanel::refresh() {
  auto& m = *impl_;
  const auto& s = paramSpec(id_);
  m.title.setText(s.name, juce::dontSendNotification);
  const float base = ctx_.proc.parameters().base(idx(id_));
  juce::String t;
  t << "StableID   " << s.stableId << "\nchemin     " << s.group << juce::String::fromUTF8("\nunité      ") << (s.unit[0] ? s.unit : "-") << "\nplage      " << fmtV(id_, s.minValue)
    << " .. " << fmtV(id_, s.maxValue) << juce::String::fromUTF8("\ndéfaut     ") << fmtV(id_, s.defaultValue) << "\nBase       " << fmtV(id_, base);
  auto& tel = ctx_.proc.telemetry();
  if (tel.modulated[static_cast<size_t>(idx(id_))].load())
    t << "\nEffective  " << fmtV(id_, tel.effective[static_cast<size_t>(idx(id_))].load()) << juce::String::fromUTF8("  (moteur, échantillon décimé)");
  else
    t << juce::String::fromUTF8("\nEffective  = Base (non modulé)");
  if (id_ == ParamId::OutGain) {
    const float pk = std::max(tel.peakL.load(), tel.peakR.load());
    t << juce::String::fromUTF8("\nmesure     crête ") << (pk > 1e-6f ? juce::String(20.0f * std::log10(pk), 1) : juce::String("-inf")) << " dBFS (sortie)";
  }
  t << "\ncadences   " << ((s.rates & kRateEvent) ? "E " : "") << ((s.rates & kRateControl) ? "C " : "") << ((s.rates & kRateAudio) ? "A" : "") << "\nautomation "
    << (s.automatable ? "oui" : juce::String::fromUTF8("non (structurel)"));
  if (isAutonomyArm(id_)) t << juce::String::fromUTF8("\nARMEMENT AUTONOME : jamais modulable ; le clic de mise en marche vaut autorisation");
  const auto ml = ctx_.proc.midiLearn();
  for (int cc = 0; cc < 128; ++cc)
    if (ml.ccToParam[static_cast<size_t>(cc)] == idx(id_)) t << "\nMIDI       CC" << cc;
  if (ctx_.proc.midiLearnTarget() == idx(id_)) t << "\nMIDI       en attente d'un CC...";
  m.info.setText(t, juce::dontSendNotification);
  // modulations globales de cette destination (retrait par route)
  const auto mm = ctx_.proc.currentMod();
  const auto st = modRouteStatus(mm);
  juce::String txt;
  std::vector<std::uint32_t> ids;
  for (size_t k = 0; k < mm.routes.size(); ++k) {
    const auto& r = mm.routes[k];
    if (r.dest != id_) continue;
    ids.push_back(r.id);
    txt << "#" << static_cast<int>(r.id) << " " << modSourceLabel(r.source) << "  " << juce::String(r.depth, 3) << "  " << modCurveName(r.curve) << (r.rate == ModRate::Audio ? "  audio" : "  control");
    if (r.via != ModSource::None) txt << "  via " << modSourceLabel(r.via);
    txt << "\n   " << modInactiveText(st[k]) << "\n";
  }
  if (!paramModulatable(id_)) txt = juce::String::fromUTF8("Paramètre structurel / discret : jamais destination continue (C25).");
  else if (ids.empty()) txt = "Aucune modulation globale.";
  m.mods.setText(txt, juce::dontSendNotification);
  if (ids != m.removeIds) {
    m.removeIds = ids;
    m.removes.clear();
    for (auto rid : ids) {
      auto b = std::make_unique<ActionCell>(juce::String::fromUTF8("Retirer #") + juce::String(static_cast<int>(rid)), tok::alert);
      b->onClick = [this, rid] { ctx_.proc.modRemove(rid); };
      addAndMakeVisible(*b);
      m.removes.push_back(std::move(b));
    }
    resized();
  }
}

void InspectorPanel::resized() {
  auto& m = *impl_;
  auto r = getLocalBounds().reduced(tok::pad);
  m.title.setBounds(r.removeFromTop(24));
  auto row = r.removeFromTop(30);
  m.pin.setBounds(row.removeFromLeft(row.getWidth() / 2).reduced(1));
  m.lock.setBounds(row.reduced(1));
  r.removeFromTop(4);
  if (m.alias) {
    auto cell = preferredCell(id_, Density::S);
    m.alias->setBounds(r.removeFromTop(cell.getHeight()).withSizeKeepingCentre(std::min(r.getWidth(), std::max(cell.getWidth(), 120)), cell.getHeight()));
  }
  auto row2 = r.removeFromTop(30);
  const int w3 = row2.getWidth() / 3;
  m.reset.setBounds(row2.removeFromLeft(w3).reduced(1));
  m.entry.setBounds(row2.removeFromLeft(w3).reduced(1));
  m.learn.setBounds(row2.reduced(1));
  r.removeFromTop(4);
  m.info.setBounds(r.removeFromTop(196));
  m.src.setBounds(r.removeFromTop(28).reduced(0, 2));
  auto r2 = r.removeFromTop(28);
  m.rate.setBounds(r2.removeFromLeft(90).reduced(0, 2));
  m.depth.setBounds(r2.reduced(4, 2));
  m.add.setBounds(r.removeFromTop(30).reduced(0, 2));
  r.removeFromTop(4);
  auto rem = r.removeFromBottom(std::min(static_cast<int>(m.removes.size()), 4) * 28);
  for (size_t k = 0; k < m.removes.size(); ++k) {
    if (k < 4) m.removes[k]->setBounds(rem.removeFromTop(28).reduced(0, 2));
    m.removes[k]->setVisible(k < 4);
  }
  m.mods.setBounds(r);
}

void InspectorPanel::paint(juce::Graphics& g) {
  g.setColour(tok::panel);
  g.fillRect(getLocalBounds());
  g.setColour(tok::mod.withAlpha(0.6f));
  g.fillRect(0, 0, 2, getHeight());
  g.setColour(tok::panelEdge);
  g.drawVerticalLine(2, 0.0f, static_cast<float>(getHeight()));
}

// ================================================================ éditeur principal
MainEditor::MainEditor(SubdestrucktProcessor& p)
    : juce::AudioProcessorEditor(p), proc_(p), ctx_{p, nullptr, RandomOptions{}, {}, {}, {}, {}, ParamId::WtX, {}}, drone_(ctx_, "Drone", tok::warn) {
  setLookAndFeel(&look_);
  ctx_.learnCc.fill(-1);
  ctx_.select = [this](ParamId id) { selectParam(id); };
  ctx_.requestLearn = [this](ParamId id) { proc_.setMidiLearnTarget(idx(id)); };
  ctx_.navigate = [this](int s, int t) { showSubTab(s, t); };
  spaces_.setTabs({"PERFORMANCE", "SOUND DESIGN", "ENGINEERING"});
  spaces_.onChange = [this](int k) { showSpace(k); };
  preset_.setFont(uiFont(tok::fontTitle, true));
  preset_.setColour(juce::Label::textColourId, tok::text);
  preset_.setTooltip(juce::String::fromUTF8("Preset courant (* = modifié)"));
  status_.setFont(uiFont(10.5f));
  status_.setColour(juce::Label::textColourId, tok::textDim);
  meter_.setFont(numFont(10.5f));
  meter_.setJustificationType(juce::Justification::centredRight);
  prev_.setTitle(juce::String::fromUTF8("Preset précédent"));
  next_.setTitle("Preset suivant");
  prev_.onClick = [this] {
    const auto list = proc_.presets();
    if (list.empty()) return;
    presetIndex_ = (presetIndex_ + static_cast<int>(list.size()) - 1) % static_cast<int>(list.size());
    proc_.loadPreset(list[static_cast<size_t>(presetIndex_)]);
  };
  next_.onClick = [this] {
    const auto list = proc_.presets();
    if (list.empty()) return;
    presetIndex_ = (presetIndex_ + 1) % static_cast<int>(list.size());
    proc_.loadPreset(list[static_cast<size_t>(presetIndex_)]);
  };
  open_.setTooltip(juce::String::fromUTF8("Bibliothèque de presets (Engineering › Presets / State)"));
  save_.setTooltip(juce::String::fromUTF8("Enregistrer un preset (Engineering › Presets / State)"));
  open_.onClick = [this] { showSubTab(2, 5); };
  save_.onClick = [this] { showSubTab(2, 5); };
  undo_.onClick = [this] { proc_.undo(); };
  redo_.onClick = [this] { proc_.redo(); };
  panic_.setTooltip(juce::String::fromUTF8("PANIC : All Sound Off, désarme le Drone et révoque l'autorisation des états autonomes"));
  panic_.onClick = [this] { proc_.requestPanic(); };
  inspectorBtn_.setClickingTogglesState(true);
  inspectorBtn_.setToggleState(true, juce::dontSendNotification);
  inspectorBtn_.onClick = [this] { setInspectorOpen(inspectorBtn_.getToggleState()); };
  for (int q = 0; q < 3; ++q) quality_.addItem(juce::String("Quality ") + qualityName(static_cast<Quality>(q)), q + 1);
  quality_.setSelectedId(static_cast<int>(proc_.quality()) + 1, juce::dontSendNotification);
  quality_.setTitle("Quality");
  quality_.setTooltip(juce::String::fromUTF8("Quality : préférence d'instance/projet (suréchantillonnage), distincte de Clean/Raw"));
  quality_.onChange = [this] { proc_.setQuality(static_cast<Quality>(quality_.getSelectedId() - 1)); };
  for (int z : {75, 100, 125, 150}) zoom_.addItem(juce::String(z) + " %", z);
  zoom_.setSelectedId(100, juce::dontSendNotification);
  zoom_.setTitle("Zoom");
  zoom_.onChange = [this] { setZoomPercent(zoom_.getSelectedId()); };
  // clic de l'utilisateur : intention explicite (autorisation sans confirmation, TA-07w)
  drone_.onUserToggle = [this](bool on) {
    proc_.setDrone(on);
    if (on) proc_.authorizeAutonomy();
  };
  drone_.setTooltip(juce::String::fromUTF8("Drone : maintient le gate. Clic = configuration + autorisation explicite ; Panic désarme"));
  authorize_.setTooltip(juce::String::fromUTF8("États autonomes suspendus (armements ou captures restaurés par le projet, la duplication, un preset ou le Panic) : "
                                               "cliquer pour autoriser leur activation dans cette instance"));
  authorize_.setTitle(juce::String::fromUTF8("Autoriser les états autonomes"));
  authorize_.onClick = [this] { proc_.authorizeAutonomy(); };
  for (auto* c : std::initializer_list<juce::Component*>{&prev_, &preset_, &next_, &open_, &save_, &undo_, &redo_, &spaces_, &status_, &meter_, &quality_, &drone_, &zoom_,
                                                         &inspectorBtn_, &panic_})
    bar_.addAndMakeVisible(*c);
  bar_.addChildComponent(authorize_);
  addAndMakeVisible(bar_);
  views_[0] = makePerformanceView(ctx_);
  views_[1] = makeSoundDesignView(ctx_);
  views_[2] = makeEngineeringView(ctx_);
  inspector_ = std::make_unique<InspectorPanel>(ctx_);
  vp_.setScrollBarsShown(true, true);
  vp_.setScrollBarThickness(10);
  content_.addAndMakeVisible(vp_);
  addAndMakeVisible(content_);
  addAndMakeVisible(*inspector_);
  juce::Component::SafePointer<MainEditor> safe(this);
  proc_.setStateListener([safe] {
    juce::MessageManager::callAsync([safe] {
      if (auto* e = safe.getComponent())
        for (auto& v : e->views_) v->refreshState();
    });
  });
  setResizable(true, true);
  setResizeLimits(960, 640, 2560, 1600);  // minimum candidat 960 x 640 (ARCH-UI-03), vérifié par UI04-SIZE
  setSize(1440, 900);
  showSpace(0);
  startTimerHz(tok::refreshHz);
}

MainEditor::~MainEditor() {
  stopTimer();
  proc_.setStateListener(nullptr);
  vp_.setViewedComponent(nullptr, false);
  setLookAndFeel(nullptr);
}

void MainEditor::selectParam(ParamId id) {
  if (inspector_) inspector_->show(id);
}

juce::Component* MainEditor::currentView() { return views_[space_].get(); }
int MainEditor::numTabs(int s) const { return views_[juce::jlimit(0, 2, s)]->numTabs(); }
int MainEditor::numSubContexts(int s, int t) const { return views_[juce::jlimit(0, 2, s)]->numSubContexts(t); }
juce::String MainEditor::contextName(int s, int t, int sub) const {
  auto& v = *views_[juce::jlimit(0, 2, s)];
  juce::String n = (s == 0 ? "PERFORMANCE" : s == 1 ? "SOUND DESIGN" : "ENGINEERING");
  n << " / " << v.tabName(t);
  const auto sc = v.subContextName(t, sub);
  if (sc.isNotEmpty()) n << " / " << sc;
  return n;
}

void MainEditor::showSpace(int k) {
  space_ = juce::jlimit(0, 2, k);
  spaces_.setCurrent(space_);
  vp_.setViewedComponent(views_[space_].get(), false);
  views_[space_]->refreshState();
  layoutContent();
}

void MainEditor::showSubTab(int s, int tab) {
  showSpace(s);
  views_[space_]->showTab(juce::jlimit(0, views_[space_]->numTabs() - 1, tab));
  layoutContent();
}

void MainEditor::showSubContext(int s, int tab, int sub) {
  showSubTab(s, tab);
  views_[space_]->showSubContext(tab, sub);
  layoutContent();
}

void MainEditor::setZoomPercent(int pct) {
  zoomF_ = juce::jlimit(75, 150, pct) / 100.0f;
  zoom_.setSelectedId(juce::jlimit(75, 150, pct), juce::dontSendNotification);
  layoutContent();
}

void MainEditor::setInspectorOpen(bool open) {
  inspectorOpen_ = open;
  inspectorBtn_.setToggleState(open, juce::dontSendNotification);
  resized();
}

void MainEditor::paint(juce::Graphics& g) {
  g.fillAll(tok::chassis);
  // barre : titane noir usiné, filet inférieur
  g.setColour(tok::carbon);
  g.fillRect(0, 0, getWidth(), barH_);
  g.setColour(tok::panelEdge);
  g.drawHorizontalLine(barH_ - 1, 0.0f, static_cast<float>(getWidth()));
  g.setColour(tok::cross.withAlpha(0.55f));
  g.fillRect(0, barH_ - 1, 120, 1);
}

void MainEditor::layoutBar() {
  // largeur préférée d'une rangée unique ; sinon deux rangées (Panic toujours visible, jamais masqué)
  const int wLeft = 24 + 180 + 24 + 70 + 92 + 56 + 56 + 8;
  const int wRight = 130 + 96 + 88 + 82 + 84 + 92 + (authorize_.isVisible() ? 92 : 0);
  const int wSpaces = spaces_.preferredWidth() + 8;
  const bool twoRows = getWidth() < wLeft + wSpaces + wRight + 160;
  barH_ = twoRows ? 2 * tok::barH - 6 : tok::barH;
  bar_.setBounds(0, 0, getWidth(), barH_);
  auto b = bar_.getLocalBounds().reduced(6, 5);
  auto row1 = twoRows ? b.removeFromTop((b.getHeight() - 4) / 2) : b;
  if (twoRows) b.removeFromTop(4);
  auto row2 = twoRows ? b : b;
  panic_.setBounds(row2.removeFromRight(92).reduced(1));
  row2.removeFromRight(4);
  inspectorBtn_.setBounds(row2.removeFromRight(84).reduced(1));
  zoom_.setBounds(row2.removeFromRight(82).reduced(2, 4));
  if (authorize_.isVisible()) authorize_.setBounds(row2.removeFromRight(92).reduced(1));
  drone_.setBounds(row2.removeFromRight(96).reduced(1));
  quality_.setBounds(row2.removeFromRight(130).reduced(2, 4));
  if (twoRows) spaces_.setBounds(row2.removeFromLeft(std::min(row2.getWidth(), wSpaces)).reduced(0, 1));
  prev_.setBounds(row1.removeFromLeft(24).reduced(0, 2));
  preset_.setBounds(row1.removeFromLeft(180));
  next_.setBounds(row1.removeFromLeft(24).reduced(0, 2));
  row1.removeFromLeft(6);
  open_.setBounds(row1.removeFromLeft(70).reduced(1));
  save_.setBounds(row1.removeFromLeft(92).reduced(1));
  row1.removeFromLeft(6);
  undo_.setBounds(row1.removeFromLeft(56).reduced(1));
  redo_.setBounds(row1.removeFromLeft(56).reduced(1));
  row1.removeFromLeft(8);
  if (!twoRows) spaces_.setBounds(row1.removeFromLeft(wSpaces).reduced(0, 1));
  else if (row2.getWidth() > 120) meter_.setBounds(row2.removeFromRight(std::min(170, row2.getWidth() - 10)));
  if (!twoRows) {
    meter_.setBounds(row1.removeFromRight(std::min(170, row1.getWidth() / 2)));
    meter_.setVisible(row1.getWidth() > 60);
  } else {
    meter_.setVisible(meter_.getWidth() > 60);
  }
  status_.setBounds(row1.reduced(6, 0));
  status_.setVisible(row1.getWidth() > 80);
}

void MainEditor::layoutContent() {
  auto r = getLocalBounds().withTrimmedTop(barH_);
  if (inspectorOpen_) inspector_->setBounds(r.removeFromRight(tok::inspectorW));
  inspector_->setVisible(inspectorOpen_);
  // zoom : le contenu est disposé à la taille logique puis mis à l'échelle ; défilement local si la taille logique est
  // inférieure au minimum de l'espace (aucun chevauchement, aucune commande inaccessible)
  const int lw = static_cast<int>(r.getWidth() / zoomF_), lh = static_cast<int>(r.getHeight() / zoomF_);
  content_.setBounds(0, 0, lw, lh);
  content_.setTransform(juce::AffineTransform::scale(zoomF_).translated(static_cast<float>(r.getX()), static_cast<float>(r.getY())));
  vp_.setBounds(0, 0, lw, lh);
  auto* v = views_[space_].get();
  const auto mn = views_[space_]->minimumSize();
  const int sb = vp_.getScrollBarThickness();
  int w = std::max(mn.x, lw), h = std::max(mn.y, lh);
  if (w > lw) h = std::max(mn.y, lh - sb);
  if (h > lh) w = std::max(mn.x, lw - sb);
  v->setBounds(0, 0, w, h);
}

void MainEditor::resized() {
  layoutBar();
  layoutContent();
}

void MainEditor::refreshBar() {
  preset_.setText(juce::String(proc_.presetName()) + (proc_.dirty() ? " *" : ""), juce::dontSendNotification);
  undo_.setEnabled(proc_.canUndo());
  redo_.setEnabled(proc_.canRedo());
  undo_.setTooltip("Undo : " + juce::String(proc_.undoLabel()));
  redo_.setTooltip("Redo : " + juce::String(proc_.redoLabel()));
  const bool drone = proc_.drone();
  drone_.setToggleState(drone, juce::dontSendNotification);
  drone_.setCellState(!drone ? CellState::Off : (proc_.autonomyAuthorized() ? CellState::Active : CellState::Armed));
  const bool susp = proc_.autonomySuspended();
  if (authorize_.isVisible() != susp) {
    authorize_.setVisible(susp);
    layoutBar();
  }
  if (quality_.getSelectedId() != static_cast<int>(proc_.quality()) + 1) quality_.setSelectedId(static_cast<int>(proc_.quality()) + 1, juce::dontSendNotification);
  auto& t = proc_.telemetry();
  const float pk = std::max(t.peakL.load(), t.peakR.load());
  const float db = pk > 1e-6f ? 20.0f * std::log10(pk) : -120.0f;
  const auto elp = t.elpHits.load();
  meter_.setText("OUT " + (db > -119.0f ? juce::String(db, 1) : juce::String("-inf")) + " dBFS" + (db > 0.0f ? " >0" : "") + (elp > 0 ? "  ELP " + juce::String(static_cast<int>(elp)) : juce::String()),
                 juce::dontSendNotification);
  meter_.setColour(juce::Label::textColourId, (db > 0.0f || elp > 0) ? tok::alert : tok::textDim);
  status_.setText(proc_.lastMessage(), juce::dontSendNotification);
}

void MainEditor::timerCallback() {
  refreshBar();
  if (ticks_++ % 3 == 0) {  // marqueurs M/CC des commandes (10 Hz)
    std::array<std::uint8_t, kNumParams> mc{};
    for (const auto& r : proc_.currentMod().routes)
      if (r.dest != ParamId::Count) mc[static_cast<size_t>(idx(r.dest))] = static_cast<std::uint8_t>(std::min(255, mc[static_cast<size_t>(idx(r.dest))] + 1));
    std::array<std::int16_t, kNumParams> lc{};
    lc.fill(-1);
    const auto ml = proc_.midiLearn();
    for (int cc = 0; cc < 128; ++cc)
      if (ml.ccToParam[static_cast<size_t>(cc)] >= 0) lc[static_cast<size_t>(ml.ccToParam[static_cast<size_t>(cc)])] = static_cast<std::int16_t>(cc);
    if (mc != ctx_.modCount || lc != ctx_.learnCc) {
      ctx_.modCount = mc;
      ctx_.learnCc = lc;
      for (auto* b : ctx_.registry) b->asComponent().repaint();
    }
    if (inspectorOpen_) inspector_->refresh();
  }
  views_[space_]->tick();
  // télémétrie : seules les commandes visibles sont rafraîchies (aucune écriture DSP)
  const auto reg = ctx_.registry;
  for (auto* b : reg)
    if (visibleIn(&b->asComponent(), this)) b->tick();
}

}  // namespace sbd::ui
