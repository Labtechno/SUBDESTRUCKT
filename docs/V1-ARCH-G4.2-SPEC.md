# SUBDESTRUCKT — ARCH-G4.2 : critères de validation (écrits AVANT toute mesure)

Date : 2026-11-27. Base : `90e12f982688cd9513b8dd50e5457cd05b259591` (ARCH-G4.1), branche
`feature/arch-g4-full-development`. Référentiel documentaire `main` : `9a07d7654cfce951900cda9e8bec7303b8007a15`
(ARCH-UI-03, non commencé ici). Ce document fixe les critères des travaux ARCH-G4.2 avant leur exécution ; aucun seuil
ne sera assoupli après mesure. Les critères historiques (G-15, DST-09, G-21w, CAP-01, CAP-02w…) et leurs résultats
historiques sont conservés tels quels ; les critères ci-dessous les **complètent**.

## 1. ELP — SPEC-HR-01 (proposition technique, arbitrage final du propriétaire en attente)

Fonction (dsp/Elp.h) : identité pour |y| ≤ T = 16 (+24,08 dBFS) ; au-delà, sign(y)·(T + R·tanh((|y|−T)/R)·(1−1e−9)),
R = 16 ; borne asymptotique G = 32 (+30,10 dBFS) = niveau de la garde numérique. Ordre de l'étage de sortie : gain de
sortie → DC → **ELP** → **garde numérique** (écrêtage ±32, compteur distinct) → **limiteur utilisateur** (optionnel,
plafond réglable, compteur distinct) → Panic. Les trois mécanismes sont distincts et comptés séparément.

| ID | Critère (seuil fixé avant mesure) |
|---|---|
| ELP-01 | Fonction statique : identité **exacte** (==) sur 10⁶ points de [−16, 16] ; impaire exacte ; strictement croissante sur ]16, 10⁶] (grille 10⁵ points) ; f(y) < 32 pour toute entrée finie (y = 16+, 10³, 10⁶, 10³⁰⁰, DBL_MAX) |
| ELP-02 | Continuité au seuil : |f(16+ε) − (16+ε)| ≤ ε²/16 pour ε ∈ {1e−6 … 1e−1} ; pente à droite du seuil ∈ [1 − 1e−6, 1] ; 1-lipschitzienne : |f(a) − f(b)| ≤ |a − b| sur 10⁵ paires (non expansive : aucun saut créé) |
| ELP-03 | Borne asymptotique : f(10⁶) ≥ 31,99 et 20·log10(f(DBL_MAX)) ≤ 30,103 dBFS |
| ELP-04 | Bit à bit sous le seuil : 30 configurations aléatoires à crête de sortie dans ]1, 16[ (au-dessus de 0 dBFS, sous +24 dBFS) : moteur avec ELP et moteur sans ELP (contre-épreuve de test) **identiques bit à bit**, 0 engagement |
| ELP-05 | Configurations extrêmes et transitions rapides : 24 configurations (gains de sortie, de filtres et de destruction au maximum de leurs plages, 6 emplacements, 4 Fs) avec automation par sauts 0 ↔ max du gain de sortie à chaque bloc : 0 non-fini, crête < 32, garde 0, engagements ELP > 0 ; télémétrie : nombre d'engagements ≥ nombre d'échantillons de sortie |y| > 16 et ≤ nombre d'échantillons |y| ≥ 16 (sans limiteur ni Panic) |
| ELP-06 | Contre-épreuve (conditionnalité) : même protocole que DST-09 avec ELP désactivée : la garde numérique s'engage (compteur > 0) — démontre que le PASS de G-15/DST-09 dépend de SPEC-HR-01 ; avec ELP : garde 0 |
| ELP-07 | Distinction des mécanismes : (a) limiteur actif, plafond −6 dB, signal sous +24 dBFS : limiteur > 0, ELP 0, garde 0, crête ≤ plafond + 1e−6 ; (b) signal > +24 dBFS, limiteur actif : ELP > 0 **et** limiteur > 0, crête ≤ plafond ; (c) télémétrie : `elpHits`, `limiterHits`, `outputClampHits` publiés séparément et égaux aux compteurs internes |

Statut : un PASS ELP-0x ne valide pas SPEC-HR-01 ; il documente son comportement. G-15/DST-09 : FAIL historiques au
commit c741fb8 maintenus au registre ; PASS actuel **conditionnel** à l'approbation de SPEC-HR-01.

## 2. Sécurité des états autonomes (correction de la non-conformité DAW de CAP-01)

Séparation explicite des quatre catégories :

1. **Configuration sonore persistée** : paramètres (dont les 9 armements `flt.*.arm`, `dst.*.arm`, les gels
   `d2.freeze`, `s2.freeze`, `flt.*.freeze`), routes, matrice, banque… Restaurée **fidèlement** (G-21w inchangé).
2. **Contenu créatif R2** : captures D2/S2 (`capture` de l'état), conservées intactes à l'identique (SHA).
3. **État d'exécution** : tampons DSP, gels en cours, Drone actif ; jamais persistés en tant que tels.
4. **Autorisation locale d'activation autonome** : drapeau d'exécution de l'instance, **non persisté**, distinct de la
   configuration. Sans autorisation, le moteur applique les armements comme désarmés (armement effectif = configuré ET
   autorisé) et n'injecte aucune capture dans le DSP ; la configuration reste intacte (aucune modification d'état,
   aucun « dirty »). Drone conserve son mécanisme antérieur (`drone_restore`, préférence d'instance du projet).

Règles : instance neuve = autorisée. Restauration externe (projet DAW, duplication d'instance, preset) portant au moins
un armement ou une capture → autorisation retirée (« états autonomes suspendus ») jusqu'à l'action explicite
**Autoriser** de l'utilisateur. Panic → autorisation retirée si un armement est configuré (et Drone désarmé, P0-C06).
Undo/Redo internes à la session : autorisation inchangée. La sauvegarde DAW d'une instance suspendue conserve les
captures restaurées (aucune capture du tampon vide).

| ID | Critère |
|---|---|
| AUT-01 | Moteur, 9 armements × comportement autonome concerné (gels D2 et S2, gel modal des 3 filtres, N2) : autorisation retirée ⇒ rendu **bit à bit identique** au même état désarmé ; autorisation accordée ⇒ rendu identique au moteur historique armé (bit à bit) ; 0 allocation RT |
| AUT-02 | Restauration DAW (D2 gelé armé + capture, S2 gelé armé + capture) : (a) G-21w : état relu identique octet par octet ; (b) avant autorisation : énergie de sortie sans note < 1e−9 pendant 3 s, télémétrie « suspendu » ; (c) après **Autoriser** : gel rejoué sans entrée, contenu = capture (nouvelle capture identique à l'originale, même SHA) |
| AUT-03 | Duplication d'instance (état de A chargé dans B) : B suspendue, A inchangée (toujours autorisée, rendu inchangé) |
| AUT-04 | Preset externe armé et avec captures : armements remis à zéro (configuration), captures conservées mais non injectées, suspension signalée ; changement de preset pendant un gel autorisé : plus aucun comportement autonome |
| AUT-05 | Panic : autorisation retirée, configuration inchangée (aucune modification d'état), Drone désarmé ; puis **Autoriser** : gel restauré depuis la capture |
| AUT-06 | Fermeture/réouverture (2 cycles getState/setState d'une instance suspendue) : armements et captures identiques (SHA) à chaque cycle, toujours suspendue |
| AUT-07 | Compatibilité : états v1 (0.1.0-b5) et v2 (0.2.0) migrés sans changement de verdict des tests existants ; un état v2 armé restauré est suspendu ; Drone : `drone_restore=0` ⇒ Drone non restauré, `=1` ⇒ restauré (inchangé) |
| AUT-08 | UI : l'état « suspendu » est affiché en permanence (barre globale) avec l'action Autoriser ; navigation/ouverture de l'éditeur sans effet sur l'autorisation |

CAP-02w (G4.1) : son étape « gel rejoué sans entrée immédiatement après restauration » est **la non-conformité
corrigée** ; le critère est conservé dans son intention (sauvegarde puis restitution du gel) avec l'étape explicite
« Autoriser » insérée ; le résultat historique (run 33) reste consigné.

## 3. D2 Glitch Buffer — Feedback à Position = 0 (étude ; adoption soumise à arbitrage A-G41-06)

Constat (analyse du code) : le segment rejoué à Position = 0 est la dernière fenêtre de longueur Size écrite avant
l'ancrage ; la réinjection (Feedback) est écrite à la tête d'écriture, **hors** du segment en cours de lecture, et la
fenêtre suivante est toujours écrite hors boucle : la réinjection n'atteint pas l'auditeur à Position = 0 (sauf
recouvrement après un tour complet du tampon).

Sémantique candidate C1 « régénération dans la boucle » (option interne de test, **non exposée, non persistée, défaut
historique**) : pendant la lecture d'un segment, hors gel, l'échantillon lu est réécrit
`seg ← sat((1 − 0,5·fb)·seg + 0,95·fb·entrée)` ; fb = 0 ⇒ aucune écriture (bit à bit historique) ; gel ⇒ aucune
écriture ; lecture (vitesse, Reverse, Repeat, Crossfade) inchangée.

| ID | Critère |
|---|---|
| D2-01 | Historique : à Position = 0, sortie(fb = 1) − sortie(fb = 0) ≤ −60 dB (inaudibilité constatée, documentée) |
| D2-02 | Candidat : à Position = 0, différence fb = 1 vs fb = 0 ≥ −20 dB (audible) et croissante avec fb (0,25/0,5/1) |
| D2-03 | Non-régression : fb = 0, 30 configurations aléatoires D2 (taille, vitesse ±4, Repeat, Position, Crossfade, gel) : candidat ≡ historique bit à bit ; gel actif : tampon gelé inchangé bit à bit |
| D2-04 | Stabilité : fb = 1, entrée pleine échelle, Repeat 1, vitesses {−4, −1, 0,25, 1, 4}, 60 s : tous finis, crête ≤ 4 (borne de saturation), énergie de la boucle bornée (aucune croissance sur les 10 dernières secondes) |
| D2-05 | Déterminisme et transitions : deux rendus identiques bit à bit ; sauts par bloc de fb, Position, gel et vitesse : 0 non-fini, aucune allocation |

## 4. Transformations Z (R19)

R19 reste BLOCKED. Recherche ciblée : tous les commits du dépôt (toutes branches), documents du dépôt et documents
locaux hors dépôt. Si le catalogue reste introuvable : propositions **nouvelles**, explicitement marquées comme telles,
sans implémentation, en attente d'arbitrage (`docs/V1-ARCH-G4.2-Z-PROPOSALS.md`).

## 5. MIDI Learn (R47)

Politique (fixée avant test) : une CC → un paramètre ; apprendre un paramètre sur une nouvelle CC retire son ancienne
CC (un paramètre → une CC) ; réapprendre une CC remplace sa cible. CC réservées, jamais apprises : 0 et 32 (Bank
Select), 6, 38, 96–101 (Data Entry/RPN/NRPN), 64 (pédale de sustain, contrat P0), 120–127 (messages de mode, 120 =
Panic, 123 = All Notes Off). Cibles refusées : paramètres non automatisables et les 9 armements autonomes (actions
explicites, jamais une modulation). Profil utilisateur : un fichier global, hors presets et hors projets ; écriture
lecture-modification-écriture et relecture par les autres instances à modification du fichier.

| ID | Critère |
|---|---|
| ML-01w | Affectation : CC20 → `wt.x` ; valeur hôte suivie (0 → min, 127 → max, 64 → milieu ± 1 pas) après service du thread message |
| ML-02w | Remplacement et suppression : réapprendre CC20 → `wt.y` remplace ; apprendre `wt.y` sur CC21 retire CC20 ; suppression de CC21 : plus aucun effet |
| ML-03w | Persistance : profil écrit hors projet (chemin de test isolé), relu par une nouvelle instance ; lignes invalides/StableID inconnus ignorés sans échec |
| ML-04w | Séparation des presets et des projets : l'état DAW et le preset ne contiennent aucune affectation ; charger un preset ou un état ne modifie pas la carte |
| ML-05w | Multi-instance : A apprend CC22, B apprend CC23 ⇒ profil contenant les deux (aucune perte), carte de A actualisée après service |
| ML-06w | Limites et conflits : CC réservées et cibles refusées rejetées avec message, carte inchangée ; automation hôte puis CC puis hôte : dernière écriture gagnante, aucune boucle |
| ML-07w | Temps réel : traitement de 512 messages CC appris par bloc : 0 allocation dans processBlock ; aucun verrou dans le chemin audio (lecture de code : atomiques uniquement) |

Validation automatisée ≠ recette physique : contrôleur MIDI matériel et DAW réelle (recette R-11 du manuel) restent à
exécuter par le propriétaire.

## 6. Compilation locale macOS sécurisée

| ID | Critère (harnais `tools/local-build/test_build_local.sh`, commandes système simulées, Linux CI et local) |
|---|---|
| LB-01 | Sans `--juce-licence` ou valeur hors liste : refus (code 2), rien construit ; `--install` : refus (installation automatique supprimée) |
| LB-02 | Dépôt modifié (fichier suivi modifié ou fichier non suivi hors `local-build/`) : refus avant toute compilation ; commit inexistant : refus |
| LB-03 | Échec d'un test obligatoire (cœur, wrapper, critère FAIL hors liste d'échecs connus, sortie interrompue) : arrêt avant pluginval, paquets et livraison ; aucun dossier de livraison |
| LB-04 | pluginval obligatoire et bloquant (empreinte vérifiée) : échec ⇒ aucune livraison |
| LB-05 | Identité du bundle (identifiant, version, build, commit, arm64) et JUCE épinglé (tag + SHA, arbre propre) vérifiés ; incohérence ⇒ refus |
| LB-06 | Succès : livraison complète (Installer.pkg, Uninstaller.pkg, README_FR.txt, CHANGELOG.txt, SHA256SUMS.txt vérifié, ZIP versionné) ; aucune écriture dans `~/Library/Audio/Plug-Ins` ni `/Library/Audio/Plug-Ins` ; doublons utilisateur/système détectés et signalés sans modification |
| LB-07 | Suppressions bornées : toute suppression récursive est limitée à `local-build/` du dépôt (refus d'un chemin vide, `/`, `$HOME` ou hors périmètre) |

Le harnais vérifie le flux de contrôle du script ; la compilation réelle, pluginval et les paquets réels restent
vérifiés par la CI macOS arm64 (G-23, G-24, PKG) et, en recette, sur le Mac du propriétaire.
