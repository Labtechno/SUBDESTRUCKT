// SUBDESTRUCKT — tests ARCH-G4 Preset & State (ST-RND, UNDO, EVO, PRE). Seuils : docs/V1-ARCH-G4-TRACKER.md §3.
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include "G4Helper.h"
#include "TestKit.h"
#include "core/PresetOps.h"
#include "core/WtAssets.h"

using namespace sbd;

namespace {

double normDiff(ParamId id, float a, float b) {
  const auto& s = paramSpec(id);
  if ((s.domain == Domain::Oct || s.domain == Domain::Time) && s.minValue > 0.0f)
    return std::fabs(std::log(a / b) / std::log(s.maxValue / s.minValue));
  return std::fabs(a - b) / (s.maxValue - s.minValue);
}

void testRandomize() {
  PluginState base = defaultState();
  RandomOptions opt;
  opt.locks.insert(ParamId::WtX);
  opt.locks.insert(ParamId::Op2Ratio);
  opt.families[static_cast<size_t>(RandomFamily::Destruction)] = false;
  PluginState a = base, b = base, c = base;
  const auto ra = randomizeState(a, opt), rb = randomizeState(b, opt);
  c.streamCounters[0] = 12345;
  randomizeState(c, opt);
  bool ok = ra.ok && rb.ok && a == b && !(a.params == c.params) && ra.changed > 20;
  ok = ok && a.params[ParamId::WtX] == base.params[ParamId::WtX] && a.params[ParamId::Op2Ratio] == base.params[ParamId::Op2Ratio];
  bool famOk = true;
  for (int i = 0; i < kNumParams; ++i) {
    const auto id = static_cast<ParamId>(i);
    if (paramInFamily(id, RandomFamily::Destruction) || paramSpec(i).group == std::string("out") || paramSpec(i).group == std::string("voice"))
      famOk = famOk && a.params.v[static_cast<size_t>(i)] == base.params.v[static_cast<size_t>(i)];
  }
  ok = ok && famOk && a.streamCounters[0] > base.streamCounters[0];
  std::string err;
  ok = ok && validateState(a, err);
  RandomOptions zero = opt;
  zero.intensity = 0.0f;
  PluginState z = base;
  randomizeState(z, zero);
  ok = ok && z == base;
  TK_CHECK(ok);
  tk::result("ST-RND-01", ok, "Randomize : deterministe (seed+compteur), compteur different -> different, %d parametres changes, verrous et familles respectes %d, intensite 0 neutre, etat valide", ra.changed, famOk);

  // Mutate : bornes
  bool mok = true;
  for (RandomProfile pr : {RandomProfile::Controlled, RandomProfile::Industrial, RandomProfile::Experimental}) {
    RandomOptions mo;
    mo.profile = pr;
    mo.intensity = 0.7f;
    PluginState m = a;
    const auto r = mutateState(m, mo);
    const double sc = pr == RandomProfile::Controlled ? 0.3 : (pr == RandomProfile::Industrial ? 0.6 : 1.0);
    for (int i = 0; i < kNumParams; ++i) {
      const auto& s = paramSpec(i);
      if (s.type != ParamType::Float) continue;
      mok = mok && normDiff(static_cast<ParamId>(i), m.params.v[static_cast<size_t>(i)], a.params.v[static_cast<size_t>(i)]) <= 0.7 * sc * 0.15 + 1e-6;
    }
    PluginState m2 = a;
    mutateState(m2, mo);
    mok = mok && r.ok && m == m2;
  }
  TK_CHECK(mok);
  tk::result("ST-RND-02", mok, "Mutate : ecart normalise <= intensite x echelle du profil x 0,15 (Controlled/Industrial/Experimental), deterministe");

  // refus transactionnel : état invalide en entrée -> refus, rien ne change
  PluginState bad = base;
  bad.bank = "../evil";
  const PluginState before = bad;
  const auto rr = randomizeState(bad, opt);
  const bool tok = !rr.ok && bad == before;
  TK_CHECK(tok);
  tk::result("ST-RND-03", tok, "transaction refusee (etat invalide) : etat et compteurs inchanges");
}

void testUndo() {
  UndoStack u;
  PluginState s = defaultState();
  std::vector<PluginState> hist{s};
  RandomOptions opt;
  for (int k = 0; k < 140; ++k) {
    PluginState n = hist.back();
    randomizeState(n, opt);
    u.push(hist.back(), n, "randomize");
    hist.push_back(n);
  }
  bool ok = u.size() == UndoStack::kCapacity;
  // undo jusqu'au bout : on retrouve l'état d'il y a 128 transactions
  const PluginState* cur = nullptr;
  int n = 0;
  while (const PluginState* p = u.undo()) {
    cur = p;
    ++n;
  }
  ok = ok && n == 128 && cur && *cur == hist[hist.size() - 1 - 128];
  // redo exact, sans tirage (compteurs restaurés)
  const PluginState* r1 = u.redo();
  ok = ok && r1 && *r1 == hist[hist.size() - 128] && r1->streamCounters == hist[hist.size() - 128].streamCounters;
  // nouvelle transaction : redo effacé
  PluginState x = *r1;
  x.params[ParamId::WtX] = 0.77f;
  u.push(*r1, x, "edit");
  ok = ok && !u.canRedo() && u.undoCause() == "edit";
  const int before = u.size();
  u.push(x, x, "doublon");
  ok = ok && u.size() == before;
  TK_CHECK(ok);
  tk::result("UNDO-01", ok, "Undo/Redo : capacite 128 (140 poussees), restauration exacte avec compteurs RNG, redo sans tirage, redo efface par une nouvelle transaction, doublon ignore ; memoire estimee %zu octets", u.memoryEstimate());
}

void testEvolve() {
  PluginState s = defaultState();
  RandomOptions opt;
  opt.intensity = 0.8f;
  opt.profile = RandomProfile::Industrial;
  const double bound = 0.8 * 0.6 * 0.25;
  auto run = [&](EvolveMode m, double& ms) {
    Evolver e;
    e.start(s, opt, m, 0.5f);
    bool within = true;
    ms = 0.0;
    int cnt = 0;
    for (int t = 0; t < 60 * 30; ++t) {
      e.step(1.0 / 30.0);
      for (int i = 0; i < kNumParams; ++i) {
        const float o = e.offsets()[static_cast<size_t>(i)];
        within = within && std::fabs(o) <= bound + 1e-6;
        if (Evolver::whitelisted(static_cast<ParamId>(i)) && o != 0.0f) {
          ms += o * o;
          ++cnt;
        }
        if (!Evolver::whitelisted(static_cast<ParamId>(i))) within = within && o == 0.0f;
      }
    }
    ms = cnt ? ms / cnt : 0.0;
    return std::make_pair(within, e);
  };
  double msA, msD;
  auto a = run(EvolveMode::Anchored, msA);
  auto d = run(EvolveMode::Drift, msD);
  PluginState c = s;
  const bool committed = a.second.commit(c);
  bool commitOk = committed;
  for (int i = 0; i < kNumParams; ++i)
    commitOk = commitOk && c.params.v[static_cast<size_t>(i)] == sanitize(static_cast<ParamId>(i), a.second.apply(static_cast<ParamId>(i), s.params.v[static_cast<size_t>(i)]));
  a.second.reset();
  bool resetOk = true;
  for (float o : a.second.offsets()) resetOk = resetOk && o == 0.0f;
  std::string err;
  const bool ok = a.first && d.first && msA < msD && commitOk && resetOk && validateState(c, err);
  TK_CHECK(ok);
  tk::result("EVO-01", ok, "Evolve : decalages bornes (<= %.3f) Anchored/Drift %d/%d ; moyenne quadratique Anchored %.2e < Drift %.2e ; Commit = Base + decalage %d ; Reset nul %d ; hors liste blanche inchange",
             bound, a.first, d.first, msA, msD, commitOk, resetOk);
}

void testLibrary() {
  const std::string dir = (std::filesystem::temp_directory_path() / "sbd_presets_test").string();
  std::error_code ec;
  std::filesystem::remove_all(dir, ec);
  PresetLibrary lib(dir);
  bool ok = true;
  std::string err;
  int factory = 0;
  for (const auto& p : lib.list()) {
    PluginState s;
    ok = ok && lib.load(p, s, err) && validateState(s, err);
    ++factory;
  }
  PluginState s = defaultState();
  RandomOptions opt;
  randomizeState(s, opt);
  ok = ok && lib.save("Mon preset/1", s, {"test", "dark"}, err);
  const auto items = lib.list();
  PresetInfo mine;
  for (const auto& p : items)
    if (!p.factory) mine = p;
  PluginState back;
  ok = ok && !mine.path.empty() && lib.load(mine, back, err) && back.params == s.params && back.routes == s.routes && back.mod == s.mod &&
       mine.tags.size() == 2 && !back.hasInstance;
  ok = ok && lib.setFavorite(mine, true, err);
  for (const auto& p : lib.list())
    if (!p.factory) ok = ok && p.favorite;
  {
    std::ofstream bad(dir + "/corrompu.sdpreset");
    bad << "SUBDESTRUCKT_STATE 2\nscope preset\nparam wt.x 0xzz\n";
  }
  PresetInfo c;
  c.path = dir + "/corrompu.sdpreset";
  PluginState junk;
  ok = ok && !lib.load(c, junk, err);
  std::filesystem::remove_all(dir, ec);
  TK_CHECK(ok);
  tk::result("PRE-01", ok, "bibliotheque : %d presets usine valides et chargeables ; sauvegarde atomique/rechargement exact (scope preset, tags) ; favori persistant ; fichier corrompu refuse", factory);
}

void testAssets() {
  bool ok = sha256Hex("abc", 3) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" &&
            sha256Hex("", 0) == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
  // WAV float32 : écriture puis lecture identiques
  const std::string dir = (std::filesystem::temp_directory_path() / "sbd_wta_test").string();
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  std::vector<float> sig(2048 * 6);
  for (size_t i = 0; i < sig.size(); ++i) {
    const double ph = static_cast<double>(i % 2048) / 2048.0;
    const int w = static_cast<int>(i / 2048);
    sig[i] = static_cast<float>(std::sin(2.0 * 3.14159265358979 * ph) * (1.0 - 0.1 * w) + 0.1 * w * std::sin(6.0 * 3.14159265358979 * ph));
  }
  std::string err;
  const std::string wav = dir + "/cycles.wav";
  std::vector<float> back;
  double sr = 0.0;
  ok = ok && writeWavFloat(wav, sig, 48000.0, err) && readWavMono(wav, back, sr, err) && back == sig && sr == 48000.0;
  // import de cycles 2048 : contenu brut exact ; identifiant déterministe
  auto b1 = bankFromCycles(sig, 2048, "test", err), b2 = bankFromCycles(sig, 2048, "test", err);
  ok = ok && b1 && b2 && b1->id() == b2->id() && b1->raw() == sig && b1->nx() == 6 && b1->ny() == 1 && b1->id().rfind("user:", 0) == 0;
  // cycle 1024 sinus -> sinus 2048
  std::vector<float> s1(1024 * 2);
  for (size_t i = 0; i < s1.size(); ++i) s1[i] = static_cast<float>(std::sin(2.0 * 3.14159265358979 * static_cast<double>(i % 1024) / 1024.0));
  auto b3 = bankFromCycles(s1, 1024, "sin", err);
  double worst = 0.0;
  if (b3)
    for (int i = 0; i < 2048; ++i) worst = std::max(worst, std::fabs(static_cast<double>(b3->raw()[static_cast<size_t>(i)]) - std::sin(2.0 * 3.14159265358979 * i / 2048.0)));
  ok = ok && b3 && worst < 1e-5;
  // état avec banque intégrée : aller-retour exact, altération refusée
  PluginState s = defaultState();
  s.bank = b1->id();
  s.bankRaw = b1->raw();
  s.bankNx = b1->nx();
  s.bankNy = b1->ny();
  s.bankName = "test";
  PluginState d;
  const std::string txt = encodeState(s, "w", StateScope::Preset);
  ok = ok && decodeState(txt, d, err) && d.bankRaw == s.bankRaw && d.bank == s.bank;
  std::string bad = txt;
  const size_t p = bad.find("bankdata ");
  const size_t q = bad.find(' ', bad.find(' ', bad.find(' ', bad.find(' ', p) + 1) + 1) + 1) + 10;
  bad[q] = bad[q] == 'A' ? 'B' : 'A';
  PluginState junk;
  ok = ok && !decodeState(bad, junk, err);
  std::filesystem::remove_all(dir, ec);
  TK_CHECK(ok);
  tk::result("WTA-01", ok, "SHA-256 (vecteurs FIPS) ; WAV float32 aller-retour exact ; import cycles 2048 exact, id deterministe %s ; cycle 1024 -> 2048 ecart %.2g (< 1e-5) ; banque integree a l'etat : aller-retour exact, alteration refusee",
             b1 ? b1->id().c_str() : "-", worst);
}

}  // namespace

void runStateTests2() {
  std::printf("[arch-g4 etat]\n");
  testRandomize();
  testUndo();
  testEvolve();
  testLibrary();
  testAssets();
}
