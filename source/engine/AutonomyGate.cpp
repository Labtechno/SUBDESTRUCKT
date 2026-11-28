#include "AutonomyGate.h"

namespace sbd {

const char* authSourceName(AuthSource s) noexcept {
  switch (s) {
    case AuthSource::None: return "aucune";
    case AuthSource::Initial: return "instance neuve";
    case AuthSource::Explicit: return "explicite";
    case AuthSource::Transport: return "transport";
    case AuthSource::Record: return "enregistrement";
    case AuthSource::Render: return "rendu hors temps reel";
  }
  return "?";
}

const char* taEventName(TaEventType t) noexcept {
  static const char* k[] = {"transport inconnu", "transport arret", "transport lecture", "debut d'enregistrement",
                            "hors temps reel", "temps reel", "autorisation", "suspension", "revocation (Panic)",
                            "capture appliquee", "commande perimee", "autorisation bloquee par Panic",
                            "fin de l'autorisation de rendu"};
  const auto i = static_cast<int>(t);
  return i >= 0 && i < static_cast<int>(TaEventType::Count) ? k[i] : "?";
}

void AutonomyGate::grant(AuthSource s, std::uint64_t clock, TaLog& log) noexcept {
  authorized_ = true;
  source_ = s;
  ++grants_;
  log.push(TaEvent{clock, TaEventType::Grant, s, gen_});
}

bool AutonomyGate::beginBlock(TransportObs obs, bool recording, bool offline, bool panicInBlock, std::uint64_t clock,
                              TaLog& log) noexcept {
  const bool was = authorized();
  realtimeReturn_ = false;
  // (1) suspension : nouvelle génération de chargement (retire l'autorisation normale ET l'autorisation de rendu)
  const std::uint32_t g = genCmd_.load(std::memory_order_acquire);
  if (g != gen_) {
    gen_ = g;
    authorized_ = false;
    render_ = false;
    source_ = AuthSource::None;
    log.push(TaEvent{clock, TaEventType::Suspend, AuthSource::None, gen_});
  }
  // (2) mode de traitement. Entrée en rendu : l'historique temps réel est figé. Sortie (rendu terminé ou annulé) : fin
  // de l'autorisation temporaire de rendu à CE bloc (aucune temporisation) ; l'historique temps réel devra être rétabli
  // par une observation connue avant toute arête (ARCH-UI-04-R1, R-28).
  if (offline != offline_) {
    offline_ = offline;
    log.push(TaEvent{clock, offline ? TaEventType::OfflineOn : TaEventType::OfflineOff, AuthSource::None, gen_});
    if (offline) {
      offObs_ = TransportObs::Unknown;
    } else {
      if (render_) {
        render_ = false;
        log.push(TaEvent{clock, TaEventType::RenderEnd, AuthSource::Render, gen_});
      }
      realtimeReturn_ = true;
      resync_ = true;
    }
  }
  // (3) transport
  bool startPlay = false, startRec = false;
  if (!offline_) {  // historique temps réel : seul fondement des arêtes ARRÊT -> LECTURE et du verrou Panic
    if (obs == TransportObs::Stopped) {
      if (last_ != TransportObs::Stopped) log.push(TaEvent{clock, TaEventType::TransportStop, AuthSource::None, gen_});
      last_ = TransportObs::Stopped;
      lastRec_ = false;
      latch_ = false;  // ARRÊT réel : fin du verrou Panic
      resync_ = false;
    } else if (obs == TransportObs::Rolling) {
      startPlay = !resync_ && last_ == TransportObs::Stopped;
      startRec = !resync_ && recording && !lastRec_ && last_ == TransportObs::Rolling;  // punch-in pendant la lecture
      if (last_ != TransportObs::Rolling) log.push(TaEvent{clock, TaEventType::TransportPlay, AuthSource::None, gen_});
      if (startRec) log.push(TaEvent{clock, TaEventType::RecordStart, AuthSource::None, gen_});
      last_ = TransportObs::Rolling;
      lastRec_ = recording;
      resync_ = false;  // première observation après un rendu : nouvelle base, jamais une arête
    }
  } else if (obs != offObs_) {  // pendant le rendu : journalisé (source Render) sans effet sur l'historique ni le verrou
    offObs_ = obs;
    if (obs == TransportObs::Stopped) log.push(TaEvent{clock, TaEventType::TransportStop, AuthSource::Render, gen_});
    if (obs == TransportObs::Rolling) log.push(TaEvent{clock, TaEventType::TransportPlay, AuthSource::Render, gen_});
  }
  // (4) candidats d'autorisation : normaux (explicite, transport et enregistrement temps réel) ; rendu (temporaire)
  const std::uint64_t u = userCmd_.load(std::memory_order_acquire);
  const std::uint32_t useq = static_cast<std::uint32_t>(u);
  bool explicitCmd = false;
  if (useq != seenUser_.load(std::memory_order_relaxed)) {
    seenUser_.store(useq, std::memory_order_release);
    if (static_cast<std::uint32_t>(u >> 32) == gen_)
      explicitCmd = true;
    else
      log.push(TaEvent{clock, TaEventType::StaleCommand, AuthSource::Explicit, gen_});
  }
  AuthSource cand = AuthSource::None;
  if (explicitCmd)
    cand = AuthSource::Explicit;
  else if (!latch_ && startPlay)
    cand = AuthSource::Transport;
  else if (!latch_ && startRec)
    cand = AuthSource::Record;
  else if (!latch_ && offline_ && obs == TransportObs::Rolling && !authorized())
    cand = AuthSource::Render;
  // (5) Panic dans le bloc : priorité absolue, aucune autorisation accordée
  if (cand != AuthSource::None) {
    if (panicInBlock) {
      log.push(TaEvent{clock, TaEventType::GrantBlockedByPanic, cand, gen_});
    } else if (cand == AuthSource::Render) {
      render_ = true;  // temporaire : jamais promue en autorisation normale
      ++grants_;
      log.push(TaEvent{clock, TaEventType::Grant, AuthSource::Render, gen_});
    } else if (!authorized_ || cand == AuthSource::Explicit) {
      if (cand == AuthSource::Explicit) latch_ = false;
      if (!authorized_) grant(cand, clock, log);
      render_ = false;  // une autorisation normale remplace l'autorisation temporaire de rendu
    }
  }
  seenGen_.store(gen_, std::memory_order_release);
  authorizedPub_.store(authorized(), std::memory_order_release);
  normalPub_.store(authorized_, std::memory_order_release);
  return !was && authorized();
}

bool AutonomyGate::panic(std::uint64_t clock, TaLog& log) noexcept {
  const bool was = authorized();
  authorized_ = false;
  render_ = false;
  latch_ = true;
  source_ = AuthSource::None;
  ++revocations_;
  log.push(TaEvent{clock, TaEventType::Revoke, AuthSource::None, gen_});
  authorizedPub_.store(false, std::memory_order_release);
  normalPub_.store(false, std::memory_order_release);
  return was;
}

}  // namespace sbd
