#include "Parameters.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace sbd {

namespace {
constexpr ParamCategory categoryFor(ParamType t, bool automatable) {
  return t == ParamType::Float ? ParamCategory::Continuous
                               : (automatable ? ParamCategory::Event : ParamCategory::Structural);
}

// Ordre = ordre de l'enum (même X-macro). L'ordre n'est PAS une identité : seule stableId l'est.
constexpr ParamSpec kSpecs[kNumParams] = {
#define SBD_PARAM(e, sid, nm, ty, un, mn, mx, df, sm, au, ve, dom, ra, rn, sc, hmn, hmx, ch, gr, st) \
  {sid, nm, un, mn, mx, df, sm, au, categoryFor(ty, au), ve, "preset", ty, dom, ra, rn, sc, hmn, hmx, ch, gr, st},
#include "ParamTable.inc"
#undef SBD_PARAM
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

int choiceCount(ParamId id) noexcept {
  const auto& s = paramSpec(id);
  if (s.type != ParamType::Choice) return 0;
  int n = 1;
  for (const char* c = s.choices; *c; ++c) n += (*c == '|');
  return n;
}

std::string choiceLabel(ParamId id, int index) {
  const char* c = paramSpec(id).choices;
  for (int k = 0; k < index && *c; ++c)
    if (*c == '|') ++k;
  const char* e = c;
  while (*e && *e != '|') ++e;
  return std::string(c, static_cast<size_t>(e - c));
}

std::string formatValue(ParamId id, float value) {
  const auto& s = paramSpec(id);
  char buf[64];
  switch (s.type) {
    case ParamType::Choice: return choiceLabel(id, static_cast<int>(value + 0.5f));
    case ParamType::Bool: return value >= 0.5f ? "On" : "Off";
    case ParamType::Int: std::snprintf(buf, sizeof buf, "%d", static_cast<int>(std::lround(value))); return buf;
    case ParamType::Float: break;
  }
  const double a = std::fabs(value);
  if (a >= 1000.0)
    std::snprintf(buf, sizeof buf, "%.0f %s", static_cast<double>(value), s.unit);
  else if (a >= 100.0)
    std::snprintf(buf, sizeof buf, "%.1f %s", static_cast<double>(value), s.unit);
  else if (a >= 1.0)
    std::snprintf(buf, sizeof buf, "%.2f %s", static_cast<double>(value), s.unit);
  else
    std::snprintf(buf, sizeof buf, "%.3f %s", static_cast<double>(value), s.unit);
  return buf;
}

double toDomain(Domain d, double v) noexcept {
  switch (d) {
    case Domain::Oct:
    case Domain::Time: return std::log2(v > 1e-9 ? v : 1e-9);
    default: return v;
  }
}

double fromDomain(Domain d, double v) noexcept {
  switch (d) {
    case Domain::Oct:
    case Domain::Time: return std::exp2(v);
    case Domain::Cyc: return v - std::floor(v);
    default: return v;
  }
}

float sanitize(ParamId id, float v) noexcept {
  const auto& s = paramSpec(id);
  if (!std::isfinite(v)) v = s.defaultValue;
  if (s.type != ParamType::Float) v = std::round(v);
  if (v < s.minValue) v = s.minValue;
  if (v > s.maxValue) v = s.maxValue;
  return v;
}

ParamValues ParamValues::defaults() noexcept {
  ParamValues p;
  for (int i = 0; i < kNumParams; ++i) p.v[static_cast<size_t>(i)] = kSpecs[i].defaultValue;
  return p;
}

}  // namespace sbd
