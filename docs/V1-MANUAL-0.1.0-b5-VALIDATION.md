# SUBDESTRUCKT — Manuel utilisateur 0.1.0-b5 : rapport de validation

| | |
|---|---|
| Manuel | https://labtechno.github.io/SUBDESTRUCKT/manual/0.1.0-b5/ |
| Assistant de recette | https://labtechno.github.io/SUBDESTRUCKT/manual/0.1.0-b5/recette.html |
| Sources | branche `feature/arch-g3-user-manual` (`manual/`, `tests/manual/`, `tools/manual-audio/`) |
| Build documentée | 0.1.0-b5, source `186fb8644ae9518e3cb5116f4368b45dcea0d430` |
| Documentation de référence | `8d2334250994758f2182b18c3a47dd059cb68495` |
| Publication | `gh-pages`, dossier `manual/` uniquement ; plateforme S0.1 non modifiée |

## 1. Périmètre

Le manuel ne décrit que la build 0.1.0-b5. Aucune modification du DSP, du VST3, de l'installateur, du
désinstallateur ni du cahier des charges : `git diff 8d23342..feature/arch-g3-user-manual` ne touche que
`manual/`, `tests/manual/`, `tools/manual-audio/` et ce rapport.

## 2. Validation technique

### 2.1 Contrôles bloquants du générateur (`manual/build.py`)

| Contrôle | Résultat |
|---|---|
| 22 paramètres : plage, unité, défaut, lissage lus dans `source/core/Parameters.cpp` | PASS |
| `parameters.json` couvre exactement les 22 identifiants stables | PASS |
| Paramètres de chaque exemple audio dans les plages du registre ; commit moteur = commit de la build | PASS |
| R-01..R-15 complets, ordonnés, répartis dans les 4 groupes | PASS |
| ≥ 8 exercices (10) | PASS |
| Aucun marqueur résiduel, identifiants HTML uniques, ancres internes résolues, fichiers référencés présents | PASS |
| Aucune ressource externe (seul le lien de téléchargement direct du ZIP est autorisé, non chargé) | PASS |
| Aucune adresse électronique, aucun jeton | PASS |

### 2.2 Tests Playwright (`tests/manual/`)

Profils : Chromium bureau 1280×900 ; WebKit avec profil iPhone 13 ; Chromium en émulation iPhone 13.
**WebKit de Playwright sous Windows n'est pas Safari iOS : aucun test n'a été fait sur iPhone physique.**

Couverture : chargement sans erreur ; 16 chapitres et sommaire ; absence de requête externe ; un `h1` par
chapitre, textes alternatifs, SVG avec `title` et `desc` ; ancres ; sommaire et chapitre courant ;
précédent/suivant ; sommaire mobile ; recherche ; sections repliables ; agrandissement des figures ;
capture réelle (empreinte vérifiée), 15 repères dans l'image et légende ; 22 fiches de paramètres ;
glossaire (liens, infobulles accessibles, ordre alphabétique) ; thème clair/sombre persistant ; absence de
débordement horizontal ; cibles tactiles ≥ 44 px (profils iPhone) ; feuille d'impression ; 15 WAV conformes
au manifeste (SHA-256) ; aucune lecture automatique et aucun téléchargement audio avant un clic ; lecture
exclusive et bascule A/B au même instant ; simulations dessinées et réactives ; assistant : état initial
PENDING sans préremplissage, clé de stockage distincte de S0.1 (donnée S0.1 préservée même après
réinitialisation), FAIL toujours visible, progression distincte de la validation, N/A bloquant sans
justification, persistance après rechargement, correction, validation finale avec confirmation,
verrouillage et réouverture, réinitialisation avec confirmation, export `SUBDESTRUCKT_ARCH_G3_RESULTS.json`
conforme au schéma, copie presse-papiers, repli quand le stockage est indisponible ; index des versions ;
non-régression S0.1 (22 fichiers identiques octet pour octet, page sans erreur).

| Cible | Résultat |
|---|---|
| Serveur local (`manual/dist`, avec requêtes Range) | 48 PASS, 2 ignorés, 0 FAIL |
| URL publiée | 48 PASS, 2 ignorés, 0 FAIL (gh-pages `5660c84`) |

Tests ignorés : `test_mobile_toc_toggle` et `test_touch_targets_44px_all` sur le profil bureau (sans objet).

### 2.3 Défauts trouvés par les tests et corrigés avant publication

1. Débordement horizontal sur mobile (753 px pour 390) : `white-space: nowrap` appliqué aux boutons audio.
2. Lien-bouton « Ouvrir l'assistant » à 38 px de haut : `a.btn` passé en `inline-flex`.
3. Bascule A/B qui repartait au début : positionnement différé à `loadedmetadata` (chargement audio à la demande).
4. Repères de la capture placés en pourcentage de la figure entière (légende comprise) : enveloppe dédiée à l'image.
5. Barre supérieure débordante et semi-transparente sur iPhone ; synthèse de recette collante masquant le test ciblé.
6. 44 lecteurs préchargeant les WAV à l'ouverture : préchargement supprimé (données mobiles).

## 3. Validation éditoriale

Chaque affirmation technique a été rapprochée du code ou des documents de la build :

| Sujet | Source vérifiée |
|---|---|
| 22 paramètres, plages, défauts, lissage 20 ms | `source/core/Parameters.cpp`, `Engine.cpp` (`kSmoothingSeconds`) |
| ADSR linéaire, temps pleine échelle, relance depuis le niveau courant | `source/dsp/Envelope.h` |
| Mono, priorité dernière note, retour sans relance d'enveloppe, CC120/CC123 | `Engine.cpp` (`noteOn`, `noteOff`, `handleEvent`), `plugin/` |
| Vélocité sur le volume uniquement ; formule du gain | `Engine.cpp` |
| PM, borne ±32 rad, garde de sortie +30 dBFS, garde DC 5 Hz, Panic 5 ms | `Engine.cpp` |
| Effective = profondeur × Master × direction ; Master sans effet sur les routes FM | `Engine.cpp` |
| Classes I/D, auto-boucle toujours D, refus d'un état incohérent | `source/core/Topology.cpp`, `StateCodec.cpp` |
| Libellés de l'interface (Eff, g, Transaction appliquee, Etat refuse…) | `ui/DiagnosticEditor.cpp`, `plugin/` |
| Décimation demi-bande FIR (Kaiser ~100 dB), latence 32 éch. à 44,1 kHz | `source/dsp/Decimator.h`, capture réelle |
| Messages et comportements de l'installateur et du désinstallateur | `packaging/distribution/*.xml.in`, `packaging/scripts/`, `packaging/resources/` |
| Statuts de test G-01..G-24, CI macOS 15.8.2 | `docs/V1-ARCH-G3-REPORT.md` |
| Critères officiels R-01..R-15 | `docs/V1-ARCH-G3-RECETTE-M4MAX.md` |

Corrections éditoriales faites pendant la relecture : nombre de critères automatisés non sourcé remplacé par
« G-01 à G-24 PASS » ; nombre de coefficients du filtre de décimation non sourcé remplacé par la description
réelle ; libellés d'interface alignés sur les chaînes exactes ; comportement en cas d'état refusé corrigé
(l'état courant est conservé).

Règles respectées : FM et PM distingués (le calcul réel est une PM) ; aucune fonction future présentée comme
disponible (tableau « disponible / prévu ») ; pas de mode Drone ni de preset inventé ; simulations marquées
« Simulation pédagogique », schémas marqués « Schéma pédagogique », mesures marquées « Mesure », capture
marquée « Capture réelle » ; aucune absence de clic garantie ; aucune compatibilité Reason ni certification
matérielle affirmée.

## 4. Informations non confirmables (reprises au chapitre 16)

1. Menus exacts de Reason (plugins VST3, entrée MIDI, audio, PDC) : documentation officielle non lisible de façon fiable.
2. Compatibilité Reason et compensation de latence dans Reason : dépendent de R-03 à R-12.
3. macOS 14.0 : minimum déclaré, jamais testé.
4. Performances processeur sur M3, M4, M4 Max : non mesurées.
5. Identité échantillon par échantillon entre les rendus Windows/MSVC du manuel et le plugin macOS : non vérifiée.
6. Perception des clics de transition en jeu réel : non évaluée.
7. Comportement de l'assistant sur Safari iOS physique (partage natif, téléchargement, presse-papiers) : non testé.

## 5. Limitations du manuel

- Conformité WCAG 2.2 AA visée, non revendiquée : aucun audit avec lecteur d'écran (VoiceOver) n'a été fait.
- Repères de la capture à 18 px sur petit écran (en dessous de 24 px), au titre de l'exception « contrôle
  équivalent » : la légende numérotée sous l'image donne la même information.
- Le partage natif (`navigator.share`) n'existe que sur les navigateurs qui le proposent ; sinon export ou copie.
- Les réponses de recette restent dans le navigateur de l'appareil : elles sont perdues en navigation privée ou
  après effacement des données du site.
