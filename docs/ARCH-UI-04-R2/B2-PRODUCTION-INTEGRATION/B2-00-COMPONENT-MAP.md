# ARCH-UI-04-R2-B2-00 — cartographie

Audit en lecture seule au SHA `73b633b1c382dd5303b58a3889ea8efeca9e0569`. Aucun fichier de production n'a été modifié.

## 1. Composants de production (0.4.1)

**Interface** : `ui/`, 6 609 lignes avec `plugin/`.

| Fichier | Classes | Rôle |
|---|---|---|
| `ui/DesignTokens.h` | `tok::*`, `Cell`, `CellState`, `Domain3` | Palette (9 couleurs de référence + 9 dérivées), grille, cellules (`nexusXL` 132 × 150, `nexusS` 84 × 96, `nexusMicro` 64 × 74, `track1` 168 × 46, `cell1` 168 × 40, `plasmaV` 48 × 150, `plasmaH` 220 × 40), typographie (plancher 9,5 px), constantes de geste (220 px, × 0,1, 1 %, 10 %, 450 ms, 30 Hz) |
| `ui/Components.h/.cpp` | `UiContext`, `BoundControl`, `ParamLink`, `ContinuousControl` (NEXUS et PLASMA), `SynapticTrack`, `NeuralCell`, `ActionCell`, `TabStrip`, `Plate`, `OpticalDisplay`, `ParamPanel`, `ScrollPanel`, `makeControl`, `preferredCell`, `shortLabel`, `layoutLabel` | Composants liés et disposition en flux |
| `ui/SymbioticLook.h/.cpp` | `SymbioticLook` (LookAndFeel_V4) | Widgets JUCE génériques : boutons, bascules, listes, menus, barres de défilement, champs, curseurs linéaires, infobulles |
| `ui/Monitors.h/.cpp` | `ScopeView` (oscilloscope, spectre), `XYView`, `NetworkView`, `MonitorPanel` | Moniteurs |
| `ui/CurveEditors.h/.cpp` | `EnvelopeEditor`, `PointCurveEditor` | Éditeurs interactifs |
| `ui/MainEditor.h/.cpp` | `MainEditor` | Barre globale, trois espaces, Inspector, zoom 75–150 % |
| `ui/Views.h`, `PerformanceView.cpp`, `SoundDesignView.cpp`, `EngineeringView.cpp` | `SpaceView`, `SymbioticCore` (`CorePad`), `InspectorPanel`, `ZOrderDisplay`, `NetGrid`, `ModRow`, `ModMatrixPage`, `WaveEditor`, outils ENGINEERING | Espaces |
| `ui/DiagnosticEditor.*` | `DiagnosticEditor` | Compilé, jamais instancié (EC-06) |

**Correspondance type de paramètre → composant** (`makeControl`, registre de 703 paramètres) :

| Type | Nombre | Composant par défaut |
|---|---|---|
| Float | 501 | NEXUS (rotatif, densité de la vue) |
| Int | 9 | NEXUS |
| Choice | 143 | SYNAPTIC TRACK (segments ou liste) |
| Bool | 50 (dont 9 armements autonomes) | NEURAL CELL |

**Formes explicites** :

- PLASMA vertical : OUT GAIN (cellule 96 × 158) et CROSS LVL du Core (74 de large, hauteur variable) ;
- PLASMA horizontal : BALANCE.

**Widgets JUCE génériques** (déclarations de membres ; certaines classes, comme `ModRow`, sont instanciées une fois par
ligne) :

| Endroit | Widgets |
|---|---|
| ENGINEERING | 7 `Slider`, 15 `ComboBox`, 1 `ToggleButton`, 4 `TextEditor`, 10 `Label`, 1 `TextButton` |
| SOUND DESIGN | 2 `ComboBox` |
| Barre | 2 `ComboBox`, 3 `Label` |
| Inspector | 1 `Slider`, 2 `ComboBox`, 3 `Label` |
| PERFORMANCE | 8 `Label` |
| Moniteur | 4 `TextButton` (sélecteur de mode) |

**Dépendances des tests envers les types et les textes** :

| Test | Dépendance |
|---|---|
| UI04-KEY | `dynamic_cast<ContinuousControl*>`, `TextEditor` |
| UI04-DECO | `Button`, `ToggleButton`, `NeuralCell`, `ComboBox` |
| AUT-08 | `TextButton` « Autoriser » |
| TA-07w | `ToggleButton` « Drone » |
| UI04-OPS | 52 chaînes visibles |
| UI04-CORE | `CorePad` |
| UI04-COV, UI04-BIND, UI04-CLIP | `BoundControl` (`userSet`, `shownBase`, `textFits`) |

Aucun composant ne porte d'identifiant `setComponentID` (0 occurrence).

## 2. Composants R1 du laboratoire

`lab/r2/`, 2 913 lignes. Rendu R1 par défaut ; révision B1 conservée seulement pour les comparatifs (non intégrée).

| Fichier | Contenu | Intégrable tel quel |
|---|---|---|
| `R2Style.h/.cpp` | Tokens dérivés (`relief1` à `relief3`, `groove`, `hudGlassCore`, `hudGlassDeep`, `hudRail`, `titaniumFace`) ; `ValueSource` ; `GestureGuard` ; primitives `draw::*` (`machinedKnob`, `arc`, `glowArc`, `ticks`, `key`, `led`, `opticalFrame`, `titaniumKey`, `recessedLed`, `machinedGroove`, `trackSelector`, `hudFrame`, `hudChip`, `hudTitleFont`, `demoTag`, `hatch`, `focusRing`, `fittedText`) | Primitives et tokens : oui, à extraire. `demoTag` et la révision B1 : non. |
| `R2Nexus.*` | NEXUS XL, S et Micro | Rendu : oui. Gestes : **non** (EC-10). |
| `R2PlasmaChannel.*` | PLASMA V (STANDARD, COMPACT) et H | Rendu : oui. Gestes : **non** (EC-05, EC-10). |
| `R2NeuralCell.*` | Six traitements, Machined Titanium, PANIC carbone | Rendu R1 : oui |
| `R2SynapticTrack.*` | Segments, liste, onglets (Machined Track) | Rendu R1 : oui. Menu : à la production. |
| `R2CyberDisplay.*`, `R2CyberHud.cpp` | Neuf variantes HUD, pastilles de nature | Rendu : oui. Données de démonstration : **non**, remplacées par des données réelles. |

## 3. Paramètres, valeurs et états réellement disponibles

| Donnée | Source (API existante, inchangée) | Classe (R2-TELEMETRY-FEASIBILITY) | Consommateur R1 |
|---|---|---|---|
| Valeur Base | `ParamLink::base()` (attachement JUCE) | — | NEXUS, PLASMA, NEURAL, SYNAPTIC |
| Valeur Effective, modulée ou non | `telemetry().effective[703]`, `modulated[703]` | A (T-11) | Marqueur EFF des NEXUS et PLASMA ; pastille EFF des HUD |
| Profondeurs des routes vers un paramètre | `currentMod()`, `modCount` (UI, 10 Hz) | A / B | Marques de route NEXUS (NX-09) |
| Autonomie : suspendue, autorisée, verrou PANIC, source, transport, hors ligne | `telemetry()` T-05, `autonomySuspended()`, `autonomyAuthorized()` | A | NEURAL Arm (OFF, ARMED, ACTIVE, DENIED), AUTHORIZE, HUD Autonomy, RENDER (T-14) |
| Drone | `drone()` | A (T-04) | NEURAL Arm DRONE |
| Captures D2 / S2 | `captures()` | B (T-29) | CAPTURE, état « — » |
| Crête L/R | `telemetry().peakL/R` | A (T-17) ; maintien côté UI (B) | HUD PEAK (barre, OUT GAIN) |
| Oscilloscope | `telemetry().scope` (sortie finale, mono, 1 sur 2) | A (T-18) | HUD Scope (MEAS) |
| Spectre | FFT côté UI | B (T-19), INDICATIVE | HUD Spectrum |
| Niveaux des enveloppes | `telemetry().envelope`, `sources[...]` | A (T-06, T-07) | HUD Envelope : niveau publié seulement, jamais d'étape (T-08, classe C) |
| Valeurs des sources (LFO…) | `sources[...]` | A (T-09) ; phase : C (T-10) | Aucune animation de phase |
| Positions WT X/Y/Z Effective | `telemetry().wtX/Y/Z` | A (T-12) | HUD Wavetable (rail X : BASE et EFF) |
| Compteurs de sûreté, CPU | `telemetry()` T-20, T-21 | A | HUD Diagnostic |
| Ordre Z | thread de contrôle | B (T-28) ; restriction inter-domaines **non validée** (OD-16) | HUD (monture) de l'ordre Z |
| État du preset, modifié, Undo/Redo | `presetName()`, `dirty()`, `canUndo()`… | — | Barre |

**Absentes de 0.4.1, donc jamais affichées comme des mesures** :

| Donnée | Classe |
|---|---|
| Étape d'enveloppe | C |
| Phase LFO | C |
| Niveau par route, par opérateur ou par chaîne | D |
| Exécution autonome par étage | C |
| Lecture d'automation | D |

## 4. Dépendances d'architecture

```
JUCE 9.0.6 (juce_gui_basics, juce_audio_processors) ──► ui/DesignTokens.h ──► ui/r2 (prévu : primitives, peintres, GestureGuard, IDs, libellés)
                                                                              │
plugin/PluginProcessor (API inchangée) ──► ParamLink (juce::ParameterAttachment) ──► BoundControl ──► ContinuousControl / SynapticTrack / NeuralCell
                         └─► telemetry() (lecture seule, 30 Hz) ──► tick() des composants visibles, moniteurs
```

**Responsabilités préservées** :

| Élément | Responsabilité |
|---|---|
| `ParamLink` | Seul chemin d'écriture d'un paramètre (begin, during, end, complete). Une transaction Undo par geste. |
| `GestureGuard` | Garantit qu'un geste ouvert est refermé une seule fois : à la fin normale, à la destruction, au masquage ou au détachement. Il s'interpose entre les gestionnaires d'événements et `ParamLink`. Il n'écrit jamais lui-même une valeur. |
| `ValueSource` | Modèle de valeur des peintres : normalisé, Effective, texte, bipolarité. En production, c'est un adaptateur de lecture sur `ParamLink` et la télémétrie (principe de `ParamSource`, `lab/app/Compare.cpp`). Il n'est jamais une seconde voie d'écriture. |
