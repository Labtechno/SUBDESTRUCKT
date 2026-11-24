# SUBDESTRUCKT — Spike S0 — Causalité FM/CROSS
Date : 2026-11-24. Statut : **MISSION R&D À EXÉCUTER**.
Objectif : choisir le contrat causal interne du réseau FM/CROSS avant développement complet. Ce spike n'implémente pas SUBDESTRUCKT V1.

Références :
- [V1-SOFTWARE-ARCHITECTURE.md](V1-SOFTWARE-ARCHITECTURE.md)
- [V1-CORE-CONTRACTS-P0.md](V1-CORE-CONTRACTS-P0.md)
- [V1-DESIGN-BASELINE.md](V1-DESIGN-BASELINE.md)

## 1. Question à résoudre
Le moteur final autorise des interactions audio-rate et récursives entre :
- 4 opérateurs FM/PM;
- feedback individuel;
- modulation croisée entre opérateurs;
- WT → FM;
- OP1–4/FM Bus → WT;
- interactions bidirectionnelles FM ↔ WT.

Un graphe récursif naïf peut créer des dépendances instantanées impossibles à calculer explicitement par sample. S0 doit comparer deux familles de stratégies causales sans ajouter de fonctionnalité produit.

## 2. Stratégies à comparer
### S0-A — Retard systématique
Toute connexion de modulation/feedback considérée récursive utilise une valeur retardée selon une politique uniforme.
But : ordre d'évaluation simple, déterministe, robuste.

À mesurer :
- perte éventuelle de comportement “immédiat” dans les cascades acycliques;
- coloration liée au retard;
- stabilité;
- coût;
- facilité de changement de routage.

### S0-B — Retard ciblé sur cycles
Les sous-graphes acycliques sont évalués sans retard supplémentaire. Seules les arêtes nécessaires pour casser les cycles reçoivent un retard causal.
But : préserver les cascades directes lorsqu'elles sont calculables.

À mesurer :
- complexité de compilation du graphe;
- stabilité du choix d'arêtes retardées;
- comportement lors de l'activation/désactivation d'une route;
- ordre d'évaluation;
- reproductibilité;
- transitions lorsque la topologie change.

**Interdiction :** ne pas faire varier silencieusement la classification d'une route à chaque passage de profondeur autour de zéro. Une route structurellement présente garde une sémantique stable jusqu'à transaction de routage.

## 3. Références temporelles à comparer
Pour A et B, comparer au minimum :
### T1 — 1 sample au sample rate hôte
La durée de feedback varie avec le sample rate.

### T2 — durée physique fixe
Référence candidate : 1 / 48000 s = 20.833333 µs.
Utiliser si nécessaire un délai fractionnaire causal pour approcher cette durée aux différents sample rates.

Le délai fractionnaire ne doit jamais réintroduire une dépendance au sample courant.

## 4. Prototype minimal
Le spike doit rester volontairement petit :
- 1 oscillateur WT simplifié : sinus ou table triviale, sans Z, sans banques complexes;
- 4 opérateurs sinus;
- PM comme mode principal de test;
- feedback self;
- matrice interne 4×4;
- 1 route WT→FM par opérateur possible;
- OP1–4→WT;
- Cross Bus minimal facultatif uniquement si nécessaire aux tests;
- sortie mono ou stéréo triviale;
- aucun filtre;
- aucun Destruction Engine;
- aucune UI finale;
- aucun preset manager complet;
- aucune modulation globale;
- aucun asset system.

Ce prototype peut être un harness de test hors VST3. L'objectif est le comportement numérique, pas l'intégration DAW.

## 5. Configurations de test obligatoires
Au minimum :
1. cascade acyclique OP1→OP2→OP3→OP4;
2. self-feedback d'un opérateur;
3. cycle OP1↔OP2;
4. cycle OP1→OP2→OP3→OP1;
5. réseau dense 4×4;
6. WT→OP1 et OP1→WT;
7. WT↔OP1 avec feedback interne OP1;
8. WT↔plusieurs opérateurs;
9. changement transactionnel d'une topologie acyclique vers cyclique;
10. même topologie avec profondeurs 0, très faibles, nominales, extrêmes bornées.

## 6. Matrice de test
### Sample rates
- 44.1 kHz
- 48 kHz
- 88.2 kHz
- 96 kHz

### Bloc
Le rendu doit être comparé avec plusieurs tailles de bloc, au minimum :
- 16
- 32
- 64
- 128
- 256
- 512
- 1024

Le résultat ne doit pas dépendre de la taille de bloc, hors différences numériques bornées et expliquées.

### Qualité / clock
Pour ce spike, tester au minimum moteur interne :
- 1×;
- 2×;
- 4×;
si le harness permet de le faire sans masquer la comparaison causale.

La durée du feedback doit respecter le contrat testé indépendamment du facteur d'oversampling.

## 7. Stimuli
Utiliser des entrées déterministes :
- phase initiale fixe;
- fréquences simples : 55, 110, 220, 440, 997 Hz;
- ratios rationnels et irrationnels candidats;
- profondeurs PM faibles, moyennes et extrêmes bornées;
- changements de route à des samples connus.

Pas de random non seedé.

## 8. Mesures
Pour chaque stratégie :
### Déterminisme
- hash/rendu répété même configuration;
- variation selon block size;
- reset state;
- changement de topologie.

### Stabilité
- NaN/Inf;
- runaway non borné;
- peak max;
- DC;
- self-oscillation;
- temps avant divergence.

### Spectral
- FFT/PSD des cas de référence;
- aliasing relatif;
- différences entre SR;
- différences T1/T2;
- comparaison des cascades acycliques A vs B.

### Temporel
- phase effective;
- retard mesuré;
- continuité lors des changements structurels;
- clicks/discontinuities.

### Performance
Sur Apple M1 si disponible :
- temps CPU par sample/bloc;
- coût graphe sparse/dense;
- coût 1×/2×/4×;
- coût de compilation/rebuild hors RT;
- aucune allocation dans la boucle de rendu mesurée.

## 9. Invariants obligatoires
S0 doit tenter de démontrer :
- même rendu pour même état/seed/configuration dans le même environnement;
- indépendance raisonnable à la taille de bloc;
- aucune dépendance instantanée implicite;
- aucun NaN/Inf dans le domaine de test;
- changement de topologie effectué transactionnellement;
- aucune allocation ou verrou bloquant dans la boucle audio;
- activation d'une connexion ne change pas silencieusement la sémantique d'autres routes sans être décrit par la compilation du graphe;
- profondeur zéro n'entraîne pas à elle seule une reclassification topologique arbitraire.

## 10. Critères de sélection
La stratégie gagnante doit être évaluée selon cet ordre :
1. correction causale;
2. déterminisme;
3. stabilité;
4. comportement musical/spectral;
5. invariance sample rate / block size;
6. transitions;
7. coût CPU;
8. complexité d'implémentation et de maintenance.

Le CPU ne peut pas justifier une solution causalement incorrecte.

## 11. Résultats possibles
### GO-A
Retard systématique retenu.

### GO-B
Retard ciblé retenu.

### GO-A-T1 / GO-A-T2 / GO-B-T1 / GO-B-T2
La décision doit préciser aussi la référence temporelle retenue.

### NO-GO
Aucune approche ne satisfait les critères. Dans ce cas, documenter précisément l'échec et proposer un nouveau spike. Ne pas modifier le cahier produit sans arbitrage.

## 12. Livrables exigés de Claude.ai
1. code du harness isolé;
2. instructions de build/reproduction;
3. tests automatisés;
4. données brutes essentielles;
5. tableaux comparatifs A/B et T1/T2;
6. rendus audio de quelques cas représentatifs si utile;
7. mesures CPU M1 si environnement disponible;
8. rapport final avec recommandation;
9. liste des hypothèses et limites;
10. aucun merge dans une branche produit principale sans validation.

## 13. Règles de mission
- Ne pas implémenter l'UI finale.
- Ne pas ajouter d'algorithmes Z.
- Ne pas implémenter les filtres/destruction/presets complets.
- Ne pas “optimiser” en changeant le comportement testé.
- Ne pas supprimer des cas instables du rapport.
- Ne pas sélectionner une stratégie uniquement parce qu'elle est plus simple.
- Ne pas déclarer PASS sans résultats reproductibles.
- Toute modification du contrat produit doit revenir à l'arbitrage.

## 14. Prompt d'exécution Claude.ai
Tu interviens comme ingénieur DSP R&D sur SUBDESTRUCKT. Ta mission est exclusivement le spike S0 Causalité défini dans ce document.

Lis d'abord les documents de référence du dépôt. Ne développe pas SUBDESTRUCKT complet. Construis un harness DSP minimal permettant de comparer :
- stratégie A : retard systématique;
- stratégie B : retard ciblé pour casser uniquement les cycles;
et, pour chacune :
- T1 : retard de référence d'un sample hôte;
- T2 : durée physique de référence 20.833333 µs, avec délai fractionnaire causal si nécessaire.

Respecte la matrice de tests, les invariants et les livrables de ce document. Le rendu doit être déterministe, indépendant de la taille de bloc dans les tolérances documentées, sans allocation ni verrou bloquant dans le chemin audio. Les changements de topologie doivent être transactionnels.

À la fin, ne choisis pas selon ton intuition. Fournis les preuves comparatives, puis recommande exactement l'un de : GO-A-T1, GO-A-T2, GO-B-T1, GO-B-T2, ou NO-GO.

Ne merge rien vers une branche produit principale. Ne modifie aucune spécification fonctionnelle sans demander un arbitrage.
