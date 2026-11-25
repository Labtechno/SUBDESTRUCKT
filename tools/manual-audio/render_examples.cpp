// SUBDESTRUCKT — rendu des exemples audio du manuel 0.1.0-b5 avec le moteur DSP réel (source/engine, non modifié).
// Usage : render_examples <dossier_sortie>
// Chaque exemple : mono, 48 kHz (K = 1), WAV PCM 24 bits. Niveau : RMS aligné à -20 dBFS sur la partie tenue de la note
// (même gain pour tout le fichier), crête vérifiée < -1 dBFS. Un fichier examples.json décrit chaque rendu.
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>

#include "BuildInfo.h"
#include "core/StateCodec.h"
#include "engine/Engine.h"

using namespace sbd;

namespace {
struct Example {
  std::string id, title;
  double seconds;
  double releaseAt;  // note-off (s)
  int note;
  ParamValues params;
  bool defaultTopology = true;
  bool crossRamp = false;  // automation de CROSS Master 0 -> 1 sur la durée
};

void writeWav24(const std::string& path, const std::vector<double>& x, int sr) {
  FILE* f = std::fopen(path.c_str(), "wb");
  const std::uint32_t data = static_cast<std::uint32_t>(x.size() * 3);
  auto u32 = [&](std::uint32_t v) { std::fwrite(&v, 4, 1, f); };
  auto u16 = [&](std::uint16_t v) { std::fwrite(&v, 2, 1, f); };
  std::fwrite("RIFF", 1, 4, f); u32(36 + data); std::fwrite("WAVEfmt ", 1, 8, f);
  u32(16); u16(1); u16(1); u32(static_cast<std::uint32_t>(sr)); u32(static_cast<std::uint32_t>(sr) * 3); u16(3); u16(24);
  std::fwrite("data", 1, 4, f); u32(data);
  for (double v : x) {
    long s = std::lround(std::fmax(-1.0, std::fmin(1.0, v)) * 8388607.0);
    unsigned char b[3] = {static_cast<unsigned char>(s & 0xff), static_cast<unsigned char>((s >> 8) & 0xff),
                          static_cast<unsigned char>((s >> 16) & 0xff)};
    std::fwrite(b, 1, 3, f);
  }
  std::fclose(f);
}

ParamValues base() { return ParamValues::defaults(); }
}  // namespace

int main(int argc, char** argv) {
  const std::string out = argc > 1 ? argv[1] : ".";
  const double fs = 48000.0;
  std::vector<Example> ex;
  auto add = [&](std::string id, std::string title, ParamValues p, double secs = 3.0, double rel = 2.2, int note = 57) {
    ex.push_back({std::move(id), std::move(title), secs, rel, note, p});
  };
  {  // E01 : réglages initiaux (preset init)
    add("e01_init", "Preset init (réglages par défaut)", base());
  }
  {  // E02 : WT seule
    auto p = base(); p[ParamId::WtLevel] = 0.8f; p[ParamId::Op1Level] = 0; p[ParamId::Op2Level] = 0; p[ParamId::CrossMaster] = 0;
    add("e02_wt_seule", "WT seule (WT Level 0,8, OP1/OP2 Level 0, CROSS Master 0)", p);
  }
  {  // E03 : FM seule, profondeur 1 rad
    auto p = base(); p[ParamId::WtLevel] = 0; p[ParamId::Op1Level] = 0.8f; p[ParamId::CrossMaster] = 0;
    add("e03_fm_seule", "FM seule (OP1 Level 0,8, FM OP2>OP1 1 rad, WT Level 0, CROSS Master 0)", p);
  }
  {  // E04/E05 : modulation faible / forte
    auto p = base(); p[ParamId::WtLevel] = 0; p[ParamId::Op1Level] = 0.8f; p[ParamId::CrossMaster] = 0; p[ParamId::DepthOp2Op1] = 0.3f;
    add("e04_fm_faible", "FM faible (FM OP2>OP1 0,3 rad)", p);
    p[ParamId::DepthOp2Op1] = 4.0f;
    add("e05_fm_forte", "FM forte (FM OP2>OP1 4 rad)", p);
  }
  {  // E06/E07 : ratio harmonique / inharmonique
    auto p = base(); p[ParamId::WtLevel] = 0; p[ParamId::Op1Level] = 0.8f; p[ParamId::CrossMaster] = 0; p[ParamId::DepthOp2Op1] = 2.0f;
    p[ParamId::Op2Ratio] = 2.0f;
    add("e06_ratio_2", "OP2 Ratio 2 (harmonique), FM OP2>OP1 2 rad", p);
    p[ParamId::Op2Ratio] = 3.5f;
    add("e07_ratio_3_5", "OP2 Ratio 3,5 (métallique), FM OP2>OP1 2 rad", p);
  }
  {  // E08/E09 : CROSS WT>FM (on écoute l'OP1)
    auto p = base(); p[ParamId::WtLevel] = 0; p[ParamId::Op1Level] = 0.8f; p[ParamId::DepthOp2Op1] = 0.0f;
    p[ParamId::CrossFwd] = 0; p[ParamId::CrossRev] = 0;
    add("e08_op1_sans_cross", "OP1 seul, CROSS WT>FM 0 (FM OP2>OP1 0)", p);
    p[ParamId::CrossFwd] = 1; p[ParamId::DepthWtOp1] = 2.0f;
    add("e09_cross_wt_vers_fm", "OP1 modulé par la WT (CROSS WT>OP1 2 rad, CROSS WT>FM 1, CROSS FM>WT 0)", p);
  }
  {  // E10/E11 : CROSS FM>WT (on écoute la WT)
    auto p = base(); p[ParamId::WtLevel] = 0.8f; p[ParamId::Op1Level] = 0; p[ParamId::Op2Level] = 0;
    p[ParamId::CrossFwd] = 0; p[ParamId::CrossRev] = 0; p[ParamId::Op1Ratio] = 1.5f;
    add("e10_wt_sans_cross", "WT seule, CROSS FM>WT 0", p);
    p[ParamId::CrossRev] = 1; p[ParamId::DepthOp1Wt] = 2.0f;
    add("e11_cross_fm_vers_wt", "WT modulée par OP1 (CROSS OP1>WT 2 rad, OP1 Ratio 1,5, CROSS FM>WT 1, CROSS WT>FM 0)", p);
  }
  {  // E12 : bidirectionnel
    auto p = base(); p[ParamId::WtLevel] = 0.5f; p[ParamId::Op1Level] = 0.5f; p[ParamId::DepthWtOp1] = 1.5f; p[ParamId::DepthOp1Wt] = 1.5f;
    add("e12_bidirectionnel", "CROSS dans les deux sens (CROSS WT>OP1 1,5 rad, CROSS OP1>WT 1,5 rad, Master 1)", p);
  }
  {  // E13/E14 : enveloppes courte / longue
    auto p = base();
    p[ParamId::AmpAttack] = 0.001f; p[ParamId::AmpDecay] = 0.15f; p[ParamId::AmpSustain] = 0.0f; p[ParamId::AmpRelease] = 0.1f;
    add("e13_env_courte", "Enveloppe courte (A 0,001 s, D 0,15 s, S 0, R 0,1 s)", p, 3.0, 0.5);
    p[ParamId::AmpAttack] = 1.0f; p[ParamId::AmpDecay] = 0.5f; p[ParamId::AmpSustain] = 0.7f; p[ParamId::AmpRelease] = 1.5f;
    add("e14_env_longue", "Enveloppe longue (A 1 s, D 0,5 s, S 0,7, R 1,5 s)", p, 4.5, 2.5);
  }
  {  // E15 : texture évolutive (automation de CROSS Master 0 -> 1)
    auto p = base(); p[ParamId::WtLevel] = 0.5f; p[ParamId::Op1Level] = 0.5f; p[ParamId::DepthWtOp1] = 3.0f; p[ParamId::DepthOp1Wt] = 3.0f;
    p[ParamId::Op1Ratio] = 0.5f; p[ParamId::CrossMaster] = 0.0f; p[ParamId::AmpRelease] = 1.0f;
    add("e15_texture", "Note tenue, automation de CROSS Master de 0 à 1 en 6 s (CROSS 3 rad, OP1 Ratio 0,5)", p, 7.0, 6.2, 45);
    ex.back().crossRamp = true;
  }

  FILE* js = std::fopen((out + "/examples.json").c_str(), "wb");
  std::fprintf(js, "{\n  \"engineSourceCommit\": \"%s\",\n  \"renderer\": \"tools/manual-audio/render_examples.cpp\",\n  \"sampleRate\": 48000,\n  \"oversampling\": 1,\n  \"format\": \"WAV PCM 24 bits mono\",\n  \"level\": \"RMS aligne a -20 dBFS sur la partie tenue (gain unique par fichier)\",\n  \"examples\": [\n", SUBDESTRUCKT_COMMIT);
  for (size_t k = 0; k < ex.size(); ++k) {
    const auto& e = ex[k];
    Engine eng(std::make_shared<const SawWavetable>());
    eng.setParameters(e.params);
    eng.prepare(fs, 256);
    const PluginState d = defaultState();  // topologie du preset init : OP2>OP1 (I), WT>OP1 (I), OP1>WT (D)
    RouteTable t;
    std::string err;
    t.assign(d.routes, d.nextRouteId, err);
    eng.setTopologyImmediate(t.compile());
    const long long n = static_cast<long long>(e.seconds * fs);
    std::vector<float> L(static_cast<size_t>(n)), R(static_cast<size_t>(n));
    ParamValues p = e.params;
    for (long long pos = 0; pos < n; pos += 256) {
      const int bl = static_cast<int>(std::min<long long>(256, n - pos));
      if (e.crossRamp) p[ParamId::CrossMaster] = static_cast<float>(std::fmin(1.0, pos / (6.0 * fs)));
      eng.setParameters(p);
      std::vector<MidiEvent> ev;
      const long long on = static_cast<long long>(0.05 * fs), off = static_cast<long long>(e.releaseAt * fs);
      if (on >= pos && on < pos + bl) ev.push_back(MidiEvent::noteOn(static_cast<int>(on - pos), e.note, 100));
      if (off >= pos && off < pos + bl) ev.push_back(MidiEvent::noteOff(static_cast<int>(off - pos), e.note));
      eng.process(&L[static_cast<size_t>(pos)], &R[static_cast<size_t>(pos)], bl, ev.data(), static_cast<int>(ev.size()));
    }
    // RMS sur la partie tenue (de 0,3 s à la note-off, ou 0,05-0,4 s pour l'enveloppe courte)
    const size_t a = static_cast<size_t>((e.releaseAt < 1.0 ? 0.06 : 0.3) * fs);
    const size_t b = static_cast<size_t>(e.releaseAt * fs);
    double s = 0; for (size_t i = a; i < b; ++i) s += double(L[i]) * L[i];
    const double rms = std::sqrt(s / double(b - a));
    double g = std::pow(10.0, -20.0 / 20.0) / (rms + 1e-12);
    double pk = 0; for (float v : L) pk = std::fmax(pk, std::fabs(v * g));
    if (pk > std::pow(10.0, -1.0 / 20.0)) g *= std::pow(10.0, -1.0 / 20.0) / pk;  // jamais de saturation
    std::vector<double> y(L.size());
    double pk2 = 0; for (size_t i = 0; i < L.size(); ++i) { y[i] = L[i] * g; pk2 = std::fmax(pk2, std::fabs(y[i])); }
    writeWav24(out + "/" + e.id + ".wav", y, 48000);
    std::fprintf(js, "    {\"id\": \"%s\", \"file\": \"%s.wav\", \"title\": \"%s\", \"note\": %d, \"velocity\": 100, \"seconds\": %.2f, \"noteOffAt\": %.2f, \"gainDb\": %.2f, \"peakDbfs\": %.2f, \"automation\": \"%s\", \"params\": {",
                 e.id.c_str(), e.id.c_str(), e.title.c_str(), e.note, e.seconds, e.releaseAt, 20 * std::log10(g), 20 * std::log10(pk2 + 1e-12),
                 e.crossRamp ? "cross.master 0 -> 1 en 6 s" : "");
    for (int i = 0; i < kNumParams; ++i)
      std::fprintf(js, "%s\"%s\": %.4g", i ? ", " : "", paramSpec(i).stableId, double(e.params.v[size_t(i)]));
    std::fprintf(js, "}}%s\n", k + 1 < ex.size() ? "," : "");
    std::fprintf(stderr, "%s : gain %.1f dB, crete %.1f dBFS\n", e.id.c_str(), 20 * std::log10(g), 20 * std::log10(pk2 + 1e-12));
  }
  std::fprintf(js, "  ]\n}\n");
  std::fclose(js);
  return 0;
}
