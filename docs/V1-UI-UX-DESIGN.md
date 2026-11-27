# SUBDESTRUCKT — UI/UX Design V1
Révision fonctionnelle 0.2 — 2026-11-24 (historique). Actualisation 2026-11-27 : voir [ARCH-UI-03](V1-ARCH-UI-03-SPEC.md), prioritaire pour les choix visuels, le Symbiotic Core et l'exhaustivité. Les mentions CRT/vintage, interdiction d'animation permanente et budget hardware ci-dessous sont supersédées dans cette portée; les autres exigences et parcours restent applicables.
Référence : [V1-DESIGN-BASELINE.md](V1-DESIGN-BASELINE.md). Ce chapitre actualise la proposition UI/UX de la section 12 du référentiel, sans modifier les autres décisions. Aucun développement ni test exécuté.

## 1. Décisions utilisateur validées
| ID | Choix | Décision |
|---|---|---|
| UX-D01 | D | Identité hybride industrielle et laboratoire expérimental |
| UX-D02 | C | Commandes aspect hardware et lisibilité moderne |
| UX-D03 | B | Trois niveaux Performance / Sound Design / Engineering |
| UX-D04 | C | Afficheurs hybrides selon fonction |
| UX-D05 | B | Couleurs distinctes WT, FM et modulation |
| UX-D06 | C | Noyau Performance fixe + quatre macros assignables |
| UX-D07 | B | Destination Inspector intégré et rétractable |
| UX-D08 | B | Oscilloscope, spectre, espace XY et réseau FM |
| UX-D09 | B | Redimensionnement et zoom global |
| UX-D10 | B | Performance directement transposable au futur hardware |

Les spécifications ci-dessous formalisaient initialement des propositions de mise en œuvre. Les huit arbitrages complémentaires UI/UX ont été validés le 2026-11-24 et sont consignés à la section 10. Le périmètre sonore V1 reste gelé; aucune nouvelle fonction DSP n'est implicitement autorisée. Les dix arbitrages complémentaires Preset & State sont également désormais validés dans le référentiel général.

## 2. Direction artistique
Nom de travail Industrial Research Instrument. Châssis graphite, sérigraphie claire, relief discret, instrumentation scientifique, afficheurs CRT/OLED stylisés quand pertinents, graphiques modernes pour données complexes. Pas de rouille décorative, néons omniprésents, faux vieillissement ni animation permanente. Sans-serif technique, chiffres tabulaires/monospace, hiérarchie claire; licences des polices et assets à vérifier.
Palette candidate : chassis #121617, panels #242A2D, text #E1E5E6, WT #79C4AC, FM #D98439, CROSS/mod #7899CB, display #080E0C, controls #363D40, alert #C65D54. Couleurs sémantiques constantes, texte/icône en complément; rouge réservé à sécurité/erreur. Contraste à mesurer, cible 4.5:1 texte normal, 3:1 éléments essentiels; effets CRT ne dégradent pas les chiffres. Logo/typographie exacte et palette définitive à valider.
Contrôles : knob continu avec valeur et unité, édition numérique, drag vertical, réglage fin, reset explicite, focus clavier, états actif/bypass/incompatible distincts. Afficher base et valeur effective sans écrire cette dernière dans automation. Un seul composant de contrôle et un registre de paramètres partagé entre vues; aucune duplication d'état. Labels stables, pas de couleur seule.

## 3. Architecture commune
Trois espaces de travail, un état sonore. Changer de vue, ouvrir/fermer Inspector, redimensionner, zoomer, changer de moniteur ne change pas le DSP, ses états, le routage ni les valeurs de base. Préférences UI locales séparées des presets. Barre persistante : preset, état modifié, save, navigation, sortie/mètre, Panic; Undo/Redo selon contrat State restant à finaliser. Drone/Freeze actifs ou armés explicitement identifiés, pas d'activation automatique par navigation. Conserver sélection et contexte; Inspector rétractable. Pas de fenêtres modales pour édition courante; erreurs et conflits explicites. Raccourcis ne capturent pas abusivement les commandes de l'hôte.
### Performance
Objectif jouer et transformer un preset en moins de 30 s (cible test, non résultat).
Noyau fixe et 4 macros. Les deux emplacements de contrôle FM de la façade Performance sont VALIDÉS comme commandes contextuelles reposant sur des paramètres et mécanismes DSP déjà existants, sans ajout de nouveau moteur, nouvelle macro globale cachée ni nouvelle connexion implicite. Les libellés fonctionnels FM INDEX et FM FEEDBACK restent des candidats d'interface à confirmer par prototype selon les paramètres existants qu'ils pilotent réellement. La façade candidate conserve X/Y/Z, deux contrôles FM contextuels, CROSS, WT/FM balance, M1–M4 et OUTPUT. Les Character WT/FM restent accessibles en Sound Design et assignables aux macros. Les quatre macros affichent leurs noms propres au preset et leurs affectations; celles-ci comptent dans 64 connexions globales. Ne pas confondre WT/FM balance et niveaux indépendants ou modes Cross Audio : sémantique à fixer avant validation. Moniteur commutable scope/spectre/XY/FM; données réelles uniquement. Panic toujours visible, preset et état dirty visibles. Drone, Freeze et sécurité lisibles. Contrôles événementiels distincts du budget des encodeurs; budget matériel total 8–12 à réconcilier.
### Sound Design
Deux panneaux équivalents WT et FM; **CROSS au centre fonctionnel et perceptif de l'instrument**. L'interface doit faire comprendre en quelques secondes que SUBDESTRUCKT est construit autour de l'interaction entre WT et FM, et non qu'il s'agit de deux moteurs indépendants auxquels on aurait ajouté un routage secondaire. Les traitements locaux WT/FM puis Cross Audio puis traitement global suivent le contrat audio. Ne pas représenter l'ordre Filter/Destruction comme fixe : Pre/Post autorisé. Un filtre + deux slots destruction par emplacement. Vue module : algo, bypass, valeurs clés, Dry/Wet, entrée/sortie et état; paramètres détaillés dans Inspector. WT : banque, X/Y/Z, deux transformations, Character, phase/pitch/sync. FM : quatre opérateurs, mode modulation par destination, waveform, ratio/fixed/offset, modulation level distinct du gain/pan audio, feedback et Character. Cross : master, directions, bus, Sync, Cross Audio. Enveloppes 5, LFO 4, random/chaos accessibles sans créer générateurs supplémentaires. Afficher les chaînes et accès direct à matrice concernée; retour conserve sélection. Proposer onglets internes WT/FM/CROSS/PROCESS/MOD sur petite fenêtre plutôt que réduire texte.
### Engineering
Espaces spécialisés FM Network (4x4 interne + accès aux 4 WT->FM via Cross), Cross Network (39 connexions), Global Modulation (64 dont macros), Curve Editor commun Z/mod, Bank Laboratory, State/Assets, Randomizer, Diagnostics. List/Inspector/Overview du même modèle; pas de patcher libre, ni affichage obligatoire de 119 liens simultanés. Source/destination, profondeur bipolaire, mode/rate, Via, enable/lock, incompatibilité et raison; inspecter causalité sans exposer d'options de retard non validées. Édition du routage avec validation, refus sans modification partielle. Overview VALIDÉ comme vue de compréhension, filtrage et navigation; l'édition détaillée reste dans Routing List et Inspector. Pas de patcher libre. Banques immuables COW, prévisualisation et préparation hors audio; Randomizer scope/locks/intensity/seed et transaction. Fonctions Evolve/Undo détaillées selon futur arbitrage State. Diagnostics CPU, SR, qualité, latence, capacités et protections; valeurs mesurées, pas inventées.

## 4. Contrat Inspector
Clic/sélection d'un paramètre ouvre ou met à jour le panneau intégré et rétractable selon préférence; l'épinglage de son contexte est VALIDÉ comme comportement UI; ne pas déplacer automatiquement le focus pendant automation. Affiche nom, chemin, unité, base, effective ou plage/mesure clairement indiquée, limites, sources, contributions, profondeur, polarité, courbe, Via, rate, état et raison d'inactivité. Édition/assignation directe écrit la même matrice; capacité, cycles et compatibilité contrôlés. Navigation source/destination et destinations de macro; épinglage candidat. Enveloppe ou LFO accessible via lien, pas d'instance par paramètre. Pas de mise à jour audio->UI à chaque échantillon; télémétrie décimée, non bloquante, sans allocation ni verrou sur traitement audio. Valeurs rapides affichées comme échantillon/étendue, non comme exacte valeur instantanée continue.

## 5. Visualisations
Scope et spectre : point de prélèvement explicitement indiqué, sortie par défaut proposée, option WT/FM si validée; échelles et unités, clipping distinct de saturation intentionnelle. XY : carte de banque, position de base/effective, limites et morphing; pas fausse perspective 3D assimilant Z à une dimension stockée. FM : graphe réel, directions, niveaux et feedback, retard selon contrat S0; pas de lien décoratif. Ne pas ajouter de visualisation chaotique avancée en V1. Mesures et dessin hors thread audio; transfert borné, cadence candidate 30 Hz (60 si justifié), réduire cadence lorsque masqué, désactiver analyse coûteuse en vue cachée. Aucune modification DSP due au moniteur. Visualisation purement conceptuelle dans wireframes, étiquetée.

## 6. Redimensionnement et accessibilité
Principe VALIDÉ : layout adaptatif, redimensionnement et zoom global. Les valeurs 1440x900 de référence, 960x640 minimum et 75–150% restent des candidats à tester; ne pas promettre compatibilité de toutes combinaisons taille/zoom, adapter layout, panneau Inspector rétractable et scroll local. Haute densité DPI, navigation clavier/focus visible, labels accessibles, valeurs numériques saisissables, infobulles non indispensables, contrastes et daltonisme, pas d'information couleur seule. Pas de réorganisation libre de panneaux. Tester le **VST3 sur macOS Apple Silicon natif uniquement pour la V1**, avec changement DPI/échelle, réouverture et persistance UI; aucun seuil validé à ce stade. Windows est hors périmètre. Linux n'est pas une cible V1 engagée.

## 7. Hardware
La couche Performance reste une façade compacte et immédiatement jouable, mais **aucune contrainte de transposition vers un hardware n'est active dans la phase VSTi actuelle**. L'inspiration hardware demeure purement ergonomique et visuelle. Le logiciel peut exploiter pleinement l'écran, la souris, le clavier et les vues Sound Design / Engineering sans budget physique de 8–12 commandes. Toute reprise d'un projet hardware nécessitera un chapitre séparé et ne modifiera pas rétroactivement les exigences VSTi.

## 8. Parcours et recette proposés
| ID | Tâche / condition GO candidate |
|---|---|
| UX01 | Charger preset et modifier son caractère en <=30 s, sans ouvrir Engineering |
| UX02 | Modifier X/Y/Z et retrouver valeurs dans Sound Design; aucune divergence |
| UX03 | Accéder aux réglages FM et Cross depuis Performance en <=2 actions |
| UX04 | Identifier une modulation de cutoff, sa source et sa profondeur en <=3 actions |
| UX05 | Assigner macro à deux destinations et vérifier que deux liens comptent dans 64 |
| UX06 | Distinguer gain audio, mute, disable et modulation d'opérateur |
| UX07 | Modifier routage, détecter cycle invalide et conserver état précédent |
| UX08 | Vérifier changement de vue/zoom/Inspector/monitor sans changement sonore ni paramètre |
| UX09 | Panic accessible en une action depuis tous les espaces, même avec panneau ouvert |
| UX10 | Drone/Freeze et états autonomes affichés, aucun armement par navigation |
| UX11 | Affichage réel des graphes et points de mesure, cohérent avec DSP |
| UX12 | Pas d'allocation/verrou RT ni surcharge audio hors budget lors des mises à jour UI |
| UX13 | Redimensionnement/DPI sans recouvrement, texte lisible, commandes atteignables |
| UX14 | Édition au clavier, focus, saisie précise, contraste et information non chromatique |
| UX15 | Automation et état de base/effectif cohérents dans toutes les vues |
| UX16 | Preset et édition dirty, erreurs, ressources absentes et sauvegarde explicites |
| UX17 | Performance reste immédiatement jouable sans dépendre d'une contrainte matérielle future |
| UX18 | Test de tâches sur wireframe, puis prototype fonctionnel et intégration DAW |
Les seuils temps/actions sont des cibles initiales. Tester utilisateurs novices et expérimentés, mesurer erreurs et temps, ajuster avant gel. Pas de PASS déclaré.

## 9. Lots
P0 : wireframes et parcours, composants/registre, barre commune, Performance, panneaux WT/FM/Cross et traitements, Inspector, routages List, moniteurs essentiels, redimensionnement/accessibilité. P1 : Overview, banque/états/Randomizer avancés selon lots DSP, diagnostics et finition des moniteurs/graphisme. P0/P1 appartiennent à V1; aucun lot de développement autorisé ici.

## 10. Huit arbitrages complémentaires VALIDÉS — 2026-11-24
1. C — Deux commandes FM contextuelles en Performance utilisant uniquement des paramètres/mécanismes DSP déjà existants, sans ajouter de nouveau DSP.
2. B — Sound Design : WT et FM de poids visuel comparable, CROSS central, traitements et modulation organisés en sections contextuelles.
3. B — Destination Inspector intégré, rétractable, ouvert depuis le paramètre sélectionné et capable de conserver/épingler son contexte.
4. B — Modulation Overview principalement destiné à comprendre, filtrer et naviguer; édition détaillée dans List/Inspector.
5. B — Esthétique CRT/laboratoire pour l'identité, mais données modernes, nettes et prioritaires sur l'effet visuel.
6. B — Layout adaptatif, redimensionnement et zoom global, avec minimum ergonomique déterminé par tests.
7. B — Décision historique désormais **supersédée** pour la phase actuelle : la couche Performance reste compacte, mais sa transposition hardware est différée et ne constitue plus une contrainte V1 active.
8. B — Processus obligatoire avant graphisme final : wireframes → tâches mesurées → prototype fonctionnel sans DSP complet → validation → design graphique final.

Ces décisions clôturent les arbitrages fonctionnels UI/UX V1. Restent à mesurer et non à arbitrer : dimensions exactes, seuils de temps/actions, budget graphique, libellés des deux commandes FM contextuelles. L'architecture logicielle et le plan de développement/recette restent ouverts.


## 11. North Star d'identité — VALIDÉ
**« Cette machine est construite autour de l'interaction entre ces deux moteurs. »**
Cette phrase devient le test de cohérence principal pour les décisions produit, DSP et UI. Le CROSS doit rester lisible comme signature centrale, sans transformer l'interface en patcher générique ni réduire WT/FM à de simples sources secondaires. Toute fonction future qui dilue cette lecture doit être challengée avant intégration.

## 12. Actualisation ARCH-UI-03 — 2026-11-27
La direction SYMBIOTIC INDUSTRIAL, le Design System révisé et l'organisation des trois espaces ont été validés. Voir [contrat technique](V1-ARCH-UI-03-SPEC.md) et [cartographie StableID](V1-ARCH-UI-03-STABLEID-MAP.md). Les maquettes restent consultatives; aucun code autorisé, aucune recette déclarée.
