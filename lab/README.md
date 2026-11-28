# SUBDESTRUCKT — Component Lab (ARCH-UI-04-R2-B1)

Laboratoire **isolé** des composants R2. Ce n'est ni le plugin, ni une application musicale, ni une cible de
distribution. Il ne traite aucun signal audio. Documentation complète et captures :
[`docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/`](../docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/README.md).

## Architecture

| Dossier | Rôle | Dépendances |
|---|---|---|
| `r2/` | Composants R2 candidats à l'intégration R2-B2 : `R2Style` (tokens dérivés, primitives, `ValueSource`, `GestureGuard`), `R2Nexus`, `R2NeuralCell`, `R2SynapticTrack`, `R2CyberDisplay`, `R2PlasmaChannel` | JUCE `juce_gui_basics` et `ui/DesignTokens.h` (palette 0.4.1, lue sans modification). Ni registre, ni processeur. |
| `app/` | Laboratoire : DEMO DATA, planches, mesures (PT-02, PT-05, contrastes), auto-tests de gestes, fenêtre interactive, comparatif 0.4.1 / R2 | `r2/` ; le comparatif compile aussi `ui/*.cpp`, `plugin/PluginProcessor.cpp` et le cœur, **sans les modifier** |
| `tools/` | `gen_labels.py` (CSV de nomenclature R2-A, reproductible), `check_literals.py` (littéraux UTF-8), `build_review.py` (fusion des mesures et revue P1) | Python 3.11 |

Les composants R2 lisent une `ValueSource` abstraite. Dans le laboratoire, c'est `DemoValue` (DEMO DATA instrumentée).
En intégration, ce sera un adaptateur vers `ParamLink` (démontré par `app/Compare.cpp`, classe `ParamSource`). Les
composants n'ont donc pas à être dupliqués.

## Commandes

```bash
cmake -S lab -B build-lab -G Ninja -DCMAKE_BUILD_TYPE=Release -DSUBDESTRUCKT_JUCE_DIR=<JUCE 9.0.6>
cmake --build build-lab
L=build-lab/sbd_component_lab_artefacts/Release/sbd_component_lab
$L                                   # fenêtre interactive (planche complète)
$L --all out --csv docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-PROPOSALS.csv --scale 2   # planches + PT-02 + PT-05 + contrastes + auto-tests
$L --contact out/CAPTURES out/contact-sheet.png
build-lab/sbd_lab_compare_artefacts/Release/sbd_lab_compare out/CAPTURES/11-compare-041-r2.png 2
py -3.11 lab/tools/gen_labels.py --check     # le CSV de nomenclature se régénère à l'identique
py -3.11 lab/tools/check_literals.py
```

Sous macOS, le workflow `.github/workflows/component-lab.yml` exécute la même campagne sur `macos-15` et verse les
résultats dans `docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/{CAPTURES,data}/macos/`.
