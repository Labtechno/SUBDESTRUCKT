# ARCH-UI-04-R2-A — risques et décisions ouvertes

## 1. Risques

Échelle qualitative (aucune probabilité mesurée) : **F** faible, **M** moyen, **É** élevé.

| ID | Risque | Probabilité | Impact | Atténuation | Lot |
|---|---|---|---|---|---|
| RK-01 | Les tests historiques repèrent des commandes par leur texte visible (AUT-08, TA-07w, UI04-OPS) ou par la structure d'onglets (UI-01w). Le passage à l'anglais et le regroupement ENGINEERING les font échouer sans régression fonctionnelle. | É | É | OD-01 : identification stable, décidée avant R2-B. Aucun critère modifié pour obtenir un PASS. | B, E |
| RK-02 | Les largeurs du CSV sont estimées (médiane 0,525 × corps). Le maximum observé atteint 0,69 × corps : des libellés riches en M ou W peuvent dépasser. | M | M | PT-02 : mesure JUCE des 2 213 libellés avant adoption | B |
| RK-03 | Abréviations peu familières (XAUDIO, INHARM, PH MODE) | M | M | Nom complet au focus, dans l'infobulle et dans l'Inspector ; validation subjective NAM-14 | B |
| RK-04 | La disposition COMPACT ne tient pas à 960 × 640 avec un pad ≥ 300 px (l'estimation de 312 px laisse peu de marge) | M | É | PT-01. Repli : ligne numérique plus fine, rangée de macros en PLASMA H, disposition CONSTRAINED. | C |
| RK-05 | Changement de disposition pendant une interaction : geste perdu, accès après libération. ARCH-UI-04 a déjà corrigé des accès après libération dus au reparentage. | M | É | WS-P05, WS-S02 : différer après le geste. Composants persistants. ASan en local. | C, D, E |
| RK-06 | **Interaction avec PB-01** : toute refonte des gestes NEXUS (double-clic, saisie, défaut) peut produire ou masquer des gestes hôte imbriqués | M | M | NX-05/NX-06 comptent les `begin`/`end`. Correction de PB-01 seulement sous OD-10. | B |
| RK-07 | **Interaction avec PB-02** : l'enveloppe du Core (CORE-11), les éditeurs d'enveloppe (R2-D) et les changements de disposition qui détruisent ou masquent un composant en cours de glisser laissent un geste hôte ouvert | M | M | CORE-11 conditionné à OD-10. Pas de destruction pendant le geste (WS-P05). | C, D |
| RK-08 | Coût de rendu des réseaux et de la réactivité sur le thread de messages de l'hôte (Reason) : latence de l'interface | M | M | PT-03, PT-04. Repli sans animation des liaisons. | F, G |
| RK-09 | La vue NEURAL est lue comme un patcher libre | M | M | Pas de port pour les paires inexistantes (NN-03). Liaisons fantômes seulement pour les emplacements autorisés. | F |
| RK-10 | Lecture erronée de l'activité : profondeur effective prise pour un niveau de signal | M | M | NN-02 : distinction visuelle ; mention ESTIMATE | F, G |
| RK-11 | Pression pour publier des données C/D (activité par route, étapes d'enveloppe) au détriment du DSP FINAL LOCK | M | É | TEL-03 : mission distincte et dérogation explicite | G, 05 |
| RK-12 | Écart entre le libellé court (CROSS LVL) et le nom hôte dans les pistes d'automation (CROSS Master) | É | F | NAM-04 : le nom hôte apparaît dans l'Inspector et l'infobulle ; documentation dans le manuel | B |
| RK-13 | Glyphes `→ · … −` absents de la police sur une plateforme | F | M | PT-05, repli ASCII (OD-11) | B |
| RK-14 | Lisibilité à 75 % : 7,125 px écran au plancher actuel | É | M | OD-06 | B |
| RK-15 | Les contrats historiques restent contradictoires si les remplacements du master §7 ne sont pas consignés | M | M | Report des remplacements à chaque lot, puis consolidation en ARCH-UI-06 | B–06 |
| RK-16 | Couverture physique limitée : une machine, Reason 14 seul (Ableton non certifié, M3 non certifié) | — | M | Maintenu. Ne pas extrapoler. | H |
| RK-17 | Spectre contaminé par repliement, y compris dans la bande affichée 20 Hz – fs/4 (décimation sans filtre anti-repliement dans le moteur verrouillé ; correction C03) : spectre indicatif seulement, trompeur en IMMERSIVE | M | F | Bande affichée (CD-06). Aucune correction sans dérogation DSP. | B, G |
| RK-18 | Le DRAWER de l'Inspector masque le moniteur pendant l'édition au minimum | É | F | Rang « — » assumé, avec indicateur (WS-P04) | E |
| RK-19 | Une modification de l'interface ou du wrapper affecte l'audio sans toucher `source/` : écriture de paramètre pendant la navigation, capture, autorisation, chargement d'état | F | É | INV-13 : REG-01, suite TA, test d'absence d'écriture | tous |
| RK-20 | Le regroupement ENGINEERING allonge les parcours (jusqu'à +1 clic) | M | F | WS-E08 ; mémorisation du dernier outil ; liens internes directs | E |

## 2. Décisions ouvertes

| ID | Question | Options | Recommandation R2-A | Bloque |
|---|---|---|---|---|
| OD-01 | Comment les tests repèrent-ils les commandes après le passage à l'anglais et le regroupement ? | (a) Texte anglais en dur dans les tests. (b) Identifiant de composant stable (`setComponentID`), indépendant du texte. (c) Titre accessible technique. | **(b)** : découple tests et sérigraphie. Les critères des tests restent identiques, seul le repérage change. Autorisation explicite requise. | R2-B, R2-E |
| OD-02 | Validation des libellés du CSV | Valider en bloc, par famille ou par priorité (P1 d'abord) | Valider P1 (323) avant R2-B, puis P2 et P3 en cours de lot. Confirmer l'écart OUT GAIN. | R2-B |
| OD-03 | Langue du manuel utilisateur | Français (projet), anglais (interface), bilingue | Anglais pour les noms de commande ; le reste du texte est à décider | ARCH-UI-06 |
| OD-04 | Seuils de disposition PERFORMANCE | Valeurs du §2.4 de la spécification des espaces, ou valeurs issues de PT-01 | Fixer après PT-01, avant les mesures de recette | R2-C |
| OD-05 | Modes de l'Inspector et seuil d'ancrage | DOCKED seul (0.4.1) ; DOCKED + DRAWER ; DOCKED + BAND + DRAWER | DOCKED + DRAWER en PERFORMANCE, DOCKED + BAND ailleurs ; seuil d'ancrage : canvas ≥ 1 080 px | R2-E (et WS-P03) |
| OD-06 | Plancher typographique écran à 75 % | Aucun (0.4.1) ; 9 px pour les repères critiques (Codex) ; 9 px pour tous les libellés | 9 px pour les repères critiques | R2-B |
| OD-07 | Modes OFF / REACTIVE / IMMERSIVE et usage des estimations | Pas de modes ; deux modes (OFF, REACTIVE) ; trois modes ; estimations autorisées ou non | OFF et REACTIVE seulement. ESTIMATE réservé à IMMERSIVE, si IMMERSIVE est retenu. | R2-G |
| OD-08 | Libellés contextuels FM INDEX / FM FDBK en PERFORMANCE (ARCH-UI-03 §8, non arbitrés) | Conserver ; les remplacer par la route explicite (OP2→OP1) | Conserver, route explicite au focus | R2-C |
| OD-09 | Indicateur « automation en cours » | Absent (donnée non fiable en VST3) ; heuristique sur les changements sans geste | Absent (INV-11) | — |
| OD-10 | Périmètre de correction de PB-01 et PB-02 | Rester au backlog ; corriger dans R2-B (PB-01) et R2-C/R2-D (PB-02) | Les corriger dans les lots qui modifient ces gestes, avec tests de reproduction préalables (méthode R1) | R2-B, R2-C, R2-D |
| OD-11 | Glyphes typographiques | Unicode (`→ · … −`) ; ASCII (`> . ... -`) | Unicode si PT-05 confirme, sinon ASCII | R2-B |
| OD-12 | Technologie de rendu d'ARCH-UI-05 | Rendu logiciel JUCE ; module `juce_opengl` (déjà dans JUCE, sans nouvelle dépendance) | À décider en ouverture d'ARCH-UI-05 | ARCH-UI-05 |
| OD-13 | Ordre R2-D / R2-E | D puis E ; E puis D | E puis D si WS-P03 est prioritaire | Planification |
| OD-14 | Consignation du remplacement DV-01 (vue réseau éditable, interdiction du patcher libre maintenue) | Consigner en R2-F ; consigner dès R2-B | Consigner dès validation de R2-A, pour éviter une contradiction documentaire | R2-F |
| OD-15 | Mission d'extension de télémétrie (données C : étapes d'enveloppe, phase des LFO, exécution autonome par étage) | Aucune ; mission distincte après R2-G | Aucune avant la fin de R2-H | — |
| OD-16 | Restriction d'ordre inter-domaines des transformations Z (héritée, non validée) | — | Reste ouverte. Représentation fidèle exigée (WS-S03, CD-08). | — |
| OD-17 | Numérotation des versions R2 | 0.5.x par lot ; 0.5.0 à la fin de R2-H | 0.5.x par lot livré | R2-B |
| OD-18 | Rattachement de l'application de la nomenclature (le brief définit R2-B comme « composants et tokens graphiques ») | Dans R2-B ; sous-lot R2-B2 ; dans chaque lot d'espace | Dans R2-B, car la sérigraphie fait partie du design system et les tests en dépendent | R2-B |
| OD-19 | Sous-lots de R2-F (brief : « prévoir des sous-lots si risques significatifs ») | Un lot ; R2-F1 (FM, CROSS) puis R2-F2 (matrice) | Deux sous-lots : les risques de la matrice configurable diffèrent de ceux des réseaux fixes | R2-F |

## 2 bis. Décisions arbitrées par le propriétaire (R2-B1, 2026-11-28)

| ID | Option retenue | Décision | Précision |
|---|---|---|---|
| OD-01 | B | Identifiant de composant stable | Stratégie : `B1-COMPONENT-LAB/INTERACTION-TEST-PLAN.md` §1 |
| OD-02 | B | Validation progressive des libellés par priorité | Revue P1 : `B1-COMPONENT-LAB/P1-NAMING-REVIEW.md` ; aucun libellé validé à ce jour |
| OD-06 | B | 9 px minimum à 75 % pour les repères critiques | Soit 12 px logiques ; vérifié 26/26 (macOS et Windows) |
| OD-10 | B | Correction de PB-01 et PB-02 dans les lots concernés | Jamais sans reproduction préalable ; protocoles : plan de test §4 et §5 |
| OD-11 | B | Unicode vérifié avec repli ASCII | `B1-COMPONENT-LAB/GLYPH-VALIDATION.md` |
| OD-17 | A | Version 0.5.x par lot livré | Pour les livraisons fonctionnelles seulement ; le Component Lab n'est pas une version ; première livraison intégrée R2-B : 0.5.0 |
| OD-18 | A | Nomenclature intégrée à R2-B | Application en R2-B2 |

Restent ouvertes : OD-03, OD-04, OD-05, OD-07, OD-08, OD-09, OD-12 à OD-16 et OD-19.

## 3. Réserves maintenues (non modifiées par R2)

R-29 et R-30 NON EXÉCUTÉS (risques acceptés) ; PANIC et sauvegarde pendant un export NON EXÉCUTÉS physiquement ;
Ableton Live non certifié ; limites d'API VST3 (`../V1-AUTONOMY-CONTRACT.md` §5) ; les dix entrées de `tests/known_failures.txt` (huit échecs documentés et deux critères remplacés, ZC-15 et TA-15) ;
licence JUCE (B-04) non résolue ; PB-01 et PB-02 au backlog.
