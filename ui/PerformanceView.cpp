// SUBDESTRUCKT — UI — espace PERFORMANCE (ARCH-UI-04) et SYMBIOTIC CORE (périmètre ARCH-UI-04 uniquement : commande
// directe X = cross.fwd, Y = cross.rev, Z = cross.master ; rendu statique provisoire identifié ; aucun renderer 3D,
// aucun morphing, aucun mapping multiparamétrique persistant, aucun nouveau paramètre — ARCH-UI-05).
#include "core/StateCodec.h"
#include "Views.h"

namespace sbd::ui {

namespace {

// ================================================================ SYMBIOTIC CORE (commande directe)
class SymbioticCore : public juce::Component, public juce::SettableTooltipClient, public CorePad {
 public:
  explicit SymbioticCore(UiContext& c)
      : ctx_(c),
        fwd_(c.proc, ParamId::CrossFwd, [this] { repaint(); }),
        rev_(c.proc, ParamId::CrossRev, [this] { repaint(); }),
        z_(c, ParamId::CrossMaster, Form::LinearV, Density::S),
        nx_(c, ParamId::CrossFwd, Form::Rotary, Density::S),
        ny_(c, ParamId::CrossRev, Form::Rotary, Density::S),
        nz_(c, ParamId::CrossMaster, Form::Rotary, Density::S) {
    z_.setCaption("Z  Master");
    nx_.setCaption("X  WT>FM");
    ny_.setCaption("Y  FM>WT");
    nz_.setCaption("Z  Master");
    for (auto* k : {&z_, &nx_, &ny_, &nz_}) addAndMakeVisible(*k);
    setTitle("Symbiotic Core : X = cross.fwd, Y = cross.rev");
    setDescription(juce::String::fromUTF8("Commande directe X/Y du CROSS ; Z = CROSS Master (canal vertical et saisie numérique)"));
    setWantsKeyboardFocus(true);
    setTooltip(juce::String::fromUTF8("SYMBIOTIC CORE — commande directe : X = CROSS WT>FM (cross.fwd), Y = CROSS FM>WT (cross.rev), Z = CROSS Master.\n"
               "Glisser : X et Y en un geste ; Maj : fin ; double-clic : défauts ; flèches : pas de 1 %.\n"
               "Rendu statique provisoire, illustratif : la forme ne mesure pas l'activité des routes."));
  }
  float shownX() const override { return fwd_.base(); }
  float shownY() const override { return rev_.base(); }
  // geste utilisateur simulé (tests) : même chemin que le glisser
  void userMove(float x, float y) override {
    fwd_.begin();
    rev_.begin();
    fwd_.during(juce::jlimit(0.0f, 1.0f, x));
    rev_.during(juce::jlimit(0.0f, 1.0f, y));
    fwd_.end();
    rev_.end();
  }
  void tick() {
    auto& t = ctx_.proc.telemetry();
    const auto ix = static_cast<size_t>(idx(ParamId::CrossFwd)), iy = static_cast<size_t>(idx(ParamId::CrossRev));
    const bool m = t.modulated[ix].load() || t.modulated[iy].load();
    const float ex = m ? t.effective[ix].load() : -1.0f, ey = m ? t.effective[iy].load() : -1.0f;
    if (ex != effX_ || ey != effY_) {
      effX_ = ex;
      effY_ = ey;
      repaint();
    }
  }
  void resized() override {
    auto r = getLocalBounds();
    auto num = r.removeFromBottom(tok::nexusS.h + 4);
    const int w = std::min(tok::nexusS.w, num.getWidth() / 3);
    const int x0 = num.getCentreX() - (3 * w) / 2;
    nx_.setBounds(x0, num.getY(), w, tok::nexusS.h);
    ny_.setBounds(x0 + w, num.getY(), w, tok::nexusS.h);
    nz_.setBounds(x0 + 2 * w, num.getY(), w, tok::nexusS.h);
    r.removeFromTop(30);
    z_.setBounds(r.removeFromRight(tok::plasmaV.w + 26).withTrimmedBottom(4));
    pad_ = r.reduced(6).toFloat();
    const float s = std::min(pad_.getWidth(), pad_.getHeight());
    pad_ = pad_.withSizeKeepingCentre(s, s);
  }
  void paint(juce::Graphics& g) override {
    // en-tête : identification explicite du rendu provisoire
    auto head = getLocalBounds().removeFromTop(28).toFloat();
    g.setFont(uiFont(tok::fontHead, true));
    g.setColour(tok::text);
    g.drawText("SYMBIOTIC CORE", head.removeFromTop(16.0f), juce::Justification::centredLeft, false);
    g.setFont(uiFont(10.0f));
    g.setColour(tok::warn);
    g.drawText(juce::String::fromUTF8("RENDU STATIQUE PROVISOIRE · illustratif · moteur 3D : ARCH-UI-05"), head, juce::Justification::centredLeft, false);
    // verre optique
    g.setColour(tok::glass);
    g.fillRoundedRectangle(pad_, tok::radius);
    g.setColour(tok::glassEdge);
    g.drawRoundedRectangle(pad_.reduced(0.5f), tok::radius, 1.2f);
    const float x = fwd_.base(), y = rev_.base();
    const float z = ctx_.proc.parameters().base(idx(ParamId::CrossMaster));
    const auto p = toScreen(x, y);
    // membranes statiques : anneaux déformés déterministes (aucune animation, aucune mesure simulée)
    const auto c = pad_.getCentre();
    for (int k = 7; k >= 1; --k) {
      const float rr = pad_.getWidth() * 0.06f * static_cast<float>(k) * (0.55f + 0.45f * z);
      juce::Path ring;
      for (int a = 0; a <= 96; ++a) {
        const float th = juce::MathConstants<float>::twoPi * static_cast<float>(a) / 96.0f;
        const float wob = 1.0f + 0.07f * std::sin(3.0f * th + static_cast<float>(k)) + 0.04f * std::sin(5.0f * th - 0.7f * static_cast<float>(k));
        const float px = c.x + (p.x - c.x) * (1.0f - static_cast<float>(k) / 8.0f) + rr * wob * std::cos(th);
        const float py = c.y + (p.y - c.y) * (1.0f - static_cast<float>(k) / 8.0f) + rr * wob * std::sin(th);
        a == 0 ? ring.startNewSubPath(px, py) : ring.lineTo(px, py);
      }
      ring.closeSubPath();
      const auto col = tok::wt.interpolatedWith(tok::fm, static_cast<float>(k) / 8.0f).interpolatedWith(tok::cross, 0.35f);
      g.setColour(col.withAlpha(0.05f + 0.05f * z));
      g.fillPath(ring);
      g.setColour(col.withAlpha(0.22f + 0.25f * z));
      g.strokePath(ring, juce::PathStrokeType(1.0f));
    }
    // axes et graduations
    g.setColour(tok::glassEdge);
    for (int k = 1; k < 4; ++k) {
      const float gx = pad_.getX() + pad_.getWidth() * static_cast<float>(k) / 4.0f;
      const float gy = pad_.getY() + pad_.getHeight() * static_cast<float>(k) / 4.0f;
      g.drawVerticalLine(static_cast<int>(gx), pad_.getY() + 4.0f, pad_.getBottom() - 4.0f);
      g.drawHorizontalLine(static_cast<int>(gy), pad_.getX() + 4.0f, pad_.getRight() - 4.0f);
    }
    g.setFont(numFont(9.5f));
    g.setColour(tok::textDim);
    g.drawText("X  cross.fwd  WT>FM", pad_.reduced(8.0f).removeFromBottom(14.0f), juce::Justification::centredRight, false);
    g.drawText("Y  cross.rev  FM>WT", pad_.reduced(8.0f).removeFromTop(14.0f), juce::Justification::centredLeft, false);
    // Effective (télémétrie réelle agrégée) : distinct de la Base
    if (effX_ >= 0.0f) {
      const auto e = toScreen(effX_, effY_);
      g.setColour(tok::mod);
      g.drawEllipse(e.x - 7.0f, e.y - 7.0f, 14.0f, 14.0f, 1.5f);
      g.drawLine(p.x, p.y, e.x, e.y, 1.0f);
    }
    // position Base
    g.setColour(tok::text);
    g.drawLine(p.x - 10.0f, p.y, p.x + 10.0f, p.y, 1.4f);
    g.drawLine(p.x, p.y - 10.0f, p.x, p.y + 10.0f, 1.4f);
    g.fillEllipse(p.x - 3.5f, p.y - 3.5f, 7.0f, 7.0f);
    if (hasKeyboardFocus(false)) {
      g.setColour(tok::focus);
      g.drawRoundedRectangle(pad_.expanded(2.0f), tok::radius, 1.0f);
    }
    if (z <= 0.0f) {
      g.setColour(tok::textDim);
      g.setFont(uiFont(tok::fontLabel));
      g.drawText(juce::String::fromUTF8("Master = 0 : contributions continues annulées (sync, Cross Audio et queues non concernés)"),
                 pad_.reduced(10.0f).removeFromBottom(34.0f), juce::Justification::centredLeft, false);
    }
  }
  void mouseDown(const juce::MouseEvent& e) override {
    if (!pad_.contains(e.position)) return;
    if (ctx_.select) ctx_.select(ParamId::CrossFwd);
    dragging_ = true;
    start_ = e.position;
    sx_ = fwd_.base();
    sy_ = rev_.base();
    fwd_.begin();
    rev_.begin();
    if (!e.mods.isShiftDown()) {  // clic = position absolue
      const auto v = fromScreen(e.position);
      sx_ = v.x;
      sy_ = v.y;
      fwd_.during(sx_);
      rev_.during(sy_);
    }
  }
  void mouseDrag(const juce::MouseEvent& e) override {
    if (!dragging_) return;
    const float f = e.mods.isShiftDown() ? 0.1f : 1.0f;
    const float nx = juce::jlimit(0.0f, 1.0f, sx_ + (e.position.x - start_.x) / pad_.getWidth() * f);
    const float ny = juce::jlimit(0.0f, 1.0f, sy_ - (e.position.y - start_.y) / pad_.getHeight() * f);
    fwd_.during(nx);
    rev_.during(ny);
  }
  void mouseUp(const juce::MouseEvent&) override {
    if (!dragging_) return;
    dragging_ = false;
    fwd_.end();
    rev_.end();
  }
  void mouseDoubleClick(const juce::MouseEvent& e) override {
    if (!pad_.contains(e.position)) return;
    userMove(paramSpec(ParamId::CrossFwd).defaultValue, paramSpec(ParamId::CrossRev).defaultValue);
  }
  bool keyPressed(const juce::KeyPress& k) override {
    const float s = k.getModifiers().isShiftDown() ? 0.001f : 0.01f;
    float dx = 0.0f, dy = 0.0f;
    if (k.getKeyCode() == juce::KeyPress::leftKey) dx = -s;
    else if (k.getKeyCode() == juce::KeyPress::rightKey) dx = s;
    else if (k.getKeyCode() == juce::KeyPress::upKey) dy = s;
    else if (k.getKeyCode() == juce::KeyPress::downKey) dy = -s;
    else return false;
    if (dx != 0.0f) fwd_.complete(juce::jlimit(0.0f, 1.0f, fwd_.base() + dx));
    if (dy != 0.0f) rev_.complete(juce::jlimit(0.0f, 1.0f, rev_.base() + dy));
    return true;
  }

 private:
  juce::Point<float> toScreen(float x, float y) const { return {pad_.getX() + x * pad_.getWidth(), pad_.getBottom() - y * pad_.getHeight()}; }
  juce::Point<float> fromScreen(juce::Point<float> p) const {
    return {juce::jlimit(0.0f, 1.0f, (p.x - pad_.getX()) / pad_.getWidth()), juce::jlimit(0.0f, 1.0f, (pad_.getBottom() - p.y) / pad_.getHeight())};
  }
  UiContext& ctx_;
  ParamLink fwd_, rev_;
  ContinuousControl z_, nx_, ny_, nz_;
  juce::Rectangle<float> pad_;
  bool dragging_ = false;
  juce::Point<float> start_;
  float sx_ = 0.0f, sy_ = 0.0f, effX_ = -1.0f, effY_ = -1.0f;
};

// ================================================================ PERFORMANCE
class PerformanceView : public SpaceView {
 public:
  explicit PerformanceView(UiContext& c)
      : SpaceView(c),
        wtPlate_("WAVETABLE  //  X Y Z", tok::wt),
        fmPlate_("FM  //  commandes contextuelles", tok::fm),
        corePlate_("CROSS", tok::cross),
        monitorPlate_("MONITEUR", tok::textDim),
        autoPlate_(juce::String::fromUTF8("ÉTATS AUTONOMES"), tok::warn),
        macroPlate_("MACROS  M1 - M4", tok::mod),
        balPlate_(juce::String::fromUTF8("BALANCE  //  pré-chaînes"), tok::cross),
        outPlate_("SORTIE", tok::text),
        wx_(c, ParamId::WtX, Form::Rotary, Density::XL),
        wy_(c, ParamId::WtY, Form::Rotary, Density::XL),
        wz_(c, ParamId::WtZ, Form::Rotary, Density::XL),
        core_(c),
        monitor_(c.proc),
        bal_(c, ParamId::PerfBalance, Form::LinearH, Density::S),
        out_(c, ParamId::OutGain, Form::LinearV, Density::S),
        authorize_("Autoriser", tok::warn) {
    wx_.setCaption("X  position");
    wy_.setCaption("Y  position");
    wz_.setCaption("Z  Master");
    for (int k = 0; k < 4; ++k) {
      macro_[k] = std::make_unique<ContinuousControl>(c, static_cast<ParamId>(idx(ParamId::Macro1) + k), Form::Rotary, Density::S);
      macro_[k]->setCaption("M" + juce::String(k + 1));
      macroPlate_.addAndMakeVisible(*macro_[k]);
      names_[k].setEditable(false, true);
      names_[k].setFont(uiFont(tok::fontLabel, true));
      names_[k].setJustificationType(juce::Justification::centred);
      names_[k].setColour(juce::Label::textColourId, tok::mod);
      names_[k].setColour(juce::Label::backgroundColourId, tok::carbon);
      names_[k].setTooltip(juce::String::fromUTF8("Nom de la macro (double-clic pour renommer ; sauvegardé avec le preset)"));
      names_[k].setTitle("Nom de la macro " + juce::String(k + 1));
      names_[k].onTextChange = [this, k] { ctx_.proc.setMacroName(k, names_[k].getText().toStdString()); };
      macroPlate_.addAndMakeVisible(names_[k]);
    }
    bal_.setCaption(juce::String::fromUTF8("WT ‹ › FM"));
    out_.setCaption("Output Gain");
    out_.setMeter([this] {
      auto& t = ctx_.proc.telemetry();
      const float pk = std::max(t.peakL.load(), t.peakR.load());
      return pk > 1e-6f ? 20.0f * std::log10(pk) : -120.0f;
    });
    wtPlate_.addAndMakeVisible(wx_);
    wtPlate_.addAndMakeVisible(wy_);
    wtPlate_.addAndMakeVisible(wz_);
    corePlate_.addAndMakeVisible(core_);
    monitorPlate_.addAndMakeVisible(monitor_);
    balPlate_.addAndMakeVisible(bal_);
    outPlate_.addAndMakeVisible(out_);
    for (auto* l : {&fmInfo_, &autoInfo_, &outInfo_, &balInfo_}) {
      l->setFont(numFont(10.0f));
      l->setColour(juce::Label::textColourId, tok::textDim);
      l->setJustificationType(juce::Justification::topLeft);
    }
    fmPlate_.addAndMakeVisible(fmInfo_);
    autoPlate_.addAndMakeVisible(autoInfo_);
    outPlate_.addAndMakeVisible(outInfo_);
    balPlate_.addAndMakeVisible(balInfo_);
    balInfo_.setText(juce::String::fromUTF8("Balance audio post-Amp, avant les chaînes ;\nCross Audio (post-chaînes) : Sound Design."),
                     juce::dontSendNotification);
    authorize_.onClick = [this] { ctx_.proc.authorizeAutonomy(); };
    authorize_.setIcon(ActionCell::Icon::Authorize);  // icône R1 (LAB-A02)
    authorize_.setTooltip(juce::String::fromUTF8("États autonomes suspendus : autoriser leur exécution dans cette instance (action explicite)"));
    autoPlate_.addChildComponent(authorize_);
    for (auto* p : std::initializer_list<juce::Component*>{&wtPlate_, &fmPlate_, &corePlate_, &monitorPlate_, &autoPlate_, &macroPlate_, &balPlate_, &outPlate_})
      addAndMakeVisible(*p);
    refreshState();
  }
  int numTabs() const override { return 1; }
  juce::String tabName(int) const override { return "PERFORMANCE"; }
  void showTab(int) override {}
  int currentTab() const override { return 0; }
  juce::Point<int> minimumSize() const override { return {1080, 812}; }  // colonne WT/FM : deux NEXUS XL + FM contextuel

  // Deux commandes FM contextuelles : profondeur de la route interne principale vers une porteuse (« FM INDEX ») et
  // auto-boucle de cette porteuse (« FM FEEDBACK »). Paramètres existants uniquement (arbitrage UI 1C) ; libellés
  // définitifs à arbitrer (ARCH-UI-03 §8).
  void refreshState() override {
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
        fmIndex_ = std::make_unique<ContinuousControl>(ctx_, index, Form::Rotary, Density::XL);
        fmIndex_->setCaption("FM INDEX");
        fmPlate_.addAndMakeVisible(*fmIndex_);
      }
      if (feedback != ParamId::Count) {
        fmFb_ = std::make_unique<ContinuousControl>(ctx_, feedback, Form::Rotary, Density::XL);
        fmFb_->setCaption("FM FEEDBACK");
        fmPlate_.addAndMakeVisible(*fmFb_);
      }
      juce::String t = "INDEX    -> ";
      t << (index != ParamId::Count ? juce::String(paramSpec(index).stableId) : juce::String::fromUTF8("aucune route FM interne (Engineering › FM Network)"));
      t << "\nFEEDBACK -> " << (feedback != ParamId::Count ? juce::String(paramSpec(feedback).stableId) : juce::String::fromUTF8("pas d'auto-boucle sur la porteuse"));
      fmInfo_.setText(t, juce::dontSendNotification);
      resized();
    }
    authorize_.setVisible(ctx_.proc.autonomySuspended());  // état courant dès la construction / restauration
    const auto names = ctx_.proc.macroNames();
    for (int k = 0; k < 4; ++k)
      if (!names_[k].isBeingEdited()) names_[k].setText(names[static_cast<size_t>(k)], juce::dontSendNotification);
  }

  void tick() override {
    core_.tick();
    monitor_.refresh();
    if (++n_ % 15 == 0) refreshState();
    const auto pv = ctx_.proc.parameters().values();
    auto& t = ctx_.proc.telemetry();
    static const struct {
      ParamId id;
      const char* name;
    } arms[9] = {{ParamId::FltWtArm, "FLT WT"}, {ParamId::FltFmArm, "FLT FM"}, {ParamId::FltGlArm, "FLT GL"},
                 {ParamId::DstWt1Arm, "DST WT1"}, {ParamId::DstWt2Arm, "DST WT2"}, {ParamId::DstFm1Arm, "DST FM1"},
                 {ParamId::DstFm2Arm, "DST FM2"}, {ParamId::DstGl1Arm, "DST GL1"}, {ParamId::DstGl2Arm, "DST GL2"}};
    juce::String armed;
    int na = 0;
    for (const auto& a : arms)
      if (pv.flag(a.id)) armed << (na++ ? ", " : "") << a.name;
    const bool auth = t.autonomyAuthorized.load();
    juce::String s;
    s << "Drone        " << (ctx_.proc.drone() ? (auth ? "ACTIF" : juce::String::fromUTF8("CONFIGURÉ · SUSPENDU")) : "OFF") << "\n";
    s << juce::String::fromUTF8("Armements    ") << na << (na ? "  (" + armed + ")" : juce::String()) << "\n";
    s << juce::String::fromUTF8("Exécution    ")
      << (auth ? juce::String::fromUTF8("autorisée") : (t.autonomyLatched.load() ? juce::String::fromUTF8("révoquée (Panic)") : juce::String("suspendue")))
      << "  (source " << authSourceName(static_cast<AuthSource>(t.autonomySource.load())) << ")\n";
    static const char* tn[] = {"jamais observé", "ARRÊT", "LECTURE"};
    s << "Transport    " << juce::String::fromUTF8(tn[juce::jlimit(0, 2, t.transportState.load())]) << (t.offline.load() ? juce::String::fromUTF8("  hors temps réel") : "");
    autoInfo_.setText(s, juce::dontSendNotification);
    autoInfo_.setColour(juce::Label::textColourId, (ctx_.proc.drone() || na) ? tok::text : tok::textDim);
    authorize_.setVisible(ctx_.proc.autonomySuspended());
    const float l = t.peakL.load(), r = t.peakR.load();
    auto db = [](float v) { return v > 1e-6f ? juce::String(20.0f * std::log10(v), 1) : juce::String("-inf"); };
    juce::String o;
    o << juce::String::fromUTF8("crête L ") << db(l) << "  R " << db(r) << " dBFS\n";
    o << "ELP " << static_cast<int>(t.elpHits.load()) << "  garde " << static_cast<int>(t.outputClampHits.load()) << "  lim. " << static_cast<int>(t.limiterHits.load());
    outInfo_.setText(o, juce::dontSendNotification);
    outInfo_.setColour(juce::Label::textColourId, (std::max(l, r) > 1.0f || t.elpHits.load() > 0) ? tok::alert : tok::textDim);
  }

  void resized() override {
    auto r = getLocalBounds().reduced(tok::gap);
    auto strip = r.removeFromBottom(196);
    r.removeFromBottom(tok::gap);
    // colonne gauche : WT XYZ + FM contextuel
    auto left = r.removeFromLeft(300);
    r.removeFromLeft(tok::gap);
    auto right = r.removeFromRight(330);
    r.removeFromRight(tok::gap);
    wtPlate_.setBounds(left.removeFromTop(tok::sectionHead + 2 * tok::nexusXL.h + 2 * tok::pad));
    {
      auto c = wtPlate_.content();
      auto row = c.removeFromTop(tok::nexusXL.h);
      wx_.setBounds(row.removeFromLeft(c.getWidth() / 2).withSizeKeepingCentre(tok::nexusXL.w, tok::nexusXL.h));
      wy_.setBounds(row.withSizeKeepingCentre(tok::nexusXL.w, tok::nexusXL.h));
      wz_.setBounds(c.removeFromTop(tok::nexusXL.h).withSizeKeepingCentre(tok::nexusXL.w, tok::nexusXL.h));
    }
    left.removeFromTop(tok::gap);
    fmPlate_.setBounds(left);
    {
      auto c = fmPlate_.content();
      auto row = c.removeFromTop(tok::nexusXL.h);
      if (fmIndex_) fmIndex_->setBounds(row.removeFromLeft(c.getWidth() / 2).withSizeKeepingCentre(tok::nexusXL.w, tok::nexusXL.h));
      else row.removeFromLeft(c.getWidth() / 2);
      if (fmFb_) fmFb_->setBounds(row.withSizeKeepingCentre(tok::nexusXL.w, tok::nexusXL.h));
      fmInfo_.setBounds(c.withTrimmedTop(4));
    }
    corePlate_.setBounds(r);
    core_.setBounds(corePlate_.content());
    monitorPlate_.setBounds(right.removeFromTop(right.getHeight() * 58 / 100));
    monitor_.setBounds(monitorPlate_.content());
    right.removeFromTop(tok::gap);
    autoPlate_.setBounds(right);
    {
      auto c = autoPlate_.content();
      authorize_.setBounds(c.removeFromBottom(30).removeFromLeft(130));
      autoInfo_.setBounds(c);
    }
    // bandeau : macros, balance, sortie
    macroPlate_.setBounds(strip.removeFromLeft(4 * (tok::nexusS.w + 26) + 2 * tok::pad));
    {
      auto c = macroPlate_.content();
      const int w = c.getWidth() / 4;
      for (int k = 0; k < 4; ++k) {
        auto cell = c.removeFromLeft(w);
        names_[k].setBounds(cell.removeFromBottom(22).reduced(4, 1));
        macro_[k]->setBounds(cell.withSizeKeepingCentre(tok::nexusS.w + 16, std::min(cell.getHeight(), tok::nexusS.h + 30)));
      }
    }
    strip.removeFromLeft(tok::gap);
    outPlate_.setBounds(strip.removeFromRight(250));
    {
      auto c = outPlate_.content();
      out_.setBounds(c.removeFromLeft(96));
      outInfo_.setBounds(c.reduced(4, 0));
    }
    strip.removeFromRight(tok::gap);
    balPlate_.setBounds(strip);
    {
      auto c = balPlate_.content();
      bal_.setBounds(c.removeFromTop(48));
      balInfo_.setBounds(c.withTrimmedTop(6));
    }
  }

 private:
  Plate wtPlate_, fmPlate_, corePlate_, monitorPlate_, autoPlate_, macroPlate_, balPlate_, outPlate_;
  ContinuousControl wx_, wy_, wz_;
  std::unique_ptr<ContinuousControl> fmIndex_, fmFb_, macro_[4];
  SymbioticCore core_;
  MonitorPanel monitor_;
  ContinuousControl bal_, out_;
  juce::Label names_[4], fmInfo_, autoInfo_, outInfo_, balInfo_;
  ActionCell authorize_;
  ParamId idxTarget_ = ParamId::Count, fbTarget_ = ParamId::Count;
  int n_ = 0;
};

}  // namespace

std::unique_ptr<SpaceView> makePerformanceView(UiContext& c) { return std::make_unique<PerformanceView>(c); }

}  // namespace sbd::ui
