# SUBDESTRUCKT 0.4.0 — Recette humaine finale ARCH-UI-04 (courte, groupée)

Durée cible : **25 minutes**, en une séance, sur le Mac Studio M4 Max (Reason 14.1, 44,1 kHz). Ne contient que ce
qu'aucun test automatique ne peut démontrer : rendu sur écran Retina réel, perception, gestes souris/trackpad réels,
dialogue avec l'hôte. Tout le reste est couvert par les tests UI04-* (voir `docs/V1-ARCH-UI-04-REPORT.md`).

Prérequis : livraison 0.4.0 compilée par `tools/local-build/build_local_macos.sh` au SHA communiqué (procédure
`docs/V1-LOCAL-BUILD.md`), installée par `SUBDESTRUCKT_Installer.pkg`. Un projet Reason 0.3.3 existant (R-26) sous la
main.

| ID | Action | Attendu | Résultat |
|---|---|---|---|
| H-01 | Ouvrir le projet R-26 (0.3.3) avec la 0.4.0 installée | Le son et les réglages sont identiques ; aucun message d'erreur ; si le projet contient des armements ou un Drone, ils restent suspendus (bouton Autoriser visible) | |
| H-02 | Ouvrir l'éditeur ; parcourir PERFORMANCE, SOUND DESIGN (3 onglets), ENGINEERING (10 onglets) | Lisibilité sur écran Retina : aucun libellé tronqué, aucun texte flou, contraste confortable ; aucun changement sonore pendant la navigation | |
| H-03 | Sur WT X : glisser, puis Maj+glisser, double-clic, double-clic sur la valeur et saisir `0,3` ; molette | Réglage fluide, réglage fin perceptible, retour au défaut, saisie acceptée ; un Undo par geste (bouton Undo de la barre ; aucun raccourci clavier n'est capturé à la DAW) | |
| H-04 | Enregistrer une automation de WT X dans Reason en tournant le NEXUS, puis la relire | L'automation est écrite pendant le geste et relue ; la commande suit la lecture ; aucun saut ni boucle | |
| H-05 | SYMBIOTIC CORE : glisser le point, puis régler Z ; écouter | X agit sur CROSS WT>FM, Y sur FM>WT, Z sur Master (audible) ; l'étiquette « rendu statique provisoire » est visible | |
| H-06 | Redimensionner la fenêtre au minimum (coin inférieur droit), puis zoom 75 % et 150 % | Défilement local disponible, PANIC toujours visible et cliquable, aucune commande superposée | |
| H-07 | Jouer une note tenue, cliquer PANIC | Silence immédiat ; Drone désarmé ; autorisation révoquée (barre et ÉTATS AUTONOMES) | |
| H-08 | Deux instances avec éditeurs ouverts ; modifier une macro dans l'une ; sauver, fermer, rouvrir le projet | Isolation des instances ; valeurs, nom de macro et routes rechargés à l'identique | |
| H-09 | Avis artistique : l'interface évoque-t-elle SYMBIOTIC INDUSTRIAL (graphite, titane, verre fumé, accents maîtrisés, aucun effet rétro) ? | Accord, ou liste des écarts à reprendre en ARCH-UI-05 | |

Non demandé ici (déjà acté) : R-27 Ableton Live (retiré), R-28/R-29/R-30 (non exécutés, risques acceptés,
`docs/V1-DSP-FINAL-LOCK.md`). Le CPU et le temps de rendu de l'interface ne sont pas mesurés dans cette recette.

Résultat global : GO / GO CONDITIONNEL / NO GO — à reporter dans le tracker (`docs/V1-ARCH-G4-TRACKER.md`).
