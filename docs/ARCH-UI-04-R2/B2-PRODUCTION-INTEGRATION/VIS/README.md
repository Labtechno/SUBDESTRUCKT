# ARCH-UI-04-R2-B2 — conformité visuelle (CE-B2-03)

Les captures R1 historiques sont des **références indépendantes et immuables** :
[`../../B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES/`](../../B1-R1-FINAL-ARTISTIC-REFINEMENT/CAPTURES/).

- Windows : rendu local, code `50cc8aa`.
- macOS : VM GitHub, component-lab run 5, code `7cbfa43`.

Aucun outil ni workflow ne les réécrit. Depuis R2-B2, `component-lab.yml` ne verse plus de captures dans ce dossier.

## Chaîne de validation

| Étape | Comparaison | Outil | Statut |
|---|---|---|---|
| 1 | Référence R1 historique → Component Lab rendu avec les **peintres partagés** (`ui/r2/`) | `lab/tools/compare_r1_refs.py` | Windows : **15/15 identiques** ; macOS : workflow `component-lab.yml` ([`lab/macos/`](lab/macos)) |
| 2 | Référence R1 historique → composant intégré en production (même taille, même état, mêmes données) | Tests VIS-01 du wrapper (B2-03, par famille) | à venir |
| 3 | Composant intégré → interface complète (captures de production) | Captures de production (B2-03, B2-08) | à venir |

## Étape 1 : méthode

Le laboratoire rend ses 15 planches R1 avec les peintres extraits dans `ui/r2/` : `R2Style`, `R2Painters`, `R2Hud`.
Les corps de rendu ont été transférés par programme, sans modification ; seules les références aux membres sont devenues
des champs de vue. Deux comparaisons sont faites.

| Comparaison | Rendu | Zone comparée | Résultat Windows |
|---|---|---|---|
| (a) | SHA courant dans le bandeau | Tout sauf le bandeau d'en-tête (132 lignes) | 13/15 identiques. Les 2 planches composites (02, 06c) diffèrent de 3 pixels, ligne 132 : c'est le bandeau de la planche B1 incrustée (réduite à 50 %, placée à y = 112), qui porte son propre SHA. **Écart légitime d'en-tête.** |
| (b) **contrôle strict** | Chaîne de SHA de la référence (`--sha 50cc8aa`) | **Images entières** | **15/15 identiques, 0 pixel différent** |

Fichiers : [`lab/windows/compare-r1.txt`](lab/windows/compare-r1.txt), `compare-r1-hors-bandeau.csv`,
`compare-r1-images-entieres.csv`.

Une comparaison entre deux composants qui utilisent le même peintre ne suffit pas (CE-B2-03). L'étape 1 compare donc le
**laboratoire** à des **références historiques produites avant l'extraction**.
