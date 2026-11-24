# SUBDESTRUCKT — Core Contracts P0 — QCM de clôture
Date : 2026-11-24. Statut : **DRAFT / NON VALIDÉ**. Aucun code.
But : fermer les contrats fondamentaux avant architecture C++ détaillée. La causalité FM/CROSS est exclue de ce QCM et reste réservée au spike S0.

## A. Voice / Amp / Note–Tail–Drone
### 1. Placement fonctionnel de l'enveloppe Amp
A. Tout à la fin, après tous les traitements.
**B. Recommandé — sur le signal de synthèse avant les traitements capables de produire des tails, afin que leurs queues puissent continuer après Note Off.**
C. Une enveloppe Amp séparée par moteur.
D. Placement libre par preset.

### 2. Note Off
A. Reset immédiat de tous les états DSP.
**B. Recommandé — ferme l'excitation/gain de voix mais ne reset pas les états de filtre, résonateur, buffer, freeze ou feedback.**
C. Laisse tout tourner indéfiniment.
D. Comportement configurable par module.

### 3. Tail
A. Durée fixe après Note Off.
**B. Recommandé — Tail continue tant qu'une énergie audible existe, avec garde-fou temporel uniquement pour états non autonomes.**
C. Tail illimité.
D. Aucun Tail, extinction avec Amp.

### 4. États autonomes
A. Self-oscillation/Freeze sont toujours coupés au Note Off.
**B. Recommandé — ils sont distincts du Tail et ne peuvent rester audibles que s'ils sont explicitement autorisés/armés.**
C. Ils restent toujours actifs.
D. Chaque preset décide sans politique globale.

### 5. Drone
A. Drone bypass l'enveloppe Amp.
**B. Recommandé — Drone maintient le Gate/état de note selon le contrat normal, sans contourner l'enveloppe ni les protections.**
C. Drone force tous les générateurs en continu.
D. Drone est supprimé de V1.

### 6. Panic
A. Hard reset instantané sample-to-sample.
**B. Recommandé — très courte rampe anti-click vers silence puis purge des états autonomes/feedback/buffers concernés et désarmement Drone.**
C. Mute seulement sans reset.
D. Reset uniquement au prochain Note On.

## B. MIDI / expressivité
### 7. Aftertouch V1
A. Aucun Aftertouch.
**B. Recommandé — Channel Pressure comme source globale de modulation.**
C. Poly Pressure uniquement.
D. Channel + Poly Pressure.

### 8. MPE
**A. Recommandé — hors périmètre V1.**
B. Support MPE complet.
C. Seulement MPE pressure.
D. Préparer l'UI sans moteur MPE.

### 9. Pitch Bend
A. Plage globale fixe.
**B. Recommandé — profondeurs Up/Down indépendantes sauvegardées avec le preset, défaut ±2 demi-tons.**
C. Toujours ±12.
D. Réglage application non sauvegardé dans le preset.

### 10. Précision temporelle MIDI
A. Traitement au bloc.
**B. Recommandé — événements MIDI appliqués à leur sample offset dans le bloc.**
C. Quantification à la fréquence de contrôle.
D. Sample-accurate uniquement pour notes.

### 11. Ordre des événements au même sample
A. Ordre reçu du host sans normalisation.
**B. Recommandé — ordre déterministe : commandes de sécurité/transport, expression (bend/CC/pressure), Note Off, puis Note On.**
C. Note On avant Note Off.
D. Ordre configurable.

### 12. MIDI Learn
A. Par preset uniquement.
**B. Recommandé — mapping MIDI Learn utilisateur/global séparé des presets sonores; un preset peut exposer des macros mais ne remappe pas le contrôleur physique.**
C. Mapping embarqué dans chaque preset.
D. Pas de MIDI Learn V1.

## C. Quality / Clean-Raw / oversampling / latence
### 13. Nature de Quality
A. Paramètre créatif sauvegardé dans les presets.
**B. Recommandé — préférence d'instance/projet, défaut Normal; pas une composante de l'identité sonore du preset.**
C. Préférence globale application uniquement.
D. Quality supprimé.

### 14. Clean / Raw
A. Dépend automatiquement de Quality.
**B. Recommandé — paramètre artistique sauvegardé dans le preset, indépendant de Quality.**
C. Raw uniquement en Eco.
D. Clean forcé en High.

### 15. Oversampling
A. Facteur libre exposé pour chaque module.
**B. Recommandé — géré automatiquement par moteur/algorithme selon ses besoins et Quality; pas de matrice de facteurs manuels en façade.**
C. Oversampling global unique.
D. Toujours 4x.

### 16. Changement de Quality pendant lecture
A. Changement instantané sans transition.
**B. Recommandé — changement sécurisé avec reconstruction hors RT et transition/crossfade contrôlé; aucune rupture de state.**
C. Interdit pendant transport.
D. Nécessite recharger le preset.

### 17. Latence VST3
A. Latence fixe maximale pour tous les presets.
**B. Recommandé — latence déclarée selon la configuration réellement active lorsqu'un algorithme l'exige, avec alignement interne dry/wet et entre moteurs; changements rares et contrôlés.**
C. Ne jamais déclarer de latence.
D. Supprimer tous les algorithmes à latence.

### 18. Sample rates officiellement testés V1
A. 44.1 / 48 uniquement.
**B. Recommandé — 44.1 / 48 / 88.2 / 96 kHz officiellement testés; 192 kHz non garanti en V1.**
C. Jusqu'à 192 kHz obligatoire.
D. 48 kHz uniquement.

## D. Protection de sortie
### 19. Headroom interne
A. Cible 0 dBFS partout.
**B. Recommandé — headroom nominal interne de l'ordre de -12 dBFS, sans limiter les excursions créatives contrôlées.**
C. -24 dBFS.
D. Auto gain permanent.

### 20. DC / denormals
A. Aucun traitement imposé.
**B. Recommandé — protection denormals + garde DC très basse fréquence / servo transparent non désactivable là où nécessaire.**
C. HPF audible global.
D. Traitement optionnel.

### 21. Dépassement de 0 dBFS en sortie plugin
A. Hard clip systématique à 0 dBFS.
**B. Recommandé — autorisé en float avec alerte/mètre; la protection obligatoire ne clamp que les états numériques dangereux, pas le signal musical normal.**
C. Limiteur brickwall permanent.
D. Normalisation automatique.

### 22. Limiteur musical
A. Toujours actif.
**B. Recommandé — option créative/sécurité utilisateur distincte du garde-fou numérique non désactivable.**
C. Aucun limiteur possible.
D. Seulement en High.

## E. Parameter Registry canonique
### 23. Identifiants
A. IDs générés depuis les noms d'UI.
**B. Recommandé — StableID explicites, immuables, jamais réutilisés; nom/path UI séparés.**
C. Indices numériques dépendant de l'ordre.
D. GUID aléatoires régénérés aux versions.

### 24. Base vs Effective
A. Le host automatise directement la valeur effective modulée.
**B. Recommandé — le host automatise la Base Value; la matrice calcule l'Effective Value sans réécrire le paramètre host.**
C. Les deux sont automatisables.
D. L'Effective Value seulement.

### 25. Paramètres structurels
A. Tous modulables continûment.
**B. Recommandé — algorithmes, banques, topologie, FFT size, oversampling et structures sont événementiels/transactionnels, jamais destinations continues par défaut.**
C. Tout est audio-rate.
D. Selon le preset.

### 26. Métadonnées obligatoires du registre
A. ID + min/max/default seulement.
**B. Recommandé — StableID, type, unité, domaine de sommation, base/artistic/DSP ranges, default, rate capabilities, smoothing, automation, randomization, preset scope, version, availability/dependencies.**
C. Métadonnées dispersées dans chaque module.
D. Générées automatiquement depuis l'UI.

### 27. Compatibilité conditionnelle
A. Une route invalide est supprimée.
**B. Recommandé — conservée inactive avec raison explicite; jamais redirigée ni downgradée silencieusement.**
C. Redirigée vers la destination la plus proche.
D. Ignorée sans notification.

## F. Axe Z — fermeture sans invention
### 28. Source de vérité du catalogue Z
A. Redéfinir maintenant huit nouvelles transformations.
**B. Recommandé — récupérer d'abord le catalogue historique exact; ne pas inventer les cinq transformations manquantes.**
C. Passer immédiatement à trois transformations seulement.
D. Laisser Claude choisir.

### 29. Si les cinq transformations historiques restent introuvables
A. Inventer les cinq pour conserver le chiffre huit.
**B. Recommandé — conserver la cible V1 de huit, verrouiller les trois connues (Harmonic Fold, Phase Warp, Spectral Mutation) et bloquer l'implémentation des cinq autres jusqu'à un QCM Z dédié.**
C. Réduire définitivement V1 à trois sans nouvel arbitrage.
D. Retirer Z.

### 30. Composition des deux slots Z
A. Toujours parallèle.
**B. Recommandé — deux slots série A→B avec ordre inversable B→A; Z Master reste global et chaque slot conserve sa profondeur/courbe.**
C. Les deux slots ne peuvent jamais être simultanés.
D. Routage libre.

## Recommandation complète
**1B 2B 3B 4B 5B 6B 7B 8A 9B 10B 11B 12B 13B 14B 15B 16B 17B 18B 19B 20B 21B 22B 23B 24B 25B 26B 27B 28B 29B 30B**

## Effet de la validation
Après validation, ce document devient le contrat P0 officiel. Le baseline et l'audit sont mis à jour. La causalité FM/CROSS reste ouverte et passe au spike S0. Aucun développement intégral n'est autorisé par cette validation seule.
