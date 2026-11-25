# LABTECHNO — plateforme de recette perceptive

Page web statique d'écoute à l'aveugle avec questionnaire intégré, pensée pour Safari sur iPhone. Elle est publiée
sur GitHub Pages (branche `gh-pages`) et réutilisable d'une campagne de recette sonore à l'autre. Elle est écrite en
HTML, CSS et JavaScript natifs, sans dépendance externe, sans service analytique et sans compte utilisateur.

## Architecture

| fichier | rôle |
|---|---|
| `src/js/listening-engine.js` | moteur d'écoute générique (voir ci-dessous) |
| `src/js/questionnaire.js` | moteur de questionnaire piloté par le manifeste : choix unique, texte |
| `src/js/session-store.js` | sauvegarde : document JSON des résultats dans `localStorage`, réécrit à chaque modification ; restauration, validation, réinitialisation |
| `src/js/app.js` | interface : accueil, cas, résultats ; export, partage, copie |
| `src/js/dom.js` | création d'éléments sans `innerHTML` |
| `src/css/labtechno.css` | identité visuelle |
| `src/campaigns/index.json` | liste des campagnes et campagne par défaut |
| `src/campaigns/<id>/manifest.json` | configuration d'une campagne |
| `campaigns/<id>/audio/` | ressources audio, ajoutées par `build.py` depuis un commit audio figé |

Le moteur d'écoute :

- charge chaque fichier, vérifie sa taille et son SHA-256, puis le décode en Web Audio dans un contexte à 48 kHz ;
- garantit une lecture exclusive : une seule source audible à la fois ;
- bascule entre les rendus à la même position temporelle et conserve une position par cas ;
- propose une boucle facultative.

Aucun traitement du signal n'est appliqué : gain 1, connexion directe à la sortie. Seul un fondu de 5 ms
(`player.switchFadeMs`, 0 pour une coupure sèche) est appliqué au démarrage, à la pause et à la bascule.

## Ajouter une campagne

1. Commiter les fichiers audio, dans une archive ZIP, sur une branche du dépôt.
2. Créer `src/campaigns/<id>/manifest.json` sur le modèle de `s01-final` :
   - `campaignId` ;
   - `audioSource` : commit et chemin de l'archive ;
   - `renders` : liste des rendus, par exemple REF, X et Y ;
   - `questions` et `contextQuestions` ;
   - `cases`, avec pour chaque rendu `file`, `bytes` et `sha256` ;
   - `exportFileName` ;
   - `keyCommitment` facultatif.
3. L'ajouter à `src/campaigns/index.json`. Elle est accessible avec `?c=<id>` ou comme campagne par défaut.
4. Construire, tester et publier (voir ci-dessous). Aucune modification du code n'est nécessaire.

Ne mettez jamais dans un manifeste, ni ailleurs dans `src/`, la clé X/Y ou une donnée qui permette de la retrouver.
`build.py` refuse de construire si une référence T1/T2, un nom de métrique de la clé ou un fichier `KEY*` est
présent. Avec `--key`, il refuse aussi toute ligne de la clé locale.

## Construire, tester, publier

```
# depuis la racine du dépôt
py -3 -I tools/recette-web/build.py <dist> --key spikes/s01/audio_final/render/KEY_NE_PAS_OUVRIR_AVANT_ECOUTE.md
py -3 -m http.server 8765 --directory <dist>          # service local
python tools/recette-web/tests/recette.py http://localhost:8765/ <sortie> --repo .   # Playwright : webkit + chromium
```

Publication : la branche orpheline `gh-pages` contient exactement `<dist>`. GitHub Pages doit servir
`gh-pages` / `(root)` : Settings → Pages → Build and deployment → Deploy from a branch.

Les résultats ne sont jamais envoyés automatiquement : la page est statique et ne contient aucun jeton GitHub.
L'auditeur exporte `S01_LISTENING_RESULTS.json`, le partage depuis iOS, ou copie le JSON et le colle dans un message.
