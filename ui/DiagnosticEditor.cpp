#include "DiagnosticEditor.h"

#include "BuildInfo.h"

namespace sbd {

namespace {
juce::Font mono(float h) { return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), h, juce::Font::plain)); }
const juce::Colour kBg(0xff121416), kPanel(0xff1b1e21), kText(0xffe6e4df), kDim(0xff9aa0a6), kAccent(0xffff6b1a);

juce::String fmt(double v, int decimals) { return juce::String(v, decimals); }
}  // namespace

DiagnosticEditor::DiagnosticEditor(SubdestrucktProcessor& p) : juce::AudioProcessorEditor(p), proc_(p) {
  title_.setText(juce::String("SUBDESTRUCKT  //  diagnostic ARCH-G3  //  build ") + SUBDESTRUCKT_BUILD_ID + "  (" +
                     juce::String(SUBDESTRUCKT_COMMIT).substring(0, 12) + ")",
                 juce::dontSendNotification);
  title_.setFont(mono(14.0f));
  title_.setColour(juce::Label::textColourId, kAccent);
  addAndMakeVisible(title_);

  for (int i = 0; i < kNumParams; ++i) {
    auto& row = params_[static_cast<size_t>(i)];
    const auto& spec = paramSpec(i);
    row.label.setText(juce::String(spec.name) + " (" + spec.unit + ")", juce::dontSendNotification);
    row.label.setFont(mono(12.0f));
    row.label.setColour(juce::Label::textColourId, kText);
    row.label.setTooltip(spec.stableId);
    row.slider.setSliderStyle(juce::Slider::LinearHorizontal);
    row.slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 18);
    row.slider.setColour(juce::Slider::thumbColourId, kAccent);
    row.attachment = std::make_unique<juce::SliderParameterAttachment>(*proc_.parameters().param(i), row.slider, nullptr);
    addAndMakeVisible(row.label);
    addAndMakeVisible(row.slider);
  }

  for (int s = 0; s < kMaxRoutes; ++s) {
    auto& r = routes_[static_cast<size_t>(s)];
    const auto& info = routeSlotInfo(s);
    const char* kind = info.kind == RouteKind::Internal ? "FM" : (info.kind == RouteKind::CrossForward ? "CROSS WT>FM" : "CROSS FM>WT");
    r.name.setText(juce::String(nodeName(info.src)) + " > " + nodeName(info.dst) + "  [" + kind + "]", juce::dontSendNotification);
    for (auto* l : {&r.name, &r.status, &r.effective}) {
      l->setFont(mono(12.0f));
      l->setColour(juce::Label::textColourId, kText);
      addAndMakeVisible(*l);
    }
    r.add.onClick = [this, s] { addRoute(s); };
    r.remove.onClick = [this, s] { removeRoute(s); };
    addAndMakeVisible(r.add);
    addAndMakeVisible(r.remove);
  }

  panic_.onClick = [this] { proc_.requestPanic(); };
  init_.onClick = [this] {
    proc_.loadDefaultPreset();
    refreshTopology();
  };
  addAndMakeVisible(panic_);
  addAndMakeVisible(init_);

  for (auto* l : {&runtime_, &telemetry_, &message_}) {
    l->setFont(mono(12.0f));
    l->setColour(juce::Label::textColourId, kText);
    l->setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(*l);
  }
  message_.setColour(juce::Label::textColourId, kDim);

  setResizable(true, true);
  setResizeLimits(980, 680, 2000, 1400);
  setSize(1080, 720);
  refreshTopology();
  startTimerHz(15);
}

DiagnosticEditor::~DiagnosticEditor() { stopTimer(); }

void DiagnosticEditor::paint(juce::Graphics& g) {
  g.fillAll(kBg);
  auto r = getLocalBounds().reduced(8);
  r.removeFromTop(28);
  auto left = r.removeFromLeft(r.getWidth() / 2 - 4);
  g.setColour(kPanel);
  g.fillRect(left);
  g.fillRect(r.withTrimmedLeft(8));
}

void DiagnosticEditor::resized() {
  auto r = getLocalBounds().reduced(8);
  title_.setBounds(r.removeFromTop(24));
  r.removeFromTop(4);
  auto left = r.removeFromLeft(r.getWidth() / 2 - 4).reduced(6);
  auto right = r.withTrimmedLeft(8).reduced(6);

  const int rowH = juce::jmax(22, left.getHeight() / kNumParams);
  for (auto& row : params_) {
    auto line = left.removeFromTop(rowH);
    row.label.setBounds(line.removeFromLeft(170));
    row.slider.setBounds(line);
  }

  runtime_.setBounds(right.removeFromTop(92));
  right.removeFromTop(4);
  for (auto& row : routes_) {
    auto line = right.removeFromTop(26);
    row.name.setBounds(line.removeFromLeft(190));
    row.remove.setBounds(line.removeFromRight(28).reduced(2));
    row.add.setBounds(line.removeFromRight(28).reduced(2));
    row.effective.setBounds(line.removeFromRight(150));
    row.status.setBounds(line);
  }
  right.removeFromTop(6);
  auto buttons = right.removeFromTop(28);
  panic_.setBounds(buttons.removeFromLeft(110).reduced(2));
  init_.setBounds(buttons.removeFromLeft(130).reduced(2));
  right.removeFromTop(4);
  message_.setBounds(right.removeFromTop(36));
  telemetry_.setBounds(right);
}

void DiagnosticEditor::refreshTopology() {
  const auto routes = proc_.currentRoutes();
  for (int s = 0; s < kMaxRoutes; ++s) {
    const auto& info = routeSlotInfo(s);
    auto& row = routes_[static_cast<size_t>(s)];
    const Route* found = nullptr;
    for (const auto& r : routes)
      if (r.src == info.src && r.dst == info.dst) found = &r;
    if (found) {
      row.status.setText("id " + juce::String(found->id) + "  classe " + juce::String::charToString(classLetter(found->cls)),
                         juce::dontSendNotification);
      row.status.setColour(juce::Label::textColourId, found->cls == CausalClass::Delayed ? kAccent : kText);
    } else {
      row.status.setText("absente", juce::dontSendNotification);
      row.status.setColour(juce::Label::textColourId, kDim);
    }
    row.add.setEnabled(found == nullptr);
    row.remove.setEnabled(found != nullptr);
  }
  message_.setText(juce::String(proc_.lastMessage()) + "\nnextRouteId " + juce::String(proc_.currentNextRouteId()),
                   juce::dontSendNotification);
}

void DiagnosticEditor::addRoute(int slot) {
  const auto& info = routeSlotInfo(slot);
  proc_.applyTopology({RouteTable::Op::add(info.src, info.dst)});
  refreshTopology();
}

void DiagnosticEditor::removeRoute(int slot) {
  const auto& info = routeSlotInfo(slot);
  for (const auto& r : proc_.currentRoutes())
    if (r.src == info.src && r.dst == info.dst) proc_.applyTopology({RouteTable::Op::remove(r.id)});
  refreshTopology();
}

void DiagnosticEditor::timerCallback() {
  auto& t = proc_.telemetry();
  const double fs = t.hostRate.load(), fsE = t.engineRate.load(), D = t.delaySamples.load();
  const int K = t.oversampling.load(), lat = t.latencySamples.load();
  juce::String rt;
  rt << "Fs hote " << fmt(fs, 0) << " Hz   K " << K << "   Fs moteur " << fmt(fsE, 0) << " Hz"
     << (t.supported.load() ? "" : "   [FREQUENCE NON SUPPORTEE : silence]") << "\n";
  rt << "Retard T2 (aretes D) : D = " << fmt(D, 4) << " ech. moteur = " << fmt(fsE > 0 ? D / fsE * 1e6 : 0.0, 3) << " us   lecteur "
     << (t.delayInteger.load() ? juce::String("entier exact") : "Thiran1 a=" + fmt(t.thiranCoefficient.load(), 5)) << "\n";
  rt << "Latence declaree (PDC) : " << lat << " ech. (" << fmt(fs > 0 ? lat / fs * 1000.0 : 0.0, 3) << " ms)\n";
  rt << "Transition : " << (t.transitionPhase.load() == 0 ? "repos" : t.transitionPhase.load() == 1 ? "fondu de sortie" : "fondu d'entree")
     << "   topologie active #" << juce::String(static_cast<juce::int64>(t.activeSerial.load()));
  runtime_.setText(rt, juce::dontSendNotification);

  for (int s = 0; s < kMaxRoutes; ++s)
    routes_[static_cast<size_t>(s)].effective.setText(
        "Eff " + fmt(t.effectiveDepth[static_cast<size_t>(s)].load(), 3) + " rad  g " + fmt(t.edgeGain[static_cast<size_t>(s)].load(), 2),
        juce::dontSendNotification);

  auto db = [](float v) { return v > 0.0f ? fmt(20.0 * std::log10(v), 1) + " dBFS" : juce::String("-inf"); };
  juce::String te;
  te << "Voix : note " << t.currentNote.load() << "   gate " << (t.gate.load() ? "on" : "off") << "   enveloppe "
     << fmt(t.envelope.load(), 3) << "\n";
  te << "Crete L " << db(t.peakL.load()) << "   R " << db(t.peakR.load()) << "\n";
  te << "Surete : reinit. non finies " << (int) t.nonFiniteResets.load() << "   garde sortie " << (int) t.outputClampHits.load()
     << "   borne PM " << (int) t.pmClampHits.load() << "   evts perdus " << (int) t.droppedEvents.load() << "   Panic "
     << (int) t.panics.load();
  telemetry_.setText(te, juce::dontSendNotification);
  refreshTopology();  // reflète aussi un état chargé par l'hôte
}

}  // namespace sbd
