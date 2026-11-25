# SUBDESTRUCKT — ARCH-G3 — Fiche de recette physique (Mac Studio M4 Max)

À remplir par l'utilisateur sur sa machine. Ces tests ne peuvent pas être exécutés par la CI : elle tourne sur une
machine virtuelle, sans DAW et sans sortie audio. Chaque ligne passe de **PENDING** à **PASS** ou **ÉCHEC**, avec la
preuve correspondante : capture, version de la DAW, observation.

**Conditions à noter :**

- macOS (version exacte) ;
- DAW, avec sa version ;
- interface audio ;
- fréquence et taille de buffer utilisées ;
- build livrée (`0.1.0-bN`) et empreinte SHA-256 du ZIP.

| # | test | procédure | attendu | statut |
|---|---|---|---|---|
| R-01 | Archive | Vérifier l'empreinte du ZIP (`shasum -a 256 SUBDESTRUCKT_v…zip`), dézipper, puis `shasum -a 256 -c SHA256SUMS.txt`. | Empreinte identique au lien publié ; 4 lignes « OK ». | PENDING |
| R-02 | Installation | Ouvrir `SUBDESTRUCKT_Installer.pkg` (Réglages Système → Confidentialité et sécurité → « Ouvrir quand même ») et installer. | Installation réussie ; bundle présent dans `/Library/Audio/Plug-Ins/VST3/SUBDESTRUCKT.vst3`. | PENDING |
| R-03 | Découverte | Lancer la DAW et analyser les plugins VST3. | SUBDESTRUCKT (Labtechno) apparaît comme instrument VST3 et s'instancie. | PENDING |
| R-04 | Son et MIDI | Jouer des notes au clavier MIDI : notes liées, relâchements. | Son immédiat ; priorité à la dernière note ; relâchement correct. | PENDING |
| R-05 | WT seul / FM seul | UI de diagnostic : WT Level 1, OP1/OP2 Level 0, puis l'inverse (WT 0, OP1 1). | Deux timbres distincts, chacun audible seul. | PENDING |
| R-06 | CROSS | Faire varier CROSS WT>FM et FM>WT, puis CROSS Master de 1 à 0 en jouant. | Effet audible dans chaque direction ; Master à 0 supprime l'effet sans coupure ni reset. | PENDING |
| R-07 | Topologie | Boutons « + » / « − » des routes (dont OP1 > WT, classe D) en jouant. | Classes affichées conformes ; aucun plantage ; noter tout clic perçu. | PENDING |
| R-08 | Fréquences | Projet à 44,1, 48, 88,2 et 96 kHz. | Son correct ; l'UI affiche K = 2 à 44,1 kHz et 1 ailleurs, ainsi que la latence déclarée. | PENDING |
| R-09 | PDC | À 44,1 kHz, comparer la position d'une note rendue (bounce) avec la grille. | Latence de 32 échantillons compensée par la DAW. | PENDING |
| R-10 | Rechargement | Modifier des paramètres et la topologie, sauvegarder le projet, fermer puis rouvrir. | Paramètres, routes, identifiants et classes restaurés ; son identique. | PENDING |
| R-11 | Instances | Deux pistes avec SUBDESTRUCKT et des réglages différents. | Instances indépendantes. | PENDING |
| R-12 | Panic | Bouton Panic, ou CC120 envoyé par la DAW, pendant une note. | Silence en moins de 5 ms environ ; les notes suivantes jouent normalement. | PENDING |
| R-13 | Désinstallation | DAW fermée : ouvrir `SUBDESTRUCKT_Uninstaller.pkg`. | Bundle retiré, projets et autres plugins intacts ; journal dans `/var/log/install.log`. | PENDING |
| R-14 | Réinstallation | Réinstaller, puis rouvrir le projet de R-10. | Le projet se recharge avec le même son. | PENDING |
| R-15 | Refus | Lancer le désinstallateur une seconde fois (plugin absent). | Message explicite « SUBDESTRUCKT n'est pas installé », rien supprimé. | PENDING |

**Une seconde recette, sur un M3 physique en hôte VST3, est obligatoire avant toute Beta publique**
(V1-PLATFORM-REQUIREMENTS).
