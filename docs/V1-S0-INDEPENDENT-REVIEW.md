# SUBDESTRUCKT — Revue indépendante du spike S0
Date : 2026-11-24. Statut : **REVUE TECHNIQUE NON NORMATIVE — décisions finales encore à arbitrer**.
Source examinée : branche `spike/s0-causality`, rapport `docs/V1-S0-REPORT.md`, données, harness et tests. `main` n'est pas fusionné avec le spike.

## 1. Verdict de revue
### Stratégie B — GO technique
La recommandation **B (retard ciblé uniquement là où la causalité l'exige)** est suffisamment étayée pour devenir la direction privilégiée :
- causalité statique et dynamique démontrée sur le harness;
- déterminisme et block-size invariance bit-exacts dans la matrice testée;
- routes acycliques réellement immédiates;
- A modifie inutilement les cascades calculables sans délai;
- B préserve mieux la sémantique d'une chaîne feed-forward.

La revue ne voit aucune raison technique forte de conserver A comme candidat principal.

### Référence T2 — PROVISOIRE, pas encore gelée
T2 a une justification conceptuelle forte : durée physique de feedback indépendante du sample rate. Mais les preuves actuelles ne montrent pas qu'il améliore réellement le comportement spectral/perceptif global face à T1.
Limites :
- PM sinus seulement, donc stabilité intrinsèquement bornée;
- pas de FM linéaire/TZ/exponentielle;
- pas de vraie wavetable;
- pas de rééchantillonnage temps réel;
- pas de benchmark M1;
- 44.1 kHz / 1× ne peut pas représenter 20.833 µs causalement;
- interpolation Lagrange introduit une réponse non parfaitement unitaire (jusqu'à +0.31 dB à 20 kHz dans les cas mesurés).

**Conclusion : retenir B maintenant, garder T2 comme candidat préféré, mais exiger S0.1 avant de figer B-T2.**

## 2. Point critique : priorité des routes
Le rapport propose de persister l'ordre de création/priorité. C'est nécessaire pour reproduire la classification B, mais insuffisant ergonomiquement :
- deux topologies visuellement identiques peuvent sonner différemment selon leur historique de création;
- supprimer une route peut reclasser une route survivante de retardée à immédiate et modifier le son;
- une simple rampe sur la route supprimée ne neutralise pas le changement de classe d'une autre route.

### Recommandation plus robuste : classification causale “sticky”
Pour toute transaction :
1. les routes survivantes conservent leur classe Immediate/Delayed;
2. ajouter une route : tester si elle peut être Immediate sans créer de cycle; sinon Delayed;
3. supprimer une route : ne jamais “optimiser” automatiquement les routes survivantes;
4. modifier source/destination = suppression + ajout;
5. sérialiser StableRouteID + CausalClass (et éventuellement CausalPriority pour audit/migration);
6. valider à la lecture que le sous-graphe Immediate est acyclique.

Avantage : l'ajout **et le retrait** ne changent jamais silencieusement le son d'une route survivante.

Le graphe peut conserver un retard qui n'est plus mathématiquement nécessaire après suppression. C'est volontaire : stabilité sonore > minimisation automatique du nombre de délais.

## 3. T2 à 44.1 kHz / 1×
Trois options :
- accepter le plancher 1 sample = 22.676 µs (+8.8 %);
- imposer K≥2 dès qu'un retard causal T2 est actif à 44.1 kHz;
- abandonner T2 pour T1.

### Recommandation
**Imposer K≥2 pour le sous-système récursif FM/CROSS à 44.1 kHz si T2 est finalement retenu.**
Ne pas dégrader silencieusement T2 à 22.676 µs : cela détruirait précisément l'invariant physique qui justifie T2.

Cette décision doit être conditionnée à un benchmark M1. À 44.1 kHz, 2× donne 88.2 kHz moteur, mais le coût du réseau complet reste à mesurer.

## 4. Scope de T2
Si T2 est retenu, il doit s'appliquer à **toutes les arêtes causalement retardées des réseaux récursifs spécialisés FM/CROSS** :
- self-feedback opérateur;
- cycles opérateur↔opérateur;
- arêtes de cycle WT↔FM;
- Cross Bus s'il participe à un cycle.

Il ne s'applique pas :
- aux routes feed-forward immédiates;
- aux delays créatifs;
- aux combs/résonateurs;
- aux compensations de latence;
- aux resamplers.

La règle doit être “retard causal du réseau récursif”, pas “uniquement le knob Feedback”.

## 5. Transitions topologiques
Le rapport démontre qu'un changement sec est audible et qu'une rampe est nécessaire. Mais **256 samples n'est pas validé comme valeur finale** : certains max restent nettement au-dessus du bruit naturel.

S0.1 doit comparer au minimum :
- 64 / 128 / 256 / 512 / 1024 samples;
- rampe linéaire vs courbe smoothstep/équivalente;
- ajout, retrait, changement source/destination;
- avec classification sticky afin d'éliminer la reclassification des routes survivantes.

Objectif : choisir une politique de transition mesurée, pas figer “256” parce que c'est ce que le harness a utilisé.

## 6. Délai fractionnaire
Le Lagrange cubique utilisé dans S0 est causal dans le harness et suffisamment précis pour le spike, mais ne doit pas être considéré automatiquement comme implémentation produit.

À tester dans S0.1 si T2 reste candidat :
- réponse amplitude/phase sur le vrai domaine de modulation;
- comportement dans feedback/cycles à profondeur extrême;
- alternatives plus simples ou à magnitude mieux contrôlée si nécessaire;
- coût M1;
- interaction avec oversampling et vrai resampling.

## 7. Extension S0.1 obligatoire avant vertical slice
Ajouter au harness :
1. PM actuelle comme référence;
2. Linear FM;
3. TZFM;
4. Exponential FM avec bornes du cahier;
5. vraie wavetable minimale (au moins quelques formes non sinusoïdales band-limited ou table fixe);
6. vrai chemin d'oversampling/resampling temps réel;
7. classification causale sticky;
8. sweep de rampes topologiques;
9. benchmark Apple M1;
10. comparaison B-T1 vs B-T2 uniquement — A peut rester témoin mais n'a plus besoin d'une campagne exhaustive.

## 8. Conditions GO finales
### GO-B
Peut être considéré **acquis techniquement** sous réserve de validation produit de la classification sticky/persistante.

### GO-B-T2
Ne doit être prononcé qu'après S0.1 si :
- les modes FM supplémentaires restent stables dans les bornes;
- T2 ne crée pas de coût disproportionné sur M1;
- l'interpolation n'introduit pas de défaut musical problématique;
- K≥2 à 44.1 kHz est soutenable;
- la politique de transition est propre.

### Repli
Si T2 n'apporte pas assez de bénéfice ou coûte trop cher, **GO-B-T1** reste une solution techniquement saine et beaucoup plus simple.

## 9. État Git
La branche `spike/s0-causality` est à un commit devant son merge-base mais derrière les derniers commits de `main`. Ne pas la fusionner telle quelle. Conserver le spike isolé, exécuter S0.1 sur cette branche ou une branche dérivée, puis seulement réconcilier avec `main` après décision.

## 10. Décisions produit à valider
Recommandations de la direction technique :
1. **B** — causalité ciblée sur cycles : valider.
2. **Sticky causal class** — conserver la classe des routes survivantes et la sérialiser : valider.
3. **T2 44.1 kHz** — si T2 retenu, imposer K≥2 au réseau récursif : valider provisoirement sous benchmark.
4. **Scope T2** — toutes les arêtes causalement retardées FM/CROSS, et seulement elles : valider.
5. **T2 lui-même** — ne pas geler avant S0.1.
