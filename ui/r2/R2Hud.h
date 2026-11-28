// SUBDESTRUCKT — ARCH-UI-04-R2-B2 — Cybernetic HUD R1 (LAB-A04) : données d'afficheur et peintre partagé des neuf variantes.
// Contrat : aucune donnée inventée ; chaque afficheur nomme sa source ; la nature de chaque donnée est signalée (BASE, EFF,
// MEAS, INDICATIVE, STATE) ; DEMO DATA seulement dans le laboratoire (demo = false en production, obligatoire). Le peintre
// n'écrit rien, n'anime rien et ne lit aucune télémétrie : le consommateur fournit des données publiées.
#pragma once
#include <array>
#include <optional>
#include <vector>

#include "ui/r2/R2Style.h"

namespace sbd::r2 {

struct DisplayData {
  bool demo = true;
  bool measure = false;  // la valeur numérique est une MESURE (ex. crête) et non un paramètre BASE / EFF
  juce::String title, source;  // titre (principal) et source de la donnée (secondaire, obligatoire)
  // valeur numérique (R1 : échelle instrumentale avec repères BASE et EFF)
  juce::String value, unit, secondary, effText, minText, maxText;
  float baseNorm = -1.0f, effNorm = -1.0f;
  // oscilloscope (-1..1) et spectre (dB, bins linéaires de 0 à fs/2 du signal décimé)
  std::vector<float> samples, spectrumDb;
  float decimatedRate = 22050.0f;  // fs hôte / 2 (une valeur sur deux)
  // wavetable
  std::vector<std::vector<float>> frames;
  float posBase = 0.0f;
  std::optional<float> posEffective;
  // courbe de modulation (points normalisés)
  std::vector<juce::Point<float>> curve;
  float curveInput = -1.0f;  // < 0 : pas de marqueur d'entrée
  // enveloppe : DLY, ATK, HOLD, DEC, SUS (niveau 0..1), REL (durées relatives)
  std::array<float, 6> env{0.05f, 0.2f, 0.1f, 0.3f, 0.6f, 0.4f};
  std::optional<float> envLevel;  // niveau instantané : publié seulement pour AMP
  // crête (dBFS) et maintien
  float peakL = -120.0f, peakR = -120.0f, holdL = -120.0f, holdR = -120.0f;
  // lignes d'état et de diagnostic : niveau 0 = principal, 1 = secondaire ; témoin 0/1/2 (éteint, anneau, plein)
  struct Row {
    juce::String key, value;
    int level = 0, led = -1;
    juce::Colour colour;
  };
  std::vector<Row> rows;
};

enum class HudVariant { Numeric, Scope, Spectrum, Wavetable, ModCurve, Envelope, Meter, Autonomy, Diagnostic };

struct HudView {
  HudVariant variant;
  Domain domain;
  const DisplayData& data;
  juce::Rectangle<int> local;  // bornes locales du composant
};

void paintHud(juce::Graphics& g, const HudView& v);

}  // namespace sbd::r2
