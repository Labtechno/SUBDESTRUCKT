# RECOMMANDATIONS · ARCH-UI-04-DA

## Une façade lisible en jeu

- Réserver un emplacement flottant ou une cellule de barre à la crête en dBFS. Tester 960 × 640 avec état autonome suspendu, PANIC visible et Inspector ouvert puis fermé. Afficher une valeur et donner à la mesure un libellé accessible qui précise le point de mesure. Garder le gain en dB dans le panneau de sortie.
- Prévoir une disposition Performance compacte, par seuils de largeur et de hauteur. Garder en premier écran WT X/Y/Z, niveau CROSS, PANIC et état d’autorisation. Regrouper macros et mesure si la hauteur baisse. Donner au moniteur sa place selon l’espace utile. Configurer le seuil sur le viewport réellement disponible, y compris le volet et les rails.
- Donner à l’Inspector compact un état déplié et un bandeau replié avec le nom, Base, Effective et épinglage. Maintenir la destination sélectionnée et l’épingle lors d’un redimensionnement. À 150 %, proposer une disposition qui garde les repères de niveau et les actions critiques visibles. À 75 %, donner un plancher lisible aux libellés critiques et déplacer les détails secondaires vers l’Inspector.

[Comparatif taille et zoom](CONTACT-SHEETS/SIZE-ZOOM.png) · [Minimum annoté](ANNOTATIONS/02-lecture-taille-minimale.png)

## Sérigraphie et chemins de signal

- Utiliser un mot pour une fonction. Nommer WT Z dans la plaque WT, et NIV. CROSS pour les deux présentations de `cross.master`. Dans le Core, juxtaposer `X · WT→FM` et `Y · FM→WT`. Conserver les mêmes noms complets dans l’Inspector et l’infobulle, avec le StableID dans le détail technique.
- Donner un emplacement de façade au point de la chaîne. Nommer le mélange « BAL. PRÉ-CHAÎNES », le traitement « CROSS AUDIO » après les chaînes et le multiplicateur « NIV. CROSS ». Dans les réseaux, lire la route à l’intersection d’un en-tête source et d’un en-tête destination. Conserver le nom de route complet au focus, le marqueur de classe causale et l’état d’application pour chaque vraie cellule.
- Dans D2/S2, placer près du module l’état `ARMÉ` ou `ACTIF` du mode autonome et le libellé `GEL` de la commande de tampon ou de spectre. Présenter `AUTORISER` avec son périmètre d’instance. Donner à l’action `MÉMORISER LES GELS` une explication qui précise qu’elle prépare le contenu à sauver dans le preset ou projet. Coller le mode de feedback au réglage de feedback en D2.
- Garder un champ de valeur stable pour les gestes continus. `Hz`, `ms`, `s`, `rad`, `st`, `ct`, `dB` et `dBFS` conservent leur échelle et leur casse. Pour les grandeurs normalisées, présenter le domaine dans l’Inspector et préciser les extrêmes utiles, en particulier le centre de la balance. Valider la distinction entre Base, valeur réellement appliquée et mesure audio.

[Audit des libellés](CONTROL-LABELS-AUDIT.md) · [Propositions](CONTROL-LABELS-PROPOSALS.csv) · [Gel et armement](ANNOTATIONS/03-gel-armement-capture.png) · [Réseau CROSS](ANNOTATIONS/04-matrice-cross.png)

## Matière et hiérarchie

- Réserver une bordure usinée plus marquée au plan de jeu et au Core. Laisser le verre le plus sombre aux graphiques et aux valeurs. Dans les plaques secondaires, utiliser davantage la disposition et les groupements pour orienter le regard.
- Réduire la brillance des actions courantes. Donner une surface d’impulsion au bouton, une diode et une position à la bascule, un repère de sélection et une position à l’onglet. Garder forme et texte pour les états à conséquence sonore. Évaluer les mêmes trois états sur un panneau de production et un panneau Engineering.
- Donner les surfaces vacantes aux éléments qui bénéficient d’échelle, par exemple Bank Lab, l’éditeur de courbe ou une route sélectionnée. Tester la hiérarchie en regardant l’éditeur seul puis à côté d’un mixeur. Évaluer le confort sur une session de programmation de son réelle avant de fixer la typographie fine.

[Performance](CAPTURES/PERFORMANCE/02-inspector-ferme.png) · [Bank Lab](CAPTURES/ENGINEERING/04-bank-lab.png) · [Modulation](CAPTURES/SOUND-DESIGN/04-enveloppes-lfo.png)

## Contrat visuel du Core avant la 3D

Reporter le rendu sur la zone de verre avec dégagement fixe pour les axes, la poignée et la lecture du niveau. La zone doit rester exploitable en 2D quand l’espace disponible ou les performances de l’hôte limitent l’affichage. Vérifier accès souris, focus, saisie numérique, lecture Base/Effective, points extrêmes et cohérence du geste lorsque la fenêtre change de taille. Distinguer graphiquement la position d’édition, les valeurs réellement modulées et le rendu de matière. La couche de rendu peut évoluer avec un comportement de commande constant et un libellé d’état simple.

Au nominal sans Inspector, le plateau de verre carré est d’environ 418 px sur la capture. Son aire libre ne fixe pas à elle seule le volume du Core. L’enveloppe est à tester au minimum et à 150 % avec les repères visibles, avant toute direction volumétrique définitive. Pour l’aspect en volume, privilégier relief biomécanique contenu, reflets de verre fumé et détails visibles en silhouette. Évaluer la latence visuelle lorsque Reason échange rapidement des valeurs, ainsi que la charge de plusieurs éditeurs ouverts, durant la recette dédiée à ce rendu.

[Core statique](CAPTURES/CORE/01-core-statique.png) · [Repères](ANNOTATIONS/01-reperes-x-y-z.png) · [Performance 150 %](CAPTURES/SIZE-ZOOM/02-performance-150.png)
