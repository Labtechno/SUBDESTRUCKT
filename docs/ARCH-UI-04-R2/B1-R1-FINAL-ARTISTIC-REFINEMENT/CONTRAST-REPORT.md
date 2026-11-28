# ARCH-UI-04-R2-B1-R1 — rapport de contraste

Fichier généré par `lab/tools/build_validation_r1.py` à partir de `data/<plateforme>/contrast.csv` et `panic-contrast.csv`
(Component Lab). Rapport de luminance WCAG 2.x. Seuils : 4,5:1 pour le texte (13 px gras reste un texte normal), 3:1 pour les
éléments d'interface non textuels et le texte inactif, 1,15:1 (seuil candidat) entre niveaux de relief, 1:1 pour le décor.

## 1. Paires de tokens

Calcul sur la valeur des tokens, indépendant de la plateforme. Les faces en dégradé R1 sont évaluées à leurs extrémités.

| Avant-plan | Fond | Usage | Rapport | Seuil | Verdict |
|---|---|---|---|---|---|
| text | relief1 (plaque) | sérigraphie, valeurs | 11.58 | 4.50 | PASS |
| text | carbon (fenêtre de valeur) | valeur numérique | 15.14 | 4.50 | PASS |
| text | glass (afficheur) | titre d'afficheur | 14.64 | 4.50 | PASS |
| text | relief3 (touche d'action) | libellé d'action | 7.83 | 4.50 | PASS |
| text | relief2 (touche plate) | libellé de bascule | 9.83 | 4.50 | PASS |
| textDim | relief1 | texte secondaire | 5.91 | 4.50 | PASS |
| textDim | glass | source d'afficheur | 7.47 | 4.50 | PASS |
| textDim | relief2 | état OFF | 5.02 | 4.50 | PASS |
| textMute | relief1 | libellé inactif | 4.21 | 3.00 | PASS |
| textMute | glass | graduation | 5.32 | 3.00 | PASS |
| wt | glass | trace WT | 11.84 | 3.00 | PASS |
| fm | glass | trace FM | 8.94 | 3.00 | PASS |
| cross | glass | trace CROSS | 7.71 | 3.00 | PASS |
| mod | glass | trace MOD / EFFECTIVE | 7.84 | 3.00 | PASS |
| wt | relief1 | arc BASE WT sur plaque | 9.36 | 3.00 | PASS |
| fm | relief1 | arc BASE FM sur plaque | 7.07 | 3.00 | PASS |
| cross | relief1 | arc BASE CROSS sur plaque | 6.09 | 3.00 | PASS |
| mod | relief1 | anneau EFFECTIVE sur plaque | 6.20 | 3.00 | PASS |
| warn | relief2 | état ARMED | 7.42 | 4.50 | PASS |
| warn | glass | étiquettes INDICATIVE / DEMO DATA | 11.05 | 4.50 | PASS |
| white | alert | texte PANIC 0.4.1 et B1 (13 px gras = texte normal WCAG) | 3.84 | 4.50 | FAIL |
| carbon | alert | texte PANIC R1 adopté (LAB-A07.1), rouge nominal ; surface rendue : panic-contrast.csv | 5.08 | 4.50 | PASS |
| alert | glass | crête > 0 dBFS | 4.91 | 3.00 | PASS |
| focus | relief1 | anneau de focus | 11.58 | 3.00 | PASS |
| track | relief1 | piste d'arc (décor) | 1.23 | 1.00 | PASS |
| relief1 | relief0 | niveau plaque / châssis | 1.17 | 1.15 | PASS |
| relief2 | relief1 | niveau corps / plaque | 1.18 | 1.15 | PASS |
| relief3 | relief2 | niveau action / corps | 1.26 | 1.15 | PASS |
| text | titane R1 haut | libellé d'action R1, haut du dégradé | 5.27 | 4.50 | PASS |
| text | titane R1 bas | libellé d'action R1, bas du dégradé | 10.66 | 4.50 | PASS |
| text | touche plate R1 haut | libellé de bascule R1 | 8.08 | 4.50 | PASS |
| textDim | touche plate R1 bas | état OFF R1 | 5.39 | 4.50 | PASS |
| text | verre HUD cœur | titre et valeur HUD | 14.20 | 4.50 | PASS |
| textDim | verre HUD cœur | source et clés HUD | 7.25 | 4.50 | PASS |
| textMute | verre HUD cœur | graduations HUD | 5.16 | 3.00 | PASS |
| warn | verre HUD profond | pastilles INDICATIVE et DEMO DATA | 11.59 | 4.50 | PASS |
| mod | verre HUD cœur | repères EFF | 7.61 | 3.00 | PASS |
| hudRail | verre HUD cœur | rails et graduations d'encadrement (décor) | 1.37 | 1.00 | PASS |

Bilan : 37 / 38 PASS. Échecs : white sur alert (texte PANIC 0.4.1 et B1 (13 px gras = texte normal WCAG), 3.84:1). Le libellé blanc de 0.4.1 et B1 est conservé comme référence historique ; R1 utilise le libellé carbone.

## 2. PANIC sur la surface rendue (LAB-A07.1)

La touche est rendue sans libellé, dans chaque état et à chaque échelle. Chaque pixel de la zone du libellé est comparé à la couleur
du libellé. La valeur retenue est le **minimum** : le pire pixel du dégradé réel.

| Plateforme | Variante | État | Échelle | Pixels | Contraste min. | Contraste max. | Verdict |
|---|---|---|---|---|---|---|---|
| macos | R1 carbone (adopté, LAB-A07.1) | normal | 1 | 407 | 5.17 | 6.77 | PASS |
| macos | R1 carbone (adopté, LAB-A07.1) | normal | 2 | 1533 | 5.14 | 6.71 | PASS |
| macos | R1 carbone (adopté, LAB-A07.1) | hover | 1 | 407 | 5.18 | 7.03 | PASS |
| macos | R1 carbone (adopté, LAB-A07.1) | hover | 2 | 1533 | 5.14 | 6.90 | PASS |
| macos | R1 carbone (adopté, LAB-A07.1) | pressed | 1 | 407 | 5.08 | 5.23 | PASS |
| macos | R1 carbone (adopté, LAB-A07.1) | pressed | 2 | 1533 | 5.08 | 5.23 | PASS |
| macos | R1 carbone (adopté, LAB-A07.1) | focus | 1 | 407 | 5.17 | 6.77 | PASS |
| macos | R1 carbone (adopté, LAB-A07.1) | focus | 2 | 1533 | 5.14 | 6.71 | PASS |
| macos | B1 blanc (0.4.1, référence) | normal | 1 | 407 | 3.85 | 4.96 | FAIL |
| macos | B1 blanc (0.4.1, référence) | normal | 2 | 1533 | 3.90 | 5.06 | FAIL |
| macos | B1 blanc (0.4.1, référence) | hover | 1 | 407 | 3.61 | 4.82 | FAIL |
| macos | B1 blanc (0.4.1, référence) | hover | 2 | 1533 | 3.65 | 4.88 | FAIL |
| macos | B1 blanc (0.4.1, référence) | pressed | 1 | 407 | 3.85 | 5.00 | FAIL |
| macos | B1 blanc (0.4.1, référence) | pressed | 2 | 1606 | 3.85 | 5.06 | FAIL |
| macos | B1 blanc (0.4.1, référence) | focus | 1 | 407 | 3.85 | 4.96 | FAIL |
| macos | B1 blanc (0.4.1, référence) | focus | 2 | 1533 | 3.90 | 5.06 | FAIL |
| macos | B1 carbone (variante B1) | normal | 1 | 407 | 3.92 | 5.06 | FAIL |
| macos | B1 carbone (variante B1) | normal | 2 | 1533 | 3.85 | 5.00 | FAIL |
| macos | B1 carbone (variante B1) | hover | 1 | 407 | 4.05 | 5.40 | FAIL |
| macos | B1 carbone (variante B1) | hover | 2 | 1533 | 3.99 | 5.34 | FAIL |
| macos | B1 carbone (variante B1) | pressed | 1 | 407 | 3.90 | 5.06 | FAIL |
| macos | B1 carbone (variante B1) | pressed | 2 | 1606 | 3.85 | 5.06 | FAIL |
| macos | B1 carbone (variante B1) | focus | 1 | 407 | 3.92 | 5.06 | FAIL |
| macos | B1 carbone (variante B1) | focus | 2 | 1533 | 3.85 | 5.00 | FAIL |
| windows | R1 carbone (adopté, LAB-A07.1) | normal | 1 | 451 | 5.17 | 5.80 | PASS |
| windows | R1 carbone (adopté, LAB-A07.1) | normal | 2 | 1701 | 5.14 | 5.76 | PASS |
| windows | R1 carbone (adopté, LAB-A07.1) | hover | 1 | 451 | 5.20 | 6.03 | PASS |
| windows | R1 carbone (adopté, LAB-A07.1) | hover | 2 | 1701 | 5.17 | 5.98 | PASS |
| windows | R1 carbone (adopté, LAB-A07.1) | pressed | 1 | 451 | 5.08 | 5.23 | PASS |
| windows | R1 carbone (adopté, LAB-A07.1) | pressed | 2 | 1701 | 5.08 | 5.23 | PASS |
| windows | R1 carbone (adopté, LAB-A07.1) | focus | 1 | 451 | 5.17 | 5.80 | PASS |
| windows | R1 carbone (adopté, LAB-A07.1) | focus | 2 | 1701 | 5.14 | 5.76 | PASS |
| windows | B1 blanc (0.4.1, référence) | normal | 1 | 451 | 3.90 | 4.96 | FAIL |
| windows | B1 blanc (0.4.1, référence) | normal | 2 | 1701 | 3.94 | 5.00 | FAIL |
| windows | B1 blanc (0.4.1, référence) | hover | 1 | 451 | 3.65 | 4.82 | FAIL |
| windows | B1 blanc (0.4.1, référence) | hover | 2 | 1701 | 3.69 | 4.82 | FAIL |
| windows | B1 blanc (0.4.1, référence) | pressed | 1 | 451 | 3.90 | 4.96 | FAIL |
| windows | B1 blanc (0.4.1, référence) | pressed | 2 | 1782 | 3.89 | 5.00 | FAIL |
| windows | B1 blanc (0.4.1, référence) | focus | 1 | 451 | 3.90 | 4.96 | FAIL |
| windows | B1 blanc (0.4.1, référence) | focus | 2 | 1701 | 3.94 | 5.00 | FAIL |
| windows | B1 carbone (variante B1) | normal | 1 | 451 | 3.92 | 5.00 | FAIL |
| windows | B1 carbone (variante B1) | normal | 2 | 1701 | 3.90 | 4.94 | FAIL |
| windows | B1 carbone (variante B1) | hover | 1 | 451 | 4.05 | 5.34 | FAIL |
| windows | B1 carbone (variante B1) | hover | 2 | 1701 | 4.05 | 5.27 | FAIL |
| windows | B1 carbone (variante B1) | pressed | 1 | 451 | 3.92 | 5.00 | FAIL |
| windows | B1 carbone (variante B1) | pressed | 2 | 1782 | 3.90 | 5.00 | FAIL |
| windows | B1 carbone (variante B1) | focus | 1 | 451 | 3.92 | 5.00 | FAIL |
| windows | B1 carbone (variante B1) | focus | 2 | 1701 | 3.90 | 4.94 | FAIL |

Pire cas du libellé carbone R1, tous états et toutes échelles : macos 5.08:1 ; windows 5.08:1 (exigence : 4,5:1). La valeur la plus basse correspond au rouge nominal, à l'appui. L'éclaircissement du survol et du repos la relève.
