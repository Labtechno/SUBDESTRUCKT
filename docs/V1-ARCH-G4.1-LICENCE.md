# SUBDESTRUCKT — ARCH-G4.1 §13 — Licence JUCE et solution de recette privée

Statut : **étude, aucune décision prise**. Aucun choix de licence, aucune acceptation de conditions et aucun
engagement financier n'ont été faits au nom de l'utilisateur. Aucun binaire n'a été publié.

## 1. Sources officielles consultées (27/11/2026)

| Page | Contenu utile |
|---|---|
| https://juce.com/get-juce/ | niveaux de licence et prix ; « JUCE is dual licensed under both the JUCE licence and the AGPLv3 » |
| https://juce.com/legal/juce-9-licence/ | EULA JUCE 9 en vigueur (publiée le 17 juin 2026) |
| https://juce.com/legal/juce-8-licence/ | EULA JUCE 8 (7 mai 2024, dernière révision 17 juillet 2025), pour mémoire |
| https://github.com/juce-framework/JUCE/blob/master/LICENSE.md | double licence AGPLv3 / licence commerciale JUCE ; sans AGPLv3, l'utilisation vaut acceptation de l'EULA JUCE 9 |

Le projet épingle JUCE 9.0.6 (commit 654bdbc7) : c'est l'EULA JUCE 9 qui s'applique hors AGPLv3.

## 2. Ce que disent les pages (faits)

- **Niveaux** (EULA JUCE 9 §1.2) : Starter gratuit, plafond de revenus **20 000 USD** ; Indie 40 USD/mois ou
  800 USD perpétuel, plafond 300 000 USD ; Pro 175 USD/mois ou 3 500 USD, sans plafond ; Educational gratuit mais
  réservé à l'enseignement et à la recherche académique (aucun usage commercial, professionnel ou promotionnel).
- **Plafond Starter** (§1.2.1) : revenus des **12 derniers mois** ; pour une personne, il inclut tous les revenus ou
  financements issus de l'utilisation du Framework, y compris dons, sponsoring et publicité ; pour une société, ceux
  de l'entité et de ses affiliés, sans compensation. Au-delà, il faut passer à un niveau supérieur ou cesser de
  développer et de distribuer (§1.14).
- **Distribution** (§1.9) : rendre un produit disponible « over a network » à une autre personne vaut distribution ;
  le produit peut être distribué et vendu par son propriétaire, y compris en binaire fermé. Interdits notamment :
  redistribuer le Framework seul (§1.17), soumettre le Framework à une licence libre exigeant la divulgation des
  sources (§2.3), mélanger Starter et Indie/Pro (§1.13).
- **Postes** (§1.7) : une licence par développeur ; pas de licence supplémentaire pour une machine qui ne sert qu'à
  compiler et tester.
- Aucune obligation d'écran de démarrage « Made with JUCE » ni de compte n'a été trouvée pour Starter dans les pages
  lues (la page de téléchargement n'a pas été lue).
- FAQ : sans AGPLv3, une licence JUCE est nécessaire au moins pendant toute la durée de distribution de binaires
  fermés.

## 3. Conséquences pour SUBDESTRUCKT (déductions, à confirmer par l'utilisateur)

1. **Publier un VST3** (release, artefact CI public, fichier dans le dépôt public) est une distribution. Il faut alors
   soit une licence EULA JUCE 9 couvrant la distribution (Starter si les revenus et financements du projet restent
   ≤ 20 000 USD sur 12 mois), soit l'option AGPLv3, qui imposerait de fournir les sources correspondantes de l'œuvre
   combinée sous AGPLv3. **Ce choix appartient à l'utilisateur.** En attendant, la règle reste : aucun binaire public.
2. **Compiler localement pour un usage privé** sur sa propre machine n'est pas une distribution. Les pages
   n'exemptent pas explicitement cet usage : l'utilisation de JUCE suppose une licence choisie (AGPLv3, ou EULA avec
   Starter gratuit sous le plafond). Les deux options sont gratuites dans ce cas ; aucune ne coûte rien à l'utilisateur.
3. La CI actuelle n'expose aucun binaire : l'artefact CI exclut les bundles et paquets (vérifié dans
   `.github/workflows/arch-g4.yml`), il ne contient que des journaux, captures et résultats.

## 4. Solution gratuite et conforme proposée pour la recette privée (Mac Studio M4 Max)

`tools/local-build/build_local_macos.sh` : l'utilisateur compile lui-même, sur son Mac, le commit choisi :

- JUCE 9.0.6 cloné et **vérifié par SHA**, comme en CI ; compilation Release arm64 (Ninja) ;
- tests du cœur et du wrapper exécutés sur la machine (les mesures PERF-01..03 deviennent celles du M4 Max, seule
  source valable pour une mesure M4 Max ; une VM ou un PC Windows ne certifient rien) ;
- bundle signé **ad hoc** localement ; option `--install` vers `~/Library/Audio/Plug-Ins/VST3` (sans sudo) ;
- options `--pluginval` (v1.0.4, empreinte vérifiée, rigueur 10) et `--packages` (Installer.pkg, Uninstaller.pkg,
  README_FR.txt, CHANGELOG.txt, SHA256SUMS.txt, ZIP) produits **localement**, dans `local-build/` (ignoré par git) ;
- le script **refuse de s'exécuter** sans `--juce-licence=agplv3|starter|autre` : il ne choisit rien, il enregistre
  seulement le choix déclaré par l'utilisateur dans le rapport local.

**Mise à jour ARCH-G4.2** (le texte ci-dessus décrit la version G4.1 du script) : l'option `--install` est
supprimée (aucune installation automatique : l'utilisateur ouvre lui-même Installer.pkg) ; pluginval et la livraison
complète sont toujours exécutés et bloquants (`--pluginval` et `--packages` sont devenus sans effet) ; dépôt propre
exigé, sources exportées du commit, arrêt au premier échec. Détail : `docs/V1-ARCH-G4.2-REPORT.md` §3 et §9.

Coût : nul (outils Apple gratuits, cmake/ninja via Homebrew). Aucun compte n'est créé par le script.

**Mise à jour ARCH-G4.3** : pipeline réordonné (pluginval sur le bundle signé AVANT les paquets ; paquets construits à
partir du bundle validé, charge utile vérifiée ; ZIP réextrait et vérifié) ; barrière exhaustive partagée avec la CI ;
suite partielle (`SBD_ONLY`), `local-build/` symbolique et collisions refusés. Procédure : `docs/V1-LOCAL-BUILD.md`.
Arbitrage du propriétaire : **JUCE Starter retenue sous réserve** d'éligibilité et d'acceptation explicite par lui ;
aucune condition n'a été acceptée par l'agent ; distribution publique toujours interdite.

## 5. Décisions restant à l'utilisateur

- D-LIC-1 : licence JUCE retenue pour la recette privée : Starter retenue sous réserve (ARCH-G4.3) ; éligibilité et
  acceptation explicite à confirmer par le propriétaire lui-même.
- D-LIC-2 : licence et modalités d'une éventuelle diffusion publique future (Starter et son plafond, Indie/Pro, ou
  AGPLv3 avec publication des sources) ; signature Developer ID et notarisation (payantes, non engagées).
