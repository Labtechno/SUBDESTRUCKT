// SUBDESTRUCKT — tests ARCH-G4.3 : TRANSPORT AUTHORIZATION (moteur, transport simulé). docs/V1-ARCH-G4.3-SPEC.md §3.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "Analysis.h"
#include "G4Helper.h"
#include "RenderHelper.h"
#include "TestKit.h"
#include "core/StateCodec.h"
#include "dsp/Destruction.h"
#include "dsp/Filters.h"

using namespace sbd;
using namespace g4;

namespace {

long long gAlloc = 0;  // allocations RT cumulées sur toutes les séquences TA (TA-20)

// configuration « tous les états autonomes » : D2 gelé (WT1), S2 gelé (FM1), N2 auto-oscillant (GL1), gel modal du
// filtre WT ; armés ou non
ParamValues autoParams(bool armed, bool frozen = true) {
  ParamValues p = ParamValues::defaults();
  auto slot = [&](int k, DstOff o) -> float& { return p.v[static_cast<size_t>(idx(ParamId::DstWt1Type) + k * static_cast<int>(DstOff::Count) + static_cast<int>(o))]; };
  slot(0, DstOff::Type) = static_cast<float>(DstType::D2);
  slot(0, DstOff::Mix) = 1.0f;
  slot(0, DstOff::D2Freeze) = frozen ? 1.0f : 0.0f;
  slot(0, DstOff::Arm) = armed ? 1.0f : 0.0f;
  slot(2, DstOff::Type) = static_cast<float>(DstType::S2);
  slot(2, DstOff::Mix) = 1.0f;
  slot(2, DstOff::S2Freeze) = frozen ? 1.0f : 0.0f;
  slot(2, DstOff::Arm) = armed ? 1.0f : 0.0f;
  slot(4, DstOff::Type) = static_cast<float>(DstType::N2);
  slot(4, DstOff::Mix) = 0.5f;
  slot(4, DstOff::N2SelfOsc) = 1.0f;
  slot(4, DstOff::N2Fb) = 0.95f;
  slot(4, DstOff::Arm) = armed ? 1.0f : 0.0f;
  const int f = idx(ParamId::FltWtType);
  p.v[static_cast<size_t>(f + static_cast<int>(FltOff::Type))] = 4.0f;
  p.v[static_cast<size_t>(f + static_cast<int>(FltOff::Decay))] = 1.0f;
  p.v[static_cast<size_t>(f + static_cast<int>(FltOff::Freeze))] = frozen ? 1.0f : 0.0f;
  p.v[static_cast<size_t>(f + static_cast<int>(FltOff::Arm))] = armed ? 1.0f : 0.0f;
  return p;
}

TransportInfo tinfo(TransportObs o, bool rec = false, bool off = false, double ppq = 0.0, double bpm = 120.0) {
  TransportInfo t;
  t.valid = o != TransportObs::Unknown;
  t.playing = o == TransportObs::Rolling;
  t.recording = rec;
  t.offline = off;
  t.ppq = ppq;
  t.bpm = bpm;
  return t;
}

struct Rig {
  std::unique_ptr<Engine> e;
  ParamValues p;
  int block;
  std::vector<float> L, R;
  std::vector<TaEvent> log;
  long long alloc = 0;
  explicit Rig(const ParamValues& pv, double fs = 48000.0, int blk = 256, Quality q = Quality::Normal, const char* bank = "factory:saw")
      : p(pv), block(blk) {
    e = std::make_unique<Engine>(FactoryBanks::get(bank));
    e->setParameters(p);
    e->prepare(fs, blk, q);
  }
  void step(const TransportInfo& t, const std::vector<MidiEvent>& ev = {}) {
    const size_t o = L.size();
    L.resize(o + static_cast<size_t>(block));
    R.resize(o + static_cast<size_t>(block));
    e->setParameters(p);
    e->setTransport(t);
    tk::allocationCount().store(0);
    tk::countingAllocations().store(true);
    e->process(&L[o], &R[o], block, ev.empty() ? nullptr : ev.data(), static_cast<int>(ev.size()));
    tk::countingAllocations().store(false);
    alloc += tk::allocationCount().load();
    gAlloc += tk::allocationCount().load();
    e->exchange().collectGarbage();
    e->modExchange().collectGarbage();
    e->captureExchange().collectGarbage();
    TaEvent x;
    while (e->taLog().pop(x)) log.push_back(x);
  }
  void run(int n, const TransportInfo& t) {
    for (int i = 0; i < n; ++i) step(t);
  }
  int blocks() const { return static_cast<int>(L.size()) / block; }
  int count(TaEventType ty) const {
    int c = 0;
    for (const auto& x : log) c += x.type == ty;
    return c;
  }
  bool authorized() const { return e->telemetry().autonomyAuthorized.load(); }
  double rmsLast(int nBlocks) const {
    const size_t b = L.size(), a = b - static_cast<size_t>(nBlocks * block);
    return std::max(rms(L, a, b), rms(R, a, b));
  }
};

void publish(Engine& e, const std::vector<CaptureAsset>& c, bool apply = true) {
  auto s = std::make_unique<CaptureSet>();
  s->items = c;
  s->apply = apply;
  e.captureExchange().publish(std::move(s));
}

MidiEvent at(MidiEvent e, int off) {
  e.offset = off;
  return e;
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

bool d2Equals(Engine& e, const CaptureAsset& ref, Rig& r) {
  e.requestCapture(ref.slot);
  r.step(tinfo(TransportObs::Unknown));
  CaptureAsset c;
  return e.fetchCapture(ref.slot, c) && c.frames == ref.frames && c.data == ref.data;
}

void testTransportAuthorization(const std::vector<CaptureAsset>& caps) {
  const auto Stop = tinfo(TransportObs::Stopped), Play = tinfo(TransportObs::Rolling), Unk = tinfo(TransportObs::Unknown);
  const ParamValues armed = autoParams(true), off = autoParams(false);
  const MidiEvent on = MidiEvent::noteOn(0, 45, 100), noteOff = MidiEvent::noteOff(0, 45);

  // TA-01 : instance neuve à l'arrêt (configuration par défaut) ; travail à l'arrêt (D2 gelé armé dans la session)
  {
    Rig a(ParamValues::defaults());
    a.run(563, Stop);  // 3 s
    const double e0 = a.rmsLast(563);
    Rig b(autoParams(true, false));
    b.step(Stop, {on});
    b.run(93, Stop);
    b.p = autoParams(true, true);  // gel armé activé pendant la note (session, instance autorisée)
    b.run(94, Stop);
    b.step(Stop, {noteOff});
    b.run(375, Stop);
    const double held = b.rmsLast(94);  // 2 s après Note Off : contenu gelé rejoué
    const bool ok = e0 < 1e-9 && held > 1e-3 && b.authorized();
    TK_CHECK(ok);
    tk::result("TA-01", ok, "instance neuve a l'arret : energie 3 s %.2e (< 1e-9) ; travail a l'arret : D2 gele arme rejoue sans transport %.2e (> 1e-3), autorisee %d",
               e0, held, b.authorized());
  }

  // TA-02, TA-03, TA-05, TA-25 : restauration à l'arrêt, STOP->PLAY, STOP après PLAY ; référence explicite au même bloc
  const int K = 200;
  {
    Rig a(off), ref(off), dis(off);
    restoreSuspended(a, armed, true, caps);
    restoreSuspended(ref, armed, true, caps);
    for (int i = 0; i < K; ++i) {
      std::vector<MidiEvent> ev;
      if (i == 10) ev = {on};
      if (i == 60) ev = {noteOff};
      a.step(Stop, ev);
      ref.step(Stop, ev);
      dis.step(Stop, ev);
    }
    const bool suspSame = th::sameFloats(a.L, dis.L) && th::sameFloats(a.R, dis.R);
    const bool suspFlag = a.e->telemetry().autonomySuspended.load() && !a.authorized();
    const int appliedBefore = a.count(TaEventType::CaptureApplied);
    ref.e->setAutonomyAuthorized(true);  // référence : autorisation explicite à la même frontière de bloc
    for (int i = K; i < K + 400; ++i) {
      a.step(Play);
      ref.step(Stop);
    }
    const bool grantAtK = a.authorized() && a.count(TaEventType::Grant) == 1;
    for (int i = 0; i < 600; ++i) {
      a.step(Stop);
      ref.step(Stop);
    }
    const bool same = th::sameFloats(a.L, ref.L) && th::sameFloats(a.R, ref.R);
    const int applied = a.count(TaEventType::CaptureApplied);
    AuthSource src = AuthSource::None;
    for (const auto& x : a.log)
      if (x.type == TaEventType::Grant) src = x.source;
    const bool t2 = suspSame && suspFlag && appliedBefore == 0;
    TK_CHECK(t2);
    tk::result("TA-02", t2, "restauration a l'arret (9 armements dont D2/S2/N2/modal, Drone, 2 captures) : 200 blocs ARRET bit a bit = etat desarme sans Drone %d ; suspendu publie %d ; captures appliquees %d (0 attendu)",
               suspSame, suspFlag, appliedBefore);
    const double autoE = rms(a.L, static_cast<size_t>((K + 100) * 256), static_cast<size_t>((K + 400) * 256));
    const bool t3 = grantAtK && src == AuthSource::Transport && same && applied == 2 && autoE > 1e-3;
    TK_CHECK(t3);
    tk::result("TA-03", t3, "STOP->PLAY : autorisation au premier bloc LECTURE (source %s, autorisations %d) ; rendu bit a bit = reference autorisee explicitement au meme bloc %d ; captures appliquees %d (2 attendu, une fois) ; energie autonome %.2e",
               authSourceName(src), a.count(TaEventType::Grant), same, applied, autoE);
    const bool t5 = a.authorized() && same;
    TK_CHECK(t5);
    tk::result("TA-05", t5, "STOP apres PLAY : autorisation conservee %d ; 600 blocs ARRET bit a bit = reference %d", a.authorized(), same);
    // TA-25 : journal ordonné
    std::vector<TaEventType> seq;
    for (const auto& x : a.log)
      if (x.type == TaEventType::Suspend || x.type == TaEventType::TransportStop || x.type == TaEventType::TransportPlay ||
          x.type == TaEventType::Grant || x.type == TaEventType::CaptureApplied)
        seq.push_back(x.type);
    const std::vector<TaEventType> want = {TaEventType::Suspend, TaEventType::TransportStop, TaEventType::TransportPlay, TaEventType::Grant,
                                           TaEventType::CaptureApplied, TaEventType::CaptureApplied, TaEventType::TransportStop};
    const bool t25 = seq == want && a.e->taLog().dropped() == 0;
    TK_CHECK(t25);
    std::string s;
    for (auto t : seq) s += std::string(" > ") + taEventName(t);
    tk::result("TA-25", t25, "journal TA : sequence%s ; pertes %u", s.c_str(), a.e->taLog().dropped());
  }

  // TA-04 : enregistrement
  {
    Rig a(off);
    restoreSuspended(a, armed, false, caps);
    a.run(10, Stop);
    a.step(tinfo(TransportObs::Rolling, true));
    const bool ta = a.authorized();
    Rig b(off);
    b.run(5, Play);  // lecture déjà en cours
    restoreSuspended(b, armed, false, caps);
    b.run(20, Play);
    const bool bBefore = !b.authorized();
    b.step(tinfo(TransportObs::Rolling, true));  // punch-in
    AuthSource src = AuthSource::None;
    for (const auto& x : b.log)
      if (x.type == TaEventType::Grant) src = x.source;
    const bool tb = bBefore && b.authorized() && src == AuthSource::Record;
    Rig c(off);
    restoreSuspended(c, armed, false, caps);
    c.run(10, Stop);
    c.run(20, tinfo(TransportObs::Stopped, true));
    const bool tc = !c.authorized();
    const bool ok = ta && tb && tc;
    TK_CHECK(ok);
    tk::result("TA-04", ok, "enregistrement : (a) ARRET->LECTURE+ENR autorise %d ; (b) restauration pendant LECTURE puis punch-in : non autorise avant %d, autorise par %s %d ; (c) ENR sans LECTURE : non autorise %d",
               ta, bBefore, authSourceName(src), tb, tc);
  }

  // TA-06 : boucle, seek, tempo pendant LECTURE
  {
    Rig a(off), r(off);
    restoreSuspended(a, armed, true, caps);
    restoreSuspended(r, armed, true, caps);
    a.run(20, Stop);
    r.run(20, Stop);
    double ppq = 0.0;
    for (int i = 0; i < 600; ++i) {
      double bpm = 120.0;
      if (i % 60 == 30) ppq = 0.0;     // retour de boucle
      if (i == 333) ppq = 64.0;        // seek
      if (i % 50 >= 25) bpm = 97.3;    // changements de tempo
      a.step(tinfo(TransportObs::Rolling, false, false, ppq, bpm), i == 5 ? std::vector<MidiEvent>{on} : std::vector<MidiEvent>{});
      r.step(Play, i == 5 ? std::vector<MidiEvent>{on} : std::vector<MidiEvent>{});
      ppq += 256.0 * bpm / 60.0 / 48000.0;
    }
    const bool same = th::sameFloats(a.L, r.L) && th::sameFloats(a.R, r.R);
    const bool ok = same && a.count(TaEventType::Grant) == 1 && a.count(TaEventType::CaptureApplied) == 2 && a.authorized();
    TK_CHECK(ok);
    tk::result("TA-06", ok, "boucle (10 retours), seek, 24 changements de tempo pendant LECTURE : autorisations %d, captures appliquees %d ; rendu bit a bit = reference sans ces evenements %d",
               a.count(TaEventType::Grant), a.count(TaEventType::CaptureApplied), same);
  }

  // TA-07 : Autoriser explicite à l'arrêt ; commande périmée
  {
    Rig a(off);
    restoreSuspended(a, armed, false, caps);
    a.run(50, Stop);
    const bool before = !a.authorized();
    a.e->setAutonomyAuthorized(true);
    a.step(Stop);
    AuthSource src = AuthSource::None;
    for (const auto& x : a.log)
      if (x.type == TaEventType::Grant) src = x.source;
    Rig b(off);
    b.run(3, Unk);
    b.e->setAutonomyAuthorized(true);   // commande émise...
    restoreSuspended(b, armed, false, caps);  // ... puis restauration plus récente avant la frontière de bloc
    b.run(5, Unk);
    const bool stale = !b.authorized() && b.count(TaEventType::StaleCommand) == 1;
    const bool ok = before && a.authorized() && src == AuthSource::Explicit && stale;
    TK_CHECK(ok);
    tk::result("TA-07", ok, "Autoriser explicite a l'arret : autorise au bloc suivant %d (source %s) ; commande anterieure a une restauration plus recente : ignoree %d (commande perimee journalisee)",
               a.authorized(), authSourceName(src), stale);
  }

  // TA-08 : preset (armements désarmés, captures retenues) pendant ARRÊT et pendant LECTURE
  {
    ParamValues preset = armed;
    PluginState ps;
    ps.params = preset;
    disarmAutonomous(ps);
    Rig a(off), d(off);
    restoreSuspended(a, ps.params, false, caps);
    for (int i = 0; i < 700; ++i) {
      const auto& t = i < 100 ? Stop : Play;
      std::vector<MidiEvent> ev;
      if (i == 150) ev = {on};
      if (i == 200) ev = {noteOff};
      a.step(t, ev);
      d.step(t, ev);
    }
    const bool stopPlay = th::sameFloats(a.L, d.L) && th::sameFloats(a.R, d.R) && a.count(TaEventType::CaptureApplied) == 0 && a.authorized();
    // preset chargé pendant une lecture autorisée avec gels actifs : plus aucun comportement autonome
    // preset chargé pendant une lecture autorisée avec gels actifs : rendu = même historique désarmé au même bloc
    // (sans captures ni suspension) ; un gel NON armé reste une traîne bornée par le garde-fou de 10 s (P0-C03/C04)
    Rig b(off), bd(off);
    restoreSuspended(b, armed, true, caps);
    restoreSuspended(bd, armed, true, caps);
    b.run(10, Stop);
    bd.run(10, Stop);
    b.run(200, Play);
    bd.run(200, Play);
    const double beforeE = b.rmsLast(50);
    const size_t mark = b.L.size();
    restoreSuspended(b, ps.params, false, caps);
    bd.p = ps.params;
    bd.e->setDrone(false);
    b.run(2625, Play);  // 14 s
    bd.run(2625, Play);
    const bool samePlay = std::equal(b.L.begin() + static_cast<long>(mark), b.L.end(), bd.L.begin() + static_cast<long>(mark)) &&
                          std::equal(b.R.begin() + static_cast<long>(mark), b.R.end(), bd.R.begin() + static_cast<long>(mark));
    const double at2s = rms(b.L, mark + 375 * 256, mark + 400 * 256), afterGuard = b.rmsLast(100);
    const bool ok = stopPlay && beforeE > 1e-3 && samePlay && b.count(TaEventType::CaptureApplied) == 2 && afterGuard < 1e-9;
    TK_CHECK(ok);
    tk::result("TA-08", ok, "preset (armements desarmes, captures) : ARRET puis LECTURE bit a bit = desarme sans capture %d (captures appliquees %d, autorisation sans armement %d) ; preset charge pendant LECTURE autorisee (%.2e) : rendu bit a bit = meme historique desarme au meme bloc %d, aucune capture reappliquee ; gel non arme = traine bornee (%.2e a 2 s, %.2e apres le garde-fou de 10 s)",
               stopPlay, a.count(TaEventType::CaptureApplied), a.authorized(), beforeE, samePlay, at2s, afterGuard);
  }

  // TA-09, TA-11, TA-12 : Panic sans armement ; Panic pendant la lecture ; ARRÊT puis LECTURE
  {
    Rig z(ParamValues::defaults());
    z.run(5, Play);
    z.step(Play, {MidiEvent::allSoundOff(0)});
    const bool noArm = !z.authorized() && z.e->telemetry().autonomyLatched.load() && z.e->telemetry().authRevocations.load() == 1;
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(10, Stop);
    a.run(300, Play);
    const double beforeE = a.rmsLast(50);
    a.step(Play, {at(MidiEvent::allSoundOff(0), 100)});
    a.run(60, Play);
    const double afterE = a.rmsLast(40);
    const bool cfgSame = a.p.v == armed.v;
    const bool t9 = noArm && beforeE > 1e-3 && afterE < 1e-9 && cfgSame;
    TK_CHECK(t9);
    tk::result("TA-09", t9, "Panic sans armement : revoque %d (verrou, revocations 1) ; avec armements : %.2e -> %.2e (< 1e-9 apres la rampe) ; configuration inchangee %d",
               noArm, beforeE, afterE, cfgSame);
    a.run(400, Play);
    a.step(tinfo(TransportObs::Rolling, true));  // punch-in après Panic
    a.run(540, tinfo(TransportObs::Rolling, true));
    const bool t11 = !a.authorized() && a.count(TaEventType::Grant) == 1;
    TK_CHECK(t11);
    tk::result("TA-11", t11, "LECTURE maintenue 5 s apres Panic, punch-in compris : toujours revoquee %d (autorisations %d)", !a.authorized(),
               a.count(TaEventType::Grant));
    const int appliedBefore = a.count(TaEventType::CaptureApplied);
    a.run(10, Stop);
    a.run(200, Play);
    const bool regrant = a.authorized() && a.count(TaEventType::Grant) == 2 && a.count(TaEventType::CaptureApplied) == appliedBefore + 2;
    const bool d2same = d2Equals(*a.e, caps[0], a);
    const double replay = a.rmsLast(100);
    const bool t12 = regrant && d2same && replay > 1e-3;
    TK_CHECK(t12);
    tk::result("TA-12", t12, "ARRET puis LECTURE apres Panic : nouvelle autorisation %d (captures reappliquees %d) ; contenu D2 = capture bit a bit %d ; gel rejoue %.2e",
               regrant, a.count(TaEventType::CaptureApplied) - appliedBefore, d2same, replay);
  }

  // TA-10 : priorité de Panic sur une autorisation du même bloc (décalages 0 et 300, blocs de 512)
  {
    int okCount = 0;
    std::string detail;
    for (int kind = 0; kind < 3; ++kind)
      for (int offs : {0, 300}) {
        Rig x(off, 48000.0, 512), y(off, 48000.0, 512);
        restoreSuspended(x, armed, true, caps);
        restoreSuspended(y, armed, true, caps);
        x.run(20, Stop);
        y.run(20, Stop);
        const MidiEvent p = at(MidiEvent::allSoundOff(0), offs);
        if (kind == 0) {  // début de lecture
          x.step(Play, {p});
          y.step(Stop, {p});
        } else if (kind == 1) {  // Autoriser
          x.e->setAutonomyAuthorized(true);
          x.step(Stop, {p});
          y.step(Stop, {p});
        } else {  // premier bloc de rendu hors temps réel
          x.step(tinfo(TransportObs::Rolling, false, true), {p});
          y.step(tinfo(TransportObs::Stopped, false, true), {p});
        }
        const bool notAuth = !x.authorized();
        for (int i = 0; i < 20; ++i) {
          x.step(kind == 0 ? Play : (kind == 2 ? tinfo(TransportObs::Rolling, false, true) : Stop));
          y.step(kind == 2 ? tinfo(TransportObs::Stopped, false, true) : Stop);
        }
        const bool same = th::sameFloats(x.L, y.L) && th::sameFloats(x.R, y.R);
        const bool ok = notAuth && same && x.count(TaEventType::GrantBlockedByPanic) == 1 && !x.authorized();
        okCount += ok;
        char b[96];
        std::snprintf(b, sizeof b, " [%s offset %d : %s]", kind == 0 ? "lecture" : (kind == 1 ? "Autoriser" : "rendu"), offs, ok ? "ok" : "ECHEC");
        detail += b;
      }
    const bool ok = okCount == 6;
    TK_CHECK(ok);
    tk::result("TA-10", ok, "Panic dans le meme bloc qu'une autorisation : non autorise et rendu bit a bit = Panic sans candidat (bloc et 20 blocs suivants) %d/6 :%s",
               okCount, detail.c_str());
  }

  // TA-13, TA-14 : transport INCONNU ; premier bloc déjà en LECTURE
  {
    Rig a(off);
    restoreSuspended(a, armed, false, caps);
    a.run(3, Play);
    a.run(50, Unk);
    a.run(3, Play);
    const bool s1 = !a.authorized();
    Rig b(off);
    restoreSuspended(b, armed, false, caps);
    b.run(3, Stop);
    b.run(50, Unk);
    b.step(Play);
    const bool s2 = b.authorized();
    Rig c(off);
    c.run(50, Unk);
    const bool s3a = c.authorized();
    restoreSuspended(c, armed, false, caps);
    c.run(50, Unk);
    const bool s3b = !c.authorized() && c.e->telemetry().transportState.load() == 0;
    const bool t13 = s1 && s2 && s3a && s3b;
    TK_CHECK(t13);
    tk::result("TA-13", t13, "transport INCONNU : LECTURE, INCONNU x50, LECTURE -> aucune transition %d ; ARRET, INCONNU x50, LECTURE -> autorise %d ; INCONNU seul : ni revocation %d ni autorisation %d",
               s1, s2, s3a, s3b);
    Rig d(off);
    restoreSuspended(d, armed, true, caps);
    d.run(100, Play);
    const bool before = !d.authorized();
    d.run(2, Stop);
    d.step(Play);
    const bool t14 = before && d.authorized();
    TK_CHECK(t14);
    tk::result("TA-14", t14, "premier bloc deja en LECTURE (restauration ou duplication pendant la lecture) : non autorise pendant 100 blocs %d ; puis ARRET->LECTURE : autorise %d",
               before, d.authorized());
  }

  // TA-15 : rendu hors temps réel
  {
    const auto OffPlay = tinfo(TransportObs::Rolling, false, true);
    Rig a(off), r(off);
    restoreSuspended(a, armed, true, caps);
    restoreSuspended(r, armed, true, caps);
    r.e->setAutonomyAuthorized(true);
    for (int i = 0; i < 1875; ++i) {  // 10 s, note à 1 s, relâchée à 1,5 s (Drone autorisé : maintien)
      std::vector<MidiEvent> ev;
      if (i == 188) ev = {on};
      if (i == 281) ev = {noteOff};
      a.step(OffPlay, ev);
      r.step(Stop, ev);
    }
    AuthSource src = AuthSource::None;
    for (const auto& x : a.log)
      if (x.type == TaEventType::Grant) src = x.source;
    const bool same = th::sameFloats(a.L, r.L) && th::sameFloats(a.R, r.R);
    const double content = a.rmsLast(375);
    const bool ta = src == AuthSource::Render && same && content > 1e-3 && a.count(TaEventType::CaptureApplied) == 2;
    Rig b(off);
    restoreSuspended(b, armed, false, caps);
    b.run(50, tinfo(TransportObs::Unknown, false, true));
    const bool tb = !b.authorized();
    Rig c(off);
    restoreSuspended(c, armed, false, caps);
    c.run(50, tinfo(TransportObs::Stopped, false, true));
    const bool tc = !c.authorized();
    Rig d(off);
    restoreSuspended(d, armed, false, caps);
    d.run(5, OffPlay);
    d.step(OffPlay, {MidiEvent::allSoundOff(0)});
    d.run(50, OffPlay);
    const bool td = !d.authorized() && d.count(TaEventType::Grant) == 1;
    a.run(100, Stop);  // retour en temps réel (arrêt)
    const bool te = a.authorized();
    const bool ok = ta && tb && tc && td && te;
    TK_CHECK(ok);
    tk::result("TA-15", ok, "rendu hors temps reel : (a) premier bloc en LECTURE apres restauration, sans ARRET observe ni editeur ni thread message : autorise par %s, rendu bit a bit = reference autorisee au meme bloc %d, contenu autonome %.2e, captures %d ; (b) INCONNU : non %d ; (c) ARRET : non %d ; (d) verrou Panic : non %d ; (e) retour temps reel : conservee %d",
               authSourceName(src), same, content, a.count(TaEventType::CaptureApplied), tb, tc, td, te);
  }

  // TA-16 : re-préparation (fréquence, taille de bloc, Quality, réactivation)
  {
    Rig a(off);
    restoreSuspended(a, armed, true, caps);
    a.run(5, Stop);
    a.run(100, Play);
    int applied = a.count(TaEventType::CaptureApplied);
    bool okAuth = true, okReapply = true;
    struct Prep {
      double fs;
      int blk;
      Quality q;
    };
    for (const Prep& pp : {Prep{96000.0, 64, Quality::Normal}, Prep{48000.0, 512, Quality::High}, Prep{44100.0, 256, Quality::Eco}, Prep{48000.0, 256, Quality::Normal}}) {
      a.e->prepare(pp.fs, pp.blk, pp.q);
      a.block = pp.blk;
      a.run(static_cast<int>(0.5 * pp.fs / pp.blk), Play);
      okAuth = okAuth && a.authorized();
      okReapply = okReapply && a.count(TaEventType::CaptureApplied) == applied + 2 && a.rmsLast(20) > 1e-3;
      applied = a.count(TaEventType::CaptureApplied);
    }
    Rig s(off);
    restoreSuspended(s, armed, true, caps);
    s.run(20, Play);  // lecture déjà en cours : reste suspendue
    s.e->prepare(96000.0, 128, Quality::High);
    s.block = 128;
    s.run(200, Play);
    const bool susp = !s.authorized() && s.count(TaEventType::CaptureApplied) == 0;
    s.run(2, Stop);
    s.step(Play);  // dernier état connu et autorisation conservés à travers prepare : transition valide
    const bool ok = okAuth && okReapply && susp && s.authorized();
    TK_CHECK(ok);
    tk::result("TA-16", ok, "re-preparation (96 kHz/64, High/512, 44,1 kHz Eco, 48 kHz) : autorisation conservee %d ; captures reappliquees apres chaque preparation %d ; instance suspendue : reste suspendue sans application %d puis STOP->PLAY autorise %d",
               okAuth, okReapply, susp, s.authorized());
  }

  // TA-17 : concurrence déterministe (chargement L, Autoriser A, Panic P avant une même frontière de bloc)
  {
    int ok = 0;
    std::string table;
    const char* names[6] = {"L,A,P", "L,P,A", "A,L,P", "A,P,L", "P,L,A", "P,A,L"};
    const int orders[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
    for (int k = 0; k < 6; ++k) {
      Rig r(off);
      r.run(3, Stop);
      bool panic = false;
      for (int j = 0; j < 3; ++j) {
        const int op = orders[k][j];
        if (op == 0) restoreSuspended(r, armed, false, caps);
        if (op == 1) r.e->setAutonomyAuthorized(true);
        if (op == 2) panic = true;  // Panic : événement du bloc (bouton Panic ou CC120)
      }
      r.step(Stop, panic ? std::vector<MidiEvent>{MidiEvent::allSoundOff(0)} : std::vector<MidiEvent>{});
      const bool res = !r.authorized();  // Panic domine dans tous les ordres
      ok += res;
      table += std::string(" ") + names[k] + (res ? ":revoquee" : ":AUTORISEE");
    }
    // sans Panic : L puis A -> autorisée ; A puis L -> suspendue (commande périmée)
    Rig la(off), al(off);
    la.run(3, Stop);
    al.run(3, Stop);
    restoreSuspended(la, armed, false, caps);
    la.e->setAutonomyAuthorized(true);
    al.e->setAutonomyAuthorized(true);
    restoreSuspended(al, armed, false, caps);
    la.step(Stop);
    al.step(Stop);
    // deux chargements avant tout bloc : seules les captures du second
    const auto caps2 = makeCaptures(64);
    Rig two(off);
    restoreSuspended(two, armed, true, caps);
    restoreSuspended(two, armed, true, caps2);
    two.e->setAutonomyAuthorized(true);
    two.run(20, Stop);
    const bool second = d2Equals(*two.e, caps2[0], two);
    const bool all = ok == 6 && la.authorized() && !al.authorized() && second;
    TK_CHECK(all);
    tk::result("TA-17", all, "concurrence (meme frontiere de bloc) :%s ; sans Panic : L,A autorisee %d, A,L suspendue %d ; deux chargements avant tout bloc : captures du second seulement %d",
               table.c_str(), la.authorized(), !al.authorized(), second);
  }

  // TA-18 : aucune persistance de l'autorisation
  {
    PluginState s = defaultState();
    s.params = armed;
    s.captures = caps;
    const std::string text = encodeState(s, "test", StateScope::Daw);
    std::string low = text;
    for (auto& ch : low) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    const bool noField = low.find("auth") == std::string::npos && low.find("autoris") == std::string::npos && low.find("suspend") == std::string::npos;
    PluginState d;
    std::string err;
    const bool rt = decodeState(text, d, err) && encodeState(d, "test", StateScope::Daw) == text;
    const bool ok = noField && rt;
    TK_CHECK(ok);
    tk::result("TA-18", ok, "aucune persistance : aucun champ d'autorisation dans l'etat %d ; etat independant de l'autorisation (codec sans entree d'execution, aller-retour exact) %d ; instance/processeur : TA-18w",
               noField, rt);
  }

  // TA-19 : indépendance des instances
  {
    Rig a(off), b(off), c(off);
    restoreSuspended(a, armed, true, caps);
    restoreSuspended(b, armed, true, caps);
    restoreSuspended(c, armed, true, caps);
    for (int i = 0; i < 600; ++i) {
      std::vector<MidiEvent> ev;
      if (i == 300) ev = {MidiEvent::allSoundOff(0)};
      if (i == 100) a.e->setAutonomyAuthorized(true);
      a.step(i < 50 ? Stop : Play, ev);
      b.step(Stop);
    }
    c.run(600, Stop);
    const bool ok = th::sameFloats(b.L, c.L) && th::sameFloats(b.R, c.R) && !b.authorized();
    TK_CHECK(ok);
    tk::result("TA-19", ok, "independance : transport, Autoriser et Panic envoyes a A sans effet sur B (rendu bit a bit = B seul) %d", ok);
  }

  // TA-21, TA-22 : Drone ; All Sound Off / All Notes Off
  {
    const ParamValues plain = ParamValues::defaults();
    Rig a(plain);
    restoreSuspended(a, plain, true, {});  // Drone restauré (configuration), suspendu
    a.step(Stop, {on});
    a.run(93, Stop);
    a.step(Stop, {noteOff});
    a.run(281, Stop);  // 1,5 s après Note Off
    const double suspE = a.rmsLast(94);
    a.step(Play);
    a.step(Play, {on});
    a.run(93, Play);
    a.step(Play, {noteOff});
    a.run(375, Play);
    const double heldE = a.rmsLast(94);
    Rig d(plain);
    d.e->setDrone(true);  // Drone mis en marche à l'arrêt (instance autorisée : explicite)
    d.step(Stop, {on});
    d.run(93, Stop);
    d.step(Stop, {noteOff});
    d.run(375, Stop);
    const double manualE = d.rmsLast(94);
    // TA-22 : All Notes Off ne révoque pas et ne relâche pas le Drone autorisé ; All Sound Off = Panic
    d.step(Stop, {MidiEvent::allNotesOff(0)});
    d.run(200, Stop);
    const double afterAno = d.rmsLast(50);
    const bool anoOk = d.authorized() && d.e->telemetry().authRevocations.load() == 0 && afterAno > 1e-3;
    d.step(Stop, {MidiEvent::allSoundOff(0)});
    const bool latchedNow = d.e->telemetry().autonomyLatched.load();  // fin du bloc du Panic (levé ensuite par un ARRÊT réel)
    d.run(50, Stop);
    const double asoE = d.rmsLast(30);
    const bool asoOk = !d.authorized() && latchedNow && !d.e->telemetry().drone.load() && asoE < 1e-9;
    const bool t21 = suspE < 1e-9 && heldE > 1e-3 && manualE > 1e-3 && asoOk;
    TK_CHECK(t21);
    tk::result("TA-21", t21, "Drone : (a) restaure et suspendu : relache normal, energie 1,5 s apres Note Off %.2e (< 1e-9) ; (b) apres STOP->PLAY : maintien %.2e ; (c) drone_restore : TA-21w ; (d) mis en marche a l'arret : maintien %.2e ; (e) Panic : Drone desarme et revocation %d",
               suspE, heldE, manualE, asoOk);
    const bool t22 = anoOk && asoOk;
    TK_CHECK(t22);
    tk::result("TA-22", t22, "All Notes Off : aucune revocation, Drone maintenu (%.2e) %d ; All Sound Off : revocation, verrou, silence %d (traduction CC120/CC123 : TA-22w)",
               afterAno, anoOk, asoOk);
  }

  // TA-20 : allocations RT ; coût de la machine d'autorisation (mesure, machine de développement)
  {
    AutonomyGate g;
    TaLog lg;
    TaEvent x;
    const TransportObs seq[4] = {TransportObs::Stopped, TransportObs::Rolling, TransportObs::Rolling, TransportObs::Unknown};
    const auto t0 = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000000; ++i) {
      if (i % 1000 == 0) g.requestSuspend();
      g.beginBlock(seq[i & 3], (i & 7) == 5, (i & 15) == 9, false, static_cast<std::uint64_t>(i) * 256, lg);
      while (lg.pop(x)) {
      }
    }
    const double ns = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - t0).count() / 1e6;
    const bool ok = gAlloc == 0;
    TK_CHECK(ok);
    tk::result("TA-20", ok, "0 allocation dans Engine::process sur toutes les sequences TA (%lld) ; processBlock : TA-20w ; RTSan : CI ; cout de la machine d'autorisation (Windows, ne certifie pas M3/M4 Max) : %.1f ns par bloc hote (journal compris), soit %.4f %% d'un bloc de 256 echantillons a 281 ns/echantillon",
               gAlloc, ns, 100.0 * ns / (256.0 * 281.0));
  }
}

// ------------------------------------------------------------------ R2-01..05 : captures R2 (clôture R50)
ParamValues s2Params(double decay, bool frozen, bool armedSlot = true) {
  ParamValues p = ParamValues::defaults();
  auto slot = [&](DstOff o) -> float& { return p.v[static_cast<size_t>(idx(ParamId::DstGl2Type) + static_cast<int>(o))]; };
  slot(DstOff::Type) = static_cast<float>(DstType::S2);
  slot(DstOff::Mix) = 1.0f;
  slot(DstOff::S2Decay) = static_cast<float>(decay);
  slot(DstOff::S2Freeze) = frozen ? 1.0f : 0.0f;
  slot(DstOff::Arm) = armedSlot ? 1.0f : 0.0f;
  return p;
}

CaptureAsset makeS2Capture(double fs, double decay, const char* bank = "factory:saw", int note = 45) {
  Rig r(s2Params(decay, false), fs, 256, Quality::Normal, bank);
  const int blocks = static_cast<int>(std::lround(0.5 * fs / 256));
  r.step(tinfo(TransportObs::Unknown), {MidiEvent::noteOn(0, note, 110)});
  r.run(blocks, tinfo(TransportObs::Unknown));
  r.p = s2Params(decay, true);
  r.run(blocks, tinfo(TransportObs::Unknown));
  r.e->requestCapture(5);
  r.step(tinfo(TransportObs::Unknown));
  CaptureAsset c;
  r.e->fetchCapture(5, c);
  return c;
}

// énergies par bande de tiers d'octave d'une trame (Hann, zéro-padding 16384)
std::vector<double> bands(const std::vector<float>& L, const std::vector<float>& R, size_t a, size_t n, double fs) {
  std::vector<std::complex<double>> b(16384);
  std::vector<double> p(8193, 0.0);
  for (const auto* x : {&L, &R}) {
    std::fill(b.begin(), b.end(), std::complex<double>(0.0, 0.0));
    for (size_t i = 0; i < n && a + i < x->size(); ++i) b[i] = (*x)[a + i] * (0.5 - 0.5 * std::cos(2.0 * an::kPi * i / n));
    an::fft(b);
    for (size_t k = 0; k <= 8192; ++k) p[k] += std::norm(b[k]);
  }
  std::vector<double> e;
  for (int k = -13; k <= 13; ++k) {
    const double fc = 1000.0 * std::pow(2.0, k / 3.0);
    if (fc * std::pow(2.0, 1.0 / 6.0) > 0.45 * fs) break;
    const double lo = fc / std::pow(2.0, 1.0 / 6.0), hi = fc * std::pow(2.0, 1.0 / 6.0);
    double s = 0.0;
    for (size_t j = static_cast<size_t>(std::ceil(lo * 16384 / fs)); j <= static_cast<size_t>(std::floor(hi * 16384 / fs)) && j <= 8192; ++j) s += p[j];
    e.push_back(s);
  }
  return e;
}

struct Traj {
  double bandRms = 0.0, frameMax = 0.0;  // dB
};
// compare deux rendus à temps écoulé égal depuis l'activation (trames de 100 ms sur `secs`)
Traj compareTraj(const Rig& x, size_t ax, const Rig& y, size_t ay, double fs, double secs, double fMin = 0.0) {
  const size_t n = static_cast<size_t>(std::lround(0.1 * fs));
  double acc = 0.0, cnt = 0.0, fm = 0.0;
  for (size_t f = 0; f + 1 <= static_cast<size_t>(secs * 10.0); ++f) {
    const auto bx = bands(x.L, x.R, ax + f * n, n, fs), by = bands(y.L, y.R, ay + f * n, n, fs);
    double mx = 0.0, ex = 0.0, ey = 0.0;
    for (double v : bx) mx = std::max(mx, v);
    for (size_t k = 0; k < bx.size() && k < by.size(); ++k) {
      const double fc = 1000.0 * std::pow(2.0, (static_cast<int>(k) - 13) / 3.0);
      ex += bx[k];
      ey += by[k];
      if (fc < fMin || bx[k] < mx * 1e-6 || bx[k] <= 0.0 || by[k] <= 0.0) continue;
      const double d = 10.0 * std::log10(by[k] / bx[k]);
      acc += d * d;
      cnt += 1.0;
    }
    if (ex > 0.0 && ey > 0.0) fm = std::max(fm, std::fabs(10.0 * std::log10(ey / ex)));
  }
  return Traj{cnt > 0.0 ? std::sqrt(acc / cnt) : 99.0, fm};
}

size_t activation(const Rig& r) {  // échantillon de début du bloc où la capture a été appliquée
  for (const auto& x : r.log)
    if (x.type == TaEventType::CaptureApplied) return static_cast<size_t>(x.sample);
  return 0;
}

void testR2() {
  const auto Stop = tinfo(TransportObs::Stopped), Play = tinfo(TransportObs::Rolling);
  // R2-01, R2-03, R2-04 : trajectoires (référence activée depuis C ; restaurée, suspendue, réautorisée par le transport)
  double worstBand = 0.0, worstFrame = 0.0, worstSlopeErr = 0.0, worstRender = 0.0;
  bool allOk = true;
  std::string detail;
  for (double fs : {44100.0, 48000.0, 96000.0})
    for (double decay : {0.3, 0.5, 0.8, 0.95, 1.0}) {
      const CaptureAsset c = makeS2Capture(fs, decay);
      const int secsBlocks = static_cast<int>(std::lround(5.6 * fs / 256));
      Rig a(s2Params(decay, true), fs);
      publish(*a.e, {c}, true);
      a.run(secsBlocks, Stop);
      Rig b(s2Params(decay, true), fs);
      restoreSuspended(b, s2Params(decay, true), false, {c});
      b.run(40, Stop);
      b.run(secsBlocks, Play);
      Rig rr(s2Params(decay, true), fs);  // R2-04 : restitution par le rendu hors temps réel
      restoreSuspended(rr, s2Params(decay, true), false, {c});
      rr.run(41, tinfo(TransportObs::Rolling, false, true));  // premier bloc : grant RENDER
      rr.run(secsBlocks, tinfo(TransportObs::Rolling, false, true));
      const size_t a0 = activation(a), b0 = activation(b), r0 = activation(rr);
      const Traj t = compareTraj(a, a0, b, b0, fs, 5.0), tr = compareTraj(a, a0, rr, r0, fs, 5.0);
      // R2-03 : pente d'énergie (régression sur les trames 0,5..5 s) vs -6,02 dB par demi-vie
      const size_t n = static_cast<size_t>(std::lround(0.1 * fs));
      double sx = 0, sy = 0, sxx = 0, sxy = 0, m = 0;
      for (int f = 5; f < 50; ++f) {
        const double e = rms(a.L, a0 + f * n, a0 + (f + 1) * n);
        const double db = 20.0 * std::log10(std::max(e, 1e-30)), x = f * 0.1;
        sx += x, sy += db, sxx += x * x, sxy += x * db, m += 1;
      }
      const double slope = (m * sxy - sx * sy) / (m * sxx - sx * sx);
      const double T = 0.1 + 30.0 * decay * decay, want = decay >= 0.999 ? 0.0 : -6.0206 / T;
      const double slopeErr = decay >= 0.999 ? std::fabs(slope * 4.5) : std::fabs(slope - want) / std::fabs(want);
      const bool slopeOk = decay >= 0.999 ? slopeErr <= 0.1 : slopeErr <= 0.10;
      const bool ok = t.bandRms <= 0.5 && t.frameMax <= 0.5 && tr.bandRms <= 0.5 && tr.frameMax <= 0.5 && slopeOk && b.authorized() && a0 > 0 && b0 > 0;
      allOk = allOk && ok;
      worstBand = std::max(worstBand, t.bandRms);
      worstFrame = std::max(worstFrame, t.frameMax);
      worstRender = std::max(worstRender, std::max(tr.bandRms, tr.frameMax));
      if (decay < 0.999) worstSlopeErr = std::max(worstSlopeErr, slopeErr);
      char s[200];
      std::snprintf(s, sizeof s, " [%.1f kHz Decay %.2f : bandes %.3f dB, trame %.3f dB, rendu %.3f dB, pente %.2f dB/s (contrat %.2f)%s]", fs / 1000.0, decay,
                    t.bandRms, t.frameMax, std::max(tr.bandRms, tr.frameMax), slope, want, ok ? "" : " ECHEC");
      detail += s;
    }
  TK_CHECK(allOk);
  tk::result("R2-01", allOk, "trajectoire S2 (reference activee depuis C vs restauree, suspendue, reautorisee par STOP->PLAY ; et par rendu hors temps reel) : pire ecart RMS par bande %.3f dB, pire trame %.3f dB, pire rendu %.3f dB (<= 0,5) ; pente vs contrat : pire ecart relatif %.1f %% (<= 10 %%) ;%s",
             worstBand, worstFrame, worstRender, 100.0 * worstSlopeErr, detail.c_str());
  tk::result("R2-03", allOk, "decroissance contractuelle (demi-vie T = 0,1 + 30 Decay^2) : pire ecart relatif de pente %.1f %% (<= 10 %%) ; Decay = 1 : energie constante (voir R2-01)", 100.0 * worstSlopeErr);
  tk::result("R2-04", allOk, "restitution via TRANSPORT AUTHORIZATION (STOP->PLAY) et via rendu hors temps reel : criteres R2-01 tenus ; D2 : segment identique bit a bit (TA-12)");

  // R2-02 : contenu initial restauré (Fs égale : exact ; Fs différente : enveloppe par bande)
  {
    const double fs = 48000.0;
    const CaptureAsset c1 = makeS2Capture(fs, 1.0);
    Rig b(s2Params(1.0, true), fs);
    restoreSuspended(b, s2Params(1.0, true), false, {c1});
    b.run(10, Stop);
    b.step(Play);
    b.run(2, Play);
    b.e->requestCapture(5);
    b.step(Play);
    CaptureAsset back;
    const bool got = b.e->fetchCapture(5, back);
    double rel = 0.0;
    for (size_t i = 0; got && i < c1.data.size() && i < back.data.size(); ++i)
      rel = std::max(rel, std::fabs(static_cast<double>(back.data[i]) - c1.data[i]) / std::max(1e-12, std::fabs(static_cast<double>(c1.data[i]))));
    const bool exact = got && back.frames == c1.frames && rel <= 1e-6;
    // Fs différente : capture 48 kHz restituée à 44,1 et 96 kHz ; comparaison des sorties (bandes >= 200 Hz)
    double worst = 0.0;
    std::string detail2, analysis;
    Rig ref(s2Params(1.0, true), fs);
    publish(*ref.e, {c1}, true);
    ref.run(200, Stop);
    for (double fs2 : {44100.0, 96000.0}) {
      Rig o(s2Params(1.0, true), fs2);
      publish(*o.e, {c1}, true);
      o.run(static_cast<int>(200 * fs2 / fs), Stop);
      const size_t ra = activation(ref) + static_cast<size_t>(0.1 * fs), oa = activation(o) + static_cast<size_t>(0.1 * fs2);
      // trames de même durée : comparaison des niveaux par bande (normalisés à la durée de trame)
      const auto br = bands(ref.L, ref.R, ra, static_cast<size_t>(0.1 * fs), fs), bo = bands(o.L, o.R, oa, static_cast<size_t>(0.1 * fs2), fs2);
      double mx = 0.0;
      for (double v : br) mx = std::max(mx, v);
      // analyse descriptive (le verdict reste le critère par bande) : perte d'énergie totale ; écart pondéré par
      // l'énergie ; pire écart parmi les bandes à moins de 20 dB du maximum ; niveau des bandes hors tolérance
      double et = 0.0, eo = 0.0, wacc = 0.0, wsum = 0.0, strong = 0.0;
      std::string outliers;
      for (size_t k = 0; k < br.size() && k < bo.size(); ++k) {
        const double fc = 1000.0 * std::pow(2.0, (static_cast<int>(k) - 13) / 3.0);
        if (fc < 200.0) continue;
        et += br[k] / (0.1 * fs);
        eo += bo[k] / (0.1 * fs2);
        if (br[k] < mx * 1e-4 || bo[k] <= 0.0) continue;
        const double dd = 10.0 * std::log10((bo[k] / (0.1 * fs2)) / (br[k] / (0.1 * fs)));
        wacc += br[k] * dd * dd;
        wsum += br[k];
        if (br[k] >= mx * 1e-2) strong = std::max(strong, std::fabs(dd));
        if (std::fabs(dd) > 0.5)
          outliers += " " + std::to_string(static_cast<int>(fc)) + " Hz (bande a " + std::to_string(static_cast<int>(std::lround(10.0 * std::log10(br[k] / mx)))) + " dB : " +
                      std::to_string(dd).substr(0, 5) + " dB)";
      }
      analysis += " [" + std::to_string(static_cast<int>(fs2)) + " Hz : energie totale " + std::to_string(10.0 * std::log10(eo / et)).substr(0, 6) +
                  " dB ; ecart pondere par l'energie " + std::to_string(std::sqrt(wacc / wsum)).substr(0, 5) + " dB ; pire bande a -20 dB du max " +
                  std::to_string(strong).substr(0, 5) + " dB ; hors tolerance :" + (outliers.empty() ? " aucune" : outliers) + "]";
      for (size_t k = 0; k < br.size() && k < bo.size(); ++k) {
        const double fc = 1000.0 * std::pow(2.0, (static_cast<int>(k) - 13) / 3.0);
        if (fc < 200.0 || br[k] < mx * 1e-4 || bo[k] <= 0.0) continue;
        // énergie d'une bande (somme des bins, Parseval) proportionnelle à la durée de trame n : normalisation par n
        const double d = std::fabs(10.0 * std::log10((bo[k] / (0.1 * fs2)) / (br[k] / (0.1 * fs))));
        detail2 += " " + std::to_string(static_cast<int>(fs2 / 100) / 10.0).substr(0, 4) + "k/" + std::to_string(static_cast<int>(fc)) + "Hz:" +
                   std::to_string(10.0 * std::log10((bo[k] / (0.1 * fs2)) / (br[k] / (0.1 * fs)))).substr(0, 5);
        worst = std::max(worst, d);
      }
    }
    const bool ok = exact && worst <= 0.5;
    TK_CHECK(ok);
    tk::result("R2-02", ok, "contenu initial : spectre tenu relu apres application = capture (Decay 1, ecart relatif max %.2e <= 1e-6, trames %d/%d) %d ; capture 48 kHz restituee a 44,1 et 96 kHz : pire ecart par bande (>= 200 Hz, -40 dB) %.3f dB (<= 0,5) ; analyse :%s",
               rel, back.frames, c1.frames, exact, worst, analysis.c_str());
  }

  // R2-05 : Panic pendant un gel restauré ; réautorisation = capture (et non l'état décru)
  {
    const double fs = 48000.0;
    const CaptureAsset c = makeS2Capture(fs, 0.3);
    Rig a(s2Params(0.3, true), fs);
    restoreSuspended(a, s2Params(0.3, true), false, {c});
    a.run(10, Stop);
    a.run(30, Play);
    const size_t n = 4800;
    const size_t a0 = activation(a);
    a.run(600, Play);  // ~3 s : décroissance (demi-vie 2,8 s)
    const double initial = rms(a.L, a0 + 2 * n, a0 + 3 * n), decayed = a.rmsLast(19);
    a.step(Play, {MidiEvent::allSoundOff(0)});
    a.run(40, Play);
    const double silence = a.rmsLast(30);
    const size_t mark = a.L.size();
    a.run(5, Stop);
    a.run(200, Play);
    size_t a1 = 0;
    for (const auto& x : a.log)
      if (x.type == TaEventType::CaptureApplied && x.sample >= mark) a1 = static_cast<size_t>(x.sample);
    const double again = rms(a.L, a1 + 2 * n, a1 + 3 * n);
    const double dAgain = std::fabs(20.0 * std::log10(again / initial)), dDecayed = 20.0 * std::log10(decayed / initial);
    // contenu = capture : spectre tenu relu après la réapplication = C x r^m (m trames de décroissance écoulées depuis
    // l'application, r = 0,5^(trame/T)), et non l'état décru d'avant Panic
    Rig chk(s2Params(0.3, true), fs);
    restoreSuspended(chk, s2Params(0.3, true), false, {c});
    chk.run(10, Stop);
    a.e->requestCapture(5);
    a.step(Play);
    CaptureAsset back;
    const bool got = a.e->fetchCapture(5, back);
    const double r = std::pow(0.5, (512.0 / fs) / (0.1 + 30.0 * 0.3 * 0.3));
    double best = 1e9;
    int bestM = -1;
    for (int m = 0; m < 400 && got; ++m) {
      double rel = 0.0;
      const double g = std::pow(r, m);
      for (size_t i = 0; i < c.data.size() && i < back.data.size(); ++i)
        if (std::fabs(c.data[i]) > 1e-9 * 1e3) rel = std::max(rel, std::fabs(back.data[i] - g * c.data[i]) / (g * std::fabs(c.data[i])));
      if (rel < best) best = rel, bestM = m;
    }
    const double elapsedFrames = static_cast<double>(a.L.size() - a1) / 512.0;
    const bool content = got && best <= 1e-6 && bestM <= static_cast<int>(elapsedFrames) + 1;
    const bool ok = silence < 1e-9 && a1 > mark && content && dDecayed < -3.0;
    TK_CHECK(ok);
    tk::result("R2-05", ok, "Panic pendant un gel restaure : silence %.2e (< 1e-9) ; reautorisation (STOP->PLAY) : spectre tenu = capture x r^%d (ecart relatif %.2e <= 1e-6 ; %d trames ecoulees depuis la reapplication), et non l'etat decru avant Panic (%.2f dB) ; descriptif : niveau de trame vs premiere activation %.3f dB",
               silence, bestM, best, static_cast<int>(elapsedFrames), dDecayed, dAgain);
  }
}


// ------------------------------------------------------------------ R2-02R (ARCH-G4.3-R1) : restitution inter-Fs
void writeWav24(const std::string& path, const std::vector<float>& L, const std::vector<float>& R, size_t a, size_t n, double fs, double gain) {
  FILE* f = std::fopen(path.c_str(), "wb");
  if (!f) return;
  const std::uint32_t sr = static_cast<std::uint32_t>(fs), data = static_cast<std::uint32_t>(n * 6);
  auto u32 = [&](std::uint32_t v) { std::fwrite(&v, 4, 1, f); };
  auto u16 = [&](std::uint16_t v) { std::fwrite(&v, 2, 1, f); };
  std::fwrite("RIFF", 1, 4, f);
  u32(36 + data);
  std::fwrite("WAVEfmt ", 1, 8, f);
  u32(16), u16(1), u16(2), u32(sr), u32(sr * 6), u16(6), u16(24);
  std::fwrite("data", 1, 4, f);
  u32(data);
  for (size_t i = a; i < a + n && i < L.size(); ++i)
    for (const auto* x : {&L, &R}) {
      const long s = std::lround(std::fmax(-1.0, std::fmin(1.0, (*x)[i] * gain)) * 8388607.0);
      const unsigned char b[3] = {static_cast<unsigned char>(s & 0xff), static_cast<unsigned char>((s >> 8) & 0xff), static_cast<unsigned char>((s >> 16) & 0xff)};
      std::fwrite(b, 1, 3, f);
    }
  std::fclose(f);
}

void testR2Resolution() {
  const double rates[4] = {44100.0, 48000.0, 88200.0, 96000.0};
  struct Src {
    const char* name;
    const char* bank;
    int note;
  };
  const Src srcs[2] = {{"scie note 45", "factory:saw", 45}, {"Metal note 52", "factory:metal", 52}};
  const char* wavDir = std::getenv("SBD_R2_WAV_DIR");
  double worstTotal = 0.0, worstWeighted = 0.0, worstStrong = 0.0;
  bool exact = true, ok = true;
  std::string fails, listing;
  int combos = 0;
  for (const auto& src : srcs) {
    for (double fc : rates) {
      const CaptureAsset c = makeS2Capture(fc, 1.0, src.bank, src.note);
      // référence : capture restituée à sa propre fréquence
      Rig ref(s2Params(1.0, true), fc, 256, Quality::Normal, src.bank);
      publish(*ref.e, {c}, true);
      ref.run(static_cast<int>(1.2 * fc / 256), tinfo(TransportObs::Stopped));
      ref.e->requestCapture(5);
      ref.step(tinfo(TransportObs::Stopped));
      CaptureAsset back;
      if (!(ref.e->fetchCapture(5, back) && back.data == c.data)) exact = false;  // (e) Fr = Fc
      const size_t ra = activation(ref) + static_cast<size_t>(0.2 * fc), rn = static_cast<size_t>(0.1 * fc);
      const auto br = bands(ref.L, ref.R, ra, rn, fc);
      struct Wav {
        double fr;
        std::vector<float> L, R;
        size_t act;
      };
      std::vector<Wav> wavs;
      for (double fr : rates) {
        if (fr == fc) continue;
        ++combos;
        Rig o(s2Params(1.0, true), fr, 256, Quality::Normal, src.bank);
        publish(*o.e, {c}, true);
        o.run(static_cast<int>(1.2 * fr / 256), tinfo(TransportObs::Stopped));
        const size_t oa = activation(o) + static_cast<size_t>(0.2 * fr), on = static_cast<size_t>(0.1 * fr);
        const auto bo = bands(o.L, o.R, oa, on, fr);
        const double binc = fc * Engine::oversamplingFor(fc) / 2048.0, binr = fr * Engine::oversamplingFor(fr) / 2048.0, bin = std::max(binc, binr);
        const double fmax = 0.45 * std::min(fc, fr);
        double mx = 0.0;
        for (double v : br) mx = std::max(mx, v);
        double et = 0.0, eo = 0.0, wacc = 0.0, wsum = 0.0, strong = 0.0, etAll = 0.0, eoAll = 0.0;
        std::string lowList;
        for (size_t k = 0; k < br.size() && k < bo.size(); ++k) {  // descriptif : toutes bandes (50 Hz - fmax)
          const double fcb = 1000.0 * std::pow(2.0, (static_cast<int>(k) - 13) / 3.0);
          if (fcb * std::pow(2.0, 1.0 / 6.0) > fmax) continue;
          etAll += br[k] / (0.1 * fc);
          eoAll += bo[k] / (0.1 * fr);
        }
        for (size_t k = 0; k < br.size() && k < bo.size(); ++k) {
          const double fcb = 1000.0 * std::pow(2.0, (static_cast<int>(k) - 13) / 3.0);
          const double lo = fcb / std::pow(2.0, 1.0 / 6.0), hi = fcb * std::pow(2.0, 1.0 / 6.0);
          if (fcb < 200.0 || hi > fmax) continue;
          const double er = br[k] / (0.1 * fc), eob = bo[k] / (0.1 * fr);
          et += er;
          eo += eob;
          if (br[k] < mx * 1e-6 || bo[k] <= 0.0) continue;
          const double d = 10.0 * std::log10(eob / er), lvl = 10.0 * std::log10(br[k] / mx);
          const bool resolvable = (hi - lo) >= 8.0 * bin;
          if (resolvable) {
            wacc += er * d * d;
            wsum += er;
            if (lvl >= -20.0) strong = std::max(strong, std::fabs(d));
          }
          if ((!resolvable || lvl < -20.0) && std::fabs(d) > 1.0 && lowList.size() < 300) {
            char b[96];
            std::snprintf(b, sizeof b, " %.0fHz(%.0fdB,%.1fbins,%+.1fdB)", fcb, lvl, (hi - lo) / bin, d);
            lowList += b;
          }
        }
        const double total = std::fabs(10.0 * std::log10(eo / et)), weighted = wsum > 0.0 ? std::sqrt(wacc / wsum) : 0.0;
        worstTotal = std::max(worstTotal, total);
        worstWeighted = std::max(worstWeighted, weighted);
        worstStrong = std::max(worstStrong, strong);
        const bool good = total <= 1.0 && weighted <= 0.5 && strong <= 1.0;
        ok = ok && good;
        char b[400];
        std::snprintf(b, sizeof b, " [%s %.1f->%.1f kHz : total %.2f dB (toutes bandes %+.2f dB), pondere %.2f dB, pire bande resoluble forte %.2f dB%s%s]", src.name, fc / 1000.0,
                      fr / 1000.0, total, 10.0 * std::log10(eoAll / etAll), weighted, strong, lowList.empty() ? "" : " ; hors tolerance non resolubles/faibles :", lowList.c_str());
        std::printf("  R2-02R%s\n", b);
        if (!good) fails += b;
        if (listing.size() < 2400) listing += b;
        if (wavDir && src.bank[8] == 's' && fc == 48000.0) wavs.push_back(Wav{fr, o.L, o.R, activation(o)});
      }
      // (f) WAV A/B : capture 48 kHz, scie ; même gain pour tous les fichiers du groupe (aucune normalisation indépendante)
      if (wavDir && src.bank[8] == 's' && fc == 48000.0) {
        double pk = peak(ref.L);
        for (auto& w : wavs) pk = std::max(pk, peak(w.L));
        const double g = 0.5 / std::max(pk, 1e-9);
        writeWav24(std::string(wavDir) + "/r2ab_capture48k_restore48k_reference.wav", ref.L, ref.R, activation(ref), static_cast<size_t>(fc), fc, g);
        for (auto& w : wavs) {
          char name[160];
          std::snprintf(name, sizeof name, "/r2ab_capture48k_restore%.1fk.wav", w.fr / 1000.0);
          writeWav24(std::string(wavDir) + name, w.L, w.R, w.act, static_cast<size_t>(w.fr), w.fr, g);  // 1 s depuis l'activation
        }
      }
    }
  }
  const bool all = ok && exact;
  TK_CHECK(all);
  tk::result("R2-02R", all, "restitution inter-Fs (Decay 1, 2 sources, %d couples Fc->Fr, 44,1/48/88,2/96 kHz) : (a) energie totale pire %.2f dB (<= 1,0) ; (b) ecart pondere par l'energie (bandes resolubles >= 8 bins) pire %.2f dB (<= 0,5) ; (c) bandes resolubles a >= -20 dB : pire %.2f dB (<= 1,0) ; (e) Fr = Fc exact %d ; (d) detail :%s%s",
             combos, worstTotal, worstWeighted, worstStrong, exact, listing.c_str(), fails.empty() ? "" : (" ; ECHECS :" + fails).c_str());
}

}  // namespace

void runG43TaTests() {
  std::printf("[ARCH-G4.3 : TRANSPORT AUTHORIZATION]\n");
  const auto caps = makeCaptures();
  const bool capOk = caps.size() == 2 && caps[0].kind == 0 && caps[1].kind == 1;
  TK_CHECK(capOk);
  if (!capOk) return;
  testTransportAuthorization(caps);
  std::printf("[ARCH-G4.3 : captures R2]\n");
  testR2();
  testR2Resolution();
}

