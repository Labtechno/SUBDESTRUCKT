// SUBDESTRUCKT — UI — moniteurs (V1-UI-UX-DESIGN §5) : données réelles uniquement, mesure et dessin hors thread
// audio, cadence de l'éditeur (30 Hz), désactivés quand masqués.
//  - Scope / Spectre : point de prélèvement = sortie du moteur (après décimation, avant gain de sortie), capture
//    décimée par 2 (télémétrie ScopeRing).
//  - XY : grille de la banque active, positions Base et Effective (Z n'est pas une dimension : barre séparée).
//  - Réseau FM/CROSS : graphe compilé réel (nœuds, arêtes, classes I/D, profondeur Effective, gain de transition).
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <vector>

#include "plugin/PluginProcessor.h"

namespace sbd::ui {

class ScopeView : public juce::Component {
 public:
  explicit ScopeView(SubdestrucktProcessor& p) : proc_(p) {}
  void refresh();
  void paint(juce::Graphics&) override;
  void setSpectrum(bool s) {
    spectrum_ = s;
    repaint();
  }

 private:
  SubdestrucktProcessor& proc_;
  std::vector<float> buf_ = std::vector<float>(2048, 0.0f);
  std::vector<float> mag_ = std::vector<float>(1024, -120.0f);
  bool spectrum_ = false;
};

class XYView : public juce::Component {
 public:
  explicit XYView(SubdestrucktProcessor& p) : proc_(p) {}
  void refresh();
  void paint(juce::Graphics&) override;

 private:
  SubdestrucktProcessor& proc_;
  float bx_ = 0, by_ = 0, ex_ = 0, ey_ = 0, z_ = 0;
  int nx_ = 1, ny_ = 1;
};

class NetworkView : public juce::Component {
 public:
  explicit NetworkView(SubdestrucktProcessor& p) : proc_(p) {}
  void refresh();
  void paint(juce::Graphics&) override;

 private:
  SubdestrucktProcessor& proc_;
  std::vector<Route> routes_;
  std::array<float, kMaxRoutes> depth_{}, gain_{};
};

// Moniteur commutable Scope / Spectre / XY / Réseau.
class MonitorPanel : public juce::Component {
 public:
  explicit MonitorPanel(SubdestrucktProcessor& p);
  void refresh();
  void resized() override;
  void paint(juce::Graphics&) override;

 private:
  void show(int k);
  juce::TextButton b_[4];
  ScopeView scope_;
  XYView xy_;
  NetworkView net_;
  int mode_ = 0;
};

}  // namespace sbd::ui
