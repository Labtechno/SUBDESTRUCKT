# ARCH-UI-04-R2-A — télémétrie et Reactive Signal Flow (DA-A15)

DA-A15 retient un flux du signal **réactif à l'activité réelle**. Ce document recense ce que le moteur 0.4.1 publie
réellement, à partir de la lecture de `source/engine/Telemetry.h` et de `Engine::publishTelemetry`
(`source/engine/Engine.cpp`), sans modification. Il classe chaque donnée selon son usage possible sans toucher au DSP
verrouillé.

## 1. Mécanisme

- Publication : à la fin de chaque bloc hôte, par le thread audio, en écritures atomiques relâchées. Aucune
  allocation, aucun verrou (contrats RT existants).
- Lecture : timer de l'éditeur à 30 Hz (`tok::refreshHz`). L'UI voit donc **une valeur de fin de bloc échantillonnée à
  30 Hz**. Une valeur brève entre deux lectures peut être manquée, sauf pour la crête (maximum du bloc) et l'oscilloscope
  (anneau).
- `ScopeRing` : 4 096 valeurs, somme mono 0,5 × (L + R), une valeur sur deux, prélevée en **sortie finale** (après
  OUT GAIN, garde ELP, limiteur et rampe PANIC). Soit environ 186 ms d'historique à 44,1 kHz.

## 2. Classes

| Classe | Catégorie du brief (§18) | Signification | Utilisable en R2 |
|---|---|---|---|
| **A** | Disponible et exploitable | Publié, directement utilisable | Oui |
| **B** | Disponible avec adaptation UI | Dérivable côté UI à partir de données A ou de l'état du thread de contrôle | Oui, avec mention ESTIMATE si la dérivation est approximative |
| **C** | Nécessitant une modification du moteur ou de son interface | Existe dans le moteur mais n'est pas publié. La publication exige une modification de `source/engine/` (verrouillé). | Non, sauf autorisation distincte (dérogation au DSP FINAL LOCK, comme R1) |
| **D** | Non disponible | Non calculé par le moteur. Exige une instrumentation nouvelle du thread audio, donc aussi une modification du moteur. | Non (hors R2) |

## 3. Inventaire

| ID | Donnée | Disponible | Source | Cadence | Utilisable sans modif DSP | Limitation |
|---|---|---|---|---|---|---|
| T-01 | Fréquences hôte/moteur, suréchantillonnage, latence, qualité, support | A | `hostRate`, `engineRate`, `oversampling`, `latencySamples`, `quality`, `supported` | Bloc | Oui | Diagnostic |
| T-02 | Alignement de délai (entier, Thiran) | A | `delaySamples`, `delayInteger`, `thiranCoefficient` | Bloc | Oui | Diagnostic |
| T-03 | Note courante, gate | A | `currentNote`, `gate` | Bloc | Oui | Une seule note publiée |
| T-04 | Drone actif | A | `drone` | Bloc | Oui | — |
| T-05 | Autonomie : suspendue, autorisée, verrou PANIC, source, transport, hors ligne, compteurs | A | `autonomySuspended`, `autonomyAuthorized`, `autonomyLatched`, `autonomySource`, `transportState`, `offline`, `authGrants`, `authRevocations`, `authGeneration`, `taDropped` | Bloc | Oui | Le transport est le dernier état observé |
| T-06 | Niveau de l'enveloppe AMP | A | `envelope` (`envs_[0].level()`) | Bloc | Oui | — |
| T-07 | Niveaux des enveloppes WT, FM, AUX1, AUX2 | A | `sources[EnvWt…EnvAux2]` | Bloc | Oui | Valeur de source de modulation |
| T-08 | Étape courante d'une enveloppe (D, A, H, D, S, R) | C | `Envelope::stage()`, exposé seulement par un accesseur de test (`testEnvStage`) | — | **Non** | Pas de curseur d'étape sans autorisation |
| T-09 | Valeurs des LFO, aléatoires, chaos, macros, suiveurs, Y2 | A | `sources[kNumModSources]` | Bloc | Oui | Valeur seule |
| T-10 | Phase des LFO | C | `phase()` interne (`source/dsp/ModSources.h`) | — | **Non** | Animation de phase impossible. La valeur seule est disponible (T-09). |
| T-11 | Valeur Effective des destinations modulées | A | `effective[703]`, `modulated[703]` | Bloc | Oui | Uniquement les destinations modulées par la matrice active |
| T-12 | Positions Effective WT X/Y/Z | A | `wtX`, `wtY`, `wtZ` | Bloc | Oui | — |
| T-13 | Profondeur effective des 55 routes FM/CROSS | A | `effectiveDepth[55]` (après Master/direction et lissage) | Bloc | Oui | Profondeur, pas niveau de signal |
| T-14 | Autorisation de rendu temporaire | A | `autonomySource = Render`, `offline` | Bloc | Oui | Contrat 0.4.1 §2 bis |
| T-15 | Fondu de topologie | A | `edgeGain[55]`, `transitionPhase`, `activeSerial` | Bloc | Oui | Transition seulement |
| T-16 | Révision de la matrice | A | `modSerial` | Bloc | Oui | Déclenche le rafraîchissement de la vue |
| T-17 | Crête de sortie L/R | A | `peakL`, `peakR` (maximum du bloc, sortie finale) | Bloc | Oui | Maintien et décroissance à calculer côté UI (B) |
| T-18 | Oscilloscope | A | `scope` (anneau) | Continu | Oui | Mono, une valeur sur deux, sans filtre anti-repliement |
| T-19 | Spectre | B | FFT côté UI de l'anneau | 30 Hz | Oui | Mono. Bande limitée à fs hôte / 4 (11,0 kHz à 44,1 kHz). Repliement possible au-delà. |
| T-20 | Compteurs de sûreté (non finis, écrêtage, PM, événements perdus, PANIC, limiteur, ELP) | A | `nonFiniteResets`, `outputClampHits`, `pmClampHits`, `droppedEvents`, `panics`, `limiterHits`, `elpHits` | Bloc | Oui | Compteurs cumulés ; activité = variation entre deux lectures (B) |
| T-21 | Charge CPU | A | `cpuLoad` (wrapper, dernier bloc) | Bloc | Oui | Pas de moyenne publiée ; moyenne glissante côté UI (B) |
| T-22 | État des liens de la matrice (actif, raison d'inactivité) | B | `modRouteStatus` (thread de contrôle), `ModInactive` | Événement | Oui | Statut logique, pas activité |
| T-23 | Contribution instantanée d'un lien global | B approximatif / C exact | Source (T-09) × profondeur configurée | 30 Hz | Approximatif seulement | Ignore la courbe, « via », le lissage et la polarité. Mention ESTIMATE obligatoire (OD-07). |
| T-24 | Niveau de signal par route FM/CROSS | D | — | — | **Non** | Instrumentation nouvelle |
| T-25 | Niveau de sortie par opérateur FM | D | — | — | **Non** | idem |
| T-26 | Niveaux par chaîne (WT, FM, GLOBAL, avant/après Cross Audio) | D | — | — | **Non** | idem |
| T-27 | État autonome effectif d'un étage (en cours d'exécution autonome, gel actif) | C | `ProcessingChain::autonomousActive()`, `Destruction::frozen()` internes | — | **Non** | L'UI affiche la configuration (ARM, FREEZE) et l'autorisation (T-05), pas l'exécution par étage |
| T-28 | Ordre effectif des transformations Z | B | Calcul du thread de contrôle à partir des paramètres (afficheur 0.4.1) | Événement | Oui | La restriction inter-domaines n'est pas validée |
| T-29 | Présence d'un contenu capturé D2/S2 | B | État du processeur (captures), thread de contrôle | Événement | Oui | — |
| T-30 | Lecture d'automation par l'hôte | D | Non exposé de façon fiable par l'API VST3 | — | **Non** | Pas d'indicateur « automation en cours » (OD-09) |

## 3 bis. Cibles de DA-A15

| Cible (brief §18) | Données | Faisabilité sans modification DSP |
|---|---|---|
| Valeur Effective des NEXUS | T-11 | **Oui** (destinations modulées) |
| Marqueurs PLASMA CHANNEL | T-11 | **Oui** |
| Activité des routes | T-13 profondeur effective, T-15 fondus (A) ; T-23 contribution (B, ESTIMATE) ; T-24 niveau de signal (D) | **Partielle** : structure et profondeur effective oui, niveau de signal non |
| Progression des enveloppes | T-06, T-07 niveaux (A) ; T-08 étape (C) | **Partielle** : niveau oui, étape non |
| Phase des LFO | T-09 valeur (A) ; T-10 phase (C) | **Partielle** : valeur oui, phase non |
| Réactivité du Core | T-11, T-12, T-17, T-09 (A) | **Oui**, sur données publiées |
| Télémétrie des OPTICAL DISPLAY | T-01…T-05, T-17…T-21 (A/B) | **Oui** |

## 4. Exigences

| ID | Exigence |
|---|---|
| TEL-01 | Le Reactive Signal Flow n'utilise que des données de classe A ou B (INV-11) |
| TEL-02 | Une donnée B approximative porte la mention ESTIMATE |
| TEL-03 | Toute donnée C ou D exige une mission distincte, autorisée explicitement, avec son propre rapport (dérogation au DSP FINAL LOCK). R2-B à R2-H n'en dépendent pas. |
| TEL-04 | Les animations ne dépassent pas 30 Hz et cessent quand l'éditeur est masqué |
| TEL-05 | L'absence de signal (silence, transport arrêté, autonomie suspendue) est affichée explicitement. Aucune animation résiduelle ne la masque. |
| TEL-06 | Le point de prélèvement de chaque mesure (sortie finale, mono) est indiqué à l'écran |
| TEL-07 | La lecture de la télémétrie ne crée ni allocation ni verrou sur le thread audio (contrat inchangé). La lecture reste du côté de l'éditeur. |
| TEL-08 | Le coût de rendu de la réactivité est mesuré en prototype (R2-G) |

## 5. Modes OFF / REACTIVE / IMMERSIVE (proposition non validée, PR-01)

| Mode | Contenu proposé | Données | Intérêt | Coût (non mesuré) | Interaction avec le futur Core |
|---|---|---|---|---|---|
| OFF | Flux statique : structure, profondeurs configurées, états logiques | Paramètres, T-13, T-15, T-22 | Lecture stable, sessions longues, machines modestes | Repeint sur changement seulement | Core en animation de repos seule, ou statique |
| REACTIVE | + activité des sources, Effective, crête, transitions | T-06, T-07, T-09, T-11, T-12, T-17 | Comprendre la modulation en jouant | Repeint à 30 Hz des éléments animés (PT-04) | Core réactif aux valeurs Effective et à la crête |
| IMMERSIVE | + oscilloscope ou spectre intégré au flux, estimation par lien (ESTIMATE) | T-18, T-19, T-23 | Démonstration, exploration sonore | Le plus élevé : FFT et liaisons animées (PT-04) | Le Core pourrait consommer le spectre. Risque de surcharge visuelle et de confusion mesure/estimation. |

Ces modes sont une **proposition**. Leur existence, leurs noms, leur défaut et leur persistance (préférence d'instance ou
d'utilisateur, jamais dans le preset) relèvent de l'arbitrage OD-07.

## 6. Conclusion

Un Reactive Signal Flow fidèle est réalisable **sans modification du DSP** pour l'activité des sources de modulation,
les valeurs Effective, la profondeur effective des routes, les transitions de topologie, les états d'autorisation et la
sortie mesurée. Il ne l'est pas pour le niveau de signal par route, par opérateur ou par chaîne, ni pour les étapes
d'enveloppe et la phase des LFO. Ces lacunes ne sont pas comblées par simulation.
