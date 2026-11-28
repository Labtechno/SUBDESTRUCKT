// SUBDESTRUCKT — tests ARCH-UI-04-R1 : TRANSPORT AUTHORIZATION & rendu hors temps réel (anomalie R-28).
//
// Critères enregistrés AVANT toute correction (brief ARCH-UI-04-R1 §3-§4) ; écrits contre les API existantes de 0.4.0
// (télémétrie, journal TA, signal audio) pour pouvoir être exécutés sur la baseline : les tests de REPRODUCTION doivent
// y échouer, les tests de NON-RÉGRESSION doivent y passer. Assertions sur le signal réel, pas seulement sur l'état.
//
//   Reproduction (échec attendu sur 0.4.0) : TA-R01, R02, R03, R05, R06, R07, R08, R09, R10, R11, R13, R16
//   Non-régression (succès attendu sur 0.4.0) : TA-R04, R12, R15
//
// Contrat visé : une autorisation acquise exclusivement pour un rendu hors temps réel est TEMPORAIRE ; elle prend fin au
// premier bloc temps réel ; les observations de transport faites pendant le rendu ne modifient ni l'historique temps
// réel ni le verrou Panic ; la fin d'une autorisation de rendu arrête réellement tout comportement autonome (signal
// nul), sans temporisation ni thread graphique ; l'autorisation normale préexistante est conservée ; les captures
// retenues sont conservées et restituées à la prochaine autorisation légitime.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "G4Helper.h"
#include "RenderHelper.h"
#include "TestKit.h"
#include "core/StateCodec.h"
#include "dsp/Destruction.h"
#include "dsp/Filters.h"

using namespace sbd;
using namespace g4;

namespace {

long long gAllocR1 = 0;  // allocations RT cumulées sur toutes les séquences TA-R (TA-R15)

// ---------------------------------------------------------------- outils (copie locale des aides de test_g43_ta.cpp,
// dont le fichier historique reste inchangé)
float& slotRef(ParamValues& p, int k, DstOff o) {
  return p.v[static_cast<size_t>(idx(ParamId::DstWt1Type) + k * static_cast<int>(DstOff::Count) + static_cast<int>(o))];
}
float& fltRef(ParamValues& p, FltOff o) { return p.v[static_cast<size_t>(idx(ParamId::FltWtType) + static_cast<int>(o))]; }

// tous les états autonomes : D2 gelé (WT1), S2 gelé (FM1), N2 auto-oscillant (GL1), gel modal du filtre WT
ParamValues autoParams(bool armed, bool frozen = true) {
  ParamValues p = ParamValues::defaults();
  slotRef(p, 0, DstOff::Type) = static_cast<float>(DstType::D2);
  slotRef(p, 0, DstOff::Mix) = 1.0f;
  slotRef(p, 0, DstOff::D2Freeze) = frozen ? 1.0f : 0.0f;
  slotRef(p, 0, DstOff::Arm) = armed ? 1.0f : 0.0f;
  slotRef(p, 2, DstOff::Type) = static_cast<float>(DstType::S2);
  slotRef(p, 2, DstOff::Mix) = 1.0f;
  slotRef(p, 2, DstOff::S2Freeze) = frozen ? 1.0f : 0.0f;
  slotRef(p, 2, DstOff::Arm) = armed ? 1.0f : 0.0f;
  slotRef(p, 4, DstOff::Type) = static_cast<float>(DstType::N2);
  slotRef(p, 4, DstOff::Mix) = 0.5f;
  slotRef(p, 4, DstOff::N2SelfOsc) = 1.0f;
  slotRef(p, 4, DstOff::N2Fb) = 0.95f;
  slotRef(p, 4, DstOff::Arm) = armed ? 1.0f : 0.0f;
  fltRef(p, FltOff::Type) = 4.0f;
  fltRef(p, FltOff::Decay) = 1.0f;
  fltRef(p, FltOff::Freeze) = frozen ? 1.0f : 0.0f;
  fltRef(p, FltOff::Arm) = armed ? 1.0f : 0.0f;
  return p;
}

TransportInfo tinfo(TransportObs o, bool rec = false, bool off = false) {
  TransportInfo t;
  t.valid = o != TransportObs::Unknown;
  t.playing = o == TransportObs::Rolling;
  t.recording = rec;
  t.offline = off;
  t.bpm = 120.0;
  return t;
}

struct Rig {
  std::unique_ptr<Engine> e;
  ParamValues p;
  int block;
  std::vector<float> L, R;
  std::vector<TaEvent> log;
  std::vector<int> logBlock;  // bloc (index) de chaque événement du journal
  explicit Rig(const ParamValues& pv, double fs = 48000.0, int blk = 256) : p(pv), block(blk) {
    e = std::make_unique<Engine>(FactoryBanks::get("factory:saw"));
    e->setParameters(p);
    e->prepare(fs, blk, Quality::Normal);
  }
  void step(const TransportInfo& t, const std::vector<MidiEvent>& ev = {}) {
    const size_t o = L.size();
    const int b = blocks();
    L.resize(o + static_cast<size_t>(block));
    R.resize(o + static_cast<size_t>(block));
    e->setParameters(p);
    e->setTransport(t);
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    e->process(&L[o], &R[o], block, ev.empty() ? nullptr : ev.data(), static_cast<int>(ev.size()));
    tk::countingAllocations().store(false);
    gAllocR1 += tk::allocationCount().load();
    e->exchange().collectGarbage();
    e->modExchange().collectGarbage();
    e->captureExchange().collectGarbage();
    TaEvent x;
    while (e->taLog().pop(x)) {
      log.push_back(x);
      logBlock.push_back(b);
    }
  }
  void run(int n, const TransportInfo& t) {
    for (int i = 0; i < n; ++i) step(t);
  }
  int blocks() const { return static_cast<int>(L.size()) / block; }
  int count(TaEventType ty, int fromBlock = 0) const {
    int c = 0;
    for (size_t i = 0; i < log.size(); ++i) c += log[i].type == ty && logBlock[i] >= fromBlock;
    return c;
  }
  int grants(AuthSource s, int fromBlock = 0) const {
    int c = 0;
    for (size_t i = 0; i < log.size(); ++i) c += log[i].type == TaEventType::Grant && log[i].source == s && logBlock[i] >= fromBlock;
    return c;
  }
  int firstGrantBlock(int fromBlock) const {
    for (size_t i = 0; i < log.size(); ++i)
      if (log[i].type == TaEventType::Grant && logBlock[i] >= fromBlock) return logBlock[i];
    return -1;
  }
  AuthSource firstGrantSource(int fromBlock) const {
    for (size_t i = 0; i < log.size(); ++i)
      if (log[i].type == TaEventType::Grant && logBlock[i] >= fromBlock) return log[i].source;
    return AuthSource::None;
  }
  bool authorized() const { return e->telemetry().autonomyAuthorized.load(); }
  bool latched() const { return e->telemetry().autonomyLatched.load(); }
  double rmsBlocks(int a, int b) const {  // [a, b) en blocs
    return std::max(rms(L, static_cast<size_t>(a * block), static_cast<size_t>(b * block)),
                    rms(R, static_cast<size_t>(a * block), static_cast<size_t>(b * block)));
  }
  double peakBlocks(int a, int b) const {
    return std::max(peak(L, static_cast<size_t>(a * block), static_cast<size_t>(b * block)),
                    peak(R, static_cast<size_t>(a * block), static_cast<size_t>(b * block)));
  }
};

void publish(Engine& e, const std::vector<CaptureAsset>& c, bool apply = true) {
  auto s = std::make_unique<CaptureSet>();
  s->items = c;
  s->apply = apply;
  e.captureExchange().publish(std::move(s));
}

// captures réelles : note jouée, gel armé activé à 0,5 s, capture à 1,5 s (D2 WT1, S2 FM1)
std::vector<CaptureAsset> makeCaptures(int note = 52) {
  ParamValues p = autoParams(true, false);
  Rig r(p);
  r.step(tinfo(TransportObs::Unknown), {MidiEvent::noteOn(0, note, 110)});
  r.run(93, tinfo(TransportObs::Unknown));
  r.p = autoParams(true, true);
  r.run(188, tinfo(TransportObs::Unknown));
  r.e->requestCapture(0);
  r.e->requestCapture(2);
  r.step(tinfo(TransportObs::Unknown));
  std::vector<CaptureAsset> out;
  for (int k : {0, 2}) {
    CaptureAsset c;
    if (r.e->fetchCapture(k, c)) out.push_back(c);
  }
  return out;
}

// restauration suspensive simulée (thread de contrôle) : configuration, Drone, captures, suspension
void restoreSuspended(Rig& r, const ParamValues& p, bool drone, const std::vector<CaptureAsset>& caps) {
  r.p = p;
  r.e->setDrone(drone);
  publish(*r.e, caps, true);
  r.e->setAutonomyAuthorized(false);
}

bool d2Equals(Rig& r, const CaptureAsset& ref) {
  r.e->requestCapture(ref.slot);
  r.step(tinfo(TransportObs::Stopped));
  CaptureAsset c;
  return r.e->fetchCapture(ref.slot, c) && c.frames == ref.frames && c.data == ref.data;
}

const TransportInfo Stop = tinfo(TransportObs::Stopped), Play = tinfo(TransportObs::Rolling), Unk = tinfo(TransportObs::Unknown);
const TransportInfo OffPlay = tinfo(TransportObs::Rolling, false, true), OffStop = tinfo(TransportObs::Stopped, false, true),
                    OffUnk = tinfo(TransportObs::Unknown, false, true);
const MidiEvent kOn = MidiEvent::noteOn(0, 45, 100), kOff = MidiEvent::noteOff(0, 45);

// export hors temps réel : note jouée au premier bloc, relâchée à mi-parcours si `release` (sinon annulation en cours de note)
void exportRun(Rig& r, int blocks, bool release = true, const TransportInfo& t = OffPlay) {
  r.step(t, {kOn});
  for (int i = 1; i < blocks; ++i) r.step(t, (release && i == blocks / 2) ? std::vector<MidiEvent>{kOff} : std::vector<MidiEvent>{});
}

std::string fmt(const char* f, double a) {
  char b[64];
  std::snprintf(b, sizeof b, f, a);
  return b;
}

constexpr double kSilence = 1e-9;  // énergie RMS d'un signal nul (comme TA-02/TA-21)
constexpr int kGuardBlocks = 2400;  // 12,8 s à 48 kHz / 256 : au-delà du garde-fou de 10 s des états non armés

}  // namespace

void runUi04R1Tests() {
  std::printf("[ARCH-UI-04-R1 : TRANSPORT AUTHORIZATION, rendu hors temps reel (R-28)]\n");
  const auto caps = makeCaptures();
  const bool capOk = caps.size() == 2 && caps[0].kind == 0 && caps[1].kind == 1;
  TK_CHECK(capOk);
  if (!capOk) return;
  const ParamValues armed = autoParams(true), off = autoParams(false);

  // TA-R01 : export avant PLAY (scénario R-28) — projet restauré, ARRÊT observé, export complet, retour temps réel
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    const double pre = a.rmsBlocks(0, 40);
    const bool susp0 = !a.authorized();
    const int b0 = a.blocks();
    exportRun(a, 400);
    const int b1 = a.blocks();
    const AuthSource src = a.firstGrantSource(b0);
    const double renderE = a.rmsBlocks(b0, b1);
    a.run(600, Stop);
    const double post = a.rmsBlocks(b1, a.blocks());
    const bool ok = pre < kSilence && susp0 && src == AuthSource::Render && renderE > 1e-3 && post < kSilence && !a.authorized();
    TK_CHECK(ok);
    tk::result("TA-R01", ok, "export avant PLAY (ARRET observe puis rendu) : silence initial %.2e, suspendu %d ; autorisation du rendu par %s (rendu attendu) ; contenu rendu %.2e ; apres le rendu (temps reel, ARRET, 3,2 s) : energie %.2e (< 1e-9), autorise %d (0 attendu)",
               pre, susp0, authSourceName(src), renderE, post, a.authorized());
  }

  // TA-R02 : annulation d'export avant PLAY (note encore tenue au moment de l'annulation)
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    const int b0 = a.blocks();
    exportRun(a, 60, false);
    const int b1 = a.blocks();
    a.run(600, Stop);
    const double post = a.rmsBlocks(b1, a.blocks()), first = a.rmsBlocks(b1, b1 + 1);
    const bool ok = a.rmsBlocks(b0, b1) > 1e-4 && post < kSilence && !a.authorized();
    TK_CHECK(ok);
    tk::result("TA-R02", ok, "annulation d'export (note tenue, aucune fin de note) puis ARRET temps reel : energie premier bloc %.2e, 3,2 s %.2e (< 1e-9 : ni restitution autonome ni note bloquee) ; autorise %d (0 attendu)",
               first, post, a.authorized());
  }

  // TA-R03 : export terminé avant PLAY (fin de lecture du rendu, hôte encore hors temps réel à l'arrêt)
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    exportRun(a, 300);
    a.run(60, OffStop);
    const int b1 = a.blocks();
    a.run(600, Stop);
    const double post = a.rmsBlocks(b1, a.blocks());
    const bool ok = post < kSilence && !a.authorized();
    TK_CHECK(ok);
    tk::result("TA-R03", ok, "export termine (rendu, puis ARRET hors temps reel) puis retour temps reel ARRET : energie %.2e (< 1e-9), autorise %d (0 attendu)", post, a.authorized());
  }

  // TA-R04 : export après autorisation normale (NON-RÉGRESSION) : l'autorisation légitime est conservée
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    a.run(200, Play);  // ARRÊT -> LECTURE : autorisation normale (transport)
    a.run(100, Stop);
    const int b0 = a.blocks();
    exportRun(a, 300);
    const int b1 = a.blocks();
    a.run(300, Stop);
    const double post = a.rmsBlocks(b1 + 20, a.blocks());
    const bool ok = a.authorized() && a.grants(AuthSource::Transport) == 1 && a.count(TaEventType::Grant, b0) == 0 && a.count(TaEventType::Revoke) == 0 &&
                    a.count(TaEventType::CaptureApplied) == 2 && post > 1e-3;
    TK_CHECK(ok);
    tk::result("TA-R04", ok, "autorisation normale (ARRET->LECTURE) avant l'export : conservee pendant et apres %d (source %s), aucune nouvelle autorisation pendant le rendu %d, aucune revocation %d ; gel arme rejoue apres le rendu %.2e (> 1e-3)",
               a.authorized(), authSourceName(static_cast<AuthSource>(a.e->telemetry().autonomySource.load())), a.count(TaEventType::Grant, b0) == 0,
               a.count(TaEventType::Revoke) == 0, post);
  }

  // TA-R05 : exports successifs : chaque rendu obtient sa propre autorisation temporaire ; silence entre et après
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    const int e1 = a.blocks();
    exportRun(a, 300);
    const int s1 = a.blocks();
    a.run(200, Stop);
    const int e2 = a.blocks();
    exportRun(a, 300);
    const int s2 = a.blocks();
    a.run(200, Stop);
    const double r1 = a.rmsBlocks(e1, s1), r2 = a.rmsBlocks(e2, s2), q1 = a.rmsBlocks(s1, e2), q2 = a.rmsBlocks(s2, a.blocks());
    const bool identical = std::equal(a.L.begin() + e1 * 256, a.L.begin() + s1 * 256, a.L.begin() + e2 * 256);
    const bool ok = a.grants(AuthSource::Render) == 2 && a.count(TaEventType::Grant) == 2 && a.count(TaEventType::CaptureApplied) == 4 && r1 > 1e-3 && r2 > 1e-3 &&
                    q1 < kSilence && q2 < kSilence && !a.authorized();
    TK_CHECK(ok);
    tk::result("TA-R05", ok, "deux exports successifs : autorisations de rendu %d (2 attendues), captures appliquees %d (4 attendues), contenus %.2e / %.2e ; silence entre et apres %.2e / %.2e ; autorise a la fin %d (0 attendu) ; [information] rendus identiques %d",
               a.grants(AuthSource::Render), a.count(TaEventType::CaptureApplied), r1, r2, q1, q2, a.authorized(), identical);
  }

  // TA-R06 : Panic pendant l'export ; un ARRÊT observé pendant le rendu ne lève pas le verrou
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    exportRun(a, 100, false);
    const int bp = a.blocks();
    a.step(OffPlay, {MidiEvent::allSoundOff(0)});
    a.run(100, OffPlay);
    a.run(30, OffStop);  // ARRÊT observé pendant le rendu
    const bool latchedAfterOffStop = a.latched();
    a.run(100, OffPlay);
    const int bx = a.blocks();
    const double afterPanic = a.rmsBlocks(bp + 5, bx);
    const bool noGrantInExport = a.count(TaEventType::Grant, bp) == 0;
    a.run(300, Stop);  // retour temps réel : ARRÊT réel (lève le verrou), toujours non autorisé, silence
    const double post = a.rmsBlocks(bx, a.blocks());
    const bool rtStopLifts = !a.latched() && !a.authorized();
    const int bplay = a.blocks();
    a.run(100, Play);  // première véritable LECTURE
    const bool regrant = a.authorized() && a.firstGrantSource(bplay) == AuthSource::Transport && a.firstGrantBlock(bplay) == bplay;
    const bool ok = latchedAfterOffStop && noGrantInExport && afterPanic < kSilence && post < kSilence && rtStopLifts && regrant;
    TK_CHECK(ok);
    tk::result("TA-R06", ok, "Panic pendant le rendu : verrou maintenu apres un ARRET observe pendant le rendu %d ; aucune autorisation jusqu'a la fin du rendu %d ; silence apres la rampe %.2e et apres le rendu %.2e ; ARRET temps reel : verrou leve, non autorise %d ; puis LECTURE : autorisation (transport, au premier bloc) %d",
               latchedAfterOffStop, noGrantInExport, afterPanic, post, rtStopLifts, regrant);
  }

  // TA-R07 : transport INCONNU pendant l'export (jamais assimilé à un ARRÊT)
  {
    Rig a(off);  // (a) export entièrement INCONNU : aucune autorisation
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    exportRun(a, 300, true, OffUnk);
    const int a1 = a.blocks();
    const bool ta = a.count(TaEventType::Grant) == 0 && a.count(TaEventType::CaptureApplied) == 0;
    a.run(300, Stop);
    const double pa = a.rmsBlocks(a1, a.blocks());
    Rig b(off);  // (b) INCONNU puis LECTURE pendant le rendu : autorisation de RENDU (pas de transport), retirée ensuite
    restoreSuspended(b, armed, true, caps);
    b.run(40, Stop);
    const int b0 = b.blocks();
    b.run(50, OffUnk);
    exportRun(b, 200);
    const int b1 = b.blocks();
    const AuthSource sb = b.firstGrantSource(b0);
    b.run(300, Stop);
    const double pb = b.rmsBlocks(b1, b.blocks());
    Rig c(off);  // (c) verrou Panic : un INCONNU pendant le rendu ne le lève pas
    restoreSuspended(c, armed, true, caps);
    c.run(40, Stop);
    c.step(Stop, {MidiEvent::allSoundOff(0)});
    c.run(50, OffUnk);
    const bool tc = c.latched();
    const bool ok = ta && pa < kSilence && !a.authorized() && sb == AuthSource::Render && pb < kSilence && !b.authorized() && tc;
    TK_CHECK(ok);
    tk::result("TA-R07", ok, "transport INCONNU pendant le rendu : (a) aucune autorisation ni capture %d, apres : %.2e, autorise %d ; (b) INCONNU puis LECTURE : autorisation par %s (rendu attendu), apres : %.2e, autorise %d ; (c) verrou Panic conserve %d",
               ta, pa, a.authorized(), authSourceName(sb), pb, b.authorized(), tc);
  }

  // TA-R08 : retour en temps réel avec transport ARRÊT : silence durable (au-delà du garde-fou de 10 s)
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    exportRun(a, 300);
    const int b1 = a.blocks();
    a.run(kGuardBlocks, Stop);
    const double post = a.rmsBlocks(b1, a.blocks()), pk = a.peakBlocks(b1, a.blocks());
    const bool st = a.e->telemetry().transportState.load() == static_cast<int>(TransportObs::Stopped);
    const bool susp = a.e->telemetry().autonomySuspended.load();
    const bool ok = post < kSilence && pk < 1e-6 && !a.authorized() && st && susp;
    TK_CHECK(ok);
    tk::result("TA-R08", ok, "retour temps reel a l'ARRET apres un rendu, 12,8 s : energie %.2e, crete %.2e ; autorise %d (0 attendu) ; transport ARRET %d ; etats autonomes suspendus publies %d",
               post, pk, a.authorized(), st, susp);
  }

  // TA-R09 : retour en temps réel directement en LECTURE : pas d'arête de lecture artificielle
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    exportRun(a, 300);
    const int b1 = a.blocks();
    a.run(300, Play);  // première observation temps réel : LECTURE
    const double post = a.rmsBlocks(b1, a.blocks());
    const bool notAuth = !a.authorized() && a.count(TaEventType::Grant, b1) == 0;
    a.run(10, Stop);
    const int bp = a.blocks();
    a.run(200, Play);
    const bool regrant = a.authorized() && a.firstGrantSource(bp) == AuthSource::Transport && a.firstGrantBlock(bp) == bp;
    const bool ok = post < kSilence && notAuth && regrant;
    TK_CHECK(ok);
    tk::result("TA-R09", ok, "retour temps reel en LECTURE apres un rendu : non autorise, aucune autorisation %d, energie %.2e (< 1e-9) ; ARRET puis LECTURE : autorisation (transport, premier bloc) %d",
               notAuth, post, regrant);
  }

  // TA-R10 : premier véritable ARRÊT -> LECTURE après l'export : autorisation normale au premier bloc, captures restituées
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    exportRun(a, 300);
    a.run(100, Stop);
    const int bp = a.blocks();
    const int applied0 = a.count(TaEventType::CaptureApplied);
    a.run(300, Play);
    const bool grantAt = a.firstGrantBlock(bp) == bp && a.firstGrantSource(bp) == AuthSource::Transport;
    const int applied = a.count(TaEventType::CaptureApplied) - applied0;
    const double content = a.rmsBlocks(bp + 100, a.blocks());
    const bool ok = grantAt && applied == 2 && content > 1e-3 && a.authorized();
    TK_CHECK(ok);
    tk::result("TA-R10", ok, "premier ARRET->LECTURE apres le rendu : autorisation au premier bloc de lecture (transport) %d ; captures reappliquees %d (2 attendues) ; gel rejoue %.2e",
               grantAt, applied, content);
  }

  // TA-R11 : restauration (ou duplication) pendant l'export
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    const int b0 = a.blocks();
    exportRun(a, 100, false);
    const std::uint32_t g0 = a.e->telemetry().authGeneration.load();
    restoreSuspended(a, armed, true, caps);  // nouvelle génération pendant le rendu
    a.run(200, OffPlay);
    const int b1 = a.blocks();
    const bool newGen = a.e->telemetry().authGeneration.load() == g0 + 1 && a.count(TaEventType::Suspend, b0) >= 1;
    int nonRender = 0;
    for (size_t i = 0; i < a.log.size(); ++i)
      if (a.log[i].type == TaEventType::Grant && a.logBlock[i] >= b0 && a.log[i].source != AuthSource::Render) ++nonRender;
    a.run(600, Stop);
    const double post = a.rmsBlocks(b1, a.blocks());
    const bool ok = newGen && a.grants(AuthSource::Render, b0) >= 1 && nonRender == 0 && post < kSilence && !a.authorized();
    TK_CHECK(ok);
    tk::result("TA-R11", ok, "restauration pendant le rendu : nouvelle generation et suspension %d ; autorisations pendant le rendu : rendu %d, autres sources %d (0 attendu) ; apres le rendu : energie %.2e, autorise %d (0 attendu)",
               newGen, a.grants(AuthSource::Render, b0), nonRender, post, a.authorized());
  }

  // TA-R12 : isolation de plusieurs instances (NON-RÉGRESSION)
  {
    Rig a(off), b(off), ref(off);
    restoreSuspended(a, armed, true, caps);
    restoreSuspended(b, armed, true, caps);
    restoreSuspended(ref, armed, true, caps);
    for (int i = 0; i < 40; ++i) a.step(Stop), b.step(Stop), ref.step(Stop);
    for (int i = 0; i < 300; ++i) {
      a.step(OffPlay, i == 0 ? std::vector<MidiEvent>{kOn} : std::vector<MidiEvent>{});
      b.step(Stop);
      ref.step(Stop);
    }
    for (int i = 0; i < 300; ++i) a.step(Stop), b.step(Stop), ref.step(Stop);
    const bool same = th::sameFloats(b.L, ref.L) && th::sameFloats(b.R, ref.R);
    const bool ok = same && !b.authorized() && b.count(TaEventType::Grant) == 0;
    TK_CHECK(ok);
    tk::result("TA-R12", ok, "deux instances, export sur A seulement : B (temps reel, suspendue) bit a bit = B seule %d, jamais autorisee %d", same, !b.authorized() && b.count(TaEventType::Grant) == 0);
  }

  // TA-R13 : conservation et restitution des captures après l'export
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(40, Stop);
    exportRun(a, 300);
    a.run(100, Stop);
    const int bp = a.blocks();
    const int before = a.count(TaEventType::CaptureApplied);
    a.run(300, Play);
    const int applied = a.count(TaEventType::CaptureApplied) - before;
    const bool d2 = d2Equals(a, caps[0]);
    const bool ok = applied == 2 && d2 && a.rmsBlocks(bp + 100, bp + 300) > 1e-3;
    TK_CHECK(ok);
    tk::result("TA-R13", ok, "captures retenues apres le rendu : restituees a la premiere autorisation legitime %d (2 attendues) ; contenu D2 = capture bit a bit %d ; gel rejoue %.2e",
               applied, d2, a.rmsBlocks(bp + 100, bp + 300));
  }

  // TA-R16 : états DSP résiduels après la fin d'une autorisation de rendu (exigence complémentaire du propriétaire)
  {
    struct Cfg {
      const char* name;
      ParamValues p;
      bool drone;
      bool withCaps;
    };
    std::vector<Cfg> cfgs;
    {
      ParamValues p = ParamValues::defaults();  // D2 Legacy gelé armé
      slotRef(p, 0, DstOff::Type) = static_cast<float>(DstType::D2);
      slotRef(p, 0, DstOff::Mix) = 1.0f;
      slotRef(p, 0, DstOff::D2Freeze) = 1.0f;
      slotRef(p, 0, DstOff::Arm) = 1.0f;
      cfgs.push_back({"D2 Legacy", p, false, true});
      ParamValues q = ParamValues::defaults();  // D2 Regenerative : boucle régénérée, Repeat 1, feedback
      slotRef(q, 0, DstOff::Type) = static_cast<float>(DstType::D2);
      slotRef(q, 0, DstOff::Mix) = 1.0f;
      slotRef(q, 0, DstOff::D2Repeat) = 1.0f;
      slotRef(q, 0, DstOff::D2Fb) = 0.9f;
      slotRef(q, 0, DstOff::Arm) = 1.0f;
      q[ParamId::DstWt1D2FbMode] = 1.0f;
      cfgs.push_back({"D2 Regenerative", q, false, false});
      ParamValues s = ParamValues::defaults();  // S2 gelé armé (Decay 1 : spectre tenu)
      slotRef(s, 2, DstOff::Type) = static_cast<float>(DstType::S2);
      slotRef(s, 2, DstOff::Mix) = 1.0f;
      slotRef(s, 2, DstOff::S2Freeze) = 1.0f;
      slotRef(s, 2, DstOff::S2Decay) = 1.0f;
      slotRef(s, 2, DstOff::Arm) = 1.0f;
      cfgs.push_back({"S2", s, false, true});
      ParamValues n = ParamValues::defaults();  // N2 auto-oscillant armé (boucle de feedback)
      slotRef(n, 4, DstOff::Type) = static_cast<float>(DstType::N2);
      slotRef(n, 4, DstOff::Mix) = 0.5f;
      slotRef(n, 4, DstOff::N2SelfOsc) = 1.0f;
      slotRef(n, 4, DstOff::N2Fb) = 0.95f;
      slotRef(n, 4, DstOff::Arm) = 1.0f;
      cfgs.push_back({"N2 feedback", n, false, false});
      ParamValues f = ParamValues::defaults();  // F4 modal gelé armé
      fltRef(f, FltOff::Type) = 4.0f;
      fltRef(f, FltOff::Decay) = 1.0f;
      fltRef(f, FltOff::Freeze) = 1.0f;
      fltRef(f, FltOff::Arm) = 1.0f;
      cfgs.push_back({"F4 modal", f, false, false});
      cfgs.push_back({"Drone (Gate)", ParamValues::defaults(), true, false});
      cfgs.push_back({"tous", armed, true, true});
    }
    bool all = true;
    std::string detail;
    for (const auto& c : cfgs) {
      ParamValues base = c.p;
      for (ParamId a : {ParamId::FltWtArm, ParamId::FltFmArm, ParamId::FltGlArm, ParamId::DstWt1Arm, ParamId::DstWt2Arm, ParamId::DstFm1Arm,
                        ParamId::DstFm2Arm, ParamId::DstGl1Arm, ParamId::DstGl2Arm})
        base[a] = 0.0f;  // instance neuve : configuration désarmée avant la restauration
      Rig r(base);
      restoreSuspended(r, c.p, c.drone, c.withCaps ? caps : std::vector<CaptureAsset>{});
      r.run(40, Stop);
      const int b0 = r.blocks();
      exportRun(r, 300);
      const int b1 = r.blocks();
      r.run(kGuardBlocks, Stop);
      const double rend = r.rmsBlocks(b0, b1), first = r.rmsBlocks(b1, b1 + 188), late = r.rmsBlocks(b1 + 188, r.blocks());
      const bool ok = rend > 1e-5 && first < kSilence && late < kSilence && !r.authorized();
      all = all && ok;
      detail += std::string(" [") + c.name + " : rendu " + fmt("%.1e", rend) + ", 0-1 s " + fmt("%.1e", first) + ", 1-12,8 s " + fmt("%.1e", late) + (ok ? " ok]" : " ECHEC]");
    }
    TK_CHECK(all);
    tk::result("TA-R16", all, "etats residuels apres la fin d'une autorisation de rendu (temps reel, ARRET, 12,8 s ; silence < 1e-9 exige des le premier bloc) :%s", detail.c_str());
  }

  // TA-R15 : sécurité temps réel (NON-RÉGRESSION) : aucune allocation dans Engine::process sur toutes les séquences TA-R
  // (verrous et accès disque : RTSan en CI sur l'ensemble des tests du cœur, G-18rt)
  {
    const bool ok = gAllocR1 == 0;
    TK_CHECK(ok);
    tk::result("TA-R15", ok, "0 allocation dans Engine::process sur toutes les sequences TA-R (%lld) ; verrous et E/S : RTSan (CI, G-18rt) ; processBlock : TA-R15w", gAllocR1);
  }
}
