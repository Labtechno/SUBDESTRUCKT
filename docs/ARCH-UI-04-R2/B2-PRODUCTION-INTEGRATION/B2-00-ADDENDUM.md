# ARCH-UI-04-R2-B2-00 — addendum normatif de clôture

Date : 2026-11-28. Référence : dossier [B2-00-DOSSIER.md](B2-00-DOSSIER.md) publié en `e45ba7d`.

Ce document **complète et prime** le dossier R2-B2-00 sur les points qu'il traite. Le dossier, la matrice, les critères,
le protocole et le plan de tests restent en vigueur pour le reste.

## 1. Décision du propriétaire

À la suite de la contre-expertise de ChatGPT, le propriétaire a validé :

- les **sept arbitrages DB2-01 à DB2-07** ;
- les **cinq amendements CE-B2-01 à CE-B2-05**.

| Élément | État |
|---|---|
| AR-15 | PASS (inchangé) |
| ARCH-UI-04-R2-B1-R1 | GO FINAL |
| Design System R1 | GELÉ |
| **ARCH-UI-04-R2-B2-00** | **GO** |
| **ARCH-UI-04-R2-B2** | **EXÉCUTION AUTORISÉE** |
| DSP FINAL LOCK | MAINTENU (`git diff d2d08ac -- source/` vide) |
| R2-C, R2-D, R2-E, R2-F, ARCH-UI-05, ARCH-UI-06 | non autorisés |

## 2. Arbitrages définitifs (DB2)

| ID | Option | Décision normative |
|---|---|---|
| DB2-01 | C | **Apple M3 de base : minimum officiel.** M1 et M2 hors support officiel. Des mesures complémentaires sur M1 sont autorisées si une machine est disponible, sans extension du support. macOS Apple Silicon reste la cible de distribution ; Windows reste une plateforme secondaire de développement et de vérification. Une mesure sur VM GitHub ne vaut jamais certification physique M3. **EC-07 résolu.** |
| DB2-02 | A | **Gestes PLASMA de 0.4.1 conservés** : le clic place le curseur en position absolue ; le glisser est rapporté à la longueur utile du canal. Les contrats clavier, molette, saisie numérique et retour au défaut sont conservés. Le glisser relatif de 220 px du laboratoire n'est pas transposé. Il faut vérifier l'absence de saut involontaire à la prise en main du curseur. Un défaut supplémentaire du comportement historique est documenté et soumis avant toute modification. PL-06 est amendé (§5). **EC-05 résolu.** |
| DB2-03 | B | **Validation progressive en B2-04** des occurrences P2 et P3 et de la microcopie hors CSV. L'interface finale est entièrement en anglais. Méthode obligatoire : familles sémantiques, décisions de vocabulaire dédupliquées, homonymies et ambiguïtés identifiées, traductions évidentes distinguées des décisions fonctionnelles, exceptions et vrais arbitrages présentés, validation groupée, traçabilité exhaustive. Aucune validation individuelle des 1 890 occurrences n'est demandée. Aucune application silencieuse. NAM-01 est exigible à la clôture de R2-B2, après validation et application. **EC-08 : traitement fixé.** |
| DB2-04 | A | **0.4.1 conservée pendant l'intégration.** Le passage à 0.5.0 (OD-17) est préparé à la clôture de R2-B2 ; l'autorisation finale est demandée avant toute modification effective. Les fichiers impactés seront documentés. Aucune release publique, aucun tag de release, aucun merge vers `main`. **EC-15 : traitement fixé.** |
| DB2-05 | A | **Monture commune Cybernetic HUD R1 seule** (verre optique, encadrement, rails, typographie, couleurs fonctionnelles, nature et source des données si pertinentes et disponibles) pour les éditeurs interactifs, les vues XY, réseau, ordre Z et flux, et l'éditeur de banque. Contenu graphique, poignées, interactions, contrats et géométrie générale sont conservés. **EC-16 résolu.** |
| DB2-06 | A | **Habillage R1 des widgets JUCE génériques par le LookAndFeel** ; widgets et contrats conservés ; remplacement réservé à R2-E et R2-F2. |
| DB2-07 | B | **Seuils GS définitifs** (§4), enregistrés avant toute mesure. |

## 3. Amendements de contre-expertise (CE-B2)

| ID | Amendement normatif |
|---|---|
| CE-B2-01 | **Tests de reproduction permanents.** Les scénarios PB-01 (et ceux de tout autre défaut reproduit puis corrigé) deviennent des tests permanents de non-régression. Ne sont jamais supprimés : la preuve historique du défaut, la séquence de gestes avant correction et le scénario de reproduction. Après correction, les assertions corrigées et les critères BE s'appliquent : profondeur maximale 1, appariement begin/end, absence de geste résiduel, **une seule transaction Undo** pour le retour au défaut, comportement correct en **instances multiples**. *Remplace la phrase « le test de reproduction est retiré » du dossier §7.* |
| CE-B2-02 | **Géométrie PLASMA par surface utile.** Le gabarit se choisit d'après la surface réellement disponible : cellule moins les marges, bordures, titres, zones de texte et contraintes de manipulation. Les dimensions externes de la cellule ne suffisent pas. STANDARD et COMPACT sont vérifiés selon les dimensions normatives R1, sur les cas suivants : OUT GAIN, Z du Core, hauteurs variables, tailles minimale, nominale et maximale, zooms 75, 100 et 150 %, lisibilité, zone de manipulation, absence de chevauchement. Si COMPACT ne tient pas réellement, le blocage est documenté, sans déformer le composant, réduire la typographie ni anticiper R2-C. *Précise TB2-03.* |
| CE-B2-03 | **Indépendance des preuves visuelles.** Les captures historiques R1 publiées (`B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES/`) sont des références **indépendantes et immuables**. Chaîne de validation obligatoire : (1) référence R1 → laboratoire utilisant les peintres partagés ; (2) référence R1 → composant intégré en production ; (3) composant intégré → interface complète. Les conditions de rendu sont normalisées : dimensions, états, données, polices, plateforme. Les écarts légitimes dus au contexte de production sont documentés. Comparer deux composants qui utilisent le même peintre ne démontre pas la conformité artistique. Une référence historique n'est jamais régénérée pour faire passer un test. *Précise VIS-01 et INT-02.* |
| CE-B2-04 | **Robustesse de GC-01.** Les mesures exigent : répétition, échauffement, alternance référence / intégré, conditions identifiées, traitement explicite des valeurs aberrantes, données brutes publiées, analyse de la dispersion, stabilisation des caches mémoire, distinction rendu hors écran / affichage interactif, distinction VM / machine physique. Un snapshot JUCE mesure un coût de rendu ; il ne prouve pas la fluidité dans Reason. La recette physique reste ouverte tant qu'elle n'est pas exécutée. |
| CE-B2-05 | **Corrections autorisées des écarts préexistants**, sans aucune modification DSP. **EC-01** : libellé et commentaires de l'oscilloscope alignés sur le vrai point de prélèvement, sans le déplacer. **EC-02** : nature INDICATIVE et limites de représentation du spectre affichées. **EC-03** : valeur du paramètre OUT GAIN séparée de la mesure de crête ; représentation PEAK distincte, identifiée MEAS ; sémantique du paramètre et mesure réelle existante conservées. Les sources et qualifications des HUD sont exactes dès leur intégration (B2-03 (5)) ; B2-06 en est la vérification exhaustive. |

## 4. Seuils GC-01 définitifs (DB2-07, enregistrés avant mesure)

| ID | Seuil définitif |
|---|---|
| GS-01 | Médiane du rendu complet ≤ 1,5 × référence 0.4.1 |
| GS-02 | p95 du rafraîchissement ≤ 1,5 × référence **et** ≤ 11,1 ms sur la machine physique de référence (M3 de base) |
| GS-03 | Aucun repeint périodique inutile au repos stabilisé, hors événements légitimes du système et de l'hôte |
| GS-04 | p99 audio ≤ 1,05 × référence ; aucune nouvelle échéance audio manquée attribuable à l'interface |
| GS-05 | Surcoût mémoire de l'éditeur ≤ 1,25 × référence ; aucune croissance persistante après stabilisation des caches |
| GS-06 | p95 des interactions ≤ 1,5 × référence ; objectif absolu supplémentaire de 16,7 ms au p95 pour le traitement et le rendu local d'une interaction directe (ce budget n'est pas une latence complète jusqu'à l'affichage physique) |
| GS-07 | Aucun surcoût significatif éditeur fermé ; aucun timer UI actif |

**Règles** :

- Un résultat bruité ou non concluant sur VM ne vaut pas PASS.
- Aucun assouplissement a posteriori sans arbitrage explicite.
- Les résultats comparatifs sur VM et les résultats physiques M3 sont toujours publiés séparément.

## 5. Amendement de PL-06 (DB2-02)

PL-06 est remplacé par :

> **PL-06 (amendé, R2-B2)** — Les PLASMA partagent avec les NEXUS les règles **communes** : clavier (flèches 1 %, Maj ×
> 0,1, Page 10 %, Début/Fin), molette regroupée en un geste (450 ms), saisie numérique, retour au défaut, une paire
> begin/end par geste. Leurs gestes **spécifiques** sont ceux de 0.4.1 :
> - le clic place la valeur en position absolue ;
> - le glisser est rapporté à la longueur utile du canal ;
> - aucun saut involontaire à la prise en main (un clic sur le curseur à sa position courante ne change pas la valeur
>   au-delà de la résolution d'un pixel).

## 6. Choix architecturaux validés (TB2-01 à TB2-08)

Validés sous réserve des amendements CE-B2. Architecture retenue :

- habillage en place ;
- classes et interfaces de production conservées ;
- contrats `ParamLink` et `BoundControl` conservés ;
- tokens, primitives et peintres R1 extraits dans `ui/r2/`, consommés par le plugin et le laboratoire ;
- registre d'identifiants ;
- alias de libellés générés ;
- workflow de preuves dédié ;
- ligne de base 0.4.1 préalable.

**Indépendance des peintres** (normatif). Un peintre ne dépend ni des écritures de paramètres, ni des gestes hôte, ni de
l'automation, ni d'un état mutable partagé entre instances, ni du moteur DSP. Il reçoit un modèle de vue et un état ; il
dessine. Les gestionnaires d'événements **ne sont pas** partagés entre le laboratoire et la production. Les mécanismes
communs de gestes (`GestureGuard`) et de valeurs (`ValueSource`) ont des responsabilités distinctes du rendu.

## 7. Critères d'acceptation amendés

Amendements de [B2-ACCEPTANCE-CRITERIA.md](B2-ACCEPTANCE-CRITERIA.md), enregistrés avant mesure :

| ID | Amendement |
|---|---|
| INT-02 | La preuve visuelle suit la chaîne CE-B2-03 (R1 historique → laboratoire ; R1 historique → composant de production ; composant → interface complète) ; VIS-01 n'est plus une comparaison entre deux consommateurs du même peintre |
| INT-04 | PL-06 lu dans sa version amendée (§5) ; test de prise en main sans saut |
| INT-05 | Scénarios PB-01 permanents (CE-B2-01), y compris l'Undo unique et les instances multiples |
| INT-06 | Mêmes règles pour tout défaut PB-02 ou PB-N reproduit et corrigé en B2 |
| INT-07 | NAM-01 exigible à la clôture, après validation groupée (DB2-03) |
| INT-10 | EC-01, EC-02 et EC-03 corrigés dès l'intégration HUD (CE-B2-05) |
| INT-12 | Seuils GS définitifs (§4) et robustesse CE-B2-04 ; une mesure VM non concluante n'est pas un PASS |
| INT-18 | Recette physique sur Mac M3 dans Reason 14 ; protocole préparé en B2-08 |
| Nouveau : PLAS-G | Géométrie PLASMA par surface utile (CE-B2-02), au minimum sur OUT GAIN et le Z du Core, aux trois tailles et aux trois zooms |

## 8. État des écarts après arbitrage

| Écart | État |
|---|---|
| EC-01, EC-02, EC-03 | Correction autorisée (CE-B2-05), dès B2-03 (5) et (2) |
| EC-04 | Consigné (addendum de la matrice) |
| EC-05 | Résolu par DB2-02 et l'amendement de PL-06 |
| EC-07 | Résolu par DB2-01 |
| EC-08 | Traitement fixé par DB2-03 (B2-04) |
| EC-12 | Précisé par CE-B2-02 |
| EC-15 | Traitement fixé par DB2-04 |
| EC-16 | Résolu par DB2-05 |
| EC-06, EC-09, EC-10, EC-11, EC-13, EC-14 | Inchangés : traitement prévu au registre |
