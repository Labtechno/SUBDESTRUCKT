// SUBDESTRUCKT — ARCH-UI-04-R2-B2 : banc de performances graphiques GC-01 (protocole B2-GRAPHICS-PERFORMANCE-PROTOCOL.md,
// seuils définitifs GS-01..GS-07 et exigences CE-B2-04 : B2-00-ADDENDUM.md §3-4).
//
// Le MÊME code est compilé dans l'arbre de référence 0.4.1 et dans l'arbre intégré ; un script alterne les exécutions
// (référence, intégré, référence, intégré...). Le banc n'écrit aucun seuil : il publie des données BRUTES et des
// statistiques (n, médiane, p95, p99, MAD, min, max, valeurs aberrantes > médiane + 5 MAD, comptées mais conservées).
//
// Mesures :
//  GP-01 rendu complet de l'éditeur (chaque espace / onglet, zooms 75/100/150, échelle 2) ;
//  GP-02 coût d'un rafraîchissement 30 Hz en scénario dynamique (refreshNow + rendu des zones réellement invalidées) ;
//  GP-03 zones invalidées par rafraîchissement (dynamique) et au repos stabilisé (statique) ;
//  GP-04 temps de traitement audio par bloc au rythme réel, éditeur absent puis actif (thread audio dédié) ;
//  GP-05 mémoire résidente (avant éditeur, éditeur ouvert, après 100 cycles ouverture/fermeture et stabilisation) ;
//  GP-06 redimensionnement (setSize + rendu complet) ;
//  GP-07 interaction directe (glisser d'un NEXUS injecté par le pair de test : traitement + rendu local) ;
//  GP-08 HUD actifs (moniteur oscilloscope puis spectre : rafraîchissement + rendu de leur zone) ;
//  GP-09 éditeur fermé (traitement audio après fermeture de l'éditeur).
// Rendu : type d'image natif de la plateforme (CoreGraphics sous macOS, Direct2D sous Windows), rendu forcé jusqu'au bout
// par la lecture d'un pixel. Un snapshot mesure un COÛT DE RENDU, pas la fluidité perçue dans Reason (CE-B2-04).
// Usage : subdestruckt_gc01_bench <dossier_sortie> <etiquette> [sha] [rapide]
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <map>
#include <string>
#include <thread>
#include <vector>
#include <typeinfo>

#include "plugin/PluginProcessor.h"
#include "support/HeadlessPeer.h"
#include "ui/MainEditor.h"

#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
 #include <windows.h>
 #include <psapi.h>
 #pragma comment(lib, "psapi.lib")
#elif JUCE_MAC
 #include <mach/mach.h>
#endif

using namespace sbd;
using namespace sbd::ui;
using Clock = std::chrono::steady_clock;

namespace {

double ms(Clock::time_point a, Clock::time_point b) { return std::chrono::duration<double, std::milli>(b - a).count(); }

double rssMiB() {
#if JUCE_WINDOWS
  PROCESS_MEMORY_COUNTERS pmc{};
  if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof pmc)) return static_cast<double>(pmc.WorkingSetSize) / 1048576.0;
#elif JUCE_MAC
  mach_task_basic_info info{};
  mach_msg_type_number_t n = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &n) == KERN_SUCCESS)
    return static_cast<double>(info.resident_size) / 1048576.0;
#endif
  return -1.0;
}

struct Recorder {
  std::ofstream raw;
  std::vector<std::string> summary;
  std::string label;
  void open(const juce::File& dir, const std::string& lab) {
    label = lab;
    raw.open(dir.getChildFile("gc01-raw-" + juce::String(lab) + ".csv").getFullPathName().toStdString(), std::ios::binary);
    raw << "label;metric;scenario;rep;value\n";
  }
  void add(const std::string& metric, const std::string& scen, int rep, double v) { raw << label << ";" << metric << ";" << scen << ";" << rep << ";" << v << "\n"; }
  void stats(const std::string& metric, const std::string& scen, std::vector<double> v, const std::string& unit) {
    if (v.empty()) return;
    std::sort(v.begin(), v.end());
    auto q = [&](double p) { return v[std::min(v.size() - 1, static_cast<size_t>(std::floor(p * static_cast<double>(v.size() - 1) + 0.5)))]; };
    const double med = q(0.5);
    std::vector<double> dev;
    for (double x : v) dev.push_back(std::abs(x - med));
    std::sort(dev.begin(), dev.end());
    const double mad = dev[dev.size() / 2];
    int out = 0;
    for (double x : v) out += (mad > 0.0 && x > med + 5.0 * mad) ? 1 : 0;
    char b[512];
    std::snprintf(b, sizeof b, "%s;%s;%s;%s;%zu;%.4f;%.4f;%.4f;%.4f;%.4f;%.4f;%d", label.c_str(), metric.c_str(), scen.c_str(), unit.c_str(), v.size(), med, q(0.95),
                  q(0.99), mad, v.front(), v.back(), out);
    summary.push_back(b);
    std::printf("  %-6s %-34s n=%-4zu med %9.3f  p95 %9.3f  p99 %9.3f  MAD %7.3f  [%s]\n", metric.c_str(), scen.c_str(), v.size(), med, q(0.95), q(0.99), mad, unit.c_str());
    std::fflush(stdout);
  }
} rec;

// Type d'image : natif sous macOS (CoreGraphics, sur CPU, celui du plugin) ; moteur logiciel de JUCE ailleurs (sous Windows,
// l'image native Direct2D impose une relecture GPU -> CPU qui domine la mesure et ne représente pas le rendu à l'écran).
juce::ImageType& benchImageType() {
#if JUCE_MAC
  static juce::NativeImageType t;
#else
  static juce::SoftwareImageType t;
#endif
  return t;
}
const char* benchImageTypeName() {
#if JUCE_MAC
  return "natif CoreGraphics";
#else
  return "logiciel JUCE (Direct2D ecarte)";
#endif
}

// rendu forcé jusqu'au bout (lecture d'un pixel) d'une zone du composant hôte, à l'échelle donnée
double renderArea(juce::Component& root, juce::Rectangle<int> area, float scale) {
  if (area.isEmpty()) return 0.0;
  double d = 0.0;
  JUCE_AUTORELEASEPOOL {  // macOS : sans boucle d'événements, les objets CoreGraphics ne sont libérés qu'à la vidange du pool
    const auto t0 = Clock::now();
    juce::Image img(juce::Image::ARGB, juce::roundToInt(area.getWidth() * scale), juce::roundToInt(area.getHeight() * scale), true, benchImageType());
    {
      juce::Graphics g(img);
      g.addTransform(juce::AffineTransform::translation(static_cast<float>(-area.getX()), static_cast<float>(-area.getY())).scaled(scale));
      g.reduceClipRegion(area);
      root.paintEntireComponent(g, true);
    }
    volatile auto px = img.getPixelAt(0, 0).getARGB();
    juce::ignoreUnused(px);
    d = ms(t0, Clock::now());
  }
  return d;
}

double renderDirty(testsupport::TestPeer& peer, juce::Component& root, float scale) {
  double t = 0.0;
  for (const auto& r : peer.dirty) t += renderArea(root, r, scale);
  peer.dirty.clear();
  return t;
}

struct Rig {
  std::unique_ptr<SubdestrucktProcessor> p = std::make_unique<SubdestrucktProcessor>();
  testsupport::TestHost host;
  std::unique_ptr<juce::AudioProcessorEditor> ed;
  MainEditor* m = nullptr;
  testsupport::TestPeer* peer = nullptr;
  juce::AudioBuffer<float> buf{2, 256};
  juce::MidiBuffer midi;
  Rig() { p->prepareToPlay(48000.0, 256); }
  void openEditor(int w = 1440, int h = 900) {
    ed.reset(p->createEditorIfNeeded());
    m = dynamic_cast<MainEditor*>(ed.get());
    host.setBounds(0, 0, w, h);
    m->setBounds(0, 0, w, h);
    host.addAndMakeVisible(*m);
    if (!host.isOnDesktop()) {
      host.setVisible(true);
      host.addToDesktop(0);
    }
    peer = dynamic_cast<testsupport::TestPeer*>(host.getPeer());
  }
  void closeEditor() {
    if (ed) {
      p->editorBeingDeleted(ed.get());
      ed.reset();
      m = nullptr;
    }
  }
  ~Rig() {
    closeEditor();
    host.removeFromDesktop();
  }
  void block(bool noteOn = false) {
    buf.clear();
    midi.clear();
    if (noteOn) midi.addEvent(juce::MidiMessage::noteOn(1, 48, 0.8f), 0);
    p->processBlock(buf, midi);
  }
  void audioFor(double seconds) {
    const int n = static_cast<int>(seconds * 48000.0 / 256.0 + 0.5);
    for (int k = 0; k < n; ++k) block();
  }
  // scénario dynamique réel : note tenue + LFO 1 vers les commandes continues visibles de PERFORMANCE (16 au plus)
  int addModulation() {
    int added = 0;
    for (auto* b : m->visibleControls()) {
      if (added >= 16) break;
      const auto id = b->boundId();
      if (paramSpec(id).type != ParamType::Float) continue;
      ModRoute r;
      r.source = ModSource::Lfo1;
      r.dest = id;
      r.depth = 0.3f;
      if (p->modAdd(r).ok) ++added;
    }
    block(true);
    return added;
  }
};

struct View {
  int space, tab;
  std::string name;
};

std::vector<View> views(MainEditor& m) {
  std::vector<View> v;
  const char* sp[] = {"PERF", "SD", "ENG"};
  for (int s = 0; s < 3; ++s)
    for (int t = 0; t < m.numTabs(s); ++t) v.push_back({s, t, std::string(sp[s]) + "-" + std::to_string(t)});
  return v;
}

}  // namespace

int main(int argc, char** argv) {
  juce::ScopedJuceInitialiser_GUI juceInit;
  if (argc < 3) {
    std::fprintf(stderr, "usage : %s <dossier_sortie> <etiquette> [sha] [rapide]\n", argv[0]);
    return 2;
  }
  const juce::File dir(juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(argv[1])));
  dir.createDirectory();
  const std::string label = argv[2];
  const std::string sha = argc > 3 ? argv[3] : "?";
  const bool quick = argc > 4;
  const int reps = quick ? 8 : 30, warm = quick ? 2 : 5;
  const float scale = 2.0f;  // équivalent Retina (cible macOS)
  rec.open(dir, label);
  {
    std::ofstream c(dir.getChildFile("gc01-conditions-" + juce::String(label) + ".txt").getFullPathName().toStdString(), std::ios::binary);
    c << "etiquette : " << label << "\ncode : " << sha << "\nos : " << juce::SystemStats::getOperatingSystemName() << "\ncpu : "
      << juce::SystemStats::getCpuModel() << " (" << juce::SystemStats::getNumPhysicalCpus() << " coeurs physiques, " << juce::SystemStats::getNumCpus()
      << " logiques)\nmemoire : " << juce::SystemStats::getMemorySizeInMegabytes() << " Mio\njuce : " << juce::SystemStats::getJUCEVersion()
      << "\naudio : 48000 Hz, blocs de 256\nrendu : image " << benchImageTypeName() << ", echelle " << scale << ", rendu force (lecture d'un pixel)\nrepetitions : "
      << reps << " (+ " << warm << " de chauffe)\nmode : " << (quick ? "rapide (non probant)" : "complet") << "\n";
  }
  std::printf("GC-01 %s (%s) : %s\n", label.c_str(), sha.c_str(), juce::SystemStats::getOperatingSystemName().toRawUTF8());

  // ---------------------------------------------------------------- GP-05 (avant éditeur) puis GP-01
  Rig r;
  r.audioFor(0.5);
  const double memBefore = rssMiB();
  r.openEditor();
  const auto vs = views(*r.m);
  for (int z : {75, 100, 150}) {
    r.m->setZoomPercent(z);
    for (const auto& v : vs) {
      r.m->showSubTab(v.space, v.tab);
      r.m->refreshLayout();
      std::vector<double> t;
      for (int k = 0; k < warm + reps; ++k) {
        const double d = renderArea(r.host, r.host.getLocalBounds(), scale);
        if (k >= warm) {
          t.push_back(d);
          rec.add("GP-01", v.name + "@" + std::to_string(z), k - warm, d);
        }
      }
      rec.stats("GP-01", v.name + "@" + std::to_string(z), t, "ms");
    }
  }
  r.m->setZoomPercent(100);
  r.m->showSpace(0);
  r.m->refreshLayout();
  renderArea(r.host, r.host.getLocalBounds(), scale);
  const double memOpen = rssMiB();

  // ---------------------------------------------------------------- GP-03 statique : repos stabilisé (aucune donnée ne change)
  {
    r.m->refreshNow();
    r.peer->dirty.clear();
    r.peer->repaintCalls = 0;
    std::vector<double> rects, calls;
    for (int k = 0; k < 60; ++k) {
      r.m->refreshNow();
      rects.push_back(static_cast<double>(r.peer->dirty.getNumRectangles()));
      calls.push_back(static_cast<double>(r.peer->repaintCalls));
      rec.add("GP-03", "repos-zones", k, rects.back());
      r.peer->dirty.clear();
      r.peer->repaintCalls = 0;
    }
    // identification des zones invalidées au repos (composant le plus profond au centre de chaque zone)
    r.m->refreshNow();
    r.peer->dirty.clear();
    r.m->refreshNow();
    std::ofstream idf(dir.getChildFile("gc01-repos-zones-" + juce::String(label) + ".txt").getFullPathName().toStdString(), std::ios::binary);
    for (const auto& d : r.peer->dirty) {
      auto* c = r.host.getComponentAt(d.getCentre());
      idf << d.toString() << " : " << (c ? typeid(*c).name() : "?") << " '" << (c ? (c->getTitle() + "|" + c->getName()).toStdString() : std::string()) << "'\n";
    }
    r.peer->dirty.clear();
    rec.stats("GP-03", "repos-zones-par-rafraichissement", rects, "zones");
    rec.stats("GP-03", "repos-appels-repaint", calls, "appels");
  }

  // ---------------------------------------------------------------- GP-02 / GP-03 dynamique / GP-08
  const int routes = r.addModulation();
  std::printf("  modulation : %d routes LFO1, note tenue\n", routes);
  for (int k = 0; k < warm * 3; ++k) {  // chauffe
    r.audioFor(1.0 / 30.0);
    r.m->refreshNow();
    renderDirty(*r.peer, r.host, scale);
  }
  {
    std::vector<double> tick, paint, total, area;
    for (int k = 0; k < reps * 6; ++k) {
      r.audioFor(1.0 / 30.0);
      r.peer->dirty.clear();
      const auto t0 = Clock::now();
      r.m->refreshNow();
      const auto t1 = Clock::now();
      double a = 0.0;
      for (const auto& d : r.peer->dirty) a += static_cast<double>(d.getWidth()) * d.getHeight();
      const double p = renderDirty(*r.peer, r.host, scale);
      tick.push_back(ms(t0, t1));
      paint.push_back(p);
      total.push_back(ms(t0, t1) + p);
      area.push_back(a / 1000.0);
      rec.add("GP-02", "PERF-dynamique", k, total.back());
    }
    rec.stats("GP-02", "PERF-dynamique-tick", tick, "ms");
    rec.stats("GP-02", "PERF-dynamique-rendu", paint, "ms");
    rec.stats("GP-02", "PERF-dynamique-total", total, "ms");
    rec.stats("GP-03", "PERF-dynamique-surface", area, "kpx2");
  }
  // GP-08 : moniteur (oscilloscope, puis spectre) : bouton du MonitorPanel actionné par le pair de test
  {
    std::vector<juce::Component*> all;
    std::function<void(juce::Component&)> walk = [&](juce::Component& c) {
      for (auto* ch : c.getChildren()) {
        all.push_back(ch);
        walk(*ch);
      }
    };
    walk(*r.m);
    MonitorPanel* mon = nullptr;
    for (auto* c : all)
      if (auto* mp = dynamic_cast<MonitorPanel*>(c)) mon = mp;
    if (mon != nullptr) {
      std::vector<juce::TextButton*> btn;
      for (auto* ch : mon->getChildren())
        if (auto* b = dynamic_cast<juce::TextButton*>(ch)) btn.push_back(b);
      const char* modes[] = {"oscilloscope", "spectre"};
      juce::int64 tm = juce::Time::currentTimeMillis();
      for (int mode = 0; mode < 2 && mode < static_cast<int>(btn.size()); ++mode) {
        auto* b = btn[static_cast<size_t>(mode)];
        const auto pos = r.peer->globalToLocal(b->localPointToGlobal(b->getLocalBounds().getCentre().toFloat()));
        r.peer->handleMouseEvent(juce::MouseInputSource::InputSourceType::mouse, pos, juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier), 1.0f, 0.0f, tm += 900);
        r.peer->handleMouseEvent(juce::MouseInputSource::InputSourceType::mouse, pos, juce::ModifierKeys(), 1.0f, 0.0f, tm += 40);
        const auto monArea = r.host.getLocalArea(mon, mon->getLocalBounds());
        std::vector<double> t;
        for (int k = 0; k < warm + reps * 3; ++k) {
          r.audioFor(1.0 / 30.0);
          const auto t0 = Clock::now();
          r.m->refreshNow();
          r.peer->dirty.clear();
          const double p = renderArea(r.host, monArea, scale);
          if (k >= warm) {
            t.push_back(ms(t0, Clock::now()) - p + p);
            rec.add("GP-08", modes[mode], k - warm, t.back());
          }
        }
        rec.stats("GP-08", std::string("moniteur-") + modes[mode], t, "ms");
      }
    }
  }

  // ---------------------------------------------------------------- GP-07 : glisser d'un NEXUS (wt.x)
  {
    ContinuousControl* c = nullptr;
    for (auto* b : r.m->visibleControls())
      if (b->boundId() == ParamId::WtX) c = dynamic_cast<ContinuousControl*>(&b->asComponent());
    if (c != nullptr) {
      juce::int64 tm = juce::Time::currentTimeMillis() + 5000;
      auto at = [&](juce::Point<float> local, bool down) {
        const auto pp = r.peer->globalToLocal(c->localPointToGlobal(local));
        r.peer->handleMouseEvent(juce::MouseInputSource::InputSourceType::mouse, pp, down ? juce::ModifierKeys(juce::ModifierKeys::leftButtonModifier) : juce::ModifierKeys(),
                                 1.0f, 0.0f, tm += 16);
      };
      std::vector<double> t;
      for (int pass = 0; pass < 3; ++pass) {
        const juce::Point<float> o{c->getWidth() * 0.5f, c->getHeight() * 0.5f};
        at(o, false);
        at(o, true);
        r.peer->dirty.clear();
        for (int k = 0; k < 60; ++k) {
          const auto t0 = Clock::now();
          at(o + juce::Point<float>(0.0f, static_cast<float>(-(k % 30) * 2)), true);
          const double p = renderDirty(*r.peer, r.host, scale);
          const double d = ms(t0, Clock::now());
          juce::ignoreUnused(p);
          t.push_back(d);
          rec.add("GP-07", "NEXUS-glisser", pass * 60 + k, d);
        }
        at(o, false);
        tm += 900;
      }
      rec.stats("GP-07", "NEXUS-glisser-traitement+rendu", t, "ms");
    }
  }

  // ---------------------------------------------------------------- GP-06 : redimensionnement
  {
    const int sizes[][2] = {{1080, 700}, {1440, 900}, {1920, 1200}, {2560, 1440}};
    for (auto& s : sizes) {
      std::vector<double> t;
      for (int k = 0; k < warm + reps / 2; ++k) {
        const auto t0 = Clock::now();
        r.host.setBounds(0, 0, s[0], s[1]);
        r.m->setBounds(0, 0, s[0], s[1]);
        r.m->refreshLayout();
        renderArea(r.host, r.host.getLocalBounds(), scale);
        const double d = ms(t0, Clock::now());
        r.host.setBounds(0, 0, 1440, 900);
        r.m->setBounds(0, 0, 1440, 900);
        r.m->refreshLayout();
        if (k >= warm) {
          t.push_back(d);
          rec.add("GP-06", std::to_string(s[0]) + "x" + std::to_string(s[1]), k - warm, d);
        }
      }
      rec.stats("GP-06", std::to_string(s[0]) + "x" + std::to_string(s[1]), t, "ms");
    }
  }

  // ---------------------------------------------------------------- GP-04 / GP-09 : audio au rythme réel
  auto audioRun = [&](const std::string& scen, bool editorActive, double seconds) {
    std::vector<double> dur, late;
    std::atomic<bool> stop{false};
    int misses = 0;  // blocs dont le TRAITEMENT dépasse la période (256 / 48000 s) : seul critère attribuable au calcul
    const auto period = std::chrono::duration<double>(256.0 / 48000.0);
    std::thread audio([&] {
      juce::AudioBuffer<float> b(2, 256);
      juce::MidiBuffer mm;
      auto next = Clock::now();
      const int blocks = static_cast<int>(seconds * 48000.0 / 256.0);
      dur.reserve(static_cast<size_t>(blocks));
      for (int k = 0; k < blocks; ++k) {
        next += std::chrono::duration_cast<Clock::duration>(period);
        const auto wake = next - std::chrono::duration_cast<Clock::duration>(period);
        std::this_thread::sleep_until(wake);
        const auto t0 = Clock::now();
        late.push_back(ms(wake, t0));  // retard de réveil (ordonnanceur, thread sans priorité temps réel) : information
        b.clear();
        mm.clear();
        r.p->processBlock(b, mm);
        const auto t1 = Clock::now();
        dur.push_back(ms(t0, t1));
        if (ms(t0, t1) > 256.0 / 48.0) ++misses;
      }
      stop = true;
    });
    while (!stop) {
      if (editorActive && r.m != nullptr) {
        r.m->refreshNow();
        renderDirty(*r.peer, r.host, scale);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
    audio.join();
    for (size_t k = 0; k < dur.size(); ++k) rec.add("GP-04", scen, static_cast<int>(k), dur[k]);
    rec.stats("GP-04", scen + "-bloc", dur, "ms");
    rec.stats("GP-04", scen + "-depassements-traitement", {static_cast<double>(misses)}, "blocs");
    rec.stats("GP-04", scen + "-retard-reveil-info", late, "ms");
  };
  const double secs = quick ? 4.0 : 20.0;
  audioRun("editeur-actif", true, secs);
  r.closeEditor();
  audioRun("editeur-ferme-apres-ouverture", false, secs);  // GP-09

  // ---------------------------------------------------------------- GP-05 : 100 cycles ouverture / fermeture, stabilisation
  // série de 200 cycles, mémoire résidente relevée tous les 10 cycles : la croissance de la SECONDE moitié distingue une
  // croissance persistante (fuite) d'une montée en charge des caches ou de l'allocateur qui se stabilise (GS-05)
  const int cycles = quick ? 40 : 200;
  double memHalf = 0.0;
  for (int k = 1; k <= cycles; ++k) {
    JUCE_AUTORELEASEPOOL {
      r.openEditor();
      r.m->refreshNow();
      renderArea(r.host, r.host.getLocalBounds(), 1.0f);
      r.closeEditor();
    }
    if (k % 10 == 0) {
      const double v = rssMiB();
      rec.add("GP-05", "rss-par-10-cycles", k, v);
      if (k == cycles / 2) memHalf = v;
    }
  }
  double memCycles = 0.0;
  JUCE_AUTORELEASEPOOL {
    r.openEditor();
    for (int k = 0; k < 3; ++k) renderArea(r.host, r.host.getLocalBounds(), scale);
    memCycles = rssMiB();
    r.closeEditor();
  }
  rec.stats("GP-05", "rss-avant-editeur", {memBefore}, "MiB");
  rec.stats("GP-05", "rss-editeur-ouvert", {memOpen}, "MiB");
  rec.stats("GP-05", "surcout-editeur", {memOpen - memBefore}, "MiB");
  rec.stats("GP-05", "rss-apres-cycles-stabilise", {memCycles}, "MiB");
  rec.stats("GP-05", "croissance-premiere-moitie", {memHalf - memOpen}, "MiB");
  rec.stats("GP-05", "croissance-seconde-moitie", {memCycles - memHalf}, "MiB");

  std::ofstream su(dir.getChildFile("gc01-summary-" + juce::String(label) + ".csv").getFullPathName().toStdString(), std::ios::binary);
  su << "label;metric;scenario;unit;n;median;p95;p99;mad;min;max;outliers_gt_med_plus_5mad\n";
  for (auto& l : rec.summary) su << l << "\n";
  std::printf("GC-01 termine : %zu series\n", rec.summary.size());
  return 0;
}
