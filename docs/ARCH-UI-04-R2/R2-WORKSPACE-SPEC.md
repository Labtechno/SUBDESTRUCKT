# ARCH-UI-04-R2-A — espaces de travail (DA-A08 à DA-A11)

Ce document spécifie l'Adaptive Instrument (PERFORMANCE), l'Adaptive Engine Workspace (SOUND DESIGN), l'Engineering
Workbench (ENGINEERING) et l'Adaptive Inspector. Les géométries du §1 sont **calculées à partir du code 0.4.1**
(`ui/MainEditor.cpp`, `ui/PerformanceView.cpp`, `ui/DesignTokens.h`), pas mesurées à l'écran. Le Core intérieur (418 px
au nominal, Inspector fermé) coïncide avec le relevé de Codex. Les épaisseurs de barre de défilement (quelques pixels)
sont négligées.

## 1. Constat géométrique 0.4.1

Règles du code : fenêtre de 960 × 640 à 2 560 × 1 600, 1 440 × 900 par défaut. L'Inspector est ancré à droite sur
300 px écran (hors zoom). La barre globale n'est pas zoomée. Elle passe sur deux rangées (82 px au lieu de 44) quand la
largeur est inférieure à 1 242 px plus la largeur préférée des onglets d'espace, soit environ 1 530 px (1 620 px si
AUTHORIZE est visible). Le contenu est disposé à la taille logique `(fenêtre − Inspector) / zoom`, puis mis à l'échelle.
Il défile si la taille logique est inférieure au minimum de l'espace : PERFORMANCE 1 080 × 812, SOUND DESIGN et
ENGINEERING 1 080 × 700.

| Fenêtre | Inspector | Zoom | Barre | Canvas logique | Défilement PERFORMANCE (min 1080 × 812) | Défilement SOUND / ENG (min 1080 × 700) | Pad Core px log. | Pad Core px écran |
|---|---|---|---|---|---|---|---|---|
| 1440 × 900 | ancré | 75 % | 82 | 1520 × 1090 | aucun | aucun | 690 | 518 |
| 1440 × 900 | ancré | 100 % | 82 | 1140 × 818 | aucun | aucun | 376 | 376 |
| 1440 × 900 | ancré | 150 % | 82 | 760 × 545 | H 320, V 267 | H 320, V 155 | 316 | 474 |
| 1440 × 900 | fermé | 75 % | 82 | 1920 × 1090 | aucun | aucun | 690 | 518 |
| 1440 × 900 | fermé | 100 % | 82 | 1440 × 818 | aucun | aucun | 418 | 418 |
| 1440 × 900 | fermé | 150 % | 82 | 960 × 545 | H 120, V 267 | H 120, V 155 | 316 | 474 |
| 960 × 640 | ancré | 75 % | 82 | 880 × 744 | H 200, V 68 | H 200 | 316 | 237 |
| 960 × 640 | ancré | 100 % | 82 | 660 × 558 | H 420, V 254 | H 420, V 142 | 316 | 316 |
| 960 × 640 | ancré | 150 % | 82 | 440 × 372 | H 640, V 440 | H 640, V 328 | 316 | 474 |
| 960 × 640 | fermé | 75 % | 82 | 1280 × 744 | V 68 | aucun | 412 | 309 |
| 960 × 640 | fermé | 100 % | 82 | 960 × 558 | H 120, V 254 | H 120, V 142 | 316 | 316 |
| 960 × 640 | fermé | 150 % | 82 | 640 × 372 | H 440, V 440 | H 440, V 328 | 316 | 474 |
| 1920 × 1200 | ancré | 100 % | 44 | 1620 × 1156 | aucun | aucun | 756 | 756 |
| 1920 × 1200 | ancré | 150 % | 44 | 1080 × 770 | V 42 | aucun | 316 | 474 |
| 1920 × 1200 | fermé | 150 % | 44 | 1280 × 770 | V 42 | aucun | 412 | 618 |

Conclusions :

1. Au **nominal à 100 %**, PERFORMANCE ne défile pas, Inspector ancré ou fermé. L'objectif DA-A08 est déjà atteint
   dans ce cas. Il DOIT être préservé.
2. Au **minimum à 100 %**, PERFORMANCE défile dans les deux états de l'Inspector (Inspector fermé : 120 px en H,
   254 px en V). L'objectif DA-A08 n'est pas atteint. La disposition 0.4.1 ne peut pas l'atteindre sans changer
   d'organisation, car son minimum (1 080 × 812) dépasse le canvas (960 × 558).
3. À **150 %**, le canvas logique d'une fenêtre nominale (760 × 545 ou 960 × 545) est du même ordre que celui de la
   fenêtre minimale. Une disposition choisie d'après le **canvas logique** traite donc le zoom et la taille de fenêtre
   avec une seule règle.
4. La barre n'est pas zoomée. PANIC, crête et état d'autonomie y restent visibles à tout zoom. C'est l'emplacement
   naturel des repères permanents.
5. La barre à deux rangées coûte 38 px de hauteur à 1 440 px de large.

## 2. PERFORMANCE — Adaptive Instrument (DA-A08)

### 2.1 Priorités

Ordre normatif de conservation lorsque l'espace manque (1 = jamais replié) :

| Rang | Élément | Emplacement cible | Règle |
|---|---|---|---|
| 1 | PANIC | Barre | Toujours visible, jamais déplacé hors de la barre |
| 2 | PEAK dBFS (L/R) | Barre | Afficheur de mesure permanent (DE-04, Codex P1-01) |
| 3 | État d'autonomie (ARMED n, AUTONOMY, RENDER, REVOKED) et AUTHORIZE | Barre | Toujours visible quand au moins un module est armé ou que le Drone est configuré |
| 4 | WT X, WT Y, WT Z | PERFORMANCE | NEXUS XL ; NEXUS S en COMPACT |
| 5 | FM principal (FM INDEX, FM FDBK) | PERFORMANCE | idem |
| 6 | Axes du Core (X · WT→FM, Y · FM→WT, Z · CROSS LVL) | PERFORMANCE | Plan XY et canal Z, saisie numérique |
| 7 | CROSS LVL | PERFORMANCE | Canal Z du Core |
| 8 | Macros M1 à M4 | PERFORMANCE | NEXUS S |
| 9 | BALANCE, OUT GAIN | PERFORMANCE | PLASMA H ou NEXUS S |
| — | Moniteur (SCOPE, SPECTRUM, XY, NETWORK), états détaillés | PERFORMANCE | Repliables avec indicateur |

### 2.2 Exigences

| ID | Exigence | Type |
|---|---|---|
| WS-P01 | Aucun défilement en PERFORMANCE à 1 440 × 900, 100 %, Inspector ancré ou fermé | Obligatoire (déjà vrai, à préserver) |
| WS-P02 | Aucun défilement en PERFORMANCE à 960 × 640, 100 %, Inspector fermé | Objectif DA-A08, à démontrer par géométrie puis prototype |
| WS-P03 | À 960 × 640, 100 %, Inspector ouvert : les rangs 1 à 9 restent visibles sans défilement. L'Inspector s'affiche en tiroir superposé ou en bande (OD-05). | Objectif |
| WS-P04 | Un élément replié laisse un indicateur visible (onglet, chevron ou compteur) et reste atteignable en un clic. Rien d'essentiel n'est masqué sans indication. | Obligatoire |
| WS-P05 | La disposition se choisit sur le canvas logique (§2.4). Le changement de disposition est différé jusqu'à la fin de tout geste en cours. | Obligatoire |
| WS-P06 | Un changement de disposition conserve la sélection de l'Inspector, le focus clavier et la destination sélectionnée (Codex P1-04) | Obligatoire |
| WS-P07 | La barre tient sur une rangée à 1 440 px, AUTHORIZE visible | Objectif, seuil candidat |
| WS-P08 | Le Core respecte son enveloppe minimale dans chaque disposition ([R2-CORE-INTEGRATION-CONTRACT.md](R2-CORE-INTEGRATION-CONTRACT.md) §3) | Obligatoire |
| WS-P09 | Les rangs 4 à 9 restent pilotables au clavier dans toutes les dispositions | Obligatoire |
| WS-P10 | Aucune donnée de mesure en PERFORMANCE n'est simulée (INV-11) | Obligatoire |
| WS-P11 | Le nom et l'édition des macros (champ utilisateur) restent accessibles | Obligatoire |
| WS-P12 | Le libellé STATIC PREVIEW du Core ne prime pas sur les axes (Codex DA-14) | Obligatoire |
| WS-P13 | Les boutons UNDO et REDO de la barre conservent au moins 50 px de large (constat Codex sur les libellés longs) | Seuil candidat |
| WS-P14 | Les seuils de disposition du §2.4 sont confirmés avant mesure | Seuil candidat (OD-04) |
| WS-P15 | PANIC et la crête restent accessibles quels que soient le zoom, l'espace et la navigation (barre non zoomée, présente dans les trois espaces) | Obligatoire |
| WS-P16 | La façade reste cohérente et musicale dans toutes les dispositions | Validation subjective |

### 2.3 Faisabilité géométrique au minimum (estimation à confirmer par prototype)

Canvas logique visé : 960 × 558 (barre sur deux rangées), soit 944 × 542 après les marges de 8 px.

| Bloc | Composition proposée | Largeur | Hauteur |
|---|---|---|---|
| Bandeau bas | M1–M4 (NEXUS S 96 px), BALANCE et OUT GAIN (NEXUS S 84 px), en-têtes compris | 4 × 96 + 2 × 84 + 6 × 8 = 600 ≤ 944 | 96 + 22 + 16 = 134 |
| Colonne gauche | WT X, WT Y, WT Z en NEXUS S (3 × 84 + 2 × 8 = 268) ; FM INDEX, FM FDBK (NEXUS S) en dessous | 268 | 2 × (96 + 22 + 16) + 8 = 276 |
| Core | Plaque restante : (944 − 268 − 8) × (542 − 134 − 8) = 668 × 400. Ligne numérique X/Y/Z compacte de 24 px au lieu de 100 px. | 668 | 400 |
| Moniteur | Replié (rang « — »), indicateur dans l'en-tête du Core | — | — |

Estimation du pad Core : min(668 − 74 − 16 − 12, 400 − 22 − 30 − 24 − 12) ≈ min(566, 312) = **312 px logiques**, soit
le même ordre que les 316 px du minimum défilant de 0.4.1. Conclusion : WS-P02 paraît géométriquement atteignable. Il
DOIT être démontré par un prototype mesuré (ACC WS-P02) avant d'être déclaré PASS.

### 2.4 Dispositions par seuils (proposition, seuils candidats, OD-04)

| Disposition | Condition (canvas logique) | Contenu |
|---|---|---|
| WIDE | w ≥ 1 280 et h ≥ 812 | Disposition 0.4.1 ; le Core reçoit le surplus |
| STANDARD | 1 080 ≤ w < 1 280 et h ≥ 812 | Disposition 0.4.1 |
| COMPACT | 940 ≤ w < 1 080 ou 540 ≤ h < 812 | §2.3 : NEXUS S, moniteur replié, ligne numérique compacte |
| CONSTRAINED | w < 940 ou h < 540 (zoom 150 % au minimum, par exemple) | COMPACT avec défilement vertical seulement. Rangs 1 à 3 dans la barre, donc toujours visibles. |

Les seuils sont des propositions. La validation propriétaire est requise avant mesure.

## 3. SOUND DESIGN — Adaptive Engine Workspace (DA-A09)

| ID | Exigence | Type |
|---|---|---|
| WS-S01 | La surface suit le contexte sélectionné : moteur (WT, FM, CROSS), emplacement de chaîne (WT1…GL2, filtre), source de modulation. Le contexte se change par une action explicite : onglet, sélection, focus Inspector. | Obligatoire |
| WS-S02 | Aucune redistribution spontanée pendant un geste. Un changement de type ou de mode (choix) ne relance la mise en page de la plaque concernée qu'à la fin du geste. La commande sous le pointeur reste en place (ancrage). | Obligatoire |
| WS-S03 | L'ordre affiché est celui du moteur : WT CHAIN et FM CHAIN avant Cross Audio, GLOBAL CHAIN après, puis OUTPUT SAFETY. Les transformations Z apparaissent dans l'ordre effectif, et la restriction inter-domaines non validée y est signalée. | Obligatoire (INV-05) |
| WS-S04 | Les paramètres conditionnels restent atteignables. Ceux d'un type non sélectionné sont accessibles par une section « ALL PARAMETERS » de la plaque, ou depuis l'Inspector, avec la mention INACTIVE et sa raison (INV-07, UI04-COV). | Obligatoire |
| WS-S05 | Les panneaux de contexte sont persistants par onglet (architecture 0.4.1 conservée : pas de reparentage pendant l'interaction, cf. correctif d'usage après libération ARCH-UI-04). | Obligatoire |
| WS-S06 | Les réglages liés sont rapprochés. Le surplus d'espace va au graphe ou à une zone de détail (Codex DA-07). Exemple : D2 ne repousse plus FB MODE hors du premier écran. | Objectif |
| WS-S07 | Pas de défilement horizontal au nominal à 100 %, Inspector ancré | Obligatoire (déjà vrai) |
| WS-S08 | Au minimum à 100 %, Inspector fermé : défilement vertical seul admis | Seuil candidat |
| WS-S09 | WT, FM et CROSS restent simultanément accessibles : chacun est atteignable sans quitter SOUND DESIGN ni perdre le contexte des deux autres | Obligatoire |
| WS-S10 | Répartition par contexte explicite (tableau ci-dessous). Aucune grande zone vide quand un panneau adjacent défile alors qu'il pourrait l'éviter. | Objectif |

| Contexte d'édition | Surface prioritaire | Surfaces réduites (jamais masquées) |
|---|---|---|
| WT XYZ | Table WT et position X/Y/Z (Base, Effective) | FM, CROSS en colonnes compactes |
| WT Z | Slots A/B, ordre Z effectif, transformations | idem |
| FM | Quatre opérateurs côte à côte | WT, CROSS compacts |
| CROSS | Routes (WT→FM, FM→WT, DIRECT, BUS / XAUDIO) | WT, FM compacts |
| PROCESS | Chaînes WT, FM, GLOBAL dans l'ordre du moteur, étage sélectionné déplié | Autres étages repliés avec titre et type |
| MOD | Courbes et sources (ENV, LFO, RANDOM, FOLLOW) | Liste des routes globales |

## 4. ENGINEERING — Engineering Workbench (DA-A10)

### 4.1 Groupes et outils

| Groupe | Outils (onglets 0.4.1 conservés) | Libellés R2 |
|---|---|---|
| NETWORK | FM NETWORK, CROSS NETWORK | FM, CROSS (vue NEURAL/TABLE, [R2-NEURAL-NETWORK-SPEC.md](R2-NEURAL-NETWORK-SPEC.md)) |
| MODULATION | MODULATION (matrice globale, 64 liens), MIDI LEARN | MOD MATRIX, MIDI LEARN |
| BANK LAB | BANK LAB | BANK LAB |
| STATE | VOICE / TUNING, PRESETS / STATE, CAPTURES / ASSETS | VOICE/TUNING, PRESETS, CAPTURES/ASSETS |
| TOOLS | RANDOMIZE, DIAGNOSTICS | RANDOMIZE, DIAGNOSTICS |

### 4.2 Exigences

| ID | Exigence | Type |
|---|---|---|
| WS-E01 | Dix outils conservés, aucune fonction supprimée ni fusionnée | Obligatoire |
| WS-E02 | Navigation à deux niveaux (groupe, outil). Le dernier outil ouvert est mémorisé par groupe (état d'interface, non persisté dans le preset, INV-03). | Obligatoire |
| WS-E03 | Les liens internes (`UiContext::navigate`) adressent un couple (groupe, outil) stable, et non un indice d'onglet. Les boutons de barre PRESETS et SAVE… ouvrent STATE › PRESETS. SAVE… place le focus sur le champ de nom. | Obligatoire |
| WS-E04 | PRESETS : sélection et validation rapprochées. Une seule navigation de preset par contexte (Codex P2-03, « double navigation de presets »). | Objectif |
| WS-E05 | Les outils courts (MIDI LEARN, DIAGNOSTICS) reçoivent une zone de détail utile à la place de l'espace vide (Codex DA-12) | Objectif |
| WS-E06 | DIAGNOSTICS affiche l'état d'autorisation, le transport, les compteurs de sûreté et le journal TA, sans valeur inventée (INV-11) | Obligatoire |
| WS-E07 | Aucune redistribution entre groupes pendant un geste | Obligatoire |
| WS-E08 | Les parcours ne se dégradent pas significativement (§4.3) : au plus un clic de plus qu'en 0.4.1 pour atteindre un outil | Seuil candidat |
| WS-E09 | ENGINEERING est un espace de programmation approfondie, pas une console de développement interne. Les textes de diagnostic restent techniques, mais structurés et en anglais. | Objectif (validation subjective) |

### 4.3 Parcours (analyse)

| Accès | 0.4.1 (dix onglets plats) | R2 proposé (groupe + outil, dernier outil mémorisé) | Écart |
|---|---|---|---|
| Outil depuis ENGINEERING, même groupe | 1 clic | 1 clic (onglets d'outil visibles) | 0 |
| Outil depuis ENGINEERING, autre groupe | 1 clic | 1 clic si c'est le dernier outil du groupe, sinon 2 | 0 à +1 |
| Outil depuis un autre espace | 2 clics | 2 à 3 clics | 0 à +1 |
| BANK LAB (groupe à outil unique) | 1 clic | 1 clic | 0 |
| Liens internes (PRESETS, SAVE…, « ouvrir dans … ») | 1 clic | 1 clic (adresse groupe + outil) | 0 |

Le pire cas (+1 clic) concerne le changement d'outil à l'intérieur d'un groupe de deux ou trois outils, depuis un autre
groupe. Ce comptage est une analyse du modèle de navigation. Il est à mesurer sur prototype (WS-E08).

## 5. Inspector — Adaptive Inspector (DA-A11)

Propriétés (brief §16) : contextuel, rétractable, épinglable, adaptatif, disponible en mode compact. La sélection et
l'épinglage sont préservés lors des changements de vue et de dimensions (contrat existant).

### 5.1 Hiérarchie normative

| Rang | Contenu | Source | Mode compact |
|---|---|---|---|
| 1 | Nom complet (nom du registre et précision éventuelle, [R2-CONTROL-NAMING-SPEC.md](R2-CONTROL-NAMING-SPEC.md) §5) | Registre | Toujours |
| 2 | BASE : valeur, unité, commande d'édition | Paramètre | Toujours |
| 3 | EFFECTIVE : valeur appliquée, ou « = BASE (not modulated) » | `effective[id]`, `modulated[id]` | Toujours |
| 4 | Plage, unité, défaut, domaine (`lin`/`pos`/`bip` explicités) | Registre | Toujours, sur une ligne |
| 5 | Modulations : liens globaux et routes de réseau vers ce paramètre, profondeur, source, état d'inactivité (`ModInactive`) | `ModMatrix`, `Topology` | Compteur et accès |
| 6 | Automation et contrôle : automatisable oui/non (déjà affiché en 0.4.1), CC MIDI appris, état de l'apprentissage | Registre, processeur | Repliable |
| 7 | Détails techniques : cadences (control/audio), lissage, chemin d'accès (espace › plaque) | Registre, UI04-COV | Repliable |
| 8 | StableID et diagnostics (raison d'inactivité, verrou Randomize) | Registre | Repliable |

En 0.4.1, le StableID s'affiche avant la valeur. R2 inverse cet ordre.

### 5.2 Exigences

| ID | Exigence | Type |
|---|---|---|
| WS-I01 | La hiérarchie §5.1 est respectée dans tous les modes | Obligatoire |
| WS-I02 | Les rangs 1 à 4 sont visibles sans défilement dans tous les modes | Obligatoire |
| WS-I03 | Trois modes : DOCKED (300 px, 0.4.1), BAND (bande horizontale compacte), DRAWER (tiroir superposé). Le choix est automatique d'après le canvas, avec une préférence utilisateur non persistée dans le preset. | Proposition (OD-05) |
| WS-I04 | Seuil candidat DOCKED : canvas logique restant ≥ 1 080 px avec l'Inspector ancré. En dessous : DRAWER en PERFORMANCE, BAND en SOUND DESIGN et ENGINEERING. | Seuil candidat (OD-05) |
| WS-I05 | Un changement de mode conserve le paramètre inspecté, l'épinglage et le focus | Obligatoire |
| WS-I06 | Le DRAWER ne recouvre jamais les rangs 1 à 9 de PERFORMANCE (il recouvre le moniteur) | Obligatoire |
| WS-I07 | Les actions PIN, LOCK, DEFAULT, ENTER, LEARN, + MOD restent atteignables en un clic dans tous les modes | Obligatoire |
| WS-I08 | Les alias de l'Inspector (vues de paramètres) sont mis en cache. Aucune destruction pendant une interaction (architecture 0.4.1 conservée). | Obligatoire |
| WS-I09 | BASE et EFFECTIVE suivent les formats de valeur de la nomenclature (§6), avec unité physique | Obligatoire |
| WS-I10 | En DOCKED, l'Inspector n'occupe pas plus de 25 % de la largeur de la fenêtre (0.4.1 : 21 % à 1 440 px, 31 % à 960 px) | Seuil candidat |
