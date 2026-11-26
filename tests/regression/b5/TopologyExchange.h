// ORACLE FIGE 0.1.0-b5 (copie de 8d23342, namespace sbd_b5) : ne jamais modifier. Reference de non-regression ARCH-G4.
// SUBDESTRUCKT — L2 — publication lock-free des topologies compilées (V1-SOFTWARE-ARCHITECTURE §4, publication d'état).
//
// Thread de contrôle (message/UI) : publish() dépose une topologie préparée hors RT ; collectGarbage() libère les
// topologies retirées. Thread audio : acquire() prend la topologie en attente (échange atomique), retire() rend
// l'ancienne via une file SPSC de capacité fixe. Le thread audio n'alloue ni ne libère jamais.
// Si une topologie publiée n'a pas encore été prise par le thread audio, publish() la remplace et la libère lui-même
// (elle n'a jamais appartenu au thread audio).
#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <memory>

#include "Topology.h"

namespace sbd_b5 {

class TopologyExchange {
 public:
  TopologyExchange() = default;
  TopologyExchange(const TopologyExchange&) = delete;
  TopologyExchange& operator=(const TopologyExchange&) = delete;
  ~TopologyExchange() {
    delete pending_.exchange(nullptr);
    collectGarbage();
  }

  // ---- thread de contrôle
  void publish(std::unique_ptr<CompiledTopology> t) {
    CompiledTopology* old = pending_.exchange(t.release(), std::memory_order_acq_rel);
    delete old;  // jamais vue par le thread audio
  }
  void collectGarbage() {
    std::size_t tail = tail_.load(std::memory_order_relaxed);
    const std::size_t head = head_.load(std::memory_order_acquire);
    while (tail != head) {
      delete ring_[tail % kCapacity];
      ring_[tail % kCapacity] = nullptr;
      ++tail;
    }
    tail_.store(tail, std::memory_order_release);
  }
  bool hasPending() const noexcept { return pending_.load(std::memory_order_acquire) != nullptr; }

  // ---- thread audio
  CompiledTopology* acquire() noexcept { return pending_.exchange(nullptr, std::memory_order_acq_rel); }
  bool canRetire() const noexcept {
    return head_.load(std::memory_order_relaxed) - tail_.load(std::memory_order_acquire) < kCapacity;
  }
  bool retire(CompiledTopology* t) noexcept {
    if (!canRetire()) return false;
    const std::size_t head = head_.load(std::memory_order_relaxed);
    ring_[head % kCapacity] = t;
    head_.store(head + 1, std::memory_order_release);
    return true;
  }

 private:
  static constexpr std::size_t kCapacity = 64;
  std::atomic<CompiledTopology*> pending_{nullptr};
  std::array<CompiledTopology*, kCapacity> ring_{};
  std::atomic<std::size_t> head_{0};  // écrit par le thread audio
  std::atomic<std::size_t> tail_{0};  // écrit par le thread de contrôle
};

}  // namespace sbd_b5
