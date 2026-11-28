# ARCH-UI-04-R2-B2 — conformité visuelle (CE-B2-03)

Les captures R1 historiques sont des **références indépendantes et immuables** :
[`../../B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES/`](../../B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES/).

- Windows : rendu local, code `50cc8aa`.
- macOS : VM GitHub, component-lab run 5, code `7cbfa43`.

Aucun outil ni workflow ne les réécrit. Depuis R2-B2, `component-lab.yml` ne verse plus de captures dans ce dossier.

## Chaîne de validation

| Étape | Comparaison | Outil | Statut |
|---|---|---|---|
| 1 | Référence R1 historique → Component Lab rendu avec les **peintres partagés** (`ui/r2/`) | `lab/tools/compare_r1_refs.py` | **VALIDÉE** : Windows 15/15 et macOS 15/15 identiques, 0 pixel différent sur l'image entière (contrôle strict) ; macOS : component-lab run 6 sur `0d576ed` ([`lab/macos/`](lab/macos)) |
| 2 | Référence R1 historique → composant intégré en production (même taille, même état, mêmes données) | VIS-01 : `sbd_lab_compare --vis01` et `lab/tools/vis01_compare.py` | **NEXUS : VALIDÉE (Windows)** : 0 DIFFÉRENTE sur 42 cas, contrôles 6/6 ; macOS : [`b2-03/macos/`](b2-03) |
| 3 | Composant intégré → interface complète (captures de production) | Captures de production (B2-03, B2-08) | à venir |

## Étape 1 : méthode

Le laboratoire rend ses 15 planches R1 avec les peintres extraits dans `ui/r2/` : `R2Style`, `R2Painters`, `R2Hud`.
Les corps de rendu ont été transférés par programme, sans modification ; seules les références aux membres sont devenues
des champs de vue. Deux comparaisons sont faites.

| Comparaison | Rendu | Zone comparée | Résultat Windows |
|---|---|---|---|
| (a) | SHA courant dans le bandeau | Tout sauf le bandeau d'en-tête (132 lignes) | 13/15 identiques. Les 2 planches composites (02, 06c) diffèrent de 3 pixels, ligne 132 : c'est le bandeau de la planche B1 incrustée (réduite à 50 %, placée à y = 112), qui porte son propre SHA. **Écart légitime d'en-tête.** |
| (b) **contrôle strict** | Chaîne de SHA de la référence (`--sha 50cc8aa` ; macOS `--sha 7cbfa43`) | **Images entières** | **15/15 identiques, 0 pixel différent** (macOS : idem) |

Fichiers : [`lab/windows/compare-r1.txt`](lab/windows/compare-r1.txt), `compare-r1-hors-bandeau.csv`,
`compare-r1-images-entieres.csv`.

Une comparaison entre deux composants qui utilisent le même peintre ne suffit pas (CE-B2-03). L'étape 1 compare donc le
**laboratoire** à des **références historiques produites avant l'extraction**.

## Étape 2 : méthode (VIS-01, B2-03)

Chaque planche historique (01, 09 à 75, 100 et 150 %, 11, 12) est **reconstruite par son code de laboratoire**, prouvé
identique aux références à l'étape 1. Le composant R1 du laboratoire visé est masqué. À sa place, avec les mêmes bornes et
le même rang de dessin, on pose le **composant de production** lié au paramètre réel correspondant :

- même valeur normalisée affichée, à l'ulp près (l'ajustement éventuel est publié) ;
- même libellé ;
- même Effective, injectée dans la télémétrie publiée, moteur arrêté : c'est un état représentatif, déclaré comme tel ;
- mêmes routes, posées par la transaction de production puis recopiées par l'éditeur.

La planche est rendue à l'échelle de la référence, avec la chaîne de SHA de la référence, **une planche par processus**.
Elle est ensuite comparée à la capture historique.

| Contrôle | Règle |
|---|---|
| Hors substitution | L'image doit être identique à la référence (sauf la zone de production déclarée de la planche 12). Sinon la comparaison est **INVALIDE**. |
| IDENTIQUE | Aucun pixel différent dans les bornes du composant |
| TEXTE DE VALEUR | Reste du composant identique au pixel près. Seule la fenêtre de valeur diffère, et le texte de production diffère de celui de la référence : donnée non équivalente, la comparaison pixel à pixel de la fenêtre ne s'applique pas. |
| ETAT DU RENDU | Le reste du composant diffère, mais le **contrôle différentiel** (même planche, ce seul cas laissé au composant du laboratoire) montre au moins les mêmes pixels différents, avec au moins le même écart, sur le composant R1 du laboratoire lui-même |
| DIFFERENTE | Tout autre cas : jamais déclaré conforme |

**Particularités Windows (Direct2D)** :

- Direct2D met en cache la géométrie des tracés répétés, ce qui change l'anticrénelage des occurrences suivantes.
- L'ordre de dessin est donc conservé : le composant de production prend le rang du composant remplacé.
- La planche 11 historique avait été rendue après les planches 01 à 10 du même processus ; VIS-01 rejoue le même
  historique avant de la rendre. Sans cela, 75 px des coins arrondis de deux autres composants diffèrent (écart maximal
  12/255). Le contrôle hors substitution l'a détecté.

## Étape 2 : résultats NEXUS (B2-03.1)

Windows : [`b2-03/windows/vis01-bilan.txt`](b2-03/windows/vis01-bilan.txt) (détail : `vis01-results.csv`,
`vis01-controle.csv`, planches référence | production | écart dans `ecarts/`).

| | Windows |
|---|---|
| Cas | 42 : planche 12 (paramètres réels, texte hôte) ×4 ; planche 01 ×28 ; planche 11 ×1 ; planche 09 ×3 à trois zooms ; gabarits XL, S, Micro ; uni et bipolaires ; Effective ; routes |
| Contrôles hors substitution | **6/6 identiques** |
| IDENTIQUE | 3 |
| TEXTE DE VALEUR | 38 : la structure R1 est identique au pixel près, seule la donnée affichée diffère (format 0.4.1 de production : `0.486 pos` au lieu de `0.49`, voir ci-dessous) |
| ETAT DU RENDU | 1 : `dst.wt1.d2.fb`, 1 pixel, écart 2/255 ; même pixel sur le composant du laboratoire dans les mêmes conditions |
| DIFFERENTE | **0** |

**Format des valeurs** : la production garde le format 0.4.1 de `formatValue`, avec l'unité `lin`, `pos` ou `bip` et
trois décimales. Les références R1 masquent l'unité des domaines normalisés (NAM-11) et utilisent les formats proposés au
§6 de la nomenclature. Ces formats sont des propositions P3-01, à valider dans la revue groupée de B2-04 : ils ne sont
pas anticipés. Les cas ajustés à l'ulp ou à norme légèrement différente (`op3.ratio`, `op4.ratio`, `lfo1.rate` : écart
≤ 8·10⁻⁷, plages non linéaires) sont publiés dans `vis01-cases.csv` ; leur structure est identique.

Données non équivalentes exclues : `dst.wt1.d2.speed`. Le paramètre réel est bipolaire (−4..+4) alors que la
démonstration est unipolaire.

## Étape 2 : résultats PLASMA (B2-03.2)

Cas équivalents : `cross.master` en STANDARD 64 × 180 (CROSS LVL, graduations R1 0 / .5 / 1, exactes pour la plage
réelle), sur les planches 12 (valeur réelle), 01 et 06a (Effective 0,78) et 09 (75, 100 et 150 %).

| | Windows |
|---|---|
| Cas PLASMA | 6 : **TEXTE DE VALEUR** (structure R1 identique au pixel près : canal, graduations, Effective, curseur, sérigraphie) |
| Bilan total (NEXUS + PLASMA) | 48 cas ; IDENTIQUE 3, TEXTE DE VALEUR 44, ETAT DU RENDU 1, **DIFFÉRENTE 0** ; contrôles **7/7** |

Exclus et justifiés :

- OUT GAIN : graduations de démonstration −48 / −24 / 0 / +6, inexactes pour la plage linéaire réelle ;
- BALANCE vertical et tout COMPACT : gabarit R1 bloqué en production, CE-B2-02 (la valeur 0.4.1 ne tient pas) ;
- M2 et LVL : graduations de démonstration.

Planche 06a : l'historique de rendu de la référence (planches 01 à 05b du même processus) est rejoué avant la planche.

## Étape 2 : résultats NEURAL CELL (B2-03.3)

12 cas, planches 12, 01 et 11 :

| Statut | Cas |
|---|---|
| IDENTIQUE au pixel près | 8 : FREEZE ×2, SYNC (hors registre, WT), CAPTURE, AUTHORIZE, EXCITE, RESET (CROSS), APPLY WAVE |
| TEXTE D'ÉTAT | 3 : ARM, ACTIF / ARMÉ contre ACTIVE / ARMED (libellés P1, B2-04) |

Bilan total (NEXUS, PLASMA, NEURAL CELL) : **60 cas, 0 DIFFÉRENTE, contrôles 7/7**.

La zone de donnée d'une cellule est celle du texte d'état, à droite de la face (50 px). Les états imposés (ARMÉ sans
autorisation) sont des états représentatifs, déclarés dans `vis01-cases.csv`.

## Règle de données non équivalentes (2026-11-28)

Le laboratoire macOS du jalon PLASMA (run 9, `28024df`) a donné 47 cas conformes sur 48 et un cas DIFFÉRENTE :
`lfo1.rate`, 9 px à 1/255 d'écart hors fenêtre de valeur. La norme affichée par la production y différait de 8·10⁻⁷ de
celle de la référence, faute d'égalité trouvée dans la recherche (±16 ulps). La donnée n'était donc pas strictement
équivalente.

Correctif de méthode, qui ne modifie aucun critère de conformité :

- recherche portée à ±4 096 ulps ;
- tout cas dont la norme reste différente est classé **DONNÉE NON ÉQUIVALENTE**, hors comparaison et listé ; il n'est
  jamais déclaré conforme.

Windows : `op3.ratio`, `op4.ratio` et `lfo1.rate` sont dans ce cas (plages non linéaires ou réelles différentes de la
démonstration). Bilan : 60 cas ; IDENTIQUE 12, TEXTE DE VALEUR 44, ÉTAT DU RENDU 1, DONNÉE NON ÉQUIVALENTE 3,
**DIFFÉRENTE 0** ; contrôles 7/7.

