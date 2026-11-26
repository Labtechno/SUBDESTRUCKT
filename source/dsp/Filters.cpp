// SUBDESTRUCKT — L3 — Filter & Resonator Engine. ÉTAPE PROVISOIRE : emplacement transparent (algorithmes au lot FLT).
#include "Filters.h"

namespace sbd {
void FilterSlot::configure(double fs) { fs_ = fs; }
void FilterSlot::reset() noexcept { energy_ = 0.0; }
void FilterSlot::process(double&, double&, const double*, int, const FilterContext&) noexcept {}
void FilterSlot::switchType(int type) noexcept { type_ = type; }
}  // namespace sbd
