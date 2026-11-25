// SUBDESTRUCKT — mini cadre de test sans dépendance (cœur, wrapper, hôte).
// Chaque critère du plan (G-xx) produit une ligne « RESULT <id> PASS|FAIL <détail> » exploitée par la CI.
#pragma once
#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

namespace tk {

struct State {
  int checks = 0;
  int failures = 0;
  std::vector<std::string> results;  // lignes RESULT
};
inline State& state() {
  static State s;
  return s;
}

// Compteur d'allocations (operator new global surchargé dans l'exécutable de test). Le comptage est propre au thread
// qui l'active (le thread « audio » du test) : les allocations d'autres threads (minuteries JUCE, etc.) sont ignorées.
inline std::atomic<bool>& countingAllocations() {
  static thread_local std::atomic<bool> on{false};
  return on;
}
inline std::atomic<long long>& allocationCount() {
  static thread_local std::atomic<long long> n{0};
  return n;
}

inline bool check(bool ok, const char* file, int line, const char* expr) {
  auto& s = state();
  ++s.checks;
  if (!ok) {
    ++s.failures;
    std::fprintf(stderr, "  ECHEC %s:%d : %s\n", file, line, expr);
  }
  return ok;
}

inline void result(const char* id, bool pass, const char* fmt, ...) {
  char buf[1024];
  va_list ap;
  va_start(ap, fmt);
  std::vsnprintf(buf, sizeof buf, fmt, ap);
  va_end(ap);
  std::string line = std::string("RESULT ") + id + " " + (pass ? "PASS" : "FAIL") + " " + buf;
  std::printf("%s\n", line.c_str());
  std::fflush(stdout);
  state().results.push_back(line);
  if (!pass) ++state().failures;
}

inline void info(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::printf("  ");
  std::vprintf(fmt, ap);
  std::printf("\n");
  va_end(ap);
}

inline int finish(const char* suite) {
  auto& s = state();
  std::printf("\n%s : %d controles, %d echecs, %zu criteres rapportes\n", suite, s.checks, s.failures, s.results.size());
  return s.failures == 0 ? 0 : 1;
}

}  // namespace tk

#define TK_CHECK(cond) tk::check(static_cast<bool>(cond), __FILE__, __LINE__, #cond)
