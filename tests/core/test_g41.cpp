// SUBDESTRUCKT — tests ARCH-G4.1 (BAL-01, ENV-02, HR-01/02, MT-01, FOL-01). Seuils : tracker §3bis (avant mesure).
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include "G4Helper.h"
#include "TestKit.h"
#include "dsp/Destruction.h"

using namespace sbd;
using namespace g4;
using Op = RouteTable::Op;

namespace {

void testBalance() {
  auto rb = [](float bal, float wtL, float opL) {
    G4Scenario sc;
    sc.samples = 24000;
    sc.state = defaultState();  // topologie CROSS active : la modulation doit rester identique
    sc.state.params[ParamId::PerfBalance] = bal;
    sc.state.params[ParamId::WtLevel] = wtL;
    sc.state.params[ParamId::Op1Level] = opL;
    sc.events = {MidiEvent::noteOn(0, 50, 100)};
    return renderG4(sc).L;
  };
  const auto ref = rb(0.0f, 0.5f, 0.5f);
  const auto wtOnly = rb(0.0f, 0.5f, 0.0f), fmOnly = rb(0.0f, 0.0f, 0.5f);
  const auto left = rb(-1.0f, 0.5f, 0.5f), right = rb(1.0f, 0.5f, 0.5f);
  // la référence « centre » est le moteur 0.2.0 (paramètre au défaut = 0) : bit à bit
  G4Scenario c0;
  c0.samples = 24000;
  c0.events = {MidiEvent::noteOn(0, 50, 100)};
  const auto def = renderG4(c0).L;
  bool ok = th::sameFloats(ref, def) && th::sameFloats(left, wtOnly) && th::sameFloats(right, fmOnly);
  const auto half = rb(0.5f, 0.5f, 0.5f);
  ok = ok && peak(half) <= peak(ref) + 1e-6;
  TK_CHECK(ok);
  tk::result("BAL-01", ok, "balance WT/FM : centre = 0.2.0 bit a bit, -1 = WT seule et +1 = FM seul bit a bit (modulation CROSS inchangee), aucune amplification");
}

void testEnvelopes2() {
  auto run = [](int voiceMode, int trig, std::vector<int>& stages, std::vector<double>& levels) {
    ParamValues p = ParamValues::defaults();
    p[ParamId::VoiceMode] = static_cast<float>(voiceMode);
    p[ParamId::EnvAux1Trigger] = static_cast<float>(trig);
    p[ParamId::EnvAux1Attack] = 0.2f;
    p[ParamId::EnvAux1Decay] = 0.2f;
    p[ParamId::EnvAux1Sustain] = 0.3f;
    Engine e(FactoryBanks::get("factory:saw"));
    e.setParameters(p);
    e.prepare(48000, 64);
    float L[64], R[64];
    auto send = [&](std::vector<MidiEvent> ev, int blocks) {
      for (int b = 0; b < blocks; ++b) e.process(L, R, 64, b == 0 ? ev.data() : nullptr, b == 0 ? static_cast<int>(ev.size()) : 0);
    };
    send({MidiEvent::noteOn(0, 60, 100)}, 600);  // ~0,8 s : sustain
    stages.push_back(static_cast<int>(e.testEnvStage(3)));
    const double before = e.testSource(ModSource::EnvAux1);
    send({MidiEvent::noteOn(0, 64, 100)}, 1);  // seconde note en legato
    stages.push_back(static_cast<int>(e.testEnvStage(3)));
    levels.push_back(std::fabs(e.testSource(ModSource::EnvAux1) - before));
  };
  std::vector<int> sMonoVoice, sLegVoice, sLegNote, sMonoGate;
  std::vector<double> lv;
  run(0, 0, sMonoVoice, lv);
  run(1, 0, sLegVoice, lv);
  run(1, 1, sLegNote, lv);
  run(0, 2, sMonoGate, lv);
  const int A = static_cast<int>(Envelope::Stage::Attack), S = static_cast<int>(Envelope::Stage::Sustain);
  bool trig = sMonoVoice[1] == A && sLegVoice[1] == S && sLegNote[1] == A && sMonoGate[1] == S;
  double jump = 0.0;
  for (double d : lv) jump = std::max(jump, d);
  const bool nojump = jump <= 64.0 / (0.2 * 48000.0) + 1e-9;  // au plus un bloc de pente d'attaque
  // synchronisation tempo : attaque d'une noire
  auto attackSamples = [](double bpm) {
    ParamValues p = ParamValues::defaults();
    p[ParamId::EnvAux1Sync] = 1.0f;
    p[ParamId::EnvAux1Attack] = 1.0f;
    Engine e(FactoryBanks::get("factory:saw"));
    e.setParameters(p);
    e.prepare(48000, 1);
    TransportInfo t;
    t.valid = true;
    t.bpm = bpm;
    e.setTransport(t);
    float L, R;
    MidiEvent on = MidiEvent::noteOn(0, 60, 100);
    e.process(&L, &R, 1, &on, 1);
    int n = 1;
    while (e.testSource(ModSource::EnvAux1) < 1.0 && n < 200000) {
      e.process(&L, &R, 1, nullptr, 0);
      ++n;
    }
    return n;
  };
  const int a120 = attackSamples(120.0), a90 = attackSamples(90.0);
  const bool sync = std::abs(a120 - 24000) <= 1 && std::abs(a90 - 32000) <= 1;
  // défaut : comportement 0.2.0 inchangé (REG-01 couvre l'Amp ; ici, toutes les enveloppes en Voice)
  const bool ok = trig && nojump && sync;
  TK_CHECK(ok);
  tk::result("ENV-02", ok, "declenchement Voice/Note/Gate conforme (Mono-Voice relance, Legato-Voice non, Legato-Note relance, Mono-Gate non) %d ; saut max %.4g (pente) %d ; sync 1 noire : %d ech. a 120 bpm (24000), %d a 90 bpm (32000)",
             trig, jump, nojump, a120, a90);
}

void testHeadroom() {
  // HR-01 : sous +24 dBFS, l'ELP n'est jamais engagée (chemin bit à bit identique au moteur sans ELP)
  std::mt19937_64 rng(0x4801ULL);
  unsigned elp = 0;
  double pk = 0.0;
  for (int c = 0; c < 50; ++c) {
    G4Scenario sc;
    sc.samples = 24000;
    for (int p = 0; p < kNumParams; ++p) {
      const auto& sp = paramSpec(p);
      if (sp.type != ParamType::Float) continue;
      std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
      sc.state.params.v[static_cast<size_t>(p)] = u(rng);
    }
    sc.state.params[ParamId::OutGain] = std::min(sc.state.params[ParamId::OutGain], 0.0f);
    for (ParamId f : {ParamId::FltWtOut, ParamId::FltFmOut, ParamId::FltGlOut}) sc.state.params[f] = std::min(sc.state.params[f], 0.0f);
    sc.events = {MidiEvent::noteOn(0, 40 + c, 100)};
    const auto r = renderG4(sc);
    elp += r.elp;
    pk = std::max(pk, peak(r.L));
  }
  const bool hr1 = elp == 0 && pk < 16.0;
  TK_CHECK(hr1);
  tk::result("HR-01", hr1, "50 configurations aleatoires a gains <= 0 dB : engagements ELP %u (0 attendu), crete %.2f (< 16) : sortie inchangee par SPEC-HR-01", elp, pk);
  // HR-02 : protocole DST-09 sous SPEC-HR-01 : engagements rapportés, garde jamais déclenchée
  std::mt19937_64 r2(0xD57ULL);
  unsigned guard = 0, elp2 = 0, nan = 0;
  for (int c = 0; c < 48; ++c) {
    G4Scenario sc;
    const double rates[4] = {44100.0, 48000.0, 88200.0, 96000.0};
    sc.fs = rates[c % 4];
    sc.samples = static_cast<long long>(sc.fs * 2.0);
    for (int slot = 0; slot < 6; ++slot) {
      const int b = idx(ParamId::DstWt1Type) + slot * static_cast<int>(DstOff::Count);
      for (int o = 0; o < static_cast<int>(DstOff::Count); ++o) {
        const auto& sp = paramSpec(b + o);
        std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
        sc.state.params.v[static_cast<size_t>(b + o)] = sanitize(static_cast<ParamId>(b + o), u(r2));
      }
    }
    sc.events = {MidiEvent::noteOn(100, 45, 100)};
    const auto r = renderG4(sc);
    guard += r.clamp;
    elp2 += r.elp;
    nan += r.nonFinite;
  }
  const bool hr2 = guard == 0 && nan == 0;
  TK_CHECK(hr2);
  tk::result("HR-02", hr2, "protocole DST-09 sous SPEC-HR-01 (proposee) : garde +30 dBFS %u, non-finis %u, engagements ELP rapportes %u", guard, nan, elp2);
}

void testMicrotuning() {
  auto f0 = [](int scale, float pc9, float a4) {
    ParamValues p = ParamValues::defaults();
    p[ParamId::TuneScale] = static_cast<float>(scale);
    p[ParamId::TunePc9] = pc9;
    p[ParamId::TuneA4] = a4;
    Engine e(FactoryBanks::get("factory:saw"));
    e.setParameters(p);
    e.prepare(48000, 64);
    float L[64], R[64];
    MidiEvent on = MidiEvent::noteOn(0, 69, 100);
    e.process(L, R, 64, &on, 1);
    return e.testF0();
  };
  const double c = f0(1, 50.0f, 440.0f), eq = f0(0, 50.0f, 440.0f), a432 = f0(0, 0.0f, 432.0f);
  const bool ok = std::fabs(1200.0 * std::log2(c / (440.0 * std::exp2(50.0 / 1200.0)))) < 0.1 && eq == 440.0 && std::fabs(a432 - 432.0) < 1e-9;
  TK_CHECK(ok);
  tk::result("MT-01", ok, "microtuning : Custom +50 ct sur La -> %.4f Hz ; Equal ignore les ecarts (%.4f) ; A4 432 -> %.4f", c, eq, a432);
}

void testFollowers() {
  // FOL-01 : suiveur WT proportionnel au niveau ; route suiveur WT -> cutoff global active et efficace
  auto fol = [](float level) {
    ParamValues p = ParamValues::defaults();
    p[ParamId::WtLevel] = level;
    p[ParamId::Op1Level] = 0.0f;
    Engine e(FactoryBanks::get("factory:saw"));
    e.setParameters(p);
    e.prepare(48000, 256);
    float L[256], R[256];
    MidiEvent on = MidiEvent::noteOn(0, 57, 127);
    for (int b = 0; b < 100; ++b) e.process(L, R, 256, b == 0 ? &on : nullptr, b == 0 ? 1 : 0);
    return e.testSource(ModSource::Follower1);
  };
  const double f1 = fol(0.2f), f2 = fol(0.4f);
  const bool prop = f1 > 0.0 && std::fabs(f2 / f1 - 2.0) < 0.05 && f2 <= 1.0;
  ModMatrixState m;
  ModRoute r;
  r.source = ModSource::Follower1;
  r.dest = ParamId::FltGlCutoff;
  r.depth = 1.0f;
  modAdd(m, r);
  const bool active = modRouteStatus(m)[0] == ModInactive::Active;
  G4Scenario a, b;
  a.samples = b.samples = 24000;
  a.state.params[ParamId::FltGlType] = b.state.params[ParamId::FltGlType] = 1.0f;
  a.state.params[ParamId::FltGlCutoff] = b.state.params[ParamId::FltGlCutoff] = 200.0f;
  b.state.mod = m;
  a.events = b.events = {MidiEvent::noteOn(0, 57, 127)};
  const double d = nmseDb(renderG4(b).L, renderG4(a).L);
  const bool ok = prop && active && d > -20.0;
  TK_CHECK(ok);
  tk::result("FOL-01", ok, "suiveur WT : %.4f puis %.4f pour un niveau double (rapport %.3f) ; route suiveur WT -> cutoff global active, effet NMSE %.1f dB", f1, f2, f2 / f1, d);
}

}  // namespace

void runG41Tests() {
  std::printf("[arch-g4.1]\n");
  testBalance();
  testEnvelopes2();
  testHeadroom();
  testMicrotuning();
  testFollowers();
}
