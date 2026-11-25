// SUBDESTRUCKT — L3 DSP primitives — constantes communes.
#pragma once

namespace sbd {

constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;

// T2 : durée physique nominale du retard causal sur les arêtes Delayed (V1-CAUSAL-CONTRACT), indépendante de Fs et K.
constexpr double kT2Seconds = 1.0 / 48000.0;

}  // namespace sbd
