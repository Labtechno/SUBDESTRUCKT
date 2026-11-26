// ORACLE FIGE 0.1.0-b5 (copie de 8d23342, namespace sbd_b5) : ne jamais modifier. Reference de non-regression ARCH-G4.
// SUBDESTRUCKT — L2 — événements MIDI de la tranche, déjà décodés par le wrapper, avec offset échantillon.
// Ordre déterministe dans un même échantillon (P0-C11) : sécurité/transport -> expression -> note-off -> note-on.
#pragma once
#include <cstdint>

namespace sbd_b5 {

struct MidiEvent {
  enum class Type : std::uint8_t { AllSoundOff = 0, AllNotesOff = 1, NoteOff = 2, NoteOn = 3 };
  int offset = 0;            // échantillon hôte dans le bloc
  Type type = Type::NoteOn;
  std::uint8_t note = 60;
  std::uint8_t velocity = 100;

  static MidiEvent noteOn(int offset, int note, int velocity) {
    return MidiEvent{offset, Type::NoteOn, static_cast<std::uint8_t>(note), static_cast<std::uint8_t>(velocity)};
  }
  static MidiEvent noteOff(int offset, int note) {
    return MidiEvent{offset, Type::NoteOff, static_cast<std::uint8_t>(note), 0};
  }
  static MidiEvent allSoundOff(int offset) { return MidiEvent{offset, Type::AllSoundOff, 0, 0}; }
  static MidiEvent allNotesOff(int offset) { return MidiEvent{offset, Type::AllNotesOff, 0, 0}; }
};

// Rang dans un même échantillon : sécurité (0) < expression (1, aucune dans la tranche) < note-off (2) < note-on (3).
inline int eventRank(MidiEvent::Type t) noexcept {
  switch (t) {
    case MidiEvent::Type::AllSoundOff:
    case MidiEvent::Type::AllNotesOff: return 0;
    case MidiEvent::Type::NoteOff: return 2;
    case MidiEvent::Type::NoteOn: return 3;
  }
  return 3;
}

}  // namespace sbd_b5
