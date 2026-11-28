// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — NEURAL CELL Hybrid (DA-A04) : touches mécaniques en titane noir, témoins intégrés.
// Six traitements distincts : action instantanée, interrupteur, armement, sélection exclusive, sûreté (PANIC),
// commande désactivée. L'état est porté par la forme, le témoin et le texte, jamais par la seule couleur.
#pragma once
#include "r2/R2Style.h"
#include "ui/r2/R2Painters.h"

namespace sbd::r2 {

class NeuralCell : public juce::Button {
 public:
  using Kind = CellKind;
  using ArmState = CellArm;
  using Icon = CellIcon;

  NeuralCell(Kind kind, Domain domain, const juce::String& label, const juce::String& fullName);

  Kind kind() const { return kind_; }
  void setArmState(ArmState s) {
    arm_ = s;
    repaint();
  }
  ArmState armState() const { return arm_; }
  void setIcon(Icon i) {
    icon_ = i;
    repaint();
  }
  void setForced(Forced f) {
    forced_ = f;
    repaint();
  }
  void setDisabledReason(const juce::String& why);
  // PANIC : libellé blanc (0.4.1, 3,84:1) ou carbone (variante proposée, 5,08:1) — arbitrage du propriétaire
  void setSafetyDarkLabel(bool d) {
    safetyDark_ = d;
    repaint();
  }
  juce::String stateText() const;  // texte d'état affiché (OFF, ON, ARMED, ACTIVE, DENIED, —)
  void setRevision(Rev r) {
    rev_ = r;
    repaint();
  }
  Rev revision() const { return rev_; }
  // mesure de contraste sur la surface rendue : dessiner la touche sans son libellé (laboratoire uniquement)
  void setLabelHidden(bool h) {
    labelHidden_ = h;
    repaint();
  }

  void paintButton(juce::Graphics&, bool over, bool down) override;

 private:
  void paintIcon(juce::Graphics&, juce::Rectangle<float> r, juce::Colour c) const;
  void paintB1(juce::Graphics&, bool over, bool down);
  void paintR1(juce::Graphics&, bool over, bool down);
  Kind kind_;
  Domain domain_;
  juce::String full_;
  ArmState arm_ = ArmState::Off;
  Icon icon_ = Icon::None;
  Forced forced_ = Forced::None;
  bool safetyDark_ = false;  // B1 seulement : en R1, le libellé PANIC est toujours carbone (LAB-A07.1)
  bool labelHidden_ = false;
  Rev rev_ = Rev::R1;
};

}  // namespace sbd::r2
