// SUBDESTRUCKT — AUD-01 : audit « chaque contrôle visible pilote une fonction réelle » (ARCH-G4.1, tracker §3bis).
// Pour chaque paramètre du registre : contexte d'activation déduit de sa famille (algorithme sélectionné, route
// présente, source de modulation routée, gamme active...), valeurs extrêmes ou choix alternatifs, rendu comparé
// bit à bit au rendu du contexte. Un paramètre sans effet mesurable doit figurer dans la liste des exceptions
// justifiées (effet hors de la fenêtre de rendu, couvert par un critère dédié) ; sinon le critère échoue.
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "G4Helper.h"
#include "TestKit.h"
#include "dsp/Destruction.h"

using namespace sbd;
using namespace g4;

namespace {

using Ctx = std::function<void(PluginState&, std::vector<MidiEvent>&)>;

bool startsWith(const std::string& s, const char* p) { return s.rfind(p, 0) == 0; }
bool endsWith(const std::string& s, const std::string& e) { return s.size() >= e.size() && s.compare(s.size() - e.size(), e.size(), e) == 0; }

void setKey(PluginState& st, const std::string& key, float v) {
  ParamId id;
  if (findParam(key, id)) st.params[id] = sanitize(id, v);
}

void addRoute(PluginState& st, int src, int dst) {
  RouteTable t;
  std::string err;
  if (!t.assign(st.routes, st.nextRouteId, err)) return;
  if (t.findPair(src, dst)) return;
  const auto r = t.apply({RouteTable::Op::add(src, dst)});
  if (!r.ok) return;
  st.routes = t.routes();
  st.nextRouteId = t.nextRouteId();
}

void addMod(PluginState& st, ModSource s, ParamId d, float depth) {
  ModRoute r;
  r.source = s;
  r.dest = d;
  r.depth = depth;
  modAdd(st.mod, r);
}

std::vector<MidiEvent> baseEvents(int a = 50, int b = 55) {
  return {MidiEvent::noteOn(0, a, 100),          MidiEvent::pitchBend(1000, 8191), MidiEvent::controller(1500, 1, 100),
          MidiEvent::channelPressure(2000, 90), MidiEvent::noteOn(3000, b, 70),   MidiEvent::pitchBend(4000, -8192),
          MidiEvent::noteOff(6000, b),          MidiEvent::noteOff(7000, a)};
}

void opsOn(PluginState& st) {
  for (int k = 1; k <= 4; ++k) {
    setKey(st, "op" + std::to_string(k) + ".level", 0.5f);
    setKey(st, "op" + std::to_string(k) + ".enable", 1.0f);
  }
}

// contextes d'activation candidats d'un paramètre (l'effet est retenu s'il apparaît dans l'un d'eux)
std::vector<Ctx> contextsFor(int p) {
  const std::string key = paramSpec(p).stableId;
  std::vector<Ctx> out;
  out.push_back([](PluginState&, std::vector<MidiEvent>&) {});
  static const char* kAlgo[9] = {"", "n1", "d1", "y1", "s1", "n2", "d2", "y2", "s2"};
  if (startsWith(key, "dst.")) {
    const std::string slot = key.substr(0, key.find('.', 4));  // dst.gl1
    const std::string rest = key.substr(slot.size() + 1);
    int algo = 1;
    for (int a = 1; a <= 8; ++a)
      if (startsWith(rest, (std::string(kAlgo[a]) + ".").c_str())) algo = a;
    out.push_back([slot, algo](PluginState& st, std::vector<MidiEvent>&) { setKey(st, slot + ".type", static_cast<float>(algo)); });
    if (algo == 7)  // Y2 : sensibilité maximale, détection rapide
      out.push_back([slot](PluginState& st, std::vector<MidiEvent>&) {
        setKey(st, slot + ".type", 7.0f);
        setKey(st, slot + ".y2.depth", 1.0f);
        setKey(st, slot + ".y2.sens", 1.0f);
        setKey(st, slot + ".y2.attack", 1.0f);
        setKey(st, slot + ".y2.hold", 20.0f);
        setKey(st, slot + ".y2.release", 20.0f);
      });
    if (algo == 1)  // N1 : drive/fold élevés pour révéler tone, étages, symétrie
      out.push_back([slot](PluginState& st, std::vector<MidiEvent>&) {
        setKey(st, slot + ".type", 1.0f);
        setKey(st, slot + ".n1.drive", 18.0f);
        setKey(st, slot + ".n1.fold", 0.6f);
      });
    if (algo == 5)  // N2 : feedback élevé
      out.push_back([slot](PluginState& st, std::vector<MidiEvent>&) {
        setKey(st, slot + ".type", 5.0f);
        setKey(st, slot + ".n2.fb", 0.9f);
      });
    if (algo == 5)  // N2 : auto-oscillation (armement requis par contrat)
      out.push_back([slot](PluginState& st, std::vector<MidiEvent>&) {
        setKey(st, slot + ".type", 5.0f);
        setKey(st, slot + ".n2.fb", 0.95f);
        setKey(st, slot + ".arm", 1.0f);
      });
    if (algo == 6 || algo == 4)  // D2 répétitions aléatoires (nouveaux segments : feedback audible), S1 shift
      out.push_back([slot, algo](PluginState& st, std::vector<MidiEvent>&) {
        setKey(st, slot + ".type", static_cast<float>(algo));
        setKey(st, slot + ".d2.repeat", 0.5f);
        setKey(st, slot + ".d2.size", 10.0f);
        setKey(st, slot + ".d2.pos", 0.02f);  // Pos > 0 : le segment remonte dans la zone enregistrée en boucle (feedback)
        setKey(st, slot + ".s1.mode", 1.0f);
      });
    if (algo == 6)  // ARCH-G4.3 : Feedback > 0 en boucle (le mode LEGACY / REGENERATIVE n'agit qu'avec du Feedback)
      out.push_back([slot](PluginState& st, std::vector<MidiEvent>&) {
        setKey(st, slot + ".type", 6.0f);
        setKey(st, slot + ".d2.fb", 0.8f);
        setKey(st, slot + ".d2.repeat", 1.0f);
        setKey(st, slot + ".d2.size", 30.0f);
        setKey(st, slot + ".d2.pos", 0.0f);
      });
    if (algo == 6)  // décisions de segments pseudo-aléatoires (graine par emplacement) : plusieurs densités
      for (float rep : {0.3f, 0.8f})
        out.push_back([slot, rep](PluginState& st, std::vector<MidiEvent>&) {
          setKey(st, slot + ".type", 6.0f);
          setKey(st, slot + ".d2.repeat", rep);
          setKey(st, slot + ".d2.size", 10.0f);
          setKey(st, slot + ".d2.pos", 0.05f);
        });
  } else if (startsWith(key, "flt.")) {
    const std::string slot = key.substr(0, key.find('.', 4));
    for (int t = 1; t <= 4; ++t) {
      out.push_back([slot, t](PluginState& st, std::vector<MidiEvent>&) { setKey(st, slot + ".type", static_cast<float>(t)); });
      out.push_back([slot, t](PluginState& st, std::vector<MidiEvent>&) {
        setKey(st, slot + ".type", static_cast<float>(t));
        setKey(st, slot + ".exc", 1.0f);
        setKey(st, slot + ".res", 0.9f);
        setKey(st, slot + ".cutoff", 800.0f);
        setKey(st, slot + ".key", 0.0f);
      });
      out.push_back([slot, t](PluginState& st, std::vector<MidiEvent>&) {
        const std::string loc = slot.substr(4);  // wt | fm | gl
        setKey(st, slot + ".type", static_cast<float>(t));
        setKey(st, slot + ".stereo", 1.0f);
        setKey(st, slot + ".detune", 12.0f);
        setKey(st, slot + ".spread", 0.5f);
        setKey(st, slot + ".arm", 1.0f);
        setKey(st, slot + ".exc", 1.0f);
        setKey(st, "dst." + loc + "1.type", 1.0f);
        setKey(st, "dst." + loc + "1.n1.drive", 18.0f);
        setKey(st, "dst." + loc + "1.n1.fold", 0.5f);
        setKey(st, "op1.pan", -0.8f);  // signal stéréo
      });
    }
  } else if (startsWith(key, "wt.za.") || startsWith(key, "wt.zb.") || startsWith(key, "wt.z")) {
    for (int t = 1; t <= 3; ++t)
      out.push_back([t](PluginState& st, std::vector<MidiEvent>&) {
        setKey(st, "wt.za.type", static_cast<float>(t));
        setKey(st, "wt.zb.type", static_cast<float>(1 + t % 3));
        setKey(st, "wt.za.depth", 0.6f);
        setKey(st, "wt.zb.depth", 0.6f);
        setKey(st, "wt.z", 0.5f);
      });
    // ARCH-G4.3 : valeurs ajoutées en fin d'énumération (transformations spectrales) sur les deux emplacements ; Shape
    for (int t = 4; t <= 8; ++t)
      out.push_back([t](PluginState& st, std::vector<MidiEvent>&) {
        setKey(st, "wt.za.type", static_cast<float>(t));
        setKey(st, "wt.zb.type", static_cast<float>(4 + (t - 3) % 5));
        setKey(st, "wt.za.depth", 0.6f);
        setKey(st, "wt.zb.depth", 0.6f);
        setKey(st, "wt.z", 0.8f);
      });
  } else if (startsWith(key, "wt.")) {
    out.push_back([](PluginState& st, std::vector<MidiEvent>&) {
      setKey(st, "wt.x", 0.9f);
      setKey(st, "wt.y", 0.9f);
      setKey(st, "wt.sync", 1.0f);
      setKey(st, "wt.char", 0.5f);
      setKey(st, "lfo1.rate", 15.0f);
      addMod(st, ModSource::Lfo1, ParamId::WtX, 1.0f);
      addMod(st, ModSource::Lfo1, ParamId::WtY, 1.0f);
    });
    out.push_back([](PluginState& st, std::vector<MidiEvent>&) {
      setKey(st, "wt.sync", 2.0f);
      setKey(st, "wt.phase_mode", 1.0f);
      opsOn(st);
    });
    for (int m = 1; m <= 2; ++m)
      out.push_back([m](PluginState& st, std::vector<MidiEvent>&) { setKey(st, "wt.phase_mode", static_cast<float>(m)); });
  } else if (startsWith(key, "op") || startsWith(key, "fm.") || startsWith(key, "cross.")) {
    out.push_back([](PluginState& st, std::vector<MidiEvent>&) { opsOn(st); });
    for (int w : {1, 3, 8})
      out.push_back([w](PluginState& st, std::vector<MidiEvent>&) {
        opsOn(st);
        for (int k = 1; k <= 4; ++k) {
          setKey(st, "op" + std::to_string(k) + ".wave", static_cast<float>(w));
          setKey(st, "op" + std::to_string(k) + ".fixed", 1.0f);
          setKey(st, "op" + std::to_string(k) + ".char", 0.6f);
          setKey(st, "op" + std::to_string(k) + ".sync", 1.0f);
        }
        setKey(st, "fm.char_master", 0.8f);
        for (int k = 1; k <= 4; ++k) setKey(st, "op" + std::to_string(k) + ".wt_x", 0.6f);  // colonne X=0 : sinus sur toutes les lignes
      });
    out.push_back([](PluginState& st, std::vector<MidiEvent>&) {  // fréquence fixe
      opsOn(st);
      for (int k = 1; k <= 4; ++k) setKey(st, "op" + std::to_string(k) + ".tune", 1.0f);
    });
    // route dont le paramètre est la profondeur : ajoutée avec sources/destinations actives
    for (int s = 0; s < kMaxRoutes; ++s) {
      const auto& info = routeSlotInfo(s);
      if (idx(info.depthParam) != p) continue;
      out.push_back([info](PluginState& st, std::vector<MidiEvent>&) {
        opsOn(st);
        addRoute(st, info.src, info.dst);
        setKey(st, "wt.za.type", 1.0f);
        setKey(st, "wt.za.depth", 0.5f);
        setKey(st, "wt.y", 0.5f);
        setKey(st, "wt.x", 0.6f);
        for (int k = 1; k <= 4; ++k) setKey(st, "cross.bus.w" + std::to_string(k), 1.0f);
        setKey(st, "cross.master", 1.0f);
      });
    }
    // contrôles Cross globaux : routes croisées présentes
    out.push_back([](PluginState& st, std::vector<MidiEvent>&) {
      opsOn(st);
      for (int k = 1; k <= 4; ++k) setKey(st, "cross.bus.w" + std::to_string(k), 1.0f);
      addRoute(st, kNodeOP1, kPortWtX);
      addRoute(st, kNodeOP1, kPortWtFreq);
      addRoute(st, kNodeWT, kNodeOP2);
      addRoute(st, kNodeBUS, kPortWtY);
      addRoute(st, kNodeBUS, kNodeWT);
      addRoute(st, kNodeOP1, kNodeOP3);
      addRoute(st, kNodeOP2, kNodeOP4);
      addRoute(st, kNodeOP3, kPortWtY);
      addRoute(st, kNodeOP3, kPortWtZ);
      setKey(st, "wt.za.type", 1.0f);
      setKey(st, "wt.za.depth", 0.5f);
      setKey(st, "wt.y", 0.5f);
      for (int s = 0; s < kMaxRoutes; ++s) {
        const auto& info = routeSlotInfo(s);
        if ((info.src == kNodeOP1 && (info.dst == kPortWtX || info.dst == kPortWtFreq || info.dst == kNodeOP3)) ||
            (info.src == kNodeWT && info.dst == kNodeOP2) || (info.src == kNodeOP2 && info.dst == kNodeOP4) ||
            (info.src == kNodeOP3 && (info.dst == kPortWtY || info.dst == kPortWtZ)) || info.src == kNodeBUS)
          st.params[info.depthParam] = sanitize(info.depthParam, 0.5f);
      }
    });
    for (int m = 1; m <= 3; ++m)
      out.push_back([m](PluginState& st, std::vector<MidiEvent>&) {
        opsOn(st);
        setKey(st, "cross.audio.mode", static_cast<float>(m));
        setKey(st, "cross.audio.amount", 0.7f);
      });
  } else if (startsWith(key, "tune.pc")) {
    const int pc = std::stoi(key.substr(7));
    out.push_back([pc](PluginState& st, std::vector<MidiEvent>& ev) {
      setKey(st, "tune.scale", 1.0f);
      ev = baseEvents(60 + pc, 48 + pc);
    });
  } else if (key == "tune.scale") {
    out.push_back([](PluginState& st, std::vector<MidiEvent>&) {
      for (int k = 0; k < 12; ++k) setKey(st, "tune.pc" + std::to_string(k), 30.0f);
    });
  } else if (startsWith(key, "lfo") || startsWith(key, "rnd.") || startsWith(key, "chaos.") || startsWith(key, "fol") ||
             startsWith(key, "macro") || startsWith(key, "env.")) {
    ModSource s = ModSource::None;
    const char k3 = key.size() > 3 ? key[3] : '1';
    if (startsWith(key, "lfo")) s = static_cast<ModSource>(static_cast<int>(ModSource::Lfo1) + (k3 - '1'));
    else if (startsWith(key, "rnd.hold")) s = ModSource::RandomHold;
    else if (startsWith(key, "rnd.smooth")) s = ModSource::RandomSmooth;
    else if (startsWith(key, "chaos")) s = ModSource::Chaos;
    else if (startsWith(key, "fol")) s = static_cast<ModSource>(static_cast<int>(ModSource::Follower1) + (k3 - '1'));
    else if (startsWith(key, "macro")) s = static_cast<ModSource>(static_cast<int>(ModSource::Macro1) + (key[5] - '1'));
    else if (startsWith(key, "env.wt")) s = ModSource::EnvWt;
    else if (startsWith(key, "env.fm")) s = ModSource::EnvFm;
    else if (startsWith(key, "env.aux1")) s = ModSource::EnvAux1;
    else if (startsWith(key, "env.aux2")) s = ModSource::EnvAux2;
    const std::string lfo = key.substr(0, 4);
    for (int v = 0; v < 2; ++v)
      out.push_back([s, lfo, v](PluginState& st, std::vector<MidiEvent>&) {
        addMod(st, s, ParamId::WtX, 1.0f);
        opsOn(st);
        addMod(st, s, ParamId::Op1Level, 1.0f);
        setKey(st, lfo + ".retrig", static_cast<float>(v));
        setKey(st, lfo + ".sync", static_cast<float>(v));
        setKey(st, "voice.mode", static_cast<float>(v));
      });
    // destination en aval du point mesuré (pas de cycle suiveur) et enveloppes courtes (boucles visibles)
    out.push_back([s, key](PluginState& st, std::vector<MidiEvent>&) {
      opsOn(st);
      setKey(st, "flt.gl.type", 1.0f);
      setKey(st, "flt.gl.cutoff", 300.0f);
      addMod(st, s, ParamId::FltGlCutoff, 1.0f);
      addMod(st, s, ParamId::OutGain, 1.0f);  // suiveur 3 (sortie globale) : seule destination en aval
      const std::string env = key.substr(0, key.rfind('.'));
      setKey(st, env + ".attack", 0.005f);
      setKey(st, env + ".decay", 0.01f);
      setKey(st, env + ".sustain", 0.2f);
      setKey(st, env + ".release", 0.01f);
    });
  } else if (startsWith(key, "out.")) {
    out.push_back([](PluginState& st, std::vector<MidiEvent>&) {
      setKey(st, "out.gain", 6.0f);
      setKey(st, "out.ceiling", -24.0f);
      setKey(st, "out.limiter", 1.0f);
    });
  } else if (startsWith(key, "voice.") || startsWith(key, "amp.")) {
    out.push_back([](PluginState& st, std::vector<MidiEvent>&) { setKey(st, "voice.mode", 1.0f); });
  }
  return out;
}

// valeurs alternatives à la valeur courante du contexte (et non au défaut du registre)
std::vector<float> candidates(int p, float current) {
  const auto& sp = paramSpec(p);
  std::vector<float> v;
  if (sp.type == ParamType::Choice || sp.type == ParamType::Int) {
    for (int c = static_cast<int>(sp.minValue); c <= static_cast<int>(sp.maxValue) && v.size() < 12; ++c)
      if (static_cast<float>(c) != current) v.push_back(static_cast<float>(c));
  } else if (sp.type == ParamType::Bool) {
    v.push_back(current >= 0.5f ? 0.0f : 1.0f);
  } else {
    for (float x : {sp.minValue, sp.maxValue, sp.minValue + 0.5f * (sp.maxValue - sp.minValue), sp.defaultValue})
      if (x != current) v.push_back(x);
  }
  return v;
}

// Exceptions justifiées : effet démontré par un critère dédié, hors de la fenêtre de 0,3 s de l'audit.
const char* exception(const std::string& key) {
  if (startsWith(key, "flt.") && endsWith(key, ".freeze"))
    return "Freeze filtre : etat autonome arme, effet apres relachement au-dela du garde-fou (F-05/F-06)";
  if (endsWith(key, ".arm")) return "armement : persistance au-dela du garde-fou de queue (DST-05, DST-06, DST-08, F-xx auto-oscillation)";
  return nullptr;
}

}  // namespace

void runAuditTests() {
  std::printf("[arch-g4.1 audit]\n");
  int effective = 0, excepted = 0, renders = 0;
  std::vector<std::string> silent;
  for (int p = 0; p < kNumParams; ++p) {
    bool eff = false;
    // D2 : segments de 10 ms à décisions pseudo-aléatoires ; le feedback n'apparaît qu'au réemploi d'une zone
    // enregistrée en boucle : fenêtre de 1 s (mesuré : identique à 0,2 s, différent à 1 s sur fm1/fm2/gl2)
    const bool longWindow = std::string(paramSpec(p).stableId).find(".d2.") != std::string::npos;
    for (const auto& ctx : contextsFor(p)) {
      G4Scenario base;
      base.samples = longWindow ? 48000 : 9600;
      base.block = 64;
      base.transport.valid = true;
      base.transport.bpm = 100.0;
      base.state.bank = "factory:harmonic";  // banque 16 x 4 : axes X et Y actifs
      base.state.params[ParamId::Op1Level] = 0.5f;
      base.events = baseEvents();
      ctx(base.state, base.events);
      const auto ref = renderG4(base);
      ++renders;
      for (float v : candidates(p, base.state.params.v[static_cast<size_t>(p)])) {
        G4Scenario s = base;
        s.state.params.v[static_cast<size_t>(p)] = sanitize(static_cast<ParamId>(p), v);
        const auto r = renderG4(s);
        ++renders;
        if (!th::sameFloats(r.L, ref.L) || !th::sameFloats(r.R, ref.R)) {
          eff = true;
          break;
        }
      }
      if (eff) break;
    }
    const std::string key = paramSpec(p).stableId;
    if (eff) {
      ++effective;
    } else if (exception(key)) {
      ++excepted;
    } else {
      silent.push_back(key);
      tk::info("AUD-01 sans effet mesure : %s", key.c_str());
    }
  }
  const bool ok = silent.empty();
  TK_CHECK(ok);
  tk::result("AUD-01", ok, "%d parametres : %d avec effet audio mesure (contexte d'activation), %d exceptions justifiees (armements, criteres dedies), %zu sans effet ; %d rendus",
             kNumParams, effective, excepted, silent.size(), renders);
}
