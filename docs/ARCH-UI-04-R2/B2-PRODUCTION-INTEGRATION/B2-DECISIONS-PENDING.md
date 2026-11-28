# ARCH-UI-04-R2-B2 — décisions soumises au propriétaire (STOP ciblés)

Chaque point bloque seulement l'étape indiquée. Les travaux indépendants continuent (mission R2-B2 §8).

## DB2-08 — Saut de valeur des PLASMA au clic sur le libellé ou sur la valeur (EC-17)

**Constat reproduit** (B2-01, dispatch JUCE réel, [PB/README.md](PB/README.md)) :

| Scénario | Action | Effet sur 0.4.1 |
|---|---|---|
| PB-J1 | Clic sur le libellé du PLASMA vertical OUT GAIN | 0 dB → **+6 dB** (maximum du paramètre) |
| PB-J2 | Clic sur le libellé du PLASMA horizontal BALANCE | 0,5 → **−1,0** (minimum) |
| PB-J3 | Clic sur la valeur du PLASMA horizontal BALANCE | 0,5 → **+1,0** (maximum) |
| PB-01c | Double-clic sur la valeur de BALANCE pour saisir un nombre | la valeur passe d'abord à **+1,0**, puis la saisie s'ouvre |

**Cause** (`ui/Components.cpp`, `ContinuousControl::mouseDown`) : le clic en position absolue est calculé sur la zone
de manipulation (`knobArea`) mais accepté sur toute la commande. Seule la zone de valeur du PLASMA vertical est exclue.
Un clic hors du canal donne donc une position hors bornes, ramenée au minimum ou au maximum.

**Pourquoi c'est soumis.** DB2-02 conserve le clic en position absolue et demande de vérifier l'absence de saut
involontaire. Elle exige aussi de soumettre tout défaut supplémentaire du comportement historique **avant** de le modifier.

| Option | Effet | Conséquences et risques |
|---|---|---|
| **(A) Limiter le clic absolu et le début de geste à la zone de manipulation (canal)** | Clic sur le libellé : sélection et focus seulement, aucune écriture. Clic sur la valeur : aucune écriture ; double-clic sur la valeur : saisie numérique sans écriture préalable (comme le PLASMA vertical aujourd'hui). Le clic absolu dans le canal reste inchangé (DB2-02). | Supprime les sauts (jusqu'à +6 dB sur OUT GAIN, audible). Aucun changement dans le canal. Le glisser commencé sur le libellé ne déplace plus la valeur (aucun usage connu). Les tests PB-J1 à J3 et PB-01c deviennent des assertions « aucun saut ». |
| (B) Conserver le comportement historique | Sauts conservés, documentés | Risque audible conservé (OUT GAIN +6 dB) ; incohérent avec « aucun saut involontaire » |
| (C) Option A, plus un glisser **relatif** commencé sur le libellé ou la valeur | Libellé et valeur saisissables sans saut | Ajoute un comportement nouveau, non demandé |

**Recommandation : (A).** C'est la correction minimale. Elle reste dans l'esprit de DB2-02, ne change rien au canal et
s'applique à la classe commune, donc aux PLASMA vertical et horizontal.

**Observation liée (EC-19, aucune modification proposée).** Un double-clic dans le canal d'un PLASMA place d'abord la
valeur à la position cliquée, puis revient au défaut. Cela fait deux transactions Undo, avec une valeur transitoire
pendant le délai du double-clic. C'est inhérent au clic absolu retenu par DB2-02 ; signalé pour information.

**Bloque** : seulement la correction du saut. Elle serait faite dans B2-03 (2), à l'intégration PLASMA. Le reste de
B2-03 (2) et les autres étapes continuent.
