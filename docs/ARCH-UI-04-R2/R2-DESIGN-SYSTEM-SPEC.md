# ARCH-UI-04-R2-A — design system (DA-A02 à DA-A06)

Ce document spécifie les familles de composants retenues : NEXUS Hybrid, NEURAL CELL Hybrid, Cybernetic Display et
Plasma Hybrid. Il couvre leurs dimensions, tokens, états, affichages numériques, comportement au zoom et gestes. Les
valeurs marquées **0.4.1** proviennent de `ui/DesignTokens.h` et `ui/Components.cpp` (relevé du code, non mesuré à
l'écran). Les valeurs marquées **seuil candidat** restent à confirmer avant mesure (OD-06).

## 1. Principes

| ID | Principe |
|---|---|
| DS-01 | Un composant ne définit ni couleur ni dimension en dur. Tout passe par `tok::` (règle 0.4.1 conservée). |
| DS-02 | Chaque état est porté par la forme et le texte, et pas seulement par la couleur (INV-12). Le test de contraste UI04-CON reste la référence (texte ≥ 4,5:1 sur `panel`, libellé inactif ≥ 3:1). |
| DS-03 | Trois traitements de surface distincts : **action** (impulsion), **bascule** (état persistant) et **sélection** (choix exclusif). Ils répondent au constat Codex DA-03 et P2-02 : toutes les actions partagent aujourd'hui un même dégradé argenté. |
| DS-04 | Précision mécanique pour ce qui se manipule, intelligence cybernétique pour ce qui informe, matière vivante réservée au Core (master §4). Un signal animé sans donnée publiée est interdit (INV-11). |
| DS-05 | Interdits : esthétique rétro, écrans CRT, lignes de balayage décoratives, néons excessifs, effets qui réduisent la lisibilité, textures envahissantes, faux instruments de mesure, animations qui laissent croire à une activité DSP inexistante. |

## 2. Tokens

### 2.1 Palette de référence et dérivés existants (inchangés)

`chassis #161A1D`, `carbon #0A0D10`, `titanium #343C43`, `text #DCE4E9`, `wt #5CE2CB`, `fm #F2A05E`, `cross #B095FF`,
`mod #7FA9E8`, `alert #E0524A` (alertes et PANIC uniquement) ; dérivés `panel #1C2125`, `panelEdge #2A3137`,
`glass #0D1215`, `glassEdge #26303A`, `textDim #98A5AE`, `textMute #7D8A93`, `track #10151A`, `focus #DCE4E9`,
`warn #E6C35A` (ARMED, attente d'autorisation).

### 2.2 Tokens proposés pour R2-B (dérivés, non normatifs avant validation)

| Token proposé | Rôle | Règle de dérivation | Contrôle |
|---|---|---|---|
| `relief0..relief3` | Niveaux de matière : châssis, plaque, corps de commande, surface d'action | Luminance croissante depuis `chassis`. Écart minimal entre deux niveaux adjacents : seuil candidat 1,15:1. | UI04-CON étendu |
| `actionFace`, `toggleFace`, `selectFace` | Les trois traitements DS-03 | Action : relief3 mat avec arête claire. Bascule : relief2 et témoin. Sélection : fond `carbon` et soulignement de domaine. | Revue visuelle et contraste |
| `effective` | Anneau et marqueur Effective | = `mod` | — |
| `measure` | Valeurs mesurées (crête, analyseurs) | = `text`, avec unité obligatoire | — |
| `inactive` | Paramètre conservé sans effet dans le contexte | = `textMute`, avec motif hachuré | ≥ 3:1 |
| `glow` | Intensité maximale d'une lueur | Opacité ≤ 0,35, seuil candidat | Revue subjective |

## 3. Typographie

| Élément | 0.4.1 | Exigence R2 |
|---|---|---|
| Plancher logique | `fontMicro` 9,5 px | Conservé. Aucun libellé n'est plus petit : au-delà du budget, il passe sur deux lignes. |
| Libellé / valeur | `fontLabel` / `fontValue` 11 px | Conservé pour NEXUS S, NEURAL CELL, SYNAPTIC TRACK |
| Titre de plaque | `fontTitle` 12,5 px | Conservé |
| En-tête / Inspector | `fontHead` 15 px | Nom complet dans l'Inspector |
| Valeur XL | `fontXL` 14 px | Conservé |
| Corps écran à 75 % | 9,5 × 0,75 = 7,125 px (constat Codex DA-04) | **Seuil candidat** : ≥ 9 px écran pour les repères critiques (PANIC, PEAK, état d'autonomie, axes du Core, WT X/Y/Z, CROSS LVL, macros). Il impose un corps logique ≥ 12 px pour ces repères ou une typographie dépendante du zoom (OD-06). |
| Chiffres | tabulaires (`numFont`) | Conservé. Précision par grandeur ([R2-CONTROL-NAMING-SPEC.md](R2-CONTROL-NAMING-SPEC.md) §6). |

## 4. NEXUS Hybrid (DA-A03)

Identité commune : corps en titane noir, périphérie usinée ou moletée, index précis, relief mécanique, arc de valeur coloré
selon le moteur, Base distincte d'Effective.

### 4.1 Gabarits

| Taille | Cellule 0.4.1 (px log.) | Budget de libellé (px log.) | Corps du libellé | Usage | Arc |
|---|---|---|---|---|---|
| NEXUS XL | 132 × 150 | 128 (w − 4) | 12,5 | Commandes majeures PERFORMANCE (WT X, WT Y, WT Z, CROSS). **Richesse visuelle maximale** : moletage, relief, anneau Effective, marques de route. | Dérivé de la cellule. À figer en R2-B. |
| NEXUS S | 84 × 96 | 80 (96 pour les macros) | 11 | Édition courante. **Traitement mécanique simplifié** : périphérie usinée sans moletage. | idem |
| NEXUS Micro | 64 × 74 | 60 (80 dans les cellules de réseau) | 10 | Matrices, réseaux, listes denses. **Priorité absolue à la lisibilité** : index, arc, valeur. Pas de marque de route (détail dans l'Inspector). | idem |

Zones interactives (0.4.1, conservées) : toute la cellule pour le glisser, la molette et le clic droit ; la zone de
valeur pour la saisie (double-clic) ; hors de la zone de valeur, le double-clic rétablit le défaut (PB-01).

Les budgets sont ceux mesurés par Codex (`../ARCH-UI-04-DA/CONTROL-LABELS-AUDIT.md`). Une modification de gabarit en
R2-B DOIT être répercutée dans la colonne « Budget » du CSV et revérifiée par mesure JUCE (ACC NAM-08).

### 4.2 Anatomie

| Élément | Donnée | Règle |
|---|---|---|
| Index de Base | Valeur Base normalisée | Trait `text` sur le corps `titanium`. Toujours visible. Prioritaire sur tout autre marqueur (constat Codex DA-02). |
| Arc de Base | Base | Couleur du domaine (WT, FM, CROSS, Mod, neutre) |
| Anneau Effective | `effective[id]` si `modulated[id]` | Anneau extérieur `effective`, indépendant de l'arc. Absent si non modulé. Le texte de valeur reste celui de la Base. |
| Marques de route | Profondeurs configurées des routes globales vers ce paramètre | Au plus une marque par route, jusqu'à quatre. Au-delà : badge « +n » et détail dans l'Inspector (Codex DA-02 : réserver les marques à la densité faible). |
| Valeur | Base, unité | Chiffres tabulaires. L'unité est masquée pour les domaines normalisés `lin`, `pos` et `bip` (§6 de la nomenclature). |
| Libellé | Alias de sérigraphie | [R2-CONTROL-NAMING-SPEC.md](R2-CONTROL-NAMING-SPEC.md) |

### 4.3 États

| État | Déclencheur | Forme | Texte |
|---|---|---|---|
| Repos | — | Corps `titanium` | Libellé, valeur |
| Survol (hover) | Pointeur | Arête `panelEdge` éclaircie | — |
| Focus clavier | Tab | Anneau `focus` de 1,5 px | — |
| Actif (geste) | Glisser, molette, clavier | Index épaissi | Valeur en surbrillance |
| Désactivé | Commande hors registre momentanément sans objet dans l'état courant | Corps `relief1`, sans arc | « — » et raison dans l'infobulle. **Jamais pour un paramètre du registre** : il est INACTIVE (INV-07). |
| Fin | Maj pendant le geste | Graduations fines visibles | « FINE » |
| Saisie | Double-clic sur la valeur, Entrée | Champ numérique | Unité affichée |
| Modulé | `modulated[id]` | Anneau Effective | « MOD » au focus |
| Inactif | Contexte sans effet (mode, type, route absente) | Motif `inactive` | « INACTIVE » et raison dans l'infobulle. La valeur reste éditable (INV-07). |
| Verrouillé | Verrou Randomize | Glyphe de verrou | « LOCK » au focus |
| Appris MIDI | CC assigné | Badge « CC n » | — |
| Apprentissage en attente | MIDI Learn armé | Contour pointillé | « LEARN… » |

L'état « automation lue par l'hôte » n'est pas détectable de façon fiable par l'API VST3. Il n'est donc pas affiché
(INV-11, OD-09).

### 4.4 Gestes (0.4.1, conservés)

| Geste | Effet | Transaction |
|---|---|---|
| Glisser vertical | 220 px pour la plage complète (`dragPixelsFull`) | Un geste hôte |
| Maj pendant le glisser | Facteur 0,1. Le changement de mode en cours de geste repart de la position. | Même geste |
| Molette | Pas normalisé = 0,05 × `deltaY` JUCE (× 0,1 avec Maj). Pas de valeur par cran fixée : elle dépend du périphérique. | Un geste jusqu'à 450 ms d'inactivité (`wheelGestureMs`) |
| Flèches | 1 % (`keyStep`). Int/Choice : un pas. | Un geste par appui |
| Page Haut/Bas | 10 % (`pageStep`) | idem |
| Début / Fin | Min / max | idem |
| Suppr / Retour arrière, Alt+clic | Valeur par défaut | idem |
| Double-clic sur la valeur, Entrée | Saisie numérique | Un geste à la validation |
| Double-clic hors valeur | Valeur par défaut | **PB-01** : deux gestes hôte imbriqués possibles (début, début, fin, fin) |
| Clic droit | Menu : défaut, saisie, MIDI Learn, modulation, verrou | — |

R2-B NE DOIT PAS modifier ces gestes sans traiter PB-01 dans un périmètre autorisé (INV-10, OD-10). Un changement de
gabarit ne change pas la course de 220 px.

### 4.5 Zoom

Le zoom est une transformation globale (75, 100, 150 %). Toutes les dimensions ci-dessus sont logiques. À 75 %, NEXUS
Micro tombe à 7,5 px écran (constat Codex). Le seuil candidat §3 s'applique aux repères critiques. Les autres libellés
Micro DEVRAIENT passer sur deux lignes plutôt que descendre sous le plancher.

## 5. NEURAL CELL Hybrid (DA-A04)

### 5.1 Types

| Type | Composant 0.4.1 | Forme | Témoin | Texte d'état | Exemples |
|---|---|---|---|---|---|
| Action instantanée | `ActionCell` | `actionFace`, relief à l'appui | Aucun | Aucun (le libellé est un verbe) | CAPTURE, CROSS RESET, EXCITE, + ROUTE, UNDO, SAVE…, AUTHORIZE |
| Bascule | `NeuralCell` (Bool) | `toggleFace` | Disque plein si ON | OFF / ON | SYNC, LOOP, LIMITER, FREEZE, MUTE |
| Armement | `NeuralCell` (`*.arm`, `voice.drone`) | `toggleFace`, liseré `warn` | Anneau (ARMED) ; disque (ACTIVE) | OFF / ARMED / ACTIVE / DENIED | ARM ×9, DRONE |
| Sélection exclusive | `SynapticTrack` (segments), `TabStrip` | `selectFace`, soulignement du domaine | Segment rempli | Libellé de l'option | modes, types, onglets, NEURAL/TABLE |
| Sûreté | PANIC | Rouge `alert`, isolée, toujours visible | Verrou Panic signalé | PANIC ; REVOKED dans l'état d'autonomie | PANIC (comportement inchangé) |
| Indisponible | `CellState::Disabled` | Contour seul | Aucun | « — » et raison dans l'infobulle | CAPTURE sans contenu gelé, UNDO à historique vide |

Matière : touches mécaniques en titane noir, témoins intégrés. Les états 0.4.1 (`Off`, `Armed`, `Active`, `Refused`,
`Disabled`) sont conservés. Seuls leurs textes passent en
anglais (ARMÉ → ARMED, ACTIF → ACTIVE, REFUS → DENIED).

### 5.2 Distinction FREEZE / ARM / AUTHORIZE / CAPTURE (NC-05)

| Opération | Type | Persistée | Automatisable | Périmètre | Où |
|---|---|---|---|---|---|
| FREEZE | Bascule (paramètre `*.d2.freeze`, `*.s2.freeze`) | Oui (Base) | Oui (registre) | Un étage D2/S2 | Plaque de l'étage |
| ARM | Armement (paramètre `*.arm`) | Oui (Base) | Non (registre) | Un module autonome | Plaque du module, résumé ARMED en PERFORMANCE |
| AUTHORIZE | Action instantanée | Non (instance) | Non | Toute l'instance | Barre globale, visible si au moins un module est armé et que l'exécution est suspendue |
| CAPTURE | Action instantanée | Le contenu capturé est enregistré avec le preset et le projet | Non | Les six emplacements D2/S2 (WT1…GL2) | Process et STATE › Captures |

Les quatre opérations DOIVENT différer par le type, le libellé, la forme et l'emplacement. La recette compare les états
avant et après restauration, dans les contextes D2, S2 et F4 (Codex P1-03). Le contrat d'autonomie 0.4.1 reste inchangé
(INV-04). Pendant un rendu hors ligne, la télémétrie publie `autonomySource = Render` et `offline`. L'état d'autonomie
DOIT alors afficher RENDER, jamais AUTHORIZED. Après le rendu, il revient à l'état normal publié
([R2-TELEMETRY-FEASIBILITY.md](R2-TELEMETRY-FEASIBILITY.md) T-14).

### 5.3 Dimensions

Cellule 0.4.1 : 168 × 40. Libellé utile w − 69 (99 à 168 px selon la cellule), corps 11, deux lignes au plus.
Témoin à gauche, texte d'état à droite sur 40 px (`sw`). Cible tactile et pointeur ≥ 24 px de haut (seuil candidat
aligné sur WCAG 2.5.8).

## 6. Cybernetic Display (DA-A05)

Option **D** retenue, et non Optical Glass Hybrid : instrumentation résolument futuriste. Caractéristiques obligatoires :
graphismes cybernétiques, graduations techniques, marqueurs lumineux maîtrisés, présentation dynamique des données
**disponibles**, encadrements et repères optiques, hiérarchie stricte entre données principales et secondaires
(principale : corps ≥ 11, `text` ; secondaire : corps ≤ 10,5, `textDim`). Le verre (`glass`, `glassEdge`) reste le fond
et la monture. Pas de balayage, pas de rémanence simulée, pas de grain (DS-05).

| Variante | Contenu | Source de données | Disponibilité | Contrainte |
|---|---|---|---|---|
| Numérique | Valeur, unité, état | Paramètre Base ou Effective | Disponible | Chiffres tabulaires, précision par grandeur |
| Courbe | Courbe de modulation, courbe custom (8 points au plus) | `ModRoute` (thread de contrôle) | Disponible | Édition dans l'éditeur dédié, Undo |
| Enveloppe | Forme DAHDSR configurée | Paramètres Base | Disponible | Le niveau instantané n'existe que pour AMP (`envelope`). L'étape courante n'est **pas publiée** (T-08). Pas de curseur d'étape. |
| Table WT | Formes de la banque, position X/Y/Z | Banque (thread de contrôle), `wtX/Y/Z` effectifs | Disponible | Base = réticule, Effective = point `effective` |
| Analyseurs | Oscilloscope, spectre (INDICATIVE, C03) | `ScopeRing` (4 096 valeurs, mono, décimation × 2, sortie finale après gain, limiteur et rampe PANIC) | Disponible | Spectre calculé côté UI. Le point de prélèvement est nommé à l'écran. |
| Crête | PEAK dBFS L/R | `peakL/peakR` (crête de bloc, sortie finale) | Disponible | Toujours visible (DA-A08). Maintien de crête calculé côté UI. Rouge seulement au-delà de 0 dBFS. |
| États / diagnostics | Autonomie, transport, compteurs de sûreté, CPU | Télémétrie (§3 du document télémétrie) | Disponible | `cpuLoad` = dernier bloc seulement, pas de moyenne publiée |
| Ordre Z | Ordre effectif des transformations Z | Moteur | Disponible | Restriction inter-domaines non validée, signalée comme telle |

Toute valeur illustrative (rendu statique du Core) porte la mention STATIC PREVIEW et n'est jamais présentée comme une
mesure (CD-04).

## 7. Plasma Hybrid (DA-A06)

| Élément | Règle |
|---|---|
| Gabarits 0.4.1 | Vertical 48 × 150, horizontal 220 × 40. Légende horizontale (w − 4) × 0,34. |
| Curseur mécanique | Curseur en titane noir à arête usinée, aligné sur la Base. C'est la zone de prise du geste. |
| Canal lumineux de Base | Remplissage de la couleur du domaine, du minimum (ou du centre pour `bip`) à la Base |
| Marqueur Effective | Trait transversal `effective` avec pointe, indépendant de la colonne. Visible seulement si `modulated[id]`. |
| Graduations | dB : 0, −6, −12, −24, −48. `lin`/`pos` : quarts. `bip` : centre marqué. Libellés de graduation ≥ plancher. |
| Mesure audio | Jamais dans un canal de paramètre. La crête de sortie est un afficheur PEAK distinct (`measure`, unité dBFS) placé à côté de OUT GAIN. Il n'est pas surimprimé. |
| Gestes | Ceux du NEXUS (§4.4), axe selon l'orientation |

## 8. Accessibilité

| ID | Exigence |
|---|---|
| AX-01 | Chaque commande liée à un StableID expose son nom complet anglais (titre accessible) et sa valeur. La présence AX a été relevée par Codex sur macOS (`Présence AX physique`). |
| AX-02 | L'ordre de tabulation suit l'ordre visuel. Le focus n'est jamais perdu lors d'un repli de disposition (Codex P1-04). |
| AX-03 | Toute action au pointeur a un équivalent clavier (déjà vrai pour NEXUS, SYNAPTIC et NEURAL CELL ; à garantir pour le Core et les réseaux). |
