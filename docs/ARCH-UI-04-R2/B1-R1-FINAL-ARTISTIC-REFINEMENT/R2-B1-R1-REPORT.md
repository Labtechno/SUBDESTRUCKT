# ARCH-UI-04-R2-B1-R1 — rapport de clôture (raffinement artistique final)

Date : 2026-11-28. HEAD de départ vérifié : `786c6ad8f65f573b61c6389303e3f971ed88250a`, dépôt propre, sans divergence.
Référentiel R2-A : `04dd7cf`. DSP FINAL LOCK : `d2d08ac`. Version de production inchangée : 0.4.1.

## 1. Verdict technique

**GO technique pour la validation artistique finale.** Les huit arbitrages LAB sont appliqués. Les critères techniques
AR-01 à AR-14 sont atteints. **AR-15** (validation artistique du propriétaire) reste une décision humaine **en
attente** ; l'agent ne la déclare pas. R2-B2, R2-C et ARCH-UI-05 restent interdits. Aucun développement R2-B2 n'est
engagé.

## 2. Couverture des huit arbitrages

| ID | Option | Application | Preuve principale |
|---|---|---|---|
| LAB-A01 | A — NEXUS conservés | Composant et primitives inchangés | Non-régression pixel à pixel, macOS et Windows : **0 pixel différent** |
| LAB-A02 | B — Machined Titanium | Alvéole, chanfrein diagonal, face bombée ou en cuvette, témoins encastrés, course d'un pixel | `03-neural-cell-before-after.png` |
| LAB-A03 | B — Machined Track | Rail encastré, sélecteur en titane, marqueur du domaine encastré, crans usinés | `04-synaptic-track-before-after.png` |
| LAB-A04 | C — Cybernetic HUD | Neuf variantes : verre profond, monture, rails, réticules, pastilles BASE, EFF, MEAS, INDICATIVE et STATE | `05a`, `05b` |
| LAB-A05 | A — PLASMA conservé | Chemin STANDARD intact | Non-régression pixel à pixel : **0 pixel différent** |
| LAB-A06 | C — Symbiotic Industrial Hybrid | Graphite mat, titane sombre, mécanique usinée, verre cybernétique, couleurs fonctionnelles ; aucune matière biomécanique | `01`, `08`, `11` |
| LAB-A07.1 | B — PANIC carbone | Libellé carbone ; face jamais plus sombre que le rouge nominal sous le libellé | `07-panic-final.png`, `panic-contrast.csv` |
| LAB-A07.2 | C — PLASMA STANDARD + COMPACT | 64 × 180 et 48 × 150, même composant | `06a`, `06b`, LAB-G15 |
| LAB-A08 | B — nomenclature P1 optimisée | AMOUNT, MODE, en-tête CROSS AUDIO, KEEP FUND, BUS / CROSS AUDIO, OUT GAIN | `P1-NAMING-VALIDATION.md`, `10-critical-nomenclature.png` |

Détail : [LAB-ARTISTIC-DECISIONS.md](LAB-ARTISTIC-DECISIONS.md).

## 3. Modifications par famille

| Famille | Modification | Fichiers |
|---|---|---|
| NEXUS | aucune | — |
| PLASMA | branche COMPACT seulement (largeur < 56 : gorge décalée d'1 px, libellés de graduation limités aux bornes et au zéro) | `R2PlasmaChannel.*` |
| NEURAL CELL | rendu R1 (`paintR1`), primitives `titaniumKey` et `recessedLed` ; libellé PANIC carbone ; option de mesure « libellé masqué » | `R2NeuralCell.*`, `R2Style.*` |
| SYNAPTIC TRACK | rendu R1 (`paintR1`), primitives `machinedGroove` et `trackSelector` | `R2SynapticTrack.*`, `R2Style.*` |
| Cybernetic Display | rendu R1 des neuf variantes (`R2CyberHud.cpp`), primitives `hudFrame` et `hudChip`, nature MEAS pour une valeur mesurée, afficheur de barre compact | `R2CyberDisplay.*`, `R2CyberHud.cpp`, `R2Style.*` |
| Laboratoire | planches R1, comparatifs avant / après dans la même exécution, non-régression pixel à pixel, mesure PANIC sur surface rendue, LAB-G14 et LAB-G15, SHA de rendu en en-tête | `lab/app/*` |
| Outils | `gen_labels_r1.py` (révision R1), `build_validation_r1.py` (validation P1, fusion PT-02, rapport de contraste) | `lab/tools/` |
| Corrections (`50cc8aa`) | planche 07 : chevauchement de la barre globale sur 1 280 px supprimé ; planche-contact : contexte de dessin fermé avant l'écriture du PNG (la planche-contact Windows de B1 était noire, voir l'erratum du rapport B1) | `lab/app/SheetsR1.cpp`, `lab/app/Main.cpp` |

La révision B1 des composants est conservée (`setRevision(Rev::B1)`) uniquement pour les comparatifs historiques.

## 4. Comparatifs visuels avant / après

| Planche | Méthode | Résultat |
|---|---|---|
| `02-nexus-nonregression.png` | Planche B1 re-rendue par le code R1, comparée **pixel à pixel** à la capture B1 publiée de la même plateforme (hors bandeau d'en-tête) | macOS : 0 / 4 095 040 ; Windows : 0 / 4 095 040 → **identique** |
| `06c-plasma-nonregression.png` | idem, planche PLASMA de B1 | macOS : 0 / 3 076 640 ; Windows : 0 / 3 076 640 → **identique** |
| `03-neural-cell-before-after.png` | B1 et R1 rendus dans la même exécution : six traitements, FREEZE / ARM / AUTHORIZE / CAPTURE, états (normal, survol, appui, focus, désactivé), armement, PANIC | voir la planche |
| `04-synaptic-track-before-after.png` | idem : segments, listes, onglets, états | voir la planche |
| `05b-cybernetic-hud-before-after.png` | idem : neuf variantes, 440 × 210, mêmes DEMO DATA | voir la planche |

## 5. PT-02 et PT-05

PT-02 a été exécuté sur la révision R1 : 2 214 occurrences (l'en-tête CROSS AUDIO est ajouté), mesures JUCE réelles aux
zooms 75, 100 et 150 %.

| Plateforme | Police | OK | Marge < 8 px | 2 lignes | Dépassement | Budget indéterminé | Repères critiques ≥ 9 px à 75 % |
|---|---|---|---|---|---|---|---|
| macOS 15.8.2 (VM) | Lucida Grande | 2 099 | 3 | 0 | **0** | 112 | **26 / 26** |
| Windows 11 | Verdana | 2 099 | 2 | 1 | **0** | 112 | **26 / 26** |

- **Nouveaux libellés** (macOS / Windows, px / budget) : AMOUNT 41,0 / 39,9 sur 80 ; MODE 27,4 / 27,5 sur 164 ;
  KEEP FUND 49,7 / 51,9 sur 99 ; CROSS AUDIO 66,5 / 71,8 et BUS / CROSS AUDIO 96,0 / 104,9 (budgets indéterminés).
- **AR-10 : aucun nouveau dépassement** dans les gabarits mesurables.
- **PT-05** : macOS 16 / 16 glyphes natifs ; Windows 14 / 16 (`→` substitué dans Verdana). Le repli ASCII OD-11 est
  conservé.

## 6. Contrastes, dont PANIC

Détail : [CONTRAST-REPORT.md](CONTRAST-REPORT.md).

- **Paires de tokens** : 37 / 38 PASS. Le seul échec est le libellé blanc historique de 0.4.1 et B1 (3,84:1), conservé
  comme référence. Les nouvelles paires R1 passent toutes :
  - faces de titane ;
  - touches plates ;
  - verre HUD ;
  - pastilles.
- **PANIC carbone** (AR-07) : contraste mesuré **sur la surface rendue**, pixel par pixel dans la zone du libellé
  (touche rendue sans libellé), en 4 états (normal, survol, appui, focus) et à 2 échelles.
  - Pire cas : **5,08:1 sur macOS et sur Windows**, pour 4,5:1 exigé ; LAB-G14 PASS.
  - Le minimum correspond au rouge nominal à l'appui : la face n'y est jamais plus sombre que `alert`.

## 7. PLASMA STANDARD et COMPACT

- **Même composant.** Le gabarit est choisi par la largeur, avec le même contrat d'interaction. LAB-G15 PASS : glisser,
  double-clic sans imbrication, redimensionnement STANDARD ↔ COMPACT pendant un geste, destruction ; 3 begin / 3 end.
- **COMPACT** conserve la valeur (« −6.0 dB » tient), l'identification (deux lignes au corps de 11 px, sans réduction),
  la BASE, le triangle EFFECTIVE, les graduations essentielles et la zone de manipulation.
- **Zooms.** Planche `06b` à 75, 100 et 150 %, en pixels écran ×1.
- **Emplacements et seuils** de disposition : renvoyés à R2-B2 et R2-C.

## 8. État de validation des occurrences P1

| Statut | Nombre |
|---|---|
| **Lexique validé (LAB-A08)** | 324 : les 323 occurrences P1 et l'en-tête CROSS AUDIO ajouté |
| Géométrie macOS (cible) | 286 PASS ; 1 PASS avec marge < 8 px (`Z · CROSS LVL`, 1,0 px) ; 37 INDÉTERMINÉS |
| Géométrie Windows | 286 PASS ; 1 PASS en deux lignes (`Z · CROSS LVL`) ; 37 INDÉTERMINÉS |

Contrôles exigés (brief §12.4), détaillés dans [P1-NAMING-VALIDATION.md](P1-NAMING-VALIDATION.md) :

- **Homonymies** : les 16 groupes sont revus et toujours levés par un en-tête ; aucune n'est créée par AMOUNT ou MODE.
- **Redondances** : les 6 sont volontaires (WT Z, CROSS LVL).
- **Marges faibles** : `Z · CROSS LVL`, `SPECTRAL` (×2).
- **Deux lignes** : `Z · CROSS LVL` sous Windows.
- **Nouveaux libellés** : mesurés (§5).
- **Repères critiques** : 26 / 26.

Traçabilité individuelle : [P1-TRACEABILITY.csv](P1-TRACEABILITY.csv). P2 et P3 restent sous OD-02.

## 9. Budgets encore indéterminés

37 occurrences P1 n'ont pas de gabarit défini : 36 en B1, plus l'en-tête CROSS AUDIO. Sur l'ensemble des priorités, il
y en a 112. Elles ne sont **pas déclarées PASS**. Leurs largeurs sont mesurées et elles seront vérifiées dans les lots
qui fixent leur gabarit :

- barre, états et Core : R2-B2 / R2-C ;
- titres et onglets de SOUND DESIGN : R2-D ;
- ENGINEERING et Inspector : R2-E.

Liste : P1-NAMING-VALIDATION.md §4.

## 10. Auto-tests

LAB-G01 à LAB-G15 : **15 / 15 PASS** sur macOS et sur Windows. Les 13 tests de B1 sont repris sans modification, et
deux s'y ajoutent :

- **LAB-G14** : contraste PANIC sur la surface rendue ;
- **LAB-G15** : contrat partagé des deux gabarits PLASMA.

Contrôles associés :

- contrôle UTF-8 (`check_literals.py`) : 0 littéral fautif ;
- identifiant stable OD-01 (LAB-G11) : PASS ;
- absence d'écriture pendant le rendu (LAB-G12) : PASS.

## 11. DSP FINAL LOCK

`git diff d2d08ac -- source/` : **vide**.

## 12. Chemins de production

`git diff 786c6ad -- source ui plugin tests cmake CMakeLists.txt tools packaging manual .github/workflows/arch-g4.yml
.github/workflows/arch-g3.yml` : **vide**.

Fichiers modifiés :
- `lab/` ;
- le workflow du laboratoire `component-lab.yml`, réorienté vers le dossier B1-R1 ;
- la documentation : référentiel R2, tracker, ce dossier et la nomenclature R1.

Les tests historiques ne sont pas touchés.

## 13. CI

| Workflow | Run | Commit | Résultat |
|---|---|---|---|
| Component Lab (macOS 15, laboratoire) | 4 | `e896dba` | **SUCCESS** : résultats versés en `7c68a27` (remplacés par le run 5) ; NEXUS et PLASMA identiques, SELFTEST 15 / 15 |
| Component Lab (macOS 15, laboratoire) | 5 | commit de documentation (code `50cc8aa`) | EN COURS (complété au commit suivant) |
| ARCH-G4 (production : plugin, tests, pluginval, RTSan, paquets) | 54 | `e896dba` | EN COURS (complété au commit suivant) |
| ARCH-G4 (production) | 55 | commit de documentation | EN COURS (complété au commit suivant) |

## 14. SHA final distant

Consigné dans le message de clôture, après la publication de la documentation et la vérification `ls-remote`. Un
fichier ne peut pas contenir le SHA du commit qui l'introduit. Résultat de la CI de production 54 : consigné au même
endroit.

## 15. Liens directs

- Dossier : https://github.com/Labtechno/SUBDESTRUCKT/tree/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT
- Captures macOS : https://github.com/Labtechno/SUBDESTRUCKT/tree/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES/macos
- Planche-contact macOS : https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/CONTACT-SHEETS/contact-sheet-macos.png
- Nomenclature R1 : https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv

## 16. Réserves avant R2-B2

1. **AR-15 en attente.** La validation artistique finale appartient au propriétaire.
2. **Planche-contact Windows de B1** noire (erratum du rapport B1) ; les captures individuelles font foi et une
   planche-contact régénérée est fournie.
3. **Pas de recette physique.** Les rendus macOS viennent d'une VM GitHub en rendu logiciel hors écran. Ce n'est ni une
   recette sur le Mac de référence, ni une recette dans Reason. Le rendu natif de la fenêtre du plugin peut différer
   légèrement (antialiasing, polices).
4. **37 budgets P1 indéterminés** (112 toutes priorités), à vérifier dans leurs lots. `Z · CROSS LVL` tient à 1,0 px sous
   macOS et passe sur deux lignes sous Windows.
5. **Simplifications du laboratoire** non reprises telles quelles : saisie numérique, menu de liste, interface de valeur
   accessible (AX-01 à vérifier en R2-B2).
6. **Coût de rendu** des primitives R1 (dégradés, stries, rails) non mesuré (GC-01) ; à mesurer à l'intégration.
7. **PB-01 et PB-02** restent au backlog : reproduction préalable obligatoire en R2-B2 (OD-10).
8. **Restriction inter-domaines** des transformations Z toujours non arbitrée. LAB-A08 ne la tranche pas.
