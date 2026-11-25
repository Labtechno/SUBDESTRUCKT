# SUBDESTRUCKT — ARCH-G3 — Clôture de recette physique
Décision de direction technique : GO ARCH-G3 dans la portée du prototype 0.1.0-b5. Recette utilisateur du 25 novembre 2026 (horodatages UTC exportés). Ce document n'est pas une certification de performance, ni une autorisation de diffusion.

## Traçabilité
- Build source : 186fb8644ae9518e3cb5116f4368b45dcea0d430.
- Livraison : b49a09e3efd6fe52fdb75a23c5b12dd794f696e8.
- Rapport/branche feature/arch-g3-vertical-slice : 8d2334250994758f2182b18c3a47dd059cb68495.
- Run de livraison 45609212850 : 60 PASS rapportés, 0 échec; 2 PASS RTSan séparés. Validation VST3 et packages sur VM.
- Session de recette : 88544a2d-1928-4652-90ae-63eecb9e395e. Export utilisateur validé à 2026-11-25T22:14:24.080Z, 15 PASS, 0 FAIL, 0 PENDING, 0 N/A. Export original indique seulement 96000 Hz; il n'est pas réécrit. Confirmations conversationnelles ultérieures : test à 44,1 kHz (dont R-09), 48 et 88,2 kHz. 
- Machine déclarée Mac Studio M4 Max, macOS Tahoe 26.5.2, Reason 14, Arturia AudioFuse 16Rig, buffer déclaré 512 (pas de mesure par fréquence). 
- Source de preuve : déclaration utilisateur acceptée; aucune capture ni mesure brute indépendante requise. R-04 observation : toutes les notes sur toutes les octaves disponibles ont joué. Pas d'observations individuelles supplémentaires.

| Test | Résultat accepté |
|---|---|
| R-01 Archive | PASS |
| R-02 Installation | PASS |
| R-03 Découverte | PASS |
| R-04 Son et MIDI | PASS |
| R-05 WT / FM | PASS |
| R-06 CROSS | PASS |
| R-07 Topologie | PASS |
| R-08 Fréquences | PASS, quatre fréquences confirmées |
| R-09 PDC | PASS à 44,1 kHz, confirmation déclarative |
| R-10 Rechargement | PASS |
| R-11 Instances | PASS |
| R-12 Panic | PASS |
| R-13 Désinstallation | PASS |
| R-14 Réinstallation | PASS |
| R-15 Refus | PASS |

## Limites
Validation du prototype seulement, pas du catalogue V1 ni des fonctions non implémentées. Aucun benchmark M3/M4 mesuré; M3 physique obligatoire avant Beta publique. NO-GO M1 historique conservé. Pas de garantie de click-free, Quality final non implémenté, undo/redo non exposé, LinFM/TZFM/ExpFM non implémentés; contention du verrou JUCE à auditer. Licence JUCE 9 non arbitrée et binaire accessible publiquement : priorité à la clarification des obligations et au contrôle de diffusion avant toute nouvelle publication binaire publique. Signature Developer ID/notarisation manquantes. Compatibilité macOS 14 non certifiée. 

## Suite recommandée
Préparer ARCH-G4 par lots indépendants selon l'audit : S1 Wavetable/XY/Z, S2 FM/PM, S3 ADAA/wavefolder, S4 Mod Matrix, S5 State/RNG, S6 performance cumulée, S7 ergonomie. Premier cadrage S1 : fondations XY, données immuables, interpolation et aliasing; cinq transformations Z historiques restent bloquées, pas de catalogue inventé. Toute implémentation part du code validé 8d23342 et intègre les décisions documentaires main; pas de merge automatique. Préserver 22 StableID et régression 0.1.0-b5, packaging obligatoire pour toute version installable, documentation versionnée. Seuils, licence et périmètre précis à arbitrer avant livraison. 
