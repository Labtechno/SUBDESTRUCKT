# SUBDESTRUCKT — ARCH-G4.1 — Rapport final

Branche : `feature/arch-g4-full-development` (aucune autre branche, aucun merge vers `main`).
Reprise : `c741fb8` (fin ARCH-G4). Commit de ce rapport : voir `git log` (SHA publié en §10).
Édition du manuel : 0.3.0. Version produit : 0.3.0 (build interne, **aucun binaire diffusé**).
Suivi détaillé : `docs/V1-ARCH-G4-TRACKER.md` (critères §3bis enregistrés avant mesure, matrice §4, journal §5).

## 1. Synthèse

| Indicateur | Valeur | Calcul |
|---|---|---|
| **Avancement fonctionnel** | **92,0 %** | (43 PASS + 0,5 × 6 IMPLEMENTED) / **50** exigences, BLOCKED compté 0 (règle §14) |
| Taux de validation | 86,0 % | 43 exigences PASS (critère exécuté et réussi en CI macOS arm64) / 50 |
| Avancement hors exigence bloquée (pour comparaison ARCH-G4) | 93,9 % | (43 + 3) / 49 (ARCH-G4 : 85,7 % selon cette règle) |
| **Prêt pour une diffusion publique** | **Non** | licence JUCE non choisie (D-LIC-2), pas de Developer ID/notarisation, aucune recette DAW ni Mac physique |
| Prêt pour une recette privée | Oui, par compilation locale | `tools/local-build/build_local_macos.sh` (§9), après choix D-LIC-1 par l'utilisateur |

Répartition : 43 PASS, 6 IMPLEMENTED (R42, R43, R44, R45, R47 : UI/MIDI Learn sans mesure d'usage ;
R50 : une clause de critère non conforme), 1 BLOCKED (R19, cinq transformations Z non retrouvées, non inventées),
0 TODO, 0 FAIL.

## 2. Matrice R01–R50

| # | Exigence | Statut | Preuve |
|---|---|---|---|
| R01 | Registre stable extensible | PASS | REG-ID (695 StableID figés, 22 v1 identiques à l'oracle) |
| R02 | État versionné + migration | PASS | ST-01/02/03 (format 3 ; v1/v2 migrés ; version future refusée) |
| R03 | Transactions hors RT | PASS | G-01..G-04, RT-G4 (0 allocation) |
| R04 | Timeline MIDI sample-accurate | PASS | MIDI-01..04 |
| R05 | Mono/Legato, priorités, sustain | PASS | MIDI-0x, ENV-02 |
| R06 | Expressivité MIDI | PASS | MIDI-0x |
| R07 | Accordage A4, microtuning | PASS | MT-01 (+50 ct exact à 0,1 ct ; A4 432) |
| R08 | 5 enveloppes DAHDSR, loop, **Trigger, Sync** | PASS | ENV-01, ENV-02, ENV-03 |
| R09 | 4 LFO | PASS | LFO-01 |
| R10 | Tail/Drone/Panic | PASS | CR/SYNC/DRN/Q |
| R11 | WT banques XY 64×16, bornes, Linear/Stepped | PASS | WT-01/02/03 |
| R12 | **WT morph Spectral** | PASS | WT-SM-01 |
| R13 | Interpolation, Clean/Raw | PASS | WT-0x |
| R14 | Phase, sync | PASS | WT-0x, SYNC |
| R15 | WT Character | PASS | CHR-01 |
| R16 | **Génération procédurale, import/export, format portable, édition COW** | PASS | WTA-01, WT-ED-01, WT-ED-02w |
| R17 | Z deux slots | PASS | WT-0x |
| R18 | Z : 3 transformations | PASS | WT-0x |
| R19 | Z : 5 transformations historiques | **BLOCKED** | catalogue non retrouvé (B-01) ; non inventées |
| R20–R22 | FM 4 opérateurs, formes, niveaux/Character | PASS | FM-01..04, CHR-01 |
| R23 | Matrice 4×4 + 10 topologies | PASS | TOPO-01 (45/45 rendus distincts, CROSS conservé) |
| R24 | ExpFM benchmark | PASS | PERF-02 |
| R25–R26 | CROSS B-T2, Cross Audio, Cross Reset | PASS | G-0x, CR |
| R27–R29 | Destruction N1..S2, 6 slots, latence | PASS | DST-01..10, N1-02 |
| R30–R32 | Filtres F1..F4, Pre/Post, excitation, stéréo | PASS | FLT-01..07 |
| R33 | Matrice 64, Via, macros, inactivité | PASS | MOD-01, MACRO-01 |
| R34 | RNG, chaos | PASS | RNG-01, CHAOS-01 |
| R35 | Suiveurs ×3, Y2 | PASS | FOL-01 (après correctif), DST-07 |
| R36–R38 | Presets, Randomize/Mutate/Evolve, Undo | PASS | PRE-01, ST-RND-0x, EVO-01, UNDO-01 |
| R39 | Quality Eco/Normal/High | PASS | Q, N1-02, PERF-03 |
| R40 | Limiteur, garde numérique | PASS | G-15c, HR-01 |
| R41 | Benchmark CPU reproductible | PASS | PERF-01/02/03 (sans seuil) |
| R42 | UI Performance | IMPLEMENTED | balance ajoutée ; tâches d'ergonomie non mesurées |
| R43 | UI Sound Design | IMPLEMENTED | Capturer gels, Trigger/Sync ; tâches non mesurées |
| R44 | UI Engineering | IMPLEMENTED | éditeur de formes ; tâches non mesurées |
| R45 | Inspector, moniteurs, zoom, accessibilité | IMPLEMENTED | UI-01w PASS ; contrastes/DPI/zoom par niveau non mesurés |
| R46 | Wrapper VST3 (695 paramètres, état v3) | PASS | G-21w, G-22w, hôte, pluginval rigueur 10 |
| R47 | MIDI Learn global | IMPLEMENTED | pas de test automatisé (écrit dans le profil utilisateur) ; recette R-11 |
| R48 | Packaging 0.3.0 testé sans diffusion | PASS | CI : Installer/Uninstaller, bac à sable, recette VM ; non publiés |
| R49 | Manuel HTML + recette | PASS | manuel 0.3.0 : contrôles bloquants, Playwright (local + CI ubuntu) |
| R50 | Captures Freeze/Glitch R2 | IMPLEMENTED | CAP-01, CAP-02w PASS ; **clause DAW de CAP-01 non conforme** (§5) |

## 3. Fonctionnalités réalisées (ARCH-G4.1)

- **§3 Wavetable** : morph Spectral par axe (spectres préparés hors RT, resynthèse iFFT toutes les 128 échantillons,
  fondu, limitation de bande) ; générateur procédural (5 procédures, grille bornée 64×16, déterministe) ; éditeur de
  formes dessinées (64 points) ; bibliothèque `.sdwt` portable (SHA-256 vérifié) ; remplacement de forme par copie
  (banques immuables) ; préparation hors RT ; UI Engineering › Banques. Les 3 transformations Z existantes sont
  conservées et testées ; les 5 manquantes ne sont pas inventées.
- **§4 Captures R2** : D2 (segment audio) et S2 (spectre figé) capturés à une frontière de bloc dans des tampons
  préalloués ; section d'état `capture` (base64, SHA-256, validation) ; restauration différée jusqu'à ce que
  l'emplacement exécute l'algorithme (pas de continuation exacte R3) ; sauvegarde DAW autonome (capture des gels
  armés à `getStateInformation`, attente bornée 50 ms) ; un preset externe est désarmé au chargement et le signale.
- **§5 Gains cumulés** : analyse des étages, maintien des plages artistiques (+24 dB / +36 dB inchangés) ;
  spécification séparée **SPEC-HR-01 (proposée)** enregistrée avant mesure : protection d'extrême niveau au-delà de
  +24 dBFS, inactive en dessous (HR-01 : 0 engagement à gains ≤ 0 dB), comptée et affichée (ELP). Pas de
  normalisation permanente. G-15 et DST-09 : FAIL historiques au commit `c741fb8` **conservés** ; PASS au commit
  courant **sous SPEC-HR-01 seulement**.
- **§6 N1** : hybride ADAA1 + suréchantillonnage interne par Quality (Eco ×1, Normal ×2, High ×4), filtres
  demi-bande, latence déclarée = mesurée (27 / 41 échantillons), chemin sec aligné. Choix retenu sur mesure (§6).
- **§7 Balance WT/FM** : gains audio post-Amp avant chaînes locales ; centre = 0.2.0 bit à bit ; extrêmes isolent
  un moteur bit à bit ; aucune amplification ; modulations, topologie et phases inchangées. Conflit documenté :
  Cross Audio Crossfade (post-chaînes) reste un réglage distinct ; les deux se cumulent (A-G41-05).
- **§8 Enveloppes** : Trigger Voice (défaut, comportement 0.2.0) / Note / Gate, relance depuis le niveau courant ;
  Tempo Sync optionnel (durées en noires, BPM suivi au bloc suivant) ; défauts des anciens presets préservés
  (état v2 migré : Voice, Sync off). Règles provisoires (B-03).
- **§9 Tests** : AUD-01 (695 paramètres), MT-01, FOL-01, CHR-01, TOPO-01, MACRO-01, UI-01w, CAP-02w, WT-ED-02w.
  **Deux défauts réels trouvés et corrigés par AUD-01** : temps des suiveurs d'enveloppe jamais appliqués ;
  modulation de Output Gain / Ceiling ignorée.
- **§10 Performance** : PERF-03 (Eco/Normal/High séparés, coût marginal par composant) ; optimisations démontrées.
- **§12 UI et manuel** : balance, Trigger/Sync, Capturer gels, indicateur ELP, éditeur de formes ; manuel 0.3.0.
- **§13 Licence** : étude sur sources officielles (`docs/V1-ARCH-G4.1-LICENCE.md`) et script de compilation locale.

## 4. Résultats CI

Voir §10 pour le run final. Historique ARCH-G4.1 : runs 16 à 25 en échec à la **barrière** seulement (aucun test
en échec) : la liste des échecs connus devenue vide faisait échouer `grep -v` sous `pipefail` ; cause trouvée sans
accès aux journaux (refus d'accès authentifié respecté), corrigée en `f1183b2` (run 26 : job macOS complet vert).
Job `manual-playwright` (ubuntu) ajouté : voir §5.

## 5. Échecs, non-conformités et limitations (aucun masqué)

| Élément | Statut | Détail |
|---|---|---|
| CAP-01, clause DAW | **NON CONFORME** | Le critère préenregistré demandait de ne restaurer les armements DAW qu'avec l'autorisation d'instance. Mis en œuvre puis retiré : il rendait G-21w (critère antérieur : état DAW identique après rechargement) en échec. Restauration DAW fidèle conservée ; presets toujours désarmés. Arbitrage A-G41-03. |
| G-15 étendu, DST-09 | FAIL historique / PASS conditionnel | FAIL à `c741fb8` (garde +30 dBFS déclenchée) ; PASS actuel dû à SPEC-HR-01 proposée (754 915 engagements ELP rapportés par HR-02). Si SPEC-HR-01 est refusée : FAIL. |
| Manuel 0.2.0 (publié) | défaut connu | `store.js` réutilise la clé de stockage de la recette 0.1.0-b5 (les deux éditions partagent l'enregistrement local). Corrigé à partir de 0.3.0 ; 0.2.0 non modifiée (préservation) ; test marqué xfail avec la raison. |
| Manuels 0.1.0-b5 et 0.2.0 (publiés) | défaut connu | sous WebKit Linux (profil iPhone, CI), les WAV sont téléchargés avant tout clic malgré `preload=none` (test `no_audio_download_before_click` en échec, marqué xfail avec la raison). Corrigé en 0.3.0 (source affectée au premier clic). Non vérifié sur Safari iOS physique. |
| D2 Feedback | limitation | inaudible avec Position = 0 par construction (le segment rejoué ne provient jamais d'une zone enregistrée en boucle) ; non modifié (sémantique non arbitrée, A-G41-06). |
| Couverture UI visible | mesure | 203/695 paramètres atteignables au contexte par défaut (affichage contextuel : algorithme choisi, opérateur sélectionné) ; tous les contrôles de paramètres ont un titre accessible. |
| Captures R2 | limitation | position de lecture et feedback en cours non restaurés (pas de continuation R3). |
| Projets 0.2.0 | changements de son volontaires | N1 en Normal/High (Eco = N1 0.2.0), suiveurs, modulation de sortie. Documentés (CHANGELOG, manuel). |
| R19 | BLOCKED | 5 transformations Z non retrouvées. |

## 6. Mesures DSP (local MSVC ; CI macOS arm64 pour la validation)

- **N1-02** (3,1 kHz, drive 24, fold 1, 48 kHz) : énergie repliée Eco/ADAA1 −2,5 dB, Normal ×2 −38,5 dB,
  High ×4 −54,1 dB (critères : ≥ 3 et ≥ 6 dB de mieux) ; cas modéré (1 kHz) −92,5 / −92,4 / −92,4 dB ; latence
  27 / 41 échantillons, déclarée = mesurée ; dry/wet aligné. Rendu audio du manuel (sinus 3,1 kHz) : énergie hors
  harmoniques −24,4 dB (Eco) contre −106,4 dB (High). Caractère agressif conservé (THD DST-01 inchangé en Eco).
- **WT-SM-01** : point de grille −144,9 dB ; formes en opposition de phase au milieu : RMS 0,337 (spectral) contre
  0,0000 (linéaire).
- **BAL-01** : centre et extrêmes bit à bit. **ENV-02/03** : 24000 / 32000 échantillons à 120 / 90 bpm ; BPM
  120 → 60 à mi-attaque : fin à 35968 (36000 ± 128), pas maximal 0,00133.
- **HR-01** : 0 engagement ELP sur 50 configurations à gains ≤ 0 dB ; **HR-02** : garde 0, non-finis 0.
- **MT-01** : 452,893 Hz (+50 ct) ; **FOL-01** : rapport 2,000 pour un niveau double ; **CAP-01** : D2/S2 restaurés
  à 0,0 dB de l'original, 0 allocation RT. **AUD-01** : 686 paramètres à effet mesuré, 9 exceptions (armements),
  0 sans effet. **REG-01** : 20/20 rendus identiques bit à bit à l'oracle 0.1.0-b5.

## 7. Performances (PC de développement Windows ; **aucune certification M3 ni M4 Max**)

Facteur temps réel mono-instance, 48 kHz (PERF-03) : Eco 0,013 (init) / 0,086 (stress) ; Normal 0,013 / 0,095 ;
High 0,029 / 0,239. Coût marginal (millièmes du temps réel, Normal) : morph spectral stationnaire ≈ 0, modulé par
LFO +6,8, N1 +18,9, S2 4096 +17,4, F4 16 modes +2,4, 16 modulations audio +4,2, ExpFM 4 opérateurs +0,2.

Optimisations démontrées (résultats identiques) :
- filtres demi-bande de N1 : prises nulles sautées (bit à bit identique) : Normal 567 → 444 ns/éch. (−22 %),
  High 1219 → 912 ns/éch. (−25 %) ;
- morph spectral : resynthèse sautée si position, poids et bande sont inchangés : coût stationnaire +11 → ≈ 0.
Mesures M3/M4 : à produire sur la machine cible par le script de compilation locale (rapport `perf_this_machine.txt`).

## 8. Arbitrages consolidés (liste unique)

| ID | Décision attendue | Recommandation |
|---|---|---|
| D-LIC-1 | Licence JUCE pour la compilation locale privée (AGPLv3 ou Starter, gratuites) | choix de l'utilisateur ; le script l'exige |
| D-LIC-2 | Diffusion publique : licence (Starter ≤ 20 kUSD/12 mois, Indie, Pro, ou AGPLv3 + sources), Developer ID, notarisation | aucune dépense engagée |
| SPEC-HR-01 | Adopter la protection d'extrême niveau (> +24 dBFS) ou revenir à la garde seule (G-15/DST-09 FAIL) | adopter (inactive en dessous, signalée) |
| A-G41-03 | Armements à la restauration DAW : fidèle (actuel) ou soumis à autorisation (critère CAP-01) | fidèle, cohérent avec G-21w |
| B-03 | Règles Trigger/Sync des enveloppes (provisoires) | confirmer Voice/Note/Gate et noires |
| A-G41-05 | Balance WT/FM et Cross Audio Crossfade cumulés | conserver deux réglages distincts |
| A-G41-06 | D2 Feedback à Position 0 | décider d'une sémantique (ex. réécriture du segment) ou documenter |
| B-01 | Cinq transformations Z | fournir le catalogue historique |

## 9. Solution de livraison pour la prochaine recette

Aucun binaire publié. L'utilisateur compile sur le Mac Studio M4 Max :
`tools/local-build/build_local_macos.sh --juce-licence=<choix> --install --pluginval [--packages]`
(JUCE épinglé vérifié, tests exécutés sur la machine, signature ad hoc locale, installation utilisateur, paquets et
ZIP locaux si demandé). Coût nul. Le manuel 0.3.0 (chapitre 2) décrit la procédure ; la recette comporte 24 tests.

## 10. Références

- Commits ARCH-G4.1 : `efeee24` … (voir `git log c741fb8..HEAD`). SHA final et run CI : `4b17df5` ; CI run 33 (45932784500) **SUCCESS complet** : build-test-package (cœur, wrapper, barrière sans échec connu, paquets 0.3.0, hôte, pluginval 1.0.4 rigueur 10 à 4 Fs et 5 tailles de bloc, bac à sable, recette VM), RTSan, manual-playwright (0.3.0 : 46 PASS ; 0.2.0 : 44 PASS + 2 xfail ; 0.1.0-b5 : 47 PASS + 1 xfail, défauts documentés §5)
- Manuel 0.3.0 : https://labtechno.github.io/SUBDESTRUCKT/manual/0.3.0/ (gh-pages `3a75990` ; vérifié en ligne : 46 PASS Playwright) ; éditions 0.1.0-b5, 0.2.0 et S0.1 inchangées
- « Hooks de sécurité » : aucun hook Git dans le dépôt ni dans la configuration Claude Code ; le contrôle de sûreté
  intégré a bloqué une suppression `rm -f` et un téléchargement authentifié de journaux CI (refus respectés).
- Node.js : actions `checkout@v5`, `upload-artifact@v6`, `setup-python@v6` (Node 24) ; plus d'avertissement Node 20
  attendu.
- Engagements tenus : un agent, une branche, aucun merge, aucune dépense, aucune licence choisie, aucun binaire
  publié, aucun critère abaissé (les FAIL historiques restent consignés), aucun test supprimé.
