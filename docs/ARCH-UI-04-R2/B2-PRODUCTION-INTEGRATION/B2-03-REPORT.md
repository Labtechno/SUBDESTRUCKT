# ARCH-UI-04-R2-B2-03 — intégration des composants de production

Ordre imposé : NEXUS, PLASMA, NEURAL CELL, SYNAPTIC TRACK, Cybernetic HUD, PANIC. Chaque famille est un jalon atomique
et réversible. Contrats : [B2-00-ADDENDUM.md](B2-00-ADDENDUM.md) et [B2-00-ADDENDUM-B2-03.md](B2-00-ADDENDUM-B2-03.md).

## 1. B2-03.1 — NEXUS

### 1.1 Intégration

Le chemin rotatif de `ui::ContinuousControl` dessine désormais le NEXUS R1 par le peintre partagé :

- `sbd::r2::paintNexus` reçoit une vue `NexusView` construite à partir des seules données publiées :
  - Base ;
  - Effective, publiée par la télémétrie ;
  - routes de la configuration publiée ;
  - libellé, états.
- Domaine 0.4.1 → domaine R1 ; densité XL / S / Micro → gabarit R1 XL / S / Micro.
- **Géométrie partagée** : zone du bouton, fenêtre de valeur et zone du libellé du composant sont les fonctions
  publiques du peintre (`nexusKnobArea`, `nexusValueArea`). Le rendu, le glisser, le double-clic, la saisie numérique et
  le contrôle anti-troncature (`textFits`) utilisent donc la même définition.
- **Sérigraphie** : corps de la famille ; s'il ne tient pas sur deux lignes, il est réduit par pas de 0,5 px jusqu'au
  plancher de 9,5 px. La règle de coupure est celle du peintre, sans ellipse. UI04-CLIP reste PASS : 12 configurations,
  4 804 commandes, 0 troncature.
- **Bipolaire** : le zéro est placé à sa vraie position normalisée (`zeroNorm`). Sept paramètres ont une plage
  asymétrique, dont `out.gain` (−48..+6 dB) et les entrées DRIVE (−24..+36 dB). Le laboratoire le plaçait au milieu de la
  course, ce qui reste vrai pour les plages symétriques : les rendus du laboratoire sont inchangés (étape 1 : 15/15
  identiques).
- **Marques de route** : profondeur × `modScale`, rapportée à la course. Elles viennent de la configuration publiée,
  recopiée par l'éditeur sur le fil message (`UiContext::mod`, 10 Hz, révision incrémentée à chaque changement). **Aucune
  lecture du fil audio n'est ajoutée** (CE-B2-06).
- **Saisie numérique** : le champ recouvre exactement la fenêtre de valeur R1, dans sa typographie.
- **Inchangé** : `ParamLink`, `BoundControl`, glisser relatif 0.4.1, Maj = réglage fin, molette, clavier, menu
  contextuel, double-clic sur le bouton = défaut, Alt+clic, Suppr, MIDI Learn, états désactivé et inactif.
- **PLASMA** : rendu 0.4.1 conservé jusqu'à B2-03.2.

### 1.2 Correction PB-01e

Défaut : un geste molette encore ouvert recevait un clic, et les gestes hôte s'imbriquaient (profondeur 2).

Correction : `closeWheelGesture()` referme le geste molette avant tout geste incompatible :

- `mouseDown` (glisser, Alt+clic, menu, double-clic) ;
- `keyPressed` ;
- les écritures complètes (`userSet` : défaut, saisie).

Aucune temporisation n'est ajoutée. La classe est commune aux NEXUS et aux PLASMA : la correction couvre les deux.

| Test permanent | Mesure (Windows) | Attendu |
|---|---|---|
| PB-01e molette puis clic avant 450 ms | profondeur max **1** (avant : 2), ouverts après relâchement 0, après destruction 0, **2 transactions Undo** (molette, glisser) | 1, 0, 0, 2 : **PASS** |
| PB-01f (nouveau) molette puis touche avant 450 ms | profondeur 1, 0, 0, 2 transactions | **PASS** |
| PB-01a, b, d, m ; PB-02a à e ; PB-N1 à N3 ; PB-J1 à J4 | statuts identiques à B2-01 ; PB-J et PB-01c restent REPRODUITS (DB2-08, B2-03.2) | inchangés |

Les preuves historiques sont conservées (`PB/windows`, `PB/macos`). Journaux après correction :
[`PB/b2-03-1/windows/`](PB/b2-03-1/windows).

Le scénario de double-clic PB-01 initial reste **non reproduit** et n'est pas corrigé, faute de nouvelle preuve.

### 1.3 Indicateur MIDI Learn (CE-B2-08, EC-21)

La donnée est celle de 0.4.1 : le CC appris, publié par l'éditeur. Le rendu suit la typographie des marqueurs R1 (« +n »,
« INACTIVE ») : corps 8,5 gras, en `textDim`. Le contraste mesuré est de 6,44:1 sur plaque et 6,94:1 sur châssis.

**Mesure des emplacements** (`NX-CC-PLACE`) : union des pixels dessinés par le peintre sur un balayage de 1 056 états
(Base, uni ou bipolaire, Effective, cinq routes, survol, focus, pression, INACTIVE) :

| Gabarit | Coins de la zone du bouton | Sérigraphie, à gauche ou à droite du libellé |
|---|---|---|
| XL | 143 à 237 pixels recouverts : arc, graduations, anneau de focus | **libre** dans tous les états |
| S | 268 à 440 pixels recouverts | recouvre le libellé : 28 à 110 pixels |

**Implantation retenue, provisoire jusqu'à DB2-10** :

- l'indicateur est placé dans la sérigraphie, à gauche du libellé, **seulement si la place est libre**, selon la mise
  en page du libellé calculée par le peintre ;
- sinon il n'est pas dessiné ;
- en Micro il est masqué, comme en 0.4.1 ;
- l'infobulle porte « MIDI Learn : CC n » dans tous les cas.

Couverture sur les 501 paramètres continus avec leur libellé court : **XL 457, S 68**.

**NX-CC PASS** sur 96 configurations (3 gabarits, libellés courts et longs, uni ou bipolaire, actif ou INACTIVE, échelles
0,75 / 1 / 1,5 / 2) :

- 0 pixel modifié hors de la zone de l'indicateur ;
- **0 pixel déjà dessiné recouvert** ;
- aucun changement quand la place manque ;
- infobulle toujours renseignée.

Le cas S est soumis à l'**arbitrage ciblé DB2-10** ([B2-DECISIONS-PENDING.md](B2-DECISIONS-PENDING.md)).

### 1.4 Tests spécifiques

| Test | Résultat (Windows) |
|---|---|
| NX-CC | PASS (§1.3) |
| NX-ROUTES | PASS. Ajout : 34 px ; inversion : 38 px, distincts de l'ajout ; retrait : retour exact au rendu initial ; une copie modifiée sans révision est ignorée. |
| NX-GEOM | PASS. Fenêtre de valeur = géométrie du peintre en XL, S et Micro. La saisie s'ouvre sur la fenêtre, sans écriture préalable. Le double-clic sur le bouton revient au défaut : contrat NEXUS inchangé, DB2-09 ne concerne que les PLASMA. |
| UI04-CLIP, UI04-KEY et suite historique | PASS ; barrière PASS (82 critères, AUT-02 connu) |

### 1.5 Conformité visuelle (CE-B2-03)

**Étape 1** (laboratoire → références) : toujours **15/15 identiques, images entières**. La plaque gauche de la planche 12
montre les composants de production ; elle est déclarée zone de production et rapportée à part (106 414 px, attendus).

**Étape 2** (référence R1 historique → composant de production, VIS-01) :
[`VIS/README.md`](VIS/README.md) §Étape 2, résultats [`VIS/b2-03/windows/`](VIS/b2-03/windows).

Méthode :

- les planches historiques 01, 09 (75, 100 et 150 %), 11 et 12 sont reconstruites ;
- 42 NEXUS R1 du laboratoire sont remplacés par le composant de production lié au paramètre réel, avec la même valeur
  normalisée, le même libellé, la même Effective et les mêmes routes ;
- chaque rendu est comparé à la capture historique.

| Statut | Windows | Lecture |
|---|---|---|
| Contrôles hors substitution | **6/6 identiques** | Planches strictement comparables |
| IDENTIQUE | 3 | Même texte de valeur : composant identique au pixel près |
| TEXTE DE VALEUR | 38 | **Structure R1 identique au pixel près** : cadran, arcs, sérigraphie, graduations, Effective, marques. Seule la fenêtre de valeur diffère : la production affiche le format 0.4.1 (`0.486 pos`) au lieu de `0.49`. C'est une différence de **donnée** (NAM-11, formats §6 : P3-01, revue groupée B2-04). |
| ETAT DU RENDU | 1 | 1 pixel (écart 2/255). Le contrôle différentiel montre le **même pixel** sur le composant R1 du laboratoire rendu au même endroit dans les mêmes conditions : c'est l'état du cache de géométrie de Direct2D. |
| DIFFERENTE | **0** | |

macOS : le run 8 du workflow Component Lab sur `f22bf8c` a échoué **avant** le versement des résultats. Cause : le
shell des étapes GitHub s'exécute avec `-e`, et le code de retour du comparateur interrompait l'étape. Les journaux
détaillés d'un run exigent une authentification. Le workflow est corrigé (versement toujours effectué, verdict appliqué
ensuite) et rejoué avec B2-03.2. Résultats : `VIS/b2-03/macos/`.

**Étape 3** (composant → interface complète) : captures de production Windows
[`CAPTURES/windows/b2-03-1-nexus/`](CAPTURES/windows/b2-03-1-nexus) (28 vues, zooms 75 et 150 % compris). Captures
macOS : workflow de preuves, jalon marqué `[captures]`.

### 1.6 Performances (GC-01) et CE-B2-06

Run 8 sur `f22bf8c`, banc v3, alternance avec la référence `0517426`, plancher A/A du run 5. Verdicts :
[`GC01/macos/run8-f22bf8c/gc01-verdicts.txt`](GC01/macos/run8-f22bf8c/gc01-verdicts.txt).

| Seuil | Verdict | Mesure |
|---|---|---|
| **GS-01** | **ÉCHEC** | Pire vue × zoom : ENG-1 à 75 %, médiane 19,7 → 31,8 ms (×1,62 ; seuil ×1,5 ; bruit A/A 0,43, inférieur à la marge de 0,5). Par exécution : courant 27,6 et 35,9 ms, référence 16,2 et 23,2 ms. Les deux séries ne se recouvrent pas : **la régression est réelle**. |
| GS-02 (comparatif) | PASS | ×0,96 |
| GS-03 | PASS | 1 zone par rafraîchissement, comme la référence |
| GS-04, GS-07 | NON CONCLUANT | Bruit VM (cœurs P/E), comme en B2-02 |
| GS-05 | PASS | ×0,94 |
| GS-06 (comparatif) | PASS | ×0,88 |

**Cause, localisée par mesure** (Windows, NEXUS S de production, 3 000 peintures à 75 %) :

| Composant | Coût |
|---|---|
| NEXUS de production | **270 µs**, dont 137 µs hors peintre |
| `knobArea()` seul | 74 µs |
| Corps du libellé | ≈ 60 µs |
| Peintre R1 seul | 128 µs |

Les mesures de texte (mise en forme HarfBuzz) de la sérigraphie étaient refaites à chaque peinture, alors que le libellé
ne change pas.

**Correction, sans dérive artistique** (B2-03.2) :

- la mise en page de la sérigraphie (corps, nombre de lignes, coupure) et la zone du bouton sont calculées au
  redimensionnement et au changement de libellé (`layoutNexus`), plus jamais à la peinture ;
- le peintre accepte cette mise en page précalculée (`labelLines`, `labelSplit`, `fittedTextSplit`) ; sans elle, il la
  calcule comme avant (laboratoire inchangé).

Résultat : **270 → 99 µs** par NEXUS (−63 %).

Identité de rendu prouvée :

- étape 1 : 15/15 planches identiques, images entières ;
- VIS-01 : mêmes statuts, 0 DIFFÉRENTE ;
- NX-CC, NX-GEOM, NX-ROUTES et UI04-CLIP : PASS.

GS-01 est **remesuré sur B2-03.2** (run de preuves suivant). Le verdict du jalon NEXUS reste ouvert jusqu'à ce run.

Revue CE-B2-06 du jalon :

- aucun timer ajouté : le minuteur de fin de molette existait déjà ;
- aucune lecture du fil audio ajoutée ; le marqueur de routes ne lit plus `modSerial` ;
- copie de la configuration de modulation sur le fil message, seulement quand elle change ;
- aucun rappel asynchrone ajouté ;
- aucune ressource graphique persistante.

### 1.6 bis Preuves macOS du wrapper (run 8, `f22bf8c`)

Barrière **PASS** : 82 critères, AUT-02 connu. NX-CC, NX-ROUTES, NX-GEOM, PB-01e, PB-01f et UI04-CLIP : PASS.

Fichiers : [`PB/b2-03-1/macos/`](PB/b2-03-1/macos). Ce run avait été versé par erreur dans `PB/macos/`, à la place des
preuves historiques de B2-01. Celles-ci (run 1) sont restaurées. Les runs suivants sont versés dans
`PB/runs/macos/<run>/`.

### 1.7 Invariants

- `git diff d2d08ac -- source/` vide ;
- `plugin/` inchangé ;
- aucun identifiant, format, seuil, référence historique ou `known_failures` modifié.

## 2. B2-03.2 — PLASMA

### 2.1 Contrat d'interaction (DB2-02, DB2-08, DB2-09), appliqué aux trois PLASMA de production

Le contrat s'applique à `out.gain` (vertical), `cross.master` (Z du Core, vertical) et `perf.balance` (horizontal).

| Zone | Interaction | Comportement |
|---|---|---|
| Zone de manipulation | Clic | Position absolue immédiate ; aucune écriture si la valeur ne change pas |
| Zone de manipulation | Glisser | Historique, rapporté à la longueur utile du canal ; le curseur suit le pointeur ; aucun saut à la prise en main |
| Zone de manipulation | Double-clic | Position absolue, **sans retour au défaut** (DB2-09) : une écriture, une transaction |
| Libellé, valeur, hors canal | Clic ou glisser | Sélection et focus seulement : aucun geste hôte, aucune écriture |
| Valeur | Double-clic | Saisie numérique, sans écriture préalable |
| Partout | Alt+clic, Suppr, menu | Retour au défaut ; molette, clavier et MIDI Learn inchangés |

La **zone de manipulation** est la zone que le rendu montre comme manipulable :

- **PLASMA R1** : le canal du peintre, élargi à l'empreinte du curseur (24 × 10) ;
- **rendu 0.4.1** (PLASMA bloqués) : le rail et l'empreinte de la poignée, bornés par la sérigraphie et la fenêtre de
  valeur.

Infobulle :

> clic ou glisser dans le canal : réglage, Maj : fin, Alt+clic, Suppr ou menu : défaut, double-clic sur la valeur ou
> Entrée : saisie

### 2.2 Gabarit par surface utile (CE-B2-02, PLAS-G)

Mesuré dans les 12 configurations d'UI04-CLIP, tous contextes :

| PLASMA | Cellule réelle | Gabarit | Raison |
|---|---|---|---|
| `cross.master` (Z du Core) | 74 × 420 à 74 × 1149 | **STANDARD 64 × 180**, R1 | Tient partout |
| `out.gain` | 96 × 158 | **bloqué** (rendu 0.4.1) | Seul COMPACT tient en surface, mais la valeur de production « −48.00 dB » (54,2 px) dépasse sa fenêtre (44 px) |
| `perf.balance` | 326 à 1 406 × 48 | **bloqué** (rendu 0.4.1) | Horizontal h44 : la valeur « −0.995 bip » (60,3 px) dépasse la fenêtre (50 px) |

Conformément à CE-B2-02, le gabarit n'est ni déformé ni réduit en typographie, et R2-C n'est pas anticipé.

Levée du blocage :

- **BALANCE** : formats R1 des valeurs (NAM-11 et §6 de la nomenclature, P3-01, revue groupée B2-04). « −0.99 » tient
  dans 50 px.
- **OUT GAIN** : un format plus court en dB, à décider en B2-04, ou une cellule de 180 de haut en R2-C, pour STANDARD.

Le contrat d'interaction (§2.1) s'applique déjà aux deux.

**Graduations** : réparties régulièrement sur la course (contrat du peintre), leur texte est la valeur réelle à cette
position :

- contenu R1 quand il est exact (`cross.master` : 0 / .5 / 1 ; `perf.balance` : WT / 0 / FM) ;
- sinon bornes et milieu réels.

Les graduations de démonstration d'OUT GAIN (−48 / −24 / 0 / +6) ne sont pas exactes pour la plage linéaire réelle : elles
ne sont pas reprises.

### 2.3 Tests

| Test | Résultat (Windows) |
|---|---|
| PB-J1, PB-J2, PB-J3 (assertions DB2-08) | **PASS** : OUT GAIN reste à 0 dB, BALANCE reste à 0,5 ; 0 geste, 0 écriture, 0 transaction |
| PB-J4 | PASS : position absolue au premier clic, stable à la reprise, gestes appariés |
| PB-01c (assertion DB2-08) | **PASS** : saisie ouverte sans saut, 0 geste, 0 écriture |
| PB-01b (assertion DB2-09) | **PASS** : position absolue, 1 écriture, aucun défaut écrit, 1 transaction |
| **PLAS-Z** | **PASS**, 99 vérifications. Trois PLASMA, zooms 75, 100 et 150 % : libellé, valeur, hors canal, glisser commencé sur le libellé, bordure intérieure, extrémités, prise en main sans saut, suivi rapporté à la longueur, focus, appariement. |
| **EC19-FIX** | **PASS**, 118 vérifications. Liste complète de l'addendum §4, aux trois zooms et en instances multiples : réponse immédiate, aucun retour au défaut, aucune valeur intermédiaire, une écriture, une transaction, gestes appariés, deux positions, double-clic puis glisser, extrémités, automation active, Alt+clic / Suppr / menu, saisie sans écriture, infobulle. |
| **PLAS-G** | **PASS**. 36 PLASMA visibles, 12 R1 sans violation d'invariant ; les blocages ont tous une raison documentée. |
| Suite complète | Barrière **PASS** (85 critères, AUT-02 connu) ; UI04-CLIP PASS |

Deux corrections de banc, sans changement de critère :

- **Fenêtre** : à 150 %, une fenêtre de 1 440 × 900 réduit la surface logique sous le minimum de la vue, et la bande
  basse défile. Le banc garde 1 440 × 900 à 75 et 100 % (configurations UI04) et agrandit la fenêtre à 150 %.
- **Journal** : chaque sous-scénario a désormais son propre journal ; avant, les écritures de plusieurs sous-scénarios
  s'additionnaient.

Le banc a aussi révélé un vrai défaut, corrigé avant publication : la zone de manipulation 0.4.1 mordait de 4 px sur la
fenêtre de valeur.

### 2.4 Conformité visuelle

**Étape 2 (VIS-01)** : `cross.master` en STANDARD, sur les planches 12, 01, 06a et 09 (75, 100 et 150 %).

- 6 cas PLASMA : **structure R1 identique au pixel près**, seule la fenêtre de valeur diffère (format 0.4.1).
- Bilan Windows, NEXUS et PLASMA confondus : 48 cas, 0 DIFFÉRENTE, contrôles 7/7.

Cas non équivalents (exclus et justifiés) :

- OUT GAIN : graduations de démonstration inexactes ;
- BALANCE vertical et tout COMPACT : bloqués en production ;
- M2 et LVL : graduations de démonstration.

**Étape 3** : captures de production Windows [`CAPTURES/windows/b2-03-2-plasma/`](CAPTURES/windows/b2-03-2-plasma).

### 2.5 EC-03

OUT GAIN reste en rendu 0.4.1 (bloqué), donc sa colonne de mesure de crête reste pour l'instant dans le rail. La mesure
est déjà affichée à part, en texte, dans la même plaque (`crête L / R dBFS`). La séparation PEAK (MEAS) est réalisée en
B2-03.5 (HUD), conformément au registre ; aucune donnée n'est perdue entre-temps.

### 2.6 Invariants

- `git diff d2d08ac -- source/` vide ;
- `plugin/` inchangé ;
- aucun identifiant, format, seuil ou référence historique modifié ;
- `known_failures` inchangé.

## 3. B2-03.3 — NEURAL CELL

### 3.1 Intégration

`ui::NeuralCell` (bascules liées à un paramètre Bool, armements autonomes, états hors registre comme Drone) et
`ui::ActionCell` (actions momentanées et boutons à bascule) sont rendues par le peintre partagé `paintCell`
(Machined Titanium, LAB-A02). Correspondance :

| Production | R1 |
|---|---|
| Bascule Bool, état hors registre | **Toggle** : témoin encastré, état ON / OFF |
| Armement autonome | **Arm** : canal ambre, témoin anneau (ARMÉ) ou plein (ACTIF), croix (REFUS) |
| Action momentanée | **Action** : touche convexe, **icône d'impulsion par défaut** (repère R1 du geste momentané, qui remplace la barrette 0.4.1) ; Capture et Autorisation : icônes dédiées |
| ActionCell à bascule (Inspector, Épingler, Verrou) | **Toggle** : sans cela, la touche Action masquerait l'état |
| PANIC (danger) | Inchangé jusqu'à B2-03.6 |

**Inchangé** : comportement, clavier, focus, autorisation explicite par le clic d'armement (TA-07w), automation.

**Textes d'état** : la production garde les siens (OFF, ON, ARMÉ, ACTIF, REFUS, —). ARMED et ACTIVE sont des libellés P1
validés, appliqués en B2-04 ; le peintre reçoit le texte du consommateur, rien n'est anticipé.

**Correctif NEXUS** : le peintre écrivait en dur « INACTIVE », une proposition **P2 non validée** (state.inactive),
introduite en production par B2-03.1. Le texte devient un champ de la vue. La production affiche de nouveau « INACTIF »
(0.4.1) ; le laboratoire reste identique au pixel.

### 3.2 Gabarit et couverture (NC-G)

Le peintre écrit le libellé sur **une ligne** (corps 11 gras), dans une zone fixe de la face. Une cellule n'est rendue en
R1 que si son libellé y tient et si sa hauteur est d'au moins 24 px (face ≥ une ligne + 2). Sinon elle garde le rendu
0.4.1, raison publiée. Le composant R1 n'est ni redessiné ni simplifié.

| 12 configurations, tous contextes | Visibles | R1 | Rendu 0.4.1 |
|---|---|---|---|
| NeuralCell | 540 | 240 | 300 (libellés : Drone, Z Gain Compensation, Z Keep Fundamental…) |
| ActionCell (hors PANIC) | 4 128 | 1 000 | 3 128 (hauteur 22 : 660 ; libellés de production trop longs pour la zone à côté de l'icône) |

Raisons : [`PB/b2-03-3/windows/nc-g-blocages.csv`](PB/b2-03-3/windows/nc-g-blocages.csv).

La couverture des actions est limitée par deux facteurs :

- **l'icône d'impulsion**, qui retire 23 px à la zone du libellé : sans elle, 2 456 actions tiendraient ;
- **les libellés de production**, français et longs ; les libellés P1 de B2-04 sont courts.

Arbitrage **DB2-11** proposé, non bloquant ([B2-DECISIONS-PENDING.md](B2-DECISIONS-PENDING.md)).

### 3.3 Tests

| Test | Résultat (Windows) |
|---|---|
| **NC-G** | PASS : aucune cellule bloquée sans raison, aucun libellé tronqué |
| **NC-STATES** | PASS : 11 paires d'états distinctes au rendu R1 (OFF / ON / REFUS / indisponible, OFF / ARMÉ / ACTIF / REFUS, survol, pression, action pressée, bascule, action indisponible) |
| UI04-BIND, UI04-KEY, TA-07w et suite complète | Barrière **PASS** (87 critères, AUT-02 connu) ; UI04-CLIP PASS |

### 3.4 Conformité visuelle

**Étape 2 (VIS-01)** : 12 cas NEURAL CELL sur les planches 12, 01 et 11.

| Statut | Cas |
|---|---|
| **IDENTIQUE au pixel près** | 8 : FREEZE ×2, SYNC (état hors registre, domaine WT), CAPTURE, AUTHORIZE, EXCITE, RESET (domaine CROSS), APPLY WAVE |
| TEXTE D'ÉTAT | 3 : ARM (ACTIF / ARMÉ en production, ACTIVE / ARMED en référence : P1 de B2-04) |

Bilan total : 60 cas, **0 DIFFÉRENTE**, contrôles 7/7.

L'outil a été corrigé au passage :

- la clé du contrôle différentiel est désormais la position du cas (elle ignorait les cellules) ;
- un état hors registre ne consulte plus le registre des paramètres.

Avant correction, EXCITE et RESET étaient classés à tort « ÉTAT DU RENDU ». L'écart réel (icône d'impulsion manquante en
production) a été corrigé.

**Étape 3** : captures de production Windows [`CAPTURES/windows/b2-03-3-neural-cell/`](CAPTURES/windows/b2-03-3-neural-cell).

### 3.5 Densité visuelle

La matière usinée R1 n'apparaît que sur les cellules dont le libellé tient. Les zones denses (barre globale, listes de
l'espace ENGINEERING) gardent majoritairement le titane 0.4.1, jusqu'à B2-04 et à DB2-11. Aucune simplification du
composant n'est faite.
