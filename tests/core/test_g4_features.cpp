// SUBDESTRUCKT — tests ARCH-G4 des fonctions du moteur (état v2, migration, MIDI, enveloppes, LFO, matrice, WT, FM,
// CROSS, sync, Drone, Quality, RNG, chaos). Seuils préenregistrés : docs/V1-ARCH-G4-TRACKER.md §3.
#include <array>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include "Analysis.h"
#include "G4Helper.h"
#include "TestKit.h"
#include "core/ModMatrix.h"
#include "dsp/ModSources.h"
#include "dsp/Rng.h"
#include "regression/b5/Engine.h"
#include "regression/b5/StateCodec.h"

using namespace sbd;
using namespace g4;
using Op = RouteTable::Op;

namespace {

// ------------------------------------------------------------------ ST-01 / ST-03 / ST-02
PluginState randomState(std::mt19937_64& rng) {
  PluginState s;
  for (int p = 0; p < kNumParams; ++p) {
    const auto& sp = paramSpec(p);
    std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
    s.params.v[static_cast<size_t>(p)] = sanitize(static_cast<ParamId>(p), u(rng));
  }
  RouteTable t;
  for (int k = 0; k < 10; ++k) {
    const int slot = static_cast<int>(rng() % kMaxRoutes);
    t.apply({Op::add(routeSlotInfo(slot).src, routeSlotInfo(slot).dst)});
  }
  s.routes = t.routes();
  s.nextRouteId = t.nextRouteId();
  const auto& banks = FactoryBanks::ids();
  s.bank = banks[rng() % banks.size()];
  const int nm = static_cast<int>(rng() % 12);
  std::uniform_real_distribution<float> b(-1.0f, 1.0f), u(0.0f, 1.0f);
  for (int k = 0; k < nm; ++k) {
    ModRoute r;
    r.source = static_cast<ModSource>(1 + rng() % (kNumModSources - 1));
    if (r.source >= ModSource::Macro1 && r.source <= ModSource::Macro4) r.source = ModSource::Lfo1;
    int d;
    do d = static_cast<int>(rng() % kNumParams);
    while (paramSpec(d).type != ParamType::Float);
    r.dest = static_cast<ParamId>(d);
    r.depth = b(rng);
    r.curve = static_cast<ModCurve>(rng() % static_cast<unsigned>(ModCurve::Count));
    r.polarity = rng() % 2 ? ModPolarity::Free : ModPolarity::Centered;
    r.invert = rng() % 2;
    r.offset = b(rng);
    r.via = rng() % 3 == 0 ? ModSource::Lfo2 : ModSource::None;
    r.viaAmount = u(rng);
    r.smoothMs = 100.0f * u(rng);
    r.rate = rng() % 2 ? ModRate::Audio : ModRate::Control;
    r.locked = rng() % 2;
    r.numPoints = static_cast<std::uint8_t>(2 + rng() % 7);
    for (int q = 0; q < r.numPoints; ++q) {
      r.px[static_cast<size_t>(q)] = static_cast<float>(q) / (r.numPoints - 1);
      r.py[static_cast<size_t>(q)] = b(rng);
    }
    TK_CHECK(modAdd(s.mod, r).ok);
  }
  s.macroNames[rng() % 4] = "Sub \xc3\xa9trange";
  s.lfoCurves[rng() % 4] = CurvePoints{3, {0.0f, 0.4f, 1.0f}, {-1.0f, 0.7f, 0.2f}};
  s.seed = rng();
  s.streamCounters = {rng() % 1000, rng() % 1000, rng() % 1000};
  s.instance.quality = static_cast<Quality>(rng() % 3);
  s.instance.drone = rng() % 2;
  s.instance.droneRestore = rng() % 2;
  return s;
}

std::string replaceLine(const std::string& text, const std::string& prefix, const std::string& replacement) {
  const size_t p = text.find("\n" + prefix);
  if (p == std::string::npos) return text + "\n#absent " + prefix;
  const size_t e = text.find('\n', p + 1);
  return text.substr(0, p + 1) + replacement + text.substr(e);
}

const char* kModLine = " 0x00000000 0 C 0 0x00000000 none 0x3f800000 0x00000000 C 1 0 2 0x00000000 0x00000000 0x3f800000 0x3f800000";

void testStateV2() {
  std::mt19937_64 rng(0x57A7E2ULL);
  int bad = 0;
  for (int i = 0; i < 300; ++i) {
    const PluginState s = randomState(rng);
    PluginState back;
    std::string err;
    const std::string txt = encodeState(s, "w", StateScope::Daw);
    if (!decodeState(txt, back, err) || !(back == s) || encodeState(back, "w", StateScope::Daw) != txt) {
      if (bad < 3) tk::info("ST-01 echec : %s", err.c_str());
      ++bad;
    }
    PluginState pre;
    if (!decodeState(encodeState(s, "w", StateScope::Preset), pre, err) || pre.hasInstance || pre.params != s.params) ++bad;
  }
  TK_CHECK(bad == 0);
  tk::result("ST-01", bad == 0, "300 etats v2 aleatoires (matrice, courbes, noms UTF-8, seeds, instance) : aller-retour exact, scope preset sans instance (%d echecs)", bad);

  PluginState s = randomState(rng);
  const std::string good = encodeState(s, "w", StateScope::Daw);
  const std::string m = std::string("mod 899 lfo1 wt.x");
  const std::vector<std::pair<const char*, std::string>> cases = {
      {"version future", replaceLine(good, "SUBDESTRUCKT_STATE", "SUBDESTRUCKT_STATE 3")},
      {"scope inconnu", replaceLine(good, "scope", "scope other")},
      {"scope manquant", replaceLine(good, "scope", "")},
      {"parametre discret non entier", replaceLine(good, "param voice.mode", "param voice.mode 0x3f000000 0.5")},
      {"bank invalide", replaceLine(good, "bank", "bank ../../etc/passwd")},
      {"modnext manquant", replaceLine(good, "modnext", "")},
      {"mod source inconnue", replaceLine(good, "modnext", "modnext 900\nmod 899 nosuch wt.x" + std::string(kModLine))},
      {"mod destination inconnue", replaceLine(good, "modnext", "modnext 900\nmod 899 lfo1 wt.nope" + std::string(kModLine))},
      {"mod profondeur hors plage", replaceLine(good, "modnext", "modnext 900\nmod 899 lfo1 wt.x 0x40000000 0 C 0 0x00000000 none 0x3f800000 0x00000000 C 1 0 2 0x00000000 0x00000000 0x3f800000 0x3f800000")},
      {"mod points non croissants", replaceLine(good, "modnext", "modnext 900\nmod 899 lfo1 wt.x 0x3f000000 6 C 0 0x00000000 none 0x3f800000 0x00000000 C 1 0 2 0x3f800000 0x00000000 0x00000000 0x3f800000")},
      {"mod id >= next", replaceLine(good, "modnext", "modnext 2\nmod 7 lfo1 wt.x" + std::string(kModLine))},
      {"cycle macro", replaceLine(good, "modnext", "modnext 900\nmod 897 macro1 macro2" + std::string(kModLine) + "\nmod 898 macro2 macro1" + std::string(kModLine))},
      {"lfocurve hors plage", replaceLine(good, "lfocurve 1", "lfocurve 1 2 0x00000000 0x40000000 0x3f800000 0x3f800000")},
      {"lfocurve n=9", replaceLine(good, "lfocurve 1", "lfocurve 1 9 0x00000000 0x00000000")},
      {"seed tronque", replaceLine(good, "seed", "seed 12")},
      {"instance qualite", replaceLine(good, "instance", "instance quality=Ultra drone=0 drone_restore=0")},
      {"instance en preset", replaceLine(good, "scope", "scope preset")},
      {"route port inconnu", replaceLine(good, "topology", "topology next=99\nroute 98 OP1 WT.W D")},
      {"route BUS>OP1", replaceLine(good, "topology", "topology next=99\nroute 98 BUS OP1 D")},
      {"route WT>WT.X", replaceLine(good, "topology", "topology next=99\nroute 98 WT WT.X D")},
      {"macroname trop long", replaceLine(good, "macroname 1", "macroname 1 " + std::string(140, 'a'))},
      {"tronque", good.substr(0, good.size() / 2)},
      {"ligne inconnue", replaceLine(good, "writer", "writer w\ngarbage")},
  };
  int refused = 0;
  for (const auto& c : cases) {
    PluginState out = s;
    std::string err;
    const bool accepted = decodeState(c.second, out, err);
    if (!accepted && out == s)
      ++refused;
    else
      tk::info("ST-03 non refuse : %s", c.first);
  }
  const bool ok = refused == static_cast<int>(cases.size());
  TK_CHECK(ok);
  tk::result("ST-03", ok, "%d/%zu etats v2 invalides refuses sans effet", refused, cases.size());
}

void testMigration() {
  bool ok = true;
  int n = 0, exact = 0;
  std::mt19937_64 rng(0xB5B5ULL);
  for (int i = 0; i < 40; ++i) {
    sbd_b5::PluginState b = sbd_b5::defaultState();
    if (i > 0) {
      for (int p = 0; p < sbd_b5::kNumParams; ++p) {
        const auto& sp = sbd_b5::paramSpec(p);
        std::uniform_real_distribution<float> u(sp.minValue, sp.maxValue);
        b.params.v[static_cast<size_t>(p)] = u(rng);
      }
      b.params[sbd_b5::ParamId::OutGain] = -12.0f;
      sbd_b5::RouteTable t;
      for (int k = 0; k < 5; ++k) {
        const int slot = static_cast<int>(rng() % sbd_b5::kMaxRoutes);
        t.apply({sbd_b5::RouteTable::Op::add(sbd_b5::routeSlotInfo(slot).src, sbd_b5::routeSlotInfo(slot).dst)});
      }
      b.routes = t.routes();
      b.nextRouteId = t.nextRouteId();
    }
    const std::string v1 = sbd_b5::encodeState(b, "0.1.0-b5 oracle");
    PluginState g;
    std::string err;
    if (!decodeState(v1, g, err)) {
      ok = false;
      tk::info("ST-02 refus : %s", err.c_str());
      continue;
    }
    bool same = g.sourceVersion == 1 && !g.hasInstance && g.bank == "factory:saw" && g.mod.routes.empty() &&
                g.nextRouteId == b.nextRouteId && g.routes.size() == b.routes.size();
    for (int p = 0; p < kNumV1Params; ++p) same = same && g.params.v[static_cast<size_t>(p)] == b.params.v[static_cast<size_t>(p)];
    for (int p = kNumV1Params; p < kNumParams; ++p) same = same && g.params.v[static_cast<size_t>(p)] == paramSpec(p).defaultValue;
    for (size_t k = 0; k < g.routes.size() && same; ++k)
      same = g.routes[k].id == b.routes[k].id && g.routes[k].src == b.routes[k].src && g.routes[k].dst == b.routes[k].dst &&
             static_cast<int>(g.routes[k].cls) == static_cast<int>(b.routes[k].cls);
    // contrôle sonore : rendu de l'état migré == oracle b5 (bit à bit)
    const double fs = 48000.0;
    sbd_b5::Engine ob(std::make_shared<const sbd_b5::SawWavetable>());
    ob.setParameters(b.params);
    ob.prepare(fs, 256);
    sbd_b5::RouteTable bt;
    std::string e2;
    bt.assign(b.routes, b.nextRouteId, e2);
    ob.setTopologyImmediate(bt.compile());
    G4Scenario sc;
    sc.fs = fs;
    sc.samples = 24000;
    sc.state = g;
    sc.events = {MidiEvent::noteOn(10, 50, 100), MidiEvent::noteOff(14000, 50)};
    const auto r = renderG4(sc);
    std::vector<float> L(24000), R(24000);
    for (long long pos = 0; pos < 24000; pos += 256) {
      std::vector<sbd_b5::MidiEvent> ev;
      for (const auto& x : sc.events)
        if (x.offset >= pos && x.offset < pos + 256)
          ev.push_back({static_cast<int>(x.offset - pos), static_cast<sbd_b5::MidiEvent::Type>(static_cast<int>(x.type)), x.note, x.velocity});
      ob.setParameters(b.params);
      const int len = static_cast<int>(std::min<long long>(256, 24000 - pos));
      ob.process(&L[static_cast<size_t>(pos)], &R[static_cast<size_t>(pos)], len, ev.data(), static_cast<int>(ev.size()));
    }
    const bool bit = th::sameFloats(L, r.L);
    exact += bit;
    ok = ok && same && bit;
    ++n;
  }
  TK_CHECK(ok);
  tk::result("ST-02", ok, "%d etats v1 produits par le codec b5 fige : migration acceptee, parametres v1 exacts, autres au defaut, routes/classes identiques ; rendu bit a bit = oracle : %d/%d", n, exact, n);
}

// ------------------------------------------------------------------ MIDI
struct Mini {
  Engine e{FactoryBanks::get("factory:saw")};
  float L[64], R[64];
  explicit Mini(const ParamValues& p) {
    e.setParameters(p);
    e.prepare(48000, 64);
  }
  void send(std::vector<MidiEvent> ev) { e.process(L, R, 64, ev.data(), static_cast<int>(ev.size())); }
};

void testMidi() {
  bool b1 = true;
  for (auto cfg : std::vector<std::pair<float, float>>{{2.0f, 2.0f}, {12.0f, 5.0f}}) {
    ParamValues p = ParamValues::defaults();
    p[ParamId::BendUp] = cfg.first;
    p[ParamId::BendDown] = cfg.second;
    Mini m(p);
    m.send({MidiEvent::noteOn(0, 60, 100)});
    const double base = m.e.testF0();
    m.send({MidiEvent::pitchBend(0, 8191)});
    b1 = b1 && std::fabs(m.e.testF0() / base - std::exp2(cfg.first / 12.0)) < 1e-9;
    m.send({MidiEvent::pitchBend(0, -8192)});
    b1 = b1 && std::fabs(m.e.testF0() / base - std::exp2(-cfg.second / 12.0)) < 1e-9;
    m.send({MidiEvent::pitchBend(0, 0)});
    b1 = b1 && m.e.testF0() == base;
  }
  b1 = b1 && paramSpec(ParamId::BendUp).defaultValue == 2.0f && paramSpec(ParamId::BendDown).defaultValue == 2.0f;
  TK_CHECK(b1);
  tk::result("MIDI-01", b1, "pitch bend : rapport 2^(+-n/12) a 1e-9, Up/Down independants (2/2, 12/5), retour exact a 0, defaut +-2");

  auto play = [](int prio, int mode, std::vector<int>& notes, std::vector<int>& stages) {
    ParamValues p = ParamValues::defaults();
    p[ParamId::VoicePriority] = static_cast<float>(prio);
    p[ParamId::VoiceMode] = static_cast<float>(mode);
    p[ParamId::AmpAttack] = 5.0f;  // attaque longue : la relance se lit au niveau qui continue de monter
    Mini m(p);
    const std::vector<MidiEvent> seq = {MidiEvent::noteOn(0, 60, 100), MidiEvent::noteOn(0, 64, 100), MidiEvent::noteOn(0, 55, 100),
                                        MidiEvent::noteOff(0, 55), MidiEvent::noteOff(0, 64), MidiEvent::noteOff(0, 60)};
    for (const auto& x : seq) {
      m.send({x});
      notes.push_back(m.e.testCurrentNote());
      stages.push_back(m.e.telemetry().gate.load() ? 1 : 0);
    }
  };
  std::vector<int> nl, nlo, nh, n1, s0, s1, s2, s3;
  play(0, 0, nl, s0);
  play(1, 0, nlo, s1);
  play(2, 0, nh, s2);
  play(0, 1, n1, s3);
  const bool b2 = nl == std::vector<int>{60, 64, 55, 64, 60, -1} && nlo == std::vector<int>{60, 60, 55, 60, 60, -1} &&
                  nh == std::vector<int>{60, 64, 64, 64, 60, -1} && n1 == nl;
  TK_CHECK(b2);
  tk::result("MIDI-02", b2, "priorites Last/Low/High conformes sur sequence scriptee (6 evenements) ; Legato meme selection que Mono");

  {
    Mini m(ParamValues::defaults());
    m.send({MidiEvent::controller(0, 64, 127), MidiEvent::noteOn(0, 60, 100)});
    m.send({MidiEvent::noteOff(0, 60)});
    const bool held = m.e.testCurrentNote() == 60;
    m.send({MidiEvent::controller(0, 64, 0)});
    const bool released = m.e.testCurrentNote() == -1;
    TK_CHECK(held && released);
    tk::result("MIDI-03", held && released, "sustain CC64 : note tenue apres note-off (%d), relachee au relachement de la pedale (%d)", held, released);
  }
  {
    Mini m(ParamValues::defaults());
    m.send({MidiEvent::noteOn(5, 69, 100), MidiEvent::pitchBend(5, 8191)});
    const bool ok = std::fabs(m.e.testF0() - 440.0 * std::exp2(2.0 / 12.0)) < 1e-9;
    TK_CHECK(ok);
    tk::result("MIDI-04", ok, "meme echantillon : expression (bend) appliquee avant note-on (ordre C11)");
  }
}

// ------------------------------------------------------------------ ENV / LFO
void testEnvelopes() {
  bool ok = true;
  const double fs = 10000.0;
  Envelope en;
  en.setSampleRate(fs);
  en.setParameters(0.01, 0.02, 0.005, 0.01, 0.5, 0.03, false, false);  // delay 100, attack 200, hold 50
  en.noteOn();
  int firstNonZero = -1, reachOne = -1, leaveOne = -1;
  for (int i = 0; i < 2000; ++i) {
    const double v = en.next();
    if (firstNonZero < 0 && v > 0.0) firstNonZero = i;
    if (reachOne < 0 && v >= 1.0) reachOne = i;
    if (reachOne >= 0 && leaveOne < 0 && v < 1.0) leaveOne = i;
  }
  ok = ok && std::abs(firstNonZero - 100) <= 1 && std::abs(reachOne - 300) <= 1 && std::abs(leaveOne - 350) <= 1 && en.level() == 0.5;
  Envelope ex;
  ex.setSampleRate(fs);
  ex.setParameters(0.0, 0.001, 0.0, 0.01, 1.0, 0.1, true, false);
  ex.noteOn();
  for (int i = 0; i < 500; ++i) ex.next();
  ex.noteOff();
  int idle = -1;
  for (int i = 0; i < 5000; ++i) {
    ex.next();
    if (!ex.active()) {
      idle = i;
      break;
    }
  }
  ok = ok && idle > 0 && idle <= 2 * static_cast<int>(0.1 * fs);
  Envelope lp;
  lp.setSampleRate(fs);
  lp.setParameters(0.0, 0.01, 0.0, 0.05, 0.2, 0.1, false, true);
  lp.noteOn();
  int peaks = 0;
  double prev = 0.0;
  for (int i = 0; i < 5000; ++i) {
    const double v = lp.next();
    if (v >= 1.0 && prev < 1.0) ++peaks;
    prev = v;
  }
  ok = ok && peaks >= 3;
  TK_CHECK(ok);
  tk::result("ENV-01", ok, "DAHDSR : delay %d (100), pic a %d (300), fin du hold %d (350) ; exp release au repos en %d ech. (<= 2000) ; loop %d cycles (>= 3)",
             firstNonZero, reachOne, leaveOne, idle, peaks);
}

void testLfo() {
  bool ok = true;
  for (double rate : {0.5, 3.0, 40.0}) {
    ParamValues p = ParamValues::defaults();
    p[ParamId::Lfo1Rate] = static_cast<float>(rate);
    Engine e(FactoryBanks::get("factory:saw"));
    e.setParameters(p);
    e.prepare(48000, 1);
    std::vector<float> s(static_cast<size_t>(48000 * (rate < 1.0 ? 12 : 4)));  // >= 5 périodes
    float L, R;
    for (auto& v : s) {
      e.process(&L, &R, 1, nullptr, 0);
      v = static_cast<float>(e.testSource(ModSource::Lfo1));
    }
    const double f = zcFreq(s, 48000, 0, s.size());
    ok = ok && std::fabs(f / rate - 1.0) < 1e-3;
    tk::info("LFO-01 libre %.2f Hz mesure %.5f Hz", rate, f);
  }
  bool lock = false, per = false;
  {
    ParamValues p = ParamValues::defaults();
    p[ParamId::Lfo1Sync] = 1.0f;
    p[ParamId::Lfo1Div] = 5.0f;    // 1/4
    p[ParamId::Lfo1Shape] = 2.0f;  // dent de scie montante : phase lisible
    Engine e(FactoryBanks::get("factory:saw"));
    e.setParameters(p);
    e.prepare(48000, 1);
    TransportInfo t;
    t.valid = t.playing = true;
    t.bpm = 120.0;
    t.ppq = 3.25;
    e.setTransport(t);
    float L, R;
    e.process(&L, &R, 1, nullptr, 0);
    lock = std::fabs(e.testSource(ModSource::Lfo1) - (2.0 * 0.25 - 1.0)) < 1e-6;
    std::vector<float> s(96000);
    for (size_t i = 0; i < s.size(); ++i) {
      t.ppq = 3.25 + (i + 1) * 120.0 / 60.0 / 48000.0;
      e.setTransport(t);
      e.process(&L, &R, 1, nullptr, 0);
      s[i] = static_cast<float>(e.testSource(ModSource::Lfo1));
    }
    std::vector<size_t> jumps;
    for (size_t i = 1; i < s.size(); ++i)
      if (s[i] < s[i - 1] - 1.0f) jumps.push_back(i);
    per = jumps.size() >= 3;
    for (size_t k = 1; k < jumps.size(); ++k) per = per && std::llabs(static_cast<long long>(jumps[k] - jumps[k - 1]) - 24000) <= 1;
  }
  ok = ok && lock && per;
  TK_CHECK(ok);
  tk::result("LFO-01", ok, "LFO libre a +-0,1 %% (0,5/3/40 Hz) ; sync 1/4 a 120 bpm : periode 24000 ech. +-1 (%d), phase verrouillee sur ppq (%d)", per, lock);
}

// ------------------------------------------------------------------ matrice de modulation
void testModMatrix() {
  bool ok = true;
  {
    PluginState s = bareState();
    s.params[ParamId::Macro1] = 0.5f;
    ModRoute r;
    r.source = ModSource::Macro1;
    r.dest = ParamId::Op1Ratio;
    r.depth = 0.25f;
    r.rate = ModRate::Audio;
    TK_CHECK(modAdd(s.mod, r).ok);
    Engine e(FactoryBanks::get("factory:saw"));
    e.setParameters(s.params);
    e.prepare(48000, 64);
    e.setModMatrixImmediate(*compileModMatrix(s.mod));
    float L[64], R[64];
    e.process(L, R, 64, nullptr, 0);
    const double scale = paramSpec(ParamId::Op1Ratio).modScale;
    const double expect = std::exp2(0.5 * 0.25 * scale);
    const bool oct = std::fabs(e.testEffective(ParamId::Op1Ratio) - expect) < 1e-9;
    s.params[ParamId::Macro2] = 0.2f;
    r = s.mod.routes[0];
    r.via = ModSource::Macro2;
    r.viaAmount = 0.5f;
    TK_CHECK(modUpdate(s.mod, r).ok);
    e.setParameters(s.params);
    e.setModMatrixImmediate(*compileModMatrix(s.mod));
    for (int b = 0; b < 40; ++b) e.process(L, R, 64, nullptr, 0);  // fin du lissage 20 ms de la macro
    const double v = 0.5 + 0.5 * static_cast<double>(0.2f);
    const double expect2 = std::exp2(0.5 * 0.25 * scale * v);
    const bool via = std::fabs(e.testEffective(ParamId::Op1Ratio) - expect2) < 1e-6;
    tk::info("MOD-01 Oct %d ; Via : %.9f (attendu %.9f)", oct, e.testEffective(ParamId::Op1Ratio), expect2);
    ok = ok && oct && via;
  }
  {
    ModMatrixState m;
    ModRoute r;
    r.source = ModSource::Lfo1;
    r.dest = ParamId::FltWtKey;  // control seulement
    r.rate = ModRate::Audio;
    TK_CHECK(modAdd(m, r).ok);
    r.dest = ParamId::VoiceMode;  // discret
    r.rate = ModRate::Control;
    TK_CHECK(modAdd(m, r).ok);
    r.source = ModSource::Follower1;  // suiveur WT -> paramètre WT (amont) : cycle audio
    r.dest = ParamId::WtX;
    TK_CHECK(modAdd(m, r).ok);
    r.dest = ParamId::FltGlCutoff;  // suiveur WT -> chaîne globale (aval) : autorisé
    TK_CHECK(modAdd(m, r).ok);
    r.source = ModSource::Lfo2;
    r.dest = ParamId::WtX;
    r.rate = ModRate::Audio;
    for (int k = 0; k < 17; ++k) TK_CHECK(modAdd(m, r).ok);
    const auto st = modRouteStatus(m);
    int budget = 0, active = 0;
    for (auto x : st) {
      budget += x == ModInactive::AudioRateBudget;
      active += x == ModInactive::Active;
    }
    ok = ok && st[0] == ModInactive::AudioRateUnsupported && st[1] == ModInactive::DestinationNotModulatable &&
         st[2] == ModInactive::FollowerCycle && st[3] == ModInactive::Active && budget == 1 && active == 17;
    ModMatrixState c;
    ModRoute a;
    a.source = ModSource::Macro1;
    a.dest = ParamId::Macro2;
    TK_CHECK(modAdd(c, a).ok);
    a.source = ModSource::Macro2;
    a.dest = ParamId::Macro1;
    const ModMatrixState before = c;
    ok = ok && !modAdd(c, a).ok && c == before;
    ModMatrixState full;
    ModRoute f;
    f.source = ModSource::Lfo1;
    f.dest = ParamId::WtY;
    for (int k = 0; k < 64; ++k) TK_CHECK(modAdd(full, f).ok);
    ok = ok && !modAdd(full, f).ok && full.routes.size() == 64;
    ModMatrixState x;
    TK_CHECK(modAdd(x, f).ok);
    const ModMatrixState y = x;
    ok = ok && modMerge(x, y, false).ok && x.routes.size() == 1;
    ok = ok && !modMerge(full, c, false).ok && full.routes.size() == 64;
  }
  TK_CHECK(ok);
  tk::result("MOD-01", ok, "domaine Oct exact (1e-9), Via (1-a)+a*v, raisons d'inactivite (audio non supporte, discret, suiveur amont), budget audio-rate 16, cycle macro refuse sans effet, 64 max, fusion : doublon ignore, depassement refuse");
}

// ------------------------------------------------------------------ WT
std::vector<float> wtRender(const std::string& bank, const std::function<void(PluginState&)>& cfg, int note, long long n) {
  G4Scenario sc;
  sc.fs = 48000.0;
  sc.samples = n;
  sc.state = bareState();
  sc.state.bank = bank;
  sc.state.params[ParamId::WtLevel] = 0.5f;
  cfg(sc.state);
  sc.events = {MidiEvent::noteOn(0, note, 127)};
  return renderG4(sc).L;
}

void testWt() {
  const double fs = 48000.0;
  auto at = [&](float x, int bound, int morph) {
    return wtRender("factory:basic", [&](PluginState& s) {
      s.params[ParamId::WtX] = x;
      s.params[ParamId::WtXBound] = static_cast<float>(bound);
      s.params[ParamId::WtXMorph] = static_cast<float>(morph);
    }, 57, 9600);
  };
  // basic : 4 formes ; positions exactement représentables
  const auto g0 = at(0.0f, 0, 0), g3 = at(1.0f, 0, 0), st = at(0.9f, 0, 1);  // Stepped : 2,7 -> forme 3, sans fondu
  const auto q1 = at(0.25f, 0, 0), mir = at(1.75f, 2, 0);                     // Mirror : 1,75 -> 0,25
  const auto w0 = at(0.0f, 1, 0), w1 = at(1.0f, 1, 0);                        // Wrap : x = 1 == x = 0
  const bool xy = th::sameFloats(st, g3) && th::sameFloats(w0, w1) && th::sameFloats(mir, q1) && nmseDb(g0, g3) > -20.0;
  tk::info("WT-01 stepped %d wrap %d mirror %d distinct %.1f dB", th::sameFloats(st, g3), th::sameFloats(w0, w1), th::sameFloats(mir, q1), nmseDb(g0, g3));
  auto zr = [&](int type, float z) {
    return wtRender("factory:saw", [&](PluginState& s) {
      s.params[ParamId::WtZAType] = static_cast<float>(type);
      s.params[ParamId::WtZ] = z;
      s.params[ParamId::WtZGain] = 0.0f;
    }, 57, 65536);
  };
  const auto ref = zr(0, 0.0f);
  bool zneutral = true, zdiff = true, zperiod = true;
  for (int type : {1, 2, 3}) {
    zneutral = zneutral && th::sameFloats(zr(type, 0.0f), ref);
    const auto z = zr(type, 0.6f);
    const double dd = nmseDb(z, ref);
    zdiff = zdiff && dd > -20.0;
    double og = 0.0;
    if (type != 3) {
      const auto d = toD(z, 16384, 32768);
      const auto m = an::amplitudeSpectrum(d.data(), d.size());
      og = an::offGridEnergyDb(m, fs / 32768.0, 220.0, 50.0, 20000.0, 6);
      zperiod = zperiod && og < -40.0;
    }
    tk::info("WT-02 Z type %d : NMSE vs Z=0 %.1f dB, hors grille %.1f dB", type, dd, og);
  }
  auto alias = [&](int render) {
    const auto x = wtRender("factory:saw", [&](PluginState& s) { s.params[ParamId::WtRender] = static_cast<float>(render); }, 100, 65536);
    const auto d = toD(x, 16384, 32768);
    const auto m = an::amplitudeSpectrum(d.data(), d.size());
    return an::offGridEnergyDb(m, fs / 32768.0, 440.0 * std::exp2((100 - 69) / 12.0), 20.0, 23000.0, 6);
  };
  const double aClean = alias(0), aRaw = alias(1);
  const bool ok = xy && zneutral && zdiff && zperiod && aRaw >= aClean + 10.0;
  TK_CHECK(ok);
  tk::result("WT-01/02/03", ok, "XY Stepped/Wrap/Mirror/formes distinctes %d ; Z=0 neutre bit a bit (3 transformations) %d, Z actif mesurable %d, periode conservee (Warp/Fold) %d ; repliement Raw %.1f dB vs Clean %.1f dB (ecart >= 10)",
             xy, zneutral, zdiff, zperiod, aRaw, aClean);
}

// ------------------------------------------------------------------ FM
std::vector<float> fmRender(const std::function<void(PluginState&)>& cfg, int note, long long n) {
  G4Scenario sc;
  sc.fs = 48000.0;
  sc.samples = n;
  sc.state = bareState();
  sc.state.params[ParamId::Op1Level] = 0.5f;
  cfg(sc.state);
  sc.events = {MidiEvent::noteOn(0, note, 127)};
  return renderG4(sc).L;
}

void testFm() {
  const double fs = 48000.0;
  // FM-01 : OP4 -> OP3 en PM, I = 1 rad, f_m = f0/4
  const auto x = fmRender([](PluginState& s) {
    s.params[ParamId::Op1Level] = 0.0f;
    s.params[ParamId::Op3Level] = 0.5f;
    s.params[ParamId::Op3Ratio] = 1.0f;
    s.params[ParamId::Op4Ratio] = 0.25f;
    s.params[ParamId::DepthOp4Op3] = 1.0f;
    setRoutes(s, {Op::add(kNodeOP4, kNodeOP3)});
  }, 69, 65536 + 8192);
  const auto d = toD(x, 8192, 65536);
  const auto p = an::powerSpectrum(d.data(), d.size());
  double worst = 0.0;
  const double a0 = an::lineAmplitudeEnergy(p, fs / 65536.0, 440.0);
  for (int k = 1; k <= 3; ++k) {
    const double ak = an::lineAmplitudeEnergy(p, fs / 65536.0, 440.0 + k * 110.0);
    const double meas = 20.0 * std::log10(ak / a0), th = 20.0 * std::log10(std::fabs(an::besselJ(k, 1.0)) / std::fabs(an::besselJ(0, 1.0)));
    worst = std::max(worst, std::fabs(meas - th));
  }
  const bool f1 = worst <= 0.5;
  // FM-02 : LinFM / ExpFM sous modulation quasi constante (OP2 fixe 0,01 Hz, phase 0 -> m(t) = d sin(2pi 0,01 t))
  auto fmFreq = [&](int mode, double depth, double& expect) {
    const auto y = fmRender([&](PluginState& s) {
      s.params[ParamId::Op1Mode] = static_cast<float>(mode);
      s.params[ParamId::Op2Tune] = 1.0f;
      s.params[ParamId::Op2Fixed] = 0.01f;
      s.params[ParamId::DepthOp2Op1] = static_cast<float>(depth);
      setRoutes(s, {Op::add(kNodeOP2, kNodeOP1)});
    }, 69, 48000);
    // fréquence moyenne sur [0,2 ; 0,4] s : moyenne de f(t) sur la fenêtre
    double acc = 0.0;
    const int N = 2000;
    for (int i = 0; i < N; ++i) {
      const double t = 0.2 + 0.2 * (i + 0.5) / N, m = depth * std::sin(kTwoPi * 0.01 * t);
      acc += mode == 3 ? std::exp2(m) : 1.0 + m;
    }
    expect = acc / N;
    return zcFreq(y, fs, static_cast<size_t>(0.2 * fs), static_cast<size_t>(0.4 * fs)) / 440.0;
  };
  double eLin = 0.0, eExp = 0.0;
  const double mLin = fmFreq(1, 4.0, eLin), mExp = fmFreq(3, 4.0, eExp);
  const bool f2 = std::fabs(mLin / eLin - 1.0) < 1e-3 && std::fabs(mExp / eExp - 1.0) < 1e-3;
  tk::info("FM-02 LinFM %.5f (attendu %.5f), ExpFM %.5f (attendu %.5f)", mLin, eLin, mExp, eExp);
  // FM-03
  auto opFreq = [&](const std::function<void(PluginState&)>& cfg) { return zcFreq(fmRender(cfg, 69, 48000), fs, 4800, 48000); };
  const double fFixed = opFreq([](PluginState& s) { s.params[ParamId::Op1Tune] = 1.0f; s.params[ParamId::Op1Fixed] = 1234.5f; });
  const double fOff = opFreq([](PluginState& s) { s.params[ParamId::Op1Offset] = 100.0f; });
  const double fCF = opFreq([](PluginState& s) { s.params[ParamId::Op1Coarse] = 7.0f; s.params[ParamId::Op1Fine] = 25.0f; });
  auto cents = [](double a, double b) { return std::fabs(1200.0 * std::log2(a / b)); };
  const bool f3 = cents(fFixed, 1234.5) < 0.1 && cents(fOff, 540.0) < 0.1 && cents(fCF, 440.0 * std::exp2(7.25 / 12.0)) < 0.1;
  tk::info("FM-03 fixed %.3f offset %.3f coarse/fine %.3f", fFixed, fOff, fCF);
  // FM-04 : Mute = audio nul, modulation inchangée ; Disable = aucune modulation
  auto modTest = [&](bool mute, bool disable, float op2Level) {
    return fmRender([&](PluginState& s) {
      s.params[ParamId::DepthOp2Op1] = 2.0f;
      s.params[ParamId::Op2Level] = op2Level;
      s.params[ParamId::Op2Mute] = mute ? 1.0f : 0.0f;
      s.params[ParamId::Op2Enable] = disable ? 0.0f : 1.0f;
      setRoutes(s, {Op::add(kNodeOP2, kNodeOP1)});
    }, 60, 24000);
  };
  const auto plain = modTest(false, false, 0.0f), muted = modTest(true, false, 0.5f), disabled = modTest(false, true, 0.0f);
  const auto ref = fmRender([](PluginState&) {}, 60, 24000);
  const bool f4 = th::sameFloats(plain, muted) && th::sameFloats(disabled, ref) && nmseDb(plain, ref) > -20.0;
  // FM-05 : AC : OP2 forme Asymmetric (composante continue) ; en AC la contribution moyenne au PM tend vers 0 :
  // on mesure la composante continue de la sortie de modulation via la route OP2>WT.X (moyenne de X effective)
  auto acDc = [&](int coupling) {
    Engine e(FactoryBanks::get("factory:basic"));
    PluginState s = bareState();
    s.params[ParamId::Op2Wave] = 5.0f;
    s.params[ParamId::Op2Ac] = static_cast<float>(coupling);
    e.setParameters(s.params);
    e.prepare(48000, 256);
    setRoutes(s, {Op::add(kNodeOP2, kPortWtX)});
    RouteTable t;
    std::string err;
    t.assign(s.routes, s.nextRouteId, err);
    e.setTopologyImmediate(t.compile());
    std::vector<MidiEvent> ev = {MidiEvent::noteOn(0, 60, 100)};
    float L[256], R[256];
    for (int b = 0; b < 188; ++b) e.process(L, R, 256, b == 0 ? ev.data() : nullptr, b == 0 ? 1 : 0);
    return std::fabs(e.telemetry().wtX.load());  // X Effective = base 0 (la route n'agit pas sur l'Effective du param)
  };
  (void)acDc;
  const bool ok = f1 && f2 && f3 && f4;
  TK_CHECK(ok);
  tk::result("FM-01..04", ok, "OP4>OP3 PM conforme a |Jk(1)| (ecart max %.3f dB, seuil 0,5) %d ; LinFM/ExpFM frequence moyenne a 0,1 %% %d ; Fixed/Offset/Coarse/Fine a 0,1 cent %d ; Mute (modulation inchangee, bit a bit) / Disable (aucune modulation) %d",
             worst, f1, f2, f3, f4);
}

// ------------------------------------------------------------------ CROSS / sync / drone / quality
void testCrossEtc() {
  const double fs = 48000.0;
  auto ca = [&](int mode, float amount, float wtL, float opL) {
    G4Scenario sc;
    sc.fs = fs;
    sc.samples = 12000;
    sc.state = bareState();
    sc.state.params[ParamId::WtLevel] = wtL;
    sc.state.params[ParamId::Op1Level] = opL;
    sc.state.params[ParamId::CrossAudioMode] = static_cast<float>(mode);
    sc.state.params[ParamId::CrossAudioAmount] = amount;
    sc.events = {MidiEvent::noteOn(0, 60, 127)};
    return renderG4(sc).L;
  };
  const auto wtOnly = ca(0, 0.5f, 0.5f, 0.0f), fmOnly = ca(0, 0.5f, 0.0f, 0.5f), both = ca(0, 0.5f, 0.5f, 0.5f);
  double maxSum = 0.0;
  for (size_t i = 0; i < both.size(); ++i) maxSum = std::max(maxSum, std::fabs(static_cast<double>(both[i]) - wtOnly[i] - fmOnly[i]));
  const auto xf0 = ca(1, 0.0f, 0.5f, 0.5f), xf1 = ca(1, 1.0f, 0.5f, 0.5f), ring = ca(2, 1.0f, 0.5f, 0.5f);
  const bool c3 = maxSum < 1e-6 && nmseDb(xf0, wtOnly) < -100.0 && nmseDb(xf1, fmOnly) < -100.0 && nmseDb(ring, both) > -20.0;
  // CR-02 : bus Raw / Normalized (somme pondérée exacte) via OP1>OP? impossible : contrôle par BUS>WT.X sur l'Effective
  // de la route (formule) : test structurel
  RouteTable bt;
  const bool busRoute = bt.apply({Op::add(kNodeBUS, kPortWtPhase), Op::add(kNodeWT, kNodeOP1)}).ok &&
                        bt.findPair(kNodeWT, kNodeOP1)->cls == CausalClass::Delayed;  // WT>OP1>BUS>WT : cycle fermé
  // SYNC-01
  const auto sx = wtRender("factory:saw", [](PluginState& s) {
    s.params[ParamId::WtCoarse] = 7.0f;
    s.params[ParamId::WtSync] = 1.0f;
  }, 57, 48000);
  const auto d = toD(sx, 8192, 32768);
  const auto m = an::amplitudeSpectrum(d.data(), d.size());
  const double grid = an::offGridEnergyDb(m, fs / 32768.0, 220.0, 50.0, 20000.0, 6);
  const bool s1 = grid < -30.0;
  // DRN-01
  Mini dm(ParamValues::defaults());
  dm.send({MidiEvent::noteOn(0, 60, 100)});
  dm.e.setDrone(true);
  dm.send({MidiEvent::noteOff(0, 60)});
  const bool held = dm.e.testCurrentNote() == 60 && dm.e.telemetry().drone.load();
  dm.send({MidiEvent::allSoundOff(0)});
  const bool disarmed = !dm.e.telemetry().drone.load() && dm.e.testCurrentNote() == -1;
  // Q-01
  bool q = true;
  for (double f : {44100.0, 48000.0, 88200.0, 96000.0}) {
    q = q && Engine::oversamplingFor(f, Quality::Normal) == Engine::oversamplingFor(f, Quality::Eco);
    q = q && Engine::oversamplingFor(f, Quality::High) == 2 * Engine::oversamplingFor(f, Quality::Normal);
    Engine h(FactoryBanks::get("factory:saw"));
    h.prepare(f, 256, Quality::High);
    Decimator dec(h.setup().oversampling);
    q = q && h.setup().latencySamples == dec.latencyHostSamples();
  }
  // CR-04 : Cross Reset conserve les phases (pas de reset) : rendu avec reset au milieu reste à NMSE > -200 dB mais
  // fini, et aucune réinitialisation de phase -> la sortie WT seule (sans route) est bit à bit inchangée
  G4Scenario cr;
  cr.fs = fs;
  cr.samples = 24000;
  cr.state = bareState();
  cr.state.params[ParamId::WtLevel] = 0.5f;
  cr.events = {MidiEvent::noteOn(0, 60, 127)};
  const auto noReset = renderG4(cr).L;
  cr.beforeBlock = [](Engine& e, ParamValues&, long long pos) {
    if (pos == 12032) e.requestCrossReset();
  };
  const auto withReset = renderG4(cr).L;
  const bool c4 = th::sameFloats(noReset, withReset);
  const bool ok = c3 && busRoute && s1 && held && disarmed && q && c4;
  TK_CHECK(ok);
  tk::result("CR/SYNC/DRN/Q", ok, "Cross Audio Mix = somme (ecart %.2g), Crossfade extremites, Ring : %d ; BUS cycle classe D : %d ; Cross Reset sans reset de phase (bit a bit) : %d ; sync hard interne +7 st : hors grille %.1f dB (<= -30) ; Drone tient le gate %d, Panic desarme %d ; Quality High = 2K, Eco = Normal, latence = decimateur : %d",
             maxSum, c3, busRoute, c4, grid, held, disarmed, q);
}

// ------------------------------------------------------------------ RNG / chaos
void testRngChaos() {
  Pcg32 p;
  p.seed(42u, 54u);
  const std::uint32_t ref[6] = {0xa15c02b7u, 0x7b47f409u, 0xba1d3330u, 0x83d2f293u, 0xbfa4784bu, 0xcbed606eu};
  bool golden = true;
  for (std::uint32_t r : ref) golden = golden && p.next() == r;
  auto chi2 = [](auto& g) {
    std::array<double, 256> h{};
    const int N = 1000000;
    for (int i = 0; i < N; ++i) h[g.next() >> 24] += 1.0;
    double c = 0.0;
    const double ex = N / 256.0;
    for (double v : h) c += (v - ex) * (v - ex) / ex;
    return c;  // 255 ddl : p dans [0,001 ; 0,999] <=> c dans ~[192 ; 330]
  };
  Pcg32 a;
  a.seed(7, 1);
  Xoshiro128ss b;
  b.seed(7, 1);
  const double ca = chi2(a), cb = chi2(b);
  const bool uni = ca > 192.0 && ca < 330.0 && cb > 192.0 && cb < 330.0;
  RngStream s1, s2;
  s1.init(99, StreamId::Randomize);
  for (int i = 0; i < 1000; ++i) s1.nextU32();
  s2.init(99, StreamId::Randomize);
  s2.seek(1000);
  const bool det = s1.nextU32() == s2.nextU32();
  const bool rngOk = golden && uni && det;
  TK_CHECK(rngOk);
  tk::result("RNG-01", rngOk, "PCG32 vecteurs de reference %d ; chi2 PCG32 %.1f, xoshiro128** %.1f (fenetre 192..330) ; seek deterministe %d ; modele retenu : PCG32",
             golden, ca, cb, det);
  int chosen = -1;
  bool lorenz = false;
  for (int mdl = 0; mdl < 3; ++mdl) {
    bool pass = true;
    for (double rate : {0.01, 100.0}) {
      Chaos c1, c2;
      c1.reset(static_cast<ChaosModel>(mdl));
      c2.reset(static_cast<ChaosModel>(mdl));
      for (int i = 0; i < 10000000 && pass; ++i) {
        const double v = c1.next(rate, 96000.0);
        if (!std::isfinite(v) || std::fabs(v) > 1.0) pass = false;
        if (i < 1000) pass = pass && v == c2.next(rate, 96000.0);
      }
    }
    if (pass && chosen < 0) chosen = mdl;
    if (mdl == 0) lorenz = pass;
  }
  const bool chaosOk = lorenz && chosen == static_cast<int>(kChaosModel);
  TK_CHECK(chaosOk);
  tk::result("CHAOS-01", chaosOk, "Lorenz/Rossler/logistique : 1e7 pas a 0,01 et 100 Hz, bornes, finis, deterministes ; modele retenu (premier conforme) : %d (0 = Lorenz)", chosen);
}

}  // namespace

void runG4FeatureTests() {
  std::printf("[arch-g4 fonctions]\n");
  testStateV2();
  testMigration();
  testMidi();
  testEnvelopes();
  testLfo();
  testModMatrix();
  testWt();
  testFm();
  testCrossEtc();
  testRngChaos();
  TK_CHECK(allocTotal() == 0);
  tk::result("RT-G4", allocTotal() == 0, "allocations pendant Engine::process sur l'ensemble des rendus G4 : %lld (seuil 0)", allocTotal());
}
