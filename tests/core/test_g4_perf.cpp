// SUBDESTRUCKT — benchmark CPU reproductible ARCH-G4 (PERF-01, PERF-02). Mesures rapportées SANS seuil : le budget
// CPU produit n'est pas normatif (arbitrage ouvert). Une mesure sur VM CI ou sur PC de développement ne certifie ni
// le M3 ni le M4 Max. Facteur temps réel = durée de traitement / durée audio (mono-instance, un cœur).
#include <chrono>
#include <functional>
#include <cstdio>
#include <string>
#include <vector>

#include "G4Helper.h"
#include "TestKit.h"

using namespace sbd;
using namespace g4;
using Op = RouteTable::Op;

namespace {

double measure(const PluginState& st, double fs, Quality q, double seconds) {
  Engine eng(FactoryBanks::get(st.bank));
  eng.setParameters(st.params);
  eng.prepare(fs, 256, q);
  RouteTable t;
  std::string err;
  t.assign(st.routes, st.nextRouteId, err);
  eng.setTopologyImmediate(t.compile());
  eng.setModMatrixImmediate(*compileModMatrix(st.mod));
  std::vector<float> L(256), R(256);
  const int blocks = static_cast<int>(seconds * fs / 256);
  MidiEvent on = MidiEvent::noteOn(0, 45, 100);
  eng.process(L.data(), R.data(), 256, &on, 1);
  const auto t0 = std::chrono::steady_clock::now();
  for (int b = 0; b < blocks; ++b) {
    eng.setParameters(st.params);
    eng.process(L.data(), R.data(), 256, nullptr, 0);
  }
  const double el = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  return el / (blocks * 256.0 / fs);
}

PluginState stressState() {
  PluginState s = defaultState();
  auto& p = s.params;
  s.bank = "factory:harmonic";
  // 5 oscillateurs actifs, 55 routes (classes B-T2 par la transaction)
  p[ParamId::Op2Level] = 0.2f;
  p[ParamId::Op3Level] = 0.2f;
  p[ParamId::Op4Level] = 0.2f;
  p[ParamId::Op3Wave] = 8.0f;  // Wavetable
  p[ParamId::Op4Wave] = 6.0f;
  p[ParamId::Op2Mode] = 1.0f;
  p[ParamId::Op3Mode] = 2.0f;
  p[ParamId::Op4Mode] = 3.0f;
  std::vector<Op> ops;
  for (int k = 0; k < kMaxRoutes; ++k) ops.push_back(Op::add(routeSlotInfo(k).src, routeSlotInfo(k).dst));
  RouteTable t;
  t.apply(ops);
  s.routes = t.routes();
  s.nextRouteId = t.nextRouteId();
  for (int k = 0; k < kMaxRoutes; ++k) p[routeSlotInfo(k).depthParam] = paramSpec(routeSlotInfo(k).depthParam).minValue < 0.0f ? 0.05f : 0.2f;
  // WT : XY, Z deux emplacements, Character
  p[ParamId::WtX] = 0.37f;
  p[ParamId::WtY] = 0.61f;
  p[ParamId::WtZ] = 0.4f;
  p[ParamId::WtZAType] = 1.0f;
  p[ParamId::WtZBType] = 2.0f;
  p[ParamId::WtChar] = 0.2f;
  p[ParamId::WtInterp] = 1.0f;
  // 3 filtres (F4 16 modes en global) et 6 processeurs (dont S2 4096)
  p[ParamId::FltWtType] = 1.0f;
  p[ParamId::FltFmType] = 3.0f;
  p[ParamId::FltGlType] = 4.0f;
  p[ParamId::FltGlModes] = 16.0f;
  const float types[6] = {1, 2, 3, 4, 7, 8};
  for (int slot = 0; slot < 6; ++slot) {
    const int b = idx(ParamId::DstWt1Type) + slot * (idx(ParamId::DstWt2Type) - idx(ParamId::DstWt1Type));
    p.v[static_cast<size_t>(b)] = types[slot];
    p.v[static_cast<size_t>(b + 49)] = 3.0f;  // S2 FFT 4096
  }
  // 64 modulations dont 16 audio-rate
  const ParamId dests[8] = {ParamId::WtX, ParamId::WtY, ParamId::WtZ, ParamId::FltWtCutoff, ParamId::FltGlCutoff, ParamId::Op1Ratio, ParamId::CrossMaster, ParamId::DepthOp2Op1};
  for (int k = 0; k < 64; ++k) {
    ModRoute r;
    r.source = static_cast<ModSource>(static_cast<int>(ModSource::Lfo1) + (k % 7));
    r.dest = dests[k % 8];
    r.depth = 0.01f;
    r.rate = k < 16 ? ModRate::Audio : ModRate::Control;
    r.curve = static_cast<ModCurve>(k % 6);
    modAdd(s.mod, r);
  }
  return s;
}

}  // namespace

void runPerfTests() {
  std::printf("[arch-g4 performance]\n");
  const PluginState def = defaultState(), stress = stressState();
  std::string line;
  char buf[160];
  for (double fs : {44100.0, 48000.0, 96000.0})
    for (Quality q : {Quality::Normal, Quality::High}) {
      const double a = measure(def, fs, q, 2.0), b = measure(stress, fs, q, 2.0);
      std::snprintf(buf, sizeof buf, " [%.1fk %s : init %.4f, stress %.4f]", fs / 1000.0, qualityName(q), a, b);
      line += buf;
    }
  tk::result("PERF-01", true, "facteur temps reel mono-instance (mesure, sans seuil normatif ; machine de test, pas M3/M4) :%s", line.c_str());
  // PERF-02 : coût relatif des modes de modulation des opérateurs (benchmark ExpFM)
  std::string modes;
  const char* names[4] = {"PM", "LinFM", "TZFM", "ExpFM"};
  for (int m = 0; m < 4; ++m) {
    PluginState s = defaultState();
    for (ParamId mp : {ParamId::Op1Mode, ParamId::Op2Mode, ParamId::Op3Mode, ParamId::Op4Mode}) s.params[mp] = static_cast<float>(m);
    s.params[ParamId::Op3Level] = 0.2f;
    std::vector<Op> ops;
    for (int sidx = 1; sidx <= 4; ++sidx)
      for (int d = 1; d <= 4; ++d) ops.push_back(Op::add(sidx, d));
    RouteTable t;
    t.apply(ops);
    s.routes = t.routes();
    s.nextRouteId = t.nextRouteId();
    std::snprintf(buf, sizeof buf, " %s %.4f", names[m], measure(s, 48000.0, Quality::Normal, 2.0));
    modes += buf;
  }
  tk::result("PERF-02", true, "matrice FM 16 routes a 48 kHz, facteur temps reel par mode (benchmark ExpFM, sans seuil) :%s", modes.c_str());
  // PERF-03 (ARCH-G4.1) : Eco / Normal / High mesurés séparément (48 kHz, état stress), et coût marginal par composant
  // (état initial + un composant ; facteur temps réel x 1000 = millièmes du temps réel). Sans seuil, machine de test.
  std::string q3;
  for (Quality q : {Quality::Eco, Quality::Normal, Quality::High}) {
    std::snprintf(buf, sizeof buf, " %s init %.4f stress %.4f ;", qualityName(q), measure(def, 48000.0, q, 2.0), measure(stress, 48000.0, q, 2.0));
    q3 += buf;
  }
  struct Comp {
    const char* name;
    std::function<void(PluginState&)> f;
  };
  const std::vector<Comp> comps = {
      {"WT morph spectral", [](PluginState& s) { s.bank = "factory:harmonic"; s.params[ParamId::WtXMorph] = 2.0f; s.params[ParamId::WtX] = 0.4f; }},
      {"WT morph spectral X module (LFO 2 Hz)", [](PluginState& s) {
         s.bank = "factory:harmonic";
         s.params[ParamId::WtXMorph] = 2.0f;
         s.params[ParamId::Lfo1Rate] = 2.0f;
         ModRoute r;
         r.source = ModSource::Lfo1;
         r.dest = ParamId::WtX;
         r.depth = 0.5f;
         modAdd(s.mod, r);
       }},
      {"WT Z (fold+warp)", [](PluginState& s) { s.params[ParamId::WtZAType] = 1.0f; s.params[ParamId::WtZBType] = 2.0f; s.params[ParamId::WtZ] = 0.5f; }},
      {"N1 (OS interne)", [](PluginState& s) { s.params[ParamId::DstGl1Type] = 1.0f; }},
      {"S2 FFT 4096", [](PluginState& s) { s.params[ParamId::DstGl1Type] = 8.0f; s.params[ParamId::DstGl1S2Fft] = 3.0f; }},
      {"F4 modal 16", [](PluginState& s) { s.params[ParamId::FltGlType] = 4.0f; s.params[ParamId::FltGlModes] = 16.0f; }},
      {"16 mod audio-rate", [](PluginState& s) {
         for (int k = 0; k < 16; ++k) {
           ModRoute r;
           r.source = ModSource::Lfo1;
           r.dest = k % 2 ? ParamId::WtX : ParamId::FltGlCutoff;
           r.depth = 0.01f * (k + 1);
           r.rate = ModRate::Audio;
           modAdd(s.mod, r);
         }
       }},
      {"ExpFM 4 op", [](PluginState& s) {
         for (ParamId mp : {ParamId::Op1Mode, ParamId::Op2Mode, ParamId::Op3Mode, ParamId::Op4Mode}) s.params[mp] = 3.0f;
         s.params[ParamId::Op2Level] = s.params[ParamId::Op3Level] = s.params[ParamId::Op4Level] = 0.2f;
       }},
  };
  std::string c3;
  for (Quality q : {Quality::Eco, Quality::Normal, Quality::High}) {
    const double b0 = measure(def, 48000.0, q, 1.0);
    c3 += std::string(" [") + qualityName(q) + " base " + std::to_string(static_cast<int>(1000 * b0 + 0.5));
    for (const auto& c : comps) {
      PluginState s = def;
      c.f(s);
      std::snprintf(buf, sizeof buf, ", %s +%.1f", c.name, 1000.0 * (measure(s, 48000.0, q, 1.0) - b0));
      c3 += buf;
    }
    c3 += "]";
  }
  tk::result("PERF-03", true, "48 kHz, facteur temps reel par Quality :%s cout marginal (millièmes du temps reel) :%s", q3.c_str(), c3.c_str());
}
