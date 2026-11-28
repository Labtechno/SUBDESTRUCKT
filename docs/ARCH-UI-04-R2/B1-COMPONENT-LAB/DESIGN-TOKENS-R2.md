# ARCH-UI-04-R2-B1 — tokens de design R2

Source : `lab/r2/R2Style.h`. La palette de référence 0.4.1 (`ui/DesignTokens.h`) est **lue sans modification**. Les
tokens R2 en sont des dérivés. Ce sont des propositions : ils seront intégrés à `ui/` en R2-B2, après validation.

## 1. Palette de référence (inchangée)

| Token | Hex | Rôle |
|---|---|---|
| `chassis` (Graphite) | `#161A1D` | châssis |
| `carbon` | `#0A0D10` | fenêtres de valeur, fonds de verre |
| `titanium` | `#343C43` | corps 0.4.1 |
| `text` (Optical Text) | `#DCE4E9` | texte principal |
| `wt` / `fm` / `cross` / `mod` | `#5CE2CB` / `#F2A05E` / `#B095FF` / `#7FA9E8` | domaines (EFFECTIVE = `mod`) |
| `alert` | `#E0524A` | alertes et PANIC uniquement |
| `warn` | `#E6C35A` | ARMED, attente d'autorisation, étiquettes DEMO DATA / INDICATIVE |
| `textDim` / `textMute` | `#98A5AE` / `#7D8A93` | secondaire / inactif |
| `glass` / `glassEdge` / `panelEdge` / `track` | `#0D1215` / `#26303A` / `#2A3137` / `#10151A` | verre, montures, arêtes, pistes |

## 2. Tokens dérivés R2

| Token | Valeur | Rôle |
|---|---|---|
| `relief0` | `#161A1D` (= chassis) | niveau châssis |
| `relief1` | `#22282D` | plaque usinée |
| `relief2` | `#2C343A` | corps de commande, touche plate (bascule, armement) |
| `relief3` | `#3A434B` | surface d'action (touche convexe), curseur mécanique |
| `bevelLight` / `bevelDark` | `#56616A` / `#07090B` | arêtes usinées (éclairage haut-gauche) |
| `groove` | `#0B0F12` | gorges, encastrement des témoins |
| `grid` / `gridStrong` | `#1D2A33` / `#2B3D4A` | graticules et graduations des afficheurs |
| `effective` | = `mod` | anneau, losange et triangle EFFECTIVE |
| `measure` | = `text` | valeurs mesurées (crête) |
| `inactive` | = `textMute` | paramètre conservé sans effet (avec hachures) |
| `glowMax` | 0,35 | opacité maximale d'une lueur (« néons excessifs » exclus) |

`relief1` et `relief2` ont été ajustés pendant B1 (`#1F252A` et `#2A3238` dans le premier rendu). L'écart plaque/châssis
mesurait 1,13:1, sous le seuil candidat de 1,15:1.

## 3. Typographie

| Token | px logiques | Usage | 75 % (px écran) |
|---|---|---|---|
| `fontMicro` | 10 | NEXUS Micro | 7,5 |
| `fontLabel` | 11 | NEXUS S, NEURAL CELL, SYNAPTIC TRACK, PLASMA | 8,25 |
| `fontCritical` | **12** | repères critiques (OD-06) | **9,0** |
| `fontTitle` | 12,5 | NEXUS XL, titres de plaque | 9,4 |
| `fontValueXL` | 14 | valeur NEXUS XL | 10,5 |

Polices : celles de 0.4.1 (sans-serif et monospace par défaut du système). Elles sont résolues en **Lucida Grande /
Menlo** sur macOS et en **Verdana / Lucida Console** sur Windows. Les largeurs dépendent donc de la plateforme : Verdana
mesure en médiane 3 % de plus que Lucida Grande, de −8 % à +20 % selon le libellé (PT-02). Aucune police n'est embarquée
(aucune licence nouvelle).

## 4. Gabarits

| Composant | 0.4.1 | R2-B1 | Écart |
|---|---|---|---|
| NEXUS XL / S / Micro | 132 × 150 / 84 × 96 / 64 × 74 | inchangés | — |
| NEURAL CELL | 168 × 40 | inchangé ; libellés de barre 84 à 150 de large | — |
| SYNAPTIC TRACK | 168 × 46 | inchangé | — |
| PLASMA vertical | 48 × 150 | **64 × 180** | proposition : loger graduations et unité lisibles (DA-A06). À arbitrer (budget Codex de 70 px sur PLASMA V). |
| PLASMA horizontal | 220 × 40 | 360 × 44 dans les planches | largeur libre selon la plaque ; légende de 30 % |

## 5. Contrastes mesurés (WCAG 2.x, rapport de luminance)

Calcul sur les valeurs des tokens (indépendant de la plateforme), identique dans `data/macos/contrast.csv` et
`data/windows/contrast.csv`. Les faces en dégradé sont évaluées sur leur teinte de base.

| Avant-plan | Fond | Usage | Rapport | Seuil | Verdict |
|---|---|---|---|---|---|
| text | relief1 | sérigraphie, valeurs | 11,58 | 4,5 | PASS |
| text | carbon | valeur numérique | 15,14 | 4,5 | PASS |
| text | glass | titre d'afficheur | 14,64 | 4,5 | PASS |
| text | relief3 | libellé d'action | 7,83 | 4,5 | PASS |
| text | relief2 | libellé de bascule | 9,83 | 4,5 | PASS |
| textDim | relief1 / glass / relief2 | texte secondaire, source, OFF | 5,91 / 7,47 / 5,02 | 4,5 | PASS |
| textMute | relief1 / glass | libellé inactif, graduation | 4,21 / 5,32 | 3,0 | PASS |
| wt / fm / cross / mod | glass | traces d'afficheur | 11,84 / 8,94 / 7,71 / 7,84 | 3,0 | PASS |
| wt / fm / cross / mod | relief1 | arcs BASE, anneau EFFECTIVE | 9,36 / 7,07 / 6,09 / 6,20 | 3,0 | PASS |
| warn | relief2 / glass | ARMED, étiquettes | 7,42 / 11,05 | 4,5 | PASS |
| **white** | **alert** | **libellé PANIC 0.4.1** (13 px gras = texte normal au sens WCAG) | **3,84** | **4,5** | **FAIL** |
| carbon | alert | libellé PANIC, variante proposée | 5,08 | 4,5 | PASS |
| alert | glass | crête > 0 dBFS | 4,91 | 3,0 | PASS |
| focus | relief1 | anneau de focus | 11,58 | 3,0 | PASS |
| track | relief1 | piste d'arc (décor) | 1,23 | 1,0 | PASS (décor) |
| relief1 / relief2 / relief3 | relief0 / relief1 / relief2 | niveaux de matière | 1,17 / 1,18 / 1,26 | 1,15 (candidat) | PASS |

**Constat PANIC.** Le libellé blanc sur rouge, hérité de 0.4.1, n'atteint pas 4,5:1. Le comportement de PANIC n'est
pas concerné. Deux options sont soumises au propriétaire :

- libellé carbone sur la même touche rouge (5,08:1, rendu sur la planche 03) ;
- libellé blanc agrandi à ≥ 18,7 px gras, qui en fait un « grand texte » au sens WCAG (seuil 3:1).

Le rouge reste réservé à PANIC et aux alertes.
