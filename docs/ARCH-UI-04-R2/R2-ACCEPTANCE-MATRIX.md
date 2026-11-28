# ARCH-UI-04-R2-A — matrice d'acceptation

Chaque critère est enregistré **avant mesure**. Il ne peut pas être abaissé après mesure (règle du tracker §3). Un critère
non exécuté reste NON EXÉCUTÉ, jamais PASS (INV-09).

**Nature** : **O** obligatoire · **OBJ** objectif de conception (à démontrer, échec documenté si non atteint) ·
**SC** seuil candidat (valeur à confirmer par le propriétaire avant mesure) · **F** étude de faisabilité (livrable =
mesure + conclusion).
**Vérification** : **AUTO** mesure automatique (tests core/wrapper, CI) · **PHYS** recette physique (Mac Studio M4 Max,
Reason 14) · **SUBJ** validation subjective du propriétaire · **AUDIT** contrôle documentaire ou statique.

Les performances graphiques ne sont pas certifiées à ce stade. Les budgets de coût de rendu sont des seuils candidats à
mesurer en prototype. Aucun résultat de mesure n'est présumé.

## 1. Invariants (tous les lots)

| ID | Critère | Nature | Vérif. | Lot |
|---|---|---|---|---|
| INV-01 | `git diff d2d08ac -- source/` vide ; REG-01 et goldens identiques | O | AUTO | tous |
| INV-02 | `tests/core/expected_param_ids.txt` inchangé ; noms hôtes identiques au registre (UI-01w) | O | AUTO | tous |
| INV-03 | Format d'état 4. Basculer de vue, replier, zoomer ou changer d'onglet ne modifie ni l'état ni le drapeau « modifié » (test : sérialisation avant/après identique octet pour octet) | O | AUTO | B–G |
| INV-04 | Suite TA (TA-01…, TA-R01…R16, TA-R01w…R15w) inchangée et dans son état documenté | O | AUTO | tous |
| INV-05 | Aucune route créable hors des 55 emplacements ; classes causales inchangées (tests B-T2 existants) | O | AUTO | F |
| INV-06 | Comptage `begin`/`end` équilibré pour chaque geste de chaque famille (extension d'UI04) | O | AUTO | B–F |
| INV-07 | UI04-COV : 703/703 StableID atteints, y compris les conditionnels | O | AUTO | tous |
| INV-08 | Aucune dépendance, aucun SDK ni licence nouvelle ; aucun rendu 3D avant ARCH-UI-05 | O | AUDIT | tous |
| INV-09 | `tests/known_failures.txt` inchangé, sauf décision consignée ; R-29/R-30 toujours NON EXÉCUTÉS | O | AUDIT | tous |
| INV-10 | PB-01/PB-02 traités seulement dans un périmètre autorisé (OD-10) | O | AUDIT | B, C, D |
| INV-11 | Inventaire « donnée affichée → source publiée » sans entrée orpheline | O | AUDIT | B–G, 05 |
| INV-12 | Rouge réservé à l'alerte et à PANIC ; chaque état porte forme + texte | O | AUTO + SUBJ | B |
| INV-13 | Modifications `plugin/` et `ui/` sans effet audio : REG-01, suite TA, test « aucune écriture pendant la navigation » | O | AUTO + AUDIT | tous |

## 2. Nomenclature

| ID | Critère | Nature | Vérif. | Lot |
|---|---|---|---|---|
| NAM-01 | Aucune chaîne visible française (balayage de tous les composants visibles, tous contextes, comme UI04-DECO) | O | AUTO | B |
| NAM-02 | Titre accessible = nom du registre pour 703/703 StableID | O | AUTO | B |
| NAM-03 | Chaque libellé court du CSV validé est affiché à son emplacement (comparaison table ↔ composants) | O | AUTO | B |
| NAM-04 | Le nom complet apparaît au focus, dans l'Inspector et dans l'infobulle | O | AUTO | B, E |
| NAM-05 | Une fonction, un libellé, à contexte égal (contrôle de la table) | O | AUTO | B |
| NAM-06 | Aucun doublon de libellé dans un même champ visuel sans en-tête (détection automatique sur les plaques) | O | AUTO | B |
| NAM-07 | Les distinctions critiques (WT Z, CROSS LVL, WT→FM, FM→WT, BALANCE, XAUDIO AMT, OUT GAIN, PEAK) sont présentes et uniques | O | AUTO + SUBJ | B |
| NAM-08 | Largeur mesurée JUCE ≤ budget de la famille pour 100 % des occurrences, sinon deux lignes admises ; corps ≥ 9,5 px | O | AUTO | B |
| NAM-09 | Abréviations conformes au lexique | O | AUTO | B |
| NAM-10 | Indices et libellés hôtes des 537 options inchangés | O | AUTO | B |
| NAM-11 | Unités physiques présentes ; aucune unité `lin`/`pos`/`bip` en façade | O | AUTO | B |
| NAM-12 | Grammaire `SOURCE→DEST` identique en PERFORMANCE, SOUND DESIGN, ENGINEERING et Inspector | O | AUTO | B, F |
| NAM-13 | Glyphes `→ · … −` rendus par la police embarquée, sinon repli appliqué (OD-11) | O | AUTO | B |
| NAM-14 | Lisibilité de la sérigraphie anglaise compacte | OBJ | SUBJ | B |

## 3. Design system

| ID | Critère | Nature | Vérif. | Lot |
|---|---|---|---|---|
| DS-01 | Aucune couleur ou dimension en dur hors `tok::` (balayage statique) | O | AUDIT | B |
| DS-02 | Contraste texte ≥ 4,5:1, libellé inactif ≥ 3:1 (UI04-CON étendu aux nouveaux tokens) | O | AUTO | B |
| DS-03 | Trois traitements action / bascule / sélection distinguables | OBJ | SUBJ | B |
| DS-04 | Écart de luminance entre niveaux de relief ≥ 1,15:1 | SC | AUTO | B |
| DS-05 | Aucun des interdits du brief (rétro, CRT, balayage, néons excessifs, textures envahissantes, faux instruments de mesure, animation sans activité DSP) | O | SUBJ + AUDIT | B–G, 05 |
| NX-01 | Index de Base visible dans tous les états (y compris modulé, inactif, 75 %) | O | AUTO + SUBJ | B |
| NX-02 | Anneau Effective présent si et seulement si `modulated[id]` | O | AUTO | B |
| NX-03 | La valeur affichée est la Base | O | AUTO | B |
| NX-04 | Gabarits et budgets du CSV mis à jour si un gabarit change | O | AUDIT | B |
| NX-05 | Gestes identiques à 0.4.1 : 220 px plage complète, Maj × 0,1, flèches 1 %, Page 10 %, Début/Fin, défaut, saisie (deltas normalisés comparés) | O | AUTO | B |
| NX-06 | Un geste = une paire `begin`/`end` (molette : regroupement à 450 ms) | O | AUTO | B |
| NX-07 | État INACTIVE : motif + texte + raison ; valeur éditable | O | AUTO | B |
| NX-08 | Anneau de focus visible au clavier | O | AUTO + SUBJ | B |
| NX-09 | Au plus quatre marques de route, puis « +n » | O | AUTO | B |
| NX-10 | Aucun libellé sous 9,5 px logiques | O | AUTO | B |
| NX-11 | Repères critiques ≥ 9 px écran à 75 % | SC | AUTO | B |
| NX-12 | Automation hôte reflétée à l'écran en ≤ 2 périodes de rafraîchissement (≤ 67 ms) | SC | AUTO | B |
| NC-01 | Six types de cellule distinguables par forme et texte | O | AUTO + SUBJ | B |
| NC-02 | Chaque état a un texte distinct (OFF, ON, ARMED, ACTIVE, DENIED, —) | O | AUTO | B |
| NC-03 | PANIC inchangé (position dans la barre, comportement, tests existants) | O | AUTO + PHYS | B, C |
| NC-04 | ARM affiche OFF / ARMED / ACTIVE selon l'armement et l'autorisation publiés | O | AUTO | B |
| NC-05 | FREEZE, ARM, AUTHORIZE et CAPTURE diffèrent par type, libellé, forme et emplacement ; recette avant/après restauration en D2, S2 et F4 | O | AUTO + PHYS | B |
| NC-06 | AUTHORIZE visible si et seulement si au moins un module est armé et que l'exécution est suspendue (équivalent AUT-08) | O | AUTO | B |
| NC-07 | Pendant un export hors ligne, l'état affiche RENDER, jamais AUTHORIZED ; retour à l'état normal après le rendu | O | AUTO + PHYS | B |
| NC-08 | Cellule indisponible : « — » et raison dans l'infobulle | O | AUTO | B |
| NC-09 | Hauteur de cible ≥ 24 px logiques | SC | AUTO | B |
| NC-10 | Contrat d'autonomie 0.4.1 : aucun changement de comportement (suite TA) | O | AUTO | tous |
| CD-01 | Chaque afficheur liste sa source publiée (INV-11) | O | AUDIT | B |
| CD-02 | Point de prélèvement affiché pour l'oscilloscope, le spectre et la crête | O | AUTO | B |
| CD-03 | Aucun effet CRT, balayage, grain ni rémanence simulée | O | SUBJ | B |
| CD-04 | Mentions STATIC PREVIEW et ESTIMATE présentes là où elles s'appliquent | O | AUTO | B, G |
| CD-05 | Maintien de crête calculé côté UI ; rouge seulement au-delà de 0 dBFS | O | AUTO | B |
| CD-06 | Bande du spectre (fs/4) indiquée | O | AUTO | B |
| CD-07 | Pas de curseur d'étape d'enveloppe (donnée non publiée) | O | AUDIT | B, D |
| CD-08 | Restriction Z inter-domaines signalée comme non validée | O | AUTO | B, D |
| PL-01 | Colonne de Base, depuis le minimum ou le centre (`bip`) | O | AUTO | B |
| PL-02 | Marqueur Effective indépendant, visible si et seulement si modulé | O | AUTO | B |
| PL-03 | Graduations selon le domaine | O | AUTO | B |
| PL-04 | Aucune mesure audio dans un canal de paramètre | O | AUDIT | B |
| PL-05 | PEAK (dBFS) distinct et adjacent à OUT GAIN | O | AUTO + SUBJ | B, C |
| PL-06 | Gestes identiques au NEXUS | O | AUTO | B |
| AX-01 | Nom complet et valeur exposés en accessibilité pour 703/703 | O | AUTO + PHYS | B |
| AX-02 | Ordre de tabulation = ordre visuel ; focus conservé au repli | O | AUTO | C, E |
| AX-03 | Équivalent clavier pour toute action au pointeur | O | AUTO | B–F |

## 4. Espaces de travail

| ID | Critère | Nature | Vérif. | Lot |
|---|---|---|---|---|
| WS-P01 | Aucun défilement en PERFORMANCE à 1440 × 900, 100 %, Inspector ancré et fermé | O | AUTO | C |
| WS-P02 | Aucun défilement en PERFORMANCE à 960 × 640, 100 %, Inspector fermé | OBJ (géométrie puis prototype) | AUTO + PHYS | C |
| WS-P03 | Rangs 1 à 9 visibles sans défilement à 960 × 640, Inspector ouvert | OBJ | AUTO + PHYS | C, E |
| WS-P04 | Élément replié : indicateur visible, accès en un clic | O | AUTO | C |
| WS-P05 | Disposition choisie sur le canvas logique ; changement différé après le geste | O | AUTO | C |
| WS-P06 | Sélection, focus et destination conservés au changement de disposition | O | AUTO | C |
| WS-P07 | Barre sur une rangée à 1440 px avec AUTHORIZE | SC | AUTO | C |
| WS-P08 | Enveloppe du Core respectée dans chaque disposition | O | AUTO | C |
| WS-P09 | Rangs 4 à 9 pilotables au clavier | O | AUTO | C |
| WS-P10 | Aucune mesure simulée en PERFORMANCE | O | AUDIT | C |
| WS-P11 | Noms de macro éditables | O | AUTO | C |
| WS-P12 | STATIC PREVIEW secondaire face aux axes | O | SUBJ | C |
| WS-P13 | UNDO/REDO ≥ 50 px | SC | AUTO | C |
| WS-P14 | Seuils WIDE / STANDARD / COMPACT / CONSTRAINED | SC | AUTO | C |
| WS-P15 | PANIC et crête accessibles à 75, 100 et 150 %, dans les trois espaces | O | AUTO | C |
| WS-P16 | Façade cohérente et musicale | OBJ | SUBJ | C |
| WS-S01 | Changement de contexte par action explicite seulement | O | AUTO | D |
| WS-S02 | Aucune redistribution pendant un geste ; ancrage de la commande sous le pointeur | O | AUTO | D |
| WS-S03 | Ordre du moteur affiché, ordre Z effectif | O | AUTO + AUDIT | D |
| WS-S04 | Paramètres conditionnels atteignables avec INACTIVE et raison | O | AUTO | D |
| WS-S05 | Panneaux persistants par onglet, sans reparentage pendant l'interaction (ASan) | O | AUTO | D |
| WS-S06 | Réglages liés rapprochés (FB MODE de D2 au premier écran) | OBJ | SUBJ | D |
| WS-S07 | Pas de défilement horizontal au nominal, Inspector ancré | O | AUTO | D |
| WS-S08 | Défilement vertical seul au minimum, Inspector fermé | SC | AUTO | D |
| WS-S09 | WT, FM et CROSS simultanément accessibles | O | AUTO | D |
| WS-S10 | Répartition par contexte ; pas de grande zone vide face à un défilement évitable | OBJ | AUTO + SUBJ | D |
| WS-E01 | Dix outils présents | O | AUTO | E |
| WS-E02 | Navigation à deux niveaux ; dernier outil mémorisé hors preset | O | AUTO | E |
| WS-E03 | Liens (groupe, outil) stables ; PRESETS et SAVE… ouvrent STATE › PRESETS | O | AUTO | E |
| WS-E04 | Une seule navigation de preset par contexte | OBJ | SUBJ | E |
| WS-E05 | Zone de détail utile dans les outils courts | OBJ | SUBJ | E |
| WS-E06 | DIAGNOSTICS sans valeur inventée | O | AUDIT | E |
| WS-E07 | Pas de redistribution pendant un geste | O | AUTO | E |
| WS-E08 | Au plus +1 clic par rapport à 0.4.1 pour chaque outil (§4.3 des espaces) | SC | AUTO | E |
| WS-E09 | Espace de programmation, pas console de développement | OBJ | SUBJ | E |
| WS-I01 | Hiérarchie 1 à 8 dans tous les modes | O | AUTO | E |
| WS-I02 | Rangs 1 à 4 visibles sans défilement | O | AUTO | E |
| WS-I03 | Modes DOCKED / BAND / DRAWER | SC (OD-05) | AUTO | E |
| WS-I04 | Seuil d'ancrage : canvas restant ≥ 1 080 px | SC | AUTO | E |
| WS-I05 | Paramètre inspecté, épinglage et focus conservés au changement de mode | O | AUTO | E |
| WS-I06 | Le DRAWER ne recouvre pas les rangs 1 à 9 | O | AUTO | E |
| WS-I07 | Actions de l'Inspector en un clic dans tous les modes | O | AUTO | E |
| WS-I08 | Alias de l'Inspector en cache, sans destruction en interaction (ASan) | O | AUTO | E |
| WS-I09 | Formats de valeur et unités de la nomenclature | O | AUTO | E |
| WS-I10 | Inspector DOCKED ≤ 25 % de la largeur de fenêtre | SC | AUTO | E |

## 5. Core

| ID | Critère | Nature | Vérif. | Lot |
|---|---|---|---|---|
| CORE-01 | Axes X/Y/Z inchangés ; aucun mapping ni paramètre nouveau | O | AUTO | C, 05 |
| CORE-02 | Écritures limitées aux trois paramètres, par geste utilisateur | O | AUTO | C, 05 |
| CORE-03 | Libellés d'axe visibles et non recouverts | O | AUTO + SUBJ | C, 05 |
| CORE-04 | Pad ≥ 300 px logiques (WIDE, STANDARD, COMPACT) | SC | AUTO | C |
| CORE-05 | Pad ≥ 220 px écran à tout zoom | SC | AUTO | C |
| CORE-06 | Rendu rogné au pad | O | AUTO | 05 |
| CORE-07 | Le Core ne bouge pas pendant un geste | O | AUTO | C |
| CORE-08 | Gestes 0.4.1 conservés | O | AUTO | C, 05 |
| CORE-09 | Clavier X/Y (1 %, 0,1 %) ; Z par Page Haut/Bas (proposition) | O / SC | AUTO | C |
| CORE-10 | Saisie numérique des trois axes | O | AUTO | C |
| CORE-11 | Gestes refermés au relâchement, à la perte de capture, au masquage et à la destruction (sous OD-10) | O sous autorisation | AUTO | C |
| CORE-12 | Base = réticule ; Effective = point si modulé | O | AUTO | C, 05 |
| CORE-13 | Automation reflétée sans écriture en retour | O | AUTO | C, 05 |
| CORE-14 | Repli 2D complet aux liaisons identiques | O | AUTO | C, 05 |
| CORE-15 | Budget de rendu ≤ 4 ms par image | SC | AUTO + PHYS | 05 |
| CORE-16 | L'animation ne compromet jamais les gestes (zone, réticule, latence, libellés) | O | AUTO + SUBJ | 05 |

## 6. Réseaux

| ID | Critère | Nature | Vérif. | Lot |
|---|---|---|---|---|
| NN-01 | Huit états de route distingués par trait, flèche et texte | O | AUTO + SUBJ | F |
| NN-02 | Profondeur configurée et activité visuellement distinctes ; activité issue de données publiées | O | AUTO + AUDIT | F |
| NN-03…NN-08 | Interdits du §6 de la spécification réseau (connexion libre, causalité, ordre, activité non publiée, second chemin d'écriture, suppression de TABLE) | O | AUTO | F |
| NN-09 | Positions de nœuds fixes (comparaison avant/après ajout et retrait) | O | AUTO | F |
| NN-10 | Aucun déplacement pendant un geste | O | AUTO | F |
| NN-11 | Filtres sans écriture | O | AUTO | F |
| NN-12 | Focus : estompage à 25 % | SC | SUBJ | F |
| NN-13 | Lisibilité à densité maximale (16 / 39 / 64) ; faisceaux au-delà de 24 liaisons visibles | F + SC | AUTO + SUBJ | F |
| NN-14 | Aucun chevauchement de libellés de nœud | O | AUTO | F |
| NN-15 | Navigation clavier complète | O | AUTO | F |
| NN-16 | Libellés de nœud au seuil critique à 75 % | SC | AUTO | F |
| NN-17 | Un seul modèle, mêmes chemins d'écriture | O | AUDIT + AUTO | F |
| NN-18 | Identités de route identiques et visibles dans les deux vues | O | AUTO | F |
| NN-19 | Sélection synchronisée NEURAL ↔ TABLE ↔ Inspector | O | AUTO | F |
| NN-20 | Basculer de vue n'écrit rien (INV-03) | O | AUTO | F |
| NN-21 | Même action = même transaction et mêmes gestes dans les deux vues | O | AUTO | F |
| NN-22 | Colonnes TABLE complètes | O | AUTO | F |
| NN-23 | Choix de vue mémorisé hors preset ; NEURAL par défaut | O | AUTO | F |
| NN-24 | Classe causale identique dans les deux vues | O | AUTO | F |
| NN-25 | Coût de rendu ≤ 2 ms par image à 64 + 55 liaisons | F + SC | AUTO + PHYS | F |
| NN-26 | Modèles de routage propres à chaque réseau (FM, CROSS, matrice) | O | AUDIT + SUBJ | F |
| NN-27 | Routes inactives autorisées atteignables (SHOW AVAILABLE, clavier) et activables par transaction validée | O | AUTO | F |

## 7. Télémétrie et flux

| ID | Critère | Nature | Vérif. | Lot |
|---|---|---|---|---|
| TEL-01 | Données de classe A/B seulement | O | AUDIT | G, 05 |
| TEL-02 | Mention ESTIMATE pour les dérivations approximatives | O | AUTO | G |
| TEL-03 | Aucune donnée C ou D sans mission autorisée | O | AUDIT | G |
| TEL-04 | Animations ≤ 30 Hz, arrêtées éditeur masqué | O | AUTO | G |
| TEL-05 | Silence et suspension affichés explicitement | O | AUTO + SUBJ | G |
| TEL-06 | Point de prélèvement indiqué | O | AUTO | G |
| TEL-07 | Aucune allocation ni verrou ajoutés sur le thread audio (RTSan en CI) | O | AUTO | G |
| TEL-08 | Coût de la réactivité mesuré | F | AUTO + PHYS | G |
| TEL-09 | Modes OFF / REACTIVE / IMMERSIVE, si OD-07 les valide | SC | AUTO + SUBJ | G |

## 8. Critères transversaux (brief §21)

| ID | Critère | Nature | Vérif. | Lot |
|---|---|---|---|---|
| UR-01 | Undo/Redo : chaque action utilisateur = une entrée ; Undo puis Redo restaure l'état octet pour octet ; aucune entrée pour la navigation | O | AUTO | B–F |
| RS-01 | Redimensionnement ou changement de zoom pendant un geste : geste conservé ou proprement refermé, aucune redistribution avant la fin du geste, aucune erreur ASan | O | AUTO | C–E |
| ED-01 | Destruction de l'éditeur pendant un geste (NEXUS, Core, enveloppe, réseau) : aucun crash ; gestes hôte refermés. La fermeture relève de PB-02 et n'est exigible que si OD-10 l'autorise. Sinon, l'état reste « non corrigé, documenté ». | O (crash) / O sous OD-10 (fermeture) | AUTO | B–F |
| MI-01 | Instances multiples : états d'interface indépendants (vue, disposition, Inspector, choix NEURAL/TABLE), aucun état statique partagé, PANIC et autorisation propres à chaque instance | O | AUTO + PHYS | B–H |
| SV-01 | Compatibilité des sauvegardes : presets et projets 0.4.1 rechargés à l'identique (format 4, indices de choix, captures, configuration Drone) | O | AUTO + PHYS | B–H |
| GC-01 | Coût graphique de l'éditeur au repos et en interaction (30 Hz) mesuré et publié dans chaque rapport de lot, sans certification | F | AUTO + PHYS | B–H |
| RT-01 | Temps réel : RTSan 0 violation, aucune allocation ni verrou ajoutés côté audio, pluginval rigueur 10 | O | AUTO | tous |
| ZM-01 | Zooms 75, 100 et 150 % dans les trois espaces : aucun chevauchement, toutes les commandes atteignables, repères critiques au seuil OD-06 | O / SC | AUTO | B–E |
| VAL-01 | Validation artistique du propriétaire à la fin de chaque lot visuel, puis globale en R2-H | O | SUBJ | B–H |

## 9. Couverture du brief §21

| Exigence minimale | Critères |
|---|---|
| 15 arbitrages | Registre des décisions, colonne « Exigences dérivées » |
| Six recommandations P1 de Codex | §10 |
| P2 et P3 pertinentes | §10 |
| 703 StableID et leurs représentations | INV-02, INV-07, NAM-02, NAM-03, AX-01 |
| Contextes conditionnels | WS-S04, NX-07, INV-07 |
| Lisibilité des libellés, budgets typographiques | NAM-08, NX-10, NX-11, NAM-14 |
| Cohérence linguistique | NAM-01, NAM-05, NAM-12 |
| Contrastes | DS-02, DS-04 |
| Navigation | WS-E02, WS-E03, WS-E08, AX-02 |
| Taille nominale et minimale | WS-P01, WS-P02, WS-P03, WS-S07, WS-S08 |
| Zooms | ZM-01, WS-P15, CORE-05 |
| États autonomes, PANIC | NC-03…NC-07, NC-10, INV-04 |
| Interactions souris et clavier | NX-05, AX-03, NN-15, CORE-09 |
| Automation | NX-06, NX-12, CORE-13, NN-21 |
| Undo/Redo | UR-01, NN-21 |
| Redimensionnement pendant les gestes | RS-01, WS-P05 |
| Destruction de l'éditeur pendant un geste | ED-01, CORE-11 |
| Instances multiples | MI-01 |
| Synchronisation Neural/Table | NN-17…NN-21 |
| Absence d'écriture lors de la navigation | INV-03, NN-20, INV-13 |
| Non-régression DSP | INV-01, INV-13 |
| Compatibilité des sauvegardes | SV-01, INV-03 |
| Coût graphique | GC-01, NN-25, CORE-15, TEL-08 |
| Temps réel | RT-01, TEL-07 |
| Validation artistique du propriétaire | VAL-01 et critères SUBJ |

## 10. Recommandations Codex

| Recommandation | Objet | Critères |
|---|---|---|
| P1-01 | Crête dBFS fixe dans la barre | WS-P15, PL-05, CD-05 |
| P1-02 | Homonymie Z Master, Balance / CROSS / Cross Audio | NAM-07, NAM-05 |
| P1-03 | Freeze, Autonomous Armed, Autoriser, Capturer gels | NC-05, NC-04, NC-06 |
| P1-04 | PERFORMANCE compacte, Inspector rétractable | WS-P02, WS-P03, WS-I03, WS-I06 |
| P1-05 | Plancher de 9 px pour les repères critiques à 75 % | NX-11, ZM-01 (OD-06) |
| P1-06 | Enveloppe du Core avant la 3D | CORE-01…CORE-16 |
| P2-01 | Densité des réseaux, libellés répétés | NN-13, NN-22, NAM-06 |
| P2-02 | Trois traitements de matière | DS-03, DS-04 |
| P2-03 | Presets et structure de l'Inspector | WS-E04, WS-I01 |
| P2-04 | Libellés secondaires des enveloppes | NAM-03 (CSV), WS-S10 |
| P3-01 | Unités normalisées et microcopie | NAM-11, nomenclature §6 |

## 11. Critères d'entrée et de sortie de R2-B (synthèse)

Entrée : validation propriétaire du référentiel R2-A ; décisions OD-01 (stratégie d'identification des tests), OD-06
(plancher à 75 %), OD-11 (glyphes), OD-10 (périmètre PB-01) et OD-18 (rattachement de la nomenclature) ; liste des libellés validés (statut CSV passé à
« Validé »).

Sortie (toutes O d'abord, puis OBJ et SC documentés) : INV-01…13 ; NAM-01…13 ; DS-01, DS-02, DS-05 ; NX-01…10 ;
NC-01…10 ; CD-01…08 ; PL-01…06 ; AX-01, AX-03 ; UR-01, ED-01 (absence de crash), MI-01, SV-01, RT-01, ZM-01 pour les
composants ; GC-01 mesuré ; aucune régression des inventaires de tests (`tools/check_results.sh`) ; CI macOS
SUCCESS ; recette physique ciblée NC-05 / NC-07 / NC-03 ; validation subjective NAM-14, DS-03, CD-03.
