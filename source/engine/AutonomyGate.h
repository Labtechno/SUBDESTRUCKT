// SUBDESTRUCKT — L2 — TRANSPORT AUTHORIZATION (ARCH-G4.3, docs/V1-ARCH-G4.3-SPEC.md §3).
//
// Autorisation d'exécution des comportements autonomes (9 armements, Drone, restitution des captures R2) : donnée
// d'exécution, JAMAIS persistée. Machine d'états évaluée par le thread audio à chaque frontière de bloc, sans allocation,
// sans verrou, sans dépendance au thread message ni à l'éditeur (exports sans interface).
//
// Commandes du thread de contrôle (atomiques) : suspension (nouvelle génération de chargement) et autorisation
// explicite (porte la génération visée : une commande antérieure à une restauration plus récente est périmée).
// Sources d'autorisation : EXPLICITE (Autoriser, clic d'armement ou de Drone dans l'éditeur), TRANSPORT (ARRÊT -> LECTURE
// observé), RECORD (début d'enregistrement pendant la lecture), RENDER (hors temps réel en lecture, borné).
// Panic : révocation systématique, prioritaire sur toute autorisation du même bloc ; verrou levé par un ARRÊT réel ou
// une autorisation explicite. Un transport INCONNU (pas d'information) ne vaut jamais ARRÊT.
#pragma once
#include <array>
#include <atomic>
#include <cstdint>

namespace sbd {

enum class TransportObs : std::uint8_t { Unknown = 0, Stopped = 1, Rolling = 2 };
enum class AuthSource : std::uint8_t { None = 0, Initial, Explicit, Transport, Record, Render };
enum class TaEventType : std::uint8_t {
  TransportUnknown = 0, TransportStop, TransportPlay, RecordStart, OfflineOn, OfflineOff,
  Grant, Suspend, Revoke, CaptureApplied, StaleCommand, GrantBlockedByPanic, Count
};
const char* authSourceName(AuthSource s) noexcept;
const char* taEventName(TaEventType t) noexcept;

struct TaEvent {
  std::uint64_t sample = 0;  // horloge hôte de l'instance (échantillons traités) au début du bloc
  TaEventType type = TaEventType::TransportUnknown;
  AuthSource source = AuthSource::None;
  std::uint32_t generation = 0;
};

// File SPSC sans verrou (producteur : thread audio ; consommateur : thread message). Débordement compté.
class TaLog {
 public:
  static constexpr std::uint32_t kSize = 256;
  void push(const TaEvent& e) noexcept {
    const std::uint32_t w = write_.load(std::memory_order_relaxed);
    if (w - read_.load(std::memory_order_acquire) >= kSize) {
      dropped_.fetch_add(1, std::memory_order_relaxed);
      return;
    }
    ring_[w % kSize] = e;
    write_.store(w + 1, std::memory_order_release);
  }
  bool pop(TaEvent& e) noexcept {
    const std::uint32_t r = read_.load(std::memory_order_relaxed);
    if (r == write_.load(std::memory_order_acquire)) return false;
    e = ring_[r % kSize];
    read_.store(r + 1, std::memory_order_release);
    return true;
  }
  std::uint32_t dropped() const noexcept { return dropped_.load(std::memory_order_relaxed); }

 private:
  std::array<TaEvent, kSize> ring_{};
  std::atomic<std::uint32_t> write_{0}, read_{0}, dropped_{0};
};

struct TransportInfo;

class AutonomyGate {
 public:
  // ---- thread de contrôle
  void requestSuspend() noexcept { genCmd_.fetch_add(1, std::memory_order_acq_rel); }
  void requestAuthorize() noexcept {
    const std::uint32_t g = genCmd_.load(std::memory_order_acquire);
    std::uint64_t cur = userCmd_.load(std::memory_order_relaxed), next;
    do {
      next = (static_cast<std::uint64_t>(g) << 32) | ((cur + 1) & 0xFFFFFFFFULL);
    } while (!userCmd_.compare_exchange_weak(cur, next, std::memory_order_acq_rel));
  }
  // vue cohérente pour le thread de contrôle : commandes en attente comprises
  bool authorizedView() const noexcept {
    const std::uint32_t g = genCmd_.load(std::memory_order_acquire);
    const std::uint64_t u = userCmd_.load(std::memory_order_acquire);
    const bool userPending = static_cast<std::uint32_t>(u) != seenUser_.load(std::memory_order_acquire);
    if (userPending && static_cast<std::uint32_t>(u >> 32) == g) return true;
    if (g != seenGen_.load(std::memory_order_acquire)) return false;
    return authorizedPub_.load(std::memory_order_acquire);
  }

  // ---- thread audio
  // Frontière de bloc. panicInBlock : le bloc contient un All Sound Off (aucune autorisation accordée dans ce bloc).
  // Renvoie vrai si l'autorisation passe de retirée à accordée (restitution des captures retenues).
  bool beginBlock(TransportObs obs, bool recording, bool offline, bool panicInBlock, std::uint64_t clock, TaLog& log) noexcept;
  // Panic (All Sound Off) : révocation systématique et verrou. Renvoie vrai si l'autorisation était accordée.
  bool panic(std::uint64_t clock, TaLog& log) noexcept;
  bool authorized() const noexcept { return authorized_; }
  bool latched() const noexcept { return latch_; }
  AuthSource source() const noexcept { return source_; }
  TransportObs lastKnown() const noexcept { return last_; }
  std::uint32_t generation() const noexcept { return gen_; }
  std::uint32_t grants() const noexcept { return grants_; }
  std::uint32_t revocations() const noexcept { return revocations_; }
  bool offline() const noexcept { return offline_; }

 private:
  void grant(AuthSource s, std::uint64_t clock, TaLog& log) noexcept;
  // commandes
  std::atomic<std::uint32_t> genCmd_{0};
  std::atomic<std::uint64_t> userCmd_{0};
  std::atomic<std::uint32_t> seenGen_{0}, seenUser_{0};
  std::atomic<bool> authorizedPub_{true};
  // état (thread audio)
  std::uint32_t gen_ = 0;
  bool authorized_ = true;  // instance neuve : autorisée (configuration par défaut sans état autonome)
  bool latch_ = false;
  bool lastRec_ = false;
  bool offline_ = false;
  TransportObs last_ = TransportObs::Unknown;  // dernier état connu (Unknown = jamais observé)
  AuthSource source_ = AuthSource::Initial;
  std::uint32_t grants_ = 0, revocations_ = 0;
};

}  // namespace sbd
