# SUBDESTRUCKT — Component Lab (ARCH-UI-04-R2-B1)

Laboratoire **isolé** des composants R2. Ce n'est ni le plugin, ni une application musicale, ni une cible de
distribution. Il ne traite aucun signal audio. Documentation complète et captures :
[`docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/`](../docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/README.md) (R1, final) et [`B1-COMPONENT-LAB/`](../docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/README.md) (B1, historique).

## Architecture

| Dossier | Rôle | Dépendances |
|---|---|---|
| `r2/` | Composants R2 candidats à l'intégration R2-B2 : `R2Style` (tokens dérivés, primitives, `ValueSource`, `GestureGuard`), `R2Nexus`, `R2NeuralCell`, `R2SynapticTrack`, `R2CyberDisplay`, `R2PlasmaChannel` | JUCE `juce_gui_basics` et `ui/DesignTokens.h` (palette 0.4.1, lue sans modification). Ni registre, ni processeur. |
| `app/` | Laboratoire : DEMO DATA, planches, mesures (PT-02, PT-05, contrastes), auto-tests de gestes, fenêtre interactive, comparatif 0.4.1 / R2 | `r2/` ; le comparatif compile aussi `ui/*.cpp`, `plugin/PluginProcessor.cpp` et le cœur, **sans les modifier** |
| `tools/` | `gen_labels.py` (nomenclature R2-A), `gen_labels_r1.py` (révision R1, LAB-A08), `build_review.py` (revue P1 de B1), `build_validation_r1.py` (validation P1 R1 : statut lexical et géométrique), `check_literals.py` (littéraux UTF-8) ; tous avec contrôle de reproductibilité | Python 3.11 |

Les composants R2 lisent une `ValueSource` abstraite. Dans le laboratoire, c'est `DemoValue` (DEMO DATA instrumentée).
En intégration, ce sera un adaptateur vers `ParamLink` (démontré par `app/Compare.cpp`, classe `ParamSource`). Les
composants n'ont donc pas à être dupliqués.

## Commandes

```bash
cmake -S lab -B build-lab -G Ninja -DCMAKE_BUILD_TYPE=Release -DSUBDESTRUCKT_JUCE_DIR=<JUCE 9.0.6>
cmake --build build-lab
L=build-lab/sbd_component_lab_artefacts/Release/sbd_component_lab
$L                                   # fenêtre interactive (planche complète)
$L --all out --csv docs/ARCH-UI-04-R2/R2-CONTROL-LABELS-R1.csv --scale 2 --b1 docs/ARCH-UI-04-R2/B1-COMPONENT-LAB/CAPTURES/<plateforme> --sha <sha>
# planches R1, non-régression NEXUS / PLASMA pixel à pixel, PT-02, PT-05, contrastes, PANIC sur surface rendue, auto-tests LAB-G01..G15
$L --contact out/CAPTURES out/contact-sheet.png
build-lab/sbd_lab_compare_artefacts/Release/sbd_lab_compare out/CAPTURES/12-compare-041-r2.png 2
py -3.11 lab/tools/gen_labels.py --check     # nomenclature R2-A : identique octet pour octet
py -3.11 lab/tools/gen_labels_r1.py --check  # nomenclature R1 (LAB-A08) : identique octet pour octet
py -3.11 lab/tools/build_validation_r1.py    # validation P1 R1 à partir des mesures de chaque plateforme
py -3.11 lab/tools/check_literals.py
```

Sous macOS, le workflow `.github/workflows/component-lab.yml` exécute la même campagne sur `macos-15` et verse les
résultats dans `docs/ARCH-UI-04-R2/B1-R1-FINAL-ARTISTIC-REFINEMENT/{CAPTURES,data}/macos/`. Les preuves B1
(`B1-COMPONENT-LAB/`) sont conservées telles quelles ; les planches B1 se reproduisent depuis le commit `786c6ad`.
