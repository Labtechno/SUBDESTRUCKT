# Manuel utilisateur SUBDESTRUCKT (HTML statique)

Une version de manuel par build, publiée sur GitHub Pages sous `manual/<version>/` (branche `gh-pages`) sans
toucher la plateforme d'écoute S0.1 à la racine du site.

## Organisation

| Élément | Emplacement |
|---|---|
| Contenu éditorial (16 chapitres) | `src/<version>/chapters/NN-*.html` |
| Paramètres (textes ; plages vérifiées contre `source/core/Parameters.cpp`) | `src/<version>/parameters.json` |
| Glossaire | `src/<version>/glossary.json` |
| Exercices | `src/<version>/exercises.json` |
| Tests de recette R-01..R-15 | `src/<version>/recette/tests.json` |
| Moteur de recette : stockage / interface | `src/<version>/recette/store.js`, `recette.js` |
| Navigation, recherche, glossaire, figures | `src/<version>/js/manual.js` |
| Lecteurs audio A/B | `src/<version>/js/audio-ab.js` |
| Simulations pédagogiques | `src/<version>/js/demos.js` |
| Styles (graphite, clair/sombre, impression) | `src/<version>/css/manual.css` |
| Rendus audio du moteur réel + paramètres | `src/<version>/audio/` (outil : `tools/manual-audio/render_examples.cpp`) |
| Figures mesurées | `src/<version>/assets/fig/` (outil : `tools/manual-audio/make_figures.py`) |
| Gabarits de page | `templates/` |
| Configuration des versions | `versions.json` |
| Schéma JSON des résultats | `schema/results.schema.json` |
| Tests automatisés | `../tests/manual/` |

## Construire et tester

    python manual/build.py                 # -> manual/dist/ (contrôles de cohérence bloquants)
    python -m pytest tests/manual -q       # Playwright (chromium, webkit) + jsonschema
    MANUAL_BASE=https://labtechno.github.io/SUBDESTRUCKT/manual/ python -m pytest tests/manual -q

## Nouvelle version

1. Copier `src/<ancienne>` vers `src/<nouvelle>` et ne documenter que ce qui existe dans la nouvelle build.
2. Régénérer l'audio et les figures depuis le commit source de la build.
3. Ajouter l'entrée dans `versions.json` (SHA build, SHA documentation, archive, campagne de recette).
4. Construire, tester, publier `dist/<nouvelle>/` et `dist/index.html` sur `gh-pages` sous `manual/`.
   Les versions précédentes restent en place.
