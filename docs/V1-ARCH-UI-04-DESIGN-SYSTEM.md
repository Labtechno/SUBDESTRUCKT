# SUBDESTRUCKT — Design System SYMBIOTIC INDUSTRIAL (ARCH-UI-04)

Statut : **implémenté** (code `ui/`), vérifié par les tests automatiques UI04-* (`tests/wrapper/Ui04Tests.cpp`).
Contrat d'origine : [ARCH-UI-03](V1-ARCH-UI-03-SPEC.md) §2. Toute valeur graphique vient de `ui/DesignTokens.h` ;
aucun composant ne définit de couleur ou de dimension en dur.

## 1. Identité

Machine de laboratoire futuriste hébergeant une entité symbiotique : graphite minéral (châssis), titane noir
(commandes usinées), verre optique fumé (afficheurs), carbone (cavités). Accents lumineux limités aux arcs Base, aux
traits de sélection et aux contours d'état. Aucun effet CRT, aucune trame, aucun vieillissement, aucun néon décoratif.
Rouge réservé au Panic et aux alertes (dépassement de 0 dBFS, ELP, refus).

## 2. Tokens (`ui/DesignTokens.h`)

| Token | Valeur | Usage |
|---|---|---|
| `chassis` | #161A1D | fond de l'éditeur |
| `carbon` | #0A0D10 | cavités, barre globale, rails |
| `titanium` | #343C43 | corps des commandes, actions |
| `text` | #DCE4E9 | texte principal, index des NEXUS, focus |
| `wt` / `fm` / `cross` | #5CE2CB / #F2A05E / #B095FF | domaine sémantique du paramètre |
| `mod` | #7FA9E8 | valeur Effective, sources et routes de modulation |
| `alert` | #E0524A | Panic, alertes (jamais décoratif) |
| `warn` | #E6C35A | état ARMÉ (configuré, exécution suspendue) — distinct du rouge |
| `panel`, `panelEdge`, `glass`, `glassEdge`, `textDim`, `textMute`, `track` | dérivés | plaques, verre, textes secondaires |

Contrastes mesurés (UI04-CON, WCAG) : texte/plaque 12,6:1 ; secondaire/plaque 6,4:1 ; inactif/plaque 4,6:1 ; accents
sur carbone ≥ 5:1. Le texte et la forme portent toujours l'information (état, valeur, unité) ; la couleur la double.

Grille : module 4 px ; `gap` 8 ; `pad` 8 ; barre 44 (88 en deux rangées) ; onglets 28 ; Inspector 300 px.
Typographie : sans-serif système (libellés), monospace système (valeurs, chiffres tabulaires). Plancher **9,5 px** ;
au-delà, retour à la ligne (deux lignes au plus), jamais d'ellipse.

## 3. Composants (`ui/Components.h`, `ui/CurveEditors.h`)

| Composant | Rôle | Interaction |
|---|---|---|
| **NEXUS XL / S / Micro** (`ContinuousControl`, forme rotative) | Float/Int | arc Base (depuis 0 si bipolaire) ; anneau fin + point **Effective** (télémétrie moteur) ; valeur + unité ; marqueurs texte `M<n>` (routes globales), `CC<n>` (MIDI Learn), `INACTIF` (valeur conservée sans effet) |
| **PLASMA CHANNEL** (`ContinuousControl`, forme linéaire) | Float/Int en canal | membrane Base, marqueur Effective, **mesure réelle** séparée (sortie : crête dBFS) |
| **SYNAPTIC TRACK** | Choice | segments (≤ 5 choix lisibles) ou liste ; trait synaptique sous le choix ; jamais d'interpolation |
| **NEURAL CELL** | Bool (+ Drone, préférences) | diode et texte d'état **OFF / ON / ARMÉ / ACTIF / REFUS** ; ARMÉ = anneau, ACTIF = disque |
| **ActionCell** | action momentanée | repère d'action ; variante danger (Panic) |
| **TabStrip** | navigation | onglets clavier (←/→) ; jamais d'écriture d'état |
| **Plate** | plaque usinée titrée | accent sémantique, note de contexte |
| **OPTICAL DISPLAY** | verre fumé | point de prélèvement nommé ; données réelles uniquement |
| **EnvelopeEditor** | DAHDSR | poignées liées aux paramètres réels (un geste = une transaction) |
| **PointCurveEditor** | 2..8 points | forme Custom des LFO (`setLfoCurve`), courbe Custom des routes (`modUpdate`) ; validation au relâchement |
| **NetCell / NetGrid** | routes spécialisées | profondeur (NEXUS Micro), présence, classe B-T2 (lecture seule), ajout/retrait transactionnel |
| **ModRow** | lien global | source, destination (→ Inspector), profondeur, courbe, cadence, via, actif, retrait, état d'activité |
| **InspectorPanel** | contexte | alias de réglage, Base / Effective / mesure, plage, défaut, automation, MIDI, modulations ; épinglable |

Gestes communs des commandes continues : glisser (vertical ; Maj = fin ×0,1) ; double-clic = défaut ; Alt-clic =
défaut ; double-clic sur la valeur ou Entrée = saisie numérique (virgule, unité, suffixe k, ms/s acceptés) ; flèches =
1 % (Int : 1 pas) ; Page = 10 % ; Origine/Fin = bornes ; Suppr = défaut ; molette = un seul geste jusqu'à 450 ms
d'inactivité ; menu contextuel (saisie, défaut, MIDI Learn, Inspecter). Chaque interaction passe par
`juce::ParameterAttachment` : begin/end de geste hôte cohérents, une transaction Undo par geste, aucune écriture de la
valeur Effective.

## 4. États

`CellState` : Off, Armed, Active, Refused, Disabled. Une commande d'un paramètre sans effet dans le contexte courant
(route absente, direction exclue, Shape ignoré par Fold/Warp/Mutation) reste éditable et affiche `INACTIF` : la valeur
est conservée (UI04-INACT). Les commandes propres à un algorithme non sélectionné sont masquées, jamais effacées.

## 5. LookAndFeel (`ui/SymbioticLook`)

Redessine menus, listes, champs de saisie, barres de défilement, info-bulles, cases, poignée de redimensionnement et
curseurs secondaires (profondeurs de matrice, réglages Randomizer). Les commandes musicales principales n'utilisent
aucun widget JUCE générique (NEXUS, PLASMA, SYNAPTIC TRACK, NEURAL CELL).

## 6. Règles d'implémentation

- Une commande liée = un StableID (titre accessible = libellé du registre ; HelpText = StableID).
- Libellé contextuel = nom du registre sans les préfixes de groupe (`shortLabel`) ou légende explicite ; le nom complet
  reste dans l'Inspector, l'info-bulle et l'accessibilité.
- Préférences de présentation (espace, onglet, zoom, Inspector) : propres à l'éditeur, jamais dans l'état sonore.
- Rafraîchissement 30 Hz ; seules les commandes visibles lisent la télémétrie ; aucun accès au thread audio.
