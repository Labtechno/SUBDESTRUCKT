#include "Parameters.h"

namespace sbd {

namespace {
constexpr ParamCategory C = ParamCategory::Continuous;
// Ordre = ordre de l'enum (contrôlé par static_assert). L'ordre n'est PAS une identité : seule stableId l'est.
constexpr ParamSpec kSpecs[kNumParams] = {
    {"amp.attack", "Amp Attack", "s", 0.001f, 5.0f, 0.005f, 0.0f, true, C, 1, "preset"},
    {"amp.decay", "Amp Decay", "s", 0.001f, 5.0f, 0.25f, 0.0f, true, C, 1, "preset"},
    {"amp.sustain", "Amp Sustain", "lin", 0.0f, 1.0f, 0.8f, 0.0f, true, C, 1, "preset"},
    {"amp.release", "Amp Release", "s", 0.001f, 10.0f, 0.3f, 0.0f, true, C, 1, "preset"},
    {"amp.velocity", "Amp Velocity", "lin", 0.0f, 1.0f, 1.0f, 20.0f, true, C, 1, "preset"},
    {"out.gain", "Output Gain", "dB", -48.0f, 6.0f, -6.0f, 20.0f, true, C, 1, "preset"},
    {"wt.level", "WT Level", "lin", 0.0f, 1.0f, 0.5f, 20.0f, true, C, 1, "preset"},
    {"op1.level", "OP1 Level", "lin", 0.0f, 1.0f, 0.5f, 20.0f, true, C, 1, "preset"},
    {"op2.level", "OP2 Level", "lin", 0.0f, 1.0f, 0.0f, 20.0f, true, C, 1, "preset"},
    {"op1.ratio", "OP1 Ratio", "x f0", 0.25f, 16.0f, 1.0f, 20.0f, true, C, 1, "preset"},
    {"op2.ratio", "OP2 Ratio", "x f0", 0.25f, 16.0f, 2.0f, 20.0f, true, C, 1, "preset"},
    {"fm.op2_op1.depth", "FM OP2>OP1", "rad", 0.0f, 8.0f, 1.0f, 20.0f, true, C, 1, "preset"},
    {"fm.op1_op2.depth", "FM OP1>OP2", "rad", 0.0f, 8.0f, 0.0f, 20.0f, true, C, 1, "preset"},
    {"fm.op1_op1.depth", "FM OP1 Self", "rad", 0.0f, 8.0f, 0.0f, 20.0f, true, C, 1, "preset"},
    {"fm.op2_op2.depth", "FM OP2 Self", "rad", 0.0f, 8.0f, 0.0f, 20.0f, true, C, 1, "preset"},
    {"cross.wt_op1.depth", "CROSS WT>OP1", "rad", 0.0f, 8.0f, 0.5f, 20.0f, true, C, 1, "preset"},
    {"cross.wt_op2.depth", "CROSS WT>OP2", "rad", 0.0f, 8.0f, 0.0f, 20.0f, true, C, 1, "preset"},
    {"cross.op1_wt.depth", "CROSS OP1>WT", "rad", 0.0f, 8.0f, 0.5f, 20.0f, true, C, 1, "preset"},
    {"cross.op2_wt.depth", "CROSS OP2>WT", "rad", 0.0f, 8.0f, 0.0f, 20.0f, true, C, 1, "preset"},
    {"cross.master", "CROSS Master", "lin", 0.0f, 1.0f, 1.0f, 20.0f, true, C, 1, "preset"},
    {"cross.fwd", "CROSS WT>FM", "lin", 0.0f, 1.0f, 1.0f, 20.0f, true, C, 1, "preset"},
    {"cross.rev", "CROSS FM>WT", "lin", 0.0f, 1.0f, 1.0f, 20.0f, true, C, 1, "preset"},
};
}  // namespace

const ParamSpec& paramSpec(ParamId id) noexcept { return kSpecs[static_cast<int>(id)]; }

bool findParam(std::string_view stableId, ParamId& out) noexcept {
  for (int i = 0; i < kNumParams; ++i)
    if (stableId == kSpecs[i].stableId) {
      out = static_cast<ParamId>(i);
      return true;
    }
  return false;
}

ParamValues ParamValues::defaults() noexcept {
  ParamValues p;
  for (int i = 0; i < kNumParams; ++i) p.v[static_cast<size_t>(i)] = kSpecs[i].defaultValue;
  return p;
}

}  // namespace sbd
