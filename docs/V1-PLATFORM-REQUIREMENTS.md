# SUBDESTRUCKT — VSTi Platform Requirements V1
Date : 2026-11-24. Statut : décisions produit + exigences de benchmark. Aucun benchmark exécuté.

## 1. Décisions validées
- **Cible desktop V1 confirmée : macOS sur Apple Silicon natif (arm64).**
- **Minimum matériel : Apple M1.**
- Apple M1 Pro/Max/Ultra, M2/M3/M4 et générations ultérieures sont dans la classe cible sous réserve de validation de la version macOS/hôte au moment de la release.
- **Windows est hors périmètre produit.**
- Intel Mac et Rosetta ne sont pas des cibles de compatibilité.
- Linux n'est pas engagé dans le périmètre V1; toute prise en charge future nécessitera une décision produit explicite et une nouvelle matrice de QA.
- **Format logiciel actuel : VST3 instrument uniquement. Le standalone est hors périmètre actuel.** Tout format supplémentaire devra être explicitement arbitré.
- **Le hardware est hors périmètre de la phase actuelle.** Aucun budget de commandes physiques, manifest de capacités hardware, processeur embarqué, Eurorack/Desktop ni portage ne doit être traité ou utilisé comme contrainte. Le DSP reste séparé proprement de l'UI et de l'hôte pour la maintenabilité du VSTi.

## 2. Philosophie performance
Le minimum M1 n'est pas une simple condition de lancement. Une machine officiellement supportée doit satisfaire une recette de performance définie.

**Principe proposé :** un Mac M1 de base doit pouvoir exécuter au moins une instance du preset de stress de référence en **Quality Normal**, à 48 kHz et à une taille de buffer réaliste à fixer, sans dropout et sous un plafond CPU mesuré.

Eco reste un mode d'économie CPU / live; il ne doit pas être obligatoire pour rendre la configuration minimale utilisable. High peut exiger une machine supérieure et sera documenté comme tel si les benchmarks le démontrent.

## 3. Benchmarks obligatoires avant release
Mesurer au minimum :
- M1 base comme machine plancher;
- une génération Apple Silicon plus récente comme référence recommandée;
- VST3 uniquement;
- **44.1 / 48 / 88.2 / 96 kHz officiellement testés**; 192 kHz non garanti en V1;
- buffers 32 / 64 / 128 / 256 / 512 échantillons selon support de l'environnement;
- Quality Eco / Normal / High;
- preset vide, preset courant représentatif, stress WT/FM/Cross, stress Filter/Destruction, stress Global Mod, état maximal cumulé;
- une instance et plusieurs instances;
- UI ouverte/fermée, analyseurs visibles/masqués;
- charge moyenne, pics, xruns/dropouts, mémoire, latence déclarée, temps de chargement des presets/captures.

Les critères de GO chiffrés seront définis avant de considérer le M1 officiellement supporté, afin d'éviter d'abaisser les seuils après les résultats.

## 4. RAM / stockage / OS — non encore validés
Ces valeurs restent candidates et ne sont PAS des décisions utilisateur :
- RAM : 8 Go minimum candidat, 16 Go recommandé candidat;
- espace disque : à mesurer après banques factory et ressources;
- version minimale de macOS : à décider près de la release selon JUCE, VST3 SDK, outils de build et hôtes réellement testés;
- GPU : aucune exigence dédiée prévue; l'UI doit rester raisonnable sur GPU intégré Apple Silicon.

## 5. Conséquences d'architecture
1. CI/QA V1 n'a aucune obligation Windows.
2. Pas de code conditionnel Windows dans le chemin produit V1.
3. L'architecture ne doit toutefois pas utiliser macOS comme modèle DSP : filesystem, UI, audio host et accélérations natives restent aux frontières de plateforme.
4. Les résultats de calcul ne doivent pas dépendre d'une API graphique ou de l'UI.
5. Les optimisations SIMD Apple Silicon/NEON sont autorisées si elles préservent le comportement validé du VSTi et restent testables.
6. Pas de compatibilité Intel/Rosetta à maintenir ni tester.
7. Aucun travail hardware n'entre dans les critères V1 actuels.

## 6. Risque produit assumé
Le choix macOS Apple Silicon réduit volontairement le marché adressable. En contrepartie, il réduit fortement la matrice de support et permet de concentrer le travail sur la qualité DSP, la stabilité et l'UX du VSTi. Cette restriction doit rester une décision produit consciente, pas une conséquence accidentelle de code non portable.


## 7. Packaging macOS — VALIDÉ (2026-11-25)
À partir d'ARCH-G3, chaque version installable VST3 arm64 (test, alpha, beta, stable) inclut un ZIP versionné avec Installer.pkg et Uninstaller.pkg, README_FR, CHANGELOG et SHA256SUMS. Installation système /Library/Audio/Plug-Ins/VST3/; désinstallation ciblée préservant les données personnelles; recette sur Apple Silicon. Signature Developer ID et notarisation exigées pour distribution publique, sous réserve d'obtention des certificats (sans ceux-ci, distribution publique bloquée). Contrat complet : [V1-PACKAGING-RELEASE-CONTRACT.md](V1-PACKAGING-RELEASE-CONTRACT.md). Les spikes non installables sont exclus. Aucun test packaging exécuté à ce stade.
