// SUBDESTRUCKT — tests ARCH-G4 du cœur. Critères et seuils préenregistrés : docs/V1-ARCH-G4-TRACKER.md §3.
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Analysis.h"
#include "RenderHelper.h"
#include "TestKit.h"
#include "core/ModMatrix.h"
#include "core/StateCodec.h"
#include "dsp/ModSources.h"
#include "dsp/Rng.h"
#include "engine/Engine.h"
#include "regression/b5/Engine.h"

using namespace sbd;
using Op = RouteTable::Op;

namespace {

// ------------------------------------------------------------------ rendu générique (nouveau moteur)
struct G4Scenario {
  double fs = 48000.0;
  int block = 256;
  std::vector<int> blockSizes;
  long long samples = 48000;
  PluginState state = defaultState();
  std::vector<MidiEvent> events;  // offsets absolus
  std::function<void(Engine&, ParamValues&, long long)> beforeBlock;
  TransportInfo transport;
  Quality quality = Quality::Normal;
};
struct G4Render {
  std::vector<float> L, R;
  long long allocations = 0;
  std::uint32_t nonFinite = 0, clamp = 0;
  EngineSetup setup;
};

G4Render renderG4(const G4Scenario& s) {
  G4Render r;
  auto bank = FactoryBanks::get(s.state.bank);
  Engine eng(bank ? bank : FactoryBanks::get("factory:saw"));
  ParamValues p = s.state.params;
  eng.setParameters(p);
  int maxBlock = s.block;
  for (int b : s.blockSizes) maxBlock = std::max(maxBlock, b);
  eng.setSeed(s.state.seed);
  eng.setLfoCurves(s.state.lfoCurves);
  eng.prepare(s.fs, std::min(maxBlock, 512), s.quality);
  RouteTable t;
  std::string err;
  TK_CHECK(t.assign(s.state.routes, s.state.nextRouteId, err));
  eng.setTopologyImmediate(t.compile());
  auto cm = compileModMatrix(s.state.mod);
  eng.setModMatrixImmediate(*cm);
  r.setup = eng.setup();
  r.L.assign(static_cast<size_t>(s.samples), 0.0f);
  r.R.assign(static_cast<size_t>(s.samples), 0.0f);
  std::vector<MidiEvent> ev;
  ev.reserve(256);
  long long pos = 0;
  size_t bi = 0;
  TransportInfo tr = s.transport;
  while (pos < s.samples) {
    int n = s.blockSizes.empty() ? s.block : s.blockSizes[bi++ % s.blockSizes.size()];
    n = static_cast<int>(std::min<long long>(n, s.samples - pos));
    if (s.beforeBlock) s.beforeBlock(eng, p, pos);
    ev.clear();
    for (const auto& e : s.events)
      if (e.offset >= pos && e.offset < pos + n) {
        MidiEvent x = e;
        x.offset = static_cast<int>(e.offset - pos);
        ev.push_back(x);
      }
    eng.setParameters(p);
    eng.setTransport(tr);
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    eng.process(&r.L[static_cast<size_t>(pos)], &r.R[static_cast<size_t>(pos)], n, ev.data(), static_cast<int>(ev.size()));
    tk::countingAllocations().store(false);
    r.allocations += tk::allocationCount().load();
    eng.exchange().collectGarbage();
    eng.modExchange().collectGarbage();
    eng.bankExchange().collectGarbage();
    if (tr.valid) tr.ppq += n * tr.bpm / 60.0 / s.fs;
    pos += n;
  }
  r.nonFinite = eng.telemetry().nonFiniteResets.load();
  r.clamp = eng.telemetry().outputClampHits.load();
  return r;
}

double peak(const std::vector<float>& x, size_t a = 0, size_t b = 0) {
  if (b == 0 || b > x.size()) b = x.size();
  double p = 0.0;
  for (size_t i = a; i < b; ++i) p = std::max(p, static_cast<double>(std::fabs(x[i])));
  return p;
}
bool allFinite(const std::vector<float>& x) {
  for (float v : x)
    if (!std::isfinite(v)) return false;
  return true;
}
double nmseDb(const std::vector<float>& a, const std::vector<float>& ref) {
  double num = 0.0, den = 0.0;
  for (size_t i = 0; i < a.size(); ++i) {
    const double d = static_cast<double>(a[i]) - ref[i];
    num += d * d;
    den += static_cast<double>(ref[i]) * ref[i];
  }
  if (num == 0.0) return -400.0;
  return 10.0 * std::log10(num / std::max(den, 1e-300));
}
double rms(const std::vector<float>& x, size_t a, size_t b) {
  double s = 0.0;
  for (size_t i = a; i < b; ++i) s += static_cast<double>(x[i]) * x[i];
  return std::sqrt(s / std::max<size_t>(1, b - a));
}

// ------------------------------------------------------------------ REG-ID : registre figé
void testRegistry() {
  bool ok = true;
  std::ifstream in("tests/core/expected_param_ids.txt");
  if (!in) in.open("../tests/core/expected_param_ids.txt");
  std::vector<std::string> ids;
  std::string line;
  while (std::getline(in, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::istringstream ls(line);
    std::string id, type;
    int ver = 0;
    ls >> id >> type >> ver;
    ids.push_back(id);
    if (static_cast<int>(ids.size()) <= kNumParams) {
      const auto& sp = paramSpec(static_cast<int>(ids.size()) - 1);
      static const char* kTypes[] = {"Float", "Choice", "Bool", "Int"};
      ok = ok && id == sp.stableId && type == kTypes[static_cast<int>(sp.type)] && ver == sp.version;
    }
  }
  ok = ok && static_cast<int>(ids.size()) == kNumParams;
  // unicité des StableID
  std::vector<std::string> sorted(ids);
  std::sort(sorted.begin(), sorted.end());
  ok = ok && std::adjacent_find(sorted.begin(), sorted.end()) == sorted.end();
  // les 22 paramètres v1 identiques à l'oracle 0.1.0-b5 (StableID, plage, défaut, lissage, unité)
  bool v1 = kNumV1Params == sbd_b5::kNumParams;
  for (int i = 0; i < kNumV1Params && v1; ++i) {
    const auto& a = paramSpec(i);
    const auto& b = sbd_b5::paramSpec(i);
    v1 = std::strcmp(a.stableId, b.stableId) == 0 && a.minValue == b.minValue && a.maxValue == b.maxValue &&
         a.defaultValue == b.defaultValue && a.smoothingMs == b.smoothingMs && std::strcmp(a.unit, b.unit) == 0 &&
         a.version == 1;
  }
  // métadonnées obligatoires (C26) : plages cohérentes, défaut dans la plage, discrets entiers
  bool meta = true;
  for (int i = 0; i < kNumParams; ++i) {
    const auto& s = paramSpec(i);
    meta = meta && s.minValue < s.maxValue && s.defaultValue >= s.minValue && s.defaultValue <= s.maxValue &&
           s.hardMin <= s.minValue && s.hardMax >= s.maxValue && s.unit != nullptr && s.group != nullptr && s.group[0];
    if (s.type != ParamType::Float) meta = meta && s.defaultValue == std::round(s.defaultValue) && s.rates == kRateEvent;
    if (s.type == ParamType::Choice) meta = meta && choiceCount(static_cast<ParamId>(i)) == static_cast<int>(s.maxValue) + 1;
  }
  TK_CHECK(ok && v1 && meta);
  tk::result("REG-ID", ok && v1 && meta,
             "%d StableID figes (liste de reference, ordre, type, version), uniques ; 22 parametres v1 identiques a "
             "l'oracle b5 : %s ; metadonnees C26 coherentes : %s",
             kNumParams, v1 ? "oui" : "NON", meta ? "oui" : "NON");
}

// ------------------------------------------------------------------ REG-01 : non-régression 0.1.0-b5 (oracle figé)
struct OracleCase {
  const char* name;
  std::vector<std::pair<int, int>> topo;  // (src, dst) en nœuds b5
  bool allDepths = false;
  bool automation = false;
  bool panic = false;
  bool variableBlocks = false;
};

void testRegression() {
  const std::vector<OracleCase> cases = {
      {"preset minimal", {{2, 1}, {0, 1}, {1, 0}}},
      {"riche (6 routes, D)", {{2, 1}, {0, 1}, {1, 0}, {1, 2}, {1, 1}, {2, 0}}, true},
      {"automation + CROSS->0", {{2, 1}, {0, 1}, {1, 0}}, false, true},
      {"panic", {{2, 1}, {0, 1}, {1, 0}, {2, 2}}, false, false, true},
      {"blocs variables", {{2, 1}, {0, 1}, {1, 0}}, false, true, false, true},
  };
  bool ok = true;
  double worst = -400.0, worstAbs = 0.0;
  int exact = 0, total = 0;
  for (double fs : {44100.0, 48000.0, 88200.0, 96000.0})
    for (const auto& c : cases) {
      const long long N = static_cast<long long>(fs * 1.2);
      std::vector<MidiEvent> ev = {MidiEvent::noteOn(100, 48, 110), MidiEvent::noteOn(static_cast<int>(fs * 0.3), 55, 70),
                                   MidiEvent::noteOff(static_cast<int>(fs * 0.5), 55), MidiEvent::noteOff(static_cast<int>(fs * 0.7), 48)};
      if (c.panic) ev.push_back(MidiEvent::allSoundOff(static_cast<int>(fs * 0.4)));
      std::vector<int> blocks = c.variableBlocks ? std::vector<int>{1, 7, 64, 333, 512, 128} : std::vector<int>{256};
      auto automate = [&](float* v, long long pos) {
        if (!c.automation) return;
        const double t = static_cast<double>(pos) / fs;
        v[static_cast<int>(ParamId::Op2Ratio)] = static_cast<float>(1.5 + 0.5 * std::sin(6.0 * t));
        v[static_cast<int>(ParamId::DepthOp2Op1)] = static_cast<float>(1.0 + std::sin(3.0 * t));
        v[static_cast<int>(ParamId::WtLevel)] = static_cast<float>(0.4 + 0.2 * std::sin(5.0 * t));
        if (t > 0.6) v[static_cast<int>(ParamId::CrossMaster)] = 0.0f;
      };
      // oracle b5
      sbd_b5::Engine ob(std::make_shared<const sbd_b5::SawWavetable>());
      sbd_b5::ParamValues bp = sbd_b5::ParamValues::defaults();
      if (c.allDepths)
        for (int k = 0; k < sbd_b5::kMaxRoutes; ++k) bp[sbd_b5::routeSlotInfo(k).depthParam] = 1.0f;
      ob.setParameters(bp);
      ob.prepare(fs, 512);
      sbd_b5::RouteTable bt;
      std::vector<sbd_b5::RouteTable::Op> bops;
      for (auto pr : c.topo) bops.push_back(sbd_b5::RouteTable::Op::add(pr.first, pr.second));
      TK_CHECK(bt.apply(bops).ok);
      ob.setTopologyImmediate(bt.compile());
      // nouveau moteur, état migré équivalent
      Engine ng(FactoryBanks::get("factory:saw"));
      ParamValues np = ParamValues::defaults();
      for (int k = 0; k < kNumV1Params; ++k) np.v[static_cast<size_t>(k)] = bp.v[static_cast<size_t>(k)];
      ng.setParameters(np);
      ng.prepare(fs, 512);
      RouteTable nt;
      std::vector<Op> nops;
      for (auto pr : c.topo) nops.push_back(Op::add(pr.first, pr.second));
      TK_CHECK(nt.apply(nops).ok);
      ng.setTopologyImmediate(nt.compile());
      ok = ok && ob.setup().latencySamples == ng.setup().latencySamples;
      std::vector<float> aL(static_cast<size_t>(N)), aR(static_cast<size_t>(N)), bL(static_cast<size_t>(N)), bR(static_cast<size_t>(N));
      long long pos = 0;
      size_t bi = 0;
      std::vector<sbd_b5::MidiEvent> e5;
      std::vector<MidiEvent> e4;
      while (pos < N) {
        const int n = static_cast<int>(std::min<long long>(blocks[bi++ % blocks.size()], N - pos));
        automate(bp.v.data(), pos);
        for (int k = 0; k < kNumV1Params; ++k) np.v[static_cast<size_t>(k)] = bp.v[static_cast<size_t>(k)];
        e5.clear();
        e4.clear();
        for (const auto& x : ev)
          if (x.offset >= pos && x.offset < pos + n) {
            MidiEvent y = x;
            y.offset = static_cast<int>(x.offset - pos);
            e4.push_back(y);
            sbd_b5::MidiEvent z{y.offset, static_cast<sbd_b5::MidiEvent::Type>(static_cast<int>(y.type)), y.note, y.velocity};
            e5.push_back(z);
          }
        ob.setParameters(bp);
        ng.setParameters(np);
        ob.process(&bL[static_cast<size_t>(pos)], &bR[static_cast<size_t>(pos)], n, e5.data(), static_cast<int>(e5.size()));
        ng.process(&aL[static_cast<size_t>(pos)], &aR[static_cast<size_t>(pos)], n, e4.data(), static_cast<int>(e4.size()));
        pos += n;
      }
      const double d = nmseDb(aL, bL);
      double mx = 0.0;
      for (size_t i = 0; i < aL.size(); ++i) mx = std::max(mx, static_cast<double>(std::fabs(aL[i] - bL[i])));
      const bool lr = th::sameFloats(aL, aR);
      const bool caseOk = d <= -100.0 && mx <= 1e-5 && lr && peak(bL) > 1e-3;
      if (!caseOk) tk::info("REG-01 ECHEC %s fs=%.0f : NMSE %.1f dB, max %.3g, L==R %d", c.name, fs, d, mx, lr);
      ok = ok && caseOk;
      worst = std::max(worst, d);
      worstAbs = std::max(worstAbs, mx);
      exact += th::sameFloats(aL, bL);
      ++total;
    }
  TK_CHECK(ok);
  tk::result("REG-01", ok,
             "non-regression 0.1.0-b5 contre l'oracle fige (5 scenarios x 4 frequences) : NMSE max %.1f dB (seuil -100), "
             "ecart max %.3g (seuil 1e-5), L==R, latence identique ; %d/%d rendus identiques bit a bit",
             worst, worstAbs, exact, total);
}

}  // namespace

void runG4Tests() {
  std::printf("[arch-g4]\n");
  testRegistry();
  testRegression();
}
