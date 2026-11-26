// SUBDESTRUCKT — L2 — publication lock-free d'objets compilés hors RT (V1-SOFTWARE-ARCHITECTURE §4).
//
// Thread de contrôle (message/UI) : publish() dépose un objet préparé hors RT ; collectGarbage() libère les objets
// retirés. Thread audio : acquire() prend l'objet en attente (échange atomique), retire() rend l'ancien via une file
// SPSC de capacité fixe. Le thread audio n'alloue ni ne libère jamais.
// Si un objet publié n'a pas encore été pris par le thread audio, publish() le remplace et le libère lui-même.
#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <memory>

#include "../core/Topology.h"

namespace sbd {

template <typename T>
class SpscExchange {
 public:
  SpscExchange() = default;
  SpscExchange(const SpscExchange&) = delete;
  SpscExchange& operator=(const SpscExchange&) = delete;
  ~SpscExchange() {
    delete pending_.exchange(nullptr);
    collectGarbage();
  }

  // ---- thread de contrôle
  void publish(std::unique_ptr<T> t) {
    T* old = pending_.exchange(t.release(), std::memory_order_acq_rel);
    delete old;  // jamais vu par le thread audio
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
  T* acquire() noexcept { return pending_.exchange(nullptr, std::memory_order_acq_rel); }
  bool canRetire() const noexcept {
    return head_.load(std::memory_order_relaxed) - tail_.load(std::memory_order_acquire) < kCapacity;
  }
  bool retire(T* t) noexcept {
    if (!canRetire()) return false;
    const std::size_t head = head_.load(std::memory_order_relaxed);
    ring_[head % kCapacity] = t;
    head_.store(head + 1, std::memory_order_release);
    return true;
  }

 private:
  static constexpr std::size_t kCapacity = 64;
  std::atomic<T*> pending_{nullptr};
  std::array<T*, kCapacity> ring_{};
  std::atomic<std::size_t> head_{0};  // écrit par le thread audio
  std::atomic<std::size_t> tail_{0};  // écrit par le thread de contrôle
};

using TopologyExchange = SpscExchange<CompiledTopology>;

}  // namespace sbd
