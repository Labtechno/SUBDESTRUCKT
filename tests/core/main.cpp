// SUBDESTRUCKT — tests natifs du cœur (sans hôte, sans JUCE). Identifiants de critères : docs/V1-ARCH-G3-PLAN.md §7.
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
void runAuditTests();
void runG42Tests();
void runG43Tests();
void runG43TaTests();
void runG43D2Tests();
void runG43ZTests();
int main() {
  std::printf("SUBDESTRUCKT ARCH-G3 — tests natifs du coeur\n");
  // SBD_ONLY=<groupe> : exécution ciblée pendant le développement (la CI exécute toujours la suite complète)
  const char* only = std::getenv("SBD_ONLY");
  auto run = [only](const char* name, void (*fn)()) {
    if (!only || std::strcmp(only, name) == 0) fn();
  };
  run("dsp", runDspTests);
  run("topology", runTopologyTests);
  run("state", runStateTests);
  run("engine", runEngineTests);
  run("g4", runG4Tests);
  run("g4features", runG4FeatureTests);
  run("filters", runFilterTests);
  run("destruction", runDestructionTests);
  run("state2", runStateTests2);
  run("g41", runG41Tests);
  run("audit", runAuditTests);
  run("g42", runG42Tests);
  run("g43", runG43Tests);
  run("g43ta", runG43TaTests);
  run("g43d2", runG43D2Tests);
  run("g43z", runG43ZTests);
  run("perf", runPerfTests);
  return tk::finish("core_tests");
}
