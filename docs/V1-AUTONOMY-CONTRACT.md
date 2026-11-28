# SUBDESTRUCKT — Contrat des états et autorisations autonomes (ARCH-G4.3 ; révisé ARCH-UI-04-R1 / 0.4.1)

Statut : **implémenté et validé en automatique** (TA-01..25, TA-xxw, AUT-01..08, R2-01..05 ; ARCH-UI-04-R1 : TA-R01..R16,
TA-R01w, R13w, R14, R15w). Recette physique : R-26 Reason 14 GO (0.3.3) ; R-27 Ableton Live retiré (non certifié) ;
**R-28 (exports) : anomalie reproduite physiquement par Codex sur 0.3.3 et 0.4.0, corrigée en 0.4.1 ; PASS physique dans
le périmètre Reason 14.1d100 exécuté (contre-audit Codex, build 263320802, SHA `d2d08ac`)** ; PANIC pendant un export et
sauvegarde pendant un export : non exécutés physiquement (dialogue modal de Reason), couverts automatiquement (TA-R06,
TA-R14). Critères : `docs/V1-ARCH-G4.3-SPEC.md` §3, §6.
Code : `source/engine/AutonomyGate.{h,cpp}`, `source/engine/Engine.cpp` (frontière de bloc, Panic, captures),
`plugin/PluginProcessor.cpp` (transport, restauration, journal).

## 1. Quatre catégories, jamais confondues

| Catégorie | Persistée | Contenu |
|---|---|---|
| Configuration sonore | oui (preset, projet) | paramètres, dont les 9 armements `flt.*.arm`, `dst.*.arm`, les gels ; Drone (`instance drone=`) |
| Contenu créatif R2 | oui (projet ; preset) | captures D2 / S2 (`capture`, intégrité SHA) |
| État d'exécution | non | tampons DSP, gels en cours, Gate maintenu |
| **Autorisation d'exécution** | **jamais** | normale (accordée / suspendue par génération / révoquée par Panic, verrou) ; **temporaire de rendu** (ARCH-UI-04-R1) |

Armement effectif = configuré ET autorisé (9 armements) ; Drone effectif = Drone configuré ET autorisé ; « autorisé » =
autorisation normale OU autorisation temporaire de rendu (§2 bis). Sans
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

Ne changent pas l'autorisation normale : boucle, seek, tempo, LECTURE maintenue, éditeur ouvert ou fermé,
`prepareToPlay` (fréquence, taille de bloc), désactivation/réactivation, Quality, rendu hors temps réel. Instance neuve :
autorisée (configuration par défaut sans état autonome).

## 2 bis. Rendu hors temps réel (ARCH-UI-04-R1, anomalie R-28) — révision de 0.4.1

Défauts de 0.3.3 / 0.4.0 (démontrés par TA-R01..R16 et TA-R01w/R14 avant correction) : un ARRÊT observé en temps réel
puis un premier bloc de rendu en LECTURE produisait une autorisation **TRANSPORT** permanente (le candidat transport
primait sur le rendu) ; la sortie du rendu ne mettait fin à aucune autorisation ; les ARRÊTS observés pendant le rendu
levaient le verrou Panic et entraient dans l'historique temps réel ; une sauvegarde DAW pendant ou après l'export
réécrivait les captures persistées. Règles en vigueur :

1. **Deux autorisations distinctes.** Normale : EXPLICITE, TRANSPORT et RECORD (ces deux derniers seulement en temps réel).
   Temporaire de **RENDU** : premier bloc hors temps réel en LECTURE (transport connu) sans autorisation, hors verrou Panic
   et hors Panic du bloc. Elle n'est **jamais** promue en autorisation normale ; une autorisation normale (ex. Autoriser
   pendant le rendu) la remplace.
2. **Fin du rendu.** Au premier bloc temps réel (rendu terminé ou annulé), l'autorisation de rendu prend fin à la
   frontière de bloc, avant tout échantillon : aucune temporisation, aucun thread graphique, aucun Panic.
3. **Arrêt réel des comportements autonomes.** Si aucune autorisation normale ne subsiste à ce bloc, l'état d'exécution
   laissé par le rendu est remis à zéro avant le premier échantillon temps réel : voix (pile, sustain, Gate, enveloppes),
   lignes et historiques, chaînes de traitement (D2 Legacy/Regenerative, S2, N2, filtres et résonateurs), lignes
   d'alignement. Signal nul exigé dès le premier bloc et au-delà du garde-fou de 10 s (TA-R16). Le Drone configuré, les
   armements configurés et les captures retenues sont conservés ; ce n'est pas un Panic (aucun désarmement, aucun verrou).
4. **Autorisation normale préexistante conservée** pendant et après le rendu (TA-R04), sauf suspension ou Panic.
5. **Historique temps réel figé pendant le rendu.** Les observations faites pendant le rendu sont journalisées (source
   « rendu ») sans effet sur l'historique ni sur le verrou : un ARRÊT pendant un export ne lève jamais le verrou Panic
   (TA-R06). Au retour en temps réel, la **première observation connue** rétablit l'historique sans constituer une arête :
   une LECTURE observée la première n'autorise pas (TA-R09) ; il faut un véritable ARRÊT -> LECTURE (TA-R10). INCONNU ne
   vaut jamais ARRÊT (TA-R07).
6. **Persistance.** Seule une autorisation normale permet la capture automatique à la sauvegarde et la capture demandée :
   un rendu sous autorisation temporaire n'écrit jamais dans l'état persistant (TA-R14).
7. **Exports successifs** : chaque rendu obtient sa propre autorisation temporaire et restitue les captures retenues
   (TA-R05) ; restauration ou duplication pendant le rendu : suspension, puis autorisation de rendu seulement (TA-R11).

Critère historique remplacé : TA-15 clause (e) « retour temps réel : conservée » (ARCH-G4.3) exige l'ancien comportement ;
il reste exécuté et rapporté FAIL, documenté comme **critère remplacé** par décision du propriétaire (brief ARCH-UI-04-R1),
remplacé par TA-R08/TA-R09 (`tests/known_failures.txt`).

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
`ta_log.txt` écrit seulement si `ta_diagnostics.on` existe (dossier `~/Library/Labtechno/SUBDESTRUCKT` sur macOS,
chemin corrigé le 2026-11-27, voir `docs/V1-DSP-FINAL-LOCK.md` §4).
`authorizeAutonomy()` (thread de contrôle) ne publie plus de captures : c'est une commande atomique.

## 5. Limites non résolubles par l'API (JUCE 9.0.6 / VST3)

- Instance neuve qui rend en **temps réel** alors que la lecture a déjà commencé : indiscernable d'une insertion pendant
  la lecture → pas d'autorisation automatique (sécurité).
- Rendu hors temps réel **sans** information de transport → pas d'autorisation automatique.
- Hôte qui ne traite pas l'audio à l'arrêt : le premier bloc observé est une LECTURE → pas d'autorisation automatique.
- Rappel d'état par l'hôte pendant la lecture (Undo hôte, rechargement) : suspension jusqu'au prochain ARRÊT → LECTURE.

- Hôte qui ne signale pas le traitement hors temps réel (VST3 `kOffline`) pendant un export : le rendu est
  indiscernable d'une lecture ; l'autorisation obtenue par un ARRÊT -> LECTURE observé est alors normale (limite d'API).
- Retour en temps réel directement en LECTURE après un export : pas d'autorisation jusqu'au prochain ARRÊT -> LECTURE
  (choix de sûreté, ARCH-UI-04-R1).

Alternative générique pour ces cas : Autoriser avant la lecture ou l'export. Aucun contournement propre à un hôte n'est
implémenté (arbitrage requis).
