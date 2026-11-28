// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — outil de comparatif 0.4.1 / R2 (sbd_lab_compare).
// À gauche : composants 0.4.1 de production, NON MODIFIÉS, liés au processeur 0.4.1 (aucun traitement audio : le
// processeur n'est instancié que pour ses paramètres). À droite : composants R2 liés aux MÊMES paramètres par
// l'adaptateur ParamSource (chemin d'intégration proposé pour R2-B2). Les valeurs viennent du registre réel.
#include <iostream>

#include "app/Sheets.h"
#include "r2/R2CyberDisplay.h"
#include "ui/Components.h"
#include "ui/SymbioticLook.h"

namespace {

using namespace sbd;
namespace t = sbd::ui::tok;

// Adaptateur R2 -> paramètre de production (ParamLink 0.4.1 : mêmes gestes hôte, mêmes transactions).
class ParamSource : public r2::ValueSource {
 public:
  ParamSource(SubdestrucktProcessor& p, ParamId id) : link_(p, id, [this] {
                                                       if (onChange) onChange();
                                                     }) {}
  float norm() const override { return link_.norm(); }
  float defaultNorm() const override { return const_cast<ui::ParamLink&>(link_).param().getDefaultValue(); }
  std::optional<float> effectiveNorm() const override { return std::nullopt; }  // moteur arrêté : aucune modulation
  juce::String text(float n) const override {
    // le texte hôte porte déjà l'unité ; les domaines normalisés (lin, pos, bip) n'affichent pas d'unité (nomenclature R2 §6)
    auto s = const_cast<ui::ParamLink&>(link_).param().getText(n, 16);
    for (const char* u : {" lin", " pos", " bip"})
      if (s.endsWith(u)) s = s.dropLastCharacters(4);
    return s;
  }
  bool bipolar() const override { return const_cast<ui::ParamLink&>(link_).param().getNormalisableRange().start < 0.0f; }
  void begin() override { link_.begin(); }
  void during(float n) override { link_.during(link_.fromNorm(n)); }
  void end() override { link_.end(); }
  void complete(float n) override { link_.complete(link_.fromNorm(n)); }

 private:
  ui::ParamLink link_;
};

void setParam(SubdestrucktProcessor& p, ParamId id, float value) {
  for (auto* prm : p.getParameters())
    if (auto* r = dynamic_cast<juce::RangedAudioParameter*>(prm))
      if (r->getParameterID() == juce::String(paramSpec(id).stableId)) r->setValueNotifyingHost(r->convertTo0to1(value));
}

}  // namespace

using namespace sbd;

int main(int argc, char** argv) {
  juce::ScopedJuceInitialiser_GUI gui;
  const juce::File out = juce::File::getCurrentWorkingDirectory().getChildFile(argc > 1 ? juce::String::fromUTF8(argv[1]) : juce::String("compare.png"));
  const float scale = argc > 2 ? juce::String(argv[2]).getFloatValue() : 2.0f;
  sbd::ui::SymbioticLook look;
  juce::LookAndFeel::setDefaultLookAndFeel(&look);
  {
    SubdestrucktProcessor proc;
    setParam(proc, ParamId::WtX, 0.486f);
    setParam(proc, ParamId::WtFine, -32.0f);
    setParam(proc, ParamId::CrossAudioAmount, 0.5f);
    setParam(proc, ParamId::DepthOp2Op1, 3.2f);
    setParam(proc, ParamId::CrossMaster, 0.64f);
    setParam(proc, ParamId::DstWt1D2Freeze, 1.0f);
    setParam(proc, ParamId::DstWt1Arm, 1.0f);
    setParam(proc, ParamId::PerfBalance, -0.2f);
    ui::UiContext ctx{proc, nullptr, RandomOptions{}, {}, {}, {}, {}, ParamId::WtX, {}};
    std::vector<std::unique_ptr<ParamSource>> sources;  // déclarées avant la planche : détruites après ses composants
    lab::Sheet s("Comparatif 0.4.1 / R2", juce::String::fromUTF8("Gauche : composants 0.4.1 de production (non modifiés). Droite : composants R2 du laboratoire. "
                                                                 "Mêmes paramètres réels du registre, liés par ParamLink (adaptateur R2 → ParamLink). Moteur arrêté."),
                 1240, 980);
    std::vector<std::unique_ptr<juce::Component>> legacy;
    const auto src = [&](ParamId id) -> ParamSource& {
      sources.push_back(std::make_unique<ParamSource>(proc, id));
      return *sources.back();
    };
    const auto put = [&](std::unique_ptr<juce::Component> c, juce::Rectangle<int> r) {
      s.addAndMakeVisible(*c);
      c->setBounds(r);
      if (auto* b = dynamic_cast<ui::BoundControl*>(c.get())) b->tick();
      legacy.push_back(std::move(c));
    };
    s.plate({16.0f, 76.0f, 596.0f, 880.0f}, "0.4.1  (production, ARCH-UI-04)", r2::Domain::Neutral, "unchanged");
    s.plate({628.0f, 76.0f, 596.0f, 880.0f}, "R2  (Component Lab, proposal)", r2::Domain::Wt, juce::String::fromUTF8("not integrated"));
    const char* rows[6] = {"NEXUS XL", "NEXUS S / Micro", "PLASMA", "NEURAL CELL", "ACTIONS", "SYNAPTIC TRACK"};
    const int ry[6] = {112, 282, 418, 620, 700, 790};
    for (int k = 0; k < 6; ++k) {
      s.caption(rows[k], {28.0f, static_cast<float>(ry[k]) - 10.0f, 200.0f, 12.0f}, 9.5f, t::textMute, juce::Justification::centredLeft, true);
      s.caption(rows[k], {640.0f, static_cast<float>(ry[k]) - 10.0f, 200.0f, 12.0f}, 9.5f, t::textMute, juce::Justification::centredLeft, true);
    }
    using ui::Density;
    using ui::Form;
    // NEXUS XL
    put(std::make_unique<ui::ContinuousControl>(ctx, ParamId::WtX, Form::Rotary, Density::XL), {40, 112, 132, 150});
    put(std::make_unique<ui::ContinuousControl>(ctx, ParamId::WtFine, Form::Rotary, Density::XL), {190, 112, 132, 150});
    s.add<r2::Nexus>({652, 112, 132, 150}, r2::Nexus::Size::XL, r2::Domain::Wt, "X", "WT X", src(ParamId::WtX));
    s.add<r2::Nexus>({802, 112, 132, 150}, r2::Nexus::Size::XL, r2::Domain::Wt, "FINE", "WT Fine", src(ParamId::WtFine));
    // NEXUS S / Micro
    put(std::make_unique<ui::ContinuousControl>(ctx, ParamId::CrossAudioAmount, Form::Rotary, Density::S), {40, 282, 84, 96});
    put(std::make_unique<ui::ContinuousControl>(ctx, ParamId::DepthOp2Op1, Form::Rotary, Density::Micro), {150, 292, 64, 74});
    s.add<r2::Nexus>({652, 282, 84, 96}, r2::Nexus::Size::S, r2::Domain::Cross, "XAUDIO AMT", "Cross Audio Amount (post-chain)", src(ParamId::CrossAudioAmount));
    s.add<r2::Nexus>({762, 292, 64, 74}, r2::Nexus::Size::Micro, r2::Domain::Fm, "DEPTH", "FM OP2>OP1", src(ParamId::DepthOp2Op1));
    // PLASMA
    put(std::make_unique<ui::ContinuousControl>(ctx, ParamId::CrossMaster, Form::LinearV, Density::S), {40, 418, 48, 150});
    put(std::make_unique<ui::ContinuousControl>(ctx, ParamId::PerfBalance, Form::LinearH, Density::S), {110, 470, 220, 40});
    s.add<r2::PlasmaChannel>({652, 418, 64, 180}, r2::PlasmaChannel::Orientation::Vertical, r2::Domain::Cross, "CROSS LVL", "CROSS Master (CROSS level)",
                             src(ParamId::CrossMaster), juce::StringArray{"0", ".5", "1"});
    s.add<r2::PlasmaChannel>({730, 470, 360, 44}, r2::PlasmaChannel::Orientation::Horizontal, r2::Domain::Cross, "BALANCE  PRE-CHAIN",
                             "WT/FM Balance (audio, pre-chain)", src(ParamId::PerfBalance), juce::StringArray{"WT", "0", "FM"});
    // NEURAL CELL : FREEZE et ARM
    put(std::make_unique<ui::NeuralCell>(ctx, ParamId::DstWt1D2Freeze), {40, 620, 168, 40});
    put(std::make_unique<ui::NeuralCell>(ctx, ParamId::DstWt1Arm), {220, 620, 168, 40});
    auto& fr = s.add<r2::NeuralCell>({652, 620, 168, 40}, r2::NeuralCell::Kind::Toggle, r2::Domain::Neutral, "FREEZE", "DST WT1 D2 Freeze");
    fr.setToggleState(true, juce::dontSendNotification);
    // même état que la cellule 0.4.1 voisine : armé ET autorisé (autorisation initiale de l'instance) = ACTIVE
    s.add<r2::NeuralCell>({832, 620, 168, 40}, r2::NeuralCell::Kind::Arm, r2::Domain::Neutral, "ARM", "DST WT1 Autonomous Armed")
        .setArmState(proc.telemetry().autonomyAuthorized.load() ? r2::NeuralCell::ArmState::Active : r2::NeuralCell::ArmState::Armed);
    // ACTIONS : capture, autorisation, PANIC
    put(std::make_unique<ui::ActionCell>("Capturer gels", t::warn), {40, 700, 168, 40});
    put(std::make_unique<ui::ActionCell>("Autoriser", t::warn), {220, 700, 120, 40});
    put(std::make_unique<ui::ActionCell>("PANIC", t::alert, true), {352, 700, 112, 40});
    auto& cap = s.add<r2::NeuralCell>({652, 700, 168, 40}, r2::NeuralCell::Kind::Action, r2::Domain::Neutral, "CAPTURE", "Capture frozen D2/S2 content");
    cap.setIcon(r2::NeuralCell::Icon::Capture);
    auto& au = s.add<r2::NeuralCell>({832, 700, 140, 40}, r2::NeuralCell::Kind::Action, r2::Domain::Neutral, "AUTHORIZE", "Authorize autonomous states");
    au.setIcon(r2::NeuralCell::Icon::Authorize);
    s.add<r2::NeuralCell>({984, 700, 112, 40}, r2::NeuralCell::Kind::Safety, r2::Domain::Alert, "PANIC", "Panic");
    // SYNAPTIC TRACK
    put(std::make_unique<ui::SynapticTrack>(ctx, ParamId::DstWt1D2FbMode), {40, 790, 168, 46});
    put(std::make_unique<ui::SynapticTrack>(ctx, ParamId::DstWt1Type), {220, 790, 200, 46});
    s.add<r2::SynapticTrack>({652, 790, 168, 46}, r2::SynapticTrack::Mode::Segments, r2::Domain::Neutral, "FB MODE", "DST WT1 D2 Feedback Mode",
                             juce::StringArray{"LEGACY", "REGEN"})
        .setIndex(juce::roundToInt(src(ParamId::DstWt1D2FbMode).norm()));
    s.add<r2::SynapticTrack>({832, 790, 200, 46}, r2::SynapticTrack::Mode::List, r2::Domain::Neutral, "TYPE", "DST WT1 Type",
                             juce::StringArray{"OFF", "N1 FOLD", "D1 CRUSH", "Y1 TRANSIENT", "S1 RING/SHIFT", "N2 FB SHAPER", "D2 GLITCH", "Y2 ENV SHAPER", "S2 SPECTRAL"});
    s.caption(juce::String::fromUTF8("Valeurs réelles du registre fixées avant rendu (WT X 0,486 ; WT Fine −32 ct ; Cross Audio Amount 0,5 ; FM OP2>OP1 3,2 rad ; "
                                     "CROSS Master 0,64 ; Freeze ON ; ARM ON ; Balance −0,2). États d'autonomie : affichage statique, moteur arrêté."),
              {28.0f, 900.0f, 1180.0f, 40.0f}, 10.0f, t::textDim, juce::Justification::topLeft);
    for (auto& c : legacy) c->repaint();
    const auto img = s.createComponentSnapshot(s.getLocalBounds(), true, scale);
    const bool ok = lab::writePng(img, out);
    std::cout << "compare " << out.getFullPathName().toStdString() << " " << img.getWidth() << "x" << img.getHeight() << (ok ? " OK" : " WRITE FAILED") << std::endl;
    legacy.clear();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    return ok ? 0 : 1;
  }
}
