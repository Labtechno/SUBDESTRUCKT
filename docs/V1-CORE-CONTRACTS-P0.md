# SUBDESTRUCKT — Core Contracts P0
Date : 2026-11-24. Statut : **VALIDÉ**. Aucun code.
Décision utilisateur : validation intégrale du QCM P0 selon la recommandation **1B 2B 3B 4B 5B 6B 7B 8A 9B 10B 11B 12B 13B 14B 15B 16B 17B 18B 19B 20B 21B 22B 23B 24B 25B 26B 27B 28B 29B 30B**.

Ce contrat ferme les règles fondamentales qui peuvent être décidées sur papier avant l'architecture C++ détaillée. La causalité FM/CROSS est régie séparément par [V1-CAUSAL-CONTRACT.md](V1-CAUSAL-CONTRACT.md).

## 1. Voice / Amp / Note–Tail–Drone
- **P0-C01 Amp placement — B.** L'enveloppe Amp agit sur le signal de synthèse avant les traitements capables de générer des tails, afin que leurs queues puissent continuer après Note Off.
- **P0-C02 Note Off — B.** Note Off ferme l'excitation/gain de voix mais ne reset pas filtres, résonateurs, buffers, Freeze ou feedback.
- **P0-C03 Tail — B.** Le Tail continue tant qu'une énergie audible subsiste; un garde-fou temporel peut s'appliquer aux états non autonomes.
- **P0-C04 Autonomous states — B.** Self-oscillation, Freeze et autres états autonomes sont distincts du Tail et ne persistent que s'ils ont été explicitement autorisés/armés.
- **P0-C05 Drone — B.** Drone maintient le Gate/état de note selon le contrat normal; il ne bypass ni l'enveloppe Amp ni les protections.
- **P0-C06 Panic — B.** Panic applique une très courte rampe anti-click vers le silence, puis purge les états autonomes/feedback/buffers concernés et désarme Drone.

## 2. MIDI / expressivité
- **P0-C07 Aftertouch — B.** Channel Pressure est une source globale de modulation.
- **P0-C08 MPE — A.** MPE est hors périmètre V1.
- **P0-C09 Pitch Bend — B.** Profondeurs Up/Down indépendantes, sauvegardées avec le preset; défaut ±2 demi-tons.
- **P0-C10 MIDI timing — B.** Les événements MIDI sont appliqués à leur sample offset dans le bloc.
- **P0-C11 Same-sample order — B.** Ordre déterministe : sécurité/transport → expression (bend/CC/pressure) → Note Off → Note On.
- **P0-C12 MIDI Learn — B.** Mapping utilisateur/global séparé des presets sonores; les presets peuvent exposer des macros mais ne remappent pas le contrôleur physique.

## 3. Quality / Clean-Raw / oversampling / latence
- **P0-C13 Quality — B.** Quality est une préférence d'instance/projet, défaut Normal; ce n'est pas une composante de l'identité sonore du preset.
- **P0-C14 Clean/Raw — B.** Clean/Raw est un paramètre artistique sauvegardé dans le preset et indépendant de Quality.
- **P0-C15 Oversampling — B.** Facteurs gérés automatiquement par moteur/algorithme selon leurs besoins et Quality; pas de matrice de facteurs manuels exposée.
- **P0-C16 Quality transition — B.** Changement sécurisé avec préparation/reconstruction hors RT et transition contrôlée, sans rupture de state.
- **P0-C17 VST3 latency — B.** Latence déclarée selon la configuration réellement active lorsqu'un algorithme l'exige; alignement interne dry/wet et entre moteurs; changements rares et contrôlés.
- **P0-C18 Sample rates — B.** V1 officiellement testée à 44.1 / 48 / 88.2 / 96 kHz. 192 kHz non garanti.

## 4. Protection de sortie
- **P0-C19 Headroom — B.** Headroom nominal interne cible de l'ordre de -12 dBFS, sans empêcher les excursions créatives contrôlées.
- **P0-C20 DC/denormals — B.** Protection denormals et garde DC/servo très basse fréquence transparent, non désactivable là où nécessaire.
- **P0-C21 >0 dBFS output — B.** Dépassement de 0 dBFS autorisé en float avec alerte/mètre; la protection obligatoire ne clamp que les états numériques dangereux, pas le signal musical normal.
- **P0-C22 Musical limiter — B.** Limiteur musical optionnel et distinct du garde-fou numérique non désactivable.

## 5. Parameter Registry canonique
- **P0-C23 Stable IDs — B.** StableID explicites, immuables, jamais réutilisés; nom et chemin UI séparés.
- **P0-C24 Base/Effective — B.** Le host automatise Base Value; la matrice calcule Effective Value sans réécrire le paramètre host.
- **P0-C25 Structural parameters — B.** Algorithmes, banques, topologies, FFT size, oversampling et autres structures sont événementiels/transactionnels; jamais destinations continues par défaut.
- **P0-C26 Required metadata — B.** StableID, type, unité, domaine de sommation, plages base/artistique/DSP, défaut, rate capabilities, smoothing, automation, randomization, preset scope, version, disponibilité/dépendances.
- **P0-C27 Conditional compatibility — B.** Une route devenue incompatible est conservée inactive avec raison explicite; jamais redirigée ou downgradée silencieusement.

## 6. Axe Z
- **P0-C28 Source of truth — B.** Récupérer d'abord le catalogue historique exact; ne pas inventer les cinq transformations manquantes.
- **P0-C29 Missing transformations — B.** La cible V1 reste huit transformations. Les trois attestées — Harmonic Fold, Phase Warp, Spectral Mutation — sont verrouillées. Les cinq autres restent bloquées jusqu'à récupération historique ou QCM Z dédié.
- **P0-C30 Z composition — B.** Deux slots série A→B avec ordre inversable B→A; chaque slot conserve profondeur/courbe, Z Master reste global.

## 7. Effets sur l'architecture
1. Voice lifecycle, Event Timeline et traitement des tails doivent refléter C01–C06.
2. Le flux MIDI doit être sample-accurate et déterministe selon C07–C12.
3. Quality, Clean/Raw et Oversampling sont trois concepts séparés; l'architecture ne doit pas les fusionner.
4. Le moteur doit supporter une latence VST3 dépendante de la configuration et une compensation interne cohérente.
5. Les protections numériques obligatoires doivent rester distinctes des traitements musicaux.
6. Le Parameter Registry devient une dépendance architecturale centrale, à créer avant l'implémentation définitive des moteurs.
7. Les transformations Z inconnues ne peuvent pas être implémentées par extrapolation.
8. La causalité FM/CROSS relève du contrat causal distinct.

## 8. Statut des anciens blocages d'audit
- AUD-P0-03 Voice/Amp/Note Off : **FERMÉ par ce contrat**.
- AUD-P0-04 MIDI/expressivité : **FERMÉ par ce contrat**.
- AUD-P0-05 Quality/latence/oversampling : **FERMÉ au niveau contractuel**; valeurs de budget et performances restent à mesurer.
- AUD-P0-06 Protection/headroom : **FERMÉ au niveau contractuel**; seuils exacts et tests restent à mesurer.
- AUD-P0-07 Parameter Registry : **FERMÉ au niveau schéma contractuel**; registre concret à produire dans l'architecture.
- AUD-P0-01 Axe Z : **PARTIELLEMENT FERMÉ**; politique de non-invention et composition verrouillées, cinq transformations historiques toujours manquantes.
- AUD-P0-02 Causalité FM/CROSS : **CHOIX CLOS B-T2 — recette intégration ouverte**.

Aucune validation de ce document n'autorise à déclarer les DSP PASS. Les décisions sont des contrats de conception, pas des résultats de tests.


## Décision de clôture S0.1 — 2026-11-25
B-T2 est retenu comme contrat produit, avec minimum permanent K=2 à 44,1 kHz. ARCH-G3 est autorisé sous conditions de recette d'intégration, sans certification de performance. Voir [V1-CAUSAL-CONTRACT.md](V1-CAUSAL-CONTRACT.md) et [V1-ARCH-G3-MISSION.md](V1-ARCH-G3-MISSION.md). Le NO-GO historique du benchmark M1 reste inchangé; M3 physique avant Beta publique. Les formulations antérieures conditionnant le choix à S0.1 ou au M1 sont historiques et remplacées pour les travaux futurs par cette décision. Aucun GO ARCH-G4 ni release publique.
