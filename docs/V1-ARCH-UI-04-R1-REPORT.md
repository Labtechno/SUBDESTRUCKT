# SUBDESTRUCKT — Rapport ARCH-UI-04-R1 : TRANSPORT AUTHORIZATION & stabilité (0.4.1)

Date : 2026-11-28. Branche `feature/arch-g4-full-development`. Origine : audit physique indépendant de Codex (Mac Studio
M4 Max, Reason 14.1d100) concluant NO GO sur 0.4.0-b263320407 (SHA `8a61b12`) à cause de l'anomalie R-28.

## 1. Verdict

**Automatique : GO CONDITIONNEL. Physique : R-28 NON déclaré PASS.** L'anomalie est reproduite par des tests
automatiques sur la baseline 0.4.0, analysée, corrigée, et tous les tests de reproduction passent après correction,
sans régression d'un autre verdict historique. Condition de levée : validation physique de la correction par Codex sur
le Mac du propriétaire (protocole §9) et CI macOS arm64 complète sur le SHA livré (§2).

## 2. Version, commits, CI

Version **0.4.1** (correctif d'une livraison 0.4.0 : incrément de correctif, comme 0.3.1–0.3.3). Commits :
`7d57d94` (tests de reproduction) — **CI run 49 (46104379578) en ÉCHEC comme attendu** : la barrière refuse les
reproductions sur macOS arm64 (TA-R01 « autorisation du rendu par transport », 0,19 RMS après le rendu, comme en local) ;
puis `d2d08ac` (correction). **SHA livré : `d2d08ac1bb6ed09ee3ea1b5e5d5d409c41b586af`, build CI 0.4.1-b50 ; CI run 50
(46105804592) SUCCESS complet** : barrière PASS (seuls FAIL documentés, dont TA-15 critère remplacé), pluginval 1.0.4
rigueur 10 avec tests d'interface PASS, RTSan 0 violation sur toute la suite du cœur (TA-R comprise), bundle arm64 0.4.1
signé ad hoc, installation réelle en VM et découverte VST3 0.4.1 PASS, paquets et livraison construits (non publiés),
script de compilation locale (LB) PASS, manuel PASS.

## 3. Analyse causale (confirmée par les tests)

1. **Le candidat TRANSPORT primait sur RENDER pendant un rendu.** Projet restauré (états autonomes suspendus), ARRÊT
   observé en temps réel, puis export : au premier bloc hors temps réel en LECTURE, `beginBlock` voyait une arête
   ARRÊT → LECTURE et accordait une autorisation **transport**, permanente (TA-R01 sur 0.4.0 : « autorisation du rendu par
   transport »).
2. **La sortie du hors temps réel ne mettait fin à aucune autorisation.** Le passage `offline → temps réel` était
   seulement journalisé : après un export terminé ou annulé, l'instance restait autorisée et rejouait ses captures et
   états autonomes à l'arrêt (TA-R01/R02/R03/R08 : énergie ≈ 0,19 RMS après le rendu ; TA-R01w : crête −1,7 dBFS sur le
   processeur réel, à rapprocher des −11,8 dBFS mesurés par Codex dans sa configuration).
3. **Les observations faites pendant le rendu contaminaient l'historique temps réel** : un ARRÊT pendant l'export levait
   le verrou Panic, et la LECTURE suivante du rendu redevenait une arête transport (TA-R06 sur 0.4.0).
4. **Défaut aggravant (persistance)** : sous cette autorisation indue, la sauvegarde DAW capturait les tampons D2/S2 et
   **réécrivait les captures persistées** (TA-R14 sur 0.4.0, avec un thread audio concurrent comme dans un hôte réel).
5. **États DSP résiduels** : même avec la seule machine d'états corrigée, l'état d'exécution laissé par le rendu restitue
   encore du signal après la fin de l'autorisation (contre-épreuve : D2 Legacy 6,9e-3, D2 Regenerative 8,8e-3, S2 9,1e-2,
   N2 1,9e-2, Drone 5,0e-2 RMS la première seconde ; F4 modal seul s'éteint) : D2 avec Freeze configuré continue de
   boucler son tampon tant que le garde-fou de 10 s des états non armés n'intervient pas.

## 4. Correctif minimal (justification par fichier : `docs/V1-DSP-FINAL-LOCK.md` §6)

- `source/engine/AutonomyGate.{h,cpp}` : autorisation **temporaire de rendu** distincte de l'autorisation normale ;
  TRANSPORT et RECORD seulement en temps réel ; fin de l'autorisation de rendu au premier bloc temps réel ; historique
  temps réel et verrou Panic figés pendant le rendu (observations journalisées, source « rendu ») ; première observation
  après le rendu sans arête ; Panic et suspension retirent aussi l'autorisation de rendu ; vue « normale ».
- `source/engine/Engine.{h,cpp}` : `resetAfterRender()` — au retour en temps réel sans autorisation normale, remise à zéro
  de l'état d'exécution avant le premier échantillon (voix, sustain, Gate, enveloppes, lignes, chaînes D2/S2/N2/filtres,
  lignes d'alignement) par les primitives existantes de fin de rampe Panic, **sans** sémantique Panic (Drone et armements
  configurés intacts, aucun verrou) ; captures retenues conservées.
- `plugin/PluginProcessor.cpp` : capture automatique à la sauvegarde et capture demandée seulement sous autorisation
  normale.
Aucune temporisation, aucun thread graphique, aucune allocation, aucun verrou, aucun accès disque ajoutés au thread audio.

## 5. Tests (local MSVC Windows ; CI macOS : tracker)

Reproduction (échec attendu sur 0.4.0, commit `7d57d94`) puis après correction :

| Critère | Objet | 0.4.0 | 0.4.1 |
|---|---|---|---|
| TA-R01 | export avant PLAY | FAIL (transport ; 0,19 RMS après) | PASS (rendu ; 0 après) |
| TA-R02 | annulation avant PLAY (note tenue) | FAIL | PASS (0 dès le 1er bloc) |
| TA-R03 | export terminé avant PLAY | FAIL | PASS |
| TA-R04 | export après autorisation normale (non-régression) | PASS | PASS (conservée, gel rejoué) |
| TA-R05 | exports successifs | FAIL (0 autorisation de rendu) | PASS (2 autorisations, 4 captures, rendus identiques) |
| TA-R06 | Panic pendant l'export, ARRÊT pendant le rendu | FAIL (verrou levé) | PASS |
| TA-R07 | transport INCONNU pendant l'export | FAIL ((b) transport) | PASS |
| TA-R08 | retour temps réel ARRÊT, 12,8 s | FAIL | PASS (énergie et crête 0) |
| TA-R09 | retour temps réel LECTURE | FAIL | PASS (aucune arête artificielle) |
| TA-R10 | premier vrai ARRÊT → LECTURE | FAIL | PASS (au 1er bloc, 2 captures) |
| TA-R11 | restauration pendant l'export | FAIL | PASS (rendu seulement) |
| TA-R12 | isolation des instances (non-régression) | PASS | PASS |
| TA-R13 | captures conservées et restituées | FAIL | PASS (D2 = capture bit à bit) |
| TA-R14 | état persistant intact (wrapper) | FAIL | PASS |
| TA-R15 / R15w | sécurité temps réel (non-régression) | PASS | PASS (0 allocation) |
| TA-R16 | états résiduels D2 Legacy, D2 Regenerative, S2, N2, F4, Drone | FAIL | PASS (0 dès le 1er bloc, 12,8 s) |
| TA-R01w / R13w | processeur réel (setNonRealtime, playhead) | FAIL | PASS |

Tests historiques : cœur 184 critères (169 + 15) — verdicts identiques à 0.4.0 sauf **TA-15 clause (e)**, qui exige le
comportement fautif ; test inchangé, rapporté FAIL et documenté comme **critère remplacé** (`tests/known_failures.txt`),
remplacé par TA-R08/TA-R09 ; ses clauses (a)–(d) passent. Seules diffèrent par ailleurs des mesures de durée. Les 8 FAIL
historiques sont inchangés. Wrapper 57 critères (33 + 20 UI04 + 4 TA-R) : seul AUT-02 (documenté) échoue ; les 20 critères
UI04 passent. Barrière `tools/check_results.sh` : PASS (local et CI run 50). ASan MSVC (suite wrapper complète, sources finales) :
0 erreur. RTSan (CI) : 0 violation. pluginval rigueur 10 (CI) : PASS.

## 6. Non-régression

DSP de synthèse : aucun fichier d'algorithme modifié ; REG-01, goldens Z, D2, R2 et tous les tests audio historiques
inchangés. État : format 4, StableID et identifiants VST3 inchangés (G-21w, G-22w, REG-ID). Interface : aucune modification
(UI04 20/20). Coût : 7,8 ns par bloc hôte pour la machine d'autorisation (TA-20, contre 5,8).

## 7. Contrat

`docs/V1-AUTONOMY-CONTRACT.md` §2 bis (révision 0.4.1) : sept règles (deux autorisations, fin du rendu, arrêt réel des
états autonomes, autorisation normale conservée, historique figé pendant le rendu, persistance, exports successifs) et
limites d'API ajoutées.

## 8. Incident H-04 (redémarrage de Reason pendant l'automation WT X)

**Cause établie par Codex** : sur la disposition AZERTY active, le raccourci Computer Use `super+a` a déclenché `⌘Q` ;
le journal macOS montre une fermeture normale de Reason suivie d'un lancement distinct. Sur 0.4.0-b263320407 : 12 essais,
2 fermetures normales reproduites, 0 crash, aucun rapport de crash ; enregistrement et lecture de l'automation WT X,
isolation des instances, fermeture/réouverture de l'éditeur pendant l'enregistrement : validés. **Limite** : la
fermeture de l'éditeur avant le relâchement effectif d'un glisser n'a pas été démontrée physiquement.
Audit statique réalisé (gestes, attachements JUCE, cycle de vie, callbacks différés, destruction de l'éditeur, threads,
restauration pendant l'automation) : aucun défaut relié à H-04. Observations sans impact démontré, **non corrigées**
conformément à la décision du propriétaire (pas de correctif préventif spéculatif) : un double-clic sur un NEXUS peut
imbriquer deux gestes hôte (début, début, fin, fin) ; le SYMBIOTIC CORE et l'éditeur d'enveloppe ne ferment pas un
geste ouvert si l'éditeur est détruit pendant un glisser. Contrôles UI04 maintenus. H-04 n'est plus un blocage.

## 9. Validation physique demandée à Codex (R-28, 0.4.1)

Installer 0.4.1 (procédure `docs/V1-LOCAL-BUILD.md`), créer `~/Library/Labtechno/SUBDESTRUCKT/ta_diagnostics.on`, puis dans
Reason 14.1 :
1. Ouvrir le projet contenant une capture autonome D2 ou S2 : silence, bouton Autoriser visible.
2. Lancer un export sans PLAY préalable, puis l'annuler : **silence à l'arrêt attendu** (aucun signal, Autoriser visible).
3. Refaire un export complet sans PLAY : le rendu contient la restitution ; après l'export, silence à l'arrêt.
4. ARRÊT puis PLAY : la restitution reprend (autorisation « transport »).
5. Panic pendant un export, puis fin de l'export : silence ; le PLAY suivant (après un ARRÊT) autorise de nouveau.
6. Autoriser avant un export : l'autorisation est conservée après l'export.
7. Joindre `~/Library/Labtechno/SUBDESTRUCKT/ta_log.txt`. Lignes attendues pour l'étape 2 : `hors temps reel`,
   `transport lecture source=rendu hors temps reel`, `autorisation source=rendu hors temps reel`, puis au retour
   `temps reel` et `fin de l'autorisation de rendu source=rendu hors temps reel` ; **aucune** ligne
   `autorisation source=transport` avant un ARRÊT puis une LECTURE réels. Supprimer ensuite `ta_diagnostics.on`.

## 10. Limitations

- Un hôte qui ne signale pas le hors temps réel pendant un export (VST3 `kOffline`) reste indiscernable d'une lecture.
- Après un export, il faut un véritable ARRÊT → LECTURE (ou Autoriser) pour réactiver les états autonomes : une LECTURE
  observée la première n'autorise pas (choix de sûreté).
- Validation physique R-28 en attente (Codex) ; R-27 (Ableton Live) retiré, R-29/R-30 non exécutés (risques acceptés).
