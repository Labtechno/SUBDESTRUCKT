// SUBDESTRUCKT — tests ARCH-G4.1 (BAL-01, ENV-02, HR-01/02, MT-01, FOL-01). Seuils : tracker §3bis (avant mesure).
#include <chrono>
#include <cmath>
#include "Analysis.h"
#include <cstdio>
#include <memory>
#include <random>
#include <set>
#include <vector>

#include "G4Helper.h"
#include "TestKit.h"
#include "core/StateCodec.h"
#include "core/WtAssets.h"
#include "core/WtLibrary.h"
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

// ------------------------------------------------------------------ N1-02 : hybride ADAA1 + suréchantillonnage interne
struct N1Run {
  double alias = 0.0, ns = 0.0;
};
N1Run n1Measure(Quality q, double f0, double driveDb, double fold) {
  const double fs = 48000.0;
  DestructionSlot d;
  d.configure(fs, 1, q);
  std::vector<double> E(kNumParams);
  ParamValues p = ParamValues::defaults();
  const int b = idx(ParamId::DstGl1Type);
  p.v[static_cast<size_t>(b)] = 1.0f;
  p.v[static_cast<size_t>(b + static_cast<int>(DstOff::N1Drive))] = static_cast<float>(driveDb);
  p.v[static_cast<size_t>(b + static_cast<int>(DstOff::N1Fold))] = static_cast<float>(fold);
  p.v[static_cast<size_t>(b + static_cast<int>(DstOff::N1Tone))] = 1.0f;
  for (int i = 0; i < kNumParams; ++i) E[static_cast<size_t>(i)] = p.v[static_cast<size_t>(i)];
  DestructionContext c;
  c.fs = fs;
  c.quality = q;
  const int N = 8192 + 16384 + 4096;
  std::vector<double> y(static_cast<size_t>(N));
  const auto t0 = std::chrono::steady_clock::now();
  for (int i = 0; i < N; ++i) {
    double l = 0.8 * std::sin(kTwoPi * f0 * i / fs), r = l;
    d.process(l, r, E.data(), b, c);
    y[static_cast<size_t>(i)] = l;
  }
  N1Run out;
  out.ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / N;
  std::vector<double> seg(y.begin() + 8192, y.begin() + 8192 + 16384);
  const auto m = an::amplitudeSpectrum(seg.data(), seg.size());
  out.alias = an::offGridEnergyDb(m, fs / 16384.0, f0, 20.0, 0.45 * fs, 6);
  return out;
}

void testN1Hybrid() {
  const auto e = n1Measure(Quality::Eco, 3100.0, 24.0, 1.0), n = n1Measure(Quality::Normal, 3100.0, 24.0, 1.0),
             h = n1Measure(Quality::High, 3100.0, 24.0, 1.0);
  const auto em = n1Measure(Quality::Eco, 1000.0, 12.0, 0.5), nm = n1Measure(Quality::Normal, 1000.0, 12.0, 0.5),
             hm = n1Measure(Quality::High, 1000.0, 12.0, 0.5);
  // latence déclarée = latence mesurée ; dry/wet aligné (impulsion à faible niveau, chemin quasi linéaire)
  bool lat = true;
  for (Quality q : {Quality::Normal, Quality::High}) {
    for (double mix : {0.0, 1.0}) {
      DestructionSlot d;
      d.configure(48000.0, 1, q);
      std::vector<double> E(kNumParams);
      ParamValues p = ParamValues::defaults();
      const int b = idx(ParamId::DstGl1Type);
      p.v[static_cast<size_t>(b)] = 1.0f;
      p.v[static_cast<size_t>(b + static_cast<int>(DstOff::N1Fold))] = 0.0f;
      p.v[static_cast<size_t>(b + static_cast<int>(DstOff::N1Tone))] = 1.0f;
      p.v[static_cast<size_t>(b + static_cast<int>(DstOff::Mix))] = static_cast<float>(mix);
      for (int i = 0; i < kNumParams; ++i) E[static_cast<size_t>(i)] = p.v[static_cast<size_t>(i)];
      DestructionContext c;
      c.fs = 48000.0;
      c.quality = q;
      std::vector<double> y(4096);
      for (int i = 0; i < 4096; ++i) {
        double l = i == 2000 ? 0.01 : 0.0, r = l;
        d.process(l, r, E.data(), b, c);
        y[static_cast<size_t>(i)] = l;
      }
      size_t arg = 0;
      for (size_t i = 0; i < y.size(); ++i)
        if (std::fabs(y[i]) > std::fabs(y[arg])) arg = i;
      const long dl = static_cast<long>(arg) - 2000;
      lat = lat && std::labs(dl - n1LatencyFor(q)) <= 1 && d.latency() == n1LatencyFor(q);
      tk::info("N1-02 Quality %s mix %.0f : pic a +%ld (declare %d)", qualityName(q), mix, dl, n1LatencyFor(q));
    }
  }
  const bool ok = n.alias <= e.alias - 3.0 && h.alias <= e.alias - 6.0 && lat;
  TK_CHECK(ok);
  tk::result("N1-02", ok, "N1 hybride (3,1 kHz, drive 24, fold 1) energie repliee : Eco/ADAA1 %.1f dB (%.0f ns), Normal x2 %.1f dB (%.0f ns), High x4 %.1f dB (%.0f ns) ; cas modere (1 kHz, drive 12, fold 0,5) : %.1f / %.1f / %.1f dB ; latence declaree = mesuree, dry/wet aligne %d (Normal %d, High %d ech.)",
             e.alias, e.ns, n.alias, n.ns, h.alias, h.ns, em.alias, nm.alias, hm.alias, lat, n1LatencyFor(Quality::Normal), n1LatencyFor(Quality::High));
}

// ------------------------------------------------------------------ WT-SM-01 : morph spectral
void testSpectralMorph() {
  // deux formes de même spectre d'amplitude, en opposition de phase
  std::vector<float> raw(2 * 2048);
  for (int i = 0; i < 2048; ++i) {
    const double th = kTwoPi * i / 2048.0;
    const float v = static_cast<float>(0.6 * std::sin(th) + 0.3 * std::sin(3.0 * th + 0.4));
    raw[static_cast<size_t>(i)] = v;
    raw[static_cast<size_t>(2048 + i)] = -v;
  }
  std::string err;
  auto bank = WtBank::build(userBankId(2, 1, raw), "opp", 2, 1, raw, WtBank::Options{}, err);
  UserBanks::add(bank);
  auto rd = [&](float x, int morph) {
    G4Scenario sc;
    sc.samples = 24000;
    sc.state = bareState();
    sc.state.bank = bank->id();
    sc.state.params[ParamId::WtLevel] = 0.5f;
    sc.state.params[ParamId::WtX] = x;
    sc.state.params[ParamId::WtXMorph] = static_cast<float>(morph);
    sc.state.params[ParamId::WtRender] = 1.0f;  // Raw : bande pleine, comparaison directe aux formes
    sc.events = {MidiEvent::noteOn(0, 45, 127)};
    Engine eng(bank);
    (void)eng;
    return sc;
  };
  auto render = [&](float x, int morph) {
    G4Scenario sc = rd(x, morph);
    // renderG4 résout la banque par l'identifiant (registre utilisateur)
    return renderG4(sc).L;
  };
  const auto lin0 = render(0.0f, 0), sp0 = render(0.0f, 2);
  const auto linMid = render(0.5f, 0), spMid = render(0.5f, 2);
  const double e0 = rms(lin0, 4800, 24000), em = rms(spMid, 4800, 24000), lm = rms(linMid, 4800, 24000);
  const double grid = nmseDb(std::vector<float>(sp0.begin() + 4800, sp0.end()), std::vector<float>(lin0.begin() + 4800, lin0.end()));
  const bool ok = grid <= -60.0 && em >= 0.7 * e0 && lm <= 0.05 * e0;
  TK_CHECK(ok);
  tk::result("WT-SM-01", ok, "morph spectral : point de grille NMSE %.1f dB (<= -60) ; milieu de formes en opposition de phase : RMS spectral %.3f vs extremite %.3f (>= 0,7x), lineaire %.4f (<= 0,05x)",
             grid, em, e0, lm);
}

// ------------------------------------------------------------------ CAP-01 : captures créatives R2
struct CapRun {
  double rms = 0.0;
  long long alloc = 0;
};
CapRun capRender(int type, bool note, const CaptureAsset* restore, CaptureAsset* grab) {
  ParamValues p = ParamValues::defaults();
  p[ParamId::DstGl1Type] = static_cast<float>(type);
  p[ParamId::DstGl1Arm] = 1.0f;
  p[ParamId::DstGl1Mix] = 1.0f;
  Engine e(FactoryBanks::get("factory:saw"));
  e.setParameters(p);
  e.prepare(48000, 256);
  float L[256], R[256];
  CapRun out;
  MidiEvent on = MidiEvent::noteOn(0, 52, 120), off = MidiEvent::noteOff(0, 52);
  if (restore) {
    e.captureExchange().publish(std::make_unique<CaptureSet>(CaptureSet{{*restore}}));
  }
  double acc = 0.0;
  long n = 0;
  for (int b = 0; b < 600; ++b) {
    const MidiEvent* ev = nullptr;
    int ne = 0;
    if (note && b == 0) ev = &on, ne = 1;
    if (note && b == 200) ev = &off, ne = 1;  // ~1 s puis relâchement : le gel armé persiste
    if (grab && b == 300) e.requestCapture(4);
    if (type == static_cast<int>(DstType::D2) && (b == 150 || (b == 0 && !note))) {  // gel activé pendant la note
      p[ParamId::DstGl1D2Freeze] = 1.0f;
      e.setParameters(p);
    }
    tk::allocationCount().store(0);
    e.process(L, R, 256, ev, ne);
    out.alloc += tk::allocationCount().load();
    if (b >= 400)
      for (int i = 0; i < 256; ++i, ++n) acc += static_cast<double>(L[i]) * L[i];
  }
  if (grab) e.fetchCapture(4, *grab);
  out.rms = std::sqrt(acc / std::max(1L, n));
  return out;
}

void testCaptures() {
  const int D2 = static_cast<int>(DstType::D2), S2 = static_cast<int>(DstType::S2);
  CaptureAsset cd, cs;
  const auto refD = capRender(D2, true, nullptr, &cd), refS = capRender(S2, true, nullptr, &cs);
  const auto silD = capRender(D2, false, nullptr, nullptr), silS = capRender(S2, false, nullptr, nullptr);
  const auto resD = capRender(D2, false, &cd, nullptr), resS = capRender(S2, false, &cs, nullptr);
  const bool grabbed = cd.frames > 0 && cd.kind == 0 && cs.frames == 2048 / 2 + 1 && cs.kind == 1;
  auto ratioDb = [](double a, double b) { return 20.0 * std::log10((a + 1e-12) / (b + 1e-12)); };
  const double dD = ratioDb(resD.rms, refD.rms), dS = ratioDb(resS.rms, refS.rms);
  const bool restored = silD.rms < 1e-6 && silS.rms < 1e-6 && std::fabs(dD) <= 3.0 && std::fabs(dS) <= 3.0;
  // état : encodage, décodage identique, intégrité, refus d'une capture altérée
  PluginState st = defaultState();
  st.captures = {cd, cs};
  st.captures[0].slot = 4;
  st.captures[1].slot = 5;
  const std::string txt = encodeState(st, "test", StateScope::Preset);
  PluginState back;
  std::string err;
  const bool rt = decodeState(txt, back, err) && back.captures == st.captures;
  std::string bad = txt;
  const size_t at = bad.find("capture 4 D2");
  const size_t pos = bad.find(static_cast<char>(10), at) - 5;
  bad[pos] = bad[pos] == 'A' ? 'B' : 'A';
  PluginState b2;
  const bool integ = !decodeState(bad, b2, err);
  // protection : un preset armé est désarmé au chargement
  PluginState armed = defaultState();
  armed.params[ParamId::DstGl1Arm] = 1.0f;
  armed.params[ParamId::FltWtArm] = 1.0f;
  const bool was = anyArmed(armed);
  disarmAutonomous(armed);
  const bool prot = was && !anyArmed(armed);
  const long long alloc = refD.alloc + refS.alloc + resD.alloc + resS.alloc;
  const bool ok = grabbed && restored && rt && integ && prot && alloc == 0;
  TK_CHECK(ok);
  tk::result("CAP-01", ok, "captures R2 : D2 %d trames, S2 %d bins ; restauration sans entree : D2 %.1f dB, S2 %.1f dB vs original (+-3 ; sans capture : silence) ; etat aller-retour %d, capture alteree refusee %d ; preset arme desarme %d ; allocations RT %lld",
             cd.frames, cs.frames, dD, dS, rt, integ, prot, alloc);
}

// ------------------------------------------------------------------ WT-ED-01 : édition, génération, bibliothèque
void testWtEditing() {
  // génération procédurale : déterministe, bornée, préparée (mip-maps + spectres)
  bool gen = true;
  std::string err;
  for (int k = 0; k < static_cast<int>(WtProcedure::Count); ++k) {
    const auto a = generateProcedural(static_cast<WtProcedure>(k), 16, 4, 7, 0.6), b = generateProcedural(static_cast<WtProcedure>(k), 16, 4, 7, 0.6);
    gen = gen && a == b && a.size() == 16u * 4u * 2048u;
    for (float v : a) gen = gen && std::isfinite(v);
    auto bank = WtBank::build(userBankId(16, 4, a), procedureName(static_cast<WtProcedure>(k)), 16, 4, a, WtBank::Options{}, err);
    gen = gen && bank && bank->nx() == 16 && bank->ny() == 4;
  }
  const auto big = generateProcedural(WtProcedure::HarmonicTilt, 200, 40, 1, 1.0);  // grille bornée 64 x 16
  gen = gen && big.size() == 64u * 16u * 2048u;
  // forme dessinée : triangle par 3 points -> DC nul, crête 1, valeur au quart de période
  const auto tri = drawnWave({{0.0, -1.0}, {0.5, 1.0}, {0.999, -1.0}});
  double mean = 0.0, pk = 0.0;
  for (float v : tri) {
    mean += v;
    pk = std::max(pk, static_cast<double>(std::fabs(v)));
  }
  mean /= tri.size();
  const bool draw = std::fabs(mean) < 1e-6 && std::fabs(pk - 1.0) < 1e-6 && std::fabs(tri[512] - tri[1536]) < 0.01;
  // remplacement de forme : copie, l'original reste inchangé ; refus d'un index invalide
  const auto base = generateProcedural(WtProcedure::PulseWidth, 4, 1, 0, 0.5);
  std::vector<float> edited;
  const bool rep = replaceWave(base, 4, 1, 2, tri, edited, err) && std::equal(tri.begin(), tri.end(), edited.begin() + 2 * 2048) &&
                   std::equal(base.begin(), base.begin() + 2 * 2048, edited.begin()) && !replaceWave(base, 4, 1, 9, tri, edited, err);
  // fichier .sdwt : aller-retour (même identifiant de contenu), altération refusée
  auto b0 = WtBank::build(userBankId(4, 1, edited), "Edit test", 4, 1, edited, WtBank::Options{}, err);
  const std::string txt = encodeBankFile(*b0);
  auto b1 = decodeBankFile(txt, err);
  const bool file = b1 && b1->id() == b0->id() && b1->raw() == b0->raw() && b1->name() == "Edit test";
  std::string bad = txt;
  const size_t at = bad.find("data ") + 40;
  bad[at] = bad[at] == 'A' ? 'B' : 'A';
  const bool tamper = !decodeBankFile(bad, err);
  // portabilité : la banque intégrée à l'état est restaurée dans un registre où elle est absente (même rendu)
  PluginState st = defaultState();
  st.bank = b0->id();
  st.bankRaw = b0->raw();
  st.bankNx = 4;
  st.bankNy = 1;
  st.bankName = "Edit test";
  PluginState back;
  const bool port = decodeState(encodeState(st, "t", StateScope::Preset), back, err) && back.bankRaw == st.bankRaw && back.bank == st.bank;
  const bool ok = gen && draw && rep && file && tamper && port;
  TK_CHECK(ok);
  tk::result("WT-ED-01", ok, "banques utilisateur : %d generateurs deterministes et bornes 64x16 %d ; forme dessinee (DC %.1e, crete %.3f) %d ; remplacement par copie %d ; fichier .sdwt aller-retour (meme id) %d, altere refuse %d ; banque integree a l'etat %d",
             static_cast<int>(WtProcedure::Count), gen, mean, pk, draw, rep, file, tamper, port);
}

// ------------------------------------------------------------------ §9 : Character, topologies FM, dépendances
void testCharacterTopologyDeps() {
  // CHR-01 : WT Character (3 profils) et FM Character : à 0, chaque profil est neutre bit à bit ; > 0, effet déterministe
  auto rc = [](int mode, float amt, bool fm) {
    G4Scenario sc;
    sc.samples = 12000;
    sc.state = fm ? defaultState() : bareState();
    if (fm) {
      sc.state.params[ParamId::Op1Char] = amt;
      sc.state.params[ParamId::FmCharMaster] = 1.0f;
    } else {
      sc.state.params[ParamId::WtLevel] = 0.5f;  // bareState coupe la WT
      sc.state.params[ParamId::WtChar] = amt;
      sc.state.params[ParamId::WtCharMode] = static_cast<float>(mode);
    }
    sc.events = {MidiEvent::noteOn(0, 52, 100)};
    return renderG4(sc).L;
  };
  bool chr = true;
  const auto ref = rc(0, 0.0f, false);
  for (int m = 0; m < 3; ++m) {
    const auto z = rc(m, 0.0f, false), a = rc(m, 0.6f, false), b = rc(m, 0.6f, false);
    chr = chr && th::sameFloats(z, ref) && !th::sameFloats(a, ref) && th::sameFloats(a, b);
  }
  const auto f0 = rc(0, 0.0f, true), f1 = rc(0, 0.7f, true);
  chr = chr && !th::sameFloats(f0, f1);
  TK_CHECK(chr);
  tk::result("CHR-01", chr, "Character WT (3 profils) et FM : neutre bit a bit a 0, effet deterministe > 0 : %d", chr);
  // TOPO-01 : 10 topologies FM : transaction valide, routes CROSS conservées, rendus finis et distincts deux à deux
  PluginState base = defaultState();
  RouteTable t0;
  std::string err;
  t0.assign(base.routes, base.nextRouteId, err);
  t0.apply({RouteTable::Op::add(kNodeOP1, kPortWtX)});  // route CROSS à conserver
  int crossBefore = 0;
  for (const auto& r : t0.routes()) crossBefore += (r.dst >= kNumNodes - 1 || r.src == kNodeWT || r.src == kNodeBUS) ? 1 : 0;
  std::vector<std::vector<float>> outs;
  bool topo = true;
  std::set<std::string> names;
  for (int k = 0; k < kNumFmTopologies; ++k) {
    RouteTable t = t0;
    const auto res = t.apply(fmTopologyOps(t, k));
    int crossAfter = 0;
    for (const auto& r : t.routes()) crossAfter += (r.dst >= kNumNodes - 1 || r.src == kNodeWT || r.src == kNodeBUS) ? 1 : 0;
    topo = topo && res.ok && t.immediateAcyclic() && crossAfter == crossBefore;
    names.insert(fmTopologyName(k));
    G4Scenario sc;
    sc.samples = 9600;
    sc.state = base;
    sc.state.routes = t.routes();
    sc.state.nextRouteId = t.nextRouteId();
    for (ParamId l : {ParamId::Op2Level, ParamId::Op3Level, ParamId::Op4Level}) sc.state.params[l] = 0.2f;
    for (int s = 0; s < kMaxRoutes; ++s)
      if (routeSlotInfo(s).src >= kNodeOP1 && routeSlotInfo(s).src <= kNodeOP4 && routeSlotInfo(s).dst >= kNodeOP1 && routeSlotInfo(s).dst <= kNodeOP4)
        sc.state.params[routeSlotInfo(s).depthParam] = 0.8f;
    sc.events = {MidiEvent::noteOn(0, 45, 100)};
    const auto r = renderG4(sc);
    topo = topo && r.nonFinite == 0;
    outs.push_back(r.L);
  }
  int distinct = 0, pairs = 0;
  for (size_t a = 0; a < outs.size(); ++a)
    for (size_t b = a + 1; b < outs.size(); ++b, ++pairs) distinct += th::sameFloats(outs[a], outs[b]) ? 0 : 1;
  topo = topo && distinct == pairs && static_cast<int>(names.size()) == kNumFmTopologies;
  TK_CHECK(topo);
  tk::result("TOPO-01", topo, "%d topologies FM : transactions valides et acycliques (immediat), routes CROSS conservees, rendus finis, %d/%d paires distinctes",
             kNumFmTopologies, distinct, pairs);
  // MACRO-01 : audit des dépendances : cycle entre macros refusé (2 et 3 macros), auto-cycle refusé, suiveur en amont
  // inactif avec raison, route valide active
  auto macroRoute = [](ModSource s, ParamId d) {
    ModRoute r;
    r.source = s;
    r.dest = d;
    r.depth = 0.5f;
    return r;
  };
  ModMatrixState m;
  const bool a1 = modAdd(m, macroRoute(ModSource::Macro1, ParamId::Macro2)).ok;
  const bool a2 = !modAdd(m, macroRoute(ModSource::Macro2, ParamId::Macro1)).ok;  // M1 -> M2 -> M1
  const bool a3 = modAdd(m, macroRoute(ModSource::Macro2, ParamId::Macro3)).ok && !modAdd(m, macroRoute(ModSource::Macro3, ParamId::Macro1)).ok;
  const bool a4 = !modAdd(m, macroRoute(ModSource::Macro4, ParamId::Macro4)).ok;
  ModMatrixState f;
  modAdd(f, macroRoute(ModSource::Follower1, ParamId::WtX));
  modAdd(f, macroRoute(ModSource::Follower1, ParamId::FltGlCutoff));
  const auto st = modRouteStatus(f);
  const bool a5 = st.size() == 2 && st[0] == ModInactive::FollowerCycle && st[1] == ModInactive::Active && std::string(modInactiveText(st[0])).size() > 5;
  const bool deps = a1 && a2 && a3 && a4 && a5;
  TK_CHECK(deps);
  tk::result("MACRO-01", deps, "dependances : M1>M2 accepte %d ; M2>M1 refuse %d ; cycle a 3 refuse %d ; auto-cycle refuse %d ; suiveur WT > WT X inactif (raison affichee), > cutoff global actif %d",
             a1, a2, a3, a4, a5);
}

// ENV-02 (clause BPM) : changement de tempo pendant l'attaque synchronisée, pris en compte au bloc suivant sans saut
void testEnvBpmChange() {
  ParamValues p = ParamValues::defaults();
  p[ParamId::EnvAux1Sync] = 1.0f;
  p[ParamId::EnvAux1Attack] = 1.0f;  // une noire
  Engine e(FactoryBanks::get("factory:saw"));
  e.setParameters(p);
  e.prepare(48000, 64);
  TransportInfo t;
  t.valid = true;
  t.bpm = 120.0;
  e.setTransport(t);
  float L[64], R[64];
  MidiEvent on = MidiEvent::noteOn(0, 60, 100);
  e.process(L, R, 64, &on, 1);
  int n = 64;
  while (n < 12000) {
    e.process(L, R, 64, nullptr, 0);
    n += 64;
  }
  const double mid = e.testSource(ModSource::EnvAux1);
  t.bpm = 60.0;  // une noire = 1 s : le reste de l'attaque ralentit
  e.setTransport(t);
  double prev = mid, jump = 0.0;
  while (e.testSource(ModSource::EnvAux1) < 1.0 && n < 200000) {
    e.process(L, R, 64, nullptr, 0);
    n += 64;
    jump = std::max(jump, e.testSource(ModSource::EnvAux1) - prev);
    prev = e.testSource(ModSource::EnvAux1);
  }
  // attendu : ~12000 éch. à 120 bpm (moitié) puis la moitié restante à 60 bpm (48000 x 0,5 = 24000) : ~36000
  const bool ok = std::abs(n - 36000) <= 128 && jump <= 64.0 / 48000.0 + 1e-9;
  TK_CHECK(ok);
  tk::result("ENV-03", ok, "sync tempo : BPM 120 -> 60 a mi-attaque : fin d'attaque a %d ech. (36000 +-128), pas max %.5f (<= pente 60 bpm), niveau a la bascule %.3f", n, jump, mid);
}

}  // namespace

void runG41Tests() {
  std::printf("[arch-g4.1]\n");
  testBalance();
  testEnvelopes2();
  testEnvBpmChange();
  testHeadroom();
  testMicrotuning();
  testFollowers();
  testN1Hybrid();
  testSpectralMorph();
  testCaptures();
  testWtEditing();
  testCharacterTopologyDeps();
}
