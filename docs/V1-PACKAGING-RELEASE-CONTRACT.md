# SUBDESTRUCKT — Contrat de packaging et de livraison V1
Date : 2026-11-25. Statut : **EXIGENCE PRODUIT VALIDÉE — IMPLÉMENTATION ET RECETTE À FAIRE**.

## 1. Périmètre
À compter d'ARCH-G3, chaque version installable de SUBDESTRUCKT, y compris build de test, alpha, beta, release candidate et stable, est livrée avec un installateur ET un désinstallateur. Un VST3 nu ne constitue pas une livraison. Les spikes de recherche ne produisant pas de VST3 installable (S0, S0.1 notamment) sont exclus. Cette décision n'autorise pas à démarrer ARCH-G3 et ne modifie aucun contrat DSP.

Cible : VST3 instrument macOS Apple Silicon arm64 natif, M3 minimum (cible non encore certifiée). Aucun standalone, AU, Windows, Intel/Rosetta ou hardware ajouté. La version minimale de macOS reste à déterminer et doit être annoncée pour chaque build testé.

## 2. Format de livraison
Archive ZIP versionnée, par exemple SUBDESTRUCKT_v0.1.0_macOS_arm64.zip (exemple, pas une version déjà attribuée).
Contenu obligatoire :
- SUBDESTRUCKT_Installer.pkg
- SUBDESTRUCKT_Uninstaller.pkg
- README_FR.txt : version, commit source, système et hôtes testés, installation, mise à jour, désinstallation, données conservées, limitations connues, statut signature/notarisation.
- CHANGELOG.txt : changements et régressions connues, compatibilité des états/presets et avertissements de retour arrière.
- SHA256SUMS.txt : empreintes des deux PKG et autres fichiers livrés, hors fichier de sommes lui-même. Calculer après signature/notarisation et éventuelle agrafe du ticket.

Identité de version unique et cohérente entre ZIP, métadonnées VST3, packages, documentation et commit source. Une reconstruction qui modifie les binaires reçoit une identité distincte ; ne pas écraser silencieusement une livraison publiée. L'empreinte du ZIP peut être publiée séparément. Identifiants de bundle VST3 et de package stables, à définir lors d'ARCH-G3; ne pas changer l'identité du plugin à chaque version. Les noms ci-dessus sont le contrat de distribution, non une prescription d'outil de build.

## 3. Installateur
Le README et la présentation de chaque livraison indiquent M3 minimum comme périmètre cible, avec statut de validation explicite. M1/M2 sont hors support officiel, sans blocage matériel imposé. La vérification arm64 reste obligatoire; aucune restriction artificielle par génération CPU n'est demandée.
- Installer le bundle VST3 sous /Library/Audio/Plug-Ins/VST3/ (installation système ; élévation macOS si nécessaire).
- Précontrôler l'architecture, la compatibilité macOS déclarée, le volume cible et l'identité des ressources existantes. Demander la fermeture des DAW utilisant le plugin avant remplacement.
- Détecter version installée et gérer installation initiale, réinstallation, mise à jour sans désinstallation préalable. Ne pas écraser un bundle d'un tiers ou un fichier non identifié.
- Ne supprimer ni modifier les presets utilisateur, banques WT personnelles, préférences, captures ni projets DAW. Ne pas effacer d'autres plugins. Les migrations nécessaires doivent être versionnées et non destructrices.
- En cas d'échec, rapporter l'erreur et ne pas annoncer une installation réussie. Prévoir et tester une stratégie de restauration de l'installation antérieure ou, si impossible, procédure de récupération explicite.
- Retour à une version plus ancienne : avertissement explicite si formats incompatibles, aucune promesse de lecture des presets plus récents; ne pas rétrograder silencieusement les données personnelles.

## 4. Désinstallateur
Package .pkg sans réinstallation du VST3, avec opérations de désinstallation ciblées, résultat vérifiable et chemins autorisés strictement bornés.
- Retirer seulement le bundle SUBDESTRUCKT et les ressources système identifiées comme appartenant au produit; manifeste versionné des fichiers installés et identifiants de reçus cohérents. Un reçu seul n'autorise pas une suppression.
- Refuser une cible inattendue, vérifier l'identité et la version installées, ne pas suivre de lien symbolique vers un chemin étranger. Un désinstallateur d'une ancienne version ne doit pas supprimer silencieusement une version plus récente : refus explicite et instruction d'utiliser celui de la version installée.
- Désinstallation par défaut conservatrice : préserver toutes les données personnelles, bibliothèques utilisateur, préférences et projets. Toute purge personnelle complète est une action distincte et volontaire, avec inventaire et confirmation; ne pas l'inclure comme comportement implicite du PKG standard.
- Aucun glob de suppression large, aucune suppression récursive de répertoires partagés, aucune suppression de caches ou préférences d'autres applications. Nettoyer seulement les reçus exacts associés lorsque pertinent.
- Réexécution sans plugin présent : résultat explicite et sûr. Tester erreurs de permission, installation partielle, ancien désinstallateur et présence d'autres plugins.

## 5. Signature et distribution
Pour la distribution publique, signature Developer ID adaptée au code VST3 et aux packages, notarisation Apple, vérifications de signature, ticket et comportement Gatekeeper selon le parcours de téléchargement. La disponibilité des certificats et de l'accès Apple Developer n'est pas encore établie. Ne jamais qualifier un package non signé/non notarisé de prêt à la distribution publique. Pour les builds internes, indiquer clairement leur statut et les limitations; ne pas imposer la désactivation globale de Gatekeeper. Aucun secret, certificat privé ni identifiant de notarisation commis dans Git.

## 6. Pipeline de livraison
Build arm64 → tests applicables → construction des deux PKG → signature/notarisation selon canal → recette sur macOS Apple Silicon → ZIP + SHA256 → publication autorisée sur GitHub Releases.
Scripts de build/packaging versionnés et reproductibles au niveau des entrées et des versions d'outils; ne pas promettre des binaires bit-exacts signés. Conserver commit, version, commandes, logs et hashes de livraison. Les builds de développement non distribués ne nécessitent pas de release GitHub. Une release ne doit pas être publiée automatiquement du seul fait d'un commit.

## 7. Critères de recette obligatoires à partir d'ARCH-G3
| ID | Critère |
|---|---|
| PKG-01 | ZIP et tous les fichiers obligatoires présents, version/commit/empreintes cohérents. |
| PKG-02 | Installation propre sur Apple Silicon, bundle arm64 présent, découvert et instanciable par un hôte VST3 de recette. |
| PKG-03 | Réinstallation et mise à jour depuis la version précédente si elle existe; identité VST3 conservée, projet de test rechargeable, données personnelles inchangées. |
| PKG-04 | Désinstallation avec DAW fermé; bundle et ressources ciblées retirés, autres plugins et projets intacts, données utilisateur conservées. |
| PKG-05 | Réinstallation après désinstallation, données préservées accessibles; compatibilité explicitement testée dans la portée annoncée. |
| PKG-06 | Ancien désinstallateur, chemins inattendus, échec et absence du plugin traités sans suppression étrangère ni faux succès. |
| PKG-07 | Documentation et statut de signature exacts; signature/notarisation/Gatekeeper vérifiés pour une livraison publique. |
| PKG-08 | Rapport de recette avec macOS, architecture, hôte, versions et preuves; cas non applicables justifiés (première version sans prédécesseur). |

Les tests restent à implémenter et à exécuter. L'outillage précis, le manifeste de fichiers, les identifiants et la stratégie de récupération seront spécifiés pendant ARCH-G3. Aucun package, tag, release ni GO de développement n'est créé par ce document. S0.1 doit encore être clôturé selon ses critères existants.


### Révision plateforme — décision utilisateur du 2026-11-25
Apple M3 de base devient la configuration matérielle minimale cible, en remplacement du M1. M1 et M2 (y compris leurs variantes) sortent du support officiel, sans obligation de blocage logiciel. M4 et générations ultérieures constituent la classe recommandée, sous réserve de recette; le Mac Studio M4 Max de l'utilisateur est la machine principale de développement et de recette physique. La validation sur M3 physique en hôte VST3 est obligatoire avant Beta publique. Aucun résultat de performance M3 n'est encore disponible. Le benchmark M1 virtualisé S0.1 (run 45591261853, rapport sur la branche expérimentale au commit a85f1947e48b1fd8d5d11a9f27df09b78437e5af) et son verdict NO-GO selon les seuils historiques restent inchangés. Ils ne certifient ni M1 ni M3. Cette révision ne gèle pas T2, ne change pas les seuils historiques, ne clôt pas les autres conditions de causalité et n'autorise pas à elle seule ARCH-G3.
