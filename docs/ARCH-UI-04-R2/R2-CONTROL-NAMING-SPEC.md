# ARCH-UI-04-R2-A — nomenclature des commandes (DA-A01)

Référentiel de sérigraphie **anglaise technique compacte**. Le détail par occurrence est dans
[R2-CONTROL-LABELS-PROPOSALS.csv](R2-CONTROL-LABELS-PROPOSALS.csv) (2 213 lignes). Tous les libellés sont des
**propositions à valider** par le propriétaire. Les exemples qu'il a donnés à titre d'orientation sont signalés comme tels.

## 1. Exigences

| ID | Exigence |
|---|---|
| NAM-01 | Toute chaîne visible est en anglais : sérigraphie, titres, onglets, actions, états, champs de l'Inspector, infobulles, messages (DA-A01). |
| NAM-02 | Le nom hôte (registre) n'est pas modifié (INV-02). Le **titre accessible** d'une commande liée reste exactement le nom du registre (UI-01w). |
| NAM-03 | Le libellé court est un alias d'affichage. Il dépend du contexte : le préfixe porté par la plaque, la section ou l'en-tête est retiré. |
| NAM-04 | Le nom complet (nom du registre, éventuellement suivi d'une précision entre parenthèses) apparaît au focus, dans l'Inspector (rang 1) et dans l'infobulle. |
| NAM-05 | Une fonction porte le même libellé court dans tous les espaces où elle a le même contexte (DE-02). |
| NAM-06 | Deux StableID visibles dans un même champ visuel (plaque, rangée ou cellule de réseau avec ses en-têtes) ne portent jamais le même libellé sans en-tête de désambiguïsation. |
| NAM-07 | Les distinctions critiques du §5 sont obligatoires. |
| NAM-08 | Le libellé tient dans le budget de sa famille, au corps de la famille, sur une ligne. À défaut, il passe sur deux lignes si la famille l'admet (NEXUS, NEURAL CELL, PLASMA V). Il n'est jamais réduit sous le plancher de 9,5 px logiques. |
| NAM-09 | Les abréviations viennent du lexique §4. Aucune abréviation à point (« FREQ. ») ni de forme française. |
| NAM-10 | Les options de liste gardent leur indice et leur libellé hôte. Seul l'affichage compact change (§7). |
| NAM-11 | Les unités physiques sont conservées. Les domaines normalisés (`lin`, `pos`, `bip`) n'affichent pas d'unité en façade : le domaine et la plage sont dans l'Inspector (§6). |
| NAM-12 | La grammaire de route est unique : `SOURCE→DEST` (DE-03). |

## 2. Grammaire

- **Casse** : sérigraphie, titres et états en MAJUSCULES. Noms complets en casse du registre (`FLT WT Cutoff`).
  Textes d'aide en phrase (`Filter source / dest…`).
- **Caractères** : ASCII, plus `→` (route), `·` (séparateur de contexte), `…` (action qui ouvre un dialogue ou se
  termine ailleurs), `−` (U+2212, signe et retrait) et `&` (S&H). La présence des glyphes `→ · … −` dans la police
  embarquée DOIT être vérifiée en R2-B. Repli : `>` pour `→`, `-` pour `−` (OD-11).
- **Ordre** : `[contexte] FONCTION [qualificatif]`. Exemples : `XAUDIO AMT`, `A DEPTH`, `EXC LVL`, `FB MODE`.
- **Verbes** pour les actions (`CAPTURE`, `APPLY WAVE`, `GENERATE`), **noms** pour les paramètres, **adjectifs ou
  participes** pour les états (`ARMED`, `ACTIVE`, `SUSPENDED`).

## 3. Retrait de préfixe

| Préfixe du nom hôte | Porté par | Exemple |
|---|---|---|
| `WT`, `FM`, `CROSS` | Plaque du moteur | `WT Coarse` → COARSE |
| `DST WT1`, `DST FM2 S2`, … | Plaque d'emplacement (WT1…GL2) et titre de l'étage | `DST GL1 D2 Feedback Mode` → FB MODE |
| `FLT WT`, `FLT FM`, `FLT GLOBAL` | Plaque FILTER de la chaîne | `FLT FM Excitation Level` → EXC LVL |
| `Amp`, `ENV WT`, `ENV AUX1`, … | Plaque de l'enveloppe | `ENV WT Loop` → LOOP |
| `LFO1`…`LFO4` | Plaque du LFO | `LFO2 Division` → DIV |
| `OP1`…`OP4` | Colonne de l'opérateur | `OP3 Mod Mode` → MOD TYPE |
| `WT ZA` / `WT ZB` | Plaque SLOT A / SLOT B | `WT ZA Shape` → A SHAPE (préfixe A conservé, car les deux slots sont côte à côte) |
| `FM OPx>OPy`, `CROSS …>…` | En-têtes de ligne et de colonne du réseau | cellule → DEPTH |

## 4. Lexique

| Mot du registre | Sérigraphie | Mot du registre | Sérigraphie |
|---|---|---|---|
| Attack | ATK | Release | REL |
| Decay | DEC (enveloppes) / DECAY (F4) | Sustain | SUS |
| Delay | DLY | Hold | HOLD |
| Feedback | FDBK | Feedback Mode | FB MODE |
| Resonance | RES | Frequency, Freq | FREQ |
| Level | LVL | Dry/Wet | MIX |
| Output (étage) | GAIN | Keytrack | KEY |
| Character | CHAR | Damping | DAMP |
| Brightness | BRIGHT | Inharmonicity | INHARM |
| Excitation / Excitation Level | EXCITE / EXC LVL | Threshold | THRESH |
| Transient | TRANS | Symmetry | SYM |
| Sensitivity | SENS | Detector | DETECT |
| Crossfade | XFADE | Playback | SPEED |
| Position | POS | Interpolation | INTERP |
| Division | DIV | Retrigger | RETRIG |
| Waveform | WAVE | Self-Oscillation | SELF OSC |
| Stereo Phase | ST PHASE | Phase Drift | DRIFT |
| FFT Size | FFT | Mod Coupling | COUPLING |
| Mod Mode | MOD TYPE | Phase Mode (opérateur) | PH MODE |
| Enable | ON | Autonomous Armed | ARM |
| Random Hold / Random Smooth | S&H / SMOOTH | Bend Down | BEND DN |

Les mots de cinq lettres ou moins restent entiers (`DRIVE`, `TONE`, `BITS`, `RATE`, `MODE`, `SHAPE`, `CURVE`, `DEPTH`).

## 5. Distinctions critiques

| Fonction | StableID | Libellé court | Nom complet affiché | Ce qui le distingue |
|---|---|---|---|---|
| Profondeur des transformations Z | `wt.z` | **WT Z** | WT Z Master (Z transform depth) | « WT » obligatoire. Jamais « Z Master » seul (homonymie relevée par Codex). |
| Niveau global CROSS | `cross.master` | **CROSS LVL** ; axe du Core : `Z · CROSS LVL` | CROSS Master (CROSS level) | « CROSS » obligatoire. En PERFORMANCE, l'axe Z du Core est préfixé « Z · ». |
| CROSS vers l'avant | `cross.fwd` | **WT→FM** ; axe du Core : `X · WT→FM` | CROSS WT>FM (forward) | Flèche unique, source à gauche |
| CROSS en retour | `cross.rev` | **FM→WT** ; axe du Core : `Y · FM→WT` | CROSS FM>WT (reverse) | idem |
| Balance audio WT/FM | `perf.balance` | **BALANCE**, mention PRE-CHAIN | WT/FM Balance (audio, pre-chain) | Balance audio post-Amp, avant les chaînes (B-02). Ce n'est pas un réglage CROSS. |
| Quantité de Cross Audio | `cross.audio.amount` | **XAUDIO AMT** | Cross Audio Amount (post-chain) | « XAUDIO », distinct de CROSS LVL et de BALANCE |
| Mode de Cross Audio | `cross.audio.mode` | **XAUDIO MODE** | Cross Audio Mode (post-chain) | idem |
| Gain de sortie | `out.gain` | **OUT GAIN** | Output Gain | « OUT » obligatoire : le GAIN de sortie des étages est un autre paramètre |
| Crête de sortie | (mesure) | **PEAK** · dBFS | Output Peak (dBFS, block peak at plugin output) | Mesure, pas un paramètre. Unité dBFS, jamais dB. |
| Position WT | `wt.x`, `wt.y` | **X**, **Y** (plaque WT) | WT X, WT Y | En PERFORMANCE, le contexte WT est porté par la plaque |
| Armement / gel / autorisation / capture | `*.arm`, `*.freeze`, —, — | **ARM**, **FREEZE**, **AUTHORIZE**, **CAPTURE** | Autonomous Armed ; Freeze ; Authorize autonomous states (this instance) ; Capture frozen D2/S2 content | Quatre types distincts ([R2-DESIGN-SYSTEM-SPEC.md](R2-DESIGN-SYSTEM-SPEC.md) §5.2) |
| Valeurs | — | **BASE**, **EFFECTIVE** | Base value ; Effective value (engine, decimated) | Glossaire (master §8) |

Écart avec l'orientation du propriétaire : son exemple « GAIN » est conservé pour le gain de sortie des étages, et
`out.gain` reçoit OUT GAIN. Cet écart est à confirmer (CSV, statut « Proposé R2-A »).

## 6. Valeurs et unités

| Grandeur | Format proposé | Exemple |
|---|---|---|
| dB | 1 décimale, signe `−`, « −inf » sous −120 | −6.0 dB |
| dBFS (mesure) | 1 décimale | −0.3 dBFS |
| Hz / kHz | 3 chiffres significatifs, kHz au-delà de 1 000 | 440 Hz, 2.40 kHz |
| ms / s | entier en ms sous 1 s, 2 décimales au-delà | 120 ms, 1.25 s |
| rad | 2 décimales | 1.00 rad |
| demi-tons / cents | entier signé | +7 st, −12 ct |
| `lin`, `pos` | 2 décimales, sans unité | 0.50 |
| `bip` | 2 décimales signées, sans unité | −0.25 |
| ratio | 2 décimales et « : 1 » | 4.00 : 1 |

Ces formats sont des propositions (Codex DA-10, P3-01). Ils ne changent pas les valeurs transmises à l'hôte.

## 7. Options de liste

L'indice et le libellé hôte de chaque option sont inchangés (choix du registre). L'affichage compact s'applique aux
segments et listes étroites : `S1 Ring/Frequency Shifter` → S1 RING/SHIFT, `Through-Zero FM` → TZFM,
`Random Hold` → S&H. Le CSV contient les **537** options du registre. 395 diffèrent du libellé hôte, dont 202 au-delà du
seul passage en majuscules. Aucune liste ne contient deux options au même libellé compact (vérifié sur les 537). Le
libellé hôte complet reste visible dans la liste déroulante et dans l'Inspector.

## 8. Méthode du CSV

### 8.1 Types de lignes

| Type (colonne « Occurrence ») | Nombre | Source |
|---|---|---|
| Paramètre (canonique) | 703 | Registre compilé, un par StableID |
| Variante d'emplacement | 851 | Emplacements distincts mesurés par Codex (zoom 100 %, 1440 × 900) : StableID × emplacement × famille × libellé actuel |
| Option de liste (liste / segments) | 537 | Choix du registre, budgets Codex (`CONTROL-LABELS-CHOICES.csv`) |
| Hors registre, titres, champs, états, unités | 122 | Chaînes de `ui/*.cpp` 0.4.1 relevées dans le code, propositions Codex |

### 8.2 Colonnes

Référence (StableID, `StableID#indice` ou référence hors registre) · Occurrence · Famille graphique · Emplacement ·
Libellé actuel (code 0.4.1 ou mesure Codex) · Libellé court proposé · Nom complet (Inspector, infobulle) · Caractères ·
Corps px log. · Largeur actuelle mesurée px log. (mesure JUCE Codex) · Largeur proposée estimée px log. · Budget px log. ·
Marge estimée px · Ajustement · Justification · Risque d'ambiguïté · Priorité · Statut · Source mesure.

### 8.3 Modèle de largeur

La largeur proposée est une **estimation**, pas une mesure. Elle vaut `caractères × 0,525 × corps`. Le coefficient
0,525 est la médiane du rapport largeur/caractère/corps sur **734 libellés en majuscules** mesurés par Codex avec la
police JUCE de l'interface. À corps 11, la médiane est de 5,77 px/caractère et le maximum observé de 7,61 px/caractère
(chaînes riches en M et W). Règle d'ajustement :

| Marge estimée | Ajustement | Action R2-B |
|---|---|---|
| ≥ 8 px | OK (≥ 8 px) | Mesure JUCE de confirmation par test automatique |
| 0 à 8 px | OK (marge < 8 px) | Mesure JUCE obligatoire avant adoption |
| < 0, famille à deux lignes | 2 lignes requises | Vérifier la coupure et la hauteur |
| < 0, autre famille | DÉPASSEMENT — à revoir | Nouveau libellé |

Pour une ligne canonique, le budget retenu est le **minimum** des budgets observés sur tous les emplacements de ce
StableID.

### 8.4 Risque d'ambiguïté

- **élevé avant R2** : homonymie relevée par Codex (§5). Le libellé proposé la résout.
- **moyen** : plusieurs StableID partagent le libellé dans un même emplacement mesuré. Ils sont désambiguïsés par
  l'en-tête de plaque, de rangée ou de colonne. La justification indique le nombre de StableID concernés.
- **faible — terme générique** : MODE, TYPE, LVL, DEPTH, RATE, PHASE, POS, CURVE. Le contexte de plaque est requis.
- **faible** : aucun partage détecté.

### 8.5 Priorité et statut

Priorité P1 pour les distinctions critiques, les armements, les gels, les modes de feedback et les lignes que Codex
classe P1. Sinon, la priorité Codex est reprise (P2 par défaut). Statut « Orientation propriétaire (§7) — à confirmer
R2-B » pour les libellés identiques aux exemples du brief, « Proposé R2-A — validation propriétaire » pour les autres.

## 9. Traitement des 1 102 propositions Codex

| Élément Codex | Traitement R2-A |
|---|---|
| Libellés proposés en français | **Rejetés** (RJ-01). Aucun n'est appliqué. |
| Mesures JUCE (largeurs, corps, budgets, lignes, hors viewport, présence AX) | **Reprises** comme données d'entrée |
| Familles, emplacements, priorités | **Reprises** |
| Constats : homonymie Z Master, Ouvrir/Enregistrer qui naviguent tous deux vers Presets/State, statut du Core qui prime sur les axes, ARMÉ/Autonomous Armed, unités `lin`/`bip`/`pos` | **Repris** et traités (§5, CSV hors registre) |
| Suppression des préfixes de groupe | **Reprise** (§3) |

## 10. Résultats (CSV R2-A)

| Indicateur | Valeur |
|---|---|
| Occurrences | 2 213 (703 canoniques, 851 variantes, 537 options, 122 hors registre) |
| Canoniques dans le budget (estimation, marge ≥ 8 px) | 703 / 703 |
| Variantes dans le budget | 850 / 851. Exception : `Z · CROSS LVL` sur PLASMA V (70 px, −5,1 px), deux lignes requises. |
| Options dans le budget | 535 ≥ 8 px ; 2 à marge < 8 px (`SPECTRAL`, `wt.x_morph`/`wt.y_morph`, 2,5 px) |
| Groupes de libellés partagés dans un emplacement | 95 groupes (585 StableID). L'essentiel concerne SOUND DESIGN / TRAITEMENTS, où les six emplacements WT1…GL2 partagent un même emplacement mesuré par Codex et sont désambiguïsés par leur plaque. Viennent ensuite ATK/REL des enveloppes et suiveurs, SYNC et DEPTH dans les cellules de réseau, identifiées par leurs en-têtes. |
| Priorités | P1 323, P2 1 884, P3 6 |
| Statuts | 14 « Orientation propriétaire — à confirmer », 2 199 « Proposé R2-A » |

Ces résultats reposent sur une estimation de largeur. Ils DOIVENT être confirmés par mesure JUCE en R2-B (NAM-08).

## 11. Impact sur les tests existants (non modifiés par R2-A)

| Test | Dépendance | Effet de la nomenclature R2 |
|---|---|---|
| UI-01w (`tests/wrapper/WrapperTests.cpp` l.364–384) | Titre accessible = nom du registre. Nombre d'onglets par espace. | Pas d'effet sur les titres si NAM-02 est respecté. Le regroupement ENGINEERING (DA-A10) change la structure d'onglets. |
| AUT-08 (l.686) | Bouton repéré par le texte « Autoriser » | Le texte devient AUTHORIZE |
| TA-07w (l.993, l.1014) | Bouton « Drone » ; HelpText `dst.gl1.arm` | Le texte devient DRONE. Le HelpText (StableID) n'est pas affecté. |
| UI04-OPS (`tests/wrapper/Ui04Tests.cpp` l.798–825) | 42 + 10 chaînes visibles françaises ou anglaises (« Ouvrir », « Capturer gels », « Importer WAV... », « Épingler », …) | Toutes changent |
| UI04-COV, UI04-DECO | Couverture par StableID, câblage des boutons | Pas d'effet direct |

La mise à jour de ces tests DOIT se faire par une stratégie d'identification stable plutôt que par le texte : identifiant
de composant ou titre accessible technique. Cette stratégie est soumise à autorisation (OD-01) : la gouvernance interdit
d'ajuster un critère de test pour obtenir un PASS sans décision explicite.
