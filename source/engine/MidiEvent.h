// SUBDESTRUCKT — L2 — événements MIDI déjà décodés par le wrapper, avec offset échantillon (P0-C10).
// Ordre déterministe dans un même échantillon (P0-C11) : sécurité/transport -> expression -> note-off -> note-on.
#pragma once
#include <cstdint>

namespace sbd {

struct MidiEvent {
  enum class Type : std::uint8_t {
    AllSoundOff = 0, AllNotesOff = 1, NoteOff = 2, NoteOn = 3, PitchBend = 4, Controller = 5, ChannelPressure = 6
  };
  int offset = 0;            // échantillon hôte dans le bloc
  Type type = Type::NoteOn;
  std::uint8_t note = 60;    // note, ou numéro de contrôleur
  std::uint8_t velocity = 100;
  std::int16_t value = 0;    // pitch bend -8192..8191, valeur CC/pression 0..127

  static MidiEvent noteOn(int offset, int note, int velocity) {
    return MidiEvent{offset, Type::NoteOn, static_cast<std::uint8_t>(note), static_cast<std::uint8_t>(velocity), 0};
  }
  static MidiEvent noteOff(int offset, int note) {
    return MidiEvent{offset, Type::NoteOff, static_cast<std::uint8_t>(note), 0, 0};
  }
  static MidiEvent allSoundOff(int offset) { return MidiEvent{offset, Type::AllSoundOff, 0, 0, 0}; }
  static MidiEvent allNotesOff(int offset) { return MidiEvent{offset, Type::AllNotesOff, 0, 0, 0}; }
  static MidiEvent pitchBend(int offset, int value) {
    return MidiEvent{offset, Type::PitchBend, 0, 0, static_cast<std::int16_t>(value)};
  }
  static MidiEvent controller(int offset, int cc, int value) {
    return MidiEvent{offset, Type::Controller, static_cast<std::uint8_t>(cc), 0, static_cast<std::int16_t>(value)};
  }
  static MidiEvent channelPressure(int offset, int value) {
    return MidiEvent{offset, Type::ChannelPressure, 0, 0, static_cast<std::int16_t>(value)};
  }
};

// Rang dans un même échantillon : sécurité (0) < expression (1) < note-off (2) < note-on (3).
inline int eventRank(MidiEvent::Type t) noexcept {
  switch (t) {
    case MidiEvent::Type::AllSoundOff:
    case MidiEvent::Type::AllNotesOff: return 0;
    case MidiEvent::Type::PitchBend:
    case MidiEvent::Type::Controller:
    case MidiEvent::Type::ChannelPressure: return 1;
    case MidiEvent::Type::NoteOff: return 2;
    case MidiEvent::Type::NoteOn: return 3;
  }
  return 3;
}

}  // namespace sbd
