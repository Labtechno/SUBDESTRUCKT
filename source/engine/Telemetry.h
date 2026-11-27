// SUBDESTRUCKT — L2 — télémétrie RT -> UI (V1-SOFTWARE-ARCHITECTURE §3.15).
// Valeurs atomiques écrites par le thread audio en fin de bloc (relaxed), lues par l'UI à sa cadence. Aucune
// contre-pression sur l'audio ; perte de valeurs intermédiaires autorisée. Les captures (scope) passent par
// ScopeRing, file bornée sans verrou.
#pragma once
#include <array>
#include <atomic>
#include <cstdint>

#include "../core/ModMatrix.h"
#include "../core/Topology.h"

namespace sbd {

// Anneau de capture décimée pour l'oscilloscope/spectre (écrit par l'audio, lu par l'UI ; pertes autorisées).
struct ScopeRing {
  static constexpr int kSize = 4096;
  std::array<std::atomic<float>, kSize> data{};
  std::atomic<std::uint32_t> write{0};
  void push(float v) noexcept {
    const std::uint32_t w = write.load(std::memory_order_relaxed);
    data[w % kSize].store(v, std::memory_order_relaxed);
    write.store(w + 1, std::memory_order_release);
  }
};

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
  std::atomic<int> quality{1};

  // état voix
  std::atomic<int> currentNote{-1};
  std::atomic<bool> gate{false};
  std::atomic<bool> drone{false};
  std::atomic<bool> autonomySuspended{false};  // armement(s) configuré(s) mais autorisation locale retirée (AUT)
  std::atomic<float> envelope{0.0f};

  // sortie
  std::atomic<float> peakL{0.0f}, peakR{0.0f};
  ScopeRing scope;

  // sûreté
  std::atomic<std::uint32_t> nonFiniteResets{0};
  std::atomic<std::uint32_t> outputClampHits{0};
  std::atomic<std::uint32_t> pmClampHits{0};
  std::atomic<std::uint32_t> droppedEvents{0};
  std::atomic<std::uint32_t> panics{0};
  std::atomic<std::uint32_t> limiterHits{0};
  std::atomic<std::uint32_t> elpHits{0};  // SPEC-HR-01 : engagements de la protection d'extrême niveau

  // topologie, matrice et Effective
  std::atomic<std::uint64_t> activeSerial{0};
  std::atomic<std::uint64_t> modSerial{0};
  std::atomic<int> transitionPhase{0};  // 0 repos, 1 fondu de sortie, 2 fondu d'entrée
  std::array<std::atomic<float>, kMaxRoutes> effectiveDepth{};  // après Master/direction et lissage
  std::array<std::atomic<float>, kMaxRoutes> edgeGain{};        // gain de transition par slot (0 si absent)
  std::atomic<float> wtX{0.0f}, wtY{0.0f}, wtZ{0.0f};          // positions Effective
  std::array<std::atomic<float>, kNumModSources> sources{};     // valeurs des sources de modulation
  std::array<std::atomic<float>, kNumParams> effective{};       // valeur Effective des destinations modulées
  std::array<std::atomic<std::uint8_t>, kNumParams> modulated{};  // 1 si la destination est modulée (matrice active)

  // performance (mesurée par le wrapper, hors moteur)
  std::atomic<float> cpuLoad{0.0f};  // durée de traitement / durée du bloc (dernier bloc)
};

}  // namespace sbd
