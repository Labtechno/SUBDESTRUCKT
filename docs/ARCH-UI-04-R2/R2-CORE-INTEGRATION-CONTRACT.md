# ARCH-UI-04-R2-A — contrat d'intégration du SYMBIOTIC CORE (DA-A07)

Ce contrat fixe l'**enveloppe** graphique et interactive du Core, avant toute réalisation volumétrique. Il ne spécifie
aucun moteur de rendu. Le rendu 3D relève d'ARCH-UI-05, non autorisé (INV-08). Ce contrat traite le constat Codex DA-14
et la recommandation P1-06 : figer l'enveloppe, les axes et l'alternative numérique avant la 3D.

## 1. État 0.4.1 (relevé du code `ui/PerformanceView.cpp`)

- Commande directe : X = `cross.fwd`, Y = `cross.rev` (plan carré `pad_`), Z = `cross.master` (canal PLASMA V à droite),
  et une rangée de trois NEXUS S numériques (X, Y, Z) en bas.
- Glisser : clic = position absolue ; Maj + clic = déplacement relatif ; Maj pendant le glisser = fin (× 0,1). X et Y
  partagent un geste : deux `begin` au clic, deux `end` au relâchement. Double-clic : valeurs par défaut. Flèches : pas de
  1 % (0,1 % avec Maj), une transaction complète par appui.
- Rendu statique provisoire signalé (« RENDU STATIQUE PROVISOIRE · illustratif · moteur 3D : ARCH-UI-05 »). Un point
  Effective est affiché quand les axes sont modulés.
- PB-02 : si l'éditeur est détruit pendant un glisser, les deux gestes ne sont pas refermés (audit statique, non
  démontré physiquement, non corrigé).

Direction validée pour ARCH-UI-05 (rappel, non réalisée en R2) : noyau biomécanique 3D vivant, structure organique et
technologique, animation de repos discrète, réactivité musicale maîtrisée, matériaux cohérents avec SYMBIOTIC
INDUSTRIAL. La matière biomécanique est réservée au Core (master §4).

## 2. Axes (normatif)

| Axe | StableID | Nom hôte | Libellé R2 | Plage | Défaut | Modulable |
|---|---|---|---|---|---|---|
| X | `cross.fwd` | CROSS WT>FM | X · WT→FM | 0…1 (`lin`) | 1 | Oui (contrôle et audio) |
| Y | `cross.rev` | CROSS FM>WT | Y · FM→WT | 0…1 (`lin`) | 1 | Oui |
| Z | `cross.master` | CROSS Master | Z · CROSS LVL | 0…1 (`lin`) | 1 | Oui |

| ID | Exigence |
|---|---|
| CORE-01 | Les axes ne changent pas. Aucun mapping multiparamétrique, aucun morphing, aucun paramètre nouveau (INV-02). |
| CORE-02 | Le Core n'écrit que ces trois paramètres, et seulement par les gestes utilisateur, via `ParamLink` (INV-06). |
| CORE-03 | Le libellé de chaque axe est visible en permanence à côté de l'axe, au corps critique (OD-06). Il n'est jamais recouvert par le rendu. |

## 3. Enveloppe spatiale

### 3.1 Composition

| Zone | Contenu | Dimension 0.4.1 (px log.) | Règle R2 |
|---|---|---|---|
| En-tête | Titre SYMBIOTIC CORE et statut | 28 (titre 16) | Le statut STATIC PREVIEW est secondaire : corps ≤ titre, `textDim`, à droite (WS-P12) |
| Plan XY | Base (réticule), Effective (point), rendu | Carré `pad_` = min(largeur − 74, hauteur − 130) − 12 | **Seule zone de rendu.** Le rendu ARCH-UI-05 est rogné à ce carré. |
| Canal Z | PLASMA V, `cross.master` | 48 + 26 de marge = 74 | Toujours visible, hors de la zone de rendu |
| Rangée numérique | X, Y, Z (NEXUS S ou champs) | 100 (NEXUS S 96 + 4) | En COMPACT : ligne numérique de 24 px (trois champs de saisie) |

### 3.2 Dimensions du plan XY (calcul, cf. [R2-WORKSPACE-SPEC.md](R2-WORKSPACE-SPEC.md) §1)

| Situation | Pad px logiques | Pad px écran |
|---|---|---|
| 1440 × 900, 100 %, Inspector fermé | 418 | 418 |
| 1440 × 900, 100 %, Inspector ancré | 376 | 376 |
| 1440 × 900, 75 % | 690 | 518 |
| 1440 × 900, 150 % (minimum de vue, défilement) | 316 | 474 |
| 960 × 640, 100 % (minimum de vue, défilement) | 316 | 316 |
| 960 × 640, 100 %, disposition COMPACT proposée (estimation) | ≈ 312 | ≈ 312 |
| 1920 × 1200, 100 % | 756 | 756 |

| ID | Exigence | Type |
|---|---|---|
| CORE-04 | Pad ≥ 300 px logiques dans les dispositions WIDE, STANDARD et COMPACT | Seuil candidat |
| CORE-05 | Pad ≥ 220 px écran à tout zoom proposé (75 % au minimum : 237 px en 0.4.1, défilant) | Seuil candidat |
| CORE-06 | La zone de rendu ne déborde jamais sur l'en-tête, le canal Z, la rangée numérique ni les libellés d'axe | Obligatoire |
| CORE-07 | Un changement de disposition ne déplace pas le Core pendant un geste (WS-P05) | Obligatoire |

## 4. Interaction

| ID | Exigence |
|---|---|
| CORE-08 | Gestes 0.4.1 conservés (§1). Un glisser XY = un geste hôte par axe, ouvert au clic, refermé au relâchement. |
| CORE-09 | Clavier : flèches gauche/droite pour X, haut/bas pour Y (pas de 1 %, 0,1 % avec Maj), conservées. Ajout proposé : Page Haut/Bas pour Z (10 %). Le canal Z et la rangée numérique restent focalisables. |
| CORE-10 | Saisie numérique pour chaque axe (alternative obligatoire au geste), avec le titre accessible = nom hôte |
| CORE-11 | Fermeture garantie des gestes ouverts : relâchement, perte de capture souris, masquage et **destruction de l'éditeur**. Ce point recoupe PB-02 : il ne peut être livré que dans un périmètre où le propriétaire autorise la correction de PB-02 (INV-10, OD-10). |
| CORE-16 | L'animation (repos, réactivité) ne compromet jamais les gestes : la zone de prise, la position du réticule et la réponse au geste restent celles de l'enveloppe. Pendant un geste, l'animation ne masque ni le réticule ni les libellés d'axe. |

## 5. Base, Effective et automation

| ID | Exigence |
|---|---|
| CORE-12 | Base = réticule `text`. Effective = point `effective`, affiché seulement si `modulated[cross.fwd]` ou `modulated[cross.rev]`. Z Effective = marqueur du canal Z. La valeur numérique affichée est la Base. |
| CORE-13 | L'automation hôte déplace le réticule à la cadence de l'éditeur (30 Hz). Le Core n'écrit jamais en retour pendant la lecture d'automation hors geste utilisateur. Une traînée d'Effective est admise si elle est calculée à partir des seules valeurs publiées, sans extrapolation. |

## 6. Repli 2D (CORE-14)

Le Core DOIT disposer d'une représentation 2D complète, aux liaisons identiques à celles du rendu volumétrique :

- par défaut jusqu'à ARCH-UI-05 ;
- si le rendu volumétrique n'est pas disponible (initialisation, contexte graphique refusé par l'hôte) ;
- en disposition CONSTRAINED ;
- par préférence utilisateur (état d'interface, hors preset).

Le passage 2D ↔ 3D ne change ni les gestes, ni les valeurs, ni l'Undo, ni l'accessibilité.

## 7. Interface attendue d'ARCH-UI-05 (contrat de frontière)

| Élément | Contrat |
|---|---|
| Entrées | Bases X, Y, Z ; Effectives et drapeaux `modulated` ; état de geste (au repos, en geste) ; disposition ; zoom ; données de télémétrie **publiées** seulement ([R2-TELEMETRY-FEASIBILITY.md](R2-TELEMETRY-FEASIBILITY.md) classe A ou B) |
| Sorties | Aucune écriture. Les gestes restent gérés par le composant d'enveloppe (§4). |
| Zone | Le carré `pad_`, rogné (CORE-06) |
| Cadence | Données : lecture de télémétrie à 30 Hz (0.4.1). Rendu : cadence propre au moteur 3D, fixée par ARCH-UI-05, **non imposée à 30 FPS** (C05). Interpolation visuelle admise entre deux lectures, sans création de donnée. |
| Budget de rendu | Seuil candidat ≤ 4 ms par image sur la machine de recette, à mesurer. Repli 2D automatique en cas de dépassement soutenu. |
| Dépendances | Aucune nouvelle dépendance (INV-08). L'usage éventuel du module JUCE `juce_opengl` est une décision d'ARCH-UI-05 (OD-12). |
| Accessibilité | Le rendu est décoratif pour l'accessibilité. Les titres et valeurs passent par l'enveloppe. |
| Réactivité | Le rendu peut refléter l'activité réelle (sources, Effective, crête). Il ne mesure pas l'activité des routes, faute de donnée publiée (T-24). |
