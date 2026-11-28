# ARCH-UI-04-R2-B1 — plan de test des interactions (OD-01, PB-01, PB-02)

Ce plan prépare R2-B2. **En B1, aucun test de production n'est modifié et aucun code de production n'est corrigé.**
PB-01 et PB-02 restent au backlog. Ils seront corrigés dans les lots concernés, et seulement après une reproduction
préalable (OD-10, option B).

## 1. Stratégie d'identification (OD-01, option B : identifiant de composant stable)

### 1.1 Règles

| Règle | Contenu |
|---|---|
| ID-01 | Chaque composant que l'on teste ou que l'on navigue porte un identifiant stable `setComponentID(...)`, en ASCII, indépendant du texte visible, de la langue et de la sérigraphie. |
| ID-02 | Schéma : `param:<StableID>` pour une commande liée à un paramètre ; `action:<réf>` pour une action hors registre (référence du CSV de nomenclature, ex. `action:bar.authorize`) ; `state:<réf>` pour un indicateur d'état ; `nav:<espace>/<groupe>/<outil>` pour la navigation ; `view:<outil>/<neural\|table>` pour la Dual View. |
| ID-03 | Les identifiants sont déclarés dans un registre unique de l'interface (`ui/r2/ComponentIds.h`, à créer en R2-B2), versionné et append-only comme les StableID. |
| ID-04 | Le **titre accessible** n'est pas utilisé comme identifiant de test. Il reste le nom du registre pour les paramètres (UI-01w) et le nom complet anglais pour les actions. Il n'est pas affaibli. |
| ID-05 | Le HelpText `= StableID` de 0.4.1 (UI-01w, UI04-COV, TA-07w) est conservé. Il devient redondant avec `param:<StableID>`, mais n'est pas retiré. |
| ID-06 | **Aucune assertion n'est affaiblie.** Une migration ne change que le moyen de trouver le composant. Le critère, le seuil et le message restent identiques. Une recherche par identifiant qui ne trouve rien est un ÉCHEC, jamais une absence silencieuse. |
| ID-07 | Les tests fonctionnels historiques ne sont pas remplacés par des tests de présence. Les tests de présence (couverture des identifiants) s'**ajoutent**. |
| ID-08 | La navigation de test énumère les outils depuis le registre de navigation (groupes, outils), et non un nombre d'onglets codé en dur. |

### 1.2 Démonstration dans le laboratoire

`LAB-G11` attribue `action:bar.panic`, puis change le texte visible du bouton (`PANIC` devient `PANIQUE`). La recherche par
identifiant retrouve le même composant, et le titre accessible est inchangé : PASS sur Windows et sur macOS
([R2-B1-REPORT.md](R2-B1-REPORT.md) §5).

## 2. Inventaire de migration pour R2-B2

| Test | Emplacement | Repérage actuel (0.4.1) | Repérage R2-B2 | Assertion | Remarque |
|---|---|---|---|---|---|
| UI-01w | `tests/wrapper/WrapperTests.cpp` l.349–384 | `showSubTab(space, tab)` pour `space < 3`, `tab < 6` ; HelpText = StableID ; titre = nom du registre | Navigation `nav:*` énumérée depuis le registre ; `param:<StableID>` | Inchangée (titre accessible = nom du registre, nombre de contrôles sans titre = 0) | **Constat préexistant** : la boucle `tab < 6` ne visite pas les onglets ENGINEERING 6 à 9 (RANDOMIZE, MIDI LEARN, DIAGNOSTICS, CAPTURES / ASSETS). L'énumération par registre les couvrira : couverture élargie, pas réduite. |
| AUT-02 | WrapperTests l.409–413 | `showSubTab(v.space, v.tab)` | `nav:<espace>/<groupe>/<outil>` | Inchangée | Les indices d'onglet changent avec DA-A10. |
| AUT-08 | WrapperTests l.686 | texte de bouton `"Autoriser"` | `action:bar.authorize` | Inchangée (visibilité selon l'armement et la suspension) | Le texte devient AUTHORIZE (OD-02). |
| TA-07w | WrapperTests l.984–1014 | texte `"Drone"` ; HelpText `dst.gl1.arm` ; boucles `showSubTab` | `action:bar.drone` ; `param:dst.gl1.arm` ; navigation par registre | Inchangée | — |
| UI04-OPS | `tests/wrapper/Ui04Tests.cpp` l.798–825 | 42 + 10 chaînes visibles (« Ouvrir », « Capturer gels », « Importer WAV... », « Épingler »…) | Liste des identifiants `action:*` issus des références du CSV (ex. « Capturer gels » → `action:sound.chain.capture`) | Inchangée : chaque opération hors registre reste présente et reliée | Un test NAM distinct vérifie la sérigraphie validée. |
| UI04-DECO | Ui04Tests l.773–790 | textes collectés pour les messages | Identifiant ajouté aux messages | Inchangée | — |
| UI04-NAV | Ui04Tests l.366 | titres dans les messages de diagnostic | Inchangé | Inchangée | — |
| UI04-COV | Ui04Tests | StableID (HelpText) | Inchangé, plus `param:` | Inchangée (703/703) | — |
| *nouveau* NAM-01 | R2-B2 | — | Balayage des textes visibles | Aucune chaîne française | Ajout |
| *nouveau* NAM-03 | R2-B2 | — | Table des libellés validés ↔ composants | Égalité | Ajout |
| *nouveau* ID-UNIQ | R2-B2 | — | Registre des identifiants | Unicité, couverture 703 + actions | Ajout |

## 3. Comptage begin / end (outil commun)

**Instrument.** Un `juce::AudioProcessorListener` de test, attaché au processeur 0.4.1. Pour chaque index de paramètre,
il compte :

- `audioProcessorParameterChangeGestureBegin` et `...End` ;
- la profondeur courante et la profondeur maximale ;
- les écritures hors geste (`audioProcessorParameterChanged` sans geste ouvert, lorsqu'elles viennent d'une action
  utilisateur simulée).

**Critères (enregistrés avant mesure).**

| Critère | Seuil |
|---|---|
| BE-01 | `begin == end` pour chaque paramètre à la fin de chaque scénario |
| BE-02 | Profondeur maximale = 1 (aucun geste imbriqué) |
| BE-03 | Un geste utilisateur = une paire begin/end (molette : regroupement à 450 ms) |
| BE-04 | Aucune écriture utilisateur hors geste |
| BE-05 | Après destruction de l'éditeur, aucun geste ouvert |

**Scénarios par famille.**

| Famille | Scénarios |
|---|---|
| NEXUS | glisser ; Maj en cours de glisser ; double-clic hors valeur ; double-clic sur la valeur (saisie) ; molette ; flèches, Page, Début/Fin, Suppr ; Alt+clic ; clic droit → défaut |
| PLASMA | glisser ; clic en position absolue ; double-clic ; clavier |
| SYMBIOTIC CORE | glisser XY (deux paramètres, deux gestes) ; Maj ; double-clic (défauts) ; flèches |
| Éditeur d'enveloppe | glisser d'un point (un ou deux paramètres) |
| Réseaux (R2-F) | glisser de profondeur sur une liaison |

## 4. PB-01 — gestes imbriqués au double-clic (NEXUS)

**Constat (audit statique H-04, ARCH-UI-04-R1 §8).** `ContinuousControl::mouseDown` ouvre un geste (`link_.begin()`).
Au second clic, `mouseDoubleClick` hors zone de valeur appelle `resetToDefault()`, qui écrit par une transaction
complète alors que le geste du second `mouseDown` est encore ouvert. La séquence possible est begin, begin, end, end.

**Protocole de reproduction (avant toute correction, OD-10).**

1. Instancier le processeur 0.4.1 et l'éditeur. Attacher le compteur du §3.
2. Sur un NEXUS rotatif (ex. `wt.x`), simuler au centre du bouton : `mouseDown`, `mouseUp`, `mouseDown` (2 clics),
   `mouseDoubleClick`, `mouseUp`.
3. Résultat attendu sur 0.4.1 : profondeur maximale 2, donc **BE-02 en échec**. Ce résultat est la preuve de
   reproduction, à committer avant la correction.
4. Après correction en R2-B2 : BE-01 à BE-04 PASS, et la valeur finale égale la valeur par défaut.

**Traitement R2 démontré dans le laboratoire (pas en production).** Le double-clic écrit la valeur par défaut **dans**
le geste ouvert par le second clic (`LAB-G02` : profondeur maximale 1, 2 begin et 2 end).

## 5. PB-02 — geste non refermé à la destruction (Core, éditeur d'enveloppe)

**Constat.** `SymbioticCore` et `EnvelopeEditor` ouvrent un ou deux gestes à `mouseDown` et ne les referment qu'à
`mouseUp`. Si l'éditeur est détruit pendant un glisser, le geste reste ouvert côté hôte.

**Protocoles de reproduction.**

| Cas | Étapes | Résultat attendu sur 0.4.1 |
|---|---|---|
| PB-02a destruction | `mouseDown` + `mouseDrag` sur le pad du Core, puis `editorBeingDeleted` et destruction de l'éditeur, sans `mouseUp` | `begin` > `end` pour `cross.fwd` et `cross.rev` : BE-05 en échec |
| PB-02b masquage | même début, puis changement d'espace (vue masquée) | geste ouvert : BE-01 en échec |
| PB-02c perte de capture | même début, puis désactivation de la fenêtre ou ouverture d'un modal | à mesurer (dépend de la remontée de `mouseUp` par JUCE) |
| PB-02d enveloppe | `mouseDown` sur un point de l'éditeur d'enveloppe, puis destruction | `begin` > `end` |
| PB-02e redimensionnement | `mouseDown` + glisser, puis changement de taille ou de zoom de l'éditeur | à mesurer (RS-01) |

**Traitement R2 démontré dans le laboratoire.** `GestureGuard` referme le geste à la destruction (`LAB-G06`, `LAB-G10`),
au masquage (`LAB-G07`) et au détachement (`LAB-G08`). Les événements tardifs sont ignorés (aucune écriture hors geste).
La perte de capture remontée par l'hôte passe par `cancelGesture()`.

## 6. Auto-tests du laboratoire (composants R2 seulement)

| ID | Scénario | Critère |
|---|---|---|
| LAB-G01 | glisser 22 px | +0,10 ; 1 begin, 1 end |
| LAB-G02 | double-clic hors valeur | défaut ; profondeur maximale 1 |
| LAB-G03 | trois crans de molette, puis une touche | 1 geste molette, refermé avant la transaction clavier |
| LAB-G04 | cinq touches | 5 transactions complètes |
| LAB-G05 | Alt+clic | défaut en une transaction |
| LAB-G06 | destruction pendant un glisser | geste refermé |
| LAB-G07 | masquage pendant un glisser | geste refermé ; événements tardifs ignorés |
| LAB-G08 | détachement pendant un glisser | geste refermé |
| LAB-G09 | Maj pendant un glisser | ré-ancrage sans saut (0,600 → 0,610) |
| LAB-G10 | PLASMA : clic, double-clic, glisser puis destruction | 3 begin, 3 end, aucune imbrication |
| LAB-G11 | OD-01 : identifiant stable | retrouvé après changement du texte ; titre accessible inchangé |
| LAB-G12 | rendu des états imposés | aucune écriture |
| LAB-G13 | NEURAL CELL | bascule, sélection exclusive, textes ARMED et « — » |

Ces tests ne remplacent pas les tests BE-01 à BE-05 sur le code de production, qui sont à écrire en R2-B2.
