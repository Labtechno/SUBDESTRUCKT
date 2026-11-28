# ARCH-UI-04-R2-B1 — rapport de clôture (Component Lab)

Date : 2026-11-28. Base : HEAD distant vérifié `04dd7cfab4f3c1be38f41d0a75a5e6201c645371` (référentiel R2-A), dépôt
propre. DSP FINAL LOCK : `d2d08ac1bb6ed09ee3ea1b5e5d5d409c41b586af`.

## 1. Verdict

**GO technique pour la revue artistique.** Tous les critères de sortie techniques sont atteints (§12). Ce verdict ne
remplace pas la validation artistique du propriétaire. Il n'autorise ni R2-B2, ni R2-C, ni ARCH-UI-05. Points soumis à
l'arbitrage :

- libellés P1 (validation progressive, OD-02) ;
- libellé PANIC : blanc 0.4.1 à 3,84:1, sous le seuil WCAG de 4,5:1, ou variante carbone à 5,08:1 ;
- gabarit PLASMA vertical : 64 × 180 proposé, contre 48 × 150 en 0.4.1 ;
- OUT GAIN (recommandation de l'architecte).

## 2. Architecture du laboratoire

- **Isolement.** `lab/` est un projet CMake séparé. Il réutilise en lecture seule JUCE 9.0.6 (même commit), la cible
  `subdestruckt_core` et, pour le seul comparatif, les sources UI et processeur 0.4.1. Aucun fichier de production n'est
  modifié. Aucune cible de distribution, aucun traitement audio.
- **Composants R2** (`lab/r2/`) : candidats directs à l'intégration. Ils dépendent seulement de JUCE et de la palette
  0.4.1, via une `ValueSource` abstraite dont le contrat de geste est celui de `ParamLink`. Le comparatif lie déjà ces
  composants aux paramètres réels 0.4.1 par l'adaptateur `ParamSource` : aucune duplication n'est nécessaire en R2-B2.
- **Laboratoire** (`lab/app/`) : DEMO DATA instrumentée, 12 planches, PT-02, PT-05, contrastes, auto-tests
  LAB-G01..G13, planches-contact, fenêtre interactive.
- **Outils reproductibles** (`lab/tools/`, C02) :
  - `gen_labels.py` régénère le CSV R2-A à l'identique, octet pour octet ;
  - `build_review.py` produit la fusion des mesures et la revue P1 ;
  - `check_literals.py` contrôle les littéraux UTF-8.
- **CI** : `.github/workflows/component-lab.yml` exécute la même campagne sur macOS 15 arm64 (VM) et verse les résultats
  par un commit `[skip ci]`. `arch-g4.yml` est inchangé.

## 3. Composants réalisés

| Famille | Réalisation | Planches |
|---|---|---|
| NEXUS Hybrid | XL (moletage), S (biseau), Micro (lisibilité) ; uni et bipolaire ; WT/FM/CROSS/MOD ; arc BASE ; EFFECTIVE en anneau et losange ; marques de route ; valeur numérique | 02, 07, 08 |
| NEURAL CELL Hybrid | action (convexe, impulsion), interrupteur (plate, témoin), armement (liseré, anneau ou disque), sélection exclusive (carbone, losange), PANIC, désactivée ; FREEZE / ARM / AUTHORIZE / CAPTURE distincts | 03 |
| SYNAPTIC TRACK | segments, liste, onglets ; libellés courts et longs ; focus, survol, désactivé | 04 |
| Cybernetic Display | valeur, oscilloscope, spectre INDICATIVE, wavetable, courbe, enveloppe, crête et vumètre, état autonome, diagnostic | 05 |
| PLASMA CHANNEL Hybrid | vertical et horizontal, uni et bipolaire, curseur mécanique, canal BASE, triangle EFFECTIVE, paramètre ≠ mesure | 06 |
| États | Normal, Hover, Focus, Pressed, Selected, Disabled, Modulated, Min, Mid, Max, Negative | 07 |

Le détail est dans [COMPONENT-DESIGN-SPEC.md](COMPONENT-DESIGN-SPEC.md).

## 4. Résultats PT-02 (mesures typographiques)

2 213 occurrences, mesurées avec la police d'interface réellement résolue par JUCE, au corps retenu (12 px pour les 26
repères critiques), aux zooms 75, 100 et 150 %. Fichier : [TYPOGRAPHY-MEASUREMENTS.csv](TYPOGRAPHY-MEASUREMENTS.csv).

| Plateforme | Police | OK | Marge < 8 px | 2 lignes | Dépassement | Sans budget relevé |
|---|---|---|---|---|---|---|
| macOS 15.8.2 (VM) | Lucida Grande | 2 099 | 3 | 0 | **0** | 111 |
| Windows 11 | Verdana | 2 099 | 2 | 1 | **0** | 111 |

- Les verdicts sont identiques à 75, 100 et 150 % : les largeurs suivent le zoom linéairement.
- Les cas limites sont `Z · CROSS LVL` sur PLASMA V (marge de 1,0 px sous macOS ; deux lignes sous Windows) et
  `SPECTRAL` en segment (marge de 3,7 px sous macOS, 1,9 px sous Windows).
- Les 111 occurrences sans budget sont des titres, états, champs, unités et actions hors registre sans gabarit 0.4.1
  relevé par Codex. Leurs largeurs sont mesurées (142 px au plus) et leur budget sera fixé par R2-C.
- **C01** : l'estimation R2-A était juste en médiane (écart de +0,4 px sous macOS), mais fausse au cas par cas (de −19,7
  à +16,7 px). Les mesures remplacent donc l'estimation.
- Verdana est plus large que Lucida Grande : 3 % en médiane, de −8 % à +20 % selon le libellé.

## 5. Résultats PT-05 (glyphes) et contrastes

- **PT-05** : macOS, 16/16 glyphes natifs (`→ · … −` dans Lucida Grande et Menlo, normal et gras). Windows, 14/16 : `→`
  est absent de Verdana et JUCE le substitue par Segoe UI. Règle proposée : vérification au démarrage, puis repli ASCII
  global ([GLYPH-VALIDATION.md](GLYPH-VALIDATION.md)).
- **Contrastes** : 27 / 28 PASS ([DESIGN-TOKENS-R2.md](DESIGN-TOKENS-R2.md) §5). Le seul échec est le libellé PANIC 0.4.1
  (blanc sur rouge, 3,84:1). Je l'avais d'abord évalué au seuil de 3:1 des grands textes ; c'est une erreur, car 13 px gras
  reste un texte normal au sens WCAG, dont le seuil est 4,5:1. Le premier ajustement des reliefs (1,13:1 sous le seuil
  candidat de 1,15:1) a été corrigé (1,17 / 1,18 / 1,26).
- **Lisibilité** : plancher de 9 px écran à 75 % atteint par les 26 repères critiques (12 px logiques), sur les deux
  plateformes. Les libellés courants à 11 px tombent à 8,25 px à 75 %, ce que montre la planche 9.
- **Auto-tests** LAB-G01..G13 : 13/13 PASS sous macOS et sous Windows. Ils couvrent la discipline begin/end (glisser,
  double-clic sans geste imbriqué, molette, clavier, Alt+clic, destruction, masquage et détachement pendant un geste,
  mode fin), l'identifiant stable OD-01, l'absence d'écriture au rendu et les états des cellules.

## 6. Libellés P1 restant à arbitrer

Aucun des 2 213 libellés n'est validé. Les 323 occurrences P1 sont groupées dans
[P1-NAMING-REVIEW.md](P1-NAMING-REVIEW.md), en 11 groupes : distinctions critiques, autonomie, gel et capture, modes de
feedback, position WT et Z, macros, Core et PERFORMANCE, barre, ENGINEERING, réseaux, autres.

À arbitrer en premier :

- **WT Z**, **CROSS LVL** (et `Z · CROSS LVL`), **WT→FM**, **FM→WT** ;
- **BALANCE** (PRE-CHAIN), **XAUDIO AMT** ;
- **OUT GAIN** (recommandation de l'architecte), **PEAK** ;
- **AUTHORIZE**, **CAPTURE**, **RENDER**.

Signalements :

- 16 groupes d'homonymies, toutes désambiguïsées par un en-tête, à confirmer visuellement ;
- 6 redondances volontaires (WT Z, CROSS LVL hors de leur plaque) ;
- abréviations à valider : LVL, XAUDIO, DIR, KEEP F0, GAIN COMP, XFADE ;
- 36 budgets P1 non mesurés.

## 7. Limites techniques

- **Rendu.** Les captures sont des rendus logiciels JUCE hors écran (`createComponentSnapshot`). Dans le plugin, la
  fenêtre macOS passe par le moteur de rendu natif : l'antialiasing et le rendu des polices peuvent différer légèrement.
  La macOS utilisée est une **VM GitHub**, pas le Mac de référence. **Aucune recette physique** n'a été faite.
- **Windows** n'est pas la plateforme cible. Ses rendus servent de comparaison.
- **Fonctions incomplètes.** La saisie numérique et le menu de liste ne sont pas implémentés dans le laboratoire (les
  éléments 0.4.1 seront repris). Il n'y a pas d'interface de valeur accessible propre : les titres sont posés, AX-01
  sera vérifié en R2-B2.
- **Statique.** Aucune animation : les planches sont figées et toutes les données sont DEMO DATA. Le coût de rendu
  (GC-01) n'est pas mesuré.
- **Production non touchée.** Les auto-tests portent sur les composants R2 seulement. PB-01 et PB-02 ne sont ni reproduits
  ni corrigés en production (OD-10), mais leurs protocoles sont prêts.

## 8. Écarts avec les spécifications

Voir [COMPONENT-DESIGN-SPEC.md](COMPONENT-DESIGN-SPEC.md) §7 :

- gabarit PLASMA vertical 64 × 180 (proposition) ;
- survol marqué par une forme (halo, contour) ;
- variante de libellé PANIC ;
- `relief1` / `relief2` ajustés ;
- saisie numérique et liste simplifiées ;
- accessibilité de valeur non implémentée.

Aucun écart avec les invariants R2-A (INV-01 à INV-13).

## 9. Captures sur GitHub

Le [README](README.md) intègre les 12 planches macOS. Liens directs :

- macOS : [01 planche complète](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/01-component-lab-board.png)
  · [02 NEXUS](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/02-nexus.png)
  · [03 NEURAL CELL](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/03-neural-cell.png)
  · [04 SYNAPTIC TRACK](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/04-synaptic-track.png)
  · [05 Cybernetic Display](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/05-cybernetic-display.png)
  · [06 PLASMA](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/06-plasma-channel.png)
  · [07 états](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/07-interactive-states.png)
  · [08 couleurs](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/08-engine-colours.png)
  · [09 zooms](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/09-zoom-75-100-150.png)
  · [10 typographie](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/10-critical-typography.png)
  · [11 comparatif 0.4.1 / R2](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/11-compare-041-r2.png)
  · [12 plaques](https://github.com/Labtechno/SUBDESTRUCKT/blob/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/macos/12-plate-grouping.png)
- Windows : [dossier](https://github.com/Labtechno/SUBDESTRUCKT/tree/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/windows)
- Planches-contact : [dossier](https://github.com/Labtechno/SUBDESTRUCKT/tree/feature/arch-g4-full-development/docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CONTACT-SHEETS)

## 10. SHA publiés

| Commit | Contenu |
|---|---|
| `a8a172b` | laboratoire, workflow, rendus et mesures Windows |
| `3aab1bd` | résultats macOS, component-lab run 1 (commit du workflow, `[skip ci]`) |
| `ab06b8c` | identifiants OD-01, variante PANIC, revue P1 |
| `09758ba` | résultats macOS, component-lab run 2 (commit du workflow, `[skip ci]`) |
| `1530687` | documentation B1, arbitrages OD, corrections C01 à C06, tracker, correction d'une légende de la planche 03 |
| `786c6ad` | résultats macOS, component-lab run 3 sur le code final (commit du workflow, `[skip ci]`) |

**SHA des preuves finales B1 : `786c6ad8f65f573b61c6389303e3f971ed88250a`** (HEAD distant vérifié à la clôture de B1).

## 11. Vérification du DSP FINAL LOCK et de la production

- `git diff d2d08ac -- source/` : **vide**.
- `git diff 04dd7cf -- source ui plugin tests cmake CMakeLists.txt tools packaging manual .github/workflows/arch-g4.yml
  .github/workflows/arch-g3.yml` : **vide**. Seuls des fichiers nouveaux sont ajoutés (`lab/`, `component-lab.yml`,
  `docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/`). Le reste concerne la documentation : référentiel R2, tracker et erratum du
  DSP FINAL LOCK.
- CI de production `arch-g4.yml` (plugin, tests du cœur et du wrapper, barrière, pluginval, RTSan, paquets, script de
  build local, manuel) :
  - run 51 sur `a8a172b` : **SUCCESS** ;
  - run 53 sur `1530687` : **SUCCESS** ;
  - run 52 sur `ab06b8c` : annulé par GitHub, remplacé par un run plus récent dans le même groupe de concurrence (pas un échec).
- Component Lab macOS (`component-lab.yml`) : runs 1, 2 et 3 en **SUCCESS** ; résultats versés en `3aab1bd`, `09758ba`
  et `786c6ad`.
- Version de production : 0.4.1, inchangée. StableID, registre et format d'état inchangés.

## 12. Critères de sortie (brief §13)

| Critère | Résultat |
|---|---|
| Toutes les familles graphiques représentées | PASS (5 familles, 9 variantes d'afficheur) |
| Direction artistique conforme aux arbitrages | à valider par le propriétaire (planches 01 à 12) |
| États interactifs démontrés | PASS (planche 07) |
| PT-02 exécuté et documenté | PASS (2 213 × 2 plateformes) |
| PT-05 exécuté et documenté | PASS (macOS 16/16 ; Windows 14/16 documenté) |
| Libellés P1 regroupés pour validation | PASS (323 occurrences, 11 groupes) |
| Contrastes mesurés | PASS (mesurés : 27/28, l'échec est documenté) |
| Plancher typographique critique vérifié | PASS (26/26, deux plateformes) |
| Aucune régression dans le plugin de production | PASS : aucun fichier de production modifié ; CI de production 51 et 53 SUCCESS |
| DSP FINAL LOCK intact | PASS |
| Captures et preuves disponibles sur GitHub | PASS |

## 13. Proposition de périmètre R2-B2 (non autorisé)

1. **Pré-requis.** Arbitrer les libellés P1 (OD-02), le libellé PANIC, le gabarit PLASMA vertical et OUT GAIN.
2. **Reproduction d'abord (OD-10).** Committer les tests BE-01 à BE-05 sur le code 0.4.1 (PB-01 : NEXUS double-clic ;
   PB-02 : Core et éditeur d'enveloppe détruits, masqués ou redimensionnés pendant un geste). Les échecs attendus sont
   documentés avant toute correction.
3. **Intégration.** Placer les composants R2 dans `ui/` avec l'adaptateur `ParamSource` / `ParamLink`. Intégrer les tokens
   R2 à `DesignTokens.h`. Remplacer les composants 0.4.1 famille par famille. Garder les dispositions existantes : R2-C
   reste un lot distinct.
4. **Nomenclature.** Table d'alias d'affichage (P1 validés d'abord). Toutes les chaînes visibles en anglais (NAM-01).
   Titres accessibles inchangés. Vérification des glyphes avec repli ASCII (GL-01 à GL-05).
5. **Tests (OD-01).** Registre d'identifiants. Migration de UI-01w, AUT-02, AUT-08, TA-07w et UI04-OPS **sans
   affaiblir** les assertions. Nouveaux tests NAM-01, NAM-03 et ID-UNIQ. La couverture des outils ENGINEERING s'élargit.
6. **Afficheurs.** Le moniteur de production adopte les variantes Cybernetic : spectre INDICATIVE (C03) ; AUTONOMY avec
   STOP, SUSPENDED et silence séparés (C04).
7. **Livraison 0.5.0** : CI SUCCESS, barrière d'inventaire, recette physique ciblée sur le Mac de référence (NC-03,
   NC-05, NC-07, glyphes, plancher de 9 px), validation artistique.

Hors de R2-B2 : nouvelles dispositions (R2-C), regroupement ENGINEERING et Inspector adaptatif (R2-E), réseaux (R2-F),
flux réactif (R2-G), Core 3D (ARCH-UI-05).

## Note de clôture (ajoutée en R2-B1-R1, 2026-11-28)

Ce rapport est autonome. B1 a obtenu son GO technique après la contre-expertise indépendante de ChatGPT. Les huit
arbitrages artistiques du propriétaire (LAB-A01 à LAB-A08) sont appliqués dans
[`../B1-R1-FINAL-ARTISTIC-REFINEMENT/`](../B1-R1-FINAL-ARTISTIC-REFINEMENT/README.md). Les preuves B1 de ce dossier sont
conservées sans modification pour la comparaison historique.

**Erratum (constaté en R2-B1-R1).** La planche-contact Windows de ce dossier
(`CONTACT-SHEETS/contact-sheet-windows.png`) est entièrement noire. L'outil écrivait le PNG avant la destruction du
contexte de dessin. Sous Windows (Direct2D, dessin différé), l'image n'était donc jamais remplie ; macOS n'est pas
affecté. Les douze captures Windows individuelles de `CAPTURES/windows/` sont correctes et font foi. L'outil est corrigé
en `50cc8aa`. Une planche-contact régénérée à partir de ces mêmes captures, sans les modifier, est publiée dans
[`../B1-R1-FINAL-ARTISTIC-REFINEMENT/CONTACT-SHEETS/contact-sheet-windows-B1.png`](../B1-R1-FINAL-ARTISTIC-REFINEMENT/CONTACT-SHEETS/contact-sheet-windows-B1.png).
