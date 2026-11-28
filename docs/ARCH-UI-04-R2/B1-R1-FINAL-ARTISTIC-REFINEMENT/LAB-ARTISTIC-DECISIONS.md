# ARCH-UI-04-R2-B1-R1 — arbitrages artistiques LAB-A01 à LAB-A08

Arbitrages **normatifs** rendus par le propriétaire après le GO technique de R2-B1 et la contre-expertise de ChatGPT.
Ce document ne les rouvre pas. Pour chacun, il consigne la décision, son application dans le Component Lab, la preuve
produite et le critère d'acceptation correspondant (AR-01 à AR-15).

| ID | Option | Décision | Application (R2-B1-R1) | Preuves | Critère |
|---|---|---|---|---|---|
| LAB-A01 | A | Conservation des NEXUS Hybrid | Aucun changement du composant ni des primitives qu'il utilise. Les primitives nouvelles (`titaniumKey`, `recessedLed`, `machinedGroove`, `trackSelector`, `hudFrame`, `hudChip`) sont séparées. | Planche B1 « 02-nexus » re-rendue par le code R1 et comparée **pixel à pixel** à la capture B1 publiée de la même plateforme : `CAPTURES/*/02-nexus-nonregression.png`, ligne `NONREG` du journal | AR-01 |
| LAB-A02 | B | NEURAL CELL Machined Titanium | Touche en titane noir : alvéole usinée, chanfrein à éclairage haut-gauche, face bombée (action) ou en cuvette (bascule, armement), stries d'usinage à 3 %, témoins encastrés avec lentille, course physique (la face descend d'un pixel et l'ombre portée disparaît). Six traitements conservés, dimensions et gestes inchangés. | `03-neural-cell-before-after.png` (B1 et R1 rendus dans la même exécution) | AR-02 |
| LAB-A03 | B | SYNAPTIC Machined Track | Rail encastré (ombre interne haute, lèvre basse éclairée), sélecteur en titane avec biseau discret et marqueur du domaine encastré, crans usinés entre segments, onglets posés sur un rail. Plus sobre que NEURAL CELL ; dimensions, indices et comportements inchangés. | `04-synaptic-track-before-after.png` | AR-03 |
| LAB-A04 | C | Cybernetic HUD | Neuf variantes : verre optique profond (cœur plus clair, vignettage, reflet), monture usinée, rails gradués, repères d'angle, encadrement intérieur, réticules (oscilloscope, courbe), échelles instrumentales, capitales techniques. La nature de chaque donnée est indiquée par une pastille : BASE (carré), EFF (losange), MEAS (crochets), INDICATIVE, STATE. DEMO DATA conservée. Spectre INDICATIVE sur toute sa bande ; STOP, SUSPENDED et silence sur des lignes distinctes. Aucune animation, aucune télémétrie nouvelle, aucune étape d'enveloppe ni phase LFO présentée comme mesure. | `05a-cybernetic-hud.png`, `05b-cybernetic-hud-before-after.png` | AR-04 |
| LAB-A05 | A | Conservation de l'apparence PLASMA | Seule l'adaptation au gabarit COMPACT est ajoutée, dans une branche qui ne touche pas le chemin STANDARD. | Planche B1 « 06-plasma-channel » re-rendue et comparée pixel à pixel : `06c-plasma-nonregression.png` | AR-05 |
| LAB-A06 | C | Symbiotic Industrial Hybrid | Châssis graphite mat ; plaques en titane sombre ; commandes usinées ; afficheurs en verre cybernétique ; couleurs fonctionnelles des domaines. Palette inchangée. Ni vis décoratives, ni textures envahissantes, ni effets vintage. Aucune matière biomécanique hors Core. | `01-final-board.png`, `08-assembly-wt-cross-fm.png`, `11-materials-palette.png` | AR-06 |
| LAB-A07.1 | B | PANIC : carbone sur rouge | Libellé carbone définitif en R1. Rouge de sécurité inchangé. Face bombée dont l'assombrissement est confiné sous la zone du libellé. Contraste mesuré **sur la surface rendue** (tous les pixels de la zone du libellé, touche rendue sans libellé), dans 4 états et à 2 échelles. | `07-panic-final.png`, `data/*/panic-contrast.csv`, LAB-G14 | AR-07 |
| LAB-A07.2 | C | PLASMA vertical STANDARD + COMPACT | Même composant : STANDARD 64 × 180 et COMPACT 48 × 150, choisi par la largeur. COMPACT garde la valeur, l'identification (deux lignes au même corps), la position BASE, le marqueur EFFECTIVE, les graduations essentielles (bornes et zéro) et la zone de manipulation. | `06a-plasma-standard-compact.png`, `06b-plasma-zoom-75-100-150.png`, LAB-G15 | AR-08 |
| LAB-A08 | B | Nomenclature P1 optimisée | AMOUNT et MODE sous l'en-tête de section CROSS AUDIO (XAUDIO abandonné), KEEP FUND, BUS / CROSS AUDIO, OUT GAIN pour le gain général. Révision identifiable `../R2-CONTROL-LABELS-R1.csv` (générateur `lab/tools/gen_labels_r1.py`). Statut lexical P1 validé, distinct du statut géométrique mesuré. | `P1-NAMING-VALIDATION.md`, `P1-TRACEABILITY.csv`, `TYPOGRAPHY-MEASUREMENTS.csv`, `10-critical-nomenclature.png` | AR-09 à AR-11 |

## Critères d'acceptation

| ID | Critère | Vérification | État |
|---|---|---|---|
| AR-01 | NEXUS artistiquement préservés | Pixel à pixel contre B1, sur macOS et Windows | voir [R2-B1-R1-REPORT.md](R2-B1-R1-REPORT.md) §4 |
| AR-02 | NEURAL CELL Machined Titanium démontré | Comparatif avant / après | démontré ; validation artistique en attente (AR-15) |
| AR-03 | SYNAPTIC Machined Track démontré | Comparatif avant / après | démontré ; validation artistique en attente |
| AR-04 | Cybernetic HUD sur les neuf variantes | Planches 05a et 05b | démontré ; validation artistique en attente |
| AR-05 | Apparence PLASMA préservée | Pixel à pixel contre B1 | voir rapport §4 |
| AR-06 | Cohérence SYMBIOTIC INDUSTRIAL démontrée | Planches 01, 08 et 11 | démontré ; validation artistique en attente |
| AR-07 | PANIC carbone conforme (≥ 4,5:1 sur la surface rendue) | `panic-contrast.csv`, LAB-G14 | voir rapport §6 |
| AR-08 | PLASMA STANDARD et COMPACT fonctionnels | LAB-G15, planches 06a et 06b | voir rapport §7 |
| AR-09 | Nomenclature P1 actualisée et traçable | `P1-TRACEABILITY.csv` | voir rapport §8 |
| AR-10 | Aucun nouveau dépassement dans les gabarits mesurables | PT-02 sur la révision R1 | voir rapport §5 |
| AR-11 | Repères critiques au plancher validé | PT-02 : 9 px écran à 75 % | voir rapport §5 |
| AR-12 | Auto-tests sans régression | LAB-G01 à LAB-G15 | voir rapport §10 |
| AR-13 | Production et DSP inchangés | `git diff`, CI de production | voir rapport §11 à §13 |
| AR-14 | Captures macOS finales publiées | `CAPTURES/macos/`, planche-contact | voir rapport §15 |
| AR-15 | Validation artistique du propriétaire | **décision humaine exclusivement** | **EN ATTENTE** (jamais déclaré PASS par l'agent) |
