# ARCH-UI-04-R2-B2 — addendum normatif B2-03 (GO d'exécution après contre-expertise B2-02)

Date : 2026-11-28. Complète [B2-00-ADDENDUM.md](B2-00-ADDENDUM.md). La mission R2-B2 et les instructions précédentes
restent normatives.

## 1. Décisions

| Élément | État |
|---|---|
| DB2-09 | **Option B validée** : plus de réinitialisation par double-clic dans le canal des PLASMA |
| CE-B2-06, CE-B2-07, CE-B2-08 | Validés (§3) |
| B2-01 | GO technique |
| B2-02 | **GO technique avec réserves GC-01** : GS-04 et GS-07 non concluants sur VM ; parties absolues de GS-02 et GS-06 à mesurer sur M3 physique |
| **B2-03** | **EXÉCUTION AUTORISÉE**, dans l'ordre NEXUS, PLASMA, NEURAL CELL, SYNAPTIC, HUD, PANIC ; un jalon atomique par famille |
| Arbitrages et amendements | DB2-01 à DB2-09 et CE-B2-01 à CE-B2-08 validés |

## 2. Contrat d'interaction définitif NEXUS / PLASMA (DB2-02, DB2-08, DB2-09)

| Interaction | NEXUS | PLASMA (vertical, horizontal, STANDARD, COMPACT) |
|---|---|---|
| Glisser | Relatif, historique 0.4.1 | Historique du canal (rapporté à la longueur utile) |
| Clic simple dans la zone de manipulation | Contrat historique | Position absolue immédiate |
| Double-clic dans la zone de manipulation | Retour au défaut | Position absolue, **sans retour au défaut** |
| Double-clic sur la valeur | Saisie numérique | Saisie numérique, sans écriture préalable |
| Clic ou glisser sur le libellé | Contrat historique | Aucune écriture (DB2-08) |
| Alt+clic, Suppr, menu contextuel | Retour au défaut | Retour au défaut |

**Interdits** :

- temporisation du clic simple ;
- glisser relatif hors canal ;
- réinitialisation par double-clic sur le libellé ;
- fusion artificielle des transactions Undo ;
- restauration transitoire de la valeur précédente ;
- nouveau raccourci ;
- toute modification du comportement NEXUS.

Les infobulles décrivent les interactions réellement disponibles.

## 3. Amendements de contre-expertise

| ID | Amendement normatif |
|---|---|
| CE-B2-06 | L'intégrité de `source/` et de `plugin/` prouve l'absence de modification du code protégé, **pas** l'absence de régression des performances audio. À vérifier : contention entre threads, timers résiduels, rappels asynchrones, ressources graphiques persistantes, synchronisations, allocations, destruction de l'éditeur, instances multiples, réouvertures répétées. GS-04 et GS-07 restent NON CONCLUANTS sans preuve suffisante. Une hypothèse d'architecture ne devient jamais un résultat expérimental. |
| CE-B2-07 | Thread audio en priorité temps réel autorisé en B2-07. Toute modification du banc impose : méthodologie identifiée et versionnée ; référence 0.4.1 rejouée avec le nouveau banc ; version intégrée rejouée avec exactement le même banc ; nouveau contrôle A/A ; mesure de la dispersion ; données brutes publiées ; résultats historiques conservés avec leur statut ; seuils GS maintenus. Deux méthodologies ne sont jamais comparées directement. La référence logicielle `0517426` reste figée. |
| CE-B2-08 | Indicateur MIDI Learn (CC) des NEXUS conservé, à partir des seules données MIDI Learn de 0.4.1 : typographie R1, couleurs fonctionnelles, sans masquer BASE, EFF, les marques de modulation ni les états, en XL, S et Micro, à tous les zooms. Aucun nouvel état MIDI. Consigné comme **écart d'intégration fonctionnel mineur (EC-21)**. Arbitrage ciblé si aucune implantation satisfaisante n'est possible. |

## 4. Critères EC-19 amendés (enregistrés avant mesure)

Le scénario historique PB-01b et ses preuves sont conservés. Les assertions corrigées démontrent :

- réponse immédiate au premier clic ;
- aucune réinitialisation au second clic ;
- aucune valeur intermédiaire due à une réinitialisation ;
- aucune seconde écriture si la position et la valeur sont identiques ;
- aucune seconde transaction Undo inutile ;
- aucune marche d'automation parasite ;
- appariement des gestes hôte, aucun geste résiduel ;
- retour au défaut fonctionnel par Alt+clic, Suppr et menu ;
- saisie numérique sans modification préalable.

Scénarios complémentaires :

- deux clics successifs à des positions différentes ;
- double-clic suivi d'un glisser ;
- double-clic aux extrémités du canal ;
- automation active ;
- instances multiples ;
- zooms 75, 100 et 150 %.

L'unicité de la transaction Undo n'est tenue pour acquise qu'après exécution.

## 5. Exigences par famille (barrière B2-03)

Une famille n'est franchie qu'avec :

- compilation réussie ;
- tests ciblés et non-régressions PASS ;
- contrats préservés et corrections démontrées ;
- conformité visuelle documentée (CE-B2-03, étapes 2 et 3, Windows et macOS, zooms 75, 100 et 150 %) ;
- mesures GC-01 acceptables ou réserves justifiées ;
- DSP FINAL LOCK intact ;
- CI de production vérifiée ;
- preuves publiées ;
- commit atomique ;
- tracker à jour.

PB-02a et PB-02d restent affectés à R2-C et R2-D (correction obligatoire avant la pré-BETA).
