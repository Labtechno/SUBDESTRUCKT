// SUBDESTRUCKT — UI — espace ENGINEERING (ARCH-UI-04) : FM Network, Cross Network (routes spécialisées, topologie
// causale B-T2 en lecture seule : création/retrait par transaction validée), matrice de modulation globale (64 liens,
// distincte des routes spécialisées), Bank Laboratory, Voice/Tuning, Presets/State, Randomize/Mutate/Evolve, MIDI Learn,
// Diagnostics, Captures/Assets. Aucune action autonome déclenchée par la navigation.
#include "BuildInfo.h"
#include "Views.h"

namespace sbd::ui {

namespace {

juce::Label& styled(juce::Label& l, float h = 10.5f, bool numeric = true, juce::Colour c = tok::text) {
  l.setFont(numeric ? numFont(h) : uiFont(h));
  l.setColour(juce::Label::textColourId, c);
  l.setJustificationType(juce::Justification::topLeft);
  return l;
}

bool classifyPreview(const std::vector<Route>& routes, int src, int dst) {
  RouteTable t;
  std::uint32_t next = 1;
  for (const auto& r : routes) next = std::max(next, r.id + 1);
  std::string err;
  t.assign(routes, next, err);
  return t.classify(src, dst) == CausalClass::Delayed;
}

// ================================================================ cellule de réseau (route spécialisée)
class NetCell : public juce::Component {
 public:
  NetCell(UiContext& c, int slot) : ctx_(c), slot_(slot), depth_(c, routeSlotInfo(slot).depthParam, Form::Rotary, Density::Micro), toggle_("+", tok::cross) {
    const auto& info = routeSlotInfo(slot);
    depth_.setCaption(juce::String(nodeName(info.src)) + juce::String::fromUTF8("›") + (info.src == info.dst ? juce::String("self") : juce::String(portName(info.dst))));
    toggle_.onClick = [this] {
      const auto& i = routeSlotInfo(slot_);
      if (found_) ctx_.proc.applyTopology({RouteTable::Op::remove(foundId_)});
      else ctx_.proc.applyTopology({RouteTable::Op::add(i.src, i.dst)});
    };
    addAndMakeVisible(depth_);
    addAndMakeVisible(toggle_);
  }
  void refresh(const std::vector<Route>& routes) {
    const auto& info = routeSlotInfo(slot_);
    const Route* f = nullptr;
    for (const auto& r : routes)
      if (r.src == info.src && r.dst == info.dst) f = &r;
    found_ = f != nullptr;
    foundId_ = f ? f->id : 0;
    const auto& t = ctx_.proc.telemetry();
    juce::String s;
    if (f) s << (f->cls == CausalClass::Delayed ? "D (T2)" : "I") << "  id " << static_cast<int>(f->id) << "\neff " << juce::String(t.effectiveDepth[static_cast<size_t>(slot_)].load(), 2);
    else s << juce::String::fromUTF8("absente\nsi ajoutée : ") << (classifyPreview(routes, info.src, info.dst) ? "D" : "I");
    if (s != text_) {
      text_ = s;
      repaint();
    }
    toggle_.setButtonText(found_ ? juce::String::fromUTF8("− route") : "+ route");
    toggle_.setTooltip(found_ ? juce::String::fromUTF8("Retirer la route (transaction validée, Undo)") : juce::String::fromUTF8("Ajouter la route (classe causale B-T2 calculée, Undo)"));
    depth_.setInactive(!found_, juce::String::fromUTF8("route absente : profondeur conservée sans effet"));
  }
  void resized() override {
    auto r = getLocalBounds();
    toggle_.setBounds(r.removeFromBottom(22).reduced(2, 0));
    r.removeFromBottom(22);
    depth_.setBounds(r);
  }
  void paint(juce::Graphics& g) override {
    auto r = getLocalBounds().toFloat();
    g.setColour(found_ ? tok::cross.withAlpha(0.08f) : tok::carbon.withAlpha(0.4f));
    g.fillRoundedRectangle(r, tok::radiusSmall);
    g.setColour(found_ ? tok::cross.withAlpha(0.5f) : tok::glassEdge);
    g.drawRoundedRectangle(r.reduced(0.5f), tok::radiusSmall, 1.0f);
    g.setFont(numFont(9.5f));
    g.setColour(found_ ? tok::text : tok::textMute);
    g.drawFittedText(text_, r.withTrimmedBottom(22.0f).removeFromBottom(22.0f).toNearestInt(), juce::Justification::centred, 2, 1.0f);
  }

 private:
  UiContext& ctx_;
  int slot_;
  ContinuousControl depth_;
  ActionCell toggle_;
  bool found_ = false;
  std::uint32_t foundId_ = 0;
  juce::String text_;
};

class NetGrid : public juce::Component {
 public:
  // lignes = sources ; colonnes = ports destination ; cellule vide si la paire n'est pas un slot autorisé
  NetGrid(UiContext& c, std::vector<int> srcs, std::vector<int> ports, bool internal) : srcs_(std::move(srcs)), ports_(std::move(ports)) {
    for (int s : srcs_)
      for (int p : ports_) {
        const int slot = routeSlot(s, p);
        const bool ok = slot >= 0 && (internal ? routeSlotInfo(slot).kind == RouteKind::Internal : routeSlotInfo(slot).kind != RouteKind::Internal);
        cells_.push_back(ok ? std::make_unique<NetCell>(c, slot) : nullptr);
        if (cells_.back()) addAndMakeVisible(*cells_.back());
      }
  }
  void refresh(const std::vector<Route>& routes) {
    for (auto& c : cells_)
      if (c) c->refresh(routes);
  }
  int cellW() const { return 84; }
  int cellH() const { return tok::nexusMicro.h + 48; }
  juce::Point<int> preferred() const { return {58 + static_cast<int>(ports_.size()) * (cellW() + 4), 20 + static_cast<int>(srcs_.size()) * (cellH() + 4)}; }
  void resized() override {
    size_t k = 0;
    for (size_t i = 0; i < srcs_.size(); ++i)
      for (size_t j = 0; j < ports_.size(); ++j, ++k)
        if (cells_[k]) cells_[k]->setBounds(58 + static_cast<int>(j) * (cellW() + 4), 20 + static_cast<int>(i) * (cellH() + 4), cellW(), cellH());
  }
  void paint(juce::Graphics& g) override {
    g.setFont(uiFont(tok::fontLabel, true));
    g.setColour(tok::textDim);
    for (size_t j = 0; j < ports_.size(); ++j)
      g.drawText(juce::String::fromUTF8("› ") + portName(ports_[j]), 58 + static_cast<int>(j) * (cellW() + 4), 0, cellW(), 18, juce::Justification::centred, false);
    for (size_t i = 0; i < srcs_.size(); ++i)
      g.drawText(nodeName(srcs_[i]), 0, 20 + static_cast<int>(i) * (cellH() + 4), 54, cellH(), juce::Justification::centredLeft, false);
  }

 private:
  std::vector<int> srcs_, ports_;
  std::vector<std::unique_ptr<NetCell>> cells_;
};

// ================================================================ matrice de modulation globale (64 liens)
class ModRow : public juce::Component {
 public:
  ModRow(UiContext& c, std::uint32_t id) : ctx_(c), id_(id), remove_(juce::String::fromUTF8("Retirer"), tok::alert), sel_("Courbe", tok::mod) {
    for (int s = 1; s < kNumModSources; ++s) src_.addItem(modSourceLabel(static_cast<ModSource>(s)), s);
    via_.addItem("(aucun)", 1);
    for (int s = 1; s < kNumModSources; ++s) via_.addItem(modSourceLabel(static_cast<ModSource>(s)), s + 1);
    for (int k = 0; k < static_cast<int>(ModCurve::Count); ++k) curve_.addItem(modCurveName(static_cast<ModCurve>(k)), k + 1);
    rate_.addItem("Control", 1);
    rate_.addItem("Audio", 2);
    depth_.setSliderStyle(juce::Slider::LinearHorizontal);
    depth_.setRange(-1.0, 1.0, 0.001);
    depth_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    depth_.setDoubleClickReturnValue(true, 0.0);
    depth_.setColour(juce::Slider::trackColourId, tok::mod);
    on_.setButtonText("actif");
    src_.setTitle("Source");
    via_.setTitle("Via");
    curve_.setTitle("Courbe");
    rate_.setTitle("Cadence");
    depth_.setTitle("Profondeur");
    auto commit = [this] { push(); };
    src_.onChange = commit;
    via_.onChange = commit;
    curve_.onChange = commit;
    rate_.onChange = commit;
    on_.onClick = commit;
    depth_.onDragEnd = commit;
    depth_.onValueChange = [this] {
      if (!depth_.isMouseButtonDown() && !updating_) push();
    };
    dest_.onClick = [this] {
      if (ctx_.select) ctx_.select(route_.dest);
    };
    remove_.onClick = [this] { ctx_.proc.modRemove(id_); };
    sel_.onClick = [this] {
      if (onEditCurve) onEditCurve(id_);
    };
    for (auto* x : std::initializer_list<juce::Component*>{&src_, &dest_, &depth_, &curve_, &rate_, &via_, &on_, &remove_, &sel_, &status_}) addAndMakeVisible(*x);
    styled(status_, 9.5f);
  }
  std::uint32_t id() const { return id_; }
  void set(const ModRoute& r, ModInactive st) {
    route_ = r;
    updating_ = true;
    src_.setSelectedId(static_cast<int>(r.source), juce::dontSendNotification);
    via_.setSelectedId(static_cast<int>(r.via) + 1, juce::dontSendNotification);
    curve_.setSelectedId(static_cast<int>(r.curve) + 1, juce::dontSendNotification);
    rate_.setSelectedId(r.rate == ModRate::Audio ? 2 : 1, juce::dontSendNotification);
    if (!depth_.isMouseButtonDown()) depth_.setValue(r.depth, juce::dontSendNotification);
    on_.setToggleState(r.enabled, juce::dontSendNotification);
    dest_.setButtonText(juce::String::fromUTF8("› ") + paramSpec(r.dest).name);
    dest_.setTooltip(juce::String(paramSpec(r.dest).stableId) + juce::String::fromUTF8(" — ouvrir dans l'Inspector"));
    status_.setText("#" + juce::String(static_cast<int>(r.id)) + "  " + modInactiveText(st), juce::dontSendNotification);
    status_.setColour(juce::Label::textColourId, st == ModInactive::Active ? tok::textDim : tok::warn);
    updating_ = false;
  }
  std::function<void(std::uint32_t)> onEditCurve;
  void resized() override {
    auto r = getLocalBounds().reduced(2);
    src_.setBounds(r.removeFromLeft(118).reduced(1));
    dest_.setBounds(r.removeFromLeft(190).reduced(1));
    depth_.setBounds(r.removeFromLeft(170).reduced(1));
    curve_.setBounds(r.removeFromLeft(96).reduced(1));
    sel_.setBounds(r.removeFromLeft(62).reduced(1));
    rate_.setBounds(r.removeFromLeft(78).reduced(1));
    via_.setBounds(r.removeFromLeft(110).reduced(1));
    on_.setBounds(r.removeFromLeft(58).reduced(1));
    remove_.setBounds(r.removeFromRight(72).reduced(1));
    status_.setBounds(r.reduced(4, 0));
  }
  void paint(juce::Graphics& g) override {
    g.setColour(tok::carbon.withAlpha(0.45f));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), tok::radiusSmall);
  }

 private:
  void push() {
    if (updating_) return;
    ModRoute r = route_;
    r.source = static_cast<ModSource>(src_.getSelectedId());
    r.via = static_cast<ModSource>(via_.getSelectedId() - 1);
    r.curve = static_cast<ModCurve>(curve_.getSelectedId() - 1);
    r.rate = rate_.getSelectedId() == 2 ? ModRate::Audio : ModRate::Control;
    r.depth = static_cast<float>(depth_.getValue());
    r.enabled = on_.getToggleState();
    if (!(r == route_)) ctx_.proc.modUpdate(r);
  }
  UiContext& ctx_;
  std::uint32_t id_;
  ModRoute route_;
  bool updating_ = false;
  juce::ComboBox src_, via_, curve_, rate_;
  juce::TextButton dest_;
  juce::Slider depth_;
  juce::ToggleButton on_;
  ActionCell remove_, sel_;
  juce::Label status_;
};

class ModMatrixPage : public juce::Component {
 public:
  explicit ModMatrixPage(UiContext& c) : ctx_(c), add_("Ajouter", tok::mod), fromInspector_(juce::String::fromUTF8("= Inspector"), tok::mod), curveEd_("COURBE CUSTOM DE LA ROUTE", tok::mod, false) {
    for (int s = 1; s < kNumModSources; ++s) src_.addItem(modSourceLabel(static_cast<ModSource>(s)), s);
    src_.setSelectedId(static_cast<int>(ModSource::Lfo1));
    for (int i = 0; i < kNumParams; ++i)
      if (paramModulatable(static_cast<ParamId>(i))) dst_.addItem(juce::String(paramSpec(i).name) + "  (" + paramSpec(i).stableId + ")", i + 1);
    dst_.setSelectedId(idx(ParamId::WtX) + 1);
    rate_.addItem("Control", 1);
    rate_.addItem("Audio", 2);
    rate_.setSelectedId(1);
    depth_.setSliderStyle(juce::Slider::LinearHorizontal);
    depth_.setRange(-1.0, 1.0, 0.001);
    depth_.setValue(0.3);
    depth_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    filter_.setTextToShowWhenEmpty("filtrer (source ou destination)", tok::textMute);
    filter_.onTextChange = [this] { rebuild(true); };
    fromInspector_.onClick = [this] { dst_.setSelectedId(idx(ctx_.inspected) + 1); };
    add_.onClick = [this] {
      ModRoute r;
      r.source = static_cast<ModSource>(src_.getSelectedId());
      r.dest = static_cast<ParamId>(dst_.getSelectedId() - 1);
      r.depth = static_cast<float>(depth_.getValue());
      r.rate = rate_.getSelectedId() == 2 ? ModRate::Audio : ModRate::Control;
      ctx_.proc.modAdd(r);
      rebuild(true);
    };
    curveEd_.onCommit = [this](int n, const float* x, const float* y) {
      const auto m = ctx_.proc.currentMod();
      for (auto r : m.routes)
        if (r.id == curveRoute_) {
          r.curve = ModCurve::Custom;
          r.numPoints = static_cast<std::uint8_t>(n);
          for (int i = 0; i < n; ++i) r.px[static_cast<size_t>(i)] = x[i], r.py[static_cast<size_t>(i)] = y[i];
          ctx_.proc.modUpdate(r);
        }
    };
    for (auto* x : std::initializer_list<juce::Component*>{&src_, &dst_, &fromInspector_, &rate_, &depth_, &add_, &filter_, &summary_, &vp_}) addAndMakeVisible(*x);
    addChildComponent(curveEd_);
    styled(summary_, 10.5f);
    vp_.setViewedComponent(&list_, false);
    vp_.setScrollBarsShown(true, false);
  }
  void rebuild(bool force) {
    const auto m = ctx_.proc.currentMod();
    const auto st = modRouteStatus(m);
    std::vector<std::uint32_t> ids;
    const auto f = filter_.getText().toLowerCase();
    for (const auto& r : m.routes) {
      const juce::String hay = (juce::String(modSourceLabel(r.source)) + " " + paramSpec(r.dest).name + " " + paramSpec(r.dest).stableId).toLowerCase();
      if (f.isEmpty() || hay.contains(f)) ids.push_back(r.id);
    }
    if (force || ids != ids_) {
      ids_ = ids;
      rows_.clear();
      for (auto id : ids) {
        rows_.push_back(std::make_unique<ModRow>(ctx_, id));
        rows_.back()->onEditCurve = [this](std::uint32_t rid) {
          curveRoute_ = rid;
          curveEd_.setVisible(true);
          resized();
        };
        list_.addAndMakeVisible(*rows_.back());
      }
      layoutRows();
    }
    int audio = 0;
    for (size_t k = 0; k < m.routes.size(); ++k) {
      if (m.routes[k].rate == ModRate::Audio && st[k] == ModInactive::Active) ++audio;
      for (auto& row : rows_)
        if (row->id() == m.routes[k].id) row->set(m.routes[k], st[k]);
      if (m.routes[k].id == curveRoute_) curveEd_.setPoints(m.routes[k].numPoints, m.routes[k].px.data(), m.routes[k].py.data());
    }
    summary_.setText(juce::String::fromUTF8("MATRICE GLOBALE  ") + juce::String(static_cast<int>(m.routes.size())) + juce::String::fromUTF8("/64 liens (macros comprises)  ·  audio-rate actifs ") +
                         juce::String(audio) + juce::String::fromUTF8("/16  ·  routes spécialisées FM/CROSS : FM Network / Cross Network (distinctes)"),
                     juce::dontSendNotification);
  }
  void layoutRows() {
    const int w = std::max(1000, vp_.getWidth() - 12);
    for (size_t k = 0; k < rows_.size(); ++k) rows_[k]->setBounds(0, static_cast<int>(k) * 32, w, 30);
    list_.setSize(w, std::max(40, static_cast<int>(rows_.size()) * 32));
  }
  void resized() override {
    auto r = getLocalBounds();
    auto a = r.removeFromTop(30);
    src_.setBounds(a.removeFromLeft(150).reduced(2));
    dst_.setBounds(a.removeFromLeft(300).reduced(2));
    fromInspector_.setBounds(a.removeFromLeft(100).reduced(2));
    rate_.setBounds(a.removeFromLeft(90).reduced(2));
    depth_.setBounds(a.removeFromLeft(200).reduced(2));
    add_.setBounds(a.removeFromLeft(90).reduced(2));
    filter_.setBounds(a.reduced(2));
    summary_.setBounds(r.removeFromTop(22));
    if (curveEd_.isVisible()) curveEd_.setBounds(r.removeFromBottom(150).removeFromLeft(420));
    vp_.setBounds(r);
    layoutRows();
  }

 private:
  UiContext& ctx_;
  juce::ComboBox src_, dst_, rate_;
  juce::Slider depth_;
  ActionCell add_, fromInspector_;
  juce::TextEditor filter_;
  juce::Label summary_;
  juce::Viewport vp_;
  juce::Component list_;
  std::vector<std::unique_ptr<ModRow>> rows_;
  std::vector<std::uint32_t> ids_;
  PointCurveEditor curveEd_;
  std::uint32_t curveRoute_ = 0;
};

// ================================================================ Bank Laboratory : éditeur de formes (ARCH-G4.1 §3)
class WaveEditor : public juce::Component {
 public:
  static constexpr int kPoints = 64;
  explicit WaveEditor(UiContext& c) : ctx_(c), apply_("Appliquer la forme", tok::wt), revert_("Reprendre", tok::text), gen_(juce::String::fromUTF8("Générer"), tok::wt),
                                      saveLib_("Enregistrer .sdwt", tok::wt), loadLib_("Charger", tok::wt) {
    styled(info_, 10.5f, true, tok::textDim);
    wave_.setSliderStyle(juce::Slider::LinearHorizontal);
    wave_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 48, 18);
    wave_.setTitle("Forme");
    wave_.onValueChange = [this] { loadWave(); };
    apply_.setTooltip(juce::String::fromUTF8("Remplace la forme sélectionnée dans une copie de la banque (nouvelle banque, annulable)"));
    apply_.onClick = [this] {
      std::vector<std::pair<double, double>> pts;
      for (int i = 0; i < kPoints; ++i) pts.emplace_back(static_cast<double>(i) / kPoints, pts_[static_cast<size_t>(i)]);
      ctx_.proc.editCurrentWave(static_cast<int>(wave_.getValue()), drawnWave(pts));
    };
    revert_.onClick = [this] { loadWave(); };
    for (int k = 0; k < static_cast<int>(WtProcedure::Count); ++k) proc_.addItem(procedureName(static_cast<WtProcedure>(k)), k + 1);
    proc_.setSelectedId(1);
    grid_.addItem("16 x 1", 1);
    grid_.addItem("16 x 4", 2);
    grid_.addItem("64 x 16", 3);
    grid_.setSelectedId(2);
    for (auto* s : {&seed_, &amount_}) {
      s->setSliderStyle(juce::Slider::LinearHorizontal);
      s->setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
    }
    seed_.setRange(0, 999, 1);
    seed_.setTitle("Graine");
    amount_.setRange(0.0, 1.0, 0.01);
    amount_.setValue(0.6);
    amount_.setTitle(juce::String::fromUTF8("Intensité"));
    gen_.onClick = [this] {
      const int g = grid_.getSelectedId();
      const int nx = g == 3 ? 64 : 16, ny = g == 1 ? 1 : (g == 2 ? 4 : 16);
      ctx_.proc.generateBank(static_cast<WtProcedure>(proc_.getSelectedId() - 1), nx, ny, static_cast<std::uint64_t>(seed_.getValue()), amount_.getValue());
    };
    name_.setTextToShowWhenEmpty("nom de la banque", tok::textMute);
    saveLib_.onClick = [this] {
      ctx_.proc.saveBankToLibrary(name_.getText().toStdString());
      libCount_ = -1;
    };
    loadLib_.onClick = [this] {
      const int k = lib_.getSelectedId() - 1;
      if (k >= 0 && k < static_cast<int>(libFiles_.size())) ctx_.proc.loadBankFromLibrary(libFiles_[static_cast<size_t>(k)].path);
    };
    for (auto* x : std::initializer_list<juce::Component*>{&info_, &wave_, &apply_, &revert_, &proc_, &grid_, &seed_, &amount_, &gen_, &name_, &saveLib_, &lib_, &loadLib_})
      addAndMakeVisible(*x);
    pts_.fill(0.0);
  }
  void refresh() {
    const auto id = ctx_.proc.currentBank();
    if (id != bankId_) {
      bankId_ = id;
      bank_ = resolveBank(id);
      const int n = bank_ ? bank_->numWaves() : 1;
      wave_.setRange(0, std::max(1, n - 1), 1);
      wave_.setValue(std::min(wave_.getValue(), static_cast<double>(n - 1)), juce::dontSendNotification);
      loadWave();
    }
    const auto files = ctx_.proc.bankLibrary();
    if (static_cast<int>(files.size()) != libCount_) {
      libCount_ = static_cast<int>(files.size());
      libFiles_ = files;
      lib_.clear(juce::dontSendNotification);
      for (size_t k = 0; k < files.size(); ++k)
        lib_.addItem(juce::String(files[k].name) + "  (" + juce::String(files[k].nx) + "x" + juce::String(files[k].ny) + ")", static_cast<int>(k) + 1);
      if (!files.empty()) lib_.setSelectedId(1, juce::dontSendNotification);
    }
    if (bank_)
      info_.setText(juce::String(bank_->name()) + "  " + juce::String(bank_->nx()) + " x " + juce::String(bank_->ny()) + "  |  " + juce::String(bankId_) +
                        juce::String::fromUTF8("  |  bibliothèque : ") + juce::String(libCount_) + " fichier(s)",
                    juce::dontSendNotification);
  }
  void paint(juce::Graphics& g) override {
    const auto a = area_.toFloat();
    g.setColour(tok::glass);
    g.fillRoundedRectangle(a, tok::radius);
    g.setColour(tok::glassEdge);
    g.drawRoundedRectangle(a.reduced(0.5f), tok::radius, 1.0f);
    g.drawHorizontalLine(static_cast<int>(a.getCentreY()), a.getX(), a.getRight());
    if (!orig_.empty()) {
      juce::Path p;
      for (int i = 0; i < 256; ++i) {
        const float x = a.getX() + a.getWidth() * static_cast<float>(i) / 255.0f;
        const float y = a.getCentreY() - 0.45f * a.getHeight() * orig_[static_cast<size_t>(i * 8)];
        i == 0 ? p.startNewSubPath(x, y) : p.lineTo(x, y);
      }
      g.setColour(tok::textMute);
      g.strokePath(p, juce::PathStrokeType(1.0f));
    }
    juce::Path q;
    for (int i = 0; i <= kPoints; ++i) {
      const float x = a.getX() + a.getWidth() * static_cast<float>(i) / static_cast<float>(kPoints);
      const float y = a.getCentreY() - 0.45f * a.getHeight() * static_cast<float>(pts_[static_cast<size_t>(i % kPoints)]);
      i == 0 ? q.startNewSubPath(x, y) : q.lineTo(x, y);
    }
    g.setColour(tok::wt);
    g.strokePath(q, juce::PathStrokeType(2.0f));
    g.setColour(tok::textDim);
    g.setFont(uiFont(10.0f));
    g.drawText(juce::String::fromUTF8("dessiner à la souris  (forme ") + juce::String(static_cast<int>(wave_.getValue())) + ")", a.reduced(8.0f).toNearestInt(), juce::Justification::topLeft);
  }
  void mouseDown(const juce::MouseEvent& e) override {
    last_ = -1;
    mouseDrag(e);
  }
  void mouseDrag(const juce::MouseEvent& e) override {
    const auto a = area_.toFloat();
    if (!a.contains(e.position) && last_ < 0) return;
    const int i = juce::jlimit(0, kPoints - 1, static_cast<int>((e.position.x - a.getX()) / a.getWidth() * kPoints));
    const double v = juce::jlimit(-1.0, 1.0, static_cast<double>((a.getCentreY() - e.position.y) / (0.45f * a.getHeight())));
    if (last_ >= 0 && last_ != i) {
      const int s = last_ < i ? 1 : -1;
      for (int k = last_ + s; k != i; k += s) pts_[static_cast<size_t>(k)] = lastV_ + (v - lastV_) * (k - last_) / static_cast<double>(i - last_);
    }
    pts_[static_cast<size_t>(i)] = v;
    last_ = i;
    lastV_ = v;
    repaint();
  }
  void resized() override {
    auto r = getLocalBounds();
    info_.setBounds(r.removeFromTop(18));
    auto l1 = r.removeFromTop(30);
    wave_.setBounds(l1.removeFromLeft(260).reduced(2));
    apply_.setBounds(l1.removeFromLeft(150).reduced(2));
    revert_.setBounds(l1.removeFromLeft(100).reduced(2));
    auto bottom = r.removeFromBottom(64);
    auto l2 = bottom.removeFromTop(32);
    proc_.setBounds(l2.removeFromLeft(150).reduced(2));
    grid_.setBounds(l2.removeFromLeft(90).reduced(2));
    seed_.setBounds(l2.removeFromLeft(150).reduced(2));
    amount_.setBounds(l2.removeFromLeft(150).reduced(2));
    gen_.setBounds(l2.removeFromLeft(90).reduced(2));
    auto l3 = bottom;
    name_.setBounds(l3.removeFromLeft(180).reduced(2));
    saveLib_.setBounds(l3.removeFromLeft(140).reduced(2));
    lib_.setBounds(l3.removeFromLeft(220).reduced(2));
    loadLib_.setBounds(l3.removeFromLeft(90).reduced(2));
    area_ = r.reduced(0, 4);
  }

 private:
  void loadWave() {
    orig_.clear();
    if (bank_) {
      const int w = juce::jlimit(0, bank_->numWaves() - 1, static_cast<int>(wave_.getValue()));
      const auto& raw = bank_->raw();
      orig_.assign(raw.begin() + static_cast<std::ptrdiff_t>(w) * WtBank::kLength, raw.begin() + static_cast<std::ptrdiff_t>(w + 1) * WtBank::kLength);
      for (int i = 0; i < kPoints; ++i) pts_[static_cast<size_t>(i)] = orig_[static_cast<size_t>(i * (WtBank::kLength / kPoints))];
    }
    repaint();
  }
  UiContext& ctx_;
  juce::Label info_;
  juce::Slider wave_, seed_, amount_;
  ActionCell apply_, revert_, gen_, saveLib_, loadLib_;
  juce::ComboBox proc_, grid_, lib_;
  juce::TextEditor name_;
  std::array<double, kPoints> pts_{};
  std::vector<float> orig_;
  std::shared_ptr<const WtBank> bank_;
  std::string bankId_;
  std::vector<BankFileInfo> libFiles_;
  int libCount_ = -1, last_ = -1;
  double lastV_ = 0.0;
  juce::Rectangle<int> area_;
};

// ================================================================ ENGINEERING
class EngineeringView : public SpaceView {
 public:
  explicit EngineeringView(UiContext& c)
      : SpaceView(c),
        tabs_(tok::cross),
        fmGrid_(c, {kNodeOP1, kNodeOP2, kNodeOP3, kNodeOP4}, {kNodeOP1, kNodeOP2, kNodeOP3, kNodeOP4}, true),
        crossFwd_(c, {kNodeWT}, {kNodeOP1, kNodeOP2, kNodeOP3, kNodeOP4}, false),
        crossRev_(c, {kNodeOP1, kNodeOP2, kNodeOP3, kNodeOP4, kNodeBUS}, revPorts(), false),
        net_(c.proc),
        modPage_(c),
        waveEd_(c),
        voice_(c, Density::S),
        import_("Importer WAV...", tok::wt),
        export_("Exporter WAV...", tok::wt),
        load_("Charger", tok::text),
        fav_("Favori", tok::warn),
        save_("Enregistrer", tok::text),
        restore_(c, juce::String::fromUTF8("Restaurer la configuration du Drone à l'ouverture du projet"), tok::warn),
        rnd_("RANDOMIZE", tok::mod),
        mut_("MUTATE", tok::mod),
        evoStart_("Evolve start", tok::mod),
        evoStop_("Stop", tok::text),
        evoReset_("Reset", tok::text),
        evoCommit_("Commit", tok::mod),
        clearLocks_("Lever les verrous", tok::text),
        learnSel_(juce::String::fromUTF8("Apprendre : paramètre de l'Inspector"), tok::mod),
        learnCancel_("Annuler l'apprentissage", tok::text),
        capNow_("Capturer maintenant", tok::warn) {
    tabs_.setTabs({"FM NETWORK", "CROSS NETWORK", "MODULATION", "BANK LAB", "VOICE / TUNING", "PRESETS / STATE", "RANDOMIZE", "MIDI LEARN", "DIAGNOSTICS", "CAPTURES / ASSETS"});
    tabs_.onChange = [this](int k) { showTab(k); };
    addAndMakeVisible(tabs_);
    // réseaux
    for (int k = 0; k < kNumFmTopologies; ++k) topo_.addItem(fmTopologyName(k), k + 1);
    topo_.setTextWhenNothingSelected(juce::String::fromUTF8("Topologie FM préréglée…"));
    topo_.setTitle(juce::String::fromUTF8("Topologie FM préréglée"));
    topo_.onChange = [this] {
      if (topo_.getSelectedId() > 0) ctx_.proc.applyFmTopology(topo_.getSelectedId() - 1);
      topo_.setSelectedId(0, juce::dontSendNotification);
    };
    styled(netInfo_, 10.0f, false, tok::textDim);
    netInfo_.setText(juce::String::fromUTF8("Classe causale B-T2 sticky (lecture seule) : I = Immediate, D = Delayed (T2). Création/retrait = transaction validée hors RT (Undo). "
                                            "Une profondeur de route absente est conservée, sans effet."),
                     juce::dontSendNotification);
    fmVp_.setViewedComponent(&fmGrid_, false);
    crossContent_.addAndMakeVisible(crossFwd_);
    crossContent_.addAndMakeVisible(crossRev_);
    crossVp_.setViewedComponent(&crossContent_, false);
    // banques
    for (int cl : {256, 512, 1024, 2048, 4096}) cycle_.addItem("cycle " + juce::String(cl), cl);
    cycle_.setSelectedId(2048);
    cycle_.setTitle(juce::String::fromUTF8("Longueur de cycle à l'import"));
    import_.onClick = [this] {
      chooser_ = std::make_unique<juce::FileChooser>("Importer une banque WAV", juce::File(), "*.wav");
      chooser_->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& fc) {
        if (fc.getResult() != juce::File()) ctx_.proc.importBankWav(fc.getResult().getFullPathName().toStdString(), cycle_.getSelectedId());
      });
    };
    export_.onClick = [this] {
      chooser_ = std::make_unique<juce::FileChooser>("Exporter la banque active", juce::File(), "*.wav");
      chooser_->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles, [this](const juce::FileChooser& fc) {
        if (fc.getResult() != juce::File()) ctx_.proc.exportBankWav(fc.getResult().withFileExtension("wav").getFullPathName().toStdString());
      });
    };
    styled(banks_, 10.5f);
    // voix / accordage
    voice_.panel.setGroups({{"Voix", range(ParamId::VoiceMode, ParamId::BendDown)}, {"Accordage", {ParamId::TuneA4, ParamId::TuneScale}},
                            {juce::String::fromUTF8("Micro-accordage (classes de hauteur, ct)"), range(ParamId::TunePc0, ParamId::TunePc11)}});
    // presets / état
    presets_.setTitle("Presets");
    load_.onClick = [this] {
      const auto list = ctx_.proc.presets();
      const int k = presets_.getSelectedId() - 1;
      if (k >= 0 && k < static_cast<int>(list.size())) ctx_.proc.loadPreset(list[static_cast<size_t>(k)]);
    };
    fav_.onClick = [this] {
      const auto list = ctx_.proc.presets();
      const int k = presets_.getSelectedId() - 1;
      if (k >= 0 && k < static_cast<int>(list.size())) ctx_.proc.setPresetFavorite(list[static_cast<size_t>(k)], !list[static_cast<size_t>(k)].favorite);
      refreshPresets();
    };
    saveName_.setTextToShowWhenEmpty("nom du preset", tok::textMute);
    saveTags_.setTextToShowWhenEmpty(juce::String::fromUTF8("tags (séparés par des virgules)"), tok::textMute);
    save_.onClick = [this] {
      std::vector<std::string> tags;
      juce::StringArray a;
      a.addTokens(saveTags_.getText(), ",", "");
      for (auto& s : a) tags.push_back(s.trim().toStdString());
      ctx_.proc.savePreset(saveName_.getText().toStdString(), tags);
      refreshPresets();
    };
    restore_.onUserToggle = [this](bool on) { ctx_.proc.setDroneRestore(on); };
    restore_.setTooltip(juce::String::fromUTF8("Préférence d'instance : restaure la CONFIGURATION du Drone, jamais son exécution (autorisation requise)"));
    styled(state_, 10.5f);
    // randomizer
    for (int p = 0; p < 4; ++p) profile_.addItem(randomProfileName(static_cast<RandomProfile>(p)), p + 1);
    profile_.setSelectedId(2);
    profile_.setTitle("Profil");
    intensity_.setSliderStyle(juce::Slider::LinearHorizontal);
    intensity_.setRange(0.0, 1.0, 0.01);
    intensity_.setValue(0.5);
    intensity_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    intensity_.setTitle(juce::String::fromUTF8("Intensité"));
    for (int f = 0; f < static_cast<int>(RandomFamily::Count); ++f) {
      fam_[f].setButtonText(randomFamilyName(static_cast<RandomFamily>(f)));
      fam_[f].setToggleState(ctx_.random.families[static_cast<size_t>(f)], juce::dontSendNotification);
    }
    rnd_.onClick = [this] { ctx_.proc.randomize(options()); };
    mut_.onClick = [this] { ctx_.proc.mutate(options()); };
    evoMode_.addItem("Anchored", 1);
    evoMode_.addItem("Drift", 2);
    evoMode_.setSelectedId(1);
    evoRate_.setSliderStyle(juce::Slider::LinearHorizontal);
    evoRate_.setRange(0.01, 2.0, 0.01);
    evoRate_.setValue(0.2);
    evoRate_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    evoRate_.setTitle("Cadence Evolve (Hz)");
    evoStart_.onClick = [this] {
      ctx_.proc.evolveStart(options(), evoMode_.getSelectedId() == 2 ? EvolveMode::Drift : EvolveMode::Anchored, static_cast<float>(evoRate_.getValue()));
    };
    evoStop_.onClick = [this] { ctx_.proc.evolveStop(); };
    evoReset_.onClick = [this] { ctx_.proc.evolveReset(); };
    evoCommit_.onClick = [this] { ctx_.proc.evolveCommit(); };
    clearLocks_.onClick = [this] { ctx_.random.locks.clear(); };
    styled(rndInfo_, 10.5f);
    // MIDI Learn
    learnSel_.onClick = [this] { ctx_.proc.setMidiLearnTarget(idx(ctx_.inspected)); };
    learnCancel_.onClick = [this] { ctx_.proc.setMidiLearnTarget(-1); };
    styled(learnInfo_, 10.5f);
    learnVp_.setViewedComponent(&learnList_, false);
    // diagnostics, captures
    styled(diag_, 10.5f);
    styled(caps_, 10.5f);
    capNow_.onClick = [this] {
      for (int k = 0; k < 6; ++k) ctx_.proc.requestCapture(k);
      capPending_ = 20;
    };
    for (int k = 0; k < 6; ++k) {
      static const char* sn[6] = {"WT1", "WT2", "FM1", "FM2", "GL1", "GL2"};
      capClear_[k] = std::make_unique<ActionCell>(juce::String::fromUTF8("Effacer ") + sn[k], tok::text);
      capClear_[k]->onClick = [this, k] { ctx_.proc.clearCapture(k); };
    }
    std::vector<juce::Component*> all{&topo_, &netInfo_, &fmVp_, &net_, &crossVp_, &modPage_, &waveEd_, &cycle_, &import_, &export_, &banks_, &voice_, &presets_, &load_, &fav_,
                                      &saveName_, &saveTags_, &save_, &restore_, &state_, &profile_, &intensity_, &rnd_, &mut_, &evoMode_, &evoRate_, &evoStart_, &evoStop_,
                                      &evoReset_, &evoCommit_, &clearLocks_, &rndInfo_, &learnSel_, &learnCancel_, &learnInfo_, &learnVp_, &diag_, &caps_, &capNow_};
    for (auto& f : fam_) all.push_back(&f);
    for (auto& cc : capClear_) all.push_back(cc.get());
    for (auto* x : all) addChildComponent(*x);
    refreshPresets();
    showTab(0);
  }
  static std::vector<int> revPorts() {
    std::vector<int> v;
    for (int s = 0; s < kMaxRoutes; ++s)
      if (routeSlotInfo(s).kind == RouteKind::CrossReverse && std::find(v.begin(), v.end(), routeSlotInfo(s).dst) == v.end()) v.push_back(routeSlotInfo(s).dst);
    return v;
  }
  int numTabs() const override { return 10; }
  juce::String tabName(int k) const override {
    static const char* n[10] = {"FM NETWORK", "CROSS NETWORK", "MODULATION", "BANK LAB", "VOICE / TUNING", "PRESETS / STATE", "RANDOMIZE", "MIDI LEARN", "DIAGNOSTICS", "CAPTURES / ASSETS"};
    return n[juce::jlimit(0, 9, k)];
  }
  int currentTab() const override { return tab_; }
  void showTab(int k) override {
    tab_ = juce::jlimit(0, 9, k);
    tabs_.setCurrent(tab_);
    auto vis = [](std::initializer_list<juce::Component*> l, bool v) {
      for (auto* x : l) x->setVisible(v);
    };
    vis({&topo_, &netInfo_, &fmVp_, &net_}, tab_ == 0);
    vis({&crossVp_}, tab_ == 1);
    netInfo_.setVisible(tab_ == 0 || tab_ == 1);
    vis({&modPage_}, tab_ == 2);
    vis({&waveEd_, &cycle_, &import_, &export_, &banks_}, tab_ == 3);
    vis({&voice_}, tab_ == 4);
    vis({&presets_, &load_, &fav_, &saveName_, &saveTags_, &save_, &restore_, &state_}, tab_ == 5);
    vis({&profile_, &intensity_, &rnd_, &mut_, &evoMode_, &evoRate_, &evoStart_, &evoStop_, &evoReset_, &evoCommit_, &clearLocks_, &rndInfo_}, tab_ == 6);
    for (auto& f : fam_) f.setVisible(tab_ == 6);
    vis({&learnSel_, &learnCancel_, &learnInfo_, &learnVp_}, tab_ == 7);
    vis({&diag_}, tab_ == 8);
    vis({&caps_, &capNow_}, tab_ == 9);
    for (auto& cc : capClear_) cc->setVisible(tab_ == 9);
    if (tab_ == 5) refreshPresets();
    if (tab_ == 2) modPage_.rebuild(false);  // navigation : aucune destruction de composant (reconstruction seulement si les routes changent)
    resized();
    tick();
  }
  void refreshState() override {
    if (tab_ == 5) refreshPresets();
    tick();
  }
  void refreshPresets() {
    presets_.clear(juce::dontSendNotification);
    int k = 1;
    for (const auto& p : ctx_.proc.presets()) {
      juce::String n = p.name;
      if (p.factory) n << juce::String::fromUTF8("  [usine]");
      if (p.favorite) n << juce::String::fromUTF8("  *");
      if (!p.tags.empty()) {
        juce::StringArray t;
        for (const auto& s : p.tags) t.add(s);
        n << "  #" << t.joinIntoString(" #");
      }
      presets_.addItem(n, k++);
    }
  }
  RandomOptions options() {
    RandomOptions o = ctx_.random;
    o.profile = static_cast<RandomProfile>(profile_.getSelectedId() - 1);
    o.intensity = static_cast<float>(intensity_.getValue());
    for (int f = 0; f < static_cast<int>(RandomFamily::Count); ++f) o.families[static_cast<size_t>(f)] = fam_[f].getToggleState();
    ctx_.random = o;
    return o;
  }
  void tick() override {
    if (tab_ == 0 || tab_ == 1) {
      const auto routes = ctx_.proc.currentRoutes();
      if (tab_ == 0) {
        fmGrid_.refresh(routes);
        net_.refresh();
      } else {
        crossFwd_.refresh(routes);
        crossRev_.refresh(routes);
      }
    }
    if (tab_ == 2) modPage_.rebuild(false);
    if (tab_ == 3) {
      juce::String t = juce::String::fromUTF8("BANQUES (immuables, partagées, Copy-on-Write)\n\n");
      const auto cur = ctx_.proc.currentBank();
      std::vector<std::string> all = FactoryBanks::ids();
      for (const auto& u : UserBanks::ids()) all.push_back(u);
      for (const auto& id : all) {
        auto b = resolveBank(id);
        t << (id == cur ? "> " : "  ") << juce::String(id).substring(0, 22).paddedRight(' ', 23) << juce::String(b->name()).substring(0, 16).paddedRight(' ', 17) << juce::String(b->nx())
          << "x" << juce::String(b->ny()) << "  " << juce::String(b->memoryBytes() / 1024) << " Kio" << (b->precise() ? juce::String::fromUTF8("  (double, réf. b5)") : juce::String()) << "\n";
      }
      t << juce::String::fromUTF8("\nImport : cycles de N échantillons, jusqu'à 64 x 16 formes, identifiant SHA-256 ;\nla banque est intégrée à l'état du projet et aux presets.");
      banks_.setText(t, juce::dontSendNotification);
      waveEd_.refresh();
    }
    if (tab_ == 5) {
      const auto s = ctx_.proc.snapshotState();
      juce::String t;
      t << "preset   " << ctx_.proc.presetName() << (ctx_.proc.dirty() ? juce::String::fromUTF8("  (modifié)") : juce::String()) << "\n";
      t << juce::String::fromUTF8("graine   ") << juce::String::toHexString(static_cast<juce::int64>(s.seed)) << "\n";
      t << "compteurs Randomize/Mutate/Evolve  " << juce::String(static_cast<juce::int64>(s.streamCounters[0])) << " / " << juce::String(static_cast<juce::int64>(s.streamCounters[1])) << " / "
        << juce::String(static_cast<juce::int64>(s.streamCounters[2])) << "\n";
      t << "Undo  " << (ctx_.proc.canUndo() ? juce::String(ctx_.proc.undoLabel()) : juce::String("-")) << "\nRedo  " << (ctx_.proc.canRedo() ? juce::String(ctx_.proc.redoLabel()) : juce::String("-")) << "\n";
      t << juce::String::fromUTF8("répertoire  ") << ctx_.proc.presetDirectory() << "\n";
      t << juce::String::fromUTF8("Format d'état : version 4 (inchangé) ; préférences d'interface (espace, zoom, Inspector) non sauvegardées dans l'état sonore.");
      state_.setText(t, juce::dontSendNotification);
      restore_.setToggleState(ctx_.proc.droneRestore(), juce::dontSendNotification);
      restore_.setCellState(ctx_.proc.droneRestore() ? CellState::Active : CellState::Off);
    }
    if (tab_ == 6) {
      juce::String t;
      t << "verrous : " << static_cast<int>(ctx_.random.locks.size()) << juce::String::fromUTF8(" paramètre(s) (Inspector › Verrou Randomize)");
      int n = 0;
      for (auto id : ctx_.random.locks)
        if (n++ < 12) t << "\n  " << paramSpec(id).stableId;
      t << "\nEvolve : " << (ctx_.proc.evolveRunning() ? juce::String::fromUTF8("EN COURS (couche temporaire, Base inchangée)") : juce::String::fromUTF8("arrêté"));
      rndInfo_.setText(t, juce::dontSendNotification);
    }
    if (tab_ == 7) refreshLearn();
    if (tab_ == 8) refreshDiag();
    if (tab_ == 9) {
      if (capPending_ > 0) {
        --capPending_;
        ctx_.proc.collectCaptures(0x3Fu);
      }
      static const char* sn[6] = {"WT1", "WT2", "FM1", "FM2", "GL1", "GL2"};
      juce::String t = juce::String::fromUTF8("CAPTURES R2 (contenu créatif, sauvegardé avec le preset et le projet ; appliquées seulement si l'emplacement est armé ET autorisé)\n\n");
      const auto caps = ctx_.proc.captures();
      if (caps.empty()) t << juce::String::fromUTF8("  aucune capture\n");
      for (const auto& c : caps)
        t << "  " << sn[juce::jlimit(0, 5, c.slot)] << "  " << (c.kind == 0 ? "D2 audio  " : "S2 spectre") << "  " << juce::String(c.sampleRate, 0) << " Hz  " << c.frames
          << (c.kind == 0 ? " trames" : " bins") << "  " << juce::String(static_cast<int>(c.data.size() * sizeof(float) / 1024)) << " Kio\n";
      t << juce::String::fromUTF8("\nACTIFS\n  banque active  ") << ctx_.proc.currentBank() << "\n  banques utilisateur  " << static_cast<int>(UserBanks::ids().size())
        << juce::String::fromUTF8("\n  bibliothèque .sdwt  ") << static_cast<int>(ctx_.proc.bankLibrary().size()) << " fichier(s)";
      caps_.setText(t, juce::dontSendNotification);
    }
  }
  void refreshLearn() {
    const auto ml = ctx_.proc.midiLearn();
    std::vector<int> ccs;
    for (int cc = 0; cc < 128; ++cc)
      if (ml.ccToParam[static_cast<size_t>(cc)] >= 0) ccs.push_back(cc);
    if (ccs != learnCcs_) {
      learnCcs_ = ccs;
      learnRows_.clear();
      int y = 0;
      for (int cc : ccs) {
        auto row = std::make_unique<ActionCell>("CC" + juce::String(cc) + juce::String::fromUTF8("  ›  ") + paramSpec(ml.ccToParam[static_cast<size_t>(cc)]).name +
                                                    "  (" + paramSpec(ml.ccToParam[static_cast<size_t>(cc)]).stableId + juce::String::fromUTF8(")   — retirer"),
                                                tok::mod);
        row->onClick = [this, cc] { ctx_.proc.clearMidiLearn(cc); };
        row->setBounds(0, y, 620, 28);
        y += 32;
        learnList_.addAndMakeVisible(*row);
        learnRows_.push_back(std::move(row));
      }
      learnList_.setSize(640, std::max(40, y));
    }
    const int tgt = ctx_.proc.midiLearnTarget();
    juce::String t = juce::String::fromUTF8("MIDI Learn : correspondance GLOBALE propre à l'utilisateur, jamais dans un preset ; CC réservés exclus ; armements autonomes non apprenables.\n");
    t << juce::String::fromUTF8("Inspector : ") << paramSpec(ctx_.inspected).name << " (" << paramSpec(ctx_.inspected).stableId << ")"
      << (MidiLearnMap::learnableParam(idx(ctx_.inspected)) ? "" : juce::String::fromUTF8("  — non apprenable")) << "\n";
    t << "Apprentissage : " << (tgt >= 0 ? juce::String::fromUTF8("en attente d'un CC pour ") + paramSpec(tgt).stableId : juce::String("inactif")) << "\n"
      << static_cast<int>(ccs.size()) << juce::String::fromUTF8(" affectation(s)");
    learnInfo_.setText(t, juce::dontSendNotification);
    learnSel_.setEnabled(MidiLearnMap::learnableParam(idx(ctx_.inspected)));
  }
  void refreshDiag() {
    auto& t = ctx_.proc.telemetry();
    const auto& st = ctx_.proc.engine().setup();
    juce::String d;
    d << "build " << SUBDESTRUCKT_BUILD_ID << "  commit " << juce::String(SUBDESTRUCKT_COMMIT).substring(0, 12) << "\n\n";
    d << juce::String::fromUTF8("Fs hôte        ") << juce::String(t.hostRate.load(), 0) << " Hz\nQuality        " << qualityName(static_cast<Quality>(t.quality.load()))
      << "\nK              " << t.oversampling.load() << "\nFs moteur      " << juce::String(t.engineRate.load(), 0) << " Hz\nT2 (D)         " << juce::String(t.delaySamples.load(), 4)
      << juce::String::fromUTF8(" éch. moteur  ") << (t.delayInteger.load() ? juce::String("entier exact") : "Thiran1 a=" + juce::String(t.thiranCoefficient.load(), 6))
      << juce::String::fromUTF8("\nlatence hôte   ") << ctx_.proc.getLatencySamples() << juce::String::fromUTF8(" éch. (décimation ") << st.decimatorLatency << ")\ncharge CPU     "
      << juce::String(100.0 * t.cpuLoad.load(), 1) << " % du bloc (dernier bloc, machine courante)\n\n";
    d << juce::String::fromUTF8("SÛRETÉ\nnon-finis      ") << static_cast<int>(t.nonFiniteResets.load()) << "\ngarde +30 dBFS " << static_cast<int>(t.outputClampHits.load()) << "\nborne PM       "
      << static_cast<int>(t.pmClampHits.load()) << "\nlimiteur       " << static_cast<int>(t.limiterHits.load()) << "\nELP +24 dBFS   " << static_cast<int>(t.elpHits.load())
      << juce::String::fromUTF8("\névénements perdus ") << static_cast<int>(t.droppedEvents.load()) << "\nPanic          " << static_cast<int>(t.panics.load()) << "\n\nmessage : " << ctx_.proc.lastMessage();
    static const char* tn[] = {"jamais observé", "ARRÊT", "LECTURE"};
    const int ts = juce::jlimit(0, 2, t.transportState.load());
    d << "\n\nAUTORISATION   " << (t.autonomyAuthorized.load() ? juce::String::fromUTF8("accordée") : juce::String::fromUTF8("retirée")) << " (source "
      << authSourceName(static_cast<AuthSource>(t.autonomySource.load())) << ")" << (t.autonomyLatched.load() ? "  verrou Panic" : "") << "\ntransport      " << juce::String::fromUTF8(tn[ts])
      << (t.offline.load() ? juce::String::fromUTF8("  hors temps réel") : juce::String()) << "\nautorisations  " << static_cast<int>(t.authGrants.load()) << juce::String::fromUTF8("  révocations ")
      << static_cast<int>(t.authRevocations.load()) << juce::String::fromUTF8("  génération ") << static_cast<int>(t.authGeneration.load()) << "  journal perdu " << static_cast<int>(t.taDropped.load())
      << "\njournal fichier : " << juce::String(SubdestrucktProcessor::taFlagPath()) << juce::String::fromUTF8(" (présent = actif)");
    const auto ev = ctx_.proc.taRecentEvents();
    for (size_t i = ev.size() > 8 ? ev.size() - 8 : 0; i < ev.size(); ++i) d << "\n  " << juce::String(ev[i]);
    diag_.setText(d, juce::dontSendNotification);
  }
  void resized() override {
    auto r = getLocalBounds().reduced(tok::gap);
    tabs_.setBounds(r.removeFromTop(tok::tabH));
    r.removeFromTop(tok::gap);
    {  // FM network
      auto a = r;
      auto top = a.removeFromTop(30);
      topo_.setBounds(top.removeFromLeft(260).reduced(2));
      netInfo_.setBounds(top.reduced(6, 0));
      net_.setBounds(a.removeFromRight(std::min(420, a.getWidth() / 2)));
      fmVp_.setBounds(a);
      const auto p = fmGrid_.preferred();
      fmGrid_.setSize(p.x, p.y);
    }
    {  // Cross network
      auto a = r;
      a.removeFromTop(30);
      if (tab_ == 1) netInfo_.setBounds(r.removeFromTop(30).reduced(6, 0));
      crossVp_.setBounds(a);
      const auto f = crossFwd_.preferred(), v = crossRev_.preferred();
      crossFwd_.setBounds(0, 0, f.x, f.y);
      crossRev_.setBounds(0, f.y + 12, v.x, v.y);
      crossContent_.setSize(std::max(f.x, v.x), f.y + 12 + v.y);
    }
    modPage_.setBounds(r);
    {  // banques
      auto a = r;
      auto row = a.removeFromTop(32);
      cycle_.setBounds(row.removeFromLeft(130).reduced(2));
      import_.setBounds(row.removeFromLeft(150).reduced(2));
      export_.setBounds(row.removeFromLeft(150).reduced(2));
      waveEd_.setBounds(a.removeFromLeft(a.getWidth() * 58 / 100).reduced(2));
      banks_.setBounds(a.reduced(8, 2));
    }
    voice_.setBounds(r);
    voice_.resized();
    {  // presets / état
      auto a = r;
      auto left = a.removeFromLeft(std::min(560, a.getWidth() / 2));
      auto l1 = left.removeFromTop(32);
      presets_.setBounds(l1.removeFromLeft(l1.getWidth() - 190).reduced(2));
      load_.setBounds(l1.removeFromLeft(95).reduced(2));
      fav_.setBounds(l1.reduced(2));
      auto l2 = left.removeFromTop(32);
      saveName_.setBounds(l2.removeFromLeft(200).reduced(2));
      saveTags_.setBounds(l2.removeFromLeft(220).reduced(2));
      save_.setBounds(l2.reduced(2));
      left.removeFromTop(8);
      restore_.setBounds(left.removeFromTop(44).reduced(2));
      state_.setBounds(a.reduced(10, 2));
    }
    {  // randomizer
      auto a = r;
      auto l1 = a.removeFromTop(32);
      profile_.setBounds(l1.removeFromLeft(160).reduced(2));
      intensity_.setBounds(l1.removeFromLeft(260).reduced(2));
      rnd_.setBounds(l1.removeFromLeft(130).reduced(2));
      mut_.setBounds(l1.removeFromLeft(130).reduced(2));
      clearLocks_.setBounds(l1.removeFromLeft(150).reduced(2));
      auto l2 = a.removeFromTop(32);
      for (auto& f : fam_) f.setBounds(l2.removeFromLeft(118).reduced(2));
      auto l3 = a.removeFromTop(32);
      evoMode_.setBounds(l3.removeFromLeft(120).reduced(2));
      evoRate_.setBounds(l3.removeFromLeft(220).reduced(2));
      evoStart_.setBounds(l3.removeFromLeft(120).reduced(2));
      evoStop_.setBounds(l3.removeFromLeft(80).reduced(2));
      evoReset_.setBounds(l3.removeFromLeft(80).reduced(2));
      evoCommit_.setBounds(l3.removeFromLeft(90).reduced(2));
      rndInfo_.setBounds(a.removeFromTop(220).reduced(2, 6));
    }
    {  // MIDI Learn
      auto a = r;
      auto l1 = a.removeFromTop(32);
      learnSel_.setBounds(l1.removeFromLeft(280).reduced(2));
      learnCancel_.setBounds(l1.removeFromLeft(200).reduced(2));
      learnInfo_.setBounds(a.removeFromTop(70));
      learnVp_.setBounds(a);
    }
    diag_.setBounds(r);
    {  // captures
      auto a = r;
      auto l1 = a.removeFromTop(32);
      capNow_.setBounds(l1.removeFromLeft(180).reduced(2));
      for (auto& cc : capClear_) cc->setBounds(l1.removeFromLeft(110).reduced(2));
      caps_.setBounds(a.reduced(2, 6));
    }
  }

 private:
  TabStrip tabs_;
  juce::ComboBox topo_;
  juce::Label netInfo_;
  NetGrid fmGrid_, crossFwd_, crossRev_;
  juce::Viewport fmVp_, crossVp_;
  juce::Component crossContent_;
  NetworkView net_;
  ModMatrixPage modPage_;
  WaveEditor waveEd_;
  juce::ComboBox cycle_;
  ScrollPanel voice_;
  ActionCell import_, export_;
  juce::Label banks_;
  juce::ComboBox presets_;
  ActionCell load_, fav_, save_;
  juce::TextEditor saveName_, saveTags_;
  NeuralCell restore_;
  juce::Label state_;
  juce::ComboBox profile_, evoMode_;
  juce::Slider intensity_, evoRate_;
  juce::ToggleButton fam_[static_cast<int>(RandomFamily::Count)];
  ActionCell rnd_, mut_, evoStart_, evoStop_, evoReset_, evoCommit_, clearLocks_;
  juce::Label rndInfo_;
  ActionCell learnSel_, learnCancel_;
  juce::Label learnInfo_;
  juce::Viewport learnVp_;
  juce::Component learnList_;
  std::vector<std::unique_ptr<ActionCell>> learnRows_;
  std::vector<int> learnCcs_;
  juce::Label diag_, caps_;
  ActionCell capNow_;
  std::unique_ptr<ActionCell> capClear_[6];
  std::unique_ptr<juce::FileChooser> chooser_;
  int tab_ = 0, capPending_ = 0;
};

}  // namespace

std::unique_ptr<SpaceView> makeEngineeringView(UiContext& c) { return std::make_unique<EngineeringView>(c); }

}  // namespace sbd::ui
