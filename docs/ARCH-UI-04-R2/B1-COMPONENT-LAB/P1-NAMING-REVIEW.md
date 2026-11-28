# ARCH-UI-04-R2-B1 — revue des libellés P1

Validation progressive par priorité (OD-02, option B). **Aucun libellé n'est validé par ce document** : le propriétaire arbitre groupe par groupe. Source : `../R2-CONTROL-LABELS-PROPOSALS.csv` (ligne P1 = priorité P1). Mesures : PT-02 (`TYPOGRAPHY-MEASUREMENTS.csv`), largeurs en px logiques à 100 %, police d'interface de chaque plateforme. Ce fichier est généré par `lab/tools/build_review.py`.

Plateformes mesurées : macos, windows. Occurrences P1 : **323** (112 canoniques, 167 variantes, 6 options, 38 hors registre).

## 1. Synthèse par groupe

| Groupe | Occurrences | Libellés distincts | Signalements |
|---|---|---|---|
| [Distinctions critiques](#distinctions-critiques) | 36 | 19 | 7 |
| [Autonomie : ARM, DRONE, AUTHORIZE, états](#autonomie--arm-drone-authorize-états) | 32 | 13 | 14 |
| [Gel et capture](#gel-et-capture) | 32 | 2 | 1 |
| [Modes de feedback](#modes-de-feedback) | 14 | 2 | 0 |
| [Position WT et transformations Z](#position-wt-et-transformations-z) | 33 | 18 | 5 |
| [Macros](#macros) | 8 | 4 | 4 |
| [Core et PERFORMANCE](#core-et-performance) | 4 | 4 | 4 |
| [Barre globale](#barre-globale) | 1 | 1 | 1 |
| [ENGINEERING : groupes, vues, ports](#engineering--groupes-vues-ports) | 8 | 8 | 7 |
| [Réseau CROSS et FM](#réseau-cross-et-fm) | 111 | 10 | 0 |
| [Autres P1 (priorité reprise de Codex)](#autres-p1-priorité-reprise-de-codex) | 44 | 8 | 4 |

## 2. Libellés critiques à arbitrer en priorité

| Fonction | Libellé proposé | Nom complet (Inspector, infobulle) | Note |
|---|---|---|---|
| `wt.z` | **WT Z** | WT Z Master (Z transform depth) | homonymie « Z Master » levée |
| `cross.master` | **CROSS LVL** | CROSS Master (CROSS level) | axe Z du Core : « Z · CROSS LVL » (deux lignes sur PLASMA V) |
| `cross.fwd` | **WT→FM** | CROSS WT>FM (forward) | axe X du Core : « X · WT→FM » |
| `cross.rev` | **FM→WT** | CROSS FM>WT (reverse) | axe Y du Core : « Y · FM→WT » |
| `perf.balance` | **BALANCE** | WT/FM Balance (audio, pre-chain) | balance audio pré-chaînes |
| `cross.audio.amount` | **XAUDIO AMT** | Cross Audio Amount (post-chain) | post-chaînes |
| `out.gain` | **OUT GAIN** | Output Gain | **recommandation de l'architecte : OUT GAIN** (et non GAIN, réservé au gain de sortie des étages) |
| `bar.peak` | **PEAK** | Output Peak (dBFS, block peak at plugin output) | mesure en dBFS, jamais dB |
| `bar.authorize` | **AUTHORIZE** | Authorize autonomous states (this instance) | action d'instance |
| `sound.chain.capture` | **CAPTURE** | Capture frozen D2/S2 content (saved with preset/project) | action ; même terme partout |
| `state.render` | **RENDER** | Temporary render authorization (0.4.1) | rendu hors ligne (0.4.1) |

## 3. Analyses transversales

**Homonymies** (même libellé, StableID différents, même emplacement mesuré) : 16 groupe(s). Toutes sont désambiguïsées par l'en-tête de plaque ou de cellule ; à confirmer visuellement.

- `DEPTH` — ENGINEERING / CROSS NETWORK — cross.bus_wt.depth, cross.bus_wt_char.depth, cross.bus_wt_freq.depth, cross.bus_wt_level.depth, cross.bus_wt_x.depth, cross.bus_wt_y.depth, cross.bus_wt_z.depth, cross.op1_wt.depth, cross.op1_wt_char.depth, cross.op1_wt_freq.depth, cross.op1_wt_level.depth, cross.op1_wt_x.depth, cross.op1_wt_y.depth, cross.op1_wt_z.depth, cross.op2_wt.depth, cross.op2_wt_char.depth, cross.op2_wt_freq.depth, cross.op2_wt_level.depth, cross.op2_wt_x.depth, cross.op2_wt_y.depth, cross.op2_wt_z.depth, cross.op3_wt.depth, cross.op3_wt_char.depth, cross.op3_wt_freq.depth, cross.op3_wt_level.depth, cross.op3_wt_x.depth, cross.op3_wt_y.depth, cross.op3_wt_z.depth, cross.op4_wt.depth, cross.op4_wt_char.depth, cross.op4_wt_freq.depth, cross.op4_wt_level.depth, cross.op4_wt_x.depth, cross.op4_wt_y.depth, cross.op4_wt_z.depth
- `ON` — SOUND DESIGN / MOTEURS / WT LECTURE \| CROSS WT › FM \| FM OP2 — op1.enable, op2.enable
- `CHAR` — SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 — cross.bus_wt_char.depth, cross.op1_wt_char.depth, cross.op2_wt_char.depth, cross.op3_wt_char.depth, cross.op4_wt_char.depth
- `FREQ` — SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 — cross.bus_wt_freq.depth, cross.op1_wt_freq.depth, cross.op2_wt_freq.depth, cross.op3_wt_freq.depth, cross.op4_wt_freq.depth
- `LVL` — SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 — cross.bus_wt_level.depth, cross.op1_wt_level.depth, cross.op2_wt_level.depth, cross.op3_wt_level.depth, cross.op4_wt_level.depth
- `ON` — SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 — op2.enable, op3.enable
- `PM` — SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 — cross.bus_wt.depth, cross.op1_wt.depth, cross.op2_wt.depth, cross.op3_wt.depth, cross.op4_wt.depth
- `X` — SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 — cross.bus_wt_x.depth, cross.op1_wt_x.depth, cross.op2_wt_x.depth, cross.op3_wt_x.depth, cross.op4_wt_x.depth
- `Y` — SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 — cross.bus_wt_y.depth, cross.op1_wt_y.depth, cross.op2_wt_y.depth, cross.op3_wt_y.depth, cross.op4_wt_y.depth
- `Z` — SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 — cross.bus_wt_z.depth, cross.op1_wt_z.depth, cross.op2_wt_z.depth, cross.op3_wt_z.depth, cross.op4_wt_z.depth
- `ON` — SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 — op3.enable, op4.enable
- `ARM` — SOUND DESIGN / TRAITEMENTS — dst.fm1.arm, dst.fm2.arm, dst.gl1.arm, dst.gl2.arm, dst.wt1.arm, dst.wt2.arm, flt.fm.arm, flt.gl.arm, flt.wt.arm
- `FB MODE` — SOUND DESIGN / TRAITEMENTS — dst.fm1.d2.fb_mode, dst.fm2.d2.fb_mode, dst.gl1.d2.fb_mode, dst.gl2.d2.fb_mode, dst.wt1.d2.fb_mode, dst.wt2.d2.fb_mode
- `FREEZE` — SOUND DESIGN / TRAITEMENTS — dst.fm1.d2.freeze, dst.fm1.s2.freeze, dst.fm2.d2.freeze, dst.fm2.s2.freeze, dst.gl1.d2.freeze, dst.gl1.s2.freeze, dst.gl2.d2.freeze, dst.gl2.s2.freeze, dst.wt1.d2.freeze, dst.wt1.s2.freeze, dst.wt2.d2.freeze, dst.wt2.s2.freeze, flt.fm.freeze, flt.gl.freeze, flt.wt.freeze
- `PRE/POST` — SOUND DESIGN / TRAITEMENTS — flt.fm.pos, flt.gl.pos, flt.wt.pos
- `SPEED` — SOUND DESIGN / TRAITEMENTS — dst.fm1.d2.speed, dst.fm2.d2.speed, dst.gl1.d2.speed, dst.gl2.d2.speed, dst.wt1.d2.speed, dst.wt2.d2.speed

**Redondances potentielles** (préfixe de domaine conservé dans le libellé) : 6. Conservées volontairement quand le libellé apparaît hors de la plaque du domaine (WT Z, CROSS LVL) ; sinon à retirer.

- `cross.master` PERFORMANCE / PERFORMANCE : **CROSS LVL**
- `wt.z` PERFORMANCE / PERFORMANCE : **WT Z**
- `wt.z` PERFORMANCE / PERFORMANCE : **WT Z**
- `wt.z` SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 : **WT Z**
- `cross.master` PERFORMANCE / PERFORMANCE : **CROSS LVL**
- `cross.master` SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 : **CROSS LVL**

**Dépassements** (toutes priorités, au moins une plateforme) : 0.


**Libellés nécessitant deux lignes** (toutes priorités, au moins une plateforme) : 1.

- `cross.master` PERFORMANCE / PERFORMANCE : **Z · CROSS LVL** — budget 70.00 px ; macos 68.96 px (OK_MARGE<8) ; windows 72.91 px (2_LIGNES)

**Marges insuffisantes (< 8 px)** (toutes priorités, au moins une plateforme) : 3.

- `cross.master` PERFORMANCE / PERFORMANCE : **Z · CROSS LVL** — budget 70.00 px ; macos 68.96 px (OK_MARGE<8) ; windows 72.91 px (2_LIGNES)
- `wt.x_morph#2` SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 : **SPECTRAL** — budget 48.67 px ; macos 44.95 px (OK_MARGE<8) ; windows 46.78 px (OK_MARGE<8)
- `wt.y_morph#2` SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 : **SPECTRAL** — budget 48.67 px ; macos 44.95 px (OK_MARGE<8) ; windows 46.78 px (OK_MARGE<8)

**Abréviations à valider** (occurrences P1) : LVL (14), XAUDIO (8), DIR (6), KEEP F0 (2), GAIN COMP (2), XFADE (1).

**Budgets non mesurés** (P1, éléments hors registre sans gabarit 0.4.1 relevé par Codex) : 36. Leur budget sera fixé par la disposition R2-C ; les largeurs mesurées sont fournies.

**Exceptions contextuelles** : `DEPTH` dans les cellules de réseau (route portée par les en-têtes) ; préfixes `X · `, `Y · `, `Z · ` des axes du Core ; préfixes `A` / `B` des slots Z ; `GAIN` (étages) face à `OUT GAIN` ; `BALANCE` avec la mention `PRE-CHAIN` ; `FM INDEX` / `FM FDBK` (libellés contextuels non arbitrés, OD-08).

## 4. Détail par groupe

### Distinctions critiques

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `out.gain` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | Output Gain | **OUT GAIN** | Output Gain | 45.37 | 46.29 | 92.00 |  |
| `cross.master` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | Z  Master | **CROSS LVL** | CROSS Master (CROSS level) | 48.71 | 50.97 | 70.00 |  |
| `cross.fwd` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | X  WT>FM | **WT→FM** | CROSS WT>FM (forward) | 36.29 | 34.49 | 80.00 |  |
| `cross.rev` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | Y  FM>WT | **FM→WT** | CROSS FM>WT (reverse) | 36.29 | 34.49 | 80.00 |  |
| `wt.z` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | Z Master | **WT Z** | WT Z Master (Z transform depth) | 22.50 | 23.91 | 80.00 |  |
| `cross.audio.mode` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Cross Audio Mode | **XAUDIO MODE** | Cross Audio Mode (post-chain) | 66.02 | 67.51 | 164.00 |  |
| `cross.audio.amount` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Cross Audio Amount | **XAUDIO AMT** | Cross Audio Amount (post-chain) | 59.05 | 59.45 | 80.00 |  |
| `out.limiter` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS | Musical Limiter | **LIMITER** | Musical Limiter | 35.28 | 37.88 | 99.00 |  |
| `out.ceiling` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; out.limiter=On | Limiter Ceiling | **CEILING** | Limiter Ceiling | 35.54 | 38.49 | 80.00 |  |
| `perf.balance` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | WT ‹ › FM | **BALANCE** | WT/FM Balance (audio, pre-chain) | 41.66 | 42.43 | 129.88 |  |
| `wt.z` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | Z  Master | **WT Z** | WT Z Master (Z transform depth) | 27.25 | 29.25 | 128.00 | critique (12 px) |
| `wt.z` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 | Z Master | **WT Z** | WT Z Master (Z transform depth) | 22.50 | 23.91 | 80.00 |  |
| `cross.master` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | Z  Master | **Z · CROSS LVL** | CROSS Master (CROSS level) | 68.96 | 72.91 | 70.00 | macos: OK_MARGE<8; windows: 2_LIGNES; critique (12 px) |
| `cross.master` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | Z  Master | **CROSS LVL** | CROSS Master (CROSS level) | 53.13 | 55.61 | 80.00 | critique (12 px) |
| `cross.master` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 | Master | **CROSS LVL** | CROSS Master (CROSS level) | 48.71 | 50.97 | 80.00 |  |
| `cross.fwd` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | X  WT>FM | **X · WT→FM** | CROSS WT>FM (forward) | 55.64 | 54.93 | 80.00 | critique (12 px) |
| `cross.fwd` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 | WT>FM | **WT→FM** | CROSS WT>FM (forward) | 36.29 | 34.49 | 80.00 |  |
| `cross.rev` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | Y  FM>WT | **Y · FM→WT** | CROSS FM>WT (reverse) | 55.61 | 54.24 | 80.00 | critique (12 px) |
| `cross.rev` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 | FM>WT | **FM→WT** | CROSS FM>WT (reverse) | 36.29 | 34.49 | 80.00 |  |
| `perf.balance` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | WT ‹ › FM | **BALANCE** | WT/FM Balance (audio, pre-chain) | 41.66 | 42.43 | 129.88 |  |
| `out.gain` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | Output Gain | **OUT GAIN** | Output Gain | 45.37 | 46.29 | 92.00 |  |
| `cross.audio.mode` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | Cross Audio Mode | **XAUDIO MODE** | Cross Audio Mode (post-chain) | 66.02 | 67.51 | 164.00 |  |
| `cross.audio.mode` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT CARACTÈRE \| CROSS BUS / AUDIO \| FM MATRICE | Cross Audio Mode | **XAUDIO MODE** | Cross Audio Mode (post-chain) | 66.02 | 67.51 | 164.00 |  |
| `cross.audio.mode` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT CARACTÈRE \| CROSS BUS / AUDIO \| FM COMMUN | Cross Audio Mode | **XAUDIO MODE** | Cross Audio Mode (post-chain) | 66.02 | 67.51 | 164.00 |  |
| `cross.audio.amount` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | Cross Audio Amount | **XAUDIO AMT** | Cross Audio Amount (post-chain) | 59.05 | 59.45 | 80.00 |  |
| `cross.audio.amount` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT CARACTÈRE \| CROSS BUS / AUDIO \| FM MATRICE | Cross Audio Amount | **XAUDIO AMT** | Cross Audio Amount (post-chain) | 59.05 | 59.45 | 80.00 |  |
| `cross.audio.amount` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT CARACTÈRE \| CROSS BUS / AUDIO \| FM COMMUN | Cross Audio Amount | **XAUDIO AMT** | Cross Audio Amount (post-chain) | 59.05 | 59.45 | 80.00 |  |
| `out.limiter` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Musical Limiter | **LIMITER** | Musical Limiter | 35.28 | 37.88 | 99.00 |  |
| `out.limiter` | Variante d'emplacement | SOUND DESIGN / MODULATION / ENV AMP \| LFO1 | Musical Limiter | **LIMITER** | Musical Limiter | 35.28 | 37.88 | 99.00 |  |
| `out.ceiling` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Limiter Ceiling | **CEILING** | Limiter Ceiling | 35.54 | 38.49 | 80.00 |  |
| `cross.audio.mode#0` | Option de liste (liste) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Mix | **MIX** | Mix (indice 0 inchangé) | 16.58 | 17.64 | 136 |  |
| `cross.audio.mode#1` | Option de liste (liste) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Crossfade | **XFADE** | Crossfade (indice 1 inchangé) | 29.36 | 29.84 | 136 |  |
| `cross.audio.mode#2` | Option de liste (liste) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Ring | **RING** | Ring (indice 2 inchangé) | 22.25 | 23.89 | 136 |  |
| `cross.audio.mode#3` | Option de liste (liste) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | AM | **AM** | AM (indice 3 inchangé) | 14.49 | 13.82 | 136 |  |
| `bar.peak` | Hors registre | Barre globale | OUT | **PEAK** | Output Peak (dBFS, block peak at plugin output) | 27.10 | 29.26 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `units.peak` | Unité | Valeurs | dBFS | **dBFS** | Peak level | 21.29 | 23.23 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |

### Autonomie : ARM, DRONE, AUTHORIZE, états

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `flt.wt.arm` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | Autonomous Armed | **ARM** | FLT WT Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `flt.fm.arm` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | Autonomous Armed | **ARM** | FLT FM Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `flt.gl.arm` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | Autonomous Armed | **ARM** | FLT GLOBAL Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.wt1.arm` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N2 Feedback Waveshaper | Autonomous Armed | **ARM** | DST WT1 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.wt2.arm` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N2 Feedback Waveshaper | Autonomous Armed | **ARM** | DST WT2 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.fm1.arm` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N2 Feedback Waveshaper | Autonomous Armed | **ARM** | DST FM1 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.fm2.arm` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N2 Feedback Waveshaper | Autonomous Armed | **ARM** | DST FM2 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.gl1.arm` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N2 Feedback Waveshaper | Autonomous Armed | **ARM** | DST GL1 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.gl2.arm` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N2 Feedback Waveshaper | Autonomous Armed | **ARM** | DST GL2 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `flt.wt.arm` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Autonomous Armed | **ARM** | FLT WT Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `flt.fm.arm` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Autonomous Armed | **ARM** | FLT FM Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `flt.gl.arm` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Autonomous Armed | **ARM** | FLT GLOBAL Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.wt1.arm` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Autonomous Armed | **ARM** | DST WT1 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.wt2.arm` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Autonomous Armed | **ARM** | DST WT2 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.fm1.arm` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Autonomous Armed | **ARM** | DST FM1 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.fm2.arm` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Autonomous Armed | **ARM** | DST FM2 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.gl1.arm` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Autonomous Armed | **ARM** | DST GL1 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `dst.gl2.arm` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Autonomous Armed | **ARM** | DST GL2 Autonomous Armed | 20.39 | 20.11 | 139.00 |  |
| `bar.authorize` | Hors registre | Barre globale | Autoriser | **AUTHORIZE** | Authorize autonomous states (this instance) | 61.47 | 65.77 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `bar.drone` | Hors registre | Barre globale | Drone | **DRONE** | Drone (hold gate) | 37.45 | 39.42 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `perf.auth.armed` | Hors registre | Performance / Autonomy | Armements | **ARMED** | Armed modules (configured) | 37.98 | 39.69 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `perf.auth.exec` | Hors registre | Performance / Autonomy | Exécution | **AUTONOMY** | Autonomous execution (authorized / suspended / revoked / render) | 62.89 | 64.19 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `eng.state.drone` | Hors registre | Engineering / State | Restaurer la configuration du Drone à l'ouverture du projet | **RESTORE DRONE** | Restore Drone configuration on project load (never its execution) | 80.98 | 86.05 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `state.off` | État | États NEURAL CELL | OFF | **OFF** | Off | 18.42 | 19.47 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `state.on` | État | États NEURAL CELL | ON | **ON** | On | 14.87 | 15.36 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `state.armed` | État | États NEURAL CELL | ARMÉ | **ARMED** | Armed, execution suspended (authorization required) | 37.98 | 39.69 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `state.active` | État | États NEURAL CELL | ACTIF | **ACTIVE** | Armed and authorized | 38.41 | 41.22 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `state.render` | État | États autonomie | hors temps réel (transport) | **RENDER** | Temporary render authorization (0.4.1) | 42.21 | 45.49 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `state.configured` | État | États autonomie | CONFIGURÉ · SUSPENDU | **SUSPENDED** | Drone configured, execution suspended (authorization required) | 61.62 | 67.53 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `state.suspended` | État | États autonomie | suspendue | **SUSPENDED** | Autonomous execution suspended | 61.62 | 67.53 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `state.authorized` | État | États autonomie | autorisée / accordée | **AUTHORIZED** | Autonomous execution authorized | 69.55 | 73.96 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `state.revoked` | État | États autonomie | révoquée (Panic) | **REVOKED** | Revoked by Panic (latched) | 50.09 | 52.96 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |

### Gel et capture

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `flt.wt.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F4 Modal | Modal Freeze | **FREEZE** | FLT WT Modal Freeze | 31.75 | 34.86 | 99.00 |  |
| `flt.fm.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F4 Modal | Modal Freeze | **FREEZE** | FLT FM Modal Freeze | 31.75 | 34.86 | 99.00 |  |
| `flt.gl.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F4 Modal | Modal Freeze | **FREEZE** | FLT GLOBAL Modal Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.wt1.d2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | Freeze | **FREEZE** | DST WT1 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.wt1.s2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S2 Spectral Freeze | Freeze | **FREEZE** | DST WT1 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.wt2.d2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | Freeze | **FREEZE** | DST WT2 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.wt2.s2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S2 Spectral Freeze | Freeze | **FREEZE** | DST WT2 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.fm1.d2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | Freeze | **FREEZE** | DST FM1 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.fm1.s2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S2 Spectral Freeze | Freeze | **FREEZE** | DST FM1 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.fm2.d2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | Freeze | **FREEZE** | DST FM2 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.fm2.s2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S2 Spectral Freeze | Freeze | **FREEZE** | DST FM2 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.gl1.d2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | Freeze | **FREEZE** | DST GL1 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.gl1.s2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S2 Spectral Freeze | Freeze | **FREEZE** | DST GL1 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.gl2.d2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | Freeze | **FREEZE** | DST GL2 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.gl2.s2.freeze` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S2 Spectral Freeze | Freeze | **FREEZE** | DST GL2 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `flt.wt.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Modal Freeze | **FREEZE** | FLT WT Modal Freeze | 31.75 | 34.86 | 99.00 |  |
| `flt.fm.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Modal Freeze | **FREEZE** | FLT FM Modal Freeze | 31.75 | 34.86 | 99.00 |  |
| `flt.gl.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Modal Freeze | **FREEZE** | FLT GLOBAL Modal Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.wt1.d2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST WT1 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.wt1.s2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST WT1 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.wt2.d2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST WT2 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.wt2.s2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST WT2 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.fm1.d2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST FM1 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.fm1.s2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST FM1 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.fm2.d2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST FM2 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.fm2.s2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST FM2 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.gl1.d2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST GL1 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.gl1.s2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST GL1 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.gl2.d2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST GL2 D2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `dst.gl2.s2.freeze` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Freeze | **FREEZE** | DST GL2 S2 Freeze | 31.75 | 34.86 | 99.00 |  |
| `sound.chain.capture` | Hors registre | Sound Design / Process | Capturer gels | **CAPTURE** | Capture frozen D2/S2 content (saved with preset/project) | 44.63 | 46.99 | 149 |  |
| `eng.captures.now` | Hors registre | Engineering / State | Capturer maintenant | **CAPTURE** | Capture frozen D2/S2 content now | 44.63 | 46.99 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |

### Modes de feedback

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `fm.fb_mode` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM COMMUN | Feedback Reset | **FB RESET** | FM Feedback Reset | 40.30 | 44.09 | 164.00 |  |
| `dst.wt1.d2.fb_mode` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | Feedback Mode | **FB MODE** | DST WT1 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.wt2.d2.fb_mode` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | Feedback Mode | **FB MODE** | DST WT2 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.fm1.d2.fb_mode` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | Feedback Mode | **FB MODE** | DST FM1 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.fm2.d2.fb_mode` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | Feedback Mode | **FB MODE** | DST FM2 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.gl1.d2.fb_mode` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | Feedback Mode | **FB MODE** | DST GL1 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.gl2.d2.fb_mode` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | Feedback Mode | **FB MODE** | DST GL2 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `fm.fb_mode` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT CARACTÈRE \| CROSS BUS / AUDIO \| FM COMMUN | Feedback Reset | **FB RESET** | FM Feedback Reset | 40.30 | 44.09 | 164.00 |  |
| `dst.wt1.d2.fb_mode` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Feedback Mode | **FB MODE** | DST WT1 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.wt2.d2.fb_mode` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Feedback Mode | **FB MODE** | DST WT2 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.fm1.d2.fb_mode` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Feedback Mode | **FB MODE** | DST FM1 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.fm2.d2.fb_mode` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Feedback Mode | **FB MODE** | DST FM2 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.gl1.d2.fb_mode` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Feedback Mode | **FB MODE** | DST GL1 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |
| `dst.gl2.d2.fb_mode` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Feedback Mode | **FB MODE** | DST GL2 D2 Feedback Mode | 40.69 | 42.04 | 164.00 |  |

### Position WT et transformations Z

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `wt.x` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | X | **X** | WT X | 5.85 | 6.20 | 80.00 |  |
| `wt.y` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | Y | **Y** | WT Y | 5.82 | 5.57 | 80.00 |  |
| `wt.za.type` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | ZA Transform | **A TYPE** | WT ZA Transform | 31.35 | 31.70 | 164.00 |  |
| `wt.za.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.za.type=Harmonic Fold | ZA Depth | **A DEPTH** | WT ZA Depth | 39.39 | 39.90 | 80.00 |  |
| `wt.za.curve` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.za.type=Harmonic Fold | ZA Curve | **A CURVE** | WT ZA Curve | 39.41 | 40.52 | 164.00 |  |
| `wt.zb.type` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | ZB Transform | **B TYPE** | WT ZB Transform | 30.28 | 31.71 | 164.00 |  |
| `wt.zb.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.zb.type=Harmonic Fold | ZB Depth | **B DEPTH** | WT ZB Depth | 38.32 | 39.92 | 80.00 |  |
| `wt.zb.curve` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.zb.type=Harmonic Fold | ZB Curve | **B CURVE** | WT ZB Curve | 38.34 | 40.54 | 164.00 |  |
| `wt.z_order` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Z Order | **ORDER** | WT Z Order | 31.13 | 32.41 | 164.00 |  |
| `wt.z_fund` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Z Keep Fundamental | **KEEP F0** | WT Z Keep Fundamental | 35.25 | 37.31 | 99.00 |  |
| `wt.z_gain` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Z Gain Compensation | **GAIN COMP** | WT Z Gain Compensation | 52.66 | 53.50 | 99.00 |  |
| `wt.za.shape` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.za.type=Harmonic Fold | ZA Shape | **A SHAPE** | WT ZA Shape | 37.96 | 39.73 | 80.00 |  |
| `wt.zb.shape` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.zb.type=Harmonic Fold | ZB Shape | **B SHAPE** | WT ZB Shape | 36.89 | 39.74 | 80.00 |  |
| `wt.x` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | X  position | **X** | WT X | 7.08 | 7.85 | 128.00 | critique (12 px) |
| `wt.x` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 | X | **X** | WT X | 5.85 | 6.20 | 80.00 |  |
| `wt.y` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | Y  position | **Y** | WT Y | 7.29 | 7.58 | 128.00 | critique (12 px) |
| `wt.y` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 | Y | **Y** | WT Y | 5.82 | 5.57 | 80.00 |  |
| `wt.za.type` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | ZA Transform | **A TYPE** | WT ZA Transform | 31.35 | 31.70 | 164.00 |  |
| `wt.zb.type` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | ZB Transform | **B TYPE** | WT ZB Transform | 30.28 | 31.71 | 164.00 |  |
| `wt.z_order` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | Z Order | **ORDER** | WT Z Order | 31.13 | 32.41 | 164.00 |  |
| `wt.z_fund` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | Z Keep Fundamental | **KEEP F0** | WT Z Keep Fundamental | 35.25 | 37.31 | 99.00 |  |
| `wt.z_gain` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | Z Gain Compensation | **GAIN COMP** | WT Z Gain Compensation | 52.66 | 53.50 | 99.00 |  |
| `wt.za.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | ZA Depth | **A DEPTH** | WT ZA Depth | 39.39 | 39.90 | 80.00 |  |
| `wt.za.curve` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | ZA Curve | **A CURVE** | WT ZA Curve | 39.41 | 40.52 | 164.00 |  |
| `wt.za.shape` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | ZA Shape | **A SHAPE** | WT ZA Shape | 37.96 | 39.73 | 80.00 |  |
| `wt.zb.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | ZB Depth | **B DEPTH** | WT ZB Depth | 38.32 | 39.92 | 80.00 |  |
| `wt.zb.curve` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | ZB Curve | **B CURVE** | WT ZB Curve | 38.34 | 40.54 | 164.00 |  |
| `wt.zb.shape` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | ZB Shape | **B SHAPE** | WT ZB Shape | 36.89 | 39.74 | 80.00 |  |
| `wt.z_order#0` | Option de liste (segments) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | A>B | **A>B** | A>B (indice 0 inchangé) | 19.24 | 19.80 | 76 |  |
| `wt.z_order#1` | Option de liste (segments) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | B>A | **B>A** | B>A (indice 1 inchangé) | 19.24 | 19.80 | 76 |  |
| `sound.wtz.slot-a` | Titre | Sound Design / WT Z | Emplacement A | **SLOT A** | Z slot A | 34.86 | 36.18 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `sound.wtz.slot-b` | Titre | Sound Design / WT Z | Emplacement B | **SLOT B** | Z slot B | 33.86 | 36.05 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `sound.wtz.order` | Hors registre | Sound Design / WT Z | ORDRE EFFECTIF DES TRANSFORMATIONS Z | **EFFECTIVE Z ORDER** | Effective Z order (engine) | 96.04 | 102.89 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |

### Macros

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `macro1` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | M1 | **M1** | Macro 1 | 13.95 | 13.38 | 96.00 |  |
| `macro2` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | M2 | **M2** | Macro 2 | 13.95 | 13.38 | 96.00 |  |
| `macro3` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | M3 | **M3** | Macro 3 | 13.95 | 13.38 | 96.00 |  |
| `macro4` | Paramètre (canonique) | PERFORMANCE / PERFORMANCE | M4 | **M4** | Macro 4 | 13.95 | 13.38 | 96.00 |  |
| `macro1` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | M1 | **M1** | Macro 1 | 15.22 | 14.60 | 96.00 | critique (12 px) |
| `macro2` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | M2 | **M2** | Macro 2 | 15.22 | 14.60 | 96.00 | critique (12 px) |
| `macro3` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | M3 | **M3** | Macro 3 | 15.22 | 14.60 | 96.00 | critique (12 px) |
| `macro4` | Variante d'emplacement | PERFORMANCE / PERFORMANCE | M4 | **M4** | Macro 4 | 15.22 | 14.60 | 96.00 | critique (12 px) |

### Core et PERFORMANCE

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `perf.core.status` | Hors registre | Performance / Core | RENDU STATIQUE PROVISOIRE · illustratif · moteur 3D : ARCH-UI-05 | **STATIC PREVIEW** | Static preview (illustrative, not measured) | 79.39 | 88.53 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `perf.core.x-axis` | Hors registre | Performance / Core | X  cross.fwd  WT>FM | **X · WT→FM** | X axis: CROSS WT>FM (cross.fwd) | 61.55 | 59.29 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `perf.core.y-axis` | Hors registre | Performance / Core | Y  cross.rev  FM>WT | **Y · FM→WT** | Y axis: CROSS FM>WT (cross.rev) | 61.75 | 59.03 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |
| `perf.balance.note` | Hors registre | Performance / Balance | Balance audio post-Amp… | **PRE-CHAIN** | Audio balance before WT/FM chains | 54.84 | 58.00 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |

### Barre globale

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `bar.panic` | Hors registre | Barre globale | PANIC | **PANIC** | Panic (All Sound Off, revoke) | 32.22 | 35.80 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE; critique (12 px) |

### ENGINEERING : groupes, vues, ports

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `eng.group.network` | Titre | Engineering / Groupes (DA-A10) | FM NETWORK · CROSS NETWORK | **NETWORK** | Network: FM, CROSS | 48.44 | 51.98 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `eng.group.modulation` | Titre | Engineering / Groupes (DA-A10) | MODULATION · MIDI LEARN | **MODULATION** | Modulation: Mod Matrix, MIDI Learn | 67.17 | 70.40 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `eng.group.banklab` | Titre | Engineering / Groupes (DA-A10) | BANK LAB | **BANK LAB** | Bank Lab | 47.84 | 51.34 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `eng.group.state` | Titre | Engineering / Groupes (DA-A10) | VOICE / TUNING · PRESETS / STATE · CAPTURES / ASSETS | **STATE** | State: Voice/Tuning, Presets, Captures/Assets | 30.71 | 31.98 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `eng.group.tools` | Titre | Engineering / Groupes (DA-A10) | RANDOMIZE · DIAGNOSTICS | **TOOLS** | Tools: Randomize, Diagnostics | 32.59 | 33.76 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `eng.view.neural` | Titre | Engineering / Réseaux | — | **NEURAL** | Neural network view | 38.42 | 41.07 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `eng.view.table` | Titre | Engineering / Réseaux | — | **TABLE** | Table view | 30.25 | 32.04 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `eng.matrix.port` | Hors registre | Engineering / CROSS | › WT.FREQ | **WT FREQ** | Destination port WT frequency | 43.07 | 46.32 | 84 |  |

### Réseau CROSS et FM

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `cross.op1_wt.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP1>WT | **PM** | CROSS OP1>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.op2_wt.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP2>WT | **PM** | CROSS OP2>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.op3_wt.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP3>WT | **PM** | CROSS OP3>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.op4_wt.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP4>WT | **PM** | CROSS OP4>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.bus_wt.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | BUS>WT | **PM** | CROSS BUS>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.op1_wt_freq.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP1>WT Freq | **FREQ** | CROSS OP1>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.op1_wt_x.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP1>WT X | **X** | CROSS OP1>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.op1_wt_y.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP1>WT Y | **Y** | CROSS OP1>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.op1_wt_z.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP1>WT Z | **Z** | CROSS OP1>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.op1_wt_level.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP1>WT Level | **LVL** | CROSS OP1>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.op1_wt_char.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP1>WT Char | **CHAR** | CROSS OP1>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.op2_wt_freq.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP2>WT Freq | **FREQ** | CROSS OP2>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.op2_wt_x.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP2>WT X | **X** | CROSS OP2>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.op2_wt_y.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP2>WT Y | **Y** | CROSS OP2>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.op2_wt_z.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP2>WT Z | **Z** | CROSS OP2>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.op2_wt_level.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP2>WT Level | **LVL** | CROSS OP2>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.op2_wt_char.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP2>WT Char | **CHAR** | CROSS OP2>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.op3_wt_freq.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP3>WT Freq | **FREQ** | CROSS OP3>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.op3_wt_x.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP3>WT X | **X** | CROSS OP3>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.op3_wt_y.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP3>WT Y | **Y** | CROSS OP3>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.op3_wt_z.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP3>WT Z | **Z** | CROSS OP3>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.op3_wt_level.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP3>WT Level | **LVL** | CROSS OP3>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.op3_wt_char.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP3>WT Char | **CHAR** | CROSS OP3>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.op4_wt_freq.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP4>WT Freq | **FREQ** | CROSS OP4>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.op4_wt_x.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP4>WT X | **X** | CROSS OP4>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.op4_wt_y.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP4>WT Y | **Y** | CROSS OP4>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.op4_wt_z.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP4>WT Z | **Z** | CROSS OP4>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.op4_wt_level.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP4>WT Level | **LVL** | CROSS OP4>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.op4_wt_char.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | OP4>WT Char | **CHAR** | CROSS OP4>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.bus_wt_freq.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | BUS>WT Freq | **FREQ** | CROSS BUS>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.bus_wt_x.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | BUS>WT X | **X** | CROSS BUS>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.bus_wt_y.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | BUS>WT Y | **Y** | CROSS BUS>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.bus_wt_z.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | BUS>WT Z | **Z** | CROSS BUS>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.bus_wt_level.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | BUS>WT Level | **LVL** | CROSS BUS>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.bus_wt_char.depth` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | BUS>WT Char | **CHAR** | CROSS BUS>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.dir` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | Direction | **DIR** | CROSS Direction | 15.59 | 17.08 | 164.00 |  |
| `cross.audio.am_dir` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; cross.audio.mode=AM | Cross AM Direction | **AM DIR** | Cross AM Direction | 33.04 | 34.07 | 164.00 |  |
| `cross.dir` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 | Direction | **DIR** | CROSS Direction | 15.59 | 17.08 | 164.00 |  |
| `cross.op1_wt.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP1>WT | **PM** | CROSS OP1>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.op1_wt.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP1›WT | **DEPTH** | CROSS OP1>WT | 27.26 | 27.76 | 80.00 |  |
| `cross.op1_wt_freq.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP1>WT Freq | **FREQ** | CROSS OP1>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.op1_wt_freq.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP1›WT.FREQ | **DEPTH** | CROSS OP1>WT Freq | 27.26 | 27.76 | 80.00 |  |
| `cross.op1_wt_x.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP1>WT X | **X** | CROSS OP1>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.op1_wt_x.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP1›WT.X | **DEPTH** | CROSS OP1>WT X | 27.26 | 27.76 | 80.00 |  |
| `cross.op1_wt_y.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP1>WT Y | **Y** | CROSS OP1>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.op1_wt_y.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP1›WT.Y | **DEPTH** | CROSS OP1>WT Y | 27.26 | 27.76 | 80.00 |  |
| `cross.op1_wt_z.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP1>WT Z | **Z** | CROSS OP1>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.op1_wt_z.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP1›WT.Z | **DEPTH** | CROSS OP1>WT Z | 27.26 | 27.76 | 80.00 |  |
| `cross.op1_wt_level.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP1>WT Level | **LVL** | CROSS OP1>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.op1_wt_level.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP1›WT.LEVEL | **DEPTH** | CROSS OP1>WT Level | 27.26 | 27.76 | 80.00 |  |
| `cross.op1_wt_char.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP1>WT Char | **CHAR** | CROSS OP1>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.op1_wt_char.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP1›WT.CHAR | **DEPTH** | CROSS OP1>WT Char | 27.26 | 27.76 | 80.00 |  |
| `cross.op2_wt.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP2>WT | **PM** | CROSS OP2>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.op2_wt.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP2›WT | **DEPTH** | CROSS OP2>WT | 27.26 | 27.76 | 80.00 |  |
| `cross.op2_wt_freq.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP2>WT Freq | **FREQ** | CROSS OP2>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.op2_wt_freq.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP2›WT.FREQ | **DEPTH** | CROSS OP2>WT Freq | 27.26 | 27.76 | 80.00 |  |
| `cross.op2_wt_x.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP2>WT X | **X** | CROSS OP2>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.op2_wt_x.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP2›WT.X | **DEPTH** | CROSS OP2>WT X | 27.26 | 27.76 | 80.00 |  |
| `cross.op2_wt_y.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP2>WT Y | **Y** | CROSS OP2>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.op2_wt_y.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP2›WT.Y | **DEPTH** | CROSS OP2>WT Y | 27.26 | 27.76 | 80.00 |  |
| `cross.op2_wt_z.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP2>WT Z | **Z** | CROSS OP2>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.op2_wt_z.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP2›WT.Z | **DEPTH** | CROSS OP2>WT Z | 27.26 | 27.76 | 80.00 |  |
| `cross.op2_wt_level.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP2>WT Level | **LVL** | CROSS OP2>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.op2_wt_level.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP2›WT.LEVEL | **DEPTH** | CROSS OP2>WT Level | 27.26 | 27.76 | 80.00 |  |
| `cross.op2_wt_char.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP2>WT Char | **CHAR** | CROSS OP2>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.op2_wt_char.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP2›WT.CHAR | **DEPTH** | CROSS OP2>WT Char | 27.26 | 27.76 | 80.00 |  |
| `cross.op3_wt.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP3>WT | **PM** | CROSS OP3>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.op3_wt.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP3›WT | **DEPTH** | CROSS OP3>WT | 27.26 | 27.76 | 80.00 |  |
| `cross.op3_wt_freq.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP3>WT Freq | **FREQ** | CROSS OP3>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.op3_wt_freq.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP3›WT.FREQ | **DEPTH** | CROSS OP3>WT Freq | 27.26 | 27.76 | 80.00 |  |
| `cross.op3_wt_x.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP3>WT X | **X** | CROSS OP3>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.op3_wt_x.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP3›WT.X | **DEPTH** | CROSS OP3>WT X | 27.26 | 27.76 | 80.00 |  |
| `cross.op3_wt_y.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP3>WT Y | **Y** | CROSS OP3>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.op3_wt_y.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP3›WT.Y | **DEPTH** | CROSS OP3>WT Y | 27.26 | 27.76 | 80.00 |  |
| `cross.op3_wt_z.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP3>WT Z | **Z** | CROSS OP3>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.op3_wt_z.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP3›WT.Z | **DEPTH** | CROSS OP3>WT Z | 27.26 | 27.76 | 80.00 |  |
| `cross.op3_wt_level.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP3>WT Level | **LVL** | CROSS OP3>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.op3_wt_level.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP3›WT.LEVEL | **DEPTH** | CROSS OP3>WT Level | 27.26 | 27.76 | 80.00 |  |
| `cross.op3_wt_char.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP3>WT Char | **CHAR** | CROSS OP3>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.op3_wt_char.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP3›WT.CHAR | **DEPTH** | CROSS OP3>WT Char | 27.26 | 27.76 | 80.00 |  |
| `cross.op4_wt.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP4>WT | **PM** | CROSS OP4>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.op4_wt.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP4›WT | **DEPTH** | CROSS OP4>WT | 27.26 | 27.76 | 80.00 |  |
| `cross.op4_wt_freq.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP4>WT Freq | **FREQ** | CROSS OP4>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.op4_wt_freq.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP4›WT.FREQ | **DEPTH** | CROSS OP4>WT Freq | 27.26 | 27.76 | 80.00 |  |
| `cross.op4_wt_x.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP4>WT X | **X** | CROSS OP4>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.op4_wt_x.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP4›WT.X | **DEPTH** | CROSS OP4>WT X | 27.26 | 27.76 | 80.00 |  |
| `cross.op4_wt_y.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP4>WT Y | **Y** | CROSS OP4>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.op4_wt_y.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP4›WT.Y | **DEPTH** | CROSS OP4>WT Y | 27.26 | 27.76 | 80.00 |  |
| `cross.op4_wt_z.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP4>WT Z | **Z** | CROSS OP4>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.op4_wt_z.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP4›WT.Z | **DEPTH** | CROSS OP4>WT Z | 27.26 | 27.76 | 80.00 |  |
| `cross.op4_wt_level.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP4>WT Level | **LVL** | CROSS OP4>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.op4_wt_level.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP4›WT.LEVEL | **DEPTH** | CROSS OP4>WT Level | 27.26 | 27.76 | 80.00 |  |
| `cross.op4_wt_char.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | OP4>WT Char | **CHAR** | CROSS OP4>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.op4_wt_char.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | OP4›WT.CHAR | **DEPTH** | CROSS OP4>WT Char | 27.26 | 27.76 | 80.00 |  |
| `cross.bus_wt.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | BUS>WT | **PM** | CROSS BUS>WT | 13.21 | 13.09 | 80.00 |  |
| `cross.bus_wt.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | BUS›WT | **DEPTH** | CROSS BUS>WT | 27.26 | 27.76 | 80.00 |  |
| `cross.bus_wt_freq.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | BUS>WT Freq | **FREQ** | CROSS BUS>WT Freq | 23.23 | 24.34 | 80.00 |  |
| `cross.bus_wt_freq.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | BUS›self | **DEPTH** | CROSS BUS>WT Freq | 27.26 | 27.76 | 80.00 |  |
| `cross.bus_wt_x.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | BUS>WT X | **X** | CROSS BUS>WT X | 5.85 | 6.20 | 80.00 |  |
| `cross.bus_wt_x.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | BUS›WT.X | **DEPTH** | CROSS BUS>WT X | 27.26 | 27.76 | 80.00 |  |
| `cross.bus_wt_y.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | BUS>WT Y | **Y** | CROSS BUS>WT Y | 5.82 | 5.57 | 80.00 |  |
| `cross.bus_wt_y.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | BUS›WT.Y | **DEPTH** | CROSS BUS>WT Y | 27.26 | 27.76 | 80.00 |  |
| `cross.bus_wt_z.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | BUS>WT Z | **Z** | CROSS BUS>WT Z | 5.65 | 6.20 | 80.00 |  |
| `cross.bus_wt_z.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | BUS›WT.Z | **DEPTH** | CROSS BUS>WT Z | 27.26 | 27.76 | 80.00 |  |
| `cross.bus_wt_level.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | BUS>WT Level | **LVL** | CROSS BUS>WT Level | 16.07 | 15.78 | 80.00 |  |
| `cross.bus_wt_level.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | BUS›WT.LEVEL | **DEPTH** | CROSS BUS>WT Level | 27.26 | 27.76 | 80.00 |  |
| `cross.bus_wt_char.depth` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | BUS>WT Char | **CHAR** | CROSS BUS>WT Char | 25.68 | 25.60 | 80.00 |  |
| `cross.bus_wt_char.depth` | Variante d'emplacement | ENGINEERING / CROSS NETWORK | BUS›WT.CHAR | **DEPTH** | CROSS BUS>WT Char | 27.26 | 27.76 | 80.00 |  |
| `cross.audio.am_dir` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | Cross AM Direction | **AM DIR** | Cross AM Direction | 33.04 | 34.07 | 164.00 |  |
| `cross.audio.am_dir` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT CARACTÈRE \| CROSS BUS / AUDIO \| FM MATRICE | Cross AM Direction | **AM DIR** | Cross AM Direction | 33.04 | 34.07 | 164.00 |  |
| `cross.audio.am_dir` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT CARACTÈRE \| CROSS BUS / AUDIO \| FM COMMUN | Cross AM Direction | **AM DIR** | Cross AM Direction | 33.04 | 34.07 | 164.00 |  |

### Autres P1 (priorité reprise de Codex)

| Référence | Occurrence | Emplacement | Actuel | Proposé | Nom complet | macos 100 % px | windows 100 % px | Budget | Signalement |
|---|---|---|---|---|---|---|---|---|---|
| `op1.mute` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | Mute | **MUTE** | OP1 Mute | 25.48 | 25.55 | 99.00 |  |
| `op1.enable` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | Enable | **ON** | OP1 Enable | 14.16 | 13.89 | 99.00 |  |
| `op2.mute` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | Mute | **MUTE** | OP2 Mute | 25.48 | 25.55 | 99.00 |  |
| `op2.enable` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | Enable | **ON** | OP2 Enable | 14.16 | 13.89 | 99.00 |  |
| `op3.mute` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | Mute | **MUTE** | OP3 Mute | 25.48 | 25.55 | 99.00 |  |
| `op3.enable` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | Enable | **ON** | OP3 Enable | 14.16 | 13.89 | 99.00 |  |
| `op4.mute` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Mute | **MUTE** | OP4 Mute | 25.48 | 25.55 | 99.00 |  |
| `op4.enable` | Paramètre (canonique) | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | Enable | **ON** | OP4 Enable | 14.16 | 13.89 | 99.00 |  |
| `flt.wt.pos` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | Position | **PRE/POST** | FLT WT Position | 44.38 | 45.93 | 164.00 |  |
| `flt.fm.pos` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | Position | **PRE/POST** | FLT FM Position | 44.38 | 45.93 | 164.00 |  |
| `flt.gl.pos` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | Position | **PRE/POST** | FLT GLOBAL Position | 44.38 | 45.93 | 164.00 |  |
| `dst.wt1.d2.speed` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | Playback | **SPEED** | DST WT1 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.wt2.d2.speed` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | Playback | **SPEED** | DST WT2 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.fm1.d2.speed` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | Playback | **SPEED** | DST FM1 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.fm2.d2.speed` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | Playback | **SPEED** | DST FM2 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.gl1.d2.speed` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | Playback | **SPEED** | DST GL1 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.gl2.d2.speed` | Paramètre (canonique) | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | Playback | **SPEED** | DST GL2 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `op1.mute` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 | Mute | **MUTE** | OP1 Mute | 25.48 | 25.55 | 99.00 |  |
| `op1.enable` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT XYZ \| CROSS DIRECT \| FM OP1 | Enable | **ON** | OP1 Enable | 14.16 | 13.89 | 99.00 |  |
| `op1.enable` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT LECTURE \| CROSS WT › FM \| FM OP2 | Enable | **ON** | OP1 Enable | 14.16 | 13.89 | 99.00 |  |
| `op2.mute` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT LECTURE \| CROSS WT › FM \| FM OP2 | Mute | **MUTE** | OP2 Mute | 25.48 | 25.55 | 99.00 |  |
| `op2.enable` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT LECTURE \| CROSS WT › FM \| FM OP2 | Enable | **ON** | OP2 Enable | 14.16 | 13.89 | 99.00 |  |
| `op2.enable` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | Enable | **ON** | OP2 Enable | 14.16 | 13.89 | 99.00 |  |
| `op3.mute` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | Mute | **MUTE** | OP3 Mute | 25.48 | 25.55 | 99.00 |  |
| `op3.enable` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT PHASE / SYNC \| CROSS FM › WT \| FM OP3 | Enable | **ON** | OP3 Enable | 14.16 | 13.89 | 99.00 |  |
| `op3.enable` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | Enable | **ON** | OP3 Enable | 14.16 | 13.89 | 99.00 |  |
| `op4.mute` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | Mute | **MUTE** | OP4 Mute | 25.48 | 25.55 | 99.00 |  |
| `op4.enable` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT Z \| CROSS BUS / AUDIO \| FM OP4 | Enable | **ON** | OP4 Enable | 14.16 | 13.89 | 99.00 |  |
| `op4.enable` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT CARACTÈRE \| CROSS BUS / AUDIO \| FM MATRICE | Enable | **ON** | OP4 Enable | 14.16 | 13.89 | 99.00 |  |
| `op4.enable` | Variante d'emplacement | SOUND DESIGN / MOTEURS / WT CARACTÈRE \| CROSS BUS / AUDIO \| FM COMMUN | Enable | **ON** | OP4 Enable | 14.16 | 13.89 | 99.00 |  |
| `op4.enable` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Enable | **ON** | OP4 Enable | 14.16 | 13.89 | 99.00 |  |
| `flt.wt.pos` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Position | **PRE/POST** | FLT WT Position | 44.38 | 45.93 | 164.00 |  |
| `flt.fm.pos` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Position | **PRE/POST** | FLT FM Position | 44.38 | 45.93 | 164.00 |  |
| `flt.gl.pos` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Position | **PRE/POST** | FLT GLOBAL Position | 44.38 | 45.93 | 164.00 |  |
| `dst.wt1.d2.speed` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Playback | **SPEED** | DST WT1 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.wt2.d2.speed` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Playback | **SPEED** | DST WT2 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.fm1.d2.speed` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Playback | **SPEED** | DST FM1 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.fm2.d2.speed` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Playback | **SPEED** | DST FM2 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.gl1.d2.speed` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Playback | **SPEED** | DST GL1 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `dst.gl2.d2.speed` | Variante d'emplacement | SOUND DESIGN / TRAITEMENTS | Playback | **SPEED** | DST GL2 D2 Playback | 27.31 | 30.07 | 80.00 |  |
| `sound.cross.tab.forward` | Titre | Sound Design / CROSS | WT › FM | **WT→FM** | CROSS forward routes | 38.07 | 37.98 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `sound.cross.tab.reverse` | Titre | Sound Design / CROSS | FM › WT | **FM→WT** | CROSS reverse routes | 38.07 | 37.98 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `inspector.base` | Champ | Inspector | Base | **BASE** | Base value | 21.91 | 24.26 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |
| `inspector.effective` | Champ | Inspector | Effective | **EFFECTIVE** | Effective value (engine, decimated) | 46.37 | 49.47 | — | macos: BUDGET_NON_MESURE; windows: BUDGET_NON_MESURE |

