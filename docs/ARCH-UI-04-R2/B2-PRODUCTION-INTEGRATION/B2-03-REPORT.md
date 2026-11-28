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

macOS : workflow Component Lab, résultats versés dans `VIS/b2-03/macos/`.

**Étape 3** (composant → interface complète) : captures de production Windows
[`CAPTURES/windows/b2-03-1-nexus/`](CAPTURES/windows/b2-03-1-nexus) (28 vues, zooms 75 et 150 % compris). Captures
macOS : workflow de preuves, jalon marqué `[captures]`.

### 1.6 Performances (GC-01) et CE-B2-06

Mesure comparative : workflow de preuves sur ce jalon, contre la référence `0517426`, avec le même banc (v3). Les
résultats sont ajoutés ci-dessous dès le run.

Revue CE-B2-06 du jalon :

- aucun timer ajouté : le minuteur de fin de molette existait déjà ;
- aucune lecture du fil audio ajoutée ; le marqueur de routes ne lit plus `modSerial` ;
- copie de la configuration de modulation sur le fil message, seulement quand elle change ;
- aucun rappel asynchrone ajouté ;
- aucune ressource graphique persistante.

### 1.7 Invariants

- `git diff d2d08ac -- source/` vide ;
- `plugin/` inchangé ;
- aucun identifiant, format, seuil, référence historique ou `known_failures` modifié.
