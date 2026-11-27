// SUBDESTRUCKT — UI — espace SOUND DESIGN (ARCH-UI-04). WT et FM de poids comparable, CROSS au centre ; traitements
// contextuels à l'algorithme (valeurs inactives conservées) ; ordre effectif des transformations Z représenté tel que
// le moteur l'applique (restriction inter-domaines non arbitrée : signalée, jamais présentée comme résolue).
#include "dsp/Destruction.h"
#include "dsp/Filters.h"
#include "Views.h"

namespace sbd::ui {

using Groups = std::vector<std::pair<juce::String, std::vector<ParamId>>>;

// ================================================================ paramètres contextuels (exposés aux tests)
std::vector<ParamId> filterParamsFor(ParamId typeParam, int type) {
  using F = FltOff;
  auto o = [](F f) { return static_cast<int>(f); };
  std::vector<int> v{o(F::Type)};
  switch (type) {
    case 1: v.insert(v.end(), {o(F::F1Mode), o(F::Cutoff), o(F::Res), o(F::Key)}); break;
    case 2: v.insert(v.end(), {o(F::Cutoff), o(F::CombFb), o(F::CombDamp), o(F::Key), o(F::Detune)}); break;
    case 3: v.insert(v.end(), {o(F::F3Mode), o(F::Cutoff), o(F::Res), o(F::Key), o(F::Drive), o(F::Bias), o(F::Char)}); break;
    case 4: v.insert(v.end(), {o(F::Cutoff), o(F::Key), o(F::Modal), o(F::Modes), o(F::Decay), o(F::Damp), o(F::Bright), o(F::Inharm), o(F::Freeze)}); break;
    default: return filterOffsets(typeParam, v);
  }
  v.insert(v.end(), {o(F::Pos), o(F::Stereo), o(F::Spread), o(F::Exc), o(F::ExcLevel), o(F::Mix), o(F::Out), o(F::Arm)});
  return filterOffsets(typeParam, v);
}

std::vector<ParamId> destructionParamsFor(ParamId typeParam, int type) {
  std::vector<int> v{0};
  static const int first[9] = {0, 4, 10, 15, 23, 29, 35, 42, 49};
  static const int count[9] = {0, 6, 5, 8, 6, 6, 7, 7, 7};
  if (type > 0 && type <= 8) {
    for (int k = 0; k < count[type]; ++k) v.push_back(first[type] + k);
    v.push_back(static_cast<int>(DstOff::Mix));
    v.push_back(static_cast<int>(DstOff::Out));
    if (type == 5 || type == 6 || type == 8) v.push_back(static_cast<int>(DstOff::Arm));  // états autonomes possibles
  }
  auto ids = filterOffsets(typeParam, v);
  if (type == static_cast<int>(DstType::D2)) {  // mode de Feedback D2 (append-only, hors bloc contigu)
    const int slot = (idx(typeParam) - idx(ParamId::DstWt1Type)) / static_cast<int>(DstOff::Count);
    ids.push_back(static_cast<ParamId>(idx(ParamId::DstWt1D2FbMode) + slot));
  }
  return ids;
}

namespace {

// ================================================================ colonne contextuelle générique
class ContextColumn : public Plate {
 public:
  using GroupsFn = std::function<Groups(int tab, const ParamValues&)>;
  using DecorateFn = std::function<void(juce::Component&, const ParamValues&)>;
  ContextColumn(UiContext& c, juce::String title, juce::Colour accent, juce::StringArray tabs, GroupsFn fn, Density d = Density::S)
      : Plate(std::move(title), accent), ctx_(c), tabs_(accent), scroll_(c, d), fn_(std::move(fn)) {
    tabs_.setTabs(tabs);
    tabs_.onChange = [this](int) { refresh(true); };
    scroll_.panel.setAccent(accent);
    if (tabs.size() > 1) addAndMakeVisible(tabs_);
    addAndMakeVisible(scroll_);
  }
  void setDecorate(DecorateFn f) { decorate_ = std::move(f); }
  void setHeader(juce::Component* h, int height) {
    header_ = h;
    headerH_ = height;
    if (h) addAndMakeVisible(*h);
  }
  void setBanner(juce::Component* b, int height) {
    banner_ = b;
    bannerH_ = height;
    if (b) addChildComponent(*b);
  }
  void setBannerVisible(bool v) {
    if (banner_ && banner_->isVisible() != v) {
      banner_->setVisible(v);
      resized();
    }
  }
  int numTabs() const { return tabs_.count(); }
  juce::String tabName(int k) const { return tabs_.name(k); }
  void setTab(int k) {
    tabs_.setCurrent(k);
    refresh(true);
  }
  int tab() const { return tabs_.current(); }
  void refresh(bool relayout = false) {
    const auto pv = ctx_.proc.parameters().values();
    const auto g = fn_(tabs_.current(), pv);
    if (g != last_) {
      last_ = g;
      scroll_.panel.setGroups(g);
      relayout = true;
    }
    if (decorate_)
      for (auto& c : scroll_.panel.controls()) decorate_(*c, pv);
    if (relayout) resized();
  }
  void resized() override {
    auto r = content();
    if (header_) header_->setBounds(r.removeFromTop(headerH_));
    if (tabs_.count() > 1) {
      tabs_.setBounds(r.removeFromTop(tok::tabH));
      r.removeFromTop(4);
    }
    if (banner_ && banner_->isVisible()) {
      banner_->setBounds(r.removeFromTop(bannerH_));
      r.removeFromTop(4);
    }
    scroll_.setBounds(r);
    scroll_.resized();
  }
  TabStrip& tabStrip() { return tabs_; }

 private:
  UiContext& ctx_;
  TabStrip tabs_;
  ScrollPanel scroll_;
  GroupsFn fn_;
  DecorateFn decorate_;
  Groups last_;
  juce::Component *header_ = nullptr, *banner_ = nullptr;
  int headerH_ = 0, bannerH_ = 0;
};

// ================================================================ Z : ordre effectif (moteur), jamais idéalisé
int zDomain(int type) { return type == 0 ? -1 : type == 1 ? 3 : (type == 2 || type == 3) ? 1 : 0; }  // 0 spectral, 1 phase, 3 forme

class ZOrderDisplay : public OpticalDisplay {
 public:
  explicit ZOrderDisplay(UiContext& c) : OpticalDisplay("ORDRE EFFECTIF DES TRANSFORMATIONS Z", "moteur 0.3.3"), ctx_(c) {}
  void refresh() {
    const auto pv = ctx_.proc.parameters().values();
    const int a = pv.choice(ParamId::WtZAType), b = pv.choice(ParamId::WtZBType), ord = pv.choice(ParamId::WtZOrder);
    const juce::String na = juce::String(choiceLabel(ParamId::WtZAType, a)), nb = juce::String(choiceLabel(ParamId::WtZBType, b));
    // composition du moteur : spectral (table) -> phase (Warp, Mutation) -> lecture -> forme d'onde (Fold) ;
    // A>B / B>A ne s'applique qu'entre transformations d'un même domaine (Z-CONTRACT §3, §3 ter)
    juce::StringArray stages[4];
    const int da = zDomain(a), db = zDomain(b);
    auto put = [&](int d, const juce::String& s) {
      if (d >= 0) stages[d].add(s);
    };
    if (da == db && da >= 0) {
      if (ord == 0) put(da, "A " + na), put(db, "B " + nb);
      else put(db, "B " + nb), put(da, "A " + na);
    } else {
      put(da, "A " + na);
      put(db, "B " + nb);
    }
    juce::String t = juce::String::fromUTF8("spectral [") + stages[0].joinIntoString(" > ") + juce::String::fromUTF8("]  ›  phase [") +
                     stages[1].joinIntoString(" > ") + juce::String::fromUTF8("]  ›  lecture  ›  forme [") + stages[3].joinIntoString(" > ") + "]";
    juce::String note;
    if (da >= 0 && db >= 0 && da != db)
      note = juce::String::fromUTF8("A et B dans des domaines différents : l'ordre A>B / B>A est SANS EFFET (ordre fixe du moteur). "
                                    "Restriction NON arbitrée (Z-CONTRACT §3 ter).");
    else if (da >= 0 && db >= 0)
      note = juce::String::fromUTF8("Même domaine : l'ordre ") + (ord == 0 ? "A>B" : "B>A") + juce::String::fromUTF8(" est appliqué.");
    else
      note = juce::String::fromUTF8("Un seul emplacement actif : l'ordre A/B est sans objet.");
    note << juce::String::fromUTF8("  Spectral : resynthèse ≤ 1/128 éch. moteur (modulation audio atténuée).");
    if (t != text_ || note != note_) {
      text_ = t;
      note_ = note;
      repaint();
    }
  }
  const juce::String& pipeline() const { return text_; }
  const juce::String& note() const { return note_; }
  void paint(juce::Graphics& g) override {
    OpticalDisplay::paint(g);
    auto r = content().toFloat();
    g.setFont(numFont(10.5f));
    g.setColour(tok::wt);
    g.drawFittedText(text_, r.removeFromTop(16.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.85f);
    g.setFont(uiFont(10.0f));
    g.setColour(note_.contains("SANS EFFET") ? tok::warn : tok::textDim);
    g.drawFittedText(note_, r.toNearestInt(), juce::Justification::topLeft, 3, 1.0f);
  }

 private:
  UiContext& ctx_;
  juce::String text_, note_;
};

// ================================================================ chaînes de traitement
class SlotPlate : public Plate {
 public:
  SlotPlate(UiContext& c, juce::String title) : Plate(std::move(title), tok::textDim), panel(c, Density::S) { addAndMakeVisible(panel); }
  int preferredHeight(int w) const { return tok::sectionHead + 2 * tok::pad + panel.preferredHeight(w - 2 * tok::pad); }
  void resized() override { panel.setBounds(content()); }
  ParamPanel panel;
};

class ChainColumn : public Plate {
 public:
  ChainColumn(UiContext& c, int location)
      : Plate(location == 0 ? juce::String::fromUTF8("CHAÎNE WT") : location == 1 ? juce::String::fromUTF8("CHAÎNE FM") : juce::String::fromUTF8("CHAÎNE GLOBALE"), location == 0 ? tok::wt : location == 1 ? tok::fm : tok::text),
        ctx_(c), loc_(location), filter_(c, "FILTRE"), d1_(c, "DESTRUCTION 1"), d2_(c, "DESTRUCTION 2"), out_(c, juce::String::fromUTF8("SORTIE  //  sûreté")) {
    setTitle2(loc_ == 0 ? juce::String::fromUTF8("CHAÎNE WT") : loc_ == 1 ? juce::String::fromUTF8("CHAÎNE FM") : juce::String::fromUTF8("CHAÎNE GLOBALE"),
              loc_ == 2 ? juce::String::fromUTF8("après Cross Audio") : juce::String::fromUTF8("avant Cross Audio"));
    static constexpr ParamId kF[3] = {ParamId::FltWtType, ParamId::FltFmType, ParamId::FltGlType};
    static constexpr ParamId kD[3][2] = {{ParamId::DstWt1Type, ParamId::DstWt2Type}, {ParamId::DstFm1Type, ParamId::DstFm2Type}, {ParamId::DstGl1Type, ParamId::DstGl2Type}};
    fBase_ = kF[location];
    dBase_[0] = kD[location][0];
    dBase_[1] = kD[location][1];
    content_.addAndMakeVisible(filter_);
    content_.addAndMakeVisible(d1_);
    content_.addAndMakeVisible(d2_);
    if (loc_ == 2) content_.addAndMakeVisible(out_);
    vp_.setViewedComponent(&content_, false);
    vp_.setScrollBarsShown(true, false);
    vp_.setScrollBarThickness(8);
    addAndMakeVisible(vp_);
    refresh();
  }
  // vrai si la disposition a changé (algorithme, ordre Pre/Post, limiteur)
  bool refresh() {
    const auto pv = ctx_.proc.parameters().values();
    const int tf = pv.choice(fBase_), t1 = pv.choice(dBase_[0]), t2 = pv.choice(dBase_[1]);
    const bool post = pv.flag(static_cast<ParamId>(idx(fBase_) + static_cast<int>(FltOff::Pos)));
    const bool lim = pv.flag(ParamId::OutLimiter);
    if (tf == tf_ && t1 == t1_ && t2 == t2_ && post == post_ && lim == lim_) return false;
    tf_ = tf, t1_ = t1, t2_ = t2, post_ = post, lim_ = lim;
    filter_.panel.setParams(filterParamsFor(fBase_, tf));
    d1_.panel.setParams(destructionParamsFor(dBase_[0], t1));
    d2_.panel.setParams(destructionParamsFor(dBase_[1], t2));
    filter_.setTitle2("FILTRE", tf == 0 ? juce::String("Off") : juce::String(post ? "Post" : "Pre") + juce::String::fromUTF8("  ·  ") + juce::String(choiceLabel(fBase_, tf)));
    d1_.setTitle2("DESTRUCTION 1", juce::String(choiceLabel(dBase_[0], t1)));
    d2_.setTitle2("DESTRUCTION 2", juce::String(choiceLabel(dBase_[1], t2)));
    for (auto* p : {&d1_, &d2_}) p->setAccent(tok::textDim);
    filter_.setAccent(accentOf(loc_ == 0 ? Domain3::Wt : loc_ == 1 ? Domain3::Fm : Domain3::Neutral));
    if (loc_ == 2) out_.panel.setParams(lim ? std::vector<ParamId>{ParamId::OutLimiter, ParamId::OutCeiling} : std::vector<ParamId>{ParamId::OutLimiter});
    resized();
    return true;
  }
  void resized() override {
    vp_.setBounds(content());
    const int w = std::max(200, vp_.getWidth() - 10);
    // l'ordre affiché suit l'ordre effectif : filtre avant (Pre) ou après (Post) les deux emplacements
    std::vector<SlotPlate*> order = post_ ? std::vector<SlotPlate*>{&d1_, &d2_, &filter_} : std::vector<SlotPlate*>{&filter_, &d1_, &d2_};
    if (loc_ == 2) order.push_back(&out_);
    int y = 0;
    for (auto* p : order) {
      const int h = p->preferredHeight(w);
      p->setBounds(0, y, w, h);
      y += h + tok::gap / 2;
    }
    content_.setSize(w, std::max(y, vp_.getHeight()));
  }

 private:
  UiContext& ctx_;
  int loc_;
  SlotPlate filter_, d1_, d2_, out_;
  juce::Component content_;
  juce::Viewport vp_;
  ParamId fBase_, dBase_[2];
  int tf_ = -1, t1_ = -1, t2_ = -1;
  bool post_ = false, lim_ = false;
};

// ================================================================ SOUND DESIGN
std::vector<ParamId> opParams(int k, const ParamValues& pv) {
  static constexpr ParamId kTune[4] = {ParamId::Op1Tune, ParamId::Op2Tune, ParamId::Op3Tune, ParamId::Op4Tune};
  static constexpr ParamId kLevel[4] = {ParamId::Op1Level, ParamId::Op2Level, ParamId::Op3Level, ParamId::Op4Level};
  static constexpr ParamId kRatio[4] = {ParamId::Op1Ratio, ParamId::Op2Ratio, ParamId::Op3Ratio, ParamId::Op4Ratio};
  auto o = [&](int off) { return static_cast<ParamId>(idx(kTune[k]) + off); };
  return {kLevel[k], kRatio[k], o(0), o(1), o(2), o(3), o(4), o(5), o(6), o(7), o(8), o(9), o(10), o(11), o(12), o(13), o(14), o(15)};
  juce::ignoreUnused(pv);
}

Groups opGroups(int k, const ParamValues& pv) {
  const auto all = opParams(k, pv);
  auto p = [&](int i) { return all[static_cast<size_t>(i)]; };  // 0 Level, 1 Ratio, 2 Tune, 3 Fixed, 4 Offset, 5 Coarse, 6 Fine, 7 Wave, 8 WtX, 9 WtY ...
  const bool fixed = pv.choice(p(2)) == 1;
  const bool wtWave = pv.choice(p(7)) == 8;
  std::vector<ParamId> pitch{p(2)};
  pitch.push_back(fixed ? p(3) : p(1));  // Ratio (mode Ratio) ou Fixed (mode Fixed) : l'autre est conservé
  for (int i : {5, 6, 4}) pitch.push_back(p(i));
  std::vector<ParamId> wave{p(7)};
  if (wtWave) wave.push_back(p(8)), wave.push_back(p(9));
  return {{"Hauteur", pitch},
          {"Forme", wave},
          {juce::String::fromUTF8("Niveau audio  (≠ modulation)"), {p(0), p(10), p(11), p(12)}},
          {"Modulation", {p(16), p(15), p(13), p(14), p(17)}}};
}

class SoundDesignView : public SpaceView {
 public:
  explicit SoundDesignView(UiContext& c)
      : SpaceView(c),
        tabs_(tok::cross),
        wt_(c, "WAVETABLE", tok::wt, {"XYZ", "LECTURE", "PHASE / SYNC", "Z", juce::String::fromUTF8("CARACTÈRE")}, wtGroups),
        cross_(c, "CROSS  //  interaction des moteurs", tok::cross, {"DIRECT", juce::String::fromUTF8("WT › FM"), juce::String::fromUTF8("FM › WT"), "BUS / AUDIO"}, crossGroups),
        fm_(c, "FM / PM", tok::fm, {"OP1", "OP2", "OP3", "OP4", "MATRICE", "COMMUN"}, fmGroups),
        zDisp_(c),
        wtChain_(c, 0),
        fmChain_(c, 1),
        glChain_(c, 2),
        env_(c, "ENVELOPPES", tok::mod, {"AMP", "WT", "FM", "AUX1", "AUX2"}, envGroups),
        lfo_(c, "LFO", tok::mod, {"LFO1", "LFO2", "LFO3", "LFO4"}, lfoGroups),
        rnd_(c, juce::String::fromUTF8("ALÉATOIRE · CHAOS · SUIVEURS"), tok::mod, {""}, rndGroups),
        envEd_(c),
        lfoEd_("FORME CUSTOM DU LFO", tok::mod, true),
        crossReset_("Cross Reset", tok::cross),
        excite_("Excitation", tok::text),
        capture_("Capturer gels", tok::warn) {
    tabs_.setTabs({"MOTEURS", "TRAITEMENTS", "MODULATION"});
    tabs_.onChange = [this](int k) { showTab(k); };
    addAndMakeVisible(tabs_);
    // MOTEURS
    for (const auto& id : FactoryBanks::ids()) bankIds_.push_back(id), bank_.addItem(FactoryBanks::displayName(id), static_cast<int>(bankIds_.size()));
    bank_.setTitle("Banque wavetable");
    bank_.setTooltip(juce::String::fromUTF8("Banque active (immuable ; édition : Engineering › Bank Laboratory)"));
    bank_.onChange = [this] {
      const int k = bank_.getSelectedId() - 1;
      if (k >= 0 && k < static_cast<int>(bankIds_.size()) && bankIds_[static_cast<size_t>(k)] != ctx_.proc.currentBank()) ctx_.proc.setBank(bankIds_[static_cast<size_t>(k)]);
    };
    wt_.setHeader(&bank_, 28);
    wt_.setBanner(&zDisp_, 78);
    wt_.setDecorate([](juce::Component& comp, const ParamValues& pv) {
      if (auto* cc = dynamic_cast<ContinuousControl*>(&comp)) {
        const auto id = cc->boundId();
        if (id == ParamId::WtZAShape || id == ParamId::WtZBShape) {
          const int t = pv.choice(id == ParamId::WtZAShape ? ParamId::WtZAType : ParamId::WtZBType);
          cc->setInactive(t >= 1 && t <= 3, juce::String::fromUTF8("Shape ignoré par Fold, Warp, Mutation"));
        }
      }
    });
    crossReset_.setTooltip(juce::String::fromUTF8("Cross Reset : phases conservées, historiques CROSS purgés (action momentanée)"));
    crossReset_.onClick = [this] { ctx_.proc.requestCrossReset(); };
    cross_.setHeader(&crossReset_, 30);
    cross_.setDecorate([this](juce::Component& comp, const ParamValues& pv) {
      auto* cc = dynamic_cast<ContinuousControl*>(&comp);
      if (!cc) return;
      const int s = slotOf(cc->boundId());
      if (s < 0) return;
      const auto& info = routeSlotInfo(s);
      const int dir = pv.choice(ParamId::CrossDir);
      bool present = false;
      for (const auto& r : routes_) present = present || (r.src == info.src && r.dst == info.dst);
      if (!present) cc->setInactive(true, juce::String::fromUTF8("route absente du réseau (Engineering › Cross Network)"));
      else if ((dir == 2 && info.kind == RouteKind::CrossForward) || (dir == 1 && info.kind == RouteKind::CrossReverse))
        cc->setInactive(true, juce::String::fromUTF8("direction CROSS exclue (cross.dir)"));
      else cc->setInactive(false);
    });
    for (int k = 0; k < kNumFmTopologies; ++k) topo_.addItem(fmTopologyName(k), k + 1);
    topo_.setTextWhenNothingSelected(juce::String::fromUTF8("Topologie FM préréglée…"));
    topo_.setTitle(juce::String::fromUTF8("Topologie FM préréglée"));
    topo_.setTooltip(juce::String::fromUTF8("Applique une topologie FM préréglée (transaction validée, annulable)"));
    topo_.onChange = [this] {
      if (topo_.getSelectedId() > 0) ctx_.proc.applyFmTopology(topo_.getSelectedId() - 1);
      topo_.setSelectedId(0, juce::dontSendNotification);
    };
    fm_.setHeader(&topo_, 28);
    fm_.setDecorate([this](juce::Component& comp, const ParamValues&) {
      auto* cc = dynamic_cast<ContinuousControl*>(&comp);
      if (!cc) return;
      const int s = slotOf(cc->boundId());
      if (s < 0 || routeSlotInfo(s).kind != RouteKind::Internal) return;
      const auto& info = routeSlotInfo(s);
      cc->setCaption(juce::String(nodeName(info.src)) + juce::String::fromUTF8(" › ") + (info.src == info.dst ? juce::String("self") : juce::String(portName(info.dst))));
      bool present = false;
      for (const auto& r : routes_) present = present || (r.src == info.src && r.dst == info.dst);
      cc->setInactive(!present, juce::String::fromUTF8("route absente (Engineering › FM Network)"));
    });
    for (auto* x : std::initializer_list<juce::Component*>{&wt_, &cross_, &fm_}) addAndMakeVisible(*x);
    // TRAITEMENTS
    flow_.setTap(juce::String::fromUTF8("ordre réel du moteur"));
    excite_.setTooltip(juce::String::fromUTF8("Excitation manuelle des filtres F4 (action momentanée)"));
    excite_.onClick = [this] { ctx_.proc.requestManualExcite(); };
    capture_.setTooltip(juce::String::fromUTF8("Capture le contenu Glitch D2 et le spectre Freeze S2 des emplacements actifs (sauvegarde preset/projet)"));
    capture_.onClick = [this] {
      for (int k = 0; k < 6; ++k) ctx_.proc.requestCapture(k);
      capturePending_ = 20;  // collecte aux rafraîchissements suivants (un bloc audio est requis)
    };
    for (auto* x : std::initializer_list<juce::Component*>{&flow_, &wtChain_, &fmChain_, &glChain_, &excite_, &capture_}) addChildComponent(*x);
    // MODULATION
    env_.setBanner(&envEd_, 150);
    env_.setBannerVisible(true);
    env_.tabStrip().onChange = [this](int k) {
      envEd_.setEnvelope(k);
      env_.setTab(k);
    };
    lfo_.setBanner(&lfoEd_, 150);
    lfoEd_.onCommit = [this](int n, const float* x, const float* y) {
      CurvePoints cp;
      cp.n = static_cast<std::uint8_t>(n);
      for (int i = 0; i < n; ++i) cp.x[static_cast<size_t>(i)] = x[i], cp.y[static_cast<size_t>(i)] = y[i];
      ctx_.proc.setLfoCurve(lfo_.tab(), cp);
    };
    lfo_.tabStrip().onChange = [this](int k) {
      lfo_.setTab(k);
      syncLfoCurve();
    };
    for (auto* x : std::initializer_list<juce::Component*>{&env_, &lfo_, &rnd_}) addChildComponent(*x);
    showTab(0);
    refreshState();
  }

  int numTabs() const override { return 3; }
  juce::String tabName(int k) const override { return k == 0 ? "MOTEURS" : k == 1 ? "TRAITEMENTS" : "MODULATION"; }
  int currentTab() const override { return tab_; }
  void showTab(int k) override {
    tab_ = juce::jlimit(0, 2, k);
    tabs_.setCurrent(tab_);
    for (auto* x : std::initializer_list<juce::Component*>{&wt_, &cross_, &fm_}) x->setVisible(tab_ == 0);
    for (auto* x : std::initializer_list<juce::Component*>{&flow_, &wtChain_, &fmChain_, &glChain_, &excite_, &capture_}) x->setVisible(tab_ == 1);
    for (auto* x : std::initializer_list<juce::Component*>{&env_, &lfo_, &rnd_}) x->setVisible(tab_ == 2);
    refreshState();
  }
  int numSubContexts(int tab) const override { return tab == 0 ? 6 : tab == 2 ? 5 : 1; }
  void showSubContext(int tab, int sub) override {
    showTab(tab);
    if (tab == 0) {
      wt_.setTab(std::min(sub, wt_.numTabs() - 1));
      cross_.setTab(std::min(sub, cross_.numTabs() - 1));
      fm_.setTab(std::min(sub, fm_.numTabs() - 1));
    }
    if (tab == 2) {
      env_.setTab(std::min(sub, 4));
      envEd_.setEnvelope(std::min(sub, 4));
      lfo_.setTab(std::min(sub, 3));
      syncLfoCurve();
    }
    refreshState();
  }
  juce::String subContextName(int tab, int sub) const override {
    auto nm = [](const ContextColumn& c, int k) { return c.tabName(std::min(k, c.numTabs() - 1)); };
    if (tab == 0) return "WT " + nm(wt_, sub) + " | CROSS " + nm(cross_, sub) + " | FM " + nm(fm_, sub);
    if (tab == 2) return "ENV " + nm(env_, sub) + " | " + nm(lfo_, sub);
    return {};
  }

  void refreshState() override {
    routes_ = ctx_.proc.currentRoutes();
    if (tab_ == 0) {
      const auto cur = ctx_.proc.currentBank();
      for (const auto& u : UserBanks::ids())
        if (std::find(bankIds_.begin(), bankIds_.end(), u) == bankIds_.end()) {
          bankIds_.push_back(u);
          bank_.addItem(juce::String(resolveBank(u)->name()) + " [user]", static_cast<int>(bankIds_.size()));
        }
      for (size_t k = 0; k < bankIds_.size(); ++k)
        if (bankIds_[k] == cur) bank_.setSelectedId(static_cast<int>(k) + 1, juce::dontSendNotification);
      wt_.setBannerVisible(wt_.tab() == 3);
      zDisp_.refresh();
      wt_.refresh();
      cross_.refresh();
      fm_.refresh();
    }
    if (tab_ == 1) {
      const bool a = wtChain_.refresh(), b = fmChain_.refresh(), c = glChain_.refresh();
      juce::ignoreUnused(a, b, c);
      const auto pv = ctx_.proc.parameters().values();
      flowText_ = juce::String::fromUTF8("( WT › balance › [chaîne WT] )  +  ( FM › balance › [chaîne FM] )  ›  Cross Audio (") +
                  juce::String(choiceLabel(ParamId::CrossAudioMode, pv.choice(ParamId::CrossAudioMode))) +
                  juce::String::fromUTF8(")  ›  [chaîne globale]  ›  sortie\nbalance WT/FM post-Amp avant les chaînes  ·  latence déclarée à l'hôte ") +
                  juce::String(ctx_.proc.getLatencySamples()) + juce::String::fromUTF8(" éch.");
      flow_.repaint();
    }
    if (tab_ == 2) {
      env_.refresh();
      lfo_.refresh();
      rnd_.refresh();
      syncLfoCurve();
    }
  }

  void tick() override {
    if (capturePending_ > 0) {
      --capturePending_;
      ctx_.proc.collectCaptures(0x3Fu);
      const int n = static_cast<int>(ctx_.proc.captures().size());
      capture_.setButtonText(n > 0 ? "Capturer gels (" + juce::String(n) + ")" : "Capturer gels");
    }
    if (++n_ % 6 == 0) refreshState();  // disposition contextuelle (algorithme changé par l'hôte, preset, Undo)
    if (tab_ == 2) envEd_.refresh();
  }

  void resized() override {
    auto r = getLocalBounds().reduced(tok::gap);
    tabs_.setBounds(r.removeFromTop(tok::tabH).removeFromLeft(std::max(360, tabs_.preferredWidth())));
    r.removeFromTop(tok::gap);
    {  // MOTEURS : WT | CROSS (central) | FM
      auto a = r;
      const int w = (a.getWidth() - 2 * tok::gap) / 3;
      wt_.setBounds(a.removeFromLeft(w));
      a.removeFromLeft(tok::gap);
      fm_.setBounds(a.removeFromRight(w));
      a.removeFromRight(tok::gap);
      cross_.setBounds(a);
    }
    {  // TRAITEMENTS
      auto a = r;
      auto head = a.removeFromTop(64);
      auto act = head.removeFromRight(260);
      excite_.setBounds(act.removeFromTop(30).removeFromLeft(124));
      act.removeFromTop(4);
      capture_.setBounds(act.removeFromTop(30).removeFromLeft(160));
      flow_.setBounds(head.withTrimmedRight(tok::gap));
      a.removeFromTop(tok::gap);
      const int w = (a.getWidth() - 2 * tok::gap) / 3;
      wtChain_.setBounds(a.removeFromLeft(w));
      a.removeFromLeft(tok::gap);
      glChain_.setBounds(a.removeFromRight(w));
      a.removeFromRight(tok::gap);
      fmChain_.setBounds(a);
    }
    {  // MODULATION
      auto a = r;
      const int w = (a.getWidth() - 2 * tok::gap) / 3;
      env_.setBounds(a.removeFromLeft(w));
      a.removeFromLeft(tok::gap);
      rnd_.setBounds(a.removeFromRight(w));
      a.removeFromRight(tok::gap);
      lfo_.setBounds(a);
    }
  }
  void paintOverChildren(juce::Graphics& g) override {
    if (tab_ != 1) return;
    auto r = flow_.content().toFloat() + flow_.getPosition().toFloat();
    g.setFont(numFont(10.5f));
    g.setColour(tok::text);
    g.drawFittedText(flowText_, r.toNearestInt(), juce::Justification::topLeft, 3, 0.9f);
  }

 private:
  static int slotOf(ParamId id) {
    for (int s = 0; s < kMaxRoutes; ++s)
      if (routeSlotInfo(s).depthParam == id) return s;
    return -1;
  }
  static Groups wtGroups(int tab, const ParamValues& pv) {
    switch (tab) {
      case 0: return {{"Position", {ParamId::WtX, ParamId::WtY, ParamId::WtZ}}, {"Niveau et hauteur", {ParamId::WtLevel, ParamId::WtCoarse, ParamId::WtFine}}};
      case 1: return {{"Lecture", {ParamId::WtRender, ParamId::WtInterp}}, {"Axe X", {ParamId::WtXBound, ParamId::WtXMorph}}, {"Axe Y", {ParamId::WtYBound, ParamId::WtYMorph}}};
      case 2: {
        std::vector<ParamId> sync{ParamId::WtSync};
        if (pv.choice(ParamId::WtSync) != 0) sync.push_back(ParamId::WtSyncSrc);
        std::vector<ParamId> ph{ParamId::WtPhaseMode};
        if (pv.choice(ParamId::WtPhaseMode) == 2) ph.push_back(ParamId::WtPhase);  // phase de départ : mode Fixed
        return {{"Phase", ph}, {"Synchronisation", sync}};
      }
      case 3: {
        auto slot = [&](ParamId type, ParamId depth, ParamId curve, ParamId shape) {
          std::vector<ParamId> v{type};
          const int t = pv.choice(type);
          if (t != 0) v.push_back(depth), v.push_back(curve), v.push_back(shape);
          return v;
        };
        return {{"Emplacement A", slot(ParamId::WtZAType, ParamId::WtZADepth, ParamId::WtZACurve, ParamId::WtZAShape)},
                {"Emplacement B", slot(ParamId::WtZBType, ParamId::WtZBDepth, ParamId::WtZBCurve, ParamId::WtZBShape)},
                {"Commun", {ParamId::WtZOrder, ParamId::WtZFund, ParamId::WtZGain}}};
      }
      default: return {{juce::String::fromUTF8("Caractère"), {ParamId::WtCharMode, ParamId::WtChar}}};
    }
  }
  static Groups crossGroups(int tab, const ParamValues& pv) {
    if (tab == 0) return {{"Direct", {ParamId::CrossMaster, ParamId::CrossFwd, ParamId::CrossRev, ParamId::CrossDir}}};
    if (tab == 1) return {{juce::String::fromUTF8("WT › opérateurs (phase)"), {ParamId::DepthWtOp1, ParamId::DepthWtOp2, ParamId::DepthWtOp3, ParamId::DepthWtOp4}}};
    if (tab == 2) {
      Groups g;
      for (int src : {kNodeOP1, kNodeOP2, kNodeOP3, kNodeOP4, kNodeBUS}) {
        std::vector<ParamId> v;
        for (int s = 0; s < kMaxRoutes; ++s)
          if (routeSlotInfo(s).kind == RouteKind::CrossReverse && routeSlotInfo(s).src == src) v.push_back(routeSlotInfo(s).depthParam);
        g.push_back({juce::String(nodeName(src)) + juce::String::fromUTF8(" › WT"), v});
      }
      return g;
    }
    std::vector<ParamId> audio{ParamId::CrossAudioMode, ParamId::CrossAudioAmount};
    if (pv.choice(ParamId::CrossAudioMode) == 3) audio.push_back(ParamId::CrossAmDir);  // direction : mode AM
    return {{juce::String::fromUTF8("Bus (somme des opérateurs)"), {ParamId::CrossBusMode, ParamId::CrossBusW1, ParamId::CrossBusW2, ParamId::CrossBusW3, ParamId::CrossBusW4}},
            {juce::String::fromUTF8("Fréquence WT"), {ParamId::CrossWtFreqMode}},
            {juce::String::fromUTF8("Cross Audio  (post-chaînes)"), audio}};
  }
  static Groups fmGroups(int tab, const ParamValues& pv) {
    if (tab < 4) return opGroups(tab, pv);
    if (tab == 4) {
      Groups g;
      for (int s = 1; s <= 4; ++s) {
        std::vector<ParamId> v;
        for (int d = 1; d <= 4; ++d) v.push_back(routeSlotInfo(routeSlot(s, d)).depthParam);
        g.push_back({"OP" + juce::String(s) + juce::String::fromUTF8(" › (rad)"), v});
      }
      return g;
    }
    return {{"Commun", {ParamId::FmCharMaster, ParamId::FmFbMode}}};
  }
  static Groups envGroups(int tab, const ParamValues&) {
    std::vector<ParamId> v = EnvelopeEditor::paramsOf(tab);
    if (tab == 0) {
      v.push_back(ParamId::AmpVelocity);
      return {{"DAHDSR", v}, {juce::String::fromUTF8("Forme et déclenchement"), {ParamId::AmpCurve, ParamId::AmpTrigger, ParamId::AmpSync}}};
    }
    const int base = idx(v.front());
    const int tr = idx(ParamId::EnvWtTrigger) + (tab - 1) * 2;
    return {{"DAHDSR", v},
            {juce::String::fromUTF8("Forme et déclenchement"),
             {static_cast<ParamId>(base + 6), static_cast<ParamId>(base + 7), static_cast<ParamId>(tr), static_cast<ParamId>(tr + 1)}}};
  }
  static Groups lfoGroups(int tab, const ParamValues& pv) {
    const int b = idx(ParamId::Lfo1Shape) + tab * 6;  // Shape, Rate, Sync, Div, Phase, Retrig
    auto p = [b](int o) { return static_cast<ParamId>(b + o); };
    std::vector<ParamId> time{p(2)};
    time.push_back(pv.flag(p(2)) ? p(3) : p(1));  // Division (sync tempo) ou Rate (libre) : l'autre est conservé
    return {{"Forme", {p(0)}}, {"Temps", time}, {"Phase", {p(4), p(5)}}};
  }
  static Groups rndGroups(int, const ParamValues&) {
    return {{juce::String::fromUTF8("Aléatoire"), {ParamId::RndHoldRate, ParamId::RndSmoothRate}},
            {"Chaos", {ParamId::ChaosRate}},
            {"Suiveurs d'enveloppe", range(ParamId::Fol1Attack, ParamId::Fol3Release)}};
  }
  void syncLfoCurve() {
    const auto pv = ctx_.proc.parameters().values();
    const int k = lfo_.tab();
    const bool custom = pv.choice(static_cast<ParamId>(idx(ParamId::Lfo1Shape) + 6 * k)) == 7;
    lfo_.setBannerVisible(custom);
    const auto curves = ctx_.proc.lfoCurves();
    const auto& c = curves[static_cast<size_t>(k)];
    lfoEd_.setPoints(c.n, c.x.data(), c.y.data());
  }

  TabStrip tabs_;
  ContextColumn wt_, cross_, fm_;
  ZOrderDisplay zDisp_;
  juce::ComboBox bank_, topo_;
  std::vector<std::string> bankIds_;
  std::vector<Route> routes_;
  OpticalDisplay flow_{juce::String::fromUTF8("FLUX DU SIGNAL"), {}};
  juce::String flowText_;
  ChainColumn wtChain_, fmChain_, glChain_;
  ContextColumn env_, lfo_, rnd_;
  EnvelopeEditor envEd_;
  PointCurveEditor lfoEd_;
  ActionCell crossReset_, excite_, capture_;
  int tab_ = 0, capturePending_ = 0, n_ = 0;
};

}  // namespace

std::unique_ptr<SpaceView> makeSoundDesignView(UiContext& c) { return std::make_unique<SoundDesignView>(c); }

}  // namespace sbd::ui
