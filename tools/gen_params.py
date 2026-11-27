#!/usr/bin/env python3
"""SUBDESTRUCKT — générateur de la table canonique des paramètres (Parameter Registry, P0-C23..C26).

Produit source/core/ParamTable.inc (X-macro) et docs/generated/PARAMETERS.md. La table générée est commitée ; elle est
la source de vérité compilée. Règles (append-only) :
  - les 22 paramètres v1 (0.1.0-b5) restent en tête, inchangés (StableID, plage, défaut) ;
  - un StableID n'est jamais supprimé, renommé, réordonné ni réutilisé : on ajoute en fin de liste ;
  - tests/core/expected_param_ids.txt fige la liste ; toute divergence fait échouer REG-ID.
Exécution : python tools/gen_params.py  (bibliothèque standard uniquement)
"""
import math
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
P = []  # dicts


def add(enum, sid, name, typ, unit, mn, mx, df, *, smooth=20.0, auto=True, ver=2, dom="Lin", rates="C", rnd=True,
        scale=None, hard=None, choices="", group="", stage=0):
    assert all(p["sid"] != sid for p in P), sid
    assert all(p["enum"] != enum for p in P), enum
    if scale is None:
        if dom in ("Oct", "Time"):
            scale = math.log2(mx / mn) if mn > 0 else (mx - mn)
        elif dom == "Cyc":
            scale = 1.0
        elif dom == "Disc":
            scale = 0.0
        else:
            scale = mx - mn
    hmn, hmx = hard if hard else (mn, mx)
    P.append(dict(enum=enum, sid=sid, name=name, typ=typ, unit=unit, mn=mn, mx=mx, df=df, smooth=smooth, auto=auto,
                  ver=ver, dom=dom, rates=rates, rnd=rnd, scale=scale, hmn=hmn, hmx=hmx, choices=choices, group=group,
                  stage=stage))


def F(enum, sid, name, unit, mn, mx, df, **k):
    add(enum, sid, name, "Float", unit, mn, mx, df, **k)


def C(enum, sid, name, choices, df=0, auto=False, **k):
    n = len(choices)
    add(enum, sid, name, "Choice", "", 0, n - 1, df, smooth=0.0, auto=auto, dom="Disc", rates="E",
        choices="|".join(choices), **k)


def B(enum, sid, name, df=0, auto=False, **k):
    add(enum, sid, name, "Bool", "", 0, 1, df, smooth=0.0, auto=auto, dom="Disc", rates="E", **k)


def I(enum, sid, name, unit, mn, mx, df, auto=False, **k):
    add(enum, sid, name, "Int", unit, mn, mx, df, smooth=0.0, auto=auto, dom="Disc", rates="E", **k)


# ------------------------------------------------------------------ v1 (0.1.0-b5) — NE JAMAIS MODIFIER
V1 = dict(ver=1)
F("AmpAttack", "amp.attack", "Amp Attack", "s", 0.001, 5.0, 0.005, smooth=0.0, dom="Time", group="env.amp", **V1)
F("AmpDecay", "amp.decay", "Amp Decay", "s", 0.001, 5.0, 0.25, smooth=0.0, dom="Time", group="env.amp", **V1)
F("AmpSustain", "amp.sustain", "Amp Sustain", "lin", 0.0, 1.0, 0.8, smooth=0.0, group="env.amp", **V1)
F("AmpRelease", "amp.release", "Amp Release", "s", 0.001, 10.0, 0.3, smooth=0.0, dom="Time", group="env.amp", **V1)
F("AmpVelocity", "amp.velocity", "Amp Velocity", "lin", 0.0, 1.0, 1.0, group="env.amp", **V1)
F("OutGain", "out.gain", "Output Gain", "dB", -48.0, 6.0, -6.0, dom="Db", group="out", stage=5, **V1)
F("WtLevel", "wt.level", "WT Level", "lin", 0.0, 1.0, 0.5, rates="CA", group="wt", **V1)
F("Op1Level", "op1.level", "OP1 Level", "lin", 0.0, 1.0, 0.5, rates="CA", group="op1", **V1)
F("Op2Level", "op2.level", "OP2 Level", "lin", 0.0, 1.0, 0.0, rates="CA", group="op2", **V1)
F("Op1Ratio", "op1.ratio", "OP1 Ratio", "x f0", 0.25, 16.0, 1.0, dom="Oct", rates="CA", group="op1", **V1)
F("Op2Ratio", "op2.ratio", "OP2 Ratio", "x f0", 0.25, 16.0, 2.0, dom="Oct", rates="CA", group="op2", **V1)
for s, d, e in ((2, 1, "DepthOp2Op1"), (1, 2, "DepthOp1Op2"), (1, 1, "DepthOp1Op1"), (2, 2, "DepthOp2Op2")):
    nm = f"FM OP{s}>OP{d}" if s != d else f"FM OP{s} Self"
    F(e, f"fm.op{s}_op{d}.depth", nm, "rad", 0.0, 8.0, 1.0 if (s, d) == (2, 1) else 0.0, dom="Rad", rates="CA",
      group="fm.matrix", **V1)
F("DepthWtOp1", "cross.wt_op1.depth", "CROSS WT>OP1", "rad", 0.0, 8.0, 0.5, dom="Rad", rates="CA", group="cross", **V1)
F("DepthWtOp2", "cross.wt_op2.depth", "CROSS WT>OP2", "rad", 0.0, 8.0, 0.0, dom="Rad", rates="CA", group="cross", **V1)
F("DepthOp1Wt", "cross.op1_wt.depth", "CROSS OP1>WT", "rad", 0.0, 8.0, 0.5, dom="Rad", rates="CA", group="cross", **V1)
F("DepthOp2Wt", "cross.op2_wt.depth", "CROSS OP2>WT", "rad", 0.0, 8.0, 0.0, dom="Rad", rates="CA", group="cross", **V1)
F("CrossMaster", "cross.master", "CROSS Master", "lin", 0.0, 1.0, 1.0, rates="CA", group="cross", **V1)
F("CrossFwd", "cross.fwd", "CROSS WT>FM", "lin", 0.0, 1.0, 1.0, rates="CA", group="cross", **V1)
F("CrossRev", "cross.rev", "CROSS FM>WT", "lin", 0.0, 1.0, 1.0, rates="CA", group="cross", **V1)
assert len(P) == 22

# ------------------------------------------------------------------ v2 (ARCH-G4) — ajout en fin de liste uniquement
# Voix / MIDI (P0-C07..C12)
C("VoiceMode", "voice.mode", "Voice Mode", ["Mono", "Legato"], group="voice")
C("VoicePriority", "voice.priority", "Note Priority", ["Last", "Low", "High"], group="voice")
F("BendUp", "voice.bend_up", "Bend Up", "st", 0.0, 48.0, 2.0, smooth=0.0, rates="E", rnd=False, group="voice")
F("BendDown", "voice.bend_down", "Bend Down", "st", 0.0, 48.0, 2.0, smooth=0.0, rates="E", rnd=False, group="voice")
F("TuneA4", "tune.a4", "Reference A4", "Hz", 400.0, 480.0, 440.0, rnd=False, group="tune")
C("TuneScale", "tune.scale", "Scale", ["Equal", "Custom"], rnd=False, group="tune")
for i, pc in enumerate(["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]):
    F(f"TunePc{i}", f"tune.pc{i}", f"Tune {pc}", "ct", -100.0, 100.0, 0.0, smooth=0.0, rates="E", rnd=False, group="tune")
# Amp (extension)
F("AmpDelay", "amp.delay", "Amp Delay", "s", 0.0, 5.0, 0.0, smooth=0.0, group="env.amp")
F("AmpHold", "amp.hold", "Amp Hold", "s", 0.0, 5.0, 0.0, smooth=0.0, group="env.amp")
C("AmpCurve", "amp.curve", "Amp Curve", ["Linear", "Exponential"], group="env.amp")
# Quatre enveloppes de modulation (WT, FM, AUX1, AUX2)
for key, label in (("wt", "WT"), ("fm", "FM"), ("aux1", "AUX1"), ("aux2", "AUX2")):
    E = key.capitalize()
    g = f"env.{key}"
    F(f"Env{E}Delay", f"env.{key}.delay", f"ENV {label} Delay", "s", 0.0, 10.0, 0.0, smooth=0.0, group=g)
    F(f"Env{E}Attack", f"env.{key}.attack", f"ENV {label} Attack", "s", 0.001, 20.0, 0.01, smooth=0.0, dom="Time", group=g)
    F(f"Env{E}Hold", f"env.{key}.hold", f"ENV {label} Hold", "s", 0.0, 10.0, 0.0, smooth=0.0, group=g)
    F(f"Env{E}Decay", f"env.{key}.decay", f"ENV {label} Decay", "s", 0.001, 20.0, 0.3, smooth=0.0, dom="Time", group=g)
    F(f"Env{E}Sustain", f"env.{key}.sustain", f"ENV {label} Sustain", "lin", 0.0, 1.0, 0.5, smooth=0.0, group=g)
    F(f"Env{E}Release", f"env.{key}.release", f"ENV {label} Release", "s", 0.001, 20.0, 0.3, smooth=0.0, dom="Time",
      group=g)
    C(f"Env{E}Curve", f"env.{key}.curve", f"ENV {label} Curve", ["Linear", "Exponential"], group=g)
    C(f"Env{E}Loop", f"env.{key}.loop", f"ENV {label} Loop", ["Off", "Loop"], group=g)
# Quatre LFO
DIVS = ["8/1", "4/1", "2/1", "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/4T", "1/8T", "1/16T", "1/4D", "1/8D", "1/16D"]
for n in range(1, 5):
    g = f"lfo{n}"
    C(f"Lfo{n}Shape", f"lfo{n}.shape", f"LFO{n} Shape",
      ["Sine", "Triangle", "Saw Up", "Saw Down", "Square", "Random Hold", "Random Smooth", "Custom"], auto=True, group=g)
    F(f"Lfo{n}Rate", f"lfo{n}.rate", f"LFO{n} Rate", "Hz", 0.01, 1000.0, 1.0, dom="Oct", group=g)
    B(f"Lfo{n}Sync", f"lfo{n}.sync", f"LFO{n} Tempo Sync", group=g)
    C(f"Lfo{n}Div", f"lfo{n}.div", f"LFO{n} Division", DIVS, df=5, auto=True, group=g)
    F(f"Lfo{n}Phase", f"lfo{n}.phase", f"LFO{n} Phase", "cyc", 0.0, 1.0, 0.0, smooth=0.0, dom="Cyc", group=g)
    C(f"Lfo{n}Retrig", f"lfo{n}.retrig", f"LFO{n} Retrigger", ["Free", "Note"], group=g)
# Sources aléatoires / chaos (rates des générateurs uniques, pas de générateurs supplémentaires)
F("RndHoldRate", "rnd.hold.rate", "Random Hold Rate", "Hz", 0.01, 100.0, 2.0, dom="Oct", group="random")
F("RndSmoothRate", "rnd.smooth.rate", "Random Smooth Rate", "Hz", 0.01, 100.0, 1.0, dom="Oct", group="random")
F("ChaosRate", "chaos.rate", "Chaos Rate", "Hz", 0.01, 100.0, 0.5, dom="Oct", group="random")
for n in range(1, 4):
    F(f"Fol{n}Attack", f"fol{n}.attack", f"Follower{n} Attack", "s", 0.0005, 1.0, 0.01, smooth=0.0, dom="Time",
      group="follower")
    F(f"Fol{n}Release", f"fol{n}.release", f"Follower{n} Release", "s", 0.005, 5.0, 0.2, smooth=0.0, dom="Time",
      group="follower")
# Wavetable XYZ
W = dict(group="wt", stage=0)
F("WtX", "wt.x", "WT X", "pos", 0.0, 1.0, 0.0, rates="CA", **W)
F("WtY", "wt.y", "WT Y", "pos", 0.0, 1.0, 0.0, rates="CA", **W)
F("WtZ", "wt.z", "WT Z Master", "lin", 0.0, 1.0, 0.0, rates="CA", **W)
C("WtXBound", "wt.x_bound", "WT X Boundary", ["Clamp", "Wrap", "Mirror"], **W)
C("WtYBound", "wt.y_bound", "WT Y Boundary", ["Clamp", "Wrap", "Mirror"], **W)
C("WtXMorph", "wt.x_morph", "WT X Morph", ["Linear", "Stepped", "Spectral"], **W)
C("WtYMorph", "wt.y_morph", "WT Y Morph", ["Linear", "Stepped", "Spectral"], **W)
C("WtInterp", "wt.interp", "WT Interpolation", ["Linear", "Cubic"], **W)
C("WtRender", "wt.render", "WT Clean/Raw", ["Clean", "Raw"], auto=True, **W)
F("WtCoarse", "wt.coarse", "WT Coarse", "st", -48.0, 48.0, 0.0, dom="Semi", rates="CA", **W)
F("WtFine", "wt.fine", "WT Fine", "ct", -100.0, 100.0, 0.0, rates="CA", **W)
C("WtPhaseMode", "wt.phase_mode", "WT Phase Mode", ["Free", "Retrigger", "Fixed", "Random"], **W)
F("WtPhase", "wt.phase", "WT Start Phase", "cyc", 0.0, 1.0, 0.0, smooth=0.0, dom="Cyc", rates="E", **W)
C("WtSync", "wt.sync", "WT Sync", ["Off", "Hard", "Soft"], **W)
C("WtSyncSrc", "wt.sync_src", "WT Sync Source", ["Internal", "OP1", "OP2", "OP3", "OP4"], **W)
F("WtChar", "wt.char", "WT Character", "lin", 0.0, 1.0, 0.0, rates="C", **W)
C("WtCharMode", "wt.char_mode", "WT Character Profile", ["Phase Quantize", "Bit Reduce", "Sample Hold"], **W)
# Énumération append-only : indices 0-3 inchangés depuis la v2 ; ARCH-G4.3 ajoute en fin les cinq transformations du
# catalogue validé par le propriétaire (Z1 Harmonic Tilt, Z4 Mask, Z5 Focus, Z6 Shift, Z7 Phase Scatter).
ZT = ["Off", "Harmonic Fold", "Phase Warp", "Spectral Mutation", "Harmonic Tilt", "Harmonic Mask", "Harmonic Focus",
      "Harmonic Shift", "Phase Scatter"]
ZC = ["Linear", "Exponential", "Logarithmic", "S-Curve"]
for s in ("a", "b"):
    S = s.upper()
    C(f"WtZ{S}Type", f"wt.z{s}.type", f"WT Z{S} Transform", ZT, **W)
    F(f"WtZ{S}Depth", f"wt.z{s}.depth", f"WT Z{S} Depth", "lin", 0.0, 1.0, 1.0, rates="CA", **W)
    C(f"WtZ{S}Curve", f"wt.z{s}.curve", f"WT Z{S} Curve", ZC, **W)
C("WtZOrder", "wt.z_order", "WT Z Order", ["A>B", "B>A"], **W)
B("WtZFund", "wt.z_fund", "WT Z Keep Fundamental", df=1, **W)
B("WtZGain", "wt.z_gain", "WT Z Gain Compensation", df=1, **W)
# FM / PM — quatre opérateurs
WAVES = ["Sine", "Triangle", "Saw", "Pulse", "Folded Sine", "Asymmetric", "Phase Distorted", "Harmonic", "Wavetable"]
for n in range(1, 5):
    g = dict(group=f"op{n}", stage=0)
    if n >= 3:
        F(f"Op{n}Level", f"op{n}.level", f"OP{n} Level", "lin", 0.0, 1.0, 0.0, rates="CA", **g)
        F(f"Op{n}Ratio", f"op{n}.ratio", f"OP{n} Ratio", "x f0", 0.25, 16.0, float(n), dom="Oct", rates="CA", **g)
    C(f"Op{n}Tune", f"op{n}.tune", f"OP{n} Tuning", ["Ratio", "Fixed"], **g)
    F(f"Op{n}Fixed", f"op{n}.fixed", f"OP{n} Fixed", "Hz", 0.01, 20000.0, 440.0, dom="Oct", rates="CA", **g)
    F(f"Op{n}Offset", f"op{n}.offset", f"OP{n} Offset", "Hz", -2000.0, 2000.0, 0.0, rates="CA", **g)
    F(f"Op{n}Coarse", f"op{n}.coarse", f"OP{n} Coarse", "st", -48.0, 48.0, 0.0, dom="Semi", rates="CA", **g)
    F(f"Op{n}Fine", f"op{n}.fine", f"OP{n} Fine", "ct", -100.0, 100.0, 0.0, rates="CA", **g)
    C(f"Op{n}Wave", f"op{n}.wave", f"OP{n} Waveform", WAVES, **g)
    F(f"Op{n}WtX", f"op{n}.wt_x", f"OP{n} WT X", "pos", 0.0, 1.0, 0.0, rates="C", **g)
    F(f"Op{n}WtY", f"op{n}.wt_y", f"OP{n} WT Y", "pos", 0.0, 1.0, 0.0, rates="C", **g)
    F(f"Op{n}Pan", f"op{n}.pan", f"OP{n} Pan", "bip", -1.0, 1.0, 0.0, rates="CA", **g)
    B(f"Op{n}Mute", f"op{n}.mute", f"OP{n} Mute", auto=True, rnd=False, **g)
    B(f"Op{n}Enable", f"op{n}.enable", f"OP{n} Enable", df=1, rnd=False, **g)
    F(f"Op{n}Char", f"op{n}.char", f"OP{n} Character", "lin", 0.0, 1.0, 0.0, rates="C", **g)
    C(f"Op{n}PhaseMode", f"op{n}.phase_mode", f"OP{n} Phase Mode", ["Free", "Retrigger", "Random"], **g)
    C(f"Op{n}Ac", f"op{n}.ac", f"OP{n} Mod Coupling", ["DC", "AC"], **g)
    C(f"Op{n}Mode", f"op{n}.mode", f"OP{n} Mod Mode", ["PM", "Linear FM", "Through-Zero FM", "Exponential FM"], **g)
    C(f"Op{n}Sync", f"op{n}.sync", f"OP{n} Sync", ["Off", "WT"], **g)
F("FmCharMaster", "fm.char_master", "FM Character Master", "lin", 0.0, 1.0, 1.0, rates="C", group="fm")
C("FmFbMode", "fm.fb_mode", "FM Feedback Reset", ["Continuous", "Note Reset"], group="fm")
for s in range(1, 5):
    for d in range(1, 5):
        if (s, d) in ((2, 1), (1, 2), (1, 1), (2, 2)):
            continue
        nm = f"FM OP{s}>OP{d}" if s != d else f"FM OP{s} Self"
        F(f"DepthOp{s}Op{d}", f"fm.op{s}_op{d}.depth", nm, "rad", 0.0, 8.0, 0.0, dom="Rad", rates="CA",
          group="fm.matrix")
# CROSS (4 WT>FM + 35 FM>WT)
X = dict(group="cross", stage=0)
for d in (3, 4):
    F(f"DepthWtOp{d}", f"cross.wt_op{d}.depth", f"CROSS WT>OP{d}", "rad", 0.0, 8.0, 0.0, dom="Rad", rates="CA", **X)
for s in ("op3", "op4", "bus"):
    F(f"Depth{s.capitalize()}Wt", f"cross.{s}_wt.depth", f"CROSS {s.upper()}>WT", "rad", 0.0, 8.0, 0.0, dom="Rad",
      rates="CA", **X)
CROSS_DST = (("freq", "Freq", "x", 0.0, 8.0), ("x", "X", "pos", -1.0, 1.0), ("y", "Y", "pos", -1.0, 1.0),
             ("z", "Z", "lin", -1.0, 1.0), ("level", "Level", "lin", -1.0, 1.0), ("char", "Char", "lin", -1.0, 1.0))
for s in ("op1", "op2", "op3", "op4", "bus"):
    for dk, dl, du, dmn, dmx in CROSS_DST:
        F(f"Depth{s.capitalize()}Wt{dl}", f"cross.{s}_wt_{dk}.depth", f"CROSS {s.upper()}>WT {dl}", du, dmn, dmx, 0.0,
          rates="CA", **X)
C("CrossWtFreqMode", "cross.wt_fmode", "CROSS WT Freq Mode", ["Linear FM", "Through-Zero FM", "Exponential FM"], **X)
for n in range(1, 5):
    F(f"CrossBusW{n}", f"cross.bus.w{n}", f"CROSS Bus OP{n}", "bip", -1.0, 1.0, 1.0, rates="CA", **X)
C("CrossBusMode", "cross.bus.mode", "CROSS Bus Mode", ["Raw", "Normalized"], df=1, **X)
C("CrossDir", "cross.dir", "CROSS Direction", ["Both", "Forward", "Reverse"], auto=True, **X)
C("CrossAudioMode", "cross.audio.mode", "Cross Audio Mode", ["Mix", "Crossfade", "Ring", "AM"], stage=3, group="cross")
F("CrossAudioAmount", "cross.audio.amount", "Cross Audio Amount", "lin", 0.0, 1.0, 0.5, rates="CA", stage=3,
  group="cross")
C("CrossAmDir", "cross.audio.am_dir", "Cross AM Direction", ["WT>FM", "FM>WT"], stage=3, group="cross")
# Filter & Resonator — trois emplacements
for key, label, stg in (("wt", "WT", 1), ("fm", "FM", 2), ("gl", "GLOBAL", 4)):
    g = dict(group=f"flt.{key}", stage=stg)
    pre = f"flt.{key}"
    E = f"Flt{key.capitalize()}"
    C(f"{E}Type", f"{pre}.type", f"FLT {label} Type", ["Off", "F1 Multimode", "F2 Comb", "F3 Nonlinear", "F4 Modal"], **g)
    C(f"{E}Pos", f"{pre}.pos", f"FLT {label} Position", ["Pre", "Post"], **g)
    C(f"{E}Stereo", f"{pre}.stereo", f"FLT {label} Stereo", ["Linked", "Dual Mono"], **g)
    C(f"{E}F1Mode", f"{pre}.f1_mode", f"FLT {label} F1 Mode", ["LP12", "LP24", "BP12", "BP24", "HP12", "HP24", "Notch"],
      **g)
    F(f"{E}Cutoff", f"{pre}.cutoff", f"FLT {label} Cutoff", "Hz", 20.0, 20000.0, 1000.0, dom="Oct", rates="CA", **g)
    F(f"{E}Res", f"{pre}.res", f"FLT {label} Resonance", "lin", 0.0, 1.0, 0.2, rates="CA", **g)
    F(f"{E}Key", f"{pre}.key", f"FLT {label} Keytrack", "lin", 0.0, 1.0, 0.0, rates="C", **g)
    F(f"{E}Drive", f"{pre}.drive", f"FLT {label} Drive", "dB", 0.0, 36.0, 0.0, dom="Db", rates="CA", **g)
    F(f"{E}Bias", f"{pre}.bias", f"FLT {label} Bias", "bip", -1.0, 1.0, 0.0, rates="CA", **g)
    F(f"{E}Char", f"{pre}.char", f"FLT {label} Character", "lin", 0.0, 1.0, 0.5, rates="C", **g)
    C(f"{E}F3Mode", f"{pre}.f3_mode", f"FLT {label} F3 Mode", ["LP", "BP", "HP"], **g)
    F(f"{E}CombFb", f"{pre}.comb_fb", f"FLT {label} Comb Feedback", "bip", -1.0, 1.0, 0.7, rates="CA", **g)
    F(f"{E}CombDamp", f"{pre}.comb_damp", f"FLT {label} Comb Damping", "lin", 0.0, 1.0, 0.2, rates="C", **g)
    F(f"{E}Detune", f"{pre}.detune", f"FLT {label} Detune", "ct", 0.0, 50.0, 0.0, rates="C", **g)
    F(f"{E}Spread", f"{pre}.spread", f"FLT {label} Stereo Spread", "lin", 0.0, 1.0, 0.0, rates="C", **g)
    C(f"{E}Modal", f"{pre}.modal", f"FLT {label} Modal Model", ["Harmonic", "Metal", "Membrane", "Inharmonic"], **g)
    I(f"{E}Modes", f"{pre}.modes", f"FLT {label} Modes", "", 1, 16, 8, **g)
    F(f"{E}Decay", f"{pre}.decay", f"FLT {label} Decay", "s", 0.01, 20.0, 1.0, dom="Time", rates="C", **g)
    F(f"{E}Damp", f"{pre}.damp", f"FLT {label} Damping", "lin", 0.0, 1.0, 0.3, rates="C", **g)
    F(f"{E}Bright", f"{pre}.bright", f"FLT {label} Brightness", "lin", 0.0, 1.0, 0.5, rates="C", **g)
    F(f"{E}Inharm", f"{pre}.inharm", f"FLT {label} Inharmonicity", "lin", 0.0, 1.0, 0.0, rates="C", **g)
    B(f"{E}Freeze", f"{pre}.freeze", f"FLT {label} Modal Freeze", rnd=False, **g)
    C(f"{E}Exc", f"{pre}.exc", f"FLT {label} Excitation", ["Off", "Note", "Manual"], **g)
    F(f"{E}ExcLevel", f"{pre}.exc_level", f"FLT {label} Excitation Level", "lin", 0.0, 1.0, 0.5, rates="C", **g)
    F(f"{E}Mix", f"{pre}.mix", f"FLT {label} Dry/Wet", "lin", 0.0, 1.0, 1.0, rates="CA", **g)
    F(f"{E}Out", f"{pre}.out", f"FLT {label} Output", "dB", -24.0, 24.0, 0.0, dom="Db", rates="C", **g)
    B(f"{E}Arm", f"{pre}.arm", f"FLT {label} Autonomous Armed", rnd=False, **g)
# Destruction Engine — six emplacements (deux par chaîne)
DT = ["Off", "N1 Asymmetric Wavefolder", "D1 Variable Rate Crusher", "Y1 Transient Destroyer",
      "S1 Ring/Frequency Shifter", "N2 Feedback Waveshaper", "D2 Glitch Buffer", "Y2 Envelope Shaper",
      "S2 Spectral Freeze"]
for key, label, stg in (("wt1", "WT1", 1), ("wt2", "WT2", 1), ("fm1", "FM1", 2), ("fm2", "FM2", 2), ("gl1", "GL1", 4),
                        ("gl2", "GL2", 4)):
    g = dict(group=f"dst.{key}", stage=stg)
    pre = f"dst.{key}"
    E = f"Dst{key.capitalize()}"
    C(f"{E}Type", f"{pre}.type", f"DST {label} Type", DT, **g)
    F(f"{E}Mix", f"{pre}.mix", f"DST {label} Dry/Wet", "lin", 0.0, 1.0, 1.0, rates="CA", **g)
    F(f"{E}Out", f"{pre}.out", f"DST {label} Output", "dB", -24.0, 24.0, 0.0, dom="Db", rates="C", **g)
    B(f"{E}Arm", f"{pre}.arm", f"DST {label} Autonomous Armed", rnd=False, **g)
    # N1
    F(f"{E}N1Drive", f"{pre}.n1.drive", f"DST {label} N1 Drive", "dB", -24.0, 36.0, 0.0, dom="Db", rates="CA", **g)
    F(f"{E}N1Fold", f"{pre}.n1.fold", f"DST {label} N1 Fold", "lin", 0.0, 1.0, 0.5, rates="CA", **g)
    F(f"{E}N1Bias", f"{pre}.n1.bias", f"DST {label} N1 Bias", "bip", -1.0, 1.0, 0.0, rates="CA", **g)
    F(f"{E}N1Sym", f"{pre}.n1.sym", f"DST {label} N1 Symmetry", "bip", -1.0, 1.0, 0.0, rates="CA", **g)
    I(f"{E}N1Stages", f"{pre}.n1.stages", f"DST {label} N1 Stages", "", 1, 4, 1, **g)
    F(f"{E}N1Tone", f"{pre}.n1.tone", f"DST {label} N1 Tone", "lin", 0.0, 1.0, 0.5, rates="C", **g)
    # D1
    F(f"{E}D1Bits", f"{pre}.d1.bits", f"DST {label} D1 Bits", "bit", 2.0, 16.0, 8.0, rates="C", **g)
    F(f"{E}D1Rate", f"{pre}.d1.rate", f"DST {label} D1 Rate Ratio", "x", 1.0, 128.0, 4.0, dom="Oct", rates="C", **g)
    F(f"{E}D1Jitter", f"{pre}.d1.jitter", f"DST {label} D1 Jitter", "lin", 0.0, 1.0, 0.0, rates="C", **g)
    C(f"{E}D1Mode", f"{pre}.d1.mode", f"DST {label} D1 Mode", ["Raw", "Controlled"], df=1, **g)
    C(f"{E}D1Hold", f"{pre}.d1.hold", f"DST {label} D1 Hold", ["Step", "Smooth"], **g)
    # Y1
    F(f"{E}Y1Thresh", f"{pre}.y1.thresh", f"DST {label} Y1 Threshold", "dB", -60.0, 0.0, -20.0, dom="Db", rates="C", **g)
    F(f"{E}Y1Ratio", f"{pre}.y1.ratio", f"DST {label} Y1 Ratio", ":1", 1.0, 20.0, 4.0, rates="C", **g)
    F(f"{E}Y1Attack", f"{pre}.y1.attack", f"DST {label} Y1 Attack", "ms", 0.1, 100.0, 5.0, dom="Time", rates="C", **g)
    F(f"{E}Y1Release", f"{pre}.y1.release", f"DST {label} Y1 Release", "ms", 5.0, 1000.0, 100.0, dom="Time", rates="C",
      **g)
    F(f"{E}Y1Knee", f"{pre}.y1.knee", f"DST {label} Y1 Knee", "dB", 0.0, 24.0, 6.0, rates="C", **g)
    F(f"{E}Y1Gate", f"{pre}.y1.gate", f"DST {label} Y1 Gate", "dB", -90.0, -20.0, -90.0, dom="Db", rates="C", **g)
    F(f"{E}Y1Transient", f"{pre}.y1.transient", f"DST {label} Y1 Transient", "bip", -1.0, 1.0, 0.0, rates="C", **g)
    F(f"{E}Y1Makeup", f"{pre}.y1.makeup", f"DST {label} Y1 Makeup", "dB", 0.0, 36.0, 0.0, dom="Db", rates="C", **g)
    # S1
    C(f"{E}S1Mode", f"{pre}.s1.mode", f"DST {label} S1 Mode", ["Ring", "Shift"], **g)
    F(f"{E}S1Freq", f"{pre}.s1.freq", f"DST {label} S1 Frequency", "Hz", 0.1, 8000.0, 100.0, dom="Oct", rates="CA", **g)
    F(f"{E}S1Shift", f"{pre}.s1.shift", f"DST {label} S1 Shift", "Hz", -5000.0, 5000.0, 0.0, rates="CA", **g)
    C(f"{E}S1Carrier", f"{pre}.s1.carrier", f"DST {label} S1 Carrier", ["Sine", "Triangle", "Square"], **g)
    F(f"{E}S1Phase", f"{pre}.s1.phase", f"DST {label} S1 Phase", "cyc", 0.0, 1.0, 0.0, dom="Cyc", rates="C", **g)
    F(f"{E}S1Stereo", f"{pre}.s1.stereo", f"DST {label} S1 Stereo Phase", "cyc", 0.0, 1.0, 0.0, dom="Cyc", rates="C",
      **g)
    # N2
    F(f"{E}N2Fb", f"{pre}.n2.fb", f"DST {label} N2 Feedback", "lin", 0.0, 1.0, 0.5, rates="C", **g)
    F(f"{E}N2Delay", f"{pre}.n2.delay", f"DST {label} N2 Delay", "ms", 0.1, 50.0, 5.0, dom="Time", rates="C", **g)
    F(f"{E}N2Damp", f"{pre}.n2.damp", f"DST {label} N2 Damping", "lin", 0.0, 1.0, 0.3, rates="C", **g)
    F(f"{E}N2Bias", f"{pre}.n2.bias", f"DST {label} N2 Bias", "bip", -1.0, 1.0, 0.0, rates="C", **g)
    F(f"{E}N2Res", f"{pre}.n2.res", f"DST {label} N2 Resonance", "lin", 0.0, 1.0, 0.3, rates="C", **g)
    B(f"{E}N2SelfOsc", f"{pre}.n2.selfosc", f"DST {label} N2 Self-Oscillation", rnd=False, **g)
    # D2
    F(f"{E}D2Size", f"{pre}.d2.size", f"DST {label} D2 Size", "ms", 10.0, 2000.0, 250.0, dom="Time", rates="C", **g)
    F(f"{E}D2Speed", f"{pre}.d2.speed", f"DST {label} D2 Playback", "x", -4.0, 4.0, 1.0, rates="C", **g)
    F(f"{E}D2Repeat", f"{pre}.d2.repeat", f"DST {label} D2 Repeat", "lin", 0.0, 1.0, 0.5, rates="C", **g)
    F(f"{E}D2Pos", f"{pre}.d2.pos", f"DST {label} D2 Position", "lin", 0.0, 1.0, 0.0, rates="C", **g)
    F(f"{E}D2Fb", f"{pre}.d2.fb", f"DST {label} D2 Feedback", "lin", 0.0, 1.0, 0.0, rates="C", **g)
    F(f"{E}D2Xfade", f"{pre}.d2.xfade", f"DST {label} D2 Crossfade", "ms", 0.0, 50.0, 5.0, rates="C", **g)
    B(f"{E}D2Freeze", f"{pre}.d2.freeze", f"DST {label} D2 Freeze", auto=True, rnd=False, **g)
    # Y2
    F(f"{E}Y2Attack", f"{pre}.y2.attack", f"DST {label} Y2 Attack", "ms", 0.1, 200.0, 5.0, dom="Time", rates="C", **g)
    F(f"{E}Y2Release", f"{pre}.y2.release", f"DST {label} Y2 Release", "ms", 5.0, 2000.0, 200.0, dom="Time", rates="C",
      **g)
    F(f"{E}Y2Sens", f"{pre}.y2.sens", f"DST {label} Y2 Sensitivity", "lin", 0.0, 1.0, 0.5, rates="C", **g)
    F(f"{E}Y2Curve", f"{pre}.y2.curve", f"DST {label} Y2 Curve", "bip", -1.0, 1.0, 0.0, rates="C", **g)
    F(f"{E}Y2Depth", f"{pre}.y2.depth", f"DST {label} Y2 Depth", "bip", -1.0, 1.0, 0.5, rates="C", **g)
    F(f"{E}Y2Hold", f"{pre}.y2.hold", f"DST {label} Y2 Hold", "ms", 0.0, 500.0, 0.0, rates="C", **g)
    C(f"{E}Y2Detect", f"{pre}.y2.detect", f"DST {label} Y2 Detector", ["Peak", "RMS"], **g)
    # S2
    C(f"{E}S2Fft", f"{pre}.s2.fft", f"DST {label} S2 FFT Size", ["512", "1024", "2048", "4096"], df=2, **g)
    B(f"{E}S2Freeze", f"{pre}.s2.freeze", f"DST {label} S2 Freeze", auto=True, rnd=False, **g)
    F(f"{E}S2Decay", f"{pre}.s2.decay", f"DST {label} S2 Decay", "lin", 0.0, 1.0, 0.5, rates="C", **g)
    F(f"{E}S2Blur", f"{pre}.s2.blur", f"DST {label} S2 Blur", "lin", 0.0, 1.0, 0.0, rates="C", **g)
    F(f"{E}S2Tilt", f"{pre}.s2.tilt", f"DST {label} S2 Tilt", "dB/oct", -6.0, 6.0, 0.0, rates="C", **g)
    F(f"{E}S2Drift", f"{pre}.s2.drift", f"DST {label} S2 Phase Drift", "lin", 0.0, 1.0, 0.0, rates="C", **g)
    F(f"{E}S2Shift", f"{pre}.s2.shift", f"DST {label} S2 Shift", "st", -24.0, 24.0, 0.0, rates="C", **g)
# Macros et sortie
for n in range(1, 5):
    F(f"Macro{n}", f"macro{n}", f"Macro {n}", "lin", 0.0, 1.0, 0.0, rates="CA", group="macro")
B("OutLimiter", "out.limiter", "Musical Limiter", rnd=False, group="out", stage=5)
F("OutCeiling", "out.ceiling", "Limiter Ceiling", "dB", -24.0, 0.0, -0.3, dom="Db", rates="C", rnd=False, group="out",
  stage=5)


# ------------------------------------------------------------------ v3 (ARCH-G4.1) — ajout en fin de liste uniquement
V3 = dict(ver=3)
F("PerfBalance", "perf.balance", "WT/FM Balance", "bip", -1.0, 1.0, 0.0, rates="CA", rnd=False, group="perf", stage=1, **V3)
for key, label in (("amp", "Amp"), ("env.wt", "ENV WT"), ("env.fm", "ENV FM"), ("env.aux1", "ENV AUX1"), ("env.aux2", "ENV AUX2")):
    E = "".join(w.capitalize() for w in key.replace(".", " ").split())
    g = "env.amp" if key == "amp" else key
    C(f"{E}Trigger", f"{key}.trigger", f"{label} Trigger", ["Voice", "Note", "Gate"], group=g, **V3)
    B(f"{E}Sync", f"{key}.sync", f"{label} Tempo Sync", group=g, **V3)

# ------------------------------------------------------------------ v4 (ARCH-G4.3) — ajout en fin de liste uniquement
V4 = dict(ver=4)
# D2 Glitch Buffer : mode de Feedback par emplacement (Legacy = rendu historique ; Regenerative = régénération dans la
# boucle). Défaut des nouvelles instances : Regenerative ; tout état antérieur au format 4 est migré en Legacy.
for key, label, stg in (("wt1", "WT1", 1), ("wt2", "WT2", 1), ("fm1", "FM1", 2), ("fm2", "FM2", 2), ("gl1", "GL1", 4),
                        ("gl2", "GL2", 4)):
    C(f"Dst{key.capitalize()}D2FbMode", f"dst.{key}.d2.fb_mode", f"DST {label} D2 Feedback Mode", ["Legacy", "Regenerative"],
      df=1, group=f"dst.{key}", stage=stg, **V4)
# Z : second axe des transformations spectrales (Tilt, Mask, Focus, Shift, Scatter) ; ignoré par Fold, Warp, Mutation
for s_ in ("a", "b"):
    F(f"WtZ{s_.upper()}Shape", f"wt.z{s_}.shape", f"WT Z{s_.upper()} Shape", "bip", -1.0, 1.0, 0.0, rates="C", group="wt", stage=0, **V4)


# ------------------------------------------------------------------ sorties
def cstr(s):
    return '"' + s.replace("\\", "\\\\").replace('"', '\\"') + '"'


def num(x):
    r = repr(float(x))
    return r + "f" if ("e" in r or "." in r) else r + ".0f"


RATES = {"E": "kRateEvent", "C": "kRateControl", "A": "kRateAudio"}
lines = ["// GÉNÉRÉ par tools/gen_params.py — ne pas éditer à la main. Append-only : voir l'en-tête du générateur.",
         "// SBD_PARAM(enum, stableId, name, type, unit, min, max, def, smoothMs, automatable, version, domain, rates,",
         "//           randomizable, modScale, hardMin, hardMax, choices, group, stage)"]
for p in P:
    rates = "|".join(RATES[c] for c in p["rates"]) or "0"
    lines.append(
        "SBD_PARAM({enum}, {sid}, {name}, ParamType::{typ}, {unit}, {mn}, {mx}, {df}, {sm}, {auto}, {ver}, "
        "Domain::{dom}, {rates}, {rnd}, {scale}, {hmn}, {hmx}, {ch}, {grp}, {stage})".format(
            enum=p["enum"], sid=cstr(p["sid"]), name=cstr(p["name"]), typ=p["typ"], unit=cstr(p["unit"]),
            mn=num(p["mn"]), mx=num(p["mx"]), df=num(p["df"]), sm=num(p["smooth"]), auto="true" if p["auto"] else "false",
            ver=p["ver"], dom=p["dom"], rates=rates, rnd="true" if p["rnd"] else "false", scale=num(p["scale"]),
            hmn=num(p["hmn"]), hmx=num(p["hmx"]), ch=cstr(p["choices"]), grp=cstr(p["group"]), stage=p["stage"]))
with open(os.path.join(ROOT, "source", "core", "ParamTable.inc"), "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(lines) + "\n")

os.makedirs(os.path.join(ROOT, "docs", "generated"), exist_ok=True)
md = ["# SUBDESTRUCKT — Registre des paramètres (généré)", "",
      f"Généré par `tools/gen_params.py` depuis la table compilée. {len(P)} paramètres. Ne pas éditer.", "",
      "| # | StableID | Nom | Type | Unité | Min | Max | Défaut | Auto | Rates | Domaine | Version |",
      "|---|---|---|---|---|---|---|---|---|---|---|---|"]
for i, p in enumerate(P):
    rng = (p["choices"].replace("|", " / ") if p["typ"] == "Choice" else "")
    md.append("| {i} | `{sid}` | {name} | {typ} | {unit} | {mn} | {mx} | {df} | {auto} | {rates} | {dom} | {ver} |".format(
        i=i, sid=p["sid"], name=p["name"], typ=p["typ"] + (f" ({rng})" if rng else ""), unit=p["unit"] or "—",
        mn=p["mn"], mx=p["mx"], df=p["df"], auto="oui" if p["auto"] else "non", rates=p["rates"], dom=p["dom"],
        ver=p["ver"]))
with open(os.path.join(ROOT, "docs", "generated", "PARAMETERS.md"), "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(md) + "\n")
print(f"{len(P)} parametres generes")
