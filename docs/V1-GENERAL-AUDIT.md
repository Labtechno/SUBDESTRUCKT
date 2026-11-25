# SUBDESTRUCKT — Audit transversal du cahier des charges V1
Date : 2026-11-24. Statut : audit de cohérence après clôture des arbitrages Preset/State et UI/UX. Aucun code développé. Aucun critère DSP déclaré PASS.

## 1. Verdict exécutif
**GO pour l'architecture logicielle et les spikes R&D. NO-GO pour le développement intégral du produit.**

Le périmètre produit est maintenant suffisamment défini pour concevoir les contrats C++/DSP, le registre de paramètres et les prototypes expérimentaux. Il n'est pas encore suffisamment fermé pour implémenter proprement le synthétiseur complet. Les blocages restants ne sont pas des demandes de fonctionnalités supplémentaires : ce sont des contrats fondamentaux qui conditionnent la causalité, le rappel sonore, la stabilité temps réel et la compatibilité DAW/hardware.

La V1 est cohérente artistiquement et techniquement plausible, mais très ambitieuse pour un premier cycle : WT XYZ + 4 opérateurs FM/PM + réseau Cross récursif + 3 filtres + 6 processeurs de destruction + 64 modulations globales + captures + randomisation + trois niveaux UI. La réponse recommandée n'est pas de supprimer arbitrairement des fonctions déjà validées, mais d'imposer un développement par tranches verticales avec gates GO/NO-GO.

## 2. Santé du cahier des charges
| Domaine | État | Audit |
|---|---|---|
| Vision / identité sonore | VERT | Cohérente et différenciante |
| Wavetable XYZ | ORANGE | Architecture claire, catalogue Z incomplet |
| FM/PM | ORANGE | Contrat riche, B-T2 retenu; intégration à valider |
| Cross Engine | ORANGE | Identité forte, B-T2 retenu; intégration à valider |
| Filtres / Destruction | VERT-ORANGE | Catalogues cohérents, algorithmes candidats non prouvés |
| Global Modulation | VERT-ORANGE | Contrat solide, budget audio-rate non mesuré |
| Preset / State | VERT | Modèle logique cohérent; format physique et budgets ouverts |
| UI/UX | VERT | Architecture fonctionnelle clôturée; métriques à tester |
| Temps réel / latence | ORANGE | Contrat P0 fermé; budgets et résultats de benchmark encore ouverts |
| MIDI / cycle de voix | VERT | Contrat P0 fermé; implémentation et tests à venir |
| Recette / performance | ORANGE | Critères nombreux, budgets chiffrés manquants |

## 3. Blocages P0 avant implémentation complète

### AUD-P0-01 — Catalogue Z exact et contrat de chaque transformation
Le contrat P0 confirme huit transformations Z visées, deux slots en série avec ordre A→B ou B→A. Harmonic Fold, Phase Warp et Spectral Mutation sont verrouillées; cinq transformations historiques restent introuvables et ne doivent pas être inventées.
À fermer : noms exacts, domaine d'entrée/sortie, neutralité à zéro, paramètres, préservation fondamentale, compensation de gain, compatibilité audio-rate, comportement DC/Nyquist, ordre de composition et coûts/caches.
**Gate :** aucune implémentation définitive du WT Engine avant réconciliation du catalogue d'origine.

### AUD-P0-02 — Causalité FM/Cross — CHOIX CLOS / RECETTE OUVERTE
**Stratégie B validée** : retard ciblé uniquement sur les arêtes nécessaires pour casser un cycle. **Classification sticky validée** : les routes survivantes conservent leur classe Immediate/Delayed et cette classe est persistée.
B-T2 retenu; restent ouverts la recette Thiran1, les transitions sûres et la performance physique M3.
K≥2 permanent à 44.1 kHz; T2 sur toutes les arêtes Delayed FM/CROSS et seulement elles.
**Gate : ARCH-G3 validé dans la portée testée.**

### AUD-P0-03 — Cycle de voix / Amp / Note Off — FERMÉ CONTRACTUELLEMENT
Fermé par [V1-CORE-CONTRACTS-P0.md](V1-CORE-CONTRACTS-P0.md) : Amp avant les générateurs de tails, Note Off sans reset des états DSP, Tail guidé par énergie, états autonomes armés explicitement, Drone via Gate normal, Panic avec rampe anti-click puis purge. Les seuils d'énergie et garde-fous temporels sont des valeurs d'implémentation à mesurer, pas des arbitrages produit.

### AUD-P0-04 — Contrat MIDI et expressivité — FERMÉ CONTRACTUELLEMENT
Channel Pressure global validé; MPE exclu V1; Pitch Bend Up/Down par preset, défaut ±2 demi-tons; événements sample-accurate; ordre déterministe au même sample; MIDI Learn global/utilisateur séparé des presets.

### AUD-P0-05 — Latence, qualité et oversampling — FERMÉ CONTRACTUELLEMENT
Quality = préférence d'instance/projet; Clean/Raw = paramètre artistique du preset; oversampling = politique interne par moteur/algorithme. Les changements de Quality sont préparés hors RT avec transition contrôlée. La latence VST3 suit la configuration active avec alignement interne. 44.1/48/88.2/96 kHz sont les rates officiellement testés; 192 kHz n'est pas garanti. **Reste à mesurer :** facteurs réels, latence maximale, coût CPU et stratégies de resampling.

### AUD-P0-06 — Protection de sortie et headroom — FERMÉ CONTRACTUELLEMENT
Headroom nominal cible ~-12 dBFS; protection denormals et garde DC transparents là où nécessaire; >0 dBFS autorisé en float avec alerte/mètre; le garde-fou obligatoire ne traite que les états numériques dangereux; limiteur musical optionnel et distinct. Les seuils exacts restent à tester.

### AUD-P0-07 — Registre canonique des paramètres — FERMÉ AU NIVEAU DU SCHÉMA
StableID explicites/immuables; host automatise Base Value; Effective Value calculée; paramètres structurels événementiels/transactionnels; métadonnées complètes obligatoires; routes incompatibles conservées inactives avec raison. **Gate maintenu :** produire le registre concret avant l'implémentation définitive des moteurs.

## 4. Incohérences / ambiguïtés à corriger

### AUD-A01 — Baseline UI devenue obsolète
La section 12 de V1-DESIGN-BASELINE décrit encore l'UI comme une proposition non validée alors que V1-UI-UX-DESIGN clôture les arbitrages. Le référentiel général doit pointer vers le document UI validé.

### AUD-A02 — “119 connexions” est une capacité logique, pas un budget de calcul
16 FM + 39 Cross + 64 Global = 119 est utile comme métrique de configuration, mais trompeuse comme métrique CPU. Les FM/Cross peuvent être au domaine audio interne, les globales ont plusieurs rates et certaines destinations sont beaucoup plus coûteuses.
**Correction :** budgets par classe de destination et domaine temporel, pas uniquement nombre de routes.

### AUD-A03 — Destinations Z/Character audio-rate “conditionnelles”
Le registre global et Cross les expose, mais la compatibilité dépend de l'algorithme Z/Character actif.
**Correction :** capacité déclarée par paramètre/algorithme; connexion incompatible conservée inactive avec raison explicite, jamais downgradée silencieusement.

### AUD-A04 — Digital Character vs Clean/Raw vs Quality
Ces trois concepts peuvent se chevaucher perceptuellement.
**Contrat recommandé :**
- Character = transformation artistique paramétrique du moteur.
- Clean/Raw = politique artistique d'aliasing/lecture lorsqu'elle existe.
- Quality = budget de calcul non créatif, qui doit préserver autant que possible le preset.
Les presets doivent stocker Character/Raw-Clean; la politique de stockage de Quality doit être explicitement décidée par contexte (preset, projet ou préférence).

### AUD-A05 — WT/FM balance
La façade Performance prévoit une balance WT/FM, tandis que les moteurs ont des niveaux audio indépendants et Cross Audio Mix/Crossfade.
**À définir :** la balance doit piloter une fonction existante bien précise et ne pas créer un quatrième mixage implicite. Recommandation : macro de performance dérivée des gains moteurs dans un domaine puissance constante, indépendante du mode Cross Audio.

### AUD-A06 — Deux contrôles FM contextuels en Performance
La décision UI est valide uniquement si ces commandes exposent des mécanismes existants. “FM INDEX” et “FM FEEDBACK” ne doivent pas devenir des sommateurs cachés supplémentaires.
**À définir au prototype :** quelles valeurs existantes sont visées, règles de neutralité, comportement avec plusieurs routes et automation.

### AUD-A07 — Evolve et sauvegarde du “son courant”
Evolve est une couche temporaire non persistante avant Commit. Cela protège le preset, mais peut surprendre si l'utilisateur fait Save alors que le son entendu contient une excursion Evolve.
**À fermer UX/State :** Save doit avertir ou rendre explicite que l'excursion n'est pas intégrée; “Commit & Save” peut être un geste séparé. Ne jamais committer implicitement.

### AUD-A08 — RNG reproductible dans le VSTi
Seed + Stream ID + version + compteur est correct, mais la reproductibilité peut diverger si les distributions utilisent des implémentations standard différentes ou des flottants non spécifiés.
**Correction :** choisir aussi la conversion bits→distribution et la précision numérique canonique; tester golden vectors RNG.

### AUD-A09 — Freeze/Glitch et taille des états DAW
Les captures créatives peuvent grossir fortement les states de plugin avec plusieurs instances.
**À mesurer :** tailles réelles, compression, limites/pratiques des hôtes, temps de sauvegarde/rappel. Ne pas supposer que la bibliothèque dédupliquée déduplique les états entre instances du DAW.

### AUD-A10 — Hardware — périmètre différé
Décision supersédante du 2026-11-24 : **aucun développement hardware n'est traité dans la phase actuelle**. Les anciennes exigences de parité matérielle ne doivent plus influencer le périmètre, les performances minimales, l'UI, le format de preset ni l'architecture de release du VSTi. Le sujet pourra être rouvert ultérieurement comme projet séparé.

### AUD-A11 — Plateforme desktop V1 resserrée
Décision produit du 2026-11-24, révisée le 2026-11-25 : macOS Apple Silicon natif uniquement, **M3 minimum** (ancien minimum : M1). Windows est exclu. Linux n'est pas engagé dans la V1 et ne doit pas augmenter la matrice de CI, de QA ou d'architecture sans décision ultérieure explicite. Intel Mac et Rosetta ne constituent pas des cibles supportées.
**Impact positif :** matrice de tests fortement réduite, possibilité d'optimiser et benchmarker sur une famille CPU cohérente, moindre dette de packaging/support. **Risque :** marché adressable plus petit; garder néanmoins une séparation saine entre DSP, host et UI afin de préserver la testabilité et la maintenabilité du VSTi.

## 5. Points solides à conserver sans réouverture
1. Deux moteurs WT XYZ et FM/PM, pas de troisième moteur.
2. Cross bidirectionnel comme signature centrale.
3. Séparation audio / modulation / événements Sync.
4. Cross modulation pré-FX et Cross Audio post-chaînes locales alignées.
5. Pas de routage audio arbitraire ni patcher global.
6. Global Mod Matrix acyclique; feedback intentionnel limité aux systèmes spécialisés.
7. 64 connexions globales, affectations macros incluses.
8. Via unique multiplicatif unipolaire.
9. Presets R1 configuration + R2 Creative Recall, pas de continuation DSP intégrale R3.
10. Assets immuables Copy-on-Write, export portable et états DAW autonomes.
11. Randomize / Mutate permanents, Evolve temporaire puis Commit.
12. Undo transactionnel incluant RNG; Redo sans nouveau tirage.
13. Trois niveaux UI partageant le même état.
14. Destination Inspector intégré; Overview non patcher.
15. P0/P1 sont des lots d'une seule V1.
16. Aucun ajout de DSP dicté uniquement par l'interface.

## 6. Risque de scope
Le projet est **sur-dimensionné pour une implémentation monolithique**, mais pas incohérent. La stratégie recommandée :
- Ne pas réduire maintenant le cahier validé.
- Construire d'abord une **vertical slice** minimale : MIDI mono → WT simple + 2 opérateurs FM → Cross causal → Amp → sortie → preset minimal → UI de diagnostic.
- Ajouter ensuite les capacités par lots validés avec non-régression.
- Chaque lot doit pouvoir être refusé ou retravaillé sans invalider le noyau.
- Les algorithmes P1 ne doivent jamais bloquer la validation du noyau P0.

## 7. Ordre recommandé après audit
### Gate A — Fermeture documentaire ciblée — MAJORITAIREMENT FERMÉE
AUD-P0-03/04/05/06/07 sont fermés par le Core Contract P0. AUD-P0-01 reste partiel uniquement pour les cinq transformations Z historiques manquantes. Ne pas inventer ces cinq algorithmes.
### Gate B — Architecture logicielle — FORMALISÉE
[V1-SOFTWARE-ARCHITECTURE.md](V1-SOFTWARE-ARCHITECTURE.md) formalise Core DSP indépendant du wrapper, Voice/Engine state, Parameter Registry, Event Timeline, Modulation Graph, Asset/State, ownership RT/non-RT et télémétrie. Le noyau de la vertical slice 0.1.0-b5 est validé dans sa portée; moteurs complets non validés.
### Gate C — Spike S0 causalité — CHOIX CLOS
S0/S0.1 : B-T2 retenu; NO-GO historique du benchmark M1 conservé. Recette intégration de la tranche acceptée; M3 physique et expansion du moteur encore ouverts.
### Gate D — Spikes ciblés
S1 Wavetable/XY/Z, S2 FM/PM, S3 ADAA/wavefolder, S4 Mod Matrix, S5 State/RNG, S6 performance cumulée, S7 prototype ergonomique.
### Gate E — Architecture gelée
Seulement après mesures : contrats finaux, budgets CPU/RAM/latence et plan de développement VSTi par lots.

## 8. Décision GO/NO-GO
- **GO :** audit, fermeture des contrats manquants, architecture C++ et prototypes R&D isolés.
- **NO-GO :** développement intégral de SUBDESTRUCKT, UI graphique finale, optimisation prématurée .

## 9. Prochaine mission recommandée
**Préparer ARCH-G4 par lots**, en commençant par le cadrage S1 Wavetable/XY/Z, sans inventer les cinq transformations Z manquantes. Voir [V1-ARCH-G3-CLOSURE.md](V1-ARCH-G3-CLOSURE.md).


### Révision plateforme — décision utilisateur du 2026-11-25
Apple M3 de base devient la configuration matérielle minimale cible, en remplacement du M1. M1 et M2 (y compris leurs variantes) sortent du support officiel, sans obligation de blocage logiciel. M4 et générations ultérieures constituent la classe recommandée, sous réserve de recette; le Mac Studio M4 Max de l'utilisateur est la machine principale de développement et de recette physique. La validation sur M3 physique en hôte VST3 est obligatoire avant Beta publique. Aucun résultat de performance M3 n'est encore disponible. Le benchmark M1 virtualisé S0.1 (run 45591261853, rapport sur la branche expérimentale au commit a85f1947e48b1fd8d5d11a9f27df09b78437e5af) et son verdict NO-GO selon les seuils historiques restent inchangés. Ils ne certifient ni M1 ni M3. Cette révision ne gèle pas T2, ne change pas les seuils historiques, ne clôt pas les autres conditions de causalité et n'autorise pas à elle seule ARCH-G3.


## Décision de clôture S0.1 — 2026-11-25
B-T2 est retenu comme contrat produit, avec minimum permanent K=2 à 44,1 kHz. ARCH-G3 est autorisé sous conditions de recette d'intégration, sans certification de performance. Voir [V1-CAUSAL-CONTRACT.md](V1-CAUSAL-CONTRACT.md) et [V1-ARCH-G3-MISSION.md](V1-ARCH-G3-MISSION.md). Le NO-GO historique du benchmark M1 reste inchangé; M3 physique avant Beta publique. Les formulations antérieures conditionnant le choix à S0.1 ou au M1 sont historiques et remplacées pour les travaux futurs par cette décision. Aucun GO ARCH-G4 ni release publique.


## Clôture ARCH-G3 — recette physique du 25 novembre 2026
GO ARCH-G3 dans la portée de la tranche 0.1.0-b5, 15/15 PASS déclarés et acceptés par la direction technique. R-08 et R-09 complétés par confirmation explicite : 44,1/48/88,2/96 kHz et PDC à 44,1 kHz. Voir [V1-ARCH-G3-CLOSURE.md](V1-ARCH-G3-CLOSURE.md). Les restrictions antérieures « avant recette » sont levées pour le prototype validé; la préparation d'ARCH-G4 par lots est ouverte, pas l'implémentation monolithique ni la diffusion publique. M3 physique, budget CPU produit, licence JUCE, signature/notarisation et dettes d'intégration restent ouverts. Aucun merge du code ni nouvelle release par cette décision.
