# ARCH-UI-04-R2-B2 — plan de tests

Ce plan complète [`../B1-COMPONENT-LAB/INTERACTION-TEST-PLAN.md`](../B1-COMPONENT-LAB/INTERACTION-TEST-PLAN.md), qui reste
normatif pour :

- la stratégie d'identification OD-01 (règles ID-01 à ID-08) ;
- l'inventaire de migration ;
- le compteur begin / end (BE-01 à BE-05) ;
- les protocoles PB-01 et PB-02.

Les critères d'acceptation sont dans [B2-ACCEPTANCE-CRITERIA.md](B2-ACCEPTANCE-CRITERIA.md).

## 1. Règles

| Règle | Contenu |
|---|---|
| T-R01 | Aucun test historique n'est modifié pour obtenir un PASS. Une migration OD-01 change seulement le moyen de trouver le composant : critère, seuil et message restent identiques. Toute migration est listée dans le rapport. |
| T-R02 | Les nouveaux tests s'ajoutent à la cible `subdestruckt_wrapper_tests`, donc à la CI de production. Leurs critères sont inscrits dans `tests/expected_results.txt` **avant** la mesure (barrière LB-10). |
| T-R03 | `tests/known_failures.txt` ne change pas (INV-09), sauf décision consignée. |
| T-R04 | Une reproduction (PB-01, PB-02) est publiée **avant** toute correction, avec son résultat brut. |
| T-R05 | Une lecture statique ne suffit jamais pour un PASS. Sans exécution : NON REPRODUIT ou BLOCKED, avec justification. |
| T-R06 | Les captures de production sont rendues à partir du plugin intégré (`MainEditor`) et rangées à part. Elles ne sont jamais présentées comme des captures du laboratoire, et inversement. |

## 2. Instrument commun

C'est l'instrument du plan B1 §3, appliqué au code de production. Un `AudioProcessorListener` de test compte, par
paramètre :

- les `begin` et les `end` ;
- la profondeur courante et la profondeur maximale ;
- les écritures hors geste.

Les événements de souris sont simulés avec la source de souris principale de JUCE. Les tests du laboratoire LAB-G01 à
G10 suivent le même principe.

| Critère | Seuil |
|---|---|
| BE-01 | `begin == end` pour chaque paramètre à la fin de chaque scénario |
| BE-02 | Profondeur maximale = 1 |
| BE-03 | Un geste utilisateur = une paire (molette : regroupement à 450 ms) |
| BE-04 | Aucune écriture utilisateur hors geste |
| BE-05 | Après destruction, masquage ou détachement du composant, aucun geste ouvert |

## 3. B2-01 — reproductions (sur le code 0.4.1, avant tout changement de composant)

| ID | Scénario | Résultat attendu sur 0.4.1 (hypothèse) | Suite |
|---|---|---|---|
| PB-01a | NEXUS rotatif `wt.x` : down, up, down, double-clic hors valeur, up | profondeur 2 (BE-02 en échec) | Correction en B2-03 (1) : défaut écrit **dans** le geste ouvert |
| PB-01b | PLASMA vertical `out.gain` : même séquence | profondeur 2 | Correction en B2-03 (2) (même classe) |
| PB-01c | PLASMA horizontal `perf.balance` : double-clic sur la zone de valeur | à mesurer : le second `mouseDown` ouvre un geste, puis la saisie s'ouvre | Selon le résultat |
| PB-01d | NEXUS : Alt+clic, menu « défaut », Suppr, saisie numérique | une paire, profondeur 1 | Contrôle |
| PB-01e | NEXUS : molette, puis clic avant 450 ms | à mesurer (geste molette ouvert pendant le clic) | Selon le résultat |
| PB-02a | Core : down, glisser, destruction de l'éditeur sans `mouseUp` | `begin > end` pour `cross.fwd` et `cross.rev` | Documenté ; correction en **R2-C** |
| PB-02b | Core : même début, changement d'espace | geste ouvert | idem |
| PB-02c | Core : même début, perte de capture (fenêtre désactivée, modal) | à mesurer | idem ; BLOCKED si la simulation hors écran est impossible |
| PB-02d | Éditeur d'enveloppe : down sur un point, destruction | `begin > end` | Documenté ; correction en **R2-D** |
| PB-02e | Core : glisser, puis changement de taille ou de zoom | à mesurer (RS-01) | idem |
| PB-N1 | NEXUS : glisser, puis destruction de l'éditeur | 0.4.1 referme dans le destructeur : BE-05 attendu PASS | Non-régression |
| PB-N2 | NEXUS : glisser, puis changement d'espace (vue masquée) | à mesurer | Si reproduit : correction dans B2-03 (1), lot qui modifie le geste NEXUS (OD-10) |
| PB-N3 | NEXUS dans un `ParamPanel` : glisser, puis changement d'algorithme par automation, qui reconstruit le panneau (RK-05) | destruction en plein geste ; BE-05 à mesurer ; ASan local | idem |

Les résultats bruts (CSV des compteurs, journal) sont publiés dans `B2-PRODUCTION-INTEGRATION/PB/` avant toute correction.

## 4. Tests par famille

| Famille | Tests existants (non modifiés) | Tests ajoutés |
|---|---|---|
| Fondation | UI04-CON (étendu aux tokens R1 : ajout, pas de remplacement), UI04-CLIP, UI04-NAV | **VIS-01** : chaque peintre partagé rend à l'identique, au pixel près, pour le composant de production et pour le laboratoire, à taille et état égaux ; **ID-UNIQ** : unicité et couverture des identifiants (703 + actions) ; **DS-01** : balayage statique (aucune couleur en dur hors `tok::`) |
| NEXUS | UI04-BIND, UI04-ALIAS, UI04-KEY, UI04-UNDO, UI04-AUTO, UI04-INACT, UI04-COV | BE-01 à BE-05 (scénarios B1 §3), NX-01 à NX-12, PB-01a/d/e après correction ; Int : pas de ±1 et ±10 conservés |
| PLASMA | idem (OUT GAIN, BALANCE, Z) | BE-01 à BE-05, PL-01 à PL-06 ; **PLAS-T** (LAB-G15 transposé : STANDARD et COMPACT, même contrat, changement de gabarit pendant un geste) ; mesure de crête absente du canal (PL-04) |
| NEURAL CELL | TA-07w, AUT-02 à AUT-08, UI04-DECO, UI04-OPS | NC-01 à NC-10 ; LAB-G13 transposé (OFF, ARMED, ACTIVE, DENIED, —) ; RENDER pendant un export hors ligne (NC-07) |
| SYNAPTIC | UI04-BIND, UI04-KEY, UI04-NAV, UI-01w | NAM-10 (indices et libellés hôtes des 537 options) ; menu de liste complet ; sélection exclusive sans écriture intermédiaire |
| HUD | UI04-RT, UI04-DSP | CD-01 à CD-08, TEL-05, TEL-06 ; **HUD-SRC** : chaque afficheur indique une source, et chaque donnée est tracée dans l'inventaire INV-11 ; état d'indisponibilité affiché si la donnée manque |
| PANIC et sûreté | UI04-PANIC, AUT-08, TA-07w | **PANIC-CON** (LAB-G14 transposé) : contraste du libellé ≥ 4,5:1 sur la surface rendue du composant de production (4 états, zooms 75, 100 et 150 %) ; NC-03, NC-06 |
| Nomenclature | — | NAM-01 (selon DB2-03), NAM-02, NAM-03 (bijection table ↔ composants), NAM-05, NAM-06, NAM-07, NAM-09, NAM-11, NAM-12, NAM-13 ; PT-02 dans la disposition de production (3 tailles × 3 zooms, macOS et Windows) |
| Accessibilité | UI-01w (titre accessible = nom du registre) | **AX-01** : nom et valeur formatée pour 703/703, par `AccessibilityHandler` ; AX-03 ; NX-08 ; lecture VoiceOver (PHYS) |
| Transversal | UI04-STATE, UI04-OPEN, UI04-MULTI, suite TA, REG-01, goldens | SV-01 (presets et projets 0.4.1 rechargés à l'identique), UR-01, MI-01, RS-01, ZM-01 ; GP-01 à GP-09 (protocole GC-01) |

## 5. Migrations OD-01 prévues

Ce sont celles de l'inventaire B1 §2 :

- UI-01w et AUT-02 : navigation énumérée par registre ;
- AUT-08 : `action:bar.authorize` ;
- TA-07w : `action:bar.drone`, `param:dst.gl1.arm` ;
- UI04-OPS : identifiants `action:*` à la place des 52 chaînes ;
- UI04-DECO : identifiant ajouté aux messages.

Chaque migration est faite **dans le commit qui change le texte visible correspondant**. Le test continue ainsi de
passer avec le même critère, sans période où il échouerait.

## 6. Laboratoire

- `component-lab.yml` reste actif : LAB-G01 à LAB-G15, non-régression NEXUS et PLASMA, PT-02, PT-05, contrastes.
- Si TB2-02 est retenu (peintres partagés), une non-régression s'ajoute : planches R1 identiques au pixel près aux
  captures R1 publiées, sur macOS.
- Les preuves B1 et B1-R1 restent intactes ; seules s'ajoutent des preuves B2.

## 7. Recette physique (NON EXÉCUTÉE tant que non faite)

À exécuter par le propriétaire sur le Mac de recette, dans Reason 14 :

- NC-03 (PANIC) ;
- NC-05 (FREEZE, ARM, AUTHORIZE, CAPTURE avant et après restauration en D2, S2 et F4) ;
- NC-07 (RENDER pendant un export) ;
- AX-01 (VoiceOver) ;
- SV-01 (projets 0.4.1) ;
- GC-01 physique (selon DB2-01) ;
- validation subjective NAM-14, DS-03 et CD-03.
