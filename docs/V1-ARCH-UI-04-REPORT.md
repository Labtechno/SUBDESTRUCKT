# SUBDESTRUCKT — Rapport ARCH-UI-04 : interface fonctionnelle SYMBIOTIC INDUSTRIAL (0.4.0)

Date : 2026-11-27. Branche `feature/arch-g4-full-development`. Mission UI seule : **aucun fichier `source/` modifié**
(DSP FINAL LOCK, `docs/V1-DSP-FINAL-LOCK.md`). Les SHA et le run CI de la livraison sont consignés au §2 et au tracker.

## 1. Verdict

**GO CONDITIONNEL.** L'interface des trois espaces est implémentée et prouvée automatiquement (20 critères UI04, tous
PASS en local ; 703/703 StableID atteints et édités par leur commande ; rendu DSP bit à bit identique avec et sans
éditeur ; ASan sans erreur). Conditions de levée : (a) CI macOS arm64 SUCCESS sur le SHA livré (pluginval rigueur 10,
RTSan, paquets) ; (b) compilation locale du SHA par `build_local_macos.sh` sur le Mac du propriétaire ; (c) recette
humaine courte H-01..H-09 (`docs/V1-ARCH-UI-04-RECETTE.md`, ~25 min). Aucun test physique n'a été exécuté à distance.

## 2. Version et commits

Version **0.4.0** (convention : nouvelle génération fonctionnelle → mineure ; lots correctifs G4.x → correctif ;
le moteur reste celui de 0.3.3). Commits : `86b84e5` (consignation DSP FINAL LOCK), `603abf8` (Design System et trois
espaces), `16e4559` (suite UI04 et correctifs), `dafa6be` (cycle de vie des composants), puis commit de version/
documentation (SHA livré et run CI : tracker §5). Contrôle : `git diff 0469c4d -- source/` vide.

## 3. Critères enregistrés et résultats (local MSVC Windows, build Release ; CI macOS : tracker)

| ID | Critère | Résultat local |
|---|---|---|
| UI04-INV | 703 StableID uniques, identiques à la liste figée | PASS (703/703, ordre identique) |
| UI04-COV | chaque StableID atteint par une commande liée visible, contextes compris | PASS 703/703 ; 23 contextes, 662 états, 440 accès contextuels |
| UI04-BIND | édition par la commande → Base hôte = cible, aucune écriture annexe, retour | PASS 703/703, 0 écriture annexe |
| UI04-ALIAS | commandes simultanées d'un même StableID cohérentes | PASS (51 StableID multi-commandes) |
| UI04-INACT | valeurs inactives conservées après changement d'algorithme/mode | PASS (340 changements, 0 violation, bit à bit) |
| UI04-NAV | navigation/zoom/tailles/Inspector : aucune écriture ; aucun composant détruit | PASS (552 vues, 235 sélections, 0 écriture, 0/885 détruit) |
| UI04-CLIP | aucune troncature, aucune commande hors parent ni chevauchée | PASS (12 configurations, 4 804 commandes) |
| UI04-PANIC | Panic visible dans toutes les configurations ; clic = révocation | PASS |
| UI04-UNDO | une édition = une transaction ; geste Core X/Y = une transaction | PASS |
| UI04-AUTO | automation hôte (thread non-UI) reflétée sans réécriture ni transaction | PASS |
| UI04-STATE | état identique éditeur ouvert/fermé (format 4) ; restauration reflétée | PASS |
| UI04-KEY | clavier : pas, reset, saisie numérique (virgule, unité) | PASS |
| UI04-CORE | X=cross.fwd, Y=cross.rev, Z=cross.master, rien d'autre ; alternative numérique | PASS |
| UI04-CON | contrastes WCAG des tokens | PASS (texte 12,6:1 ; secondaire 6,4:1 ; inactif 4,6:1) |
| UI04-OPEN | 100 cycles ouverture/fermeture avec audio | PASS |
| UI04-MULTI | 4 instances avec éditeurs : isolation | PASS |
| UI04-RT | processBlock avec éditeur actif rafraîchi : 0 allocation | PASS (0) |
| UI04-DSP | rendu bit à bit identique avec/sans éditeur (navigation entre blocs) | PASS (0/153 600 échantillons différents) |
| UI04-DECO | aucun bouton visible sans action | PASS (474 boutons, 0 non relié) |
| UI04-OPS | opérations hors registre présentes | PASS (0 manquante) |

Tests historiques : cœur 169 critères, **8 FAIL documentés identiques à la baseline 0.3.3** (ELP-01, D2-04, D2-04c,
R2-02, R2-02R, ZC-05, ZC-13, ZC-15) ; seules diffèrent des mesures de durée (ns) ; wrapper 53 critères (33 + 20), seul
FAIL documenté AUT-02. Barrière `tools/check_results.sh` : **PASS**. ASan (MSVC `/fsanitize=address`, suite wrapper
complète) : **0 erreur**. Non exécutés localement : pluginval, RTSan, paquets (CI macOS uniquement) ; aucun test
physique, aucune mesure Retina réelle, aucune mesure de temps de rendu UI.

Précision (demande du propriétaire) : la baseline locale compte **8 FAIL cœur + 1 FAIL wrapper**, identiques fichier
par fichier aux annotations CI de 0.3.3 (run 44). L'annonce intermédiaire « 9 FAIL cœur » était une erreur de
décompte (confusion avec les 9 entrées de `tests/known_failures.txt`, toutes portées confondues). Aucune nouvelle
régression.

## 4. Défauts trouvés et corrigés pendant la mission (par les tests)

1. Changement d'espace par re-parentage de la vue → LookAndFeel transitoire et recréation de labels internes ;
   use-after-free dans UI-01w, crash intermittent (pile ASan). Correctif : vues attachées en permanence.
2. Destruction de composants lors d'une simple sélection (alias de l'Inspector) ou d'un changement de volet local ;
   pointeurs périmés → exception RTTI → `std::terminate` (0xC0000409, invisible pour ASan), CI `603abf8` interrompue.
   Correctif : alias et volets persistants ; garde UI04-NAV (0 composant détruit).
3. Bouton Autoriser de PERFORMANCE non synchronisé (AUT-08, test historique). Corrigé.
4. Libellés des NEURAL CELL tronqués alors que le test les déclarait conformes (géométrie de mesure ≠ dessin).
   Corrigé (zone unique) ; défaut révélé par la revue des captures, pas par le test initial.
5. Littéraux non ASCII décodés en Latin-1 (« prÃ©-chaÃ®nes »), glyphes ◂/▸ absents de la police système. Corrigés.

## 5. Couverture fonctionnelle

Cartographie générée depuis l'éditeur réel : [V1-ARCH-UI-04-STABLEID-MAP.md](V1-ARCH-UI-04-STABLEID-MAP.md)
(703/703 ; accès principal : SOUND DESIGN 672, PERFORMANCE 13, ENGINEERING 18 ; alias dans PERFORMANCE, Inspector,
FM/Cross Network). Opérations hors registre (UI04-OPS) : presets (précédent/suivant, chargement, favori, sauvegarde,
tags), banques (import/export WAV, éditeur de formes, générateur, .sdwt), topologies FM, routes FM/CROSS (ajout/retrait
avec classe B-T2 en lecture seule), matrice globale (ajout, profondeur, courbe dont Custom, cadence, via, actif,
retrait, filtre), courbe Custom des LFO, MIDI Learn (apprentissage, annulation, retrait), Randomize/Mutate/Evolve
(profil, intensité, familles, verrous), captures R2 (capture, effacement par emplacement), Excitation, Cross Reset,
Drone, Autoriser, Panic, Undo/Redo, Quality, zoom, Inspector (épingler, verrou, défaut, saisie). Captures réelles :
`docs/ui04-captures/` (20 vues, rendu logiciel JUCE de l'éditeur compilé sous Windows, 1440×900, 960×640, zoom 75 et
150 % ; ce ne sont pas des captures macOS Retina).

## 6. Écarts vis-à-vis du contrat ARCH-UI-03

- SYMBIOTIC CORE : commande directe seule, rendu statique provisoire identifié (périmètre ARCH-UI-04) ; aucun
  renderer 3D, morphing, mapping multiparamétrique ni nouveau paramètre.
- Placement : les profondeurs FM 4×4 et CROSS sont éditables à la fois en SOUND DESIGN (MOTEURS) et en ENGINEERING
  (réseaux) ; Voice/Tuning uniquement en ENGINEERING (pas de raccourci SOUND DESIGN) ; sortie/limiteur en fin de
  chaîne globale.
- Non implémentés (candidats ARCH-UI-03, non exigés) : recherche et favoris de paramètres, rétraction du Core,
  raccourcis clavier globaux (Undo, navigation) — volontairement absents pour ne pas capturer ceux de la DAW.
- Mesure réelle dans l'Inspector : sortie seulement (crête) ; aucune autre mesure n'existe dans la télémétrie.
- Tâches UX mesurées (UI03-02, UX01–18), profils de performance (UI03-07/08), contexte GPU (UI03-09) : non exécutés
  (aucun renderer GPU dans cette mission ; recette humaine pour la perception).
- Manuel HTML : aucune édition 0.4.0 produite ; le manuel 0.3.3 décrit l'interface provisoire (écart documentaire).

## 7. Régressions

Aucune régression DSP, d'état ou de latence détectée (cœur identique, UI04-DSP, UI04-STATE, G-21w/G-22w). UI-01w
(historique) reste PASS ; son indicateur « paramètres atteignables par un contrôle visible » mesure les seuls contextes
par défaut (220/703 contre 205/703 avant) — la couverture exhaustive est UI04-COV.

## 8. Limitations et arbitrages nécessaires

- Restriction d'ordre inter-domaines des transformations Z : représentée (« ordre effectif », alerte « sans effet ») ;
  **non arbitrée**.
- Libellés définitifs des deux commandes FM contextuelles (« FM INDEX », « FM FEEDBACK ») : provisoires.
- Tailles finales, densités et zoom : vérifiés en géométrie logique ; validation Retina humaine (H-02, H-06).
- Licence JUCE (B-04), signature/notarisation : inchangées ; aucune diffusion publique.
- Hooks Claude Code : le plugin claude-context-optimizer 4.12.1 invoque `node`, absent de la machine Windows (erreurs
  « node: command not found ») ; contrôles non exécutés : read-cache, context-shield, prompt-coach, tracker, budget,
  tasks, dashboard (télémétrie de contexte, aucun contrôle du projet). Hooks laissés actifs ; installer Node.js LTS
  rétablirait ces contrôles.

## 9. Avancement

R42–R45 (UI Performance, Sound Design, Engineering, Inspector/moniteurs/zoom) : IMPLEMENTED → **PASS automatique
(UI04)**, recette humaine H-01..H-09 à faire. Pondération ARCH-G4.1 sur 50 exigences : voir tracker §4.

## 10. Installation

Procédure inchangée (`docs/V1-LOCAL-BUILD.md`) : `tools/local-build/build_local_macos.sh --juce-licence=<choix>` au
SHA livré → `local-build/delivery-…/SUBDESTRUCKT_v0.4.0-b…_macOS_arm64/` (ZIP, Installer.pkg, Uninstaller.pkg,
README_FR, CHANGELOG, SHA256SUMS ; vérifications d'identité et de contenu par le script). La CI produit les mêmes
paquets comme artefacts internes, non publiés.

## 11. Recette humaine minimale

`docs/V1-ARCH-UI-04-RECETTE.md` : 9 points, ~25 minutes, Reason 14.1 sur M4 Max.

## 12. Recommandations pour ARCH-UI-05

1. Arbitrer d'abord la source de vérité des axes du Core (paramètres existants vs IDs append-only), la persistance et
   l'interaction avec l'automation, avant tout renderer.
2. Spike renderer isolé (Metal natif candidat) avec télémétrie bornée, pause hors écran, mesures frame time/CPU sur
   M4 Max puis M3 ; conserver l'alternative numérique et UI04-DSP/UI04-RT comme garde-fous.
3. Réutiliser `CorePad` (geste = une transaction) et les tokens ; aucune commande accessible uniquement en 3D.
4. Produire l'édition 0.4.x du manuel à partir des captures réelles macOS.
5. Ajouter une mesure de temps de rendu de l'éditeur (P95) au harnais local avant d'introduire l'animation.
