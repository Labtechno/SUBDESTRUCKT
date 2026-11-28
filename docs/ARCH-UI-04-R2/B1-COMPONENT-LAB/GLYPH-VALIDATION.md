# ARCH-UI-04-R2-B1 — validation des glyphes (PT-05, OD-11)

OD-11, option B : **Unicode vérifié, avec repli ASCII**. Glyphes de la nomenclature R2 : `→` (U+2192, routes),
`·` (U+00B7, séparateur de contexte), `…` (U+2026, action complétée ailleurs), `−` (U+2212, signe moins et retrait).

## 1. Méthode

Le Component Lab (`sbd_component_lab --glyphs`) interroge la police réellement résolue par JUCE pour l'interface
(sans-serif par défaut, normal et gras) et pour les valeurs (monospace par défaut, normal et gras). Il appelle
`juce::Typeface::getNominalGlyphForCodepoint`. Une absence signifie que JUCE substitue une autre police au rendu ; le
laboratoire indique alors laquelle (`Font::findSuitableFontForText`). Les glyphes sont aussi rendus réellement à 9, 10,
11, 12 et 14 px (planche 10, en rouge si le glyphe est substitué).

## 2. Résultats

| Plateforme | Police d'interface | Police des valeurs | → | · | … | − | Natifs |
|---|---|---|---|---|---|---|---|
| macOS 15.8.2 (VM GitHub, arm64) | Lucida Grande | Menlo | natif | natif | natif | natif | **16 / 16** |
| Windows 11 (poste local) | Verdana | Lucida Console | **substitué** (Segoe UI) dans Verdana ; natif dans Lucida Console | natif | natif | natif | **14 / 16** |

Données : [`data/macos/glyphs.csv`](data/macos/glyphs.csv), [`data/windows/glyphs.csv`](data/windows/glyphs.csv).
Rendus : [`CAPTURES/macos/10-critical-typography.png`](CAPTURES/macos/10-critical-typography.png),
[`CAPTURES/windows/10-critical-typography.png`](CAPTURES/windows/10-critical-typography.png).

## 3. Règle proposée pour R2-B2

| Règle | Contenu |
|---|---|
| GL-01 | La table d'alias stocke la forme Unicode (`WT→FM`, `Z · CROSS LVL`, `SAVE…`, `−0.20`). |
| GL-02 | Au démarrage de l'éditeur, chaque glyphe est vérifié dans la police d'interface résolue (même appel que PT-05). |
| GL-03 | Si un glyphe n'est pas natif, son repli ASCII est utilisé pour tout l'éditeur : `→` → `>`, `·` → `.`, `…` → `...`, `−` → `-`. Pas de mélange de polices dans un même libellé. |
| GL-04 | Le repli ne touche que l'affichage. Les noms hôtes, les StableID et le registre sont inchangés. |
| GL-05 | Les largeurs du repli sont mesurées avant adoption. Exemple : `WT>FM` mesure 37,9 px à 12 px sous Windows, pour un budget de 80 px (planche 10). |

Sur la cible produit (macOS arm64), les quatre glyphes sont natifs. Le repli ne s'y déclencherait pas. La vérification
porte sur une VM macOS 15.8.2 : la recette physique sur le Mac de référence reste à faire (R2-H).
