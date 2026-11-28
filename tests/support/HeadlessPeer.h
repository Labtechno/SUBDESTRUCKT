// SUBDESTRUCKT — tests R2-B2 : pair de test commun (reproductions de gestes, banc GC-01).
// Remplace SEULEMENT la couche fenêtre de l'OS (empilement des fenêtres, rendu à l'écran) : un composant hôte posé sur le
// bureau avec ce pair reçoit les événements injectés par ComponentPeer::handleMouseEvent / handleMouseWheel, dispatchés
// par le code réel de JUCE (MouseInputSource, Component). Les zones invalidées (repaint) sont enregistrées pour mesurer le
// coût réel d'un rafraîchissement sans fenêtre. Aucun code de production n'en dépend.
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace sbd::testsupport {

class TestPeer final : public juce::ComponentPeer {
 public:
  TestPeer(juce::Component& c, int flags) : juce::ComponentPeer(c, flags) {}
  void* getNativeHandle() const override { return nullptr; }
  void setVisible(bool v) override { visible_ = v; }
  void setTitle(const juce::String&) override {}
  void setBounds(const juce::Rectangle<int>& r, bool) override { bounds_ = r; }
  juce::Rectangle<int> getBounds() const override { return bounds_; }
  juce::Point<float> localToGlobal(juce::Point<float> p) override { return p + bounds_.getPosition().toFloat(); }
  juce::Point<float> globalToLocal(juce::Point<float> p) override { return p - bounds_.getPosition().toFloat(); }
  using juce::ComponentPeer::globalToLocal;
  using juce::ComponentPeer::localToGlobal;
  void setMinimised(bool) override {}
  bool isMinimised() const override { return false; }
  bool isShowing() const override { return visible_; }
  void setFullScreen(bool) override {}
  bool isFullScreen() const override { return false; }
  void setIcon(const juce::Image&) override {}
  bool contains(juce::Point<int> p, bool) const override { return bounds_.withZeroOrigin().contains(p); }
  OptionalBorderSize getFrameSizeIfPresent() const override { return {}; }
  juce::BorderSize<int> getFrameSize() const override { return {}; }
  bool setAlwaysOnTop(bool) override { return true; }
  void toFront(bool) override {}
  void toBehind(juce::ComponentPeer*) override {}
  bool isFocused() const override { return true; }
  void grabFocus() override {}
  void repaint(const juce::Rectangle<int>& area) override {
    dirty.add(area.getIntersection(bounds_.withZeroOrigin()));
    ++repaintCalls;
  }
  void performAnyPendingRepaintsNow() override {}
  void setAlpha(float) override {}
  juce::StringArray getAvailableRenderingEngines() override { return {"test"}; }
  void textInputRequired(juce::Point<int>, juce::TextInputTarget&) override {}

  juce::RectangleList<int> dirty;  // zones invalidées depuis le dernier rendu (coordonnées du composant hôte)
  int repaintCalls = 0;

 private:
  juce::Rectangle<int> bounds_;
  bool visible_ = false;
};

class TestHost final : public juce::Component {
 protected:
  juce::ComponentPeer* createNewPeer(const juce::DesktopWindowOptions& o) override { return new TestPeer(*this, o.getStyleFlags()); }
};

}  // namespace sbd::testsupport
