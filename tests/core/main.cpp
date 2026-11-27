// SUBDESTRUCKT — tests natifs du cœur (sans hôte, sans JUCE). Identifiants de critères : docs/V1-ARCH-G3-PLAN.md §7.
#include <cstdio>
#include <cstdlib>
#include <new>

#include "TestKit.h"

// ---- compteur d'allocations (G-18) : toute allocation pendant Engine::process est comptée
void* operator new(std::size_t n) {
  if (tk::countingAllocations().load(std::memory_order_relaxed)) tk::allocationCount().fetch_add(1);
  if (void* p = std::malloc(n ? n : 1)) return p;
  throw std::bad_alloc();
}
void* operator new[](std::size_t n) { return operator new(n); }
void operator delete(void* p) noexcept {
  if (p && tk::countingAllocations().load(std::memory_order_relaxed)) tk::allocationCount().fetch_add(1);
  std::free(p);
}
void operator delete[](void* p) noexcept { operator delete(p); }
void operator delete(void* p, std::size_t) noexcept { operator delete(p); }
void operator delete[](void* p, std::size_t) noexcept { operator delete(p); }

void runDspTests();
void runTopologyTests();
void runStateTests();
void runEngineTests();
void runG4Tests();
void runG4FeatureTests();
void runFilterTests();
void runDestructionTests();
void runStateTests2();
void runPerfTests();
void runG41Tests();

int main() {
  std::printf("SUBDESTRUCKT ARCH-G3 — tests natifs du coeur\n");
  runDspTests();
  runTopologyTests();
  runStateTests();
  runEngineTests();
  runG4Tests();
  runG4FeatureTests();
  runFilterTests();
  runDestructionTests();
  runStateTests2();
  runG41Tests();
  runPerfTests();
  return tk::finish("core_tests");
}
