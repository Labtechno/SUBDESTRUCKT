# ARCH-UI-04-R2-B2 — critères d'acceptation (enregistrés avant mesure)

Les critères sont enregistrés **avant toute mesure** ; ils ne pourront pas être abaissés après mesure (règle du tracker
§3, INV-09). Un critère non exécuté reste NON EXÉCUTÉ, jamais PASS.

**Compatibilité des identifiants.** Le préfixe `INT-` n'existe nulle part dans le dépôt : vérifié sur `docs/`, `tests/`
et `lab/` au SHA `73b633b`. Chaque critère INT regroupe des critères de la matrice R2
([`../R2-ACCEPTANCE-MATRIX.md`](../R2-ACCEPTANCE-MATRIX.md)) ou des tests existants, qui restent la référence de mesure.
INT-19 à INT-21 complètent la liste proposée par la mission (§9).

**Nature** :

- **O** : obligatoire ;
- **SC** : seuil candidat, à confirmer par le propriétaire avant la mesure (DB2-07) ;
- **F** : mesure et conclusion.

**Vérification** :

- **AUTO** : automatique ;
- **PHYS** : recette physique ;
- **SUBJ** : validation du propriétaire ;
- **AUDIT** : contrôle documentaire.

| ID | Critère | Critères et tests rattachés | Nature | Vérif. | Plateforme de preuve |
|---|---|---|---|---|---|
| INT-01 | DSP FINAL LOCK intact | INV-01 (`git diff d2d08ac -- source/` vide, REG-01 et goldens identiques), INV-13 | O | AUTO | CI macOS |
| INT-02 | Design System R1 intégré sans dérive artistique | DS-01, DS-02, DS-05 ; **VIS-01** : rendu d'un composant de production et rendu du peintre partagé identiques au pixel près, à taille et état égaux ; non-régression du laboratoire contre les captures R1 publiées ; planches de production comparées aux planches R1 (SUBJ) | O + SUBJ | AUTO + SUBJ | macOS (référence), Windows (comparatif) |
| INT-03 | Identifiants stables et contrats de paramètres préservés | INV-02 (`expected_param_ids.txt`, noms hôtes), INV-07 (UI04-COV 703/703), NAM-10 (537 options), `tests/host/expected_vst3_param_ids.txt` inchangé | O | AUTO | CI macOS |
| INT-04 | Interactions et gestes hôte conformes | NX-05, NX-06, PL-06 (selon DB2-02), INV-06 ; BE-01 à BE-05 pour chaque famille modifiée ; UI04-KEY, UI04-BIND | O | AUTO | CI macOS + Windows local |
| INT-05 | PB-01 reproduit et traité selon OD-10 | Preuve de reproduction sur 0.4.1 publiée **avant** la correction ; puis BE-01 à BE-04 PASS (profondeur maximale 1) et valeur finale égale au défaut | O | AUTO | CI macOS |
| INT-06 | PB-02 reproduit ou état documenté, traitement rattaché au bon lot | Résultats PB-02a à PB-02e sur 0.4.1 : REPRODUIT, NON REPRODUIT ou BLOCKED, avec justification ; aucune correction du Core (R2-C) ni des éditeurs d'enveloppe (R2-D) en B2 ; ED-01 sans crash | O | AUTO + AUDIT | CI macOS |
| INT-07 | Nomenclature R1 appliquée dans le périmètre | NAM-02, NAM-03 (table ↔ composants, bijection), NAM-05, NAM-06, NAM-07 (lu selon LAB-A08), NAM-09, NAM-11, NAM-12, NAM-13 ; NAM-01 selon DB2-03 | O | AUTO | macOS |
| INT-08 | Repères critiques conformes à OD-06 | NX-11, ZM-01 : 26 repères ≥ 9 px écran à 75 %, mesurés dans la disposition de production ; NAM-08 ; aucune occurrence à gabarit inconnu déclarée PASS ; `Z · CROSS LVL` traité selon EC-13 | O | AUTO | macOS (référence), Windows |
| INT-09 | Accessibilité AX-01 vérifiée | AX-01 : nom complet et valeur formatée exposés pour 703/703 ; AX-03 ; NX-08 ; contrôle par l'API d'accessibilité de JUCE (AUTO) puis lecture VoiceOver (PHYS) | O | AUTO + PHYS | macOS |
| INT-10 | Télémétrie fidèle aux données disponibles | INV-11, CD-01 à CD-08, TEL-05, TEL-06 ; EC-01 et EC-02 corrigés ; aucune donnée de classe C ou D (TEL-03) ; état d'indisponibilité explicite quand la donnée manque | O | AUTO + AUDIT | macOS |
| INT-11 | PANIC conforme visuellement et fonctionnellement | NC-03, UI04-PANIC ; contraste du libellé carbone ≥ 4,5:1 sur la surface rendue en production (4 états, 3 zooms) ; isolement et visibilité inchangés | O | AUTO + PHYS | macOS |
| INT-12 | Performances graphiques mesurées | GC-01 selon [B2-GRAPHICS-PERFORMANCE-PROTOCOL.md](B2-GRAPHICS-PERFORMANCE-PROTOCOL.md) ; comparaison avec 0.4.1 ; seuils candidats confirmés avant mesure (DB2-07) | F + SC | AUTO (+ PHYS) | macOS VM (non certifiant) ; machine physique selon DB2-01 |
| INT-13 | Sauvegarde, restauration et automation sans régression démontrée | SV-01, INV-03, UI04-STATE, UI04-AUTO, NX-12, UR-01 | O | AUTO + PHYS | macOS |
| INT-14 | Tests historiques et nouveaux tests conformes | INV-04 (suite TA), INV-09 (`known_failures.txt` inchangé) ; barrière `tools/check_results.sh` ; migrations OD-01 sans affaiblissement (règles ID-06 et ID-07) | O | AUTO + AUDIT | CI macOS |
| INT-15 | CI de production verte | `arch-g4.yml` SUCCESS sur le SHA final (build, cœur, wrapper, hôte, pluginval rigueur 10, RTSan, paquets) | O | AUTO | CI macOS |
| INT-16 | Périmètre des travaux respecté | Aucun fichier de `source/` modifié ; aucun travail R2-C, D, E, ARCH-UI-05 ou 06 ; aucune réorganisation spatiale ; liste des fichiers modifiés comparée à la liste prévue (dossier §10) | O | AUDIT | — |
| INT-17 | Preuves et documentation publiées | Livrables du §10 de la mission, SHA distant vérifié, dépôt propre | O | AUDIT | — |
| INT-18 | Réserves de recette physique conservées | Recette physique (Reason 14, Mac de recette) NON EXÉCUTÉE tant qu'elle n'est pas faite ; VM ≠ machine physique ; Windows ≠ cible | O | AUDIT | — |
| INT-19 | Zooms et redimensionnement | ZM-01 (75, 100 et 150 % dans les trois espaces) ; UI04-CLIP ; RS-01 pour les familles modifiées (geste conservé ou proprement refermé) | O | AUTO | macOS |
| INT-20 | Laboratoire opérationnel | LAB-G01 à LAB-G15 PASS ; NEXUS et PLASMA identiques aux captures B1 ; planches R1 identiques au pixel près aux captures R1 publiées (si TB2-02 est retenu) ; outils de nomenclature `--check` | O | AUTO | macOS (CI du laboratoire) |
| INT-21 | Undo/Redo et instances multiples | UR-01, UI04-UNDO (une édition = une transaction, y compris le défaut au double-clic après PB-01), MI-01, UI04-MULTI ; aucun cache statique mutable dans les peintres | O | AUTO | macOS |

## Seuils de non-régression typographique

- **NAM-08** : largeur mesurée ≤ budget, sinon deux lignes admises ; corps ≥ 9,5 px.
- **Marge d'alerte** : moins de 8 px (`OK_MARGE<8` de PT-02). Une marge sous ce seuil est une réserve documentée, pas un
  PASS sans commentaire.
- **Budget inconnu** : une occurrence sans budget connu dans la disposition finale reste INDÉTERMINÉE, rattachée à son lot.
