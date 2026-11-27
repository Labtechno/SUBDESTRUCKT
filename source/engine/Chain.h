// SUBDESTRUCKT — L2 — chaîne de traitement d'un emplacement (WT local, FM local, Global) : un filtre/résonateur et
// deux processeurs de destruction, ordre Pre/Post (filtre avant ou après les deux processeurs). Stéréo, au rythme
// moteur. Toute la mémoire est préallouée par configure() (hors RT) pour le pire cas : un changement d'algorithme
// est un événement structurel traité sans allocation (fondu de sortie, remise à zéro de l'état, fondu d'entrée).
#pragma once
#include <array>
#include <cstdint>

#include "../core/Parameters.h"
#include "../dsp/Destruction.h"
#include "../dsp/Filters.h"
#include "../dsp/Rng.h"

namespace sbd {

struct ChainContext {
  double fs = 48000.0;     // rythme moteur
  double noteHz = 261.63;  // fréquence de la note courante (keytracking)
  bool gate = false;
  bool noteOnEdge = false;     // note-on à cet échantillon (excitation Note)
  bool manualExcite = false;   // excitation manuelle (commande UI)
  bool voiceIdle = false;      // enveloppe Amp au repos (Tail)
  double bpm = 120.0;
  Quality quality = Quality::Normal;
};

class ProcessingChain {
 public:
  enum Location { Wt = 0, Fm = 1, Global = 2 };
  void configure(int location, double fs, Quality q);  // hors RT : préallocation pire cas
  void reset() noexcept;                                // Panic / prepare
  void process(double& L, double& R, const double* E, const ChainContext& c) noexcept;
  int latency() const noexcept;                         // échantillons moteur (alignement dry/wet interne compris)
  static int latencyFor(int location, const ParamValues& p, double fs, Quality q = Quality::Normal) noexcept;
  double y2Env(int slot) const noexcept { return dst_[static_cast<size_t>(slot)].y2Env(); }
  bool autonomousActive() const noexcept;
  DestructionSlot& slot(int k) noexcept { return dst_[static_cast<size_t>(k)]; }
  void tailGuardPurge() noexcept;  // garde-fou temporel des états non armés

 private:
  int location_ = 0;
  FilterSlot filter_;
  std::array<DestructionSlot, 2> dst_;
  ParamId pFilterBase_ = ParamId::FltWtType;
  std::array<ParamId, 2> pDstBase_{ParamId::DstWt1Type, ParamId::DstWt2Type};
};

}  // namespace sbd
