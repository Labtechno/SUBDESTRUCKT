#include "Chain.h"

namespace sbd {

void ProcessingChain::configure(int location, double fs, Quality q) {
  location_ = location;
  static constexpr ParamId kFilt[3] = {ParamId::FltWtType, ParamId::FltFmType, ParamId::FltGlType};
  static constexpr ParamId kDst[3][2] = {{ParamId::DstWt1Type, ParamId::DstWt2Type},
                                         {ParamId::DstFm1Type, ParamId::DstFm2Type},
                                         {ParamId::DstGl1Type, ParamId::DstGl2Type}};
  pFilterBase_ = kFilt[location];
  pDstBase_ = {kDst[location][0], kDst[location][1]};
  filter_.configure(fs);
  for (int k = 0; k < 2; ++k)
    dst_[static_cast<size_t>(k)].configure(fs, 0xD5700000ULL + static_cast<std::uint64_t>(location * 2 + k), q);
}

void ProcessingChain::reset() noexcept {
  filter_.reset();
  for (auto& d : dst_) d.reset();
}

void ProcessingChain::process(double& L, double& R, const double* E, const ChainContext& c) noexcept {
  const int fb = idx(pFilterBase_);
  const bool post = E[fb + static_cast<int>(FltOff::Pos)] >= 0.5;
  FilterContext fc;
  fc.fs = c.fs;
  fc.noteHz = c.noteHz;
  fc.voiceIdle = c.voiceIdle;
  const int exc = static_cast<int>(E[fb + static_cast<int>(FltOff::Exc)] + 0.5);
  fc.excite = (exc == 1 && c.noteOnEdge) || (exc == 2 && c.manualExcite);
  DestructionContext dc;
  dc.fs = c.fs;
  dc.voiceIdle = c.voiceIdle;
  dc.quality = c.quality;
  if (!post) filter_.process(L, R, E, fb, fc);
  dst_[0].process(L, R, E, idx(pDstBase_[0]), dc);
  dst_[1].process(L, R, E, idx(pDstBase_[1]), dc);
  if (post) filter_.process(L, R, E, fb, fc);
}

int ProcessingChain::latency() const noexcept { return dst_[0].latency() + dst_[1].latency(); }

int ProcessingChain::latencyFor(int location, const ParamValues& p, double) noexcept {
  static constexpr ParamId kDst[3][2] = {{ParamId::DstWt1Type, ParamId::DstWt2Type},
                                         {ParamId::DstFm1Type, ParamId::DstFm2Type},
                                         {ParamId::DstGl1Type, ParamId::DstGl2Type}};
  int lat = 0;
  for (int k = 0; k < 2; ++k) {
    const int base = idx(kDst[location][k]);
    lat += DestructionSlot::latencyFor(static_cast<int>(p.v[static_cast<size_t>(base)] + 0.5f),
                                       static_cast<int>(p.v[static_cast<size_t>(base + static_cast<int>(DstOff::S2Fft))] + 0.5f));
  }
  return lat;
}

bool ProcessingChain::autonomousActive() const noexcept {
  return filter_.selfOscillating() || dst_[0].frozen() || dst_[1].frozen();
}

void ProcessingChain::tailGuardPurge() noexcept { reset(); }

}  // namespace sbd
