# ARCH-UI-04-R2-B1-R1 — validation de la nomenclature P1 (LAB-A08)

Fichier généré par `lab/tools/build_validation_r1.py`. Traçabilité individuelle des occurrences P1 :
[P1-TRACEABILITY.csv](P1-TRACEABILITY.csv). Nomenclature révisée : [`../R2-CONTROL-LABELS-R1.csv`](../R2-CONTROL-LABELS-R1.csv)
(générée par `lab/tools/gen_labels_r1.py`). La révision R2-A reste intacte et se régénère à l'identique.

## 1. Deux statuts distincts

| Statut | Portée | Source |
|---|---|---|
| **Lexical** | 324 occurrences P1 : **validées** par le propriétaire (LAB-A08, option B), y compris 1 occurrence ajoutée par LAB-A08 (en-tête CROSS AUDIO) | arbitrage LAB-A08 |
| **Géométrique** | mesure JUCE réelle dans le gabarit (PT-02) ; un budget indéterminé n'est jamais déclaré PASS | `TYPOGRAPHY-MEASUREMENTS.csv` |

P2 et P3 restent soumis à la validation progressive OD-02. Les 703 StableID, noms hôtes, titres accessibles, indices de
choix, plages et unités sont inchangés. LAB-A08 n'arbitre pas la restriction inter-domaines des transformations Z.

## 2. Statut géométrique des occurrences P1

| Statut | macos | windows |
|---|---|---|
| INDÉTERMINÉ (gabarit futur) | 37 | 37 |
| PASS | 286 | 286 |
| PASS (marge < 8 px) | 1 | 0 |
| PASS en deux lignes | 0 | 1 |

## 3. Modifications LAB-A08

| Référence | Emplacement | R2-A | R1 | Nom complet | macos (px / budget) | windows (px / budget) |
|---|---|---|---|---|---|---|
| `wt.z_fund` | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | KEEP F0 | **KEEP FUND** | WT Z Keep Fundamental | 49.71 / 99.00 (PASS) | 51.93 / 99.00 (PASS) |
| `cross.audio.mode` | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | XAUDIO MODE | **MODE** | Cross Audio Mode (post-chain) | 27.36 / 164.00 (PASS) | 27.45 / 164.00 (PASS) |
| `cross.audio.amount` | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | XAUDIO AMT | **AMOUNT** | Cross Audio Amount (post-chain) | 41.02 / 80.00 (PASS) | 39.91 / 80.00 (PASS) |
| `wt.z_fund` | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | KEEP F0 | **KEEP FUND** | WT Z Keep Fundamental | 49.71 / 99.00 (PASS) | 51.93 / 99.00 (PASS) |
| `cross.audio.mode` | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | XAUDIO MODE | **MODE** | Cross Audio Mode (post-chain) | 27.36 / 164.00 (PASS) | 27.45 / 164.00 (PASS) |
| `cross.audio.mode` | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | XAUDIO MODE | **MODE** | Cross Audio Mode (post-chain) | 27.36 / 164.00 (PASS) | 27.45 / 164.00 (PASS) |
| `cross.audio.mode` | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM COMMUN | XAUDIO MODE | **MODE** | Cross Audio Mode (post-chain) | 27.36 / 164.00 (PASS) | 27.45 / 164.00 (PASS) |
| `cross.audio.amount` | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | XAUDIO AMT | **AMOUNT** | Cross Audio Amount (post-chain) | 41.02 / 80.00 (PASS) | 39.91 / 80.00 (PASS) |
| `cross.audio.amount` | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | XAUDIO AMT | **AMOUNT** | Cross Audio Amount (post-chain) | 41.02 / 80.00 (PASS) | 39.91 / 80.00 (PASS) |
| `cross.audio.amount` | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM COMMUN | XAUDIO AMT | **AMOUNT** | Cross Audio Amount (post-chain) | 41.02 / 80.00 (PASS) | 39.91 / 80.00 (PASS) |
| `sound.cross.tab.bus` | Sound Design / CROSS | BUS / XAUDIO | **BUS / CROSS AUDIO** | Bus and Cross Audio | 96.02 / ? (INDÉTERMINÉ (gabarit futur)) | 104.90 / ? (INDÉTERMINÉ (gabarit futur)) |
| `sound.cross.section.audio` | Sound Design / CROSS / BUS / AUDIO | — | **CROSS AUDIO** | Cross Audio section (post-chain) | 66.48 / ? (INDÉTERMINÉ (gabarit futur)) | 71.80 / ? (INDÉTERMINÉ (gabarit futur)) |

AMOUNT et MODE n'apparaissent que dans la section CROSS AUDIO (onglet BUS / CROSS AUDIO de SOUND DESIGN). Le recalcul ne trouve
aucune nouvelle homonymie dans leurs emplacements. XAUDIO n'est plus utilisé nulle part. Le gain général reste OUT GAIN ; GAIN
désigne le gain de sortie d'un étage, dans sa plaque.

## 4. Contrôles exigés (brief §12.4)

**Homonymies** (même libellé, StableID différents, même emplacement mesuré) : **16 groupes** (16 relevés en B1). Toutes sont levées par un en-tête de plaque, de rangée ou de colonne ; aucune n'a été créée par LAB-A08.

| Libellé | Emplacement mesuré | StableID | Désambiguïsation |
|---|---|---|---|
| `DEPTH` | ENGINEERING / CROSS NETWORK | 35 : cross.bus_wt.depth, cross.bus_wt_char.depth, cross.bus_wt_freq.depth, cross.bus_wt_level.depth … | en-têtes source / destination de la cellule |
| `ON` | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 2 : op1.enable, op2.enable | colonne ou rangée (opérateur, source CROSS) |
| `CHAR` | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 5 : cross.bus_wt_char.depth, cross.op1_wt_char.depth, cross.op2_wt_char.depth, cross.op3_wt_char.depth … | colonne ou rangée (opérateur, source CROSS) |
| `FREQ` | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 5 : cross.bus_wt_freq.depth, cross.op1_wt_freq.depth, cross.op2_wt_freq.depth, cross.op3_wt_freq.depth … | colonne ou rangée (opérateur, source CROSS) |
| `LVL` | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 5 : cross.bus_wt_level.depth, cross.op1_wt_level.depth, cross.op2_wt_level.depth, cross.op3_wt_level.depth … | colonne ou rangée (opérateur, source CROSS) |
| `ON` | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 2 : op2.enable, op3.enable | colonne ou rangée (opérateur, source CROSS) |
| `PM` | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 5 : cross.bus_wt.depth, cross.op1_wt.depth, cross.op2_wt.depth, cross.op3_wt.depth … | colonne ou rangée (opérateur, source CROSS) |
| `X` | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 5 : cross.bus_wt_x.depth, cross.op1_wt_x.depth, cross.op2_wt_x.depth, cross.op3_wt_x.depth … | colonne ou rangée (opérateur, source CROSS) |
| `Y` | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 5 : cross.bus_wt_y.depth, cross.op1_wt_y.depth, cross.op2_wt_y.depth, cross.op3_wt_y.depth … | colonne ou rangée (opérateur, source CROSS) |
| `Z` | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 5 : cross.bus_wt_z.depth, cross.op1_wt_z.depth, cross.op2_wt_z.depth, cross.op3_wt_z.depth … | colonne ou rangée (opérateur, source CROSS) |
| `ON` | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 2 : op3.enable, op4.enable | colonne ou rangée (opérateur, source CROSS) |
| `ARM` | SOUND DESIGN / TRAITEMENTS | 9 : dst.fm1.arm, dst.fm2.arm, dst.gl1.arm, dst.gl2.arm … | plaque de l'emplacement (WT1 … GL2, filtre) |
| `FB MODE` | SOUND DESIGN / TRAITEMENTS | 6 : dst.fm1.d2.fb_mode, dst.fm2.d2.fb_mode, dst.gl1.d2.fb_mode, dst.gl2.d2.fb_mode … | plaque de l'emplacement (WT1 … GL2, filtre) |
| `FREEZE` | SOUND DESIGN / TRAITEMENTS | 15 : dst.fm1.d2.freeze, dst.fm1.s2.freeze, dst.fm2.d2.freeze, dst.fm2.s2.freeze … | plaque de l'emplacement (WT1 … GL2, filtre) |
| `PRE/POST` | SOUND DESIGN / TRAITEMENTS | 3 : flt.fm.pos, flt.gl.pos, flt.wt.pos | plaque de l'emplacement (WT1 … GL2, filtre) |
| `SPEED` | SOUND DESIGN / TRAITEMENTS | 6 : dst.fm1.d2.speed, dst.fm2.d2.speed, dst.gl1.d2.speed, dst.gl2.d2.speed … | plaque de l'emplacement (WT1 … GL2, filtre) |

**Redondances volontaires** : 4 (WT Z, CROSS LVL). Elles sont conservées parce que le libellé apparaît hors de la plaque de son domaine et que le préfixe empêche l'homonymie « Z Master » / CROSS (distinctions validées §12.2).

- `wt.z` — PERFORMANCE / PERFORMANCE : **WT Z**
- `wt.z` — SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 : **WT Z**
- `cross.master` — PERFORMANCE / PERFORMANCE : **CROSS LVL**
- `cross.master` — SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 : **CROSS LVL**

**Budgets P1 indéterminés** : **37** (36 en B1, plus l'en-tête CROSS AUDIO ajouté par LAB-A08). Statut géométrique : INDÉTERMINÉ, à vérifier dans les lots qui définissent leurs gabarits (barre et états : R2-B2 et R2-C ; titres et onglets : R2-D et R2-E). Largeurs mesurées :

| Référence | Libellé R1 | macos px | windows px | Lot de vérification |
|---|---|---|---|---|
| `bar.peak` | PEAK | 27.10 | 29.26 | R2-B2 / R2-C |
| `bar.authorize` | AUTHORIZE | 61.47 | 65.77 | R2-B2 / R2-C |
| `bar.panic` | PANIC | 32.22 | 35.80 | R2-B2 / R2-C |
| `bar.drone` | DRONE | 37.45 | 39.42 | R2-B2 / R2-C |
| `perf.core.status` | STATIC PREVIEW | 79.39 | 88.53 | R2-B2 / R2-C |
| `perf.core.x-axis` | X · WT→FM | 61.55 | 59.29 | R2-B2 / R2-C |
| `perf.core.y-axis` | Y · FM→WT | 61.75 | 59.03 | R2-B2 / R2-C |
| `perf.auth.armed` | ARMED | 37.98 | 39.69 | R2-B2 / R2-C |
| `perf.auth.exec` | AUTONOMY | 62.89 | 64.19 | R2-B2 / R2-C |
| `perf.balance.note` | PRE-CHAIN | 54.84 | 58.00 | R2-B2 / R2-C |
| `sound.cross.tab.forward` | WT→FM | 38.07 | 37.98 | R2-D |
| `sound.cross.tab.reverse` | FM→WT | 38.07 | 37.98 | R2-D |
| `sound.wtz.slot-a` | SLOT A | 34.86 | 36.18 | R2-D |
| `sound.wtz.slot-b` | SLOT B | 33.86 | 36.05 | R2-D |
| `sound.wtz.order` | EFFECTIVE Z ORDER | 96.04 | 102.89 | R2-D |
| `eng.group.network` | NETWORK | 48.44 | 51.98 | R2-E |
| `eng.group.modulation` | MODULATION | 67.17 | 70.40 | R2-E |
| `eng.group.banklab` | BANK LAB | 47.84 | 51.34 | R2-E |
| `eng.group.state` | STATE | 30.71 | 31.98 | R2-E |
| `eng.group.tools` | TOOLS | 32.59 | 33.76 | R2-E |
| `eng.view.neural` | NEURAL | 38.42 | 41.07 | R2-E |
| `eng.view.table` | TABLE | 30.25 | 32.04 | R2-E |
| `eng.state.drone` | RESTORE DRONE | 80.98 | 86.05 | R2-E |
| `eng.captures.now` | CAPTURE | 44.63 | 46.99 | R2-E |
| `inspector.base` | BASE | 21.91 | 24.26 | R2-E |
| `inspector.effective` | EFFECTIVE | 46.37 | 49.47 | R2-E |
| `state.off` | OFF | 18.42 | 19.47 | R2-B2 / R2-C |
| `state.on` | ON | 14.87 | 15.36 | R2-B2 / R2-C |
| `state.armed` | ARMED | 37.98 | 39.69 | R2-B2 / R2-C |
| `state.active` | ACTIVE | 38.41 | 41.22 | R2-B2 / R2-C |
| `state.render` | RENDER | 42.21 | 45.49 | R2-B2 / R2-C |
| `state.configured` | SUSPENDED | 61.62 | 67.53 | R2-B2 / R2-C |
| `state.suspended` | SUSPENDED | 61.62 | 67.53 | R2-B2 / R2-C |
| `state.authorized` | AUTHORIZED | 69.55 | 73.96 | R2-B2 / R2-C |
| `state.revoked` | REVOKED | 50.09 | 52.96 | R2-B2 / R2-C |
| `units.peak` | dBFS | 21.29 | 23.23 | R2-B2 / R2-C |
| `sound.cross.section.audio` | CROSS AUDIO | 66.48 | 71.80 | R2-D |

**Marges faibles (< 8 px)** (toutes priorités, au moins une plateforme) : 3.

- `cross.master` PERFORMANCE / PERFORMANCE : **Z · CROSS LVL** — budget 70.00 px ; macos 68.96 px (PASS (marge < 8 px)) ; windows 72.91 px (PASS en deux lignes)
- `wt.x_morph#2` SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 : **SPECTRAL** — budget 48.67 px ; macos 44.95 px (PASS (marge < 8 px)) ; windows 46.78 px (PASS (marge < 8 px))
- `wt.y_morph#2` SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 : **SPECTRAL** — budget 48.67 px ; macos 44.95 px (PASS (marge < 8 px)) ; windows 46.78 px (PASS (marge < 8 px))

**Libellés nécessitant deux lignes** (toutes priorités, au moins une plateforme) : 1.

- `cross.master` PERFORMANCE / PERFORMANCE : **Z · CROSS LVL** — budget 70.00 px ; macos 68.96 px (PASS (marge < 8 px)) ; windows 72.91 px (PASS en deux lignes)

**Dépassements** (toutes priorités, au moins une plateforme) : 0.


**Repères critiques au plancher de 9 px à 75 %** (OD-06) : 26 occurrences, corps retenu 12 px (XL 12,5).

| Référence | Libellé | Corps px | 75 % écran px | macos | windows |
|---|---|---|---|---|---|
| `wt.x` | X | 12.5 | 9.38 | atteint | atteint |
| `wt.y` | Y | 12.5 | 9.38 | atteint | atteint |
| `wt.z` | WT Z | 12.5 | 9.38 | atteint | atteint |
| `cross.master` | Z · CROSS LVL | 12.0 | 9.00 | atteint | atteint |
| `cross.master` | CROSS LVL | 12.0 | 9.00 | atteint | atteint |
| `cross.fwd` | X · WT→FM | 12.0 | 9.00 | atteint | atteint |
| `cross.rev` | Y · FM→WT | 12.0 | 9.00 | atteint | atteint |
| `macro1` | M1 | 12.0 | 9.00 | atteint | atteint |
| `macro2` | M2 | 12.0 | 9.00 | atteint | atteint |
| `macro3` | M3 | 12.0 | 9.00 | atteint | atteint |
| `macro4` | M4 | 12.0 | 9.00 | atteint | atteint |
| `bar.peak` | PEAK | 12.0 | 9.00 | atteint | atteint |
| `bar.authorize` | AUTHORIZE | 12.0 | 9.00 | atteint | atteint |
| `bar.panic` | PANIC | 12.0 | 9.00 | atteint | atteint |
| `bar.drone` | DRONE | 12.0 | 9.00 | atteint | atteint |
| `perf.core.x-axis` | X · WT→FM | 12.0 | 9.00 | atteint | atteint |
| `perf.core.y-axis` | Y · FM→WT | 12.0 | 9.00 | atteint | atteint |
| `perf.auth.armed` | ARMED | 12.0 | 9.00 | atteint | atteint |
| `perf.auth.exec` | AUTONOMY | 12.0 | 9.00 | atteint | atteint |
| `state.armed` | ARMED | 12.0 | 9.00 | atteint | atteint |
| `state.active` | ACTIVE | 12.0 | 9.00 | atteint | atteint |
| `state.render` | RENDER | 12.0 | 9.00 | atteint | atteint |
| `state.configured` | SUSPENDED | 12.0 | 9.00 | atteint | atteint |
| `state.suspended` | SUSPENDED | 12.0 | 9.00 | atteint | atteint |
| `state.authorized` | AUTHORIZED | 12.0 | 9.00 | atteint | atteint |
| `state.revoked` | REVOKED | 12.0 | 9.00 | atteint | atteint |

