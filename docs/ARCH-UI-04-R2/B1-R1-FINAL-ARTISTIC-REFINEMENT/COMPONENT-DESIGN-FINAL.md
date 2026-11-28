# ARCH-UI-04-R2-B1-R1 — spécification finale des composants (avant intégration R2-B2)

Ce document est la référence de conception remise à R2-B2. Il complète
[`../B1-COMPONENT-LAB/COMPONENT-DESIGN-SPEC.md`](../B1-COMPONENT-LAB/COMPONENT-DESIGN-SPEC.md), dont il conserve
l'architecture (`ValueSource`, `GestureGuard`, adaptateur `ParamLink`). Il intègre les arbitrages LAB-A01 à LAB-A08.
Code : `lab/r2/`. Révision de rendu par défaut : **R1**. La révision B1 est conservée pour les comparatifs historiques
(`setRevision(Rev::B1)`) ; elle n'est pas destinée à l'intégration.

## 1. Familles et statut

| Famille | Statut | Code |
|---|---|---|
| NEXUS Hybrid XL / S / Micro | **validé, gelé** (LAB-A01) | `R2Nexus.*` (sans révision) |
| PLASMA CHANNEL | **apparence validée** (LAB-A05) + gabarit COMPACT (LAB-A07.2) | `R2PlasmaChannel.*` |
| NEURAL CELL | **Machined Titanium** (LAB-A02) | `R2NeuralCell.*` (`paintR1`) |
| SYNAPTIC TRACK | **Machined Track** (LAB-A03) | `R2SynapticTrack.*` (`paintR1`) |
| Cybernetic Display | **Cybernetic HUD** (LAB-A04) | `R2CyberDisplay.*`, `R2CyberHud.cpp` |
| PANIC | **libellé carbone** (LAB-A07.1) | `R2NeuralCell.cpp`, kind `Safety` |

## 2. NEURAL CELL — Machined Titanium

| Élément | Règle |
|---|---|
| Alvéole | Gorge `groove` élargie de 2 px autour de la touche, lèvre basse éclairée (`bevelLight` 30 %) |
| Chanfrein | Largeur 2,3 px, dégradé diagonal : `bevelLight` éclairci en haut à gauche, quasi noir en bas à droite. Inversé à l'appui. Plus clair au survol. |
| Face | Action : bombée (`titanium` #343C43, éclaircie de 34 % en haut, assombrie de 38 % en bas, bande spéculaire ≤ 9 %). Bascule et armement : en cuvette (`relief2`, ombre interne haute). Stries d'usinage horizontales à 3 % d'opacité. |
| Course physique | Au repos : ombre portée de 2,2 px. À l'appui : face décalée de 1 px vers le bas, ombre supprimée, libellé et témoin suivent la face. |
| Témoins | Logement percé (arête sombre en haut, claire en bas), lentille avec reflet ; éteint, anneau (ARMED) ou plein (ON, ACTIVE), lueur ≤ 0,35 |
| Armement | Canal ambre incrusté dans la face (`warn`, 1,3 px), état ARMED / ACTIVE / DENIED |
| Sélection exclusive | Logement encastré ; option active en fond carbone, losange plein, filet du domaine |
| Désactivée | Touche à plat, matière éteinte (`relief1`), hachures, « — » |
| Dimensions | Inchangées (pas de touches massives) ; la marge intérieure passe de 2 à 4 px pour loger l'alvéole |

FREEZE (bascule), ARM (canal ambre et témoin), AUTHORIZE (action, icône ambre) et CAPTURE (action, icône d'instantané)
restent distincts par la forme, le témoin, l'icône, le texte et l'emplacement.

## 3. PANIC (LAB-A07.1)

Rouge de sécurité `alert` #E0524A inchangé ; libellé **carbone** #0A0D10 à 13 px gras. La face est bombée (haut éclairci
jusqu'à 30 % au survol). Dans la zone du libellé, elle n'est jamais plus sombre que le rouge nominal ; l'assombrissement
de relief commence à 72 % de la hauteur, sous le libellé. Le chanfrein est rouge sombre, avec une ombre et une course
identiques aux autres touches. La position (isolée à droite de la barre), la zone interactive et le comportement sont
inchangés. Mesure : [CONTRAST-REPORT.md](CONTRAST-REPORT.md) §2.

## 4. SYNAPTIC TRACK — Machined Track

Rail encastré : `groove`, ombre interne haute, lèvre basse éclairée. Sélecteur actif en titane (`relief3` → `relief2`),
biseau de 1 px, marqueur du domaine encastré en pied (44 % de la largeur, lueur ≤ 0,35). Crans usinés entre segments
inactifs. Liste : champ encastré, marqueur vertical du domaine, cran avant le chevron, rang n/N. Onglets : rail usiné en
pied, onglet actif signalé par un marqueur du domaine encastré dans le rail. Indisponible : hachures, aucun marqueur de
domaine. Aucun ornement ne réduit la surface des libellés.

## 5. Cybernetic HUD

| Élément | Règle |
|---|---|
| Monture | Titane usiné (chanfrein vertical), joint carbone |
| Verre | Profond : cœur `hudGlassCore` #0E161B, bords `hudGlassDeep` #070B0E, reflet haut ≤ 4,5 %, vignettage interne |
| Encadrement | Rails gradués haut et bas (graduations tous les 6 px, majeures tous les 30), encadrement intérieur interrompu, repères d'angle du domaine |
| En-tête | Onglet du domaine, titre en capitales techniques espacées, pastilles de nature, DEMO DATA. Si la place manque, les pastilles se réduisent à leur forme et DEMO DATA devient DEMO (jamais supprimé). |
| Nature de la donnée | **BASE** : carré du domaine (gris titane pour un paramètre neutre, jamais blanc). **EFF** : losange MOD. **MEAS** : crochets, blanc optique. **INDICATIVE** : contour ambre. **STATE** : point. |
| Pied | `SRC` et source de la donnée (obligatoire) |
| Variantes | Valeur (lecture principale et échelle avec repères BASE et EFF), oscilloscope (graticule, axes gradués, réticule), spectre (échelles dB et fréquence, INDICATIVE), wavetable (rail X avec BASE et EFF), courbe (axes IN / OUT, réticule d'entrée EFF), enveloppe (étapes, maintien, niveau publié AMP seulement, aucun curseur d'étape), crête (segments neutres, rouge au-delà de 0 dBFS, lecture PEAK dBFS), autonomie et diagnostic (matrice clé, témoin, valeur) |
| Barre | Afficheur compact sur une ligne (hauteur < 70 px) : titre, valeur, nature, DEMO |
| Interdits respectés | Ni CRT, ni balayage, ni néons excessifs, ni faux analyseur, ni fausse activité, ni animation |

## 6. PLASMA CHANNEL — STANDARD et COMPACT

| Gabarit | Taille | Règle |
|---|---|---|
| STANDARD | 64 × 180 | Rendu B1 validé, identique au pixel près |
| COMPACT | 48 × 150 (largeur < 56) | Gorge décalée de 1 px ; graduations : toutes les marques, mais libellés limités aux bornes et au zéro ; libellé sur deux lignes au même corps de 11 px ; valeur en monospace de 10 px |

Même composant, mêmes gestes et même garde de geste (LAB-G15). Le choix du gabarit selon l'emplacement appartient à
R2-B2 et R2-C.

## 7. Intégration R2-B2 (rappel)

Les composants se branchent sur `ParamLink` par l'adaptateur `ParamSource` (démontré dans `lab/app/Compare.cpp`). Les
tokens dérivés (`relief1` à `relief3`, `groove`, `hudGlassCore`, `hudGlassDeep`, `hudRail`) rejoignent
`ui/DesignTokens.h`. La révision B1 n'est pas intégrée.
