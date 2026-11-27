// SUBDESTRUCKT — tests ARCH-G4.3 (DSP FINAL LOCK). Critères : docs/V1-ARCH-G4.3-SPEC.md (écrits avant mesure).
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Analysis.h"
#include "G4Helper.h"
#include "TestKit.h"
#include "core/StateCodec.h"
#include "dsp/Destruction.h"
#include "dsp/Elp.h"

using namespace sbd;
using namespace g4;

namespace {

// ------------------------------------------------------------------ empreintes de référence 0.3.1 (ZC-03, MIG-02)
// Configurations construites uniquement avec les 695 paramètres de 0.3.1 ; les paramètres ajoutés en G4.3 (s'ils
// existent) reçoivent la valeur de migration d'un état v3 (D2 LEGACY) et une valeur Shape aléatoire (ignorée par les
// transformations existantes). Le tirage des 695 paramètres ne dépend pas de ces ajouts (générateurs séparés).
void setIfExists(PluginState& s, const char* sid, float v) {
  ParamId id;
  if (findParam(sid, id)) s.params[id] = v;
}

PluginState goldenConfig(int k) {
  std::mt19937_64 rng(0x6A3C0000ULL + static_cast<std::uint64_t>(k));
  std::uniform_real_distribution<double> u(0.0, 1.0);
  PluginState st = defaultState();
  const char* banks[] = {"factory:basic", "factory:pwm", "factory:harmonic", "factory:metal", "factory:fold", "factory:formant"};
  if (k < 10 || k >= 16) {  // Z existant (types 1..3), morph, banque
    st.bank = banks[k % 6];
    st.params[ParamId::WtZ] = static_cast<float>(0.2 + 0.8 * u(rng));
    st.params[ParamId::WtZAType] = static_cast<float>(1 + static_cast<int>(u(rng) * 3.0) % 3);
    st.params[ParamId::WtZBType] = static_cast<float>(static_cast<int>(u(rng) * 4.0) % 4);
    st.params[ParamId::WtZADepth] = static_cast<float>(u(rng));
    st.params[ParamId::WtZBDepth] = static_cast<float>(u(rng));
    st.params[ParamId::WtZACurve] = static_cast<float>(static_cast<int>(u(rng) * 4.0) % 4);
    st.params[ParamId::WtZBCurve] = static_cast<float>(static_cast<int>(u(rng) * 4.0) % 4);
    st.params[ParamId::WtZOrder] = u(rng) < 0.5 ? 0.0f : 1.0f;
    st.params[ParamId::WtZFund] = u(rng) < 0.5 ? 0.0f : 1.0f;
    st.params[ParamId::WtZGain] = u(rng) < 0.5 ? 0.0f : 1.0f;
    st.params[ParamId::WtXMorph] = static_cast<float>(static_cast<int>(u(rng) * 3.0) % 3);
    st.params[ParamId::WtYMorph] = static_cast<float>(static_cast<int>(u(rng) * 3.0) % 3);
    st.params[ParamId::WtX] = static_cast<float>(u(rng));
    st.params[ParamId::WtY] = static_cast<float>(u(rng));
    st.params[ParamId::WtLevel] = 0.7f;
  }
  if (k >= 10) {  // D2 avec Feedback > 0 dans un emplacement (WT1, FM1 ou GL1)
    const int slot = k % 3 == 0 ? 0 : (k % 3 == 1 ? 2 : 4);
    const int b = idx(ParamId::DstWt1Type) + slot * static_cast<int>(DstOff::Count);
    auto P = [&](DstOff o) -> float& { return st.params.v[static_cast<size_t>(b + static_cast<int>(o))]; };
    P(DstOff::Type) = static_cast<float>(DstType::D2);
    P(DstOff::Mix) = 1.0f;
    P(DstOff::D2Size) = static_cast<float>(20.0 + 400.0 * u(rng));
    P(DstOff::D2Speed) = static_cast<float>(-2.0 + 4.0 * u(rng));
    P(DstOff::D2Repeat) = static_cast<float>(u(rng));
    P(DstOff::D2Pos) = static_cast<float>(u(rng));
    P(DstOff::D2Fb) = static_cast<float>(0.3 + 0.7 * u(rng));
    P(DstOff::D2Xfade) = static_cast<float>(1.0 + 20.0 * u(rng));
    if (slot == 2) st.params[ParamId::Op1Level] = 0.6f;
  }
  for (int i = 0; i < kNumParams; ++i) st.params.v[static_cast<size_t>(i)] = sanitize(static_cast<ParamId>(i), st.params.v[static_cast<size_t>(i)]);
  // ajouts G4.3 (absents en 0.3.1) : migration v3 et Shape quelconque
  std::mt19937_64 extra(0x5A7E0000ULL + static_cast<std::uint64_t>(k));
  std::uniform_real_distribution<float> s(-1.0f, 1.0f);
  for (const char* sid : {"dst.wt1.d2.fb_mode", "dst.wt2.d2.fb_mode", "dst.fm1.d2.fb_mode", "dst.fm2.d2.fb_mode", "dst.gl1.d2.fb_mode", "dst.gl2.d2.fb_mode"})
    setIfExists(st, sid, 0.0f);
  setIfExists(st, "wt.za.shape", s(extra));
  setIfExists(st, "wt.zb.shape", s(extra));
  return st;
}

std::uint64_t fnv(const std::vector<float>& a, const std::vector<float>& b) {
  std::uint64_t h = 1469598103934665603ULL;
  for (const auto* v : {&a, &b})
    for (float x : *v) {
      std::uint32_t bits;
      std::memcpy(&bits, &x, 4);
      for (int i = 0; i < 4; ++i) {
        h ^= (bits >> (8 * i)) & 0xFF;
        h *= 1099511628211ULL;
      }
    }
  return h;
}

std::uint64_t goldenHash(const PluginState& st) {
  G4Scenario sc;
  sc.state = st;
  sc.samples = 72000;  // 1,5 s à 48 kHz
  sc.events = {MidiEvent::noteOn(0, 45, 100), MidiEvent::noteOff(30000, 45), MidiEvent::noteOn(33000, 52, 90),
               MidiEvent::noteOn(50000, 57, 110), MidiEvent::noteOff(66000, 57)};
  const auto r = renderG4(sc);
  return fnv(r.L, r.R);
}

std::string platformTag() {
#if defined(_MSC_VER)
  return "msvc-x64";
#elif defined(__aarch64__) || defined(__arm64__)
  return "clang-arm64";
#else
  return "other";
#endif
}

constexpr int kGolden = 20;

void testGolden() {
  std::vector<std::uint64_t> h(kGolden);
  for (int k = 0; k < kGolden; ++k) h[static_cast<size_t>(k)] = goldenHash(goldenConfig(k));
  std::ostringstream line;
  line << platformTag();
  for (auto x : h) line << " " << std::hex << x;
  if (const char* w = std::getenv("SBD_GOLDEN_WRITE")) {  // génération (moteur 0.3.1 avant modification)
    std::ofstream out(w, std::ios::app);
    out << line.str() << "\n";
    std::printf("::notice title=GOLDEN-G43::%s\n", line.str().c_str());
    // états v3 de référence (MIG-02) : écrits par le codec 0.3.1
    for (int k : {3, 12, 17}) {
      std::ofstream st("golden_g43_v3_" + std::to_string(k) + ".txt", std::ios::binary);
      st << encodeState(goldenConfig(k), "SUBDESTRUCKT 0.3.1 golden", StateScope::Daw);
    }
    return;
  }
  std::ifstream in("tests/core/golden_g43.txt");
  if (!in) in.open("../tests/core/golden_g43.txt");
  std::string l, want;
  while (std::getline(in, l))
    if (l.rfind(platformTag() + " ", 0) == 0) want = l;
  if (want.empty()) {  // plate-forme sans empreinte consignée : échec (empreinte publiée pour consignation)
    std::printf("::notice title=GOLDEN-G43::%s\n", line.str().c_str());
    tk::result("ZC-03", false, "empreintes 0.3.1 absentes pour la plate-forme %s (publiees en annotation, a consigner)", platformTag().c_str());
    return;
  }
  std::istringstream ws(want);
  std::string tag;
  ws >> tag;
  int same = 0, sameZ = 0, sameD2 = 0;
  for (int k = 0; k < kGolden; ++k) {
    std::string hx;
    ws >> hx;
    const bool eq = std::strtoull(hx.c_str(), nullptr, 16) == h[static_cast<size_t>(k)];
    same += eq;
    if (k < 10 || k >= 16) sameZ += eq;
    if (k >= 10) sameD2 += eq;
  }
  const bool ok = same == kGolden;
  TK_CHECK(ok);
  tk::result("ZC-03", ok, "transformations Z existantes et D2 LEGACY vs empreintes 0.3.1 (a9cde4b, %s) : %d/%d rendus identiques bit a bit (Z existant %d/14, D2 fb>0 %d/10 ; Shape aleatoire ignore)",
             platformTag().c_str(), same, kGolden, sameZ, sameD2);
  const bool d3 = sameD2 == 10;
  TK_CHECK(d3);
  tk::result("D2R-03", d3, "LEGACY = moteur historique : 10 configurations D2 a Feedback > 0 (WT1, FM1, GL1 ; tailles, vitesses +-2, Repeat, Position, Crossfade aleatoires), etats migres en LEGACY : empreintes 0.3.1 identiques bit a bit %d/10",
             sameD2);
}

// ------------------------------------------------------------------ ELP-01R : critère prospectif (plateau accepté)
void testElp01R() {
  bool ident = true, sym = true, mono = true, cont = true, lip = true, bound = true;
  for (int i = 0; i <= 1000000; ++i) {
    const double y = -16.0 + 32.0 * i / 1000000.0;
    if (elpTransfer(y) != y) ident = false;
  }
  double prev = elpTransfer(16.0);
  const double lmax = std::log(DBL_MAX / 16.0);
  for (int i = 1; i <= 100000; ++i) {
    const double y = 16.0 * std::exp(lmax * i / 100000.0);  // ]16, DBL_MAX]
    const double f = elpTransfer(y);
    if (f < prev) mono = false;
    if (y <= 1e300 && elpTransfer(-y) != -f) sym = false;
    if (!(std::fabs(f) < kOutputGuard) || !(static_cast<float>(f) < 32.0f) || !(static_cast<float>(-f) > -32.0f)) bound = false;
    prev = f;
  }
  std::mt19937_64 rng(0xE101ULL);
  std::uniform_real_distribution<double> u(0.0, 1.0);
  for (int i = 0; i < 1000000; ++i) {
    double a = 16.0 * std::exp(u(rng) * 700.0), b = 16.0 * std::exp(u(rng) * 700.0);
    if (!std::isfinite(a) || !std::isfinite(b)) continue;
    if (a > b) std::swap(a, b);
    if (elpTransfer(b) < elpTransfer(a) || elpTransfer(-a) < elpTransfer(-b)) mono = false;
  }
  for (double eps : {1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1})
    if (std::fabs(elpTransfer(16.0 + eps) - (16.0 + eps)) > eps * eps / 16.0) cont = false;
  std::uniform_real_distribution<double> w(-400.0, 400.0);
  for (int i = 0; i < 100000; ++i) {
    const double a = w(rng), b = w(rng);
    if (std::fabs(elpTransfer(a) - elpTransfer(b)) > std::fabs(a - b) * (1.0 + 1e-12)) lip = false;
  }
  for (double y : {std::nextafter(16.0, 100.0), 1e3, 1e6, 1e300, DBL_MAX})
    if (!(elpTransfer(y) < kOutputGuard) || !(static_cast<float>(elpTransfer(y)) < 32.0f)) bound = false;
  // (f) non-finis : jamais traités par ELP (garde numérique, compteur distinct)
  Engine e(FactoryBanks::get("factory:saw"));
  ParamValues p = ParamValues::defaults();
  e.setParameters(p);
  e.prepare(48000.0, 256);
  RouteTable rt;  // routes à historique de retard (protocole G-16)
  rt.apply({RouteTable::Op::add(kNodeOP2, kNodeOP1), RouteTable::Op::add(kNodeWT, kNodeOP1), RouteTable::Op::add(kNodeOP1, kNodeWT)});
  e.setTopologyImmediate(rt.compile());
  std::vector<float> L(256), R(256);
  const MidiEvent on = MidiEvent::noteOn(0, 60, 100);
  e.process(L.data(), R.data(), 256, &on, 1);
  e.testInjectNaN();
  for (int b = 0; b < 20; ++b) e.process(L.data(), R.data(), 256, nullptr, 0);
  const auto tc = e.testCounters();
  const bool nonFinite = e.telemetry().nonFiniteResets.load() > 0 && tc.elp == 0;
  const bool ok = ident && sym && mono && cont && lip && bound && nonFinite;
  TK_CHECK(ok);
  tk::result("ELP-01R", ok, "ELP (plateau numerique accepte) : (a) identite exacte sur 10^6 points %d ; (b) symetrie exacte %d ; (c) monotonie non stricte (10^5 points jusqu'a DBL_MAX, 10^6 paires) %d ; (d) continuite au seuil et 1-lipschitzienne %d %d ; (e) |f| < 32 et (float)f < 32.0f %d ; (f) non-finis traites par la garde (reinitialisations %u, engagements ELP %u) %d",
             ident, sym, mono, cont, lip, bound, e.telemetry().nonFiniteResets.load(), tc.elp, nonFinite);
}

}  // namespace

void runG43Tests() {
  std::printf("[ARCH-G4.3 : empreintes 0.3.1, ELP]\n");
  testGolden();
  testElp01R();
}
