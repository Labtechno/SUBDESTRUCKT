// SUBDESTRUCKT — L1/L2 — matrice de modulation compilée (forme RT, taille fixe) et évaluation.
// compileModMatrix() s'exécute hors RT : statuts d'inactivité, ordre (connexions vers macros d'abord, en ordre
// topologique des macros), destinations uniques. Le moteur évalue la forme compilée sans allocation.
#pragma once
#include <array>
#include <cstdint>
#include <memory>

#include "../core/ModMatrix.h"
#include "../core/Parameters.h"

namespace sbd {

struct CompiledModRoute {
  std::uint32_t id = 0;
  std::uint8_t src = 0, via = 0, curve = 0, numPoints = 2;
  bool centered = true, invert = false, audio = false, srcBipolar = false, viaBipolar = false;
  std::int16_t dest = 0;
  std::uint8_t destSlot = 0;     // indice dans dests[]
  float depth = 0.0f, offset = 0.0f, viaAmount = 1.0f, smoothMs = 0.0f;
  double scale = 1.0;            // modScale de la destination
  std::array<float, kMaxCurvePoints> px{}, py{};
};

struct CompiledMod {
  std::array<CompiledModRoute, kMaxModRoutes> routes{};
  int numRoutes = 0;
  int numMacroRoutes = 0;                         // routes[0..numMacroRoutes) visent des macros (ordre topologique)
  std::array<std::int16_t, kMaxModRoutes> dests{};  // destinations uniques (macros d'abord)
  int numDests = 0;
  int numMacroDests = 0;
  std::array<std::uint8_t, kMaxModRoutes> status{};  // ModInactive par route de l'état source (diagnostic)
  std::uint64_t serial = 0;
};

// Hors RT : compile l'état (doit être valide).
std::unique_ptr<CompiledMod> compileModMatrix(const ModMatrixState& m);

}  // namespace sbd
