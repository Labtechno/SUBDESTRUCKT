// SUBDESTRUCKT — rendu des exemples audio du manuel 0.2.0 avec le moteur DSP réel (source/, non modifié).
// Usage : subdestruckt_manual_render <dossier_sortie> <commit_source>
// Chaque exemple : état complet (PluginState), stéréo mixée en mono, 48 kHz, WAV PCM 24 bits. Niveau : RMS aligné à
// -20 dBFS sur la partie tenue (gain unique par fichier), crête vérifiée < -1 dBFS. examples.json décrit chaque rendu
// (paramètres modifiés par rapport au défaut, routes, matrice, banque).
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "core/PresetOps.h"
#include "core/StateCodec.h"
#include "core/WtAssets.h"
#include "engine/Engine.h"

using namespace sbd;
using Op = RouteTable::Op;

namespace {
struct Example {
  std::string id, title;
  int note = 45;
  double seconds = 3.0, noteOffAt = 2.2;
  PluginState state;
  std::string automation;
  std::function<void(ParamValues&, double)> automate;
};

void writeWav24(const std::string& path, const std::vector<double>& x, int sr) {
  FILE* f = std::fopen(path.c_str(), "wb");
  if (!f) return;
  const std::uint32_t data = static_cast<std::uint32_t>(x.size() * 3);
  auto u32 = [&](std::uint32_t v) { std::fwrite(&v, 4, 1, f); };
  auto u16 = [&](std::uint16_t v) { std::fwrite(&v, 2, 1, f); };
  std::fwrite("RIFF", 1, 4, f);
  u32(36 + data);
  std::fwrite("WAVEfmt ", 1, 8, f);
  u32(16);
  u16(1);
  u16(1);
  u32(static_cast<std::uint32_t>(sr));
  u32(static_cast<std::uint32_t>(sr) * 3);
  u16(3);
  u16(24);
  std::fwrite("data", 1, 4, f);
  u32(data);
  for (double v : x) {
    const long s = std::lround(std::fmax(-1.0, std::fmin(1.0, v)) * 8388607.0);
    const unsigned char b[3] = {static_cast<unsigned char>(s & 0xff), static_cast<unsigned char>((s >> 8) & 0xff), static_cast<unsigned char>((s >> 16) & 0xff)};
    std::fwrite(b, 1, 3, f);
  }
  std::fclose(f);
}

std::vector<double> render(const Example& e, double fs) {
  Engine eng(resolveBank(e.state.bank) ? resolveBank(e.state.bank) : FactoryBanks::get("factory:saw"));
  ParamValues p = e.state.params;
  eng.setParameters(p);
  eng.setSeed(e.state.seed);
  eng.setLfoCurves(e.state.lfoCurves);
  eng.prepare(fs, 256, Quality::Normal);
  RouteTable t;
  std::string err;
  t.assign(e.state.routes, e.state.nextRouteId, err);
  eng.setTopologyImmediate(t.compile());
  eng.setModMatrixImmediate(*compileModMatrix(e.state.mod));
  const long long N = static_cast<long long>(e.seconds * fs);
  std::vector<double> out(static_cast<size_t>(N));
  std::vector<float> L(256), R(256);
  const long long off = static_cast<long long>(e.noteOffAt * fs);
  for (long long pos = 0; pos < N; pos += 256) {
    const int n = static_cast<int>(std::min<long long>(256, N - pos));
    if (e.automate) e.automate(p, pos / fs);
    eng.setParameters(p);
    std::vector<MidiEvent> ev;
    if (pos == 0) ev.push_back(MidiEvent::noteOn(0, e.note, 100));
    if (off >= pos && off < pos + n) ev.push_back(MidiEvent::noteOff(static_cast<int>(off - pos), e.note));
    eng.process(L.data(), R.data(), n, ev.data(), static_cast<int>(ev.size()));
    for (int i = 0; i < n; ++i) out[static_cast<size_t>(pos + i)] = 0.5 * (L[static_cast<size_t>(i)] + R[static_cast<size_t>(i)]);
  }
  return out;
}

void setRoutes(PluginState& s, const std::vector<Op>& ops) {
  RouteTable t;
  t.apply(ops);
  s.routes = t.routes();
  s.nextRouteId = t.nextRouteId();
}
PluginState bare() {
  PluginState s = defaultState();
  s.routes.clear();
  s.nextRouteId = 1;
  for (int k = 0; k < kMaxRoutes; ++k) s.params[routeSlotInfo(k).depthParam] = 0.0f;
  s.params[ParamId::Op1Level] = 0.0f;
  s.params[ParamId::WtLevel] = 0.5f;
  return s;
}
std::string jsonEsc(const std::string& s) {
  std::string o;
  for (char c : s) o += (c == '"' || c == '\\') ? std::string("\\") + c : std::string(1, c);
  return o;
}
}  // namespace

int main(int argc, char** argv) {
  if (argc < 3) {
    std::printf("usage : %s <dossier> <commit>\n", argv[0]);
    return 1;
  }
  const std::string dir = argv[1], commit = argv[2];
  std::vector<Example> ex;
  auto add = [&](const char* id, const char* title, PluginState s, int note = 45) {
    Example e;
    e.id = id;
    e.title = title;
    e.state = std::move(s);
    e.note = note;
    ex.push_back(std::move(e));
  };
  add("e01_init", "Preset init (identique a 0.1.0-b5)", defaultState(), 57);
  {
    PluginState s = bare();
    s.bank = "factory:basic";
    add("e02_xy_basic", "WT seule, banque Basic, X a 0 (sinus)", s);
    s.params[ParamId::WtX] = 1.0f;
    add("e03_xy_square", "WT seule, banque Basic, X a 1 (carre)", s);
  }
  {
    PluginState s = bare();
    s.params[ParamId::WtZAType] = 1.0f;
    s.params[ParamId::WtZ] = 0.5f;
    add("e04_z_fold", "Z : Harmonic Fold a 0,5 (dent de scie)", s);
    s.params[ParamId::WtZAType] = 2.0f;
    add("e05_z_warp", "Z : Phase Warp a 0,5", s);
    s.params[ParamId::WtZAType] = 3.0f;
    s.params[ParamId::WtZ] = 0.8f;
    add("e06_z_mutation", "Z : Spectral Mutation a 0,8", s);
  }
  {
    PluginState s = bare();
    s.params[ParamId::WtLevel] = 0.0f;
    s.params[ParamId::Op1Level] = 0.5f;
    s.params[ParamId::Op2Ratio] = 2.0f;
    s.params[ParamId::DepthOp2Op1] = 2.0f;
    setRoutes(s, {Op::add(kNodeOP2, kNodeOP1)});
    add("e07_fm_pm", "FM : OP2>OP1 en PM (2 rad)", s);
    s.params[ParamId::Op1Mode] = 1.0f;
    s.params[ParamId::DepthOp2Op1] = 1.0f;
    add("e08_fm_lin", "FM : OP2>OP1 en Linear FM (100 %)", s);
    s.params[ParamId::Op1Mode] = 3.0f;
    add("e09_fm_exp", "FM : OP2>OP1 en Exponential FM (1 octave)", s);
  }
  {
    PluginState s = defaultState();
    s.params[ParamId::FltGlType] = 1.0f;
    s.params[ParamId::FltGlCutoff] = 500.0f;
    s.params[ParamId::FltGlRes] = 0.6f;
    add("e10_f1_lp", "Filtre F1 LP12 a 500 Hz, resonance 0,6", s);
    s.params[ParamId::FltGlType] = 2.0f;
    s.params[ParamId::FltGlCutoff] = 110.0f;
    s.params[ParamId::FltGlKey] = 1.0f;
    s.params[ParamId::FltGlMix] = 0.5f;
    add("e11_f2_comb", "Filtre F2 Comb accorde sur la note (keytrack 1)", s);
    s.params[ParamId::FltGlType] = 4.0f;
    s.params[ParamId::FltGlModal] = 1.0f;
    add("e12_f4_modal", "Resonateur F4 modal, modele Metal", s);
  }
  {
    PluginState s = defaultState();
    s.params[ParamId::DstGl1Type] = 1.0f;
    s.params[ParamId::DstGl1N1Drive] = 12.0f;
    s.params[ParamId::DstGl1N1Fold] = 0.7f;
    add("e13_n1_fold", "Destruction N1 wavefolder (drive 12 dB, fold 0,7)", s);
    s.params[ParamId::DstGl1Type] = 2.0f;
    s.params[ParamId::DstGl1D1Bits] = 5.0f;
    s.params[ParamId::DstGl1D1Rate] = 8.0f;
    add("e14_d1_crush", "Destruction D1 crusher (5 bits, ratio 8)", s);
    s.params[ParamId::DstGl1Type] = 4.0f;
    s.params[ParamId::DstGl1S1Freq] = 70.0f;
    add("e15_s1_ring", "Destruction S1 ring modulator (70 Hz)", s);
  }
  {
    PluginState s = defaultState();
    ModRoute r;
    r.source = ModSource::Lfo1;
    r.dest = ParamId::DepthOp2Op1;
    r.depth = 0.4f;
    modAdd(s.mod, r);
    s.params[ParamId::Lfo1Rate] = 0.7f;
    add("e16_mod_lfo", "Matrice : LFO1 (0,7 Hz) vers FM OP2>OP1", s);
  }
  {
    PluginState s = defaultState();
    s.params[ParamId::CrossAudioMode] = 2.0f;
    s.params[ParamId::CrossAudioAmount] = 0.8f;
    add("e17_cross_ring", "Cross Audio Ring (WT x FM, 0,8)", s);
  }
  for (const auto& fp : factoryPresets()) {
    if (fp.first.name == "Init (0.1.0-b5)") continue;
    std::string id = "p_" + PresetLibrary::sanitizeName(fp.first.name);
    for (auto& c : id) c = (c == ' ' || c == '-') ? '_' : static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    add(id.c_str(), ("Preset usine : " + fp.first.name).c_str(), fp.second);
  }

  const double fs = 48000.0;
  std::string json = "{\n  \"engineSourceCommit\": \"" + commit + "\",\n  \"renderer\": \"tools/manual-audio/render_examples_g4.cpp\",\n"
                     "  \"sampleRate\": 48000,\n  \"oversampling\": 1,\n  \"format\": \"WAV PCM 24 bits mono\",\n"
                     "  \"level\": \"RMS aligne a -20 dBFS sur la partie tenue (gain unique par fichier)\",\n  \"examples\": [\n";
  for (size_t k = 0; k < ex.size(); ++k) {
    const auto& e = ex[k];
    auto x = render(e, fs);
    double rms = 0.0;
    const size_t a = static_cast<size_t>(0.6 * fs), b = static_cast<size_t>(std::min(e.noteOffAt, e.seconds) * fs);
    for (size_t i = a; i < b; ++i) rms += x[i] * x[i];
    rms = std::sqrt(rms / std::max<size_t>(1, b - a));
    double g = rms > 1e-9 ? std::pow(10.0, -20.0 / 20.0) / rms : 1.0, pk = 0.0;
    for (double v : x) pk = std::max(pk, std::fabs(v * g));
    if (pk > std::pow(10.0, -1.0 / 20.0)) g *= std::pow(10.0, -1.0 / 20.0) / pk;
    pk = 0.0;
    for (auto& v : x) {
      v *= g;
      pk = std::max(pk, std::fabs(v));
    }
    const std::string file = e.id + ".wav";
    writeWav24(dir + "/" + file, x, 48000);
    std::string params;
    const auto def = ParamValues::defaults();
    for (int i = 0; i < kNumParams; ++i)
      if (e.state.params.v[static_cast<size_t>(i)] != def.v[static_cast<size_t>(i)]) {
        char buf[96];
        std::snprintf(buf, sizeof buf, "%s\"%s\": %.6g", params.empty() ? "" : ", ", paramSpec(i).stableId, static_cast<double>(e.state.params.v[static_cast<size_t>(i)]));
        params += buf;
      }
    std::string routes;
    for (const auto& r : e.state.routes) routes += std::string(routes.empty() ? "" : ", ") + "\"" + nodeName(r.src) + ">" + portName(r.dst) + " " + classLetter(r.cls) + "\"";
    char hdr[512];
    std::snprintf(hdr, sizeof hdr, "    {\"id\": \"%s\", \"file\": \"%s\", \"title\": \"%s\", \"note\": %d, \"velocity\": 100, \"seconds\": %.2f, \"noteOffAt\": %.2f, \"gainDb\": %.2f, \"peakDbfs\": %.2f, \"bank\": \"%s\", \"modRoutes\": %d, ",
                  e.id.c_str(), file.c_str(), jsonEsc(e.title).c_str(), e.note, e.seconds, e.noteOffAt, 20.0 * std::log10(g), 20.0 * std::log10(std::max(pk, 1e-12)),
                  e.state.bank.c_str(), static_cast<int>(e.state.mod.routes.size()));
    json += std::string(hdr) + "\"routes\": [" + routes + "], \"automation\": \"\", \"params\": {" + params + "}}" + (k + 1 < ex.size() ? ",\n" : "\n");
    std::printf("%s : crete %.2f dBFS\n", e.id.c_str(), 20.0 * std::log10(std::max(pk, 1e-12)));
  }
  json += "  ]\n}\n";
  FILE* f = std::fopen((dir + "/examples.json").c_str(), "wb");
  if (f) {
    std::fwrite(json.data(), 1, json.size(), f);
    std::fclose(f);
  }
  return 0;
}
