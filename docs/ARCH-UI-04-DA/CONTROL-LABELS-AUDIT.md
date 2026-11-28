# DA-09 · LIBELLÉS DES COMMANDES

**5/10.** Le système gère déjà les noms courts, le contexte et les valeurs avec une discipline technique. Les collisions de sens et la densité au zoom faible limitent le jeu. La priorité est la distinction des commandes, suivie de la largeur et de la lisibilité.

## Données et méthode

- **25 captures Reason** et relevés d’accessibilité associés. 151 StableID identifiés explicitement dans l’arbre d’accessibilité physique, ce qui corrobore la présence de la commande dans l’hôte. Cet arbre donne souvent le nom complet. La sérigraphie exacte vient de l’image ou de la commande JUCE.
- **703 StableID sur 703 mesurés** au banc JUCE de la révision compilée. Le banc instancie l’éditeur nominal avec Inspector ouvert, sélectionne 23 contextes de navigation et explore des choix et bascules conditionnels jusqu’à trois niveaux avec le parcours UI04-COV. Au total, 1 988 états visités pour les trois zooms et 2 706 relevés distincts, dont 828 de façade par zoom. Une ligne représente une combinaison de commande, contexte, emplacement, famille et largeur. Des alias et variantes d’un même StableID peuvent compter plusieurs fois. L’éditeur de ce banc travaille en mémoire sur son état jetable. [Mesures](CONTROL-LABELS-MEASUREMENTS.csv) · [Parcours UI04-COV](../../tests/wrapper/Ui04Tests.cpp)
- **Largeur** calculée sur le texte actuel avec `juce::GlyphArrangement::getStringWidth` et la fonte réellement choisie pour ce composant. La largeur « une ligne » représente l’avance typographique complète au corps retenu. Elle peut dépasser le budget quand deux lignes sont prévues. Le budget vient du rectangle de dessin du contrôle. Le corps écran est le corps nominal multiplié par le zoom pour le canvas. Les pixels logiques restent indépendants du facteur Retina. Le champ « hors viewport initial » indique qu’une partie de la commande sort du rectangle d’au moins un viewport dans l’état du banc relevé. [Composants](../../ui/Components.cpp) · [Tokens](../../ui/DesignTokens.h) · [Disposition](../../ui/MainEditor.cpp)
- **Choix** mesurés avec la fonte de la liste ou du segment. 587 combinaisons de choix et contextes par zoom, soit 1 761 lignes. Les textes des menus natifs et les titres secondaires non liés sont traités par relevé des chaînes et contrôle des captures. Pour les propositions de ces surfaces, le champ budget vide signale une géométrie dynamique ou non instrumentée. [Mesures des options](CONTROL-LABELS-CHOICES.csv)
- **Propositions** pour chaque occurrence à corriger repérée dans le périmètre, dont 667 libellés liés, 365 options et 70 références hors registre. Les longueurs comptent les caractères, y compris espaces et signes. Les largeurs actuelles et proposées sont recalculées au même corps de référence sur une ligne. Le budget est le minimum observé au nominal pour la combinaison, ou le budget calculable pour la chaîne manuelle. Les deux libellés de barre « Annuler » et « Rétablir » dépassent le budget actuel de 43 px au corps de référence. Prévoir une largeur de texte d’au moins 50 px si ces changements sont retenus. [Propositions détaillées](CONTROL-LABELS-PROPOSALS.csv)

Le test UI04-CLIP archivé rapporte zéro troncature, chevauchement, dépassement du parent et onglet tronqué sur 4 804 contrôles testés dans 12 configurations. Dans notre banc, aucun libellé lié relevé ne sort de son allocation après repli sur deux lignes. Les captures montrent en revanche le découpage normal par les viewports, ainsi que l’absence de OUT dans la barre minimale en état suspendu. Les tests diffèrent par leur contexte, leur mécanisme de mesure et leurs zones d’observation. [Rapport UI04](../V1-ARCH-UI-04-REPORT.md) · [Captures de taille](CONTACT-SHEETS/SIZE-ZOOM.png)

## Cas mesurés à 100 %

| StableID, surface | Actuel → proposé | Caractères | Largeur actuelle → proposée, px logiques | Budget / corps | Décision |
|---|---|---:|---:|---:|---|
| `wt.x`, Performance WT | `X  position` → `X` | 11 → 1 | 55,18 → 6,64 | 128 / 12,5 | Plaque WT XYZ et intitulé WT X au focus. |
| `wt.z`, Performance WT | `Z  Master` → `WT Z` | 9 → 4 | 47,77 → 25,56 | 128 / 12,5 | Profondeur des transformations de table, distincte de CROSS. |
| `cross.master`, Core | `Z  Master` → `NIV. CROSS` | 9 → 10 | 42,04 → 51,29 | 70 / 11 pour le rail, 80 / 11 pour le NEXUS | Accepter le libellé plus long pour désigner clairement le même niveau sur les deux contrôles. |
| `wt.z_gain`, Transformations Z | `Z Gain Compensation` → `COMP. GAIN Z` | 19 → 12 | 97,40 → 64,22 | 99 / 11 | Reprendre la marge de 1,6 px, garder la diode et le statut. |
| `cross.audio.amount`, Bus / Audio | `Cross Audio Amount` → `DOSAGE` | 18 → 6 | 94 → 37,54 | 80 / 11 | Le texte courant tient sur deux lignes. Le nom de l’étage se trouve déjà dans le titre de section. |
| `cross.op1_wt_freq.depth`, Sound / FM→WT | `OP1>WT Freq` → `FRÉQ.` | 11 → 5 | 62,51 → 26,19 | 80 / 11 | La rangée « OP1 → WT » porte le sens. |
| `cross.op1_wt_freq.depth`, Engineering | `OP1›WT.FREQ` → `PROF.` | 11 → 5 | 55,91 → 23,90 | 80 / 10 | La cellule est identifiée par les deux en-têtes. Route complète à la sélection et au focus. |
| `dst.gl1.arm`, chaîne globale | `Autonomous Armed` → `AUTONOMIE` | 16 → 9 | 91,04 → 56,03 | 139 / 11 | Réserver ARMÉ et ACTIF au badge et expliquer l’autorisation d’instance. |
| `dst.gl1.d2.fb_mode`, D2 | `Feedback Mode` → `MODE FDBK` | 13 → 9 | 70,39 → 53,79 | 164 / 11, 76 par choix | Aligner sur FDBK. Rapprocher du réglage concerné et distinguer Historique/Régénératif. |

[X/Y/Z et preuves](ANNOTATIONS/01-reperes-x-y-z.png) · [D2 et S2](ANNOTATIONS/03-gel-armement-capture.png) · [En-têtes de réseau](ANNOTATIONS/04-matrice-cross.png)

## 75 %, 100 % et 150 %

| Zoom | Corps écran XL / S / Micro | Surface utile et constat |
|---|---|---|
| 75 % | 9,38 / 8,25 / 7,50 px | Performance conserve sa vue d’ensemble. Les petites inscriptions de Sound Design deviennent difficiles à isoler sous fond sombre et à côté des valeurs Micro. 7 relevés de façade du banc dépassent au moins un viewport initial. [Performance](CAPTURES/SIZE-ZOOM/01-performance-75.png) · [Sound](CAPTURES/SIZE-ZOOM/05-sound-75.png) |
| 100 % | 12,50 / 11 / 10 px | Le meilleur compromis pour comparer des zones au nominal. 48 relevés de façade du banc dépassent un viewport initial, par exemple D2 Feedback Mode dans la chaîne. Au minimum, la crête et les macros sortent de l’écran initial. [Nominal](CAPTURES/PERFORMANCE/01-nominal-inspector.png) · [Minimum](CAPTURES/SIZE-ZOOM/03-minimum-100-inspector.png) |
| 150 % | 18,75 / 16,50 / 15 px | Les inscriptions visibles se lisent mieux mais le canvas demande un pan et un défilement. 528 relevés de façade du banc dépassent un viewport initial dans la pose nominale, avec les contextes explorés. L’Inspector et la barre conservent leur échelle à chaque zoom. [Performance](CAPTURES/SIZE-ZOOM/02-performance-150.png) · [Sound](CAPTURES/SIZE-ZOOM/06-sound-150.png) |

Les décomptes mesurent l’accès initial dans les états instrumentés. L’appréciation de l’effort de lecture vient de l’examen des captures. L’historique de défilement, le zoom macOS et la densité de l’écran peuvent changer l’expérience.

## Règles de sérigraphie par famille

| Famille | Zone de dessin à 100 % et taille actuelle | Règle proposée |
|---|---|---|
| NEXUS XL | 128 px en cellules de 132, corps 12,5, deux lignes autorisées | Un mot à 100 % quand la plaque identifie la fonction. Laisser une marge latérale et réserver le détail à l’Inspector. Maintenir les repères X/Y/Z, le signe et l’unité de valeur. |
| NEXUS S | 80 px pour la cellule standard, 96 px pour une macro Performance, corps 11 | Une ligne si possible. Conserver ATK, REL, FDBK et RÉS. si les mots sont déjà familiers du public. En général, viser une marge minimale de 8 px dans la cellule, avec seconde ligne de secours. |
| NEXUS Micro | 60 px au token standard, 80 px en grille NetCell, corps 10 | L’axe ou le titre porte le contexte. Au focus, le détail donne la route complète. Donner la priorité à la valeur et à l’application du lien. Vérifier à 75 % et en état inactif. |
| NEURAL CELL | Largeur utile `w − 69`. 99 px à 168 px. Corps 11, deux lignes | Un substantif ou un état configuré, avec forme et badge distinct. Nommer la fonction AUTONOMIE et réserver ARMÉ/ACTIF au statut. Donner une formulation propre à l’opérateur pour Enable et Mute. |
| SYNAPTIC TRACK | Titre `w − 4`. Corps 11. Liste `w − 32`. Segments `(w − 4)/n − 6` | Nom de choix non équivoque, indice stable. Privilégier une liste pour les algorithmes à nom composé. Examiner tous les choix et leurs états actif/inactif. |
| PLASMA CHANNEL | Légende horizontale `(w − 4) × 0,34`, verticale `w − 4`, corps 11 dans les exemples | Donner la grandeur, sa direction utile et l’unité de valeur. Séparer gain et crête, balance pré-chaînes et Cross Audio post-chaînes. |
| OPTICAL DISPLAY | Corps 10 pour le cartouche standard, contenu variable | Titre court, échelle et unité visibles. Reporter StableID, versions et justification dans le détail. Préserver la légende d’une mesure et celle d’une illustration. |
| Matrices | En-tête NetGrid 84 px, cellule 84 px, intitulé Micro spécifique | Source en rangée, destination en colonne et classe causale distincte. Utiliser une flèche unique. Dans Sound Design, la rangée explicite la source même quand aucune grille n’est affichée. |
| Inspector | 300 px de volet à chaque zoom, 284 px avant bordures internes, corps 15 pour le nom et 10,5 pour les métadonnées | Afficher le nom complet, la valeur de base et la valeur appliquée. Grouper plage, domaine, unité, modulation et automation. Mettre le StableID, le chemin et l’explication des cadences dans les détails techniques. |
| Titres et onglets | Titre de plaque 12,5, onglets corps 11 gras | Français pour les fonctions et actions de cette interface, avec les noms produit Performance, Sound Design, Engineering, CROSS, Bank Lab et SYMBIOTIC CORE. Garder les termes normalisés WT, FM, FFT et les symboles d’unités. Chaque niveau fournit un contexte exploité par les libellés enfants. |

Le nom complet est construit depuis le registre dans l’entête de l’Inspector et dans les infobulles des trois primitives liées. L’Inspector expose aussi les champs qui permettent de retrouver le contrôle dans l’hôte. Pour les routes spécialisées, la proposition maintient une description complète du sens et du port. Pour les actions hors registre, l’Inspector ne représente pas une destination éditable. Le nom et le contexte doivent se trouver sur le contrôle ou dans son aide. [Inspector](CAPTURES/PERFORMANCE/01-nominal-inspector.png) · [Code du volet](../../ui/MainEditor.cpp)

## Critères de réalisation

- Préserver les StableID, les indices de choix, les noms d’automation hôte et le sens des gestes. Représenter la nouvelle sérigraphie dans une table d’affichage séparée. Garder la variante visuelle locale quand un onglet précise la signification.
- Recontrôler les noms et états critiques à 75 %, 100 % et 150 % au nominal et au minimum. Les lignes P1 définissent les corrections avant intégration du Core. Vérifier aussi les valeurs signées, extrêmes et unités. Conserver un libellé ou marqueur d’état même avec une couleur atténuée.
- Valider avec le propriétaire l’orientation francophone et les formules courtes. Les états conditionnels testés hors hôte ne donnent pas de preuve visuelle physique pour chaque combinaison. La présente campagne ne mesure pas chaque menu natif, chaque nom de preset utilisateur ni chaque chaîne dynamique possible.

[Backlog](PRIORITIZED-BACKLOG.md) · [Principes de réalisation](DESIGN-RECOMMENDATIONS.md)
