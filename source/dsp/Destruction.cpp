// SUBDESTRUCKT — L3 — Destruction Engine. ÉTAPE PROVISOIRE : emplacement transparent (algorithmes au lot DST).
#include "Destruction.h"

namespace sbd {
void DestructionSlot::configure(double fs, std::uint64_t seed, Quality q) {
  fs_ = fs;
  seed_ = seed;
  quality_ = q;
}
void DestructionSlot::reset() noexcept {}
void DestructionSlot::process(double&, double&, const double*, int, const DestructionContext&) noexcept {}
int DestructionSlot::latencyFor(int, int) noexcept { return 0; }
void DestructionSlot::switchType(int type, int fftChoice) noexcept {
  type_ = type;
  fftChoice_ = fftChoice;
}
}  // namespace sbd
