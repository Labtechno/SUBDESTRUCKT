#include "MainEditor.h"

#include "BuildInfo.h"
#include "dsp/Destruction.h"
#include "dsp/Filters.h"

namespace sbd::ui {

namespace {
constexpr int kBarH = 34;
constexpr int kInspectorW = 300;

std::vector<ParamId> P(std::initializer_list<ParamId> l) { return std::vector<ParamId>(l); }

// Paramètres pertinents par algorithme (seuls ceux de l'algorithme actif sont présentés).
std::vector<int> filterOffsetsFor(int type) {
  using F = FltOff;
  auto o = [](F f) { return static_cast<int>(f); };
  std::vector<int> v{o(F::Type)};
  switch (type) {
    case 1: v.insert(v.end(), {o(F::F1Mode), o(F::Cutoff), o(F::Res), o(F::Key)}); break;
    case 2: v.insert(v.end(), {o(F::Cutoff), o(F::CombFb), o(F::CombDamp), o(F::Key), o(F::Detune)}); break;
    case 3: v.insert(v.end(), {o(F::F3Mode), o(F::Cutoff), o(F::Res), o(F::Key), o(F::Drive), o(F::Bias), o(F::Char)}); break;
    case 4: v.insert(v.end(), {o(F::Cutoff), o(F::Key), o(F::Modal), o(F::Modes), o(F::Decay), o(F::Damp), o(F::Bright), o(F::Inharm), o(F::Freeze)}); break;
    default: return v;
  }
  v.insert(v.end(), {o(F::Stereo), o(F::Spread), o(F::Exc), o(F::ExcLevel), o(F::Pos), o(F::Mix), o(F::Out), o(F::Arm)});
  return v;
}
std::vector<int> dstOffsetsFor(int type) {
  std::vector<int> v{0};
  static const int first[9] = {0, 4, 10, 15, 23, 29, 35, 42, 49};
  static const int count[9] = {0, 6, 5, 8, 6, 6, 7, 7, 7};
  if (type <= 0 || type > 8) return v;
  for (int k = 0; k < count[type]; ++k) v.push_back(first[type] + k);
  v.push_back(static_cast<int>(DstOff::Mix));
  v.push_back(static_cast<int>(DstOff::Out));
  if (type == 5 || type == 6 || type == 8) v.push_back(static_cast<int>(DstOff::Arm));  // états autonomes possibles
  return v;
}

juce::String fmtV(ParamId id, float v) { return juce::String(formatValue(id, v)); }

void styleLabel(juce::Label& l, float h = 12.0f, juce::Colour c = col::text, bool monoFont = false) {
  l.setFont(monoFont ? mono(h) : sans(h));
  l.setColour(juce::Label::textColourId, c);
  l.setJustificationType(juce::Justification::topLeft);
}
}  // namespace

// ================================================================ Inspector
class Inspector : public juce::Component {
 public:
  explicit Inspector(UiContext& c) : ctx_(c) {
    styleLabel(title_, 14.0f, col::text);
    title_.setFont(sans(14.0f, true));
    styleLabel(info_, 11.0f, col::text, true);
    styleLabel(mods_, 11.0f, col::text, true);
    addAndMakeVisible(title_);
    addAndMakeVisible(info_);
    addAndMakeVisible(mods_);
    pin_.setButtonText("Epingler");
    lock_.setButtonText("Verrou Randomize");
    addAndMakeVisible(pin_);
    addAndMakeVisible(lock_);
    lock_.onClick = [this] {
      if (lock_.getToggleState()) ctx_.random.locks.insert(id_); else ctx_.random.locks.erase(id_);
    };
    for (int s = 1; s < kNumModSources; ++s) src_.addItem(modSourceLabel(static_cast<ModSource>(s)), s);
    src_.setSelectedId(static_cast<int>(ModSource::Lfo1));
    rate_.addItem("Control", 1);
    rate_.addItem("Audio", 2);
    rate_.setSelectedId(1);
    depth_.setSliderStyle(juce::Slider::LinearHorizontal);
    depth_.setRange(-1.0, 1.0, 0.001);
    depth_.setValue(0.25);
    depth_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 52, 18);
    add_.setButtonText("+ Modulation");
    add_.onClick = [this] { addModulation(); };
    learn_.setButtonText("MIDI Learn");
    learn_.onClick = [this] { ctx_.proc.setMidiLearnTarget(idx(id_)); };
    removeLast_.setButtonText("Retirer la derniere");
    removeLast_.onClick = [this] {
      const auto m = ctx_.proc.currentMod();
      for (auto it = m.routes.rbegin(); it != m.routes.rend(); ++it)
        if (it->dest == id_) {
          ctx_.proc.modRemove(it->id);
          break;
        }
    };
    for (auto* c2 : std::initializer_list<juce::Component*>{&src_, &rate_, &depth_, &add_, &learn_, &removeLast_}) addAndMakeVisible(*c2);
    show(ParamId::WtX);
  }
  void show(ParamId id) {
    if (pin_.getToggleState() && pinnedOnce_) return;  // contexte épinglé : la sélection ne le remplace pas
    pinnedOnce_ = pin_.getToggleState();
    id_ = id;
    lock_.setToggleState(ctx_.random.locks.count(id) > 0, juce::dontSendNotification);
    const bool mod = paramModulatable(id);
    for (auto* c : std::initializer_list<juce::Component*>{&src_, &rate_, &depth_, &add_, &removeLast_}) c->setEnabled(mod);
    refresh();
  }
  void refresh() {
    const auto& s = paramSpec(id_);
    title_.setText(s.name, juce::dontSendNotification);
    auto& br = ctx_.proc.parameters();
    const float base = br.base(idx(id_));
    juce::String t;
    t << "StableID  " << s.stableId << "\nchemin    " << s.group << "\nunite     " << (s.unit[0] ? s.unit : "-") << "\nplage     "
      << fmtV(id_, s.minValue) << " .. " << fmtV(id_, s.maxValue) << "\ndefaut    " << fmtV(id_, s.defaultValue) << "\nBase      " << fmtV(id_, base);
    auto& tel = ctx_.proc.telemetry();
    if (tel.modulated[static_cast<size_t>(idx(id_))].load())
      t << "\nEffective " << fmtV(id_, tel.effective[static_cast<size_t>(idx(id_))].load()) << " (echantillon decime)";
    t << "\nrates     " << ((s.rates & kRateEvent) ? "E " : "") << ((s.rates & kRateControl) ? "C " : "") << ((s.rates & kRateAudio) ? "A" : "")
      << "\nautomation " << (s.automatable ? "oui" : "non (structurel)");
    const auto ml = ctx_.proc.midiLearn();
    for (int cc = 0; cc < 128; ++cc)
      if (ml.ccToParam[static_cast<size_t>(cc)] == idx(id_)) t << "\nMIDI      CC" << cc;
    if (ctx_.proc.midiLearnTarget() == idx(id_)) t << "\nMIDI      en attente d'un CC...";
    info_.setText(t, juce::dontSendNotification);
    juce::String m;
    const auto mm = ctx_.proc.currentMod();
    const auto st = modRouteStatus(mm);
    int n = 0;
    for (size_t k = 0; k < mm.routes.size(); ++k) {
      const auto& r = mm.routes[k];
      if (r.dest != id_) continue;
      m << "#" << static_cast<int>(r.id) << " " << modSourceLabel(r.source) << "  " << juce::String(r.depth, 3) << "  " << modCurveName(r.curve)
        << (r.rate == ModRate::Audio ? "  audio" : "  control");
      if (r.via != ModSource::None) m << "  via " << modSourceLabel(r.via);
      m << "\n   " << modInactiveText(st[k]) << "\n";
      ++n;
    }
    if (!paramModulatable(id_)) m = "Parametre structurel/discret : jamais destination continue (C25).";
    else if (n == 0) m = "Aucune modulation.";
    mods_.setText(m, juce::dontSendNotification);
  }
  void resized() override {
    auto r = getLocalBounds().reduced(8);
    title_.setBounds(r.removeFromTop(22));
    auto row = r.removeFromTop(24);
    pin_.setBounds(row.removeFromLeft(110));
    lock_.setBounds(row);
    info_.setBounds(r.removeFromTop(190));
    learn_.setBounds(r.removeFromTop(24).reduced(0, 2));
    r.removeFromTop(6);
    src_.setBounds(r.removeFromTop(24).reduced(0, 2));
    auto r2 = r.removeFromTop(24);
    rate_.setBounds(r2.removeFromLeft(90).reduced(0, 2));
    depth_.setBounds(r2.reduced(4, 2));
    auto r3 = r.removeFromTop(26);
    add_.setBounds(r3.removeFromLeft(r3.getWidth() / 2).reduced(0, 2));
    removeLast_.setBounds(r3.reduced(2, 2));
    r.removeFromTop(4);
    mods_.setBounds(r);
  }
  void paint(juce::Graphics& g) override {
    g.setColour(col::panel);
    g.fillRect(getLocalBounds());
    g.setColour(col::cross);
    g.fillRect(0, 0, 2, getHeight());
  }

 private:
  void addModulation() {
    ModRoute r;
    r.source = static_cast<ModSource>(src_.getSelectedId());
    r.dest = id_;
    r.depth = static_cast<float>(depth_.getValue());
    r.rate = rate_.getSelectedId() == 2 ? ModRate::Audio : ModRate::Control;
    ctx_.proc.modAdd(r);
    refresh();
  }
  UiContext& ctx_;
  ParamId id_ = ParamId::WtX;
  bool pinnedOnce_ = false;
  juce::Label title_, info_, mods_;
  juce::ToggleButton pin_, lock_;
  juce::ComboBox src_, rate_;
  juce::Slider depth_;
  juce::TextButton add_, learn_, removeLast_;
};

// ================================================================ Performance
class PerformanceView : public juce::Component {
 public:
  explicit PerformanceView(UiContext& c)
      : ctx_(c), core_(c.proc, c.select, "PERFORMANCE  //  noyau fixe", col::cross), macros_(c.proc, c.select, "MACROS M1-M4", col::cross), monitor_(c.proc) {
    core_.grid.setCellSize(84, 118);  // six commandes visibles sans défilement (balance WT/FM ajoutée)
    core_.grid.setParams(P({ParamId::WtX, ParamId::WtY, ParamId::WtZ, ParamId::CrossMaster, ParamId::PerfBalance, ParamId::OutGain}));
    macros_.grid.setCellSize(104, 118);
    macros_.grid.setParams(range(ParamId::Macro1, ParamId::Macro4));
    addAndMakeVisible(core_);
    addAndMakeVisible(macros_);
    addAndMakeVisible(monitor_);
    for (int k = 0; k < 4; ++k) {
      names_[k].setEditable(true);
      styleLabel(names_[k], 12.0f, col::cross);
      names_[k].setJustificationType(juce::Justification::centred);
      names_[k].onTextChange = [this, k] { ctx_.proc.setMacroName(k, names_[k].getText().toStdString()); };
      addAndMakeVisible(names_[k]);
    }
    styleLabel(fmInfo_, 11.0f, col::dim, true);
    styleLabel(states_, 11.0f, col::text, true);
    addAndMakeVisible(fmInfo_);
    addAndMakeVisible(states_);
    refreshContext();
  }
  // Deux commandes FM contextuelles : elles pilotent des paramètres existants (aucun DSP ajouté, arbitrage UI 1C).
  void refreshContext() {
    const auto routes = ctx_.proc.currentRoutes();
    const auto pv = ctx_.proc.parameters().values();
    int best = -1;
    std::uint32_t bestId = 0;
    bool carrierFound = false;
    static constexpr ParamId kLevel[4] = {ParamId::Op1Level, ParamId::Op2Level, ParamId::Op3Level, ParamId::Op4Level};
    for (const auto& r : routes) {
      if (r.src < kNodeOP1 || r.src > kNodeOP4 || r.dst < kNodeOP1 || r.dst > kNodeOP4 || r.src == r.dst) continue;
      const bool carrier = pv[kLevel[r.dst - 1]] > 0.0f;
      if (best < 0 || (carrier && !carrierFound) || (carrier == carrierFound && r.id < bestId)) {
        best = routeSlot(r.src, r.dst);
        bestId = r.id;
        carrierFound = carrier;
      }
    }
    ParamId index = ParamId::Count, feedback = ParamId::Count;
    if (best >= 0) {
      index = routeSlotInfo(best).depthParam;
      const int dst = routeSlotInfo(best).dst;
      for (const auto& r : routes)
        if (r.src == dst && r.dst == dst) feedback = routeSlotInfo(routeSlot(dst, dst)).depthParam;
    }
    if (index != idxTarget_ || feedback != fbTarget_) {
      idxTarget_ = index;
      fbTarget_ = feedback;
      fmIndex_.reset();
      fmFb_.reset();
      if (index != ParamId::Count) {
        fmIndex_ = std::make_unique<ParamControl>(ctx_.proc, index, ctx_.select, true);
        fmIndex_->setCaption("FM INDEX");
        addAndMakeVisible(*fmIndex_);
      }
      if (feedback != ParamId::Count) {
        fmFb_ = std::make_unique<ParamControl>(ctx_.proc, feedback, ctx_.select, true);
        fmFb_->setCaption("FM FEEDBACK");
        addAndMakeVisible(*fmFb_);
      }
      juce::String t = "FM INDEX -> ";
      t << (index != ParamId::Count ? paramSpec(index).stableId : "aucune route FM interne (FM Network)");
      t << "\nFM FEEDBACK -> " << (feedback != ParamId::Count ? paramSpec(feedback).stableId : "pas d'auto-boucle sur la porteuse");
      fmInfo_.setText(t, juce::dontSendNotification);
      resized();
    }
    const auto names = ctx_.proc.macroNames();
    for (int k = 0; k < 4; ++k)
      if (!names_[k].isBeingEdited()) names_[k].setText(names[static_cast<size_t>(k)], juce::dontSendNotification);
  }
  void refreshTelemetry() {
    core_.grid.refreshTelemetry();
    macros_.grid.refreshTelemetry();
    if (fmIndex_) fmIndex_->refreshTelemetry();
    if (fmFb_) fmFb_->refreshTelemetry();
    monitor_.refresh();
    // états autonomes : Drone et emplacements armés (jamais activés par la navigation)
    const auto pv = ctx_.proc.parameters().values();
    juce::String s = juce::String("DRONE ") + (ctx_.proc.drone() ? "ACTIF" : "off");
    int armed = 0;
    for (ParamId a : {ParamId::FltWtArm, ParamId::FltFmArm, ParamId::FltGlArm, ParamId::DstWt1Arm, ParamId::DstWt2Arm, ParamId::DstFm1Arm,
                      ParamId::DstFm2Arm, ParamId::DstGl1Arm, ParamId::DstGl2Arm})
      armed += pv.flag(a);
    s << "   |   etats autonomes armes : " << armed << "   |   execution "
      << (ctx_.proc.telemetry().autonomyAuthorized.load() ? "autorisee" : (ctx_.proc.telemetry().autonomyLatched.load() ? "revoquee (Panic)" : "suspendue"));
    states_.setText(s, juce::dontSendNotification);
    states_.setColour(juce::Label::textColourId, (ctx_.proc.drone() || armed) ? col::alert : col::dim);
  }
  void resized() override {
    auto r = getLocalBounds().reduced(6);
    auto left = r.removeFromLeft(r.getWidth() * 55 / 100);
    auto top = left.removeFromTop(150);
    core_.setBounds(top.removeFromLeft(560));
    auto fm = top.reduced(4, 0);
    if (fmIndex_) fmIndex_->setBounds(fm.removeFromLeft(110).withTrimmedTop(20));
    if (fmFb_) fmFb_->setBounds(fm.removeFromLeft(110).withTrimmedTop(20));
    left.removeFromTop(6);
    auto mac = left.removeFromTop(170);
    macros_.setBounds(mac.removeFromLeft(450));
    for (int k = 0; k < 4; ++k) names_[k].setBounds(macros_.getX() + 6 + k * 104, macros_.getBottom() + 2, 98, 18);
    left.removeFromTop(26);
    fmInfo_.setBounds(left.removeFromTop(36));
    states_.setBounds(left.removeFromTop(20));
    monitor_.setBounds(r.reduced(4, 0));
  }

 private:
  UiContext& ctx_;
  GridSection core_, macros_;
  MonitorPanel monitor_;
  juce::Label names_[4], fmInfo_, states_;
  std::unique_ptr<ParamControl> fmIndex_, fmFb_;
  ParamId idxTarget_ = ParamId::Count, fbTarget_ = ParamId::Count;
};

// ================================================================ Sound Design
class ChainColumn : public juce::Component {
 public:
  ChainColumn(UiContext& c, int location)
      : ctx_(c),
        filter_(c.proc, c.select, "FILTRE", col::text),
        d1_(c.proc, c.select, "DESTRUCTION 1", col::alert),
        d2_(c.proc, c.select, "DESTRUCTION 2", col::alert) {
    static constexpr ParamId kF[3] = {ParamId::FltWtType, ParamId::FltFmType, ParamId::FltGlType};
    static constexpr ParamId kD[3][2] = {{ParamId::DstWt1Type, ParamId::DstWt2Type}, {ParamId::DstFm1Type, ParamId::DstFm2Type}, {ParamId::DstGl1Type, ParamId::DstGl2Type}};
    fBase_ = kF[location];
    dBase_[0] = kD[location][0];
    dBase_[1] = kD[location][1];
    for (auto* g : {&filter_, &d1_, &d2_}) {
      g->grid.setCellSize(74, 84);
      addAndMakeVisible(*g);
    }
    refresh();
  }
  // Renvoie vrai si la disposition a changé (type d'algorithme ou ordre Pre/Post).
  bool refresh() {
    const auto pv = ctx_.proc.parameters().values();
    const int tf = pv.choice(fBase_), t1 = pv.choice(dBase_[0]), t2 = pv.choice(dBase_[1]);
    const bool post = pv.flag(static_cast<ParamId>(idx(fBase_) + static_cast<int>(FltOff::Pos)));
    if (tf == tf_ && t1 == t1_ && t2 == t2_ && post == post_) return false;
    tf_ = tf;
    t1_ = t1;
    t2_ = t2;
    post_ = post;
    filter_.grid.setParams(filterOffsets(fBase_, filterOffsetsFor(tf)));
    d1_.grid.setParams(filterOffsets(dBase_[0], dstOffsetsFor(t1)));
    d2_.grid.setParams(filterOffsets(dBase_[1], dstOffsetsFor(t2)));
    return true;
  }
  void refreshTelemetry() {
    filter_.grid.refreshTelemetry();
    d1_.grid.refreshTelemetry();
    d2_.grid.refreshTelemetry();
  }
  void resized() override {
    auto r = getLocalBounds();
    // l'ordre affiché suit l'ordre effectif Pre/Post (filtre avant ou après les deux processeurs)
    std::vector<GridSection*> order = post_ ? std::vector<GridSection*>{&d1_, &d2_, &filter_} : std::vector<GridSection*>{&filter_, &d1_, &d2_};
    for (auto* g : order) {
      const int h = g->grid.preferredHeight(r.getWidth() - 12) + 28;
      g->setBounds(r.removeFromTop(std::max(110, h)));
      g->resized();
      r.removeFromTop(4);
    }
  }

 private:
  UiContext& ctx_;
  GridSection filter_, d1_, d2_;
  ParamId fBase_, dBase_[2];
  bool post_ = false;
  int tf_ = -1, t1_ = -1, t2_ = -1;
};

class SoundDesignView : public juce::Component {
 public:
  explicit SoundDesignView(UiContext& c)
      : ctx_(c),
        wt_(c.proc, c.select, "WAVETABLE XYZ", col::wt),
        cross_(c.proc, c.select, "CROSS  //  interaction des moteurs", col::cross),
        fm_(c.proc, c.select, "FM / PM  //  OP1", col::fm),
        matrix_(c.proc, c.select, "MATRICE FM 4x4 (rad)", col::fm),
        wtChain_(c, 0),
        fmChain_(c, 1),
        glChain_(c, 2),
        mod_(c.proc, c.select, "ENVELOPPES / LFO / ALEATOIRE", col::cross),
        voice_(c.proc, c.select, "VOIX / ACCORDAGE / SORTIE", col::text) {
    wt_.grid.setCellSize(72, 84);
    wt_.grid.setParams(P({ParamId::WtLevel, ParamId::WtX, ParamId::WtY, ParamId::WtZ, ParamId::WtCoarse, ParamId::WtFine, ParamId::WtChar,
                          ParamId::WtCharMode, ParamId::WtRender, ParamId::WtInterp, ParamId::WtXBound, ParamId::WtYBound, ParamId::WtXMorph,
                          ParamId::WtYMorph, ParamId::WtPhaseMode, ParamId::WtPhase, ParamId::WtSync, ParamId::WtSyncSrc, ParamId::WtZAType,
                          ParamId::WtZADepth, ParamId::WtZACurve, ParamId::WtZBType, ParamId::WtZBDepth, ParamId::WtZBCurve, ParamId::WtZOrder,
                          ParamId::WtZFund, ParamId::WtZGain}));
    cross_.grid.setCellSize(72, 84);
    cross_.grid.setParams(P({ParamId::CrossMaster, ParamId::CrossFwd, ParamId::CrossRev, ParamId::CrossDir, ParamId::DepthWtOp1, ParamId::DepthWtOp2,
                             ParamId::DepthWtOp3, ParamId::DepthWtOp4, ParamId::DepthOp1Wt, ParamId::DepthOp2Wt, ParamId::DepthOp3Wt, ParamId::DepthOp4Wt,
                             ParamId::DepthBusWt, ParamId::CrossBusW1, ParamId::CrossBusW2, ParamId::CrossBusW3, ParamId::CrossBusW4, ParamId::CrossBusMode,
                             ParamId::CrossWtFreqMode, ParamId::CrossAudioMode, ParamId::CrossAudioAmount, ParamId::CrossAmDir}));
    fm_.grid.setCellSize(72, 84);
    matrix_.grid.setCellSize(64, 78);
    std::vector<ParamId> mx;
    for (int s = 1; s <= 4; ++s)
      for (int d = 1; d <= 4; ++d) mx.push_back(routeSlotInfo(routeSlot(s, d)).depthParam);
    matrix_.grid.setParams(mx);
    for (int k = 0; k < 4; ++k) {
      opBtn_[k].setButtonText("OP" + juce::String(k + 1));
      opBtn_[k].onClick = [this, k] { showOp(k); };
      addAndMakeVisible(opBtn_[k]);
    }
    for (int k = 0; k < kNumFmTopologies; ++k) topo_.addItem(fmTopologyName(k), k + 1);
    topo_.setTextWhenNothingSelected("Topologie FM...");
    topo_.onChange = [this] {
      if (topo_.getSelectedId() > 0) ctx_.proc.applyFmTopology(topo_.getSelectedId() - 1);
      topo_.setSelectedId(0, juce::dontSendNotification);
    };
    addAndMakeVisible(topo_);
    for (const auto& id : FactoryBanks::ids()) bank_.addItem(FactoryBanks::displayName(id), static_cast<int>(bankIds_.size()) + 1), bankIds_.push_back(id);
    bank_.onChange = [this] {
      const int k = bank_.getSelectedId() - 1;
      if (k >= 0 && k < static_cast<int>(bankIds_.size())) ctx_.proc.setBank(bankIds_[static_cast<size_t>(k)]);
    };
    addAndMakeVisible(bank_);
    crossReset_.setButtonText("Cross Reset");
    crossReset_.onClick = [this] { ctx_.proc.requestCrossReset(); };
    addAndMakeVisible(crossReset_);
    excite_.setButtonText("Excitation");
    excite_.onClick = [this] { ctx_.proc.requestManualExcite(); };
    addAndMakeVisible(excite_);
    // captures R2 : gels D2 / spectres S2 des six emplacements, sauvegardés avec le preset ou le projet
    capture_.setButtonText("Capturer gels");
    capture_.setTooltip("Capture le contenu Glitch D2 et le spectre Freeze S2 des emplacements actifs (sauvegarde preset/projet)");
    capture_.onClick = [this] {
      for (int k = 0; k < 6; ++k) ctx_.proc.requestCapture(k);
      capturePending_ = 20;  // collecte aux rafraîchissements suivants (un bloc audio est requis)
    };
    addAndMakeVisible(capture_);
    std::vector<ParamId> m = range(ParamId::AmpAttack, ParamId::AmpVelocity);
    for (ParamId x : {ParamId::AmpDelay, ParamId::AmpHold, ParamId::AmpCurve}) m.push_back(x);
    for (ParamId x : range(ParamId::EnvWtDelay, ParamId::EnvAux2Loop)) m.push_back(x);
    for (ParamId x : range(ParamId::AmpTrigger, ParamId::EnvAux2Sync)) m.push_back(x);  // déclenchement et sync (v3)
    for (ParamId x : range(ParamId::Lfo1Shape, ParamId::Lfo4Retrig)) m.push_back(x);
    for (ParamId x : range(ParamId::RndHoldRate, ParamId::Fol3Release)) m.push_back(x);
    mod_.grid.setCellSize(70, 82);
    mod_.grid.setParams(m);
    std::vector<ParamId> v = range(ParamId::VoiceMode, ParamId::TunePc11);
    for (ParamId x : {ParamId::FmCharMaster, ParamId::FmFbMode, ParamId::OutLimiter, ParamId::OutCeiling}) v.push_back(x);
    voice_.grid.setCellSize(70, 82);
    voice_.grid.setParams(v);
    for (auto* x : std::initializer_list<juce::Component*>{&wt_, &cross_, &fm_, &matrix_, &wtChain_, &fmChain_, &glChain_}) addAndMakeVisible(*x);
    processView_.addAndMakeVisible(wtChain_);
    processView_.addAndMakeVisible(fmChain_);
    processView_.addAndMakeVisible(glChain_);
    procVp_.setViewedComponent(&processView_, false);
    modVp_.setViewedComponent(&mod_, false);
    voiceVp_.setViewedComponent(&voice_, false);
    addAndMakeVisible(procVp_);
    addChildComponent(modVp_);
    addChildComponent(voiceVp_);
    const char* tabs[3] = {"PROCESS", "MOD", "VOIX"};
    for (int k = 0; k < 3; ++k) {
      tabBtn_[k].setButtonText(tabs[k]);
      tabBtn_[k].onClick = [this, k] { showTab(k); };
      addAndMakeVisible(tabBtn_[k]);
    }
    showOp(0);
    showTab(0);
  }
  void showOp(int k) {
    op_ = k;
    for (int i = 0; i < 4; ++i) opBtn_[i].setToggleState(i == k, juce::dontSendNotification);
    static constexpr ParamId kTune[4] = {ParamId::Op1Tune, ParamId::Op2Tune, ParamId::Op3Tune, ParamId::Op4Tune};
    static constexpr ParamId kLevel[4] = {ParamId::Op1Level, ParamId::Op2Level, ParamId::Op3Level, ParamId::Op4Level};
    static constexpr ParamId kRatio[4] = {ParamId::Op1Ratio, ParamId::Op2Ratio, ParamId::Op3Ratio, ParamId::Op4Ratio};
    std::vector<ParamId> v{kLevel[k], kRatio[k]};
    for (int o = 0; o <= 15; ++o) v.push_back(static_cast<ParamId>(idx(kTune[k]) + o));
    fm_.grid.setParams(v);
    fm_.setTitle2("FM / PM  //  OP" + juce::String(k + 1));
  }
  void showTab(int k) {
    tab_ = k;
    for (int i = 0; i < 3; ++i) tabBtn_[i].setToggleState(i == k, juce::dontSendNotification);
    procVp_.setVisible(k == 0);
    modVp_.setVisible(k == 1);
    voiceVp_.setVisible(k == 2);
  }
  void refreshState() {
    const auto cur = ctx_.proc.currentBank();
    for (const auto& u2 : UserBanks::ids())
      if (std::find(bankIds_.begin(), bankIds_.end(), u2) == bankIds_.end()) {
        bankIds_.push_back(u2);
        bank_.addItem(juce::String(resolveBank(u2)->name()) + " [user]", static_cast<int>(bankIds_.size()));
      }
    for (size_t k = 0; k < bankIds_.size(); ++k)
      if (bankIds_[k] == cur) bank_.setSelectedId(static_cast<int>(k) + 1, juce::dontSendNotification);
    wtChain_.refresh();
    fmChain_.refresh();
    glChain_.refresh();
    layoutProcess();
  }
  void refreshTelemetry() {
    if (capturePending_ > 0) {
      --capturePending_;
      ctx_.proc.collectCaptures(0x3Fu);
      const int n = static_cast<int>(ctx_.proc.captures().size());
      capture_.setButtonText(n > 0 ? "Capturer gels (" + juce::String(n) + ")" : "Capturer gels");
    }
    const bool a = wtChain_.refresh(), b = fmChain_.refresh(), c = glChain_.refresh();
    if (a || b || c) layoutProcess();
    wt_.grid.refreshTelemetry();
    cross_.grid.refreshTelemetry();
    fm_.grid.refreshTelemetry();
    matrix_.grid.refreshTelemetry();
    if (tab_ == 0) {
      wtChain_.refreshTelemetry();
      fmChain_.refreshTelemetry();
      glChain_.refreshTelemetry();
    }
    if (tab_ == 1) mod_.grid.refreshTelemetry();
  }
  void layoutProcess() {
    const int w = std::max(900, procVp_.getWidth() - 12), cw = w / 3;
    int h = 0;
    for (auto* c : {&wtChain_, &fmChain_, &glChain_}) {
      c->setSize(cw - 6, 2000);
      c->resized();
      int bottom = 0;
      for (auto* ch : c->getChildren()) bottom = std::max(bottom, ch->getBottom());
      h = std::max(h, bottom);
    }
    wtChain_.setBounds(0, 0, cw - 6, h);
    fmChain_.setBounds(cw, 0, cw - 6, h);
    glChain_.setBounds(2 * cw, 0, cw - 6, h);
    processView_.setSize(w, h + 8);
  }
  void resized() override {
    auto r = getLocalBounds().reduced(4);
    auto top = r.removeFromTop(r.getHeight() * 52 / 100);
    const int w3 = top.getWidth() / 3;
    auto wtA = top.removeFromLeft(w3);
    bank_.setBounds(wtA.removeFromTop(26).reduced(2));
    wt_.setBounds(wtA);
    auto crA = top.removeFromLeft(w3).reduced(4, 0);
    auto crB = crA.removeFromTop(26);
    const int cw = crB.getWidth() / 3;
    crossReset_.setBounds(crB.removeFromLeft(cw).reduced(2));
    excite_.setBounds(crB.removeFromLeft(cw).reduced(2));
    capture_.setBounds(crB.reduced(2));
    cross_.setBounds(crA);
    auto fmA = top;
    auto ob = fmA.removeFromTop(26);
    for (int k = 0; k < 4; ++k) opBtn_[k].setBounds(ob.removeFromLeft(48).reduced(1));
    topo_.setBounds(ob.reduced(2));
    matrix_.setBounds(fmA.removeFromBottom(std::min(fmA.getHeight() / 2, 190)));
    fm_.setBounds(fmA);
    auto tabs = r.removeFromTop(26);
    for (int k = 0; k < 3; ++k) tabBtn_[k].setBounds(tabs.removeFromLeft(110).reduced(2));
    procVp_.setBounds(r);
    modVp_.setBounds(r);
    voiceVp_.setBounds(r);
    mod_.setSize(r.getWidth() - 12, mod_.grid.preferredHeight(r.getWidth() - 24) + 30);
    voice_.setSize(r.getWidth() - 12, voice_.grid.preferredHeight(r.getWidth() - 24) + 30);
    layoutProcess();
  }

 private:
  UiContext& ctx_;
  GridSection wt_, cross_, fm_, matrix_;
  ChainColumn wtChain_, fmChain_, glChain_;
  GridSection mod_, voice_;
  juce::Component processView_;
  juce::Viewport procVp_, modVp_, voiceVp_;
  juce::TextButton opBtn_[4], tabBtn_[3], crossReset_, excite_, capture_;
  int capturePending_ = 0;
  juce::ComboBox topo_, bank_;
  std::vector<std::string> bankIds_;
  int op_ = 0, tab_ = 0;
};

// ================================================================ Engineering
class RouteListView : public juce::Component {
 public:
  explicit RouteListView(UiContext& c) : ctx_(c) {
    for (int s = 0; s < kMaxRoutes; ++s) {
      auto& r = rows_[static_cast<size_t>(s)];
      styleLabel(r.label, 11.0f, col::text, true);
      r.add.setButtonText("+");
      r.rem.setButtonText("-");
      r.add.onClick = [this, s] { ctx_.proc.applyTopology({RouteTable::Op::add(routeSlotInfo(s).src, routeSlotInfo(s).dst)}); };
      r.rem.onClick = [this, s] {
        for (const auto& rt : ctx_.proc.currentRoutes())
          if (rt.src == routeSlotInfo(s).src && rt.dst == routeSlotInfo(s).dst) ctx_.proc.applyTopology({RouteTable::Op::remove(rt.id)});
      };
      addAndMakeVisible(r.label);
      addAndMakeVisible(r.add);
      addAndMakeVisible(r.rem);
    }
    setSize(760, kMaxRoutes * 20 + 8);
  }
  void refresh() {
    const auto routes = ctx_.proc.currentRoutes();
    auto& t = ctx_.proc.telemetry();
    for (int s = 0; s < kMaxRoutes; ++s) {
      const auto& info = routeSlotInfo(s);
      const Route* found = nullptr;
      for (const auto& r : routes)
        if (r.src == info.src && r.dst == info.dst) found = &r;
      const char* kind = info.kind == RouteKind::Internal ? "FM   " : (info.kind == RouteKind::CrossForward ? "WT>FM" : "FM>WT");
      juce::String txt = juce::String(kind) + "  " + juce::String(nodeName(info.src)).paddedRight(' ', 4) + "> " + juce::String(portName(info.dst)).paddedRight(' ', 9);
      if (found)
        txt << "  id " << static_cast<int>(found->id) << "  " << (found->cls == CausalClass::Delayed ? "Delayed (T2)" : "Immediate   ") << "  eff "
            << juce::String(t.effectiveDepth[static_cast<size_t>(s)].load(), 3) << "  gain " << juce::String(t.edgeGain[static_cast<size_t>(s)].load(), 2);
      else
        txt << "  -- (classe si ajoutee : " << (classifyPreview(routes, info.src, info.dst) ? "Delayed" : "Immediate") << ")";
      auto& row = rows_[static_cast<size_t>(s)];
      row.label.setText(txt, juce::dontSendNotification);
      row.label.setColour(juce::Label::textColourId, found ? col::text : col::dim);
      row.add.setEnabled(!found);
      row.rem.setEnabled(found != nullptr);
    }
  }
  void resized() override {
    for (int s = 0; s < kMaxRoutes; ++s) {
      auto& r = rows_[static_cast<size_t>(s)];
      r.label.setBounds(4, 4 + s * 20, getWidth() - 70, 18);
      r.add.setBounds(getWidth() - 62, 4 + s * 20, 28, 18);
      r.rem.setBounds(getWidth() - 32, 4 + s * 20, 28, 18);
    }
  }

 private:
  static bool classifyPreview(const std::vector<Route>& routes, int src, int dst) {
    RouteTable t;
    std::uint32_t next = 1;
    for (const auto& r : routes) next = std::max(next, r.id + 1);
    std::string err;
    t.assign(routes, next, err);
    return t.classify(src, dst) == CausalClass::Delayed;
  }
  struct Row {
    juce::Label label;
    juce::TextButton add, rem;
  };
  UiContext& ctx_;
  std::array<Row, kMaxRoutes> rows_;
};

class ModListView : public juce::Component {
 public:
  explicit ModListView(UiContext& c) : ctx_(c) {
    styleLabel(list_, 11.0f, col::text, true);
    addAndMakeVisible(list_);
    for (int s = 1; s < kNumModSources; ++s) src_.addItem(modSourceLabel(static_cast<ModSource>(s)), s);
    src_.setSelectedId(static_cast<int>(ModSource::Lfo1));
    for (int i = 0; i < kNumParams; ++i)
      if (paramModulatable(static_cast<ParamId>(i))) dst_.addItem(paramSpec(i).name, i + 1);
    dst_.setSelectedId(idx(ParamId::WtX) + 1);
    rate_.addItem("Control", 1);
    rate_.addItem("Audio", 2);
    rate_.setSelectedId(1);
    for (int c2 = 0; c2 < static_cast<int>(ModCurve::Count) - 1; ++c2) curve_.addItem(modCurveName(static_cast<ModCurve>(c2)), c2 + 1);
    curve_.setSelectedId(1);
    depth_.setSliderStyle(juce::Slider::LinearHorizontal);
    depth_.setRange(-1.0, 1.0, 0.001);
    depth_.setValue(0.3);
    depth_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 52, 18);
    add_.setButtonText("Ajouter");
    add_.onClick = [this] {
      ModRoute r;
      r.source = static_cast<ModSource>(src_.getSelectedId());
      r.dest = static_cast<ParamId>(dst_.getSelectedId() - 1);
      r.depth = static_cast<float>(depth_.getValue());
      r.rate = rate_.getSelectedId() == 2 ? ModRate::Audio : ModRate::Control;
      r.curve = static_cast<ModCurve>(curve_.getSelectedId() - 1);
      ctx_.proc.modAdd(r);
    };
    styleLabel(remLbl_, 11.0f, col::dim);
    remLbl_.setText("Retirer #", juce::dontSendNotification);
    remId_.setInputRestrictions(6, "0123456789");
    rem_.setButtonText("Retirer");
    rem_.onClick = [this] { ctx_.proc.modRemove(static_cast<std::uint32_t>(remId_.getText().getIntValue())); };
    for (auto* x : std::initializer_list<juce::Component*>{&src_, &dst_, &rate_, &curve_, &depth_, &add_, &remLbl_, &remId_, &rem_}) addAndMakeVisible(*x);
  }
  void refresh() {
    const auto m = ctx_.proc.currentMod();
    const auto st = modRouteStatus(m);
    int audio = 0;
    juce::String t;
    t << "MATRICE GLOBALE  " << static_cast<int>(m.routes.size()) << "/64 connexions (macros incluses)\n";
    for (size_t k = 0; k < m.routes.size(); ++k) {
      const auto& r = m.routes[k];
      if (r.rate == ModRate::Audio && st[k] == ModInactive::Active) ++audio;
      t << "#" << juce::String(static_cast<int>(r.id)).paddedLeft(' ', 3) << "  " << juce::String(modSourceLabel(r.source)).paddedRight(' ', 16) << "> "
        << juce::String(paramSpec(r.dest).name).paddedRight(' ', 26) << juce::String(r.depth, 3).paddedLeft(' ', 7) << "  " << modCurveName(r.curve)
        << (r.rate == ModRate::Audio ? "  audio  " : "  ctrl   ") << modInactiveText(st[k]) << "\n";
    }
    t << "audio-rate actives : " << audio << "/16";
    list_.setText(t, juce::dontSendNotification);
  }
  void resized() override {
    auto r = getLocalBounds().reduced(4);
    auto a = r.removeFromTop(26);
    src_.setBounds(a.removeFromLeft(150).reduced(2));
    dst_.setBounds(a.removeFromLeft(220).reduced(2));
    curve_.setBounds(a.removeFromLeft(110).reduced(2));
    rate_.setBounds(a.removeFromLeft(90).reduced(2));
    depth_.setBounds(a.removeFromLeft(200).reduced(2));
    add_.setBounds(a.removeFromLeft(80).reduced(2));
    auto b = r.removeFromTop(26);
    remLbl_.setBounds(b.removeFromLeft(70));
    remId_.setBounds(b.removeFromLeft(60).reduced(2));
    rem_.setBounds(b.removeFromLeft(80).reduced(2));
    list_.setBounds(r);
  }

 private:
  UiContext& ctx_;
  juce::Label list_, remLbl_;
  juce::ComboBox src_, dst_, rate_, curve_;
  juce::Slider depth_;
  juce::TextButton add_, rem_;
  juce::TextEditor remId_;
};

// ================================================================ éditeur de formes wavetable (ARCH-G4.1 §3)
// Dessin à la souris (64 points de contrôle, interpolation linéaire bouclée, DC retiré, crête normalisée), générateur
// procédural et bibliothèque .sdwt. Chaque action crée une nouvelle banque immuable, préparée hors RT, sélectionnée
// par une transaction annulable (Undo). Aucune écriture dans la banque jouée.
class WaveEditor : public juce::Component {
 public:
  static constexpr int kPoints = 64;
  explicit WaveEditor(UiContext& c) : ctx_(c) {
    styleLabel(title_, 12.0f, col::wt);
    title_.setText("EDITEUR DE FORMES  //  dessin, generateur, bibliotheque", juce::dontSendNotification);
    styleLabel(info_, 11.0f, col::dim, true);
    wave_.setSliderStyle(juce::Slider::LinearHorizontal);
    wave_.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 48, 18);
    wave_.onValueChange = [this] { loadWave(); };
    apply_.setButtonText("Appliquer la forme");
    apply_.setTooltip("Remplace la forme selectionnee dans une copie de la banque (nouvelle banque, annulable)");
    apply_.onClick = [this] {
      std::vector<std::pair<double, double>> pts;
      for (int i = 0; i < kPoints; ++i) pts.emplace_back(static_cast<double>(i) / kPoints, pts_[static_cast<size_t>(i)]);
      ctx_.proc.editCurrentWave(static_cast<int>(wave_.getValue()), drawnWave(pts));
    };
    revert_.setButtonText("Reprendre");
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
    amount_.setRange(0.0, 1.0, 0.01);
    amount_.setValue(0.6);
    gen_.setButtonText("Generer");
    gen_.onClick = [this] {
      const int g = grid_.getSelectedId();
      const int nx = g == 3 ? 64 : 16, ny = g == 1 ? 1 : (g == 2 ? 4 : 16);
      ctx_.proc.generateBank(static_cast<WtProcedure>(proc_.getSelectedId() - 1), nx, ny, static_cast<std::uint64_t>(seed_.getValue()),
                             amount_.getValue());
    };
    name_.setTextToShowWhenEmpty("nom de la banque", col::dim);
    saveLib_.setButtonText("Enregistrer .sdwt");
    saveLib_.onClick = [this] {
      ctx_.proc.saveBankToLibrary(name_.getText().toStdString());
      libCount_ = -1;
    };
    loadLib_.setButtonText("Charger");
    loadLib_.onClick = [this] {
      const int k = lib_.getSelectedId() - 1;
      if (k >= 0 && k < static_cast<int>(libFiles_.size())) ctx_.proc.loadBankFromLibrary(libFiles_[static_cast<size_t>(k)].path);
    };
    for (auto* x : std::initializer_list<juce::Component*>{&title_, &info_, &wave_, &apply_, &revert_, &proc_, &grid_, &seed_, &amount_, &gen_, &name_,
                                                           &saveLib_, &lib_, &loadLib_})
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
      if (n <= 1) wave_.setRange(0, 1, 1);
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
                        "  |  bibliotheque : " + juce::String(libCount_) + " fichier(s)",
                    juce::dontSendNotification);
  }
  void paint(juce::Graphics& g) override {
    g.setColour(col::panel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
    const auto a = drawArea().toFloat();
    g.setColour(col::display);
    g.fillRect(a);
    g.setColour(col::dim.withAlpha(0.5f));
    g.drawHorizontalLine(static_cast<int>(a.getCentreY()), a.getX(), a.getRight());
    // forme d'origine (lecture de la banque) en filigrane
    if (!orig_.empty()) {
      juce::Path p;
      for (int i = 0; i < 256; ++i) {
        const float x = a.getX() + a.getWidth() * i / 255.0f;
        const float y = a.getCentreY() - 0.45f * a.getHeight() * orig_[static_cast<size_t>(i * 8)];
        i == 0 ? p.startNewSubPath(x, y) : p.lineTo(x, y);
      }
      g.setColour(col::dim);
      g.strokePath(p, juce::PathStrokeType(1.0f));
    }
    juce::Path q;
    for (int i = 0; i <= kPoints; ++i) {
      const float x = a.getX() + a.getWidth() * i / static_cast<float>(kPoints);
      const float y = a.getCentreY() - 0.45f * a.getHeight() * static_cast<float>(pts_[static_cast<size_t>(i % kPoints)]);
      i == 0 ? q.startNewSubPath(x, y) : q.lineTo(x, y);
    }
    g.setColour(col::wt);
    g.strokePath(q, juce::PathStrokeType(2.0f));
    g.setColour(col::dim);
    g.setFont(11.0f);
    g.drawText("dessiner a la souris  (forme " + juce::String(static_cast<int>(wave_.getValue())) + ")", a.reduced(6).toNearestInt(),
               juce::Justification::topLeft);
  }
  void mouseDown(const juce::MouseEvent& e) override {
    last_ = -1;
    mouseDrag(e);
  }
  void mouseDrag(const juce::MouseEvent& e) override {
    const auto a = drawArea().toFloat();
    if (!a.contains(e.position) && last_ < 0) return;
    const int i = juce::jlimit(0, kPoints - 1, static_cast<int>((e.position.x - a.getX()) / a.getWidth() * kPoints));
    const double v = juce::jlimit(-1.0, 1.0, static_cast<double>((a.getCentreY() - e.position.y) / (0.45f * a.getHeight())));
    if (last_ >= 0 && last_ != i) {  // continuité entre deux positions de glisser
      const int s = last_ < i ? 1 : -1;
      for (int k = last_ + s; k != i; k += s)
        pts_[static_cast<size_t>(k)] = lastV_ + (v - lastV_) * (k - last_) / static_cast<double>(i - last_);
    }
    pts_[static_cast<size_t>(i)] = v;
    last_ = i;
    lastV_ = v;
    repaint();
  }
  void resized() override {
    auto r = getLocalBounds().reduced(6);
    title_.setBounds(r.removeFromTop(20));
    info_.setBounds(r.removeFromTop(18));
    auto l1 = r.removeFromTop(28);
    wave_.setBounds(l1.removeFromLeft(260).reduced(2));
    apply_.setBounds(l1.removeFromLeft(150).reduced(2));
    revert_.setBounds(l1.removeFromLeft(100).reduced(2));
    auto bottom = r.removeFromBottom(60);
    auto l2 = bottom.removeFromTop(30);
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
  juce::Rectangle<int> drawArea() const { return area_; }
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
  juce::Label title_, info_;
  juce::Slider wave_, seed_, amount_;
  juce::TextButton apply_, revert_, gen_, saveLib_, loadLib_;
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

class EngineeringView : public juce::Component {
 public:
  explicit EngineeringView(UiContext& c) : ctx_(c), routes_(c), mods_(c), waveEd_(c) {
    const char* tabs[6] = {"RESEAUX FM / CROSS", "MODULATION", "BANQUES", "RANDOMIZER", "ETAT / PRESETS", "DIAGNOSTICS"};
    for (int k = 0; k < 6; ++k) {
      tab_[k].setButtonText(tabs[k]);
      tab_[k].onClick = [this, k] { showTab(k); };
      addAndMakeVisible(tab_[k]);
    }
    routesVp_.setViewedComponent(&routes_, false);
    addAndMakeVisible(routesVp_);
    addChildComponent(mods_);
    // banques
    styleLabel(banks_, 12.0f, col::text, true);
    addChildComponent(banks_);
    for (int cl : {256, 512, 1024, 2048, 4096}) cycle_.addItem("cycle " + juce::String(cl), cl);
    cycle_.setSelectedId(2048);
    import_.setButtonText("Importer WAV...");
    export_.setButtonText("Exporter WAV...");
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
    for (auto* b : std::initializer_list<juce::Component*>{&cycle_, &import_, &export_, &waveEd_}) addChildComponent(*b);
    // randomizer
    for (int p = 0; p < 4; ++p) profile_.addItem(randomProfileName(static_cast<RandomProfile>(p)), p + 1);
    profile_.setSelectedId(2);
    intensity_.setSliderStyle(juce::Slider::LinearHorizontal);
    intensity_.setRange(0.0, 1.0, 0.01);
    intensity_.setValue(0.5);
    intensity_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 52, 18);
    for (int f = 0; f < static_cast<int>(RandomFamily::Count); ++f) {
      fam_[f].setButtonText(randomFamilyName(static_cast<RandomFamily>(f)));
      fam_[f].setToggleState(ctx_.random.families[static_cast<size_t>(f)], juce::dontSendNotification);
    }
    rnd_.setButtonText("RANDOMIZE");
    mut_.setButtonText("MUTATE");
    rnd_.onClick = [this] { ctx_.proc.randomize(options()); };
    mut_.onClick = [this] { ctx_.proc.mutate(options()); };
    evoMode_.addItem("Anchored", 1);
    evoMode_.addItem("Drift", 2);
    evoMode_.setSelectedId(1);
    evoRate_.setSliderStyle(juce::Slider::LinearHorizontal);
    evoRate_.setRange(0.01, 2.0, 0.01);
    evoRate_.setValue(0.2);
    evoRate_.setTextBoxStyle(juce::Slider::TextBoxRight, false, 52, 18);
    evoStart_.setButtonText("Evolve start");
    evoStop_.setButtonText("Stop");
    evoReset_.setButtonText("Reset");
    evoCommit_.setButtonText("Commit");
    evoStart_.onClick = [this] { ctx_.proc.evolveStart(options(), evoMode_.getSelectedId() == 2 ? EvolveMode::Drift : EvolveMode::Anchored, static_cast<float>(evoRate_.getValue())); };
    evoStop_.onClick = [this] { ctx_.proc.evolveStop(); };
    evoReset_.onClick = [this] { ctx_.proc.evolveReset(); };
    evoCommit_.onClick = [this] { ctx_.proc.evolveCommit(); };
    styleLabel(rndInfo_, 11.0f, col::dim, true);
    for (auto* x : std::initializer_list<juce::Component*>{&profile_, &intensity_, &rnd_, &mut_, &evoMode_, &evoRate_, &evoStart_, &evoStop_, &evoReset_, &evoCommit_, &rndInfo_}) addChildComponent(*x);
    for (auto& f : fam_) addChildComponent(f);
    // état / presets
    presetList_.onChange = [this] {};
    load_.setButtonText("Charger");
    load_.onClick = [this] {
      const int k = presetList_.getSelectedId() - 1;
      const auto list = ctx_.proc.presets();
      if (k >= 0 && k < static_cast<int>(list.size())) ctx_.proc.loadPreset(list[static_cast<size_t>(k)]);
    };
    fav_.setButtonText("Favori");
    fav_.onClick = [this] {
      const int k = presetList_.getSelectedId() - 1;
      const auto list = ctx_.proc.presets();
      if (k >= 0 && k < static_cast<int>(list.size())) ctx_.proc.setPresetFavorite(list[static_cast<size_t>(k)], fav_.getToggleState());
      refreshPresets();
    };
    saveName_.setTextToShowWhenEmpty("nom du preset", col::dim);
    saveTags_.setTextToShowWhenEmpty("tags (separes par des virgules)", col::dim);
    save_.setButtonText("Sauvegarder");
    save_.onClick = [this] {
      std::vector<std::string> tags;
      juce::StringArray a;
      a.addTokens(saveTags_.getText(), ",", "");
      for (auto& s : a) tags.push_back(s.trim().toStdString());
      ctx_.proc.savePreset(saveName_.getText().toStdString(), tags);
      refreshPresets();
    };
    restore_.setButtonText("Autoriser la restauration du Drone a l'ouverture du projet");
    restore_.onClick = [this] { ctx_.proc.setDroneRestore(restore_.getToggleState()); };
    styleLabel(stateInfo_, 11.0f, col::text, true);
    for (auto* x : std::initializer_list<juce::Component*>{&presetList_, &load_, &fav_, &saveName_, &saveTags_, &save_, &restore_, &stateInfo_}) addChildComponent(*x);
    styleLabel(diag_, 12.0f, col::text, true);
    addChildComponent(diag_);
    refreshPresets();
    showTab(0);
  }
  RandomOptions options() {
    RandomOptions o = ctx_.random;
    o.profile = static_cast<RandomProfile>(profile_.getSelectedId() - 1);
    o.intensity = static_cast<float>(intensity_.getValue());
    for (int f = 0; f < static_cast<int>(RandomFamily::Count); ++f) o.families[static_cast<size_t>(f)] = fam_[f].getToggleState();
    ctx_.random = o;
    return o;
  }
  void refreshPresets() {
    presetList_.clear(juce::dontSendNotification);
    int k = 1;
    for (const auto& p : ctx_.proc.presets()) {
      juce::String n = p.name;
      if (p.factory) n << "  [usine]";
      if (p.favorite) n << "  *";
      presetList_.addItem(n, k++);
    }
  }
  void showTab(int k) {
    current_ = k;
    for (int i = 0; i < 6; ++i) tab_[i].setToggleState(i == k, juce::dontSendNotification);
    routesVp_.setVisible(k == 0);
    mods_.setVisible(k == 1);
    banks_.setVisible(k == 2);
    for (auto* b : std::initializer_list<juce::Component*>{&cycle_, &import_, &export_, &waveEd_}) b->setVisible(k == 2);
    for (auto* x : std::initializer_list<juce::Component*>{&profile_, &intensity_, &rnd_, &mut_, &evoMode_, &evoRate_, &evoStart_, &evoStop_, &evoReset_, &evoCommit_, &rndInfo_}) x->setVisible(k == 3);
    for (auto& f : fam_) f.setVisible(k == 3);
    for (auto* x : std::initializer_list<juce::Component*>{&presetList_, &load_, &fav_, &saveName_, &saveTags_, &save_, &restore_, &stateInfo_}) x->setVisible(k == 4);
    diag_.setVisible(k == 5);
    refresh();
  }
  void refresh() {
    if (current_ == 0) routes_.refresh();
    if (current_ == 1) mods_.refresh();
    if (current_ == 2) {
      juce::String t = "BANQUES (immuables, partagees, Copy-on-Write)\n\n";
      const auto cur = ctx_.proc.currentBank();
      std::vector<std::string> all = FactoryBanks::ids();
      for (const auto& u2 : UserBanks::ids()) all.push_back(u2);
      for (const auto& id : all) {
        auto b = resolveBank(id);
        t << (id == cur ? "> " : "  ") << juce::String(id).paddedRight(' ', 22) << juce::String(b->name()).paddedRight(' ', 18)
          << juce::String(b->nx()) << "x" << juce::String(b->ny()) << "  " << juce::String(b->memoryBytes() / 1024) << " Kio" << (b->precise() ? "  (double, reference b5)" : "") << "\n";
      }
      t << "\nImport : cycles de N echantillons (choix ci-dessus), jusqu'a 64 x 16 formes, identifiant SHA-256 ; la banque est\n"
           "integree a l'etat du projet et aux presets. Editeur (a gauche) : dessin d'une forme, generateur procedural,\n"
           "bibliotheque .sdwt (portable, SHA-256 verifie). Chaque edition cree une nouvelle banque (Undo).";
      banks_.setText(t, juce::dontSendNotification);
      waveEd_.refresh();
    }
    if (current_ == 3) {
      juce::String t;
      t << "verrous : " << static_cast<int>(ctx_.random.locks.size()) << " parametre(s) (Inspector > Verrou Randomize)\n";
      t << "Evolve : " << (ctx_.proc.evolveRunning() ? "EN COURS (couche temporaire, Base inchangee)" : "arrete");
      rndInfo_.setText(t, juce::dontSendNotification);
    }
    if (current_ == 4) {
      const auto s = ctx_.proc.snapshotState();
      juce::String t;
      t << "preset : " << ctx_.proc.presetName() << (ctx_.proc.dirty() ? "  (modifie)" : "") << "\nseed maitre : " << juce::String::toHexString(static_cast<juce::int64>(s.seed))
        << "\ncompteurs Randomize/Mutate/Evolve : " << juce::String(static_cast<juce::int64>(s.streamCounters[0])) << " / "
        << juce::String(static_cast<juce::int64>(s.streamCounters[1])) << " / " << juce::String(static_cast<juce::int64>(s.streamCounters[2]))
        << "\nUndo : " << (ctx_.proc.canUndo() ? ctx_.proc.undoLabel() : "-") << "   Redo : " << (ctx_.proc.canRedo() ? ctx_.proc.redoLabel() : "-")
        << "\nrepertoire : " << ctx_.proc.presetDirectory() << "\nMIDI Learn (global, hors presets) :";
      const auto ml = ctx_.proc.midiLearn();
      for (int cc = 0; cc < 128; ++cc)
        if (ml.ccToParam[static_cast<size_t>(cc)] >= 0) t << "\n  CC" << cc << " -> " << paramSpec(ml.ccToParam[static_cast<size_t>(cc)]).stableId;
      stateInfo_.setText(t, juce::dontSendNotification);
      restore_.setToggleState(ctx_.proc.droneRestore(), juce::dontSendNotification);
    }
    if (current_ == 5) {
      auto& t = ctx_.proc.telemetry();
      const auto& st = ctx_.proc.engine().setup();
      juce::String d;
      d << "build " << SUBDESTRUCKT_BUILD_ID << "  commit " << juce::String(SUBDESTRUCKT_COMMIT).substring(0, 12) << "\n\n";
      d << "Fs hote        " << juce::String(t.hostRate.load(), 0) << " Hz\nQuality        " << qualityName(static_cast<Quality>(t.quality.load()))
        << "\nK              " << t.oversampling.load() << "\nFs moteur      " << juce::String(t.engineRate.load(), 0) << " Hz\nT2 (D)         "
        << juce::String(t.delaySamples.load(), 4) << " ech. moteur  " << (t.delayInteger.load() ? "entier exact" : "Thiran1 a=" + juce::String(t.thiranCoefficient.load(), 6))
        << "\nlatence hote   " << ctx_.proc.getLatencySamples() << " ech. (decimation " << st.decimatorLatency << ")\ncharge CPU     "
        << juce::String(100.0 * t.cpuLoad.load(), 1) << " % du bloc (mesure du dernier bloc, machine courante)\n\n";
      d << "SURETE\nnon-finis      " << static_cast<int>(t.nonFiniteResets.load()) << "\ngarde +30 dBFS " << static_cast<int>(t.outputClampHits.load())
        << "\nborne PM       " << static_cast<int>(t.pmClampHits.load()) << "\nlimiteur       " << static_cast<int>(t.limiterHits.load())
        << "\nprotection +24 dBFS (ELP, SPEC-HR-01) " << static_cast<int>(t.elpHits.load())
        << "\nevenements perdus " << static_cast<int>(t.droppedEvents.load()) << "\nPanic          " << static_cast<int>(t.panics.load())
        << "\n\nmessage : " << ctx_.proc.lastMessage();
      // TRANSPORT AUTHORIZATION (ARCH-G4.3) : état et derniers événements (recette Reason / Live)
      static const char* tn[] = {"jamais observe", "ARRET", "LECTURE"};
      const int ts = juce::jlimit(0, 2, t.transportState.load());
      d << "\n\nAUTORISATION   " << (t.autonomyAuthorized.load() ? "accordee" : "retiree") << " (source "
        << authSourceName(static_cast<AuthSource>(t.autonomySource.load())) << ")" << (t.autonomyLatched.load() ? "  verrou Panic" : "")
        << "\ntransport      " << tn[ts] << (t.offline.load() ? "  hors temps reel" : "") << "\nautorisations  "
        << static_cast<int>(t.authGrants.load()) << "  revocations " << static_cast<int>(t.authRevocations.load()) << "  generation "
        << static_cast<int>(t.authGeneration.load()) << "  journal perdu " << static_cast<int>(t.taDropped.load())
        << "\njournal fichier : " << juce::String(SubdestrucktProcessor::taFlagPath()) << " (present = actif)";
      const auto ev = ctx_.proc.taRecentEvents();
      for (size_t i = ev.size() > 8 ? ev.size() - 8 : 0; i < ev.size(); ++i) d << "\n  " << juce::String(ev[i]);
      diag_.setText(d, juce::dontSendNotification);
    }
  }
  void resized() override {
    auto r = getLocalBounds().reduced(4);
    auto tabs = r.removeFromTop(28);
    for (int k = 0; k < 6; ++k) tab_[k].setBounds(tabs.removeFromLeft(std::min(170, tabs.getWidth() / (6 - k))).reduced(2));
    r.removeFromTop(4);
    routesVp_.setBounds(r);
    routes_.setSize(std::max(700, r.getWidth() - 14), routes_.getHeight());
    mods_.setBounds(r);
    {
      auto bb = r;
      auto row = bb.removeFromTop(28);
      cycle_.setBounds(row.removeFromLeft(130).reduced(2));
      import_.setBounds(row.removeFromLeft(150).reduced(2));
      export_.setBounds(row.removeFromLeft(150).reduced(2));
      waveEd_.setBounds(bb.removeFromLeft(bb.getWidth() * 58 / 100).reduced(2));
      banks_.setBounds(bb.reduced(6, 2));
    }
    diag_.setBounds(r);
    auto a = r;
    auto l1 = a.removeFromTop(28);
    profile_.setBounds(l1.removeFromLeft(160).reduced(2));
    intensity_.setBounds(l1.removeFromLeft(260).reduced(2));
    rnd_.setBounds(l1.removeFromLeft(120).reduced(2));
    mut_.setBounds(l1.removeFromLeft(120).reduced(2));
    auto l2 = a.removeFromTop(28);
    for (auto& f : fam_) f.setBounds(l2.removeFromLeft(110).reduced(2));
    auto l3 = a.removeFromTop(28);
    evoMode_.setBounds(l3.removeFromLeft(120).reduced(2));
    evoRate_.setBounds(l3.removeFromLeft(220).reduced(2));
    evoStart_.setBounds(l3.removeFromLeft(110).reduced(2));
    evoStop_.setBounds(l3.removeFromLeft(70).reduced(2));
    evoReset_.setBounds(l3.removeFromLeft(70).reduced(2));
    evoCommit_.setBounds(l3.removeFromLeft(80).reduced(2));
    rndInfo_.setBounds(a.removeFromTop(60));
    auto s = r;
    auto s1 = s.removeFromTop(28);
    presetList_.setBounds(s1.removeFromLeft(320).reduced(2));
    load_.setBounds(s1.removeFromLeft(90).reduced(2));
    fav_.setBounds(s1.removeFromLeft(90).reduced(2));
    auto s2 = s.removeFromTop(28);
    saveName_.setBounds(s2.removeFromLeft(220).reduced(2));
    saveTags_.setBounds(s2.removeFromLeft(260).reduced(2));
    save_.setBounds(s2.removeFromLeft(110).reduced(2));
    restore_.setBounds(s.removeFromTop(26).reduced(2));
    stateInfo_.setBounds(s);
  }

 private:
  UiContext& ctx_;
  juce::TextButton tab_[6];
  RouteListView routes_;
  juce::Viewport routesVp_;
  ModListView mods_;
  juce::Label banks_, rndInfo_, stateInfo_, diag_;
  juce::ComboBox profile_, evoMode_, presetList_;
  juce::Slider intensity_, evoRate_;
  juce::ToggleButton fam_[static_cast<int>(RandomFamily::Count)], fav_, restore_;
  juce::TextButton rnd_, mut_, evoStart_, evoStop_, evoReset_, evoCommit_, load_, save_;
  juce::TextEditor saveName_, saveTags_;
  juce::ComboBox cycle_;
  juce::TextButton import_, export_;
  WaveEditor waveEd_;
  std::unique_ptr<juce::FileChooser> chooser_;
  int current_ = 0;
};

// ================================================================ éditeur principal
MainEditor::MainEditor(SubdestrucktProcessor& p) : juce::AudioProcessorEditor(p), proc_(p), ctx_{p, nullptr, RandomOptions{}} {
  setLookAndFeel(&look_);
  ctx_.select = [this](ParamId id) { selectParam(id); };
  const char* spaces[3] = {"PERFORMANCE", "SOUND DESIGN", "ENGINEERING"};
  for (int k = 0; k < 3; ++k) {
    spaceBtn_[k].setButtonText(spaces[k]);
    spaceBtn_[k].onClick = [this, k] { showSpace(k); };
    addAndMakeVisible(spaceBtn_[k]);
  }
  styleLabel(preset_, 13.0f, col::text);
  preset_.setFont(sans(13.0f, true));
  styleLabel(status_, 11.0f, col::dim);
  styleLabel(meter_, 11.0f, col::text, true);
  for (auto* l : {&preset_, &status_, &meter_}) addAndMakeVisible(*l);
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
  undo_.onClick = [this] { proc_.undo(); };
  redo_.onClick = [this] { proc_.redo(); };
  panic_.setColour(juce::TextButton::buttonColourId, col::alert);
  panic_.onClick = [this] { proc_.requestPanic(); };
  inspectorBtn_.setClickingTogglesState(true);
  inspectorBtn_.setToggleState(true, juce::dontSendNotification);
  inspectorBtn_.onClick = [this] {
    inspectorOpen_ = inspectorBtn_.getToggleState();
    resized();
  };
  for (int q = 0; q < 3; ++q) quality_.addItem(juce::String("Quality ") + qualityName(static_cast<Quality>(q)), q + 1);
  quality_.setSelectedId(static_cast<int>(proc_.quality()) + 1, juce::dontSendNotification);
  quality_.onChange = [this] { proc_.setQuality(static_cast<Quality>(quality_.getSelectedId() - 1)); };
  const int zooms[4] = {75, 100, 125, 150};
  for (int z = 0; z < 4; ++z) zoom_.addItem(juce::String(zooms[z]) + " %", zooms[z]);
  zoom_.setSelectedId(100, juce::dontSendNotification);
  zoom_.onChange = [this] {
    zoom_f_ = zoom_.getSelectedId() / 100.0f;
    resized();
  };
  drone_.onClick = [this] {  // clic de l'utilisateur : intention explicite (autorisation sans confirmation)
    proc_.setDrone(drone_.getToggleState());
    if (drone_.getToggleState()) proc_.authorizeAutonomy();
  };
  authorize_.setColour(juce::TextButton::buttonColourId, col::fm.darker(0.4f));
  authorize_.setTooltip("Etats autonomes suspendus (armements ou captures restaures par le projet, la duplication, un preset "
                        "ou le Panic) : cliquer pour autoriser leur activation dans cette instance");
  authorize_.setTitle("Autoriser les etats autonomes");
  authorize_.onClick = [this] { proc_.authorizeAutonomy(); };
  for (auto* c : std::initializer_list<juce::Component*>{&prev_, &next_, &undo_, &redo_, &panic_, &inspectorBtn_, &quality_, &zoom_, &drone_}) addAndMakeVisible(*c);
  addChildComponent(authorize_);
  perf_ = std::make_unique<PerformanceView>(ctx_);
  sound_ = std::make_unique<SoundDesignView>(ctx_);
  eng_ = std::make_unique<EngineeringView>(ctx_);
  inspector_ = std::make_unique<Inspector>(ctx_);
  content_.addAndMakeVisible(*perf_);
  content_.addChildComponent(*sound_);
  content_.addChildComponent(*eng_);
  addAndMakeVisible(content_);
  addAndMakeVisible(*inspector_);
  juce::Component::SafePointer<MainEditor> safe(this);
  proc_.setStateListener([safe] {
    juce::MessageManager::callAsync([safe] {
      MainEditor* e = safe.getComponent();
      if (e == nullptr) return;
      e->perf_->refreshContext();
      e->sound_->refreshState();
      e->eng_->refreshPresets();
    });
  });
  setResizable(true, true);
  setResizeLimits(960, 640, 2400, 1600);  // minimum ergonomique candidat (UI §6), à valider par tests
  setSize(1440, 900);
  showSpace(0);
  startTimerHz(30);
}

MainEditor::~MainEditor() {
  stopTimer();
  proc_.setStateListener(nullptr);
  setLookAndFeel(nullptr);
}

void MainEditor::selectParam(ParamId id) {
  if (inspector_) inspector_->show(id);
}

void MainEditor::showSpace(int k) {
  space_ = k;
  for (int i = 0; i < 3; ++i) spaceBtn_[i].setToggleState(i == k, juce::dontSendNotification);
  perf_->setVisible(k == 0);
  sound_->setVisible(k == 1);
  eng_->setVisible(k == 2);
  if (k == 1) sound_->refreshState();
  if (k == 2) eng_->refresh();
}

void MainEditor::showSubTab(int space, int tab) {
  showSpace(space);
  if (space == 1) sound_->showTab(tab);
  if (space == 2) eng_->showTab(tab);
}

void MainEditor::paint(juce::Graphics& g) {
  g.fillAll(col::chassis);
  g.setColour(col::panel.darker(0.3f));
  g.fillRect(0, 0, getWidth(), kBarH);
}

void MainEditor::layoutContent() {
  auto r = getLocalBounds().withTrimmedTop(kBarH);
  if (inspectorOpen_) inspector_->setBounds(r.removeFromRight(kInspectorW));
  inspector_->setVisible(inspectorOpen_);
  // zoom global : le contenu est disposé à la taille logique puis mis à l'échelle
  const int lw = static_cast<int>(r.getWidth() / zoom_f_), lh = static_cast<int>(r.getHeight() / zoom_f_);
  content_.setBounds(0, 0, lw, lh);
  content_.setTransform(juce::AffineTransform::scale(zoom_f_).translated(static_cast<float>(r.getX()), static_cast<float>(r.getY())));
  for (auto* v : std::initializer_list<juce::Component*>{perf_.get(), sound_.get(), eng_.get()}) v->setBounds(0, 0, lw, lh);
}

void MainEditor::resized() {
  auto bar = getLocalBounds().removeFromTop(kBarH).reduced(4);
  prev_.setBounds(bar.removeFromLeft(24));
  preset_.setBounds(bar.removeFromLeft(190));
  next_.setBounds(bar.removeFromLeft(24));
  bar.removeFromLeft(6);
  undo_.setBounds(bar.removeFromLeft(56).reduced(1));
  redo_.setBounds(bar.removeFromLeft(56).reduced(1));
  bar.removeFromLeft(8);
  for (int k = 0; k < 3; ++k) spaceBtn_[k].setBounds(bar.removeFromLeft(118).reduced(1));
  panic_.setBounds(bar.removeFromRight(70).reduced(1));
  inspectorBtn_.setBounds(bar.removeFromRight(82).reduced(1));
  zoom_.setBounds(bar.removeFromRight(76).reduced(1));
  drone_.setBounds(bar.removeFromRight(72).reduced(1));
  authorize_.setBounds(bar.removeFromRight(86).reduced(1));
  quality_.setBounds(bar.removeFromRight(140).reduced(1));
  meter_.setBounds(bar.removeFromRight(150));
  status_.setBounds(bar.reduced(4, 0));
  layoutContent();
}

void MainEditor::refreshBar() {
  preset_.setText(juce::String(proc_.presetName()) + (proc_.dirty() ? " *" : ""), juce::dontSendNotification);
  undo_.setEnabled(proc_.canUndo());
  redo_.setEnabled(proc_.canRedo());
  undo_.setTooltip(proc_.undoLabel());
  redo_.setTooltip(proc_.redoLabel());
  drone_.setToggleState(proc_.drone(), juce::dontSendNotification);
  drone_.setColour(juce::ToggleButton::textColourId, proc_.drone() ? col::alert : col::text);
  authorize_.setVisible(proc_.autonomySuspended());
  auto& t = proc_.telemetry();
  const float pk = std::max(t.peakL.load(), t.peakR.load());
  const float db = pk > 1e-6f ? 20.0f * std::log10(pk) : -120.0f;
  const auto elp = t.elpHits.load();
  meter_.setText("OUT " + juce::String(db, 1) + " dBFS" + (db > 0.0f ? " >0" : "") + (elp > 0 ? " ELP " + juce::String(static_cast<int>(elp)) : juce::String()),
                 juce::dontSendNotification);
  meter_.setColour(juce::Label::textColourId, (db > 0.0f || elp > 0) ? col::alert : col::text);
  status_.setText(proc_.lastMessage(), juce::dontSendNotification);
}

void MainEditor::timerCallback() {
  refreshBar();
  if (++ticks_ % 15 == 0) perf_->refreshContext();
  if (space_ == 0) perf_->refreshTelemetry();
  if (space_ == 1) sound_->refreshTelemetry();
  if (space_ == 2) eng_->refresh();
  if (inspectorOpen_) inspector_->refresh();
}

}  // namespace sbd::ui
