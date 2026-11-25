// SUBDESTRUCKT — L2 — télémétrie RT -> UI (V1-SOFTWARE-ARCHITECTURE §3.15).
// Valeurs atomiques écrites par le thread audio en fin de bloc (relaxed), lues par l'UI à sa cadence. Aucune
// contre-pression sur l'audio ; perte de valeurs intermédiaires autorisée.
#pragma once
#include <array>
#include <atomic>
#include <cstdint>

#include "../core/Topology.h"

namespace sbd {

struct Telemetry {
  // configuration active (écrite par prepare(), hors RT)
  std::atomic<double> hostRate{0.0};
  std::atomic<double> engineRate{0.0};
  std::atomic<int> oversampling{1};
  std::atomic<double> delaySamples{0.0};
  std::atomic<bool> delayInteger{true};
  std::atomic<double> thiranCoefficient{0.0};
  std::atomic<int> latencySamples{0};
  std::atomic<bool> supported{false};

  // état voix
  std::atomic<int> currentNote{-1};
  std::atomic<bool> gate{false};
  std::atomic<float> envelope{0.0f};

  // sortie
  std::atomic<float> peakL{0.0f}, peakR{0.0f};

  // sûreté
  std::atomic<std::uint32_t> nonFiniteResets{0};
  std::atomic<std::uint32_t> outputClampHits{0};
  std::atomic<std::uint32_t> pmClampHits{0};
  std::atomic<std::uint32_t> droppedEvents{0};
  std::atomic<std::uint32_t> panics{0};

  // topologie et Effective
  std::atomic<std::uint64_t> activeSerial{0};
  std::atomic<int> transitionPhase{0};  // 0 repos, 1 fondu de sortie, 2 fondu d'entrée
  std::array<std::atomic<float>, kMaxRoutes> effectiveDepth{};  // rad, après Master/direction et lissage
  std::array<std::atomic<float>, kMaxRoutes> edgeGain{};        // gain de transition par slot (0 si absent)
};

}  // namespace sbd
