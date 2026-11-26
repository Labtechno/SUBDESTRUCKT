// SUBDESTRUCKT — L3 — générateurs pseudo-aléatoires déterministes (architecture §3.10, arbitrage Preset 8B).
// PCG32 (O'Neill, XSH-RR 64/32) et xoshiro128** (Blackman/Vigna) ; benchmark et choix : tests/core/test_rng.cpp et
// docs/V1-ARCH-G4-TRACKER.md. L'API masque l'algorithme : RngStream encapsule le modèle retenu.
// Aucun état global ; chaque flux possède (seed, streamId, compteur) explicites et reproductibles.
#pragma once
#include <cstdint>

namespace sbd {

struct Pcg32 {
  std::uint64_t state = 0x853c49e6748fea9bULL, inc = 0xda3e39cb94b95bdbULL;
  void seed(std::uint64_t initState, std::uint64_t seq) noexcept {
    state = 0u;
    inc = (seq << 1u) | 1u;
    next();
    state += initState;
    next();
  }
  std::uint32_t next() noexcept {
    const std::uint64_t old = state;
    state = old * 6364136223846793005ULL + inc;
    const std::uint32_t xorshifted = static_cast<std::uint32_t>(((old >> 18u) ^ old) >> 27u);
    const std::uint32_t rot = static_cast<std::uint32_t>(old >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31u));
  }
};

struct Xoshiro128ss {
  std::uint32_t s[4] = {1, 2, 3, 4};
  static std::uint32_t rotl(std::uint32_t x, int k) noexcept { return (x << k) | (x >> (32 - k)); }
  void seed(std::uint64_t seed, std::uint64_t seq) noexcept {
    // initialisation par SplitMix64 (recommandation des auteurs)
    std::uint64_t z = seed ^ (seq * 0x9E3779B97F4A7C15ULL);
    for (int i = 0; i < 4; i += 2) {
      z += 0x9E3779B97F4A7C15ULL;
      std::uint64_t x = z;
      x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
      x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
      x ^= x >> 31;
      s[i] = static_cast<std::uint32_t>(x);
      s[i + 1] = static_cast<std::uint32_t>(x >> 32);
    }
    if ((s[0] | s[1] | s[2] | s[3]) == 0) s[0] = 1;
  }
  std::uint32_t next() noexcept {
    const std::uint32_t result = rotl(s[1] * 5, 7) * 9;
    const std::uint32_t t = s[1] << 9;
    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rotl(s[3], 11);
    return result;
  }
};

// Identifiants de flux stables (StableStreamID).
enum class StreamId : std::uint32_t {
  Audio = 1, Randomize = 2, Mutate = 3, Evolve = 4, Lfo1 = 10, Lfo2 = 11, Lfo3 = 12, Lfo4 = 13, RandomHold = 20,
  RandomSmooth = 21, Chaos = 22, WtPhase = 30, OpPhase = 31, ZMutation = 32, Destruction = 40
};

// Flux du modèle retenu (PCG32, voir tracker : décision RNG-01). Compteur = nombre de tirages depuis l'initialisation.
class RngStream {
 public:
  static constexpr int kModelVersion = 1;  // version du modèle, persistée avec le seed
  void init(std::uint64_t seed, StreamId id) noexcept {
    seed_ = seed;
    id_ = id;
    g_.seed(seed, static_cast<std::uint64_t>(id));
    count_ = 0;
  }
  // Repositionne le flux après `counter` tirages (restauration Undo/Redo, chargement d'état).
  void seek(std::uint64_t counter) noexcept {
    init(seed_, id_);
    while (count_ < counter) nextU32();
  }
  std::uint32_t nextU32() noexcept {
    ++count_;
    return g_.next();
  }
  double uniform() noexcept { return (nextU32() >> 8) * (1.0 / 16777216.0); }       // [0,1)
  double bipolar() noexcept { return 2.0 * uniform() - 1.0; }                        // [-1,1)
  std::uint64_t counter() const noexcept { return count_; }
  std::uint64_t seed() const noexcept { return seed_; }

 private:
  Pcg32 g_;
  std::uint64_t seed_ = 0;
  StreamId id_ = StreamId::Audio;
  std::uint64_t count_ = 0;
};

}  // namespace sbd
