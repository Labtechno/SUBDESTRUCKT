"""ARCH-UI-04-R2 — générateur reproductible de docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-PROPOSALS.csv (C02, R2-B1).

Entrées (versionnées) : source/core/ParamTable.inc (registre compilé, lecture seule), docs/V1-ARCH-UI-04-STABLEID-MAP.md
(accès UI04-COV), docs/ARCH-UI-04-DA/CONTROL-LABELS-{MEASUREMENTS,CHOICES,PROPOSALS}.csv (relevés Codex).
Règles : docs/ARCH-UI-04-R2/R2-CONTROL-NAMING-SPEC.md §8. Les largeurs « proposées » de ce CSV sont des ESTIMATIONS
(0,525 x corps) ; les mesures réelles sont produites par le Component Lab (PT-02, TYPOGRAPHY-MEASUREMENTS.csv).

Reproduction :  py -3.11 lab/tools/gen_labels.py            (réécrit le CSV)
                py -3.11 lab/tools/gen_labels.py --check    (vérifie l'identité octet pour octet, code 1 sinon)
"""
import csv, collections, io, json, pathlib, re, sys

ROOT = pathlib.Path(__file__).resolve().parents[2].as_posix() + '/'
DA = ROOT + 'docs/ARCH-UI-04-DA/'
OUT = ROOT + 'docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-PROPOSALS.csv'
CHECK = '--check' in sys.argv

# ------------------------------------------------------------------ registre compilé
reg = []
for line in open(ROOT + 'source/core/ParamTable.inc', encoding='utf-8'):
    if not line.startswith('SBD_PARAM('):
        continue
    s = re.findall(r'"([^"]*)"', line)
    typ = re.search(r'ParamType::(\w+)', line).group(1)
    reg.append(dict(id=s[0], name=s[1], unit=s[2], choices=s[3].split('|') if s[3] else [], group=s[4], type=typ))
assert len(reg) == 703
byid = {r['id']: r for r in reg}

# accès vérifié (UI04-COV)
access = {}
for line in open(ROOT + 'docs/V1-ARCH-UI-04-STABLEID-MAP.md', encoding='utf-8'):
    m = re.match(r'\| (\d+) \| `([^`]+)` \| [^|]* \| [^|]* \| ([^|]*) \|', line)
    if m:
        access[m.group(2)] = m.group(3).strip()

# mesures Codex (zoom 100)
meas = [r for r in csv.DictReader(open(DA + 'CONTROL-LABELS-MEASUREMENTS.csv', encoding='utf-8')) if r['Zoom'] == '100']
choices_m = [r for r in csv.DictReader(open(DA + 'CONTROL-LABELS-CHOICES.csv', encoding='utf-8')) if r['Zoom'] == '100']
props = list(csv.DictReader(open(DA + 'CONTROL-LABELS-PROPOSALS.csv', encoding='utf-8')))
codex_prio = {}
for p in props:
    k = p['StableID ou référence']
    codex_prio[k] = min(codex_prio.get(k, 'P3'), p['Priorité'])

K = 0.525  # largeur moyenne d'un caractère majuscule / corps (médiane calée sur 734 mesures JUCE de Codex)
def est(label, body):
    return round(len(label) * K * float(body), 1)

# ------------------------------------------------------------------ lexique (abréviations de sérigraphie)
WORD = {
    'Attack': 'ATK', 'Release': 'REL', 'Decay': 'DEC', 'Sustain': 'SUS', 'Delay': 'DLY', 'Hold': 'HOLD', 'Curve': 'CURVE',
    'Loop': 'LOOP', 'Velocity': 'VEL', 'Trigger': 'TRIG', 'Feedback': 'FDBK', 'Resonance': 'RES', 'Frequency': 'FREQ',
    'Freq': 'FREQ', 'Level': 'LVL', 'Dry/Wet': 'MIX', 'Output': 'GAIN', 'Keytrack': 'KEY', 'Character': 'CHAR', 'Damping': 'DAMP',
    'Brightness': 'BRIGHT', 'Inharmonicity': 'INHARM', 'Threshold': 'THRESH', 'Transient': 'TRANS', 'Symmetry': 'SYM',
    'Sensitivity': 'SENS', 'Detector': 'DETECT', 'Crossfade': 'XFADE', 'Playback': 'SPEED', 'Position': 'POS',
    'Interpolation': 'INTERP', 'Division': 'DIV', 'Retrigger': 'RETRIG', 'Waveform': 'WAVE', 'Tuning': 'TUNING',
    'Excitation': 'EXCITE', 'Spread': 'SPREAD', 'Shape': 'SHAPE', 'Rate': 'RATE', 'Phase': 'PHASE', 'Depth': 'DEPTH',
}
# libellés contextuels par StableID ou par motif (le titre de plaque/section porte le préfixe retiré)
SPECIAL = {
    'wt.x': 'X', 'wt.y': 'Y', 'wt.z': 'WT Z', 'wt.level': 'LVL', 'wt.coarse': 'COARSE', 'wt.fine': 'FINE', 'wt.char': 'CHAR',
    'wt.char_mode': 'CHAR MODE', 'wt.render': 'RENDER', 'wt.interp': 'INTERP', 'wt.x_bound': 'X EDGE', 'wt.y_bound': 'Y EDGE',
    'wt.x_morph': 'X MORPH', 'wt.y_morph': 'Y MORPH', 'wt.phase_mode': 'PHASE MODE', 'wt.phase': 'START PHASE', 'wt.sync': 'SYNC',
    'wt.sync_src': 'SYNC SRC', 'wt.za.type': 'A TYPE', 'wt.za.depth': 'A DEPTH', 'wt.za.curve': 'A CURVE', 'wt.za.shape': 'A SHAPE',
    'wt.zb.type': 'B TYPE', 'wt.zb.depth': 'B DEPTH', 'wt.zb.curve': 'B CURVE', 'wt.zb.shape': 'B SHAPE', 'wt.z_order': 'ORDER',
    'wt.z_fund': 'KEEP F0', 'wt.z_gain': 'GAIN COMP',
    'cross.master': 'CROSS LVL', 'cross.fwd': 'WT→FM', 'cross.rev': 'FM→WT', 'cross.dir': 'DIR', 'cross.bus.mode': 'BUS MODE',
    'cross.wt_fmode': 'FREQ MODE', 'cross.audio.mode': 'XAUDIO MODE', 'cross.audio.amount': 'XAUDIO AMT', 'cross.audio.am_dir': 'AM DIR',
    'perf.balance': 'BALANCE', 'out.gain': 'OUT GAIN', 'out.limiter': 'LIMITER', 'out.ceiling': 'CEILING',
    'fm.char_master': 'FM CHAR', 'fm.fb_mode': 'FB RESET',
    'voice.mode': 'MODE', 'voice.priority': 'PRIORITY', 'voice.bend_up': 'BEND UP', 'voice.bend_down': 'BEND DN', 'tune.a4': 'A4 REF',
    'tune.scale': 'SCALE', 'rnd.hold.rate': 'S&H RATE', 'rnd.smooth.rate': 'SMOOTH RATE', 'chaos.rate': 'CHAOS RATE',
}
# noms complets d'affichage (Inspector, infobulle) : nom hôte inchangé, sauf clarifications d'étage (affichage seulement)
FULL_CLARIFY = {
    'wt.z': 'WT Z Master (Z transform depth)', 'cross.master': 'CROSS Master (CROSS level)', 'cross.fwd': 'CROSS WT>FM (forward)',
    'cross.rev': 'CROSS FM>WT (reverse)', 'perf.balance': 'WT/FM Balance (audio, pre-chain)', 'cross.audio.amount': 'Cross Audio Amount (post-chain)',
    'cross.audio.mode': 'Cross Audio Mode (post-chain)', 'out.gain': 'Output Gain', 'out.ceiling': 'Limiter Ceiling',
}
OPWORD = {'Tuning': 'TUNING', 'Fixed': 'FIXED', 'Offset': 'OFFSET', 'Coarse': 'COARSE', 'Fine': 'FINE', 'Waveform': 'WAVE',
          'WT X': 'WT X', 'WT Y': 'WT Y', 'Pan': 'PAN', 'Mute': 'MUTE', 'Enable': 'ON', 'Character': 'CHAR', 'Phase Mode': 'PH MODE',
          'Mod Coupling': 'COUPLING', 'Mod Mode': 'MOD TYPE', 'Sync': 'SYNC', 'Level': 'LVL', 'Ratio': 'RATIO'}
DSTWORD = {'Type': 'TYPE', 'Dry/Wet': 'MIX', 'Output': 'GAIN', 'Autonomous Armed': 'ARM', 'Drive': 'DRIVE', 'Fold': 'FOLD', 'Bias': 'BIAS',
           'Symmetry': 'SYM', 'Stages': 'STAGES', 'Tone': 'TONE', 'Bits': 'BITS', 'Rate Ratio': 'RATE', 'Jitter': 'JITTER', 'Mode': 'MODE',
           'Hold': 'HOLD', 'Threshold': 'THRESH', 'Ratio': 'RATIO', 'Attack': 'ATK', 'Release': 'REL', 'Knee': 'KNEE', 'Gate': 'GATE',
           'Transient': 'TRANS', 'Makeup': 'MAKEUP', 'Frequency': 'FREQ', 'Shift': 'SHIFT', 'Carrier': 'CARRIER', 'Phase': 'PHASE',
           'Stereo Phase': 'ST PHASE', 'Feedback': 'FDBK', 'Delay': 'DLY', 'Damping': 'DAMP', 'Resonance': 'RES',
           'Self-Oscillation': 'SELF OSC', 'Size': 'SIZE', 'Playback': 'SPEED', 'Repeat': 'REPEAT', 'Position': 'POS',
           'Crossfade': 'XFADE', 'Freeze': 'FREEZE', 'Sensitivity': 'SENS', 'Curve': 'CURVE', 'Depth': 'DEPTH', 'Detector': 'DETECT',
           'FFT Size': 'FFT', 'Decay': 'DEC', 'Blur': 'BLUR', 'Tilt': 'TILT', 'Phase Drift': 'DRIFT', 'Feedback Mode': 'FB MODE'}
FLTWORD = {'Type': 'TYPE', 'Position': 'PRE/POST', 'Stereo': 'STEREO', 'F1 Mode': 'MODE', 'Cutoff': 'CUTOFF', 'Resonance': 'RES',
           'Keytrack': 'KEY', 'Drive': 'DRIVE', 'Bias': 'BIAS', 'Character': 'CHAR', 'F3 Mode': 'MODE', 'Comb Feedback': 'FDBK',
           'Comb Damping': 'DAMP', 'Detune': 'DETUNE', 'Stereo Spread': 'SPREAD', 'Modal Model': 'MODEL', 'Modes': 'MODES',
           'Decay': 'DECAY', 'Damping': 'DAMP', 'Brightness': 'BRIGHT', 'Inharmonicity': 'INHARM', 'Modal Freeze': 'FREEZE',
           'Excitation': 'EXCITE', 'Excitation Level': 'EXC LVL', 'Dry/Wet': 'MIX', 'Output': 'GAIN', 'Autonomous Armed': 'ARM'}
ENVWORD = {'Delay': 'DLY', 'Attack': 'ATK', 'Hold': 'HOLD', 'Decay': 'DEC', 'Sustain': 'SUS', 'Release': 'REL', 'Curve': 'CURVE',
           'Loop': 'LOOP', 'Velocity': 'VEL', 'Trigger': 'TRIG', 'Tempo Sync': 'SYNC'}
LFOWORD = {'Shape': 'SHAPE', 'Rate': 'RATE', 'Tempo Sync': 'SYNC', 'Division': 'DIV', 'Phase': 'PHASE', 'Retrigger': 'RETRIG'}
PORT = {'': 'PM', 'Freq': 'FREQ', 'X': 'X', 'Y': 'Y', 'Z': 'Z', 'Level': 'LVL', 'Char': 'CHAR'}

def canonical(r):
    i, n, g = r['id'], r['name'], r['group']
    if i in SPECIAL: return SPECIAL[i]
    if re.match(r'macro\d', i): return 'M' + i[-1]
    m = re.match(r'tune\.pc(\d+)', i)
    if m: return n.split(' ', 1)[1]  # C, C#, ...
    m = re.match(r'op(\d)\.', i)
    if m: return OPWORD.get(n.split(' ', 1)[1], n.split(' ', 1)[1].upper())
    m = re.match(r'fm\.op(\d)_op(\d)\.depth', i)
    if m: return ('OP%s FB' % m.group(1)) if m.group(1) == m.group(2) else 'OP%s→OP%s' % (m.group(1), m.group(2))
    m = re.match(r'cross\.wt_op(\d)\.depth', i)
    if m: return '→OP' + m.group(1)
    m = re.match(r'cross\.(op\d|bus)_wt(?:_(\w+))?\.depth', i)
    if m: return PORT[{'freq': 'Freq', 'x': 'X', 'y': 'Y', 'z': 'Z', 'level': 'Level', 'char': 'Char', None: ''}[m.group(2)]]
    m = re.match(r'cross\.bus\.w(\d)', i)
    if m: return 'OP' + m.group(1)
    if g.startswith('dst.'):
        tail = re.sub(r'^DST (WT|FM|GL)\d ', '', n)
        tail = re.sub(r'^(N1|D1|Y1|S1|N2|D2|Y2|S2) ', '', tail)
        return DSTWORD.get(tail, tail.upper())
    if g.startswith('flt.'):
        tail = re.sub(r'^FLT (WT|FM|GLOBAL) ', '', n)
        return FLTWORD.get(tail, tail.upper())
    if g.startswith('env.'):
        tail = re.sub(r'^(Amp|ENV (WT|FM|AUX1|AUX2)) ', '', n)
        return ENVWORD.get(tail, tail.upper())
    if g.startswith('lfo'):
        return LFOWORD.get(n.split(' ', 1)[1], n.split(' ', 1)[1].upper())
    if g == 'follower':
        return {'Attack': 'ATK', 'Release': 'REL'}[n.split(' ', 1)[1]]
    return n.upper()

def fullname(r):
    return FULL_CLARIFY.get(r['id'], r['name'])

CHOICE = {
    'N1 Asymmetric Wavefolder': 'N1 FOLD', 'D1 Variable Rate Crusher': 'D1 CRUSH', 'Y1 Transient Destroyer': 'Y1 TRANSIENT',
    'S1 Ring/Frequency Shifter': 'S1 RING/SHIFT', 'N2 Feedback Waveshaper': 'N2 FB SHAPER', 'D2 Glitch Buffer': 'D2 GLITCH',
    'Y2 Envelope Shaper': 'Y2 ENV SHAPER', 'S2 Spectral Freeze': 'S2 SPECTRAL', 'F1 Multimode': 'F1 MULTI', 'F2 Comb': 'F2 COMB',
    'F3 Nonlinear': 'F3 DRIVE', 'F4 Modal': 'F4 MODAL', 'Harmonic Fold': 'FOLD', 'Phase Warp': 'WARP', 'Spectral Mutation': 'MUTATE',
    'Harmonic Tilt': 'TILT', 'Harmonic Mask': 'MASK', 'Harmonic Focus': 'FOCUS', 'Harmonic Shift': 'SHIFT', 'Phase Scatter': 'SCATTER',
    'Through-Zero FM': 'TZFM', 'Linear FM': 'LIN FM', 'Exponential FM': 'EXP FM', 'Exponential': 'EXP', 'Linear': 'LIN',
    'Retrigger': 'RETRIG', 'Random': 'RAND', 'Random Hold': 'S&H', 'Random Smooth': 'SMOOTH', 'Saw Up': 'SAW UP', 'Saw Down': 'SAW DN',
    'Phase Distorted': 'PHASE DIST', 'Folded Sine': 'FOLD SINE', 'Asymmetric': 'ASYM', 'Harmonic': 'HARM', 'Wavetable': 'WT',
    'Normalized': 'NORM', 'Crossfade': 'XFADE', 'Controlled': 'CTRL', 'Regenerative': 'REGEN', 'Inharmonic': 'INHARM',
    'Dual Mono': 'DUAL', 'Linked': 'LINK', 'Internal': 'INT', 'Forward': 'FWD', 'Reverse': 'REV', 'Triangle': 'TRI', 'Square': 'SQR',
    'Membrane': 'MEMBRANE', 'Clean': 'CLEAN', 'Raw': 'RAW', 'Stepped': 'STEP', 'Spectral': 'SPECTRAL', 'Phase Quantize': 'PH QUANT',
    'Bit Reduce': 'BIT RED', 'Sample Hold': 'S&H', 'Mirror': 'MIRROR', 'Clamp': 'CLAMP', 'Wrap': 'WRAP', 'Cubic': 'CUBIC',
}
def choice_short(lbl):
    if lbl in CHOICE: return CHOICE[lbl]
    return lbl.upper() if len(lbl) <= 10 else lbl.upper()

# ------------------------------------------------------------------ occurrences
CRIT = {'wt.x', 'wt.y', 'wt.z', 'cross.master', 'cross.fwd', 'cross.rev', 'perf.balance', 'cross.audio.amount', 'cross.audio.mode',
        'out.gain', 'out.limiter', 'out.ceiling', 'macro1', 'macro2', 'macro3', 'macro4', 'wt.z_gain', 'wt.z_fund', 'wt.z_order'}
ORIENT = {'wt.x': 'X', 'wt.z': 'WT Z', 'cross.master': 'CROSS LVL'}
def prio(i):
    if i in CRIT or i.endswith('.arm') or i.endswith('freeze') or i.endswith('fb_mode') or codex_prio.get(i) == 'P1': return 'P1'
    return codex_prio.get(i, 'P2') if codex_prio.get(i, 'P2') != 'P3' else 'P3'

def status(i, lab):
    if i in ORIENT and ORIENT[i] == lab: return 'Orientation propriétaire (§7) — à confirmer R2-B'
    return 'Proposé R2-A — validation propriétaire'

HDR = ['Référence', 'Occurrence', 'Famille graphique', 'Emplacement', 'Libellé actuel', 'Libellé court proposé', 'Nom complet (Inspector, infobulle)',
       'Caractères', 'Corps px log.', 'Largeur actuelle mesurée px log.', 'Largeur proposée estimée px log.', 'Budget px log.',
       'Marge estimée px', 'Ajustement', 'Justification', "Risque d'ambiguïté", 'Priorité', 'Statut', 'Source mesure']
out = []

# (B) variantes d'emplacement mesurées
var_by_id = collections.defaultdict(list)
seen = set()
for m in meas:
    key = (m['StableID'], m['Emplacement'], m['Famille'], m['Libellé actuel'])
    if key in seen: continue
    seen.add(key)
    var_by_id[m['StableID']].append(m)

def ctx_label(i, m):
    lab = canonical(byid[i])
    loc, fam, cur = m['Emplacement'], m['Famille'], m['Libellé actuel']
    if loc.startswith('ENGINEERING') and fam == 'NEXUS Micro' and ('NETWORK' in loc) and (i.startswith('fm.op') or i.startswith('cross.')):
        return 'DEPTH'  # cellule identifiée par les en-têtes source / destination
    if loc.startswith('PERFORMANCE'):
        if i == 'cross.fwd': return 'X · WT→FM'
        if i == 'cross.rev': return 'Y · FM→WT'
        if i == 'cross.master': return 'Z · CROSS LVL' if cur.startswith('Z') and fam == 'PLASMA V' else 'CROSS LVL'
        if i in ('wt.x', 'wt.y'): return lab
        if cur in ('FM INDEX', 'FM FEEDBACK'): return 'FM INDEX' if cur == 'FM INDEX' else 'FM FDBK'
    if 'MATRICE' in loc and i.startswith('fm.op'):
        return lab
    return lab

def row(ref, occ, fam, loc, cur, lab, full, body, wcur, budget, just, risk, pr, st, src):
    w = est(lab, body) if body else ''
    margin = round(float(budget) - w, 1) if budget not in ('', None) and w != '' else ''
    adj = ''
    if margin != '':
        lines2 = fam in ('NEXUS XL', 'NEXUS S', 'NEXUS Micro', 'NEURAL', 'PLASMA V')
        adj = 'OK (≥ 8 px)' if margin >= 8 else ('OK (marge < 8 px)' if margin >= 0 else ('2 lignes requises' if lines2 else 'DÉPASSEMENT — à revoir'))
    return [ref, occ, fam, loc, cur, lab, full, len(lab), body, wcur, w, budget, margin, adj, just, risk, pr, st, src]

variant_rows = []
for i, ms in var_by_id.items():
    for m in ms:
        lab = ctx_label(i, m)
        variant_rows.append((i, m, lab))
# collisions : même libellé court pour des StableID différents dans le même emplacement
coll = collections.defaultdict(set)
for i, m, lab in variant_rows:
    coll[(m['Emplacement'], lab)].add(i)
STRONG = {'WT Z', 'CROSS LVL', 'Z · CROSS LVL', 'BALANCE', 'XAUDIO AMT', 'OUT GAIN', 'X · WT→FM', 'Y · FM→WT'}
def risk(i, loc, lab):
    n = len(coll[(loc, lab)])
    if lab in STRONG: return 'élevé avant R2 (homonymie relevée par Codex) — résolu par libellé distinct'
    if n > 1: return 'moyen — %d StableID partagent ce libellé dans l\'emplacement ; désambiguïsés par en-tête de groupe, rangée ou colonne' % n
    if lab in ('MODE', 'TYPE', 'LVL', 'DEPTH', 'RATE', 'PHASE', 'POS', 'CURVE'): return 'faible — terme générique, contexte de plaque requis'
    return 'faible'

risk_by_id = {}
for i, m, lab in variant_rows:
    rk = risk(i, m['Emplacement'], lab)
    just = 'Préfixe porté par la plaque/section' if lab != byid[i]['name'].upper() else 'Nom déjà compact'
    if i in SPECIAL or i in ORIENT: just = 'Distinction de fonction (§7) ; nom complet dans l\'Inspector'
    if lab == 'DEPTH': just = 'Cellule de réseau identifiée par les en-têtes source et destination ; route complète au focus'
    out.append(row(i, "Variante d'emplacement", m['Famille'], m['Emplacement'], m['Libellé actuel'], lab, fullname(byid[i]),
                   m['Corps retenu px logiques'], m['Largeur texte une ligne px logiques'], m['Largeur disponible px logiques'],
                   just, rk, prio(i), status(i, lab), 'Mesure JUCE Codex (zoom 100, nominal)'))
    risk_by_id.setdefault(i, []).append(rk)

# (A) canoniques (un par StableID)
canon_rows = []
for r in reg:
    i = r['id']
    ms = var_by_id.get(i, [])
    lab = canonical(r)
    if ms:
        mb = min(ms, key=lambda m: float(m['Largeur disponible px logiques'] or 1e9))
        body, budget, wcur, fam, cur = mb['Corps retenu px logiques'], mb['Largeur disponible px logiques'], mb['Largeur texte une ligne px logiques'], mb['Famille'], mb['Libellé actuel']
    else:
        body, budget, wcur, fam, cur = '11', '', '', '', ''
    rks = risk_by_id.get(i, ['faible'])
    rk = sorted(rks, key=lambda s: (not s.startswith('élevé'), not s.startswith('moyen')))[0]
    canon_rows.append(row(i, 'Paramètre (canonique)', fam, access.get(i, ''), cur, lab, fullname(r), body, wcur, budget,
                          'Libellé de référence ; budget = minimum observé parmi %d emplacement(s)' % max(1, len(ms)), rk, prio(i), status(i, lab),
                          'Registre + UI04-COV + mesure JUCE Codex'))

# (C) options de listes (complètes depuis le registre)
cm = {}
for c in choices_m:
    k = (c['StableID'], c['Indice'])
    b = float(c['Budget utile minimum px logiques'] or 1e9)
    if k not in cm or b < cm[k][1]: cm[k] = (c, b)
choice_rows = []
for r in reg:
    for k, lbl in enumerate(r['choices']):
        c = cm.get((r['id'], str(k)))
        lab = choice_short(lbl)
        budget = c[0]['Budget utile minimum px logiques'] if c else ''
        wcur = c[0]['Largeur px logiques'] if c else ''
        mode = c[0]['Mode'] if c else 'non observé au banc'
        choice_rows.append(row('%s#%d' % (r['id'], k), 'Option de liste (%s)' % mode, 'SYNAPTIC TRACK', access.get(r['id'], ''), lbl, lab,
                               lbl + ' (indice %d inchangé)' % k, '11' if c else '11', wcur, budget,
                               'Indice et libellé hôte inchangés ; affichage compact' if lab != lbl else 'Inchangé (déjà compact)',
                               'faible', 'P2' if r['id'] not in CRIT else 'P1', 'Proposé R2-A — validation propriétaire',
                               'Mesure JUCE Codex' if c else 'Registre (non mesuré)'))

# (D) hors registre, titres, états, Inspector
OFF = [
 # référence, emplacement, famille, actuel, proposé, nom complet, priorité, justification
 ('bar.peak', 'Barre globale', 'Hors registre', 'OUT', 'PEAK', 'Output Peak (dBFS, block peak at plugin output)', 'P1', 'Mesure distincte du gain ; toujours visible (DA-A08)'),
 ('bar.authorize', 'Barre globale', 'Hors registre', 'Autoriser', 'AUTHORIZE', 'Authorize autonomous states (this instance)', 'P1', 'Périmètre d\'instance au focus ; contrat 0.4.1'),
 ('bar.panic', 'Barre globale', 'Hors registre', 'PANIC', 'PANIC', 'Panic (All Sound Off, revoke)', 'P1', 'Inchangé'),
 ('bar.drone', 'Barre globale', 'Hors registre', 'Drone', 'DRONE', 'Drone (hold gate)', 'P1', 'Majuscules de sérigraphie'),
 ('bar.quality', 'Barre globale', 'Hors registre', 'Quality Normal', 'QUALITY', 'Quality (Eco / Normal / High)', 'P2', 'Valeur dans la liste'),
 ('bar.open', 'Barre globale', 'Hors registre', 'Ouvrir', 'PRESETS', 'Open preset browser', 'P2', 'Le bouton navigue (constat Codex) : nommer la destination'),
 ('bar.save', 'Barre globale', 'Hors registre', 'Enregistrer', 'SAVE…', 'Save preset (opens Presets/State)', 'P2', 'Ellipse : action complétée ailleurs'),
 ('bar.undo', 'Barre globale', 'Hors registre', 'Undo', 'UNDO', 'Undo', 'P2', 'Anglais conservé'),
 ('bar.redo', 'Barre globale', 'Hors registre', 'Redo', 'REDO', 'Redo', 'P2', 'Anglais conservé'),
 ('bar.inspector', 'Barre globale', 'Hors registre', 'Inspector', 'INSPECTOR', 'Inspector', 'P2', 'Majuscules'),
 ('bar.zoom', 'Barre globale', 'Hors registre', '100 %', '100%', 'Zoom', 'P3', 'Format compact'),
 ('perf.core.status', 'Performance / Core', 'Hors registre', 'RENDU STATIQUE PROVISOIRE · illustratif · moteur 3D : ARCH-UI-05', 'STATIC PREVIEW', 'Static preview (illustrative, not measured)', 'P1', 'Le statut ne doit pas primer sur les axes (Codex DA-14)'),
 ('perf.core.x-axis', 'Performance / Core', 'Hors registre', 'X  cross.fwd  WT>FM', 'X · WT→FM', 'X axis: CROSS WT>FM (cross.fwd)', 'P1', 'StableID au focus seulement'),
 ('perf.core.y-axis', 'Performance / Core', 'Hors registre', 'Y  cross.rev  FM>WT', 'Y · FM→WT', 'Y axis: CROSS FM>WT (cross.rev)', 'P1', 'StableID au focus seulement'),
 ('perf.auth.armed', 'Performance / Autonomy', 'Hors registre', 'Armements', 'ARMED', 'Armed modules (configured)', 'P1', 'Liste des modules armés'),
 ('perf.auth.exec', 'Performance / Autonomy', 'Hors registre', 'Exécution', 'AUTONOMY', 'Autonomous execution (authorized / suspended / revoked / render)', 'P1', 'État d\'autorisation, distinct de ARM'),
 ('perf.auth.transport', 'Performance / Autonomy', 'Hors registre', 'Transport', 'TRANSPORT', 'Host transport (last known)', 'P2', 'Majuscules'),
 ('perf.fm.index', 'Performance / FM', 'Hors registre', 'FM INDEX', 'FM INDEX', 'FM index (contextual: main internal route depth)', 'P2', 'Libellé contextuel non arbitré (ARCH-UI-03 §8)'),
 ('perf.fm.feedback', 'Performance / FM', 'Hors registre', 'FM FEEDBACK', 'FM FDBK', 'FM feedback (contextual: carrier self-loop depth)', 'P2', 'Libellé contextuel non arbitré'),
 ('perf.macro.name', 'Performance / Macros', 'Hors registre', '(nom saisi par utilisateur)', '(nom utilisateur)', 'Macro name (user, saved with preset)', 'P2', 'Le nom utilisateur reste libre'),
 ('perf.balance.note', 'Performance / Balance', 'Hors registre', 'Balance audio post-Amp…', 'PRE-CHAIN', 'Audio balance before WT/FM chains', 'P1', 'Distinction BALANCE / XAUDIO'),
 ('perf.monitor.scope', 'Performance / Monitor', 'Hors registre', 'SCOPE', 'SCOPE', 'Oscilloscope (plugin output, mono sum, post gain and limiter)', 'P2', 'Point de prélèvement nommé'),
 ('perf.monitor.spectrum', 'Performance / Monitor', 'Hors registre', 'SPECTRE', 'SPECTRUM', 'Spectrum (plugin output, mono sum, post gain and limiter)', 'P2', 'Anglais'),
 ('perf.monitor.xy', 'Performance / Monitor', 'Hors registre', 'XY', 'XY', 'WT XY position (Base / Effective)', 'P2', 'Inchangé'),
 ('perf.monitor.net', 'Performance / Monitor', 'Hors registre', 'RESEAU', 'NETWORK', 'FM / CROSS network', 'P2', 'Anglais'),
 ('sound.tab.engines', 'Sound Design / Onglets', 'Titre', 'MOTEURS', 'ENGINES', 'Engines', 'P2', 'Anglais'),
 ('sound.tab.process', 'Sound Design / Onglets', 'Titre', 'TRAITEMENTS', 'PROCESS', 'Process chains', 'P2', 'Anglais'),
 ('sound.tab.mod', 'Sound Design / Onglets', 'Titre', 'MODULATION', 'MOD', 'Modulation', 'P2', 'Anglais compact'),
 ('sound.cross.tab.forward', 'Sound Design / CROSS', 'Titre', 'WT › FM', 'WT→FM', 'CROSS forward routes', 'P1', 'Flèche unique'),
 ('sound.cross.tab.reverse', 'Sound Design / CROSS', 'Titre', 'FM › WT', 'FM→WT', 'CROSS reverse routes', 'P1', 'Flèche unique'),
 ('sound.cross.tab.direct', 'Sound Design / CROSS', 'Titre', 'DIRECT', 'DIRECT', 'CROSS direct controls', 'P2', 'Inchangé'),
 ('sound.cross.tab.bus', 'Sound Design / CROSS', 'Titre', 'BUS / AUDIO', 'BUS / XAUDIO', 'Bus and Cross Audio', 'P2', 'Distinction XAUDIO'),
 ('sound.cross.reset', 'Sound Design / CROSS', 'Hors registre', 'Cross Reset', 'CROSS RESET', 'Cross Reset (clear CROSS history)', 'P2', 'Majuscules'),
 ('sound.wt.tab.phase', 'Sound Design / WT', 'Titre', 'PHASE / SYNC', 'PHASE/SYNC', 'Phase and sync', 'P3', 'Compact'),
 ('sound.wt.tab.read', 'Sound Design / WT', 'Titre', 'LECTURE', 'READ', 'Table reading', 'P2', 'Anglais'),
 ('sound.wt.tab.char', 'Sound Design / WT', 'Titre', 'CARACTÈRE', 'CHAR', 'Character', 'P2', 'Anglais'),
 ('sound.wtz.slot-a', 'Sound Design / WT Z', 'Titre', 'Emplacement A', 'SLOT A', 'Z slot A', 'P1', 'Anglais ; préfixe A des enfants'),
 ('sound.wtz.slot-b', 'Sound Design / WT Z', 'Titre', 'Emplacement B', 'SLOT B', 'Z slot B', 'P1', 'Anglais'),
 ('sound.wtz.order', 'Sound Design / WT Z', 'Hors registre', 'ORDRE EFFECTIF DES TRANSFORMATIONS Z', 'EFFECTIVE Z ORDER', 'Effective Z order (engine)', 'P1', 'Restriction inter-domaines signalée (non arbitrée)'),
 ('sound.chain.capture', 'Sound Design / Process', 'Hors registre', 'Capturer gels', 'CAPTURE', 'Capture frozen D2/S2 content (saved with preset/project)', 'P1', 'Opération distincte de FREEZE et ARM'),
 ('sound.chain.excite', 'Sound Design / Process', 'Hors registre', 'Excitation', 'EXCITE', 'Manual excitation (F4)', 'P2', 'Anglais'),
 ('sound.chain.output', 'Sound Design / Process', 'Titre', 'SORTIE // sûreté', 'OUTPUT SAFETY', 'Output safety (limiter)', 'P2', 'Anglais'),
 ('sound.chain.wt', 'Sound Design / Process', 'Titre', 'CHAÎNE WT', 'WT CHAIN', 'WT chain (pre Cross Audio)', 'P2', 'Anglais'),
 ('sound.chain.fm', 'Sound Design / Process', 'Titre', 'CHAÎNE FM', 'FM CHAIN', 'FM chain (pre Cross Audio)', 'P2', 'Anglais'),
 ('sound.chain.gl', 'Sound Design / Process', 'Titre', 'CHAÎNE GLOBALE', 'GLOBAL CHAIN', 'Global chain (post Cross Audio)', 'P2', 'Anglais'),
 ('sound.chain.filter', 'Sound Design / Process', 'Titre', 'FILTRE', 'FILTER', 'Filter / resonator', 'P2', 'Anglais'),
 ('sound.chain.dst', 'Sound Design / Process', 'Titre', 'DESTRUCTION 1 / DESTRUCTION 2', 'DESTRUCT 1 / 2', 'Destruction slot 1/2', 'P2', 'Compact'),
 ('sound.flow', 'Sound Design / Process', 'Titre', 'FLUX DU SIGNAL', 'SIGNAL FLOW', 'Signal flow (engine order)', 'P2', 'Anglais'),
 ('sound.mod.env', 'Sound Design / Mod', 'Titre', 'ENVELOPPES', 'ENVELOPES', 'Envelopes', 'P2', 'Anglais'),
 ('sound.mod.rnd', 'Sound Design / Mod', 'Titre', 'ALÉATOIRE · CHAOS · SUIVEURS', 'RANDOM · CHAOS · FOLLOW', 'Random, chaos, followers', 'P2', 'Anglais'),
 ('eng.group.network', 'Engineering / Groupes (DA-A10)', 'Titre', 'FM NETWORK · CROSS NETWORK', 'NETWORK', 'Network: FM, CROSS', 'P1', 'Regroupement DA-A10'),
 ('eng.group.modulation', 'Engineering / Groupes (DA-A10)', 'Titre', 'MODULATION · MIDI LEARN', 'MODULATION', 'Modulation: Mod Matrix, MIDI Learn', 'P1', 'Regroupement DA-A10'),
 ('eng.group.banklab', 'Engineering / Groupes (DA-A10)', 'Titre', 'BANK LAB', 'BANK LAB', 'Bank Lab', 'P1', 'Inchangé'),
 ('eng.group.state', 'Engineering / Groupes (DA-A10)', 'Titre', 'VOICE / TUNING · PRESETS / STATE · CAPTURES / ASSETS', 'STATE', 'State: Voice/Tuning, Presets, Captures/Assets', 'P1', 'Regroupement DA-A10'),
 ('eng.group.tools', 'Engineering / Groupes (DA-A10)', 'Titre', 'RANDOMIZE · DIAGNOSTICS', 'TOOLS', 'Tools: Randomize, Diagnostics', 'P1', 'Regroupement DA-A10'),
 ('eng.view.neural', 'Engineering / Réseaux', 'Titre', '—', 'NEURAL', 'Neural network view', 'P1', 'DA-A14'),
 ('eng.view.table', 'Engineering / Réseaux', 'Titre', '—', 'TABLE', 'Table view', 'P1', 'DA-A14'),
 ('eng.matrix.add', 'Engineering / Réseaux', 'Hors registre', '+ route', '+ ROUTE', 'Add route (validated transaction)', 'P2', 'Majuscules'),
 ('eng.matrix.remove', 'Engineering / Réseaux', 'Hors registre', '− route', '− ROUTE', 'Remove route (validated transaction)', 'P2', 'Majuscules'),
 ('eng.matrix.preview', 'Engineering / Réseaux', 'Hors registre', 'absente / ajout : D (T2)', 'OFF · ADD: D', 'Route absent; class if added: Delayed (T2)', 'P2', 'Classe causale lisible'),
 ('eng.matrix.self', 'Engineering / Réseaux', 'Hors registre', 'self', 'FB', 'Self-feedback', 'P2', 'Cohérent avec OPn FB'),
 ('eng.matrix.port', 'Engineering / CROSS', 'Hors registre', '› WT.FREQ', 'WT FREQ', 'Destination port WT frequency', 'P1', 'En-tête de colonne'),
 ('eng.fm.topology', 'Engineering / FM', 'Hors registre', 'Topologie FM préréglée…', 'TOPOLOGY…', 'FM topology preset', 'P2', 'Anglais'),
 ('eng.mod.add', 'Engineering / Modulation', 'Hors registre', 'Ajouter', '+ LINK', 'Add modulation link', 'P2', 'Lien global ≠ route spécialisée'),
 ('eng.mod.remove', 'Engineering / Modulation', 'Hors registre', 'Retirer', 'REMOVE', 'Remove link', 'P2', 'Anglais'),
 ('eng.mod.from', 'Engineering / Modulation', 'Hors registre', '= Inspector', '← INSPECTOR', 'Destination from Inspector', 'P2', 'Anglais'),
 ('eng.mod.filter', 'Engineering / Modulation', 'Hors registre', 'filtrer (source ou destination)', 'Filter source / dest…', 'Filter links', 'P2', 'Texte d\'aide'),
 ('eng.mod.curve', 'Engineering / Modulation', 'Hors registre', 'Courbe', 'CURVE', 'Edit custom curve', 'P2', 'Anglais'),
 ('eng.mod.active', 'Engineering / Modulation', 'Hors registre', 'actif', 'ON', 'Link enabled', 'P2', 'Anglais'),
 ('eng.learn.selected', 'Engineering / MIDI Learn', 'Hors registre', "Apprendre : paramètre de l'Inspector", 'LEARN TARGET', 'Learn: Inspector parameter', 'P2', 'Anglais'),
 ('eng.learn.cancel', 'Engineering / MIDI Learn', 'Hors registre', "Annuler l'apprentissage", 'CANCEL', 'Cancel learn', 'P2', 'Anglais'),
 ('eng.bank.import', 'Engineering / Bank Lab', 'Hors registre', 'Importer WAV...', 'IMPORT WAV…', 'Import WAV bank', 'P2', 'Anglais'),
 ('eng.bank.export', 'Engineering / Bank Lab', 'Hors registre', 'Exporter WAV...', 'EXPORT WAV…', 'Export WAV bank', 'P2', 'Anglais'),
 ('eng.bank.apply', 'Engineering / Bank Lab', 'Hors registre', 'Appliquer la forme', 'APPLY WAVE', 'Apply drawn wave (new bank)', 'P2', 'Anglais'),
 ('eng.bank.revert', 'Engineering / Bank Lab', 'Hors registre', 'Reprendre', 'REVERT', 'Revert drawing', 'P2', 'Anglais'),
 ('eng.bank.generate', 'Engineering / Bank Lab', 'Hors registre', 'Générer', 'GENERATE', 'Generate bank', 'P2', 'Anglais'),
 ('eng.bank.save', 'Engineering / Bank Lab', 'Hors registre', 'Enregistrer .sdwt', 'SAVE .SDWT', 'Save to .sdwt library', 'P2', 'Anglais'),
 ('eng.bank.load', 'Engineering / Bank Lab', 'Hors registre', 'Charger', 'LOAD', 'Load from library', 'P2', 'Anglais'),
 ('eng.state.load', 'Engineering / State', 'Hors registre', 'Charger', 'LOAD', 'Load preset', 'P2', 'Anglais'),
 ('eng.state.fav', 'Engineering / State', 'Hors registre', 'Favori', 'FAVORITE', 'Toggle favorite', 'P2', 'Anglais'),
 ('eng.state.save', 'Engineering / State', 'Hors registre', 'Enregistrer', 'SAVE', 'Save preset', 'P2', 'Anglais'),
 ('eng.state.tags', 'Engineering / State', 'Hors registre', 'tags (séparés par des virgules)', 'Tags, comma separated', 'Preset tags', 'P2', 'Texte d\'aide'),
 ('eng.state.drone', 'Engineering / State', 'Hors registre', "Restaurer la configuration du Drone à l'ouverture du projet", 'RESTORE DRONE', 'Restore Drone configuration on project load (never its execution)', 'P1', 'Distinct de AUTHORIZE'),
 ('eng.rnd.random', 'Engineering / Tools', 'Hors registre', 'RANDOMIZE', 'RANDOMIZE', 'Randomize', 'P2', 'Inchangé'),
 ('eng.rnd.mutate', 'Engineering / Tools', 'Hors registre', 'MUTATE', 'MUTATE', 'Mutate', 'P2', 'Inchangé'),
 ('eng.rnd.start', 'Engineering / Tools', 'Hors registre', 'Evolve start', 'EVOLVE', 'Start Evolve', 'P2', 'Bascule START/STOP'),
 ('eng.rnd.stop', 'Engineering / Tools', 'Hors registre', 'Stop', 'STOP', 'Stop Evolve', 'P2', 'Majuscules'),
 ('eng.rnd.reset', 'Engineering / Tools', 'Hors registre', 'Reset', 'RESET', 'Reset Evolve offsets', 'P2', 'Majuscules'),
 ('eng.rnd.commit', 'Engineering / Tools', 'Hors registre', 'Commit', 'COMMIT', 'Commit Evolve to Base', 'P2', 'Majuscules'),
 ('eng.rnd.locks', 'Engineering / Tools', 'Hors registre', 'Lever les verrous', 'CLEAR LOCKS', 'Clear randomize locks', 'P2', 'Anglais'),
 ('eng.captures.now', 'Engineering / State', 'Hors registre', 'Capturer maintenant', 'CAPTURE', 'Capture frozen D2/S2 content now', 'P1', 'Même terme que Process'),
 ('eng.captures.clear', 'Engineering / State', 'Hors registre', 'Effacer WT1', 'CLEAR WT1', 'Clear capture of slot WT1 (… GL2)', 'P2', 'Anglais'),
 ('inspector.pin', 'Inspector', 'Hors registre', 'Épingler', 'PIN', 'Pin inspector context', 'P2', 'Anglais'),
 ('inspector.lock', 'Inspector', 'Hors registre', 'Verrou Randomize', 'LOCK', 'Lock against Randomize / Mutate', 'P2', 'Anglais'),
 ('inspector.reset', 'Inspector', 'Hors registre', 'Défaut', 'DEFAULT', 'Reset to default', 'P2', 'Anglais'),
 ('inspector.entry', 'Inspector', 'Hors registre', 'Saisie', 'ENTER', 'Numeric entry', 'P2', 'Anglais'),
 ('inspector.learn', 'Inspector', 'Hors registre', 'MIDI Learn', 'LEARN', 'MIDI Learn', 'P2', 'Anglais'),
 ('inspector.add', 'Inspector', 'Hors registre', '+ Modulation', '+ MOD', 'Add global modulation link', 'P2', 'Anglais'),
 ('inspector.base', 'Inspector', 'Champ', 'Base', 'BASE', 'Base value', 'P1', 'Orientation §7'),
 ('inspector.effective', 'Inspector', 'Champ', 'Effective', 'EFFECTIVE', 'Effective value (engine, decimated)', 'P1', 'Orientation §7'),
 ('inspector.range', 'Inspector', 'Champ', 'plage … / défaut …', 'RANGE · UNIT · DEFAULT', 'Range, unit, default', 'P2', 'Hiérarchie DA-A11'),
 ('inspector.cadence', 'Inspector', 'Champ', 'cadences C A', 'RATES: CONTROL, AUDIO', 'Modulation rates', 'P2', 'Détails techniques'),
 ('inspector.stableid', 'Inspector', 'Champ', 'StableID', 'ID', 'StableID (technical details)', 'P3', 'Détails techniques'),
 ('inspector.mods', 'Inspector', 'Champ', 'Aucune modulation globale.', 'NO MODULATION', 'Global modulations', 'P2', 'Anglais'),
 ('state.off', 'États NEURAL CELL', 'État', 'OFF', 'OFF', 'Off', 'P1', 'Forme + texte'),
 ('state.on', 'États NEURAL CELL', 'État', 'ON', 'ON', 'On', 'P1', 'Forme + texte'),
 ('state.armed', 'États NEURAL CELL', 'État', 'ARMÉ', 'ARMED', 'Armed, execution suspended (authorization required)', 'P1', 'Anglais ; jamais confondu avec ARM (fonction)'),
 ('state.active', 'États NEURAL CELL', 'État', 'ACTIF', 'ACTIVE', 'Armed and authorized', 'P1', 'Anglais'),
 ('state.refused', 'États NEURAL CELL', 'État', 'REFUS', 'DENIED', 'Request refused', 'P2', 'Anglais'),
 ('state.render', 'États autonomie', 'État', 'hors temps réel (transport)', 'RENDER', 'Temporary render authorization (0.4.1)', 'P1', 'Contrat 0.4.1 §2 bis'),
 ('state.configured', 'États autonomie', 'État', 'CONFIGURÉ · SUSPENDU', 'SUSPENDED', 'Drone configured, execution suspended (authorization required)', 'P1', 'Configuration ≠ exécution'),
 ('state.suspended', 'États autonomie', 'État', 'suspendue', 'SUSPENDED', 'Autonomous execution suspended', 'P1', 'Anglais'),
 ('state.authorized', 'États autonomie', 'État', 'autorisée / accordée', 'AUTHORIZED', 'Autonomous execution authorized', 'P1', 'Anglais'),
 ('state.revoked', 'États autonomie', 'État', 'révoquée (Panic)', 'REVOKED', 'Revoked by Panic (latched)', 'P1', 'Anglais'),
 ('state.inactive', 'NEXUS', 'État', 'INACTIF', 'INACTIVE', 'Value kept, no effect in this context', 'P2', 'Anglais'),
 ('units.pos', 'Valeurs', 'Unité', 'pos', '(aucune) 0–1', 'Normalized position', 'P2', 'Unité affichée seulement si utile ; domaine dans l\'Inspector'),
 ('units.lin', 'Valeurs', 'Unité', 'lin', '(aucune) 0–1', 'Linear normalized', 'P2', 'Idem'),
 ('units.bip', 'Valeurs', 'Unité', 'bip', '(aucune) −1…+1', 'Bipolar normalized', 'P2', 'Idem'),
 ('units.peak', 'Valeurs', 'Unité', 'dBFS', 'dBFS', 'Peak level', 'P1', 'Inchangé, distinct de dB'),
 ('space.performance', 'Barre / Espaces', 'Titre', 'PERFORMANCE', 'PERFORMANCE', 'Performance', 'P3', 'Inchangé'),
 ('space.sound', 'Barre / Espaces', 'Titre', 'SOUND DESIGN', 'SOUND DESIGN', 'Sound Design', 'P3', 'Inchangé'),
 ('space.engineering', 'Barre / Espaces', 'Titre', 'ENGINEERING', 'ENGINEERING', 'Engineering', 'P3', 'Inchangé'),
]
off_rows = []
for ref, loc, fam, cur, lab, full, pr, just in OFF:
    budget = ''
    for p in props:
        if p['StableID ou référence'] == ref and p['Budget utile minimum px logiques']: budget = p['Budget utile minimum px logiques']
    st = 'Orientation propriétaire (§7) — à confirmer R2-B' if lab in ('AUTHORIZE', 'CAPTURE', 'BASE', 'EFFECTIVE', 'ARM', 'FREEZE') else 'Proposé R2-A — validation propriétaire'
    off_rows.append(row(ref, fam if fam in ('Titre', 'Champ', 'État', 'Unité') else 'Hors registre', fam, loc, cur, lab, full, '11', '', budget,
                        just, 'faible', pr, st, 'Relevé des chaînes UI (code 0.4.1) / proposition Codex'))

allrows = canon_rows + out + choice_rows + off_rows
buf = io.StringIO(newline='')
w = csv.writer(buf)
w.writerow(HDR)
w.writerows(allrows)
data = buf.getvalue().encode('utf-8-sig')
if CHECK:
    same = open(OUT, 'rb').read() == data
    print('CSV identique octet pour octet' if same else 'CSV DIFFÉRENT du fichier versionné')
    sys.exit(0 if same else 1)
open(OUT, 'wb').write(data)

print('lignes', len(allrows))
