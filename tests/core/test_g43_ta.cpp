// SUBDESTRUCKT — tests ARCH-G4.3 : TRANSPORT AUTHORIZATION (moteur, transport simulé). docs/V1-ARCH-G4.3-SPEC.md §3.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
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
  explicit Rig(const ParamValues& pv, double fs = 48000.0, int blk = 256, Quality q = Quality::Normal) : p(pv), block(blk) {
    e = std::make_unique<Engine>(FactoryBanks::get("factory:saw"));
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

  // TA-20 : allocations RT
  {
    const bool ok = gAlloc == 0;
    TK_CHECK(ok);
    tk::result("TA-20", ok, "0 allocation dans Engine::process sur toutes les sequences TA (%lld) ; processBlock : TA-20w ; RTSan : CI", gAlloc);
  }
}

}  // namespace

void runG43TaTests() {
  std::printf("[ARCH-G4.3 : TRANSPORT AUTHORIZATION]\n");
  const auto caps = makeCaptures();
  const bool capOk = caps.size() == 2 && caps[0].kind == 0 && caps[1].kind == 1;
  TK_CHECK(capOk);
  if (!capOk) return;
  testTransportAuthorization(caps);
}

