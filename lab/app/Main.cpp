// SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab — point d'entrée.
//
//   sbd_component_lab                          fenêtre interactive (planche complète, DEMO DATA)
//   sbd_component_lab --smoke                  ouvre la fenêtre puis la ferme (contrôle de démarrage)
//   sbd_component_lab --render <dir> [--scale 2]
//   sbd_component_lab --measure <proposals.csv> <out.csv>      PT-02
//   sbd_component_lab --glyphs <out.csv>                       PT-05
//   sbd_component_lab --contrast <out.csv>
//   sbd_component_lab --selftest
//   sbd_component_lab --all <dir> --csv <proposals.csv> [--scale 2]   tout ce qui précède (CI)
#include <fstream>
#include <iostream>

#include "app/Measure.h"
#include "app/SelfTest.h"
#include "app/Sheets.h"

namespace {

using namespace sbd::lab;

class LabWindow : public juce::DocumentWindow, private juce::Timer {
 public:
  explicit LabWindow(int closeAfterMs)
      : juce::DocumentWindow("SUBDESTRUCKT R2 Component Lab (DEMO DATA)", sbd::ui::tok::chassis, juce::DocumentWindow::allButtons) {
    board_ = buildBoard();
    view_.setViewedComponent(board_.get(), false);
    setContentNonOwned(&view_, false);
    setResizable(true, true);
    centreWithSize(1440, 900);
    setUsingNativeTitleBar(true);
    setVisible(true);
    if (closeAfterMs > 0) startTimer(closeAfterMs);
  }
  ~LabWindow() override { view_.setViewedComponent(nullptr, false); }
  void closeButtonPressed() override { juce::MessageManager::getInstance()->stopDispatchLoop(); }

 private:
  void timerCallback() override {
    stopTimer();
    shown_ = isShowing();
    std::cout << "SMOKE window shown " << (shown_ ? "yes" : "no") << std::endl;
    closeButtonPressed();
  }
  std::unique_ptr<Sheet> board_;
  juce::Viewport view_;
  bool shown_ = false;
};

juce::File arg(const juce::StringArray& a, const char* key, int n = 1) {
  const int i = a.indexOf(key);
  if (i < 0 || i + n >= a.size()) return {};
  return juce::File::getCurrentWorkingDirectory().getChildFile(a[i + n]);
}

void emit(const juce::StringArray& lines, const juce::File& logFile) {
  for (const auto& l : lines) std::cout << l.toStdString() << std::endl;
  if (logFile != juce::File()) logFile.appendText(lines.joinIntoString("\n") + "\n", false, false, "\n");
}

}  // namespace

int main(int argc, char** argv) {
  juce::ScopedJuceInitialiser_GUI gui;
  juce::StringArray a;
  for (int i = 1; i < argc; ++i) a.add(juce::String::fromUTF8(argv[i]));
  const float scale = a.contains("--scale") ? a[a.indexOf("--scale") + 1].getFloatValue() : 2.0f;
  int rc = 0;
  juce::StringArray log;

  if (a.contains("--all")) {
    const auto dir = arg(a, "--all");
    const auto csv = arg(a, "--csv");
    dir.createDirectory();
    const auto logFile = dir.getChildFile("lab-run.log");
    logFile.replaceWithText("platform " + platformLine() + "\n");
    rc |= renderSheets(dir.getChildFile("CAPTURES"), scale, log) ? 0 : 1;
    if (csv.existsAsFile()) rc |= runMeasure(csv, dir.getChildFile("typography-measurements.csv"), log);
    else log.add("PT-02 skipped: proposals CSV not found");
    rc |= runGlyphs(dir.getChildFile("glyphs.csv"), log);
    rc |= runContrast(dir.getChildFile("contrast.csv"), log);
    juce::StringArray st;
    rc |= runSelfTest(st);
    log.addArray(st);
    emit(log, logFile);
    return rc;
  }
  if (a.contains("--render")) {
    rc = renderSheets(arg(a, "--render"), scale, log) ? 0 : 1;
    emit(log, {});
    return rc;
  }
  if (a.contains("--measure")) {
    rc = runMeasure(arg(a, "--measure"), arg(a, "--measure", 2), log);
    emit(log, {});
    return rc;
  }
  if (a.contains("--glyphs")) {
    rc = runGlyphs(arg(a, "--glyphs"), log);
    emit(log, {});
    return rc;
  }
  if (a.contains("--contrast")) {
    rc = runContrast(arg(a, "--contrast"), log);
    emit(log, {});
    return rc;
  }
  if (a.contains("--contact")) {  // planche-contact : vignettes des PNG d'un dossier
    const auto dir = arg(a, "--contact");
    const auto out = arg(a, "--contact", 2);
    juce::Array<juce::File> files = dir.findChildFiles(juce::File::findFiles, false, "*.png");
    files.sort();
    const int cols = 3, tw = 620, pad = 20, capH = 22;
    std::vector<juce::Image> thumbs;
    int rowH = 0;
    for (auto& f : files) {
      auto img = juce::ImageFileFormat::loadFrom(f);
      if (!img.isValid()) continue;
      const int th = juce::roundToInt(static_cast<double>(img.getHeight()) * tw / img.getWidth());
      thumbs.push_back(img.rescaled(tw, th, juce::Graphics::highResamplingQuality));
      rowH = std::max(rowH, th);
    }
    const int rows = (static_cast<int>(thumbs.size()) + cols - 1) / cols;
    juce::Image sheet(juce::Image::RGB, pad + cols * (tw + pad), 70 + rows * (rowH + capH + pad), true);
    juce::Graphics g(sheet);
    g.fillAll(sbd::ui::tok::chassis);
    g.setColour(sbd::ui::tok::text);
    g.setFont(sbd::r2::labelFont(20.0f, true));
    g.drawText("R2-B1 Component Lab  -  contact sheet  -  " + juce::SystemStats::getOperatingSystemName(), pad, 14, sheet.getWidth() - 2 * pad, 26,
               juce::Justification::centredLeft);
    g.setFont(sbd::r2::labelFont(10.0f));
    g.setColour(sbd::ui::tok::textMute);
    g.drawText(platformLine(), pad, 42, sheet.getWidth() - 2 * pad, 14, juce::Justification::centredLeft);
    for (size_t k = 0; k < thumbs.size(); ++k) {
      const int x = pad + static_cast<int>(k % cols) * (tw + pad), y = 70 + static_cast<int>(k / cols) * (rowH + capH + pad);
      g.setColour(sbd::ui::tok::textDim);
      g.setFont(sbd::r2::labelFont(11.0f, true));
      g.drawText(files[static_cast<int>(k)].getFileName(), x, y, tw, capH - 4, juce::Justification::centredLeft);
      g.drawImageAt(thumbs[k], x, y + capH);
    }
    rc = writePng(sheet, out) ? 0 : 1;
    std::cout << "contact " << out.getFullPathName().toStdString() << " " << sheet.getWidth() << "x" << sheet.getHeight() << std::endl;
    return rc;
  }
  if (a.contains("--selftest")) {
    rc = runSelfTest(log);
    emit(log, {});
    return rc;
  }
  // fenêtre interactive (aucun son, aucune connexion au moteur)
  {
    LabWindow w(a.contains("--smoke") ? 1500 : 0);
    juce::MessageManager::getInstance()->runDispatchLoop();
  }
  return 0;
}
