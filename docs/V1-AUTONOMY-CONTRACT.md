# SUBDESTRUCKT — Contrat des états et autorisations autonomes (ARCH-G4.3, inchangé en ARCH-G4.3-R1 / 0.3.3)

Statut : **implémenté et validé en automatique** (TA-01..25, TA-xxw, AUT-01..08, R2-01..05) ; **validation physique en
attente** (Reason 14, Ableton Live, exports : recette R-26, R-27, R-28). Critères : `docs/V1-ARCH-G4.3-SPEC.md` §3, §6.
Code : `source/engine/AutonomyGate.{h,cpp}`, `source/engine/Engine.cpp` (frontière de bloc, Panic, captures),
`plugin/PluginProcessor.cpp` (transport, restauration, journal).

## 1. Quatre catégories, jamais confondues

| Catégorie | Persistée | Contenu |
|---|---|---|
| Configuration sonore | oui (preset, projet) | paramètres, dont les 9 armements `flt.*.arm`, `dst.*.arm`, les gels ; Drone (`instance drone=`) |
| Contenu créatif R2 | oui (projet ; preset) | captures D2 / S2 (`capture`, intégrité SHA) |
| État d'exécution | non | tampons DSP, gels en cours, Gate maintenu |
| **Autorisation d'exécution** | **jamais** | accordée / suspendue (génération) / révoquée (Panic, verrou) |

Armement effectif = configuré ET autorisé (9 armements) ; Drone effectif = Drone configuré ET autorisé. Sans
autorisation, le moteur applique les armements comme désarmés et n'injecte aucune capture ; la configuration reste
intacte (aucun « dirty »). `drone_restore` décide de restaurer la **configuration** du Drone, jamais son exécution.

## 2. Sources et ordre d'évaluation (thread audio, une fois par bloc hôte)

1. Suspension : nouvelle génération (restauration d'un projet ou d'une duplication portant un armement, une capture ou
   un Drone restauré ; preset portant une capture).
2. Transport : INCONNU (pas de playhead, position absente, contexte VST3 absent : temps non fini) / ARRÊT / LECTURE
   (± enregistrement). INCONNU ne modifie jamais le dernier état connu et ne vaut jamais ARRÊT.
3. Candidats : EXPLICITE (Autoriser, clic d'armement ou de Drone dans l'éditeur ; lève le verrou Panic) ; TRANSPORT
   (ARRÊT → LECTURE observé) ; RECORD (début d'enregistrement pendant une lecture) ; RENDER (traitement hors temps réel,
   transport connu en LECTURE, autorisation absente). Une commande explicite antérieure à une restauration plus récente
   est périmée.
4. Un bloc contenant un All Sound Off (bouton Panic, CC120) n'accorde **aucune** autorisation.
5. Événements : Panic révoque **systématiquement** (même sans armement) et pose le verrou ; le verrou n'est levé que par
   un ARRÊT réel ou une autorisation explicite. All Notes Off (CC123) ne révoque pas.

Ne changent pas l'autorisation : boucle, seek, tempo, LECTURE maintenue, éditeur ouvert ou fermé, `prepareToPlay`
(fréquence, taille de bloc), désactivation/réactivation, Quality. Instance neuve : autorisée (configuration par défaut
sans état autonome).

## 3. Captures R2

Préparées hors RT au chargement et publiées au moteur, qui les **retient** (l'ensemble précédent est retiré, libéré hors
RT). Appliquées une fois à chaque autorisation (et après re-préparation), à la première frontière de bloc où
l'emplacement, **armé et autorisé**, exécute D2/S2 (fenêtre de 2 s) ; jamais à chaque bloc. Un emplacement non armé ne
rejoue jamais un contenu mémorisé. Sauvegarde : aucune capture automatique d'une instance suspendue ou dont la
restitution est en attente (aucune perte). Restitution S2 à une autre fréquence : rééchantillonnage en énergie
(énergie totale conservée ; résolution limitée par la grille de bins cible : R2-02, échec documenté).

## 4. Contraintes temps réel

Aucune allocation, aucun verrou, aucun accès disque, aucune FFT de reconstruction dans le callback pour l'autorisation ;
commandes atomiques ; journal d'événements SPSC (256 entrées, débordement compté) vidé par le thread message ; fichier
`ta_log.txt` écrit seulement si `ta_diagnostics.on` existe (dossier `Application Support/Labtechno/SUBDESTRUCKT`).
`authorizeAutonomy()` (thread de contrôle) ne publie plus de captures : c'est une commande atomique.

## 5. Limites non résolubles par l'API (JUCE 9.0.6 / VST3)

- Instance neuve qui rend en **temps réel** alors que la lecture a déjà commencé : indiscernable d'une insertion pendant
  la lecture → pas d'autorisation automatique (sécurité).
- Rendu hors temps réel **sans** information de transport → pas d'autorisation automatique.
- Hôte qui ne traite pas l'audio à l'arrêt : le premier bloc observé est une LECTURE → pas d'autorisation automatique.
- Rappel d'état par l'hôte pendant la lecture (Undo hôte, rechargement) : suspension jusqu'au prochain ARRÊT → LECTURE.

Alternative générique pour ces cas : Autoriser avant la lecture ou l'export. Aucun contournement propre à un hôte n'est
implémenté (arbitrage requis).
