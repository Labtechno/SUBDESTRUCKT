# SUBDESTRUCKT — ARCH-G4.2 : transformations Z (R19) — recherche historique et propositions NOUVELLES

Date : 2026-11-27. Statut : **R19 reste BLOCKED**. Ce document n'autorise aucune implémentation : les propositions de la
section 3 sont **nouvelles**, inventées pendant ARCH-G4.2, et ne prétendent pas reconstituer le catalogue historique.
Elles attendent l'arbitrage du propriétaire (P0-C29 : « les cinq autres restent bloquées jusqu'à récupération historique
ou QCM Z dédié »).

## 1. Recherche ciblée (résultat : catalogue historique introuvable)

| Source | Méthode | Résultat |
|---|---|---|
| Dépôt git, **toutes branches**, 101 commits | `git log --all -S "Spectral Mutation"` ; `git grep` des noms attestés, de « huit transformations », « 8 transformations », « cahier d'origine », « catalogue Z », « QCM Z » sur chaque commit | Plus ancienne mention : 745e583 (2026-11-24, V1-DESIGN-BASELINE) : « Libellés exacts des huit transformations et des options historiques Z non intégralement disponibles dans le contexte exploitable … Harmonic Fold, Phase Warp, Spectral Mutation attestés dans les échanges. Ne pas inventer les autres. » Aucun autre nom. |
| Documents du dépôt (docs/, manuel) | mêmes motifs | P0-C29, AUD-P0-01, QCM n° 28/29, rapports G4/G4.1 : tous renvoient au catalogue manquant ; aucun libellé supplémentaire. |
| Documents locaux hors dépôt (`C:\Users\jerem\SUBDESTRUCKT\docs`, `spikes`, `tools`) | mêmes motifs | Copies des documents de conception ; aucun libellé supplémentaire. |

Conclusion : le « cahier d'origine » cité par V1-DESIGN-BASELINE n'est disponible dans aucune source accessible. Les
trois transformations attestées restent les seules implémentées : Harmonic Fold, Phase Warp, Spectral Mutation.

## 2. Contraintes préservées (inchangées)

Deux emplacements Z en série (ordre A→B ou B→A), profondeurs et courbes individuelles, Z Master, préservation de la
fondamentale configurable, compensation de gain, neutralité exacte à Z = 0, aucune FFT par échantillon, aucun nouveau
moteur. Toute transformation ajoutée devra prendre un indice **après** les trois existantes dans le choix Z (enum
append-only : les projets existants restent identiques), sans modifier les 695 StableID.

## 3. Propositions NOUVELLES (non historiques, non implémentées)

Chacune opère sur la forme lue (cycle unique, périodique) ou sur les spectres précalculés des formes (1024 harmoniques,
déjà disponibles pour le morph spectral), ce qui conserve la période et permet la préservation de la fondamentale.

| # | Nom proposé | Principe | Z = 0 | Fondamentale | Coût / risque |
|---|---|---|---|---|---|
| N-Z1 | Spectral Tilt | pondère l'harmonique k par k^(±α·z) (assombrir / éclaircir) | identité | harmonique 1 inchangée (k^0) | faible (spectres précalculés, iFFT au rythme du morph) ; risque : recouvre en partie le filtre |
| N-Z2 | Odd/Even Balance | atténue progressivement les harmoniques paires (vers le carré) ou impaires (vers une octave creuse) | identité | conservée (impaire) ; en mode « impaires » optionnellement protégée | faible ; risque : son proche de formes existantes |
| N-Z3 | Formant Shift | décale l'enveloppe spectrale de la forme sans déplacer les harmoniques (voix, nasalité) | identité | conservée (seule l'enveloppe se déplace) | moyen (estimation d'enveloppe hors RT) ; risque : artefacts à fort décalage |
| N-Z4 | Sync Window | relit le cycle à un rapport 1 + 7·z avec fenêtre de cycle (timbre « hard sync ») | identité | période conservée ; énergie de la fondamentale réduite (compensation optionnelle) | faible ; risque d'aliasing : ADAA ou mip-map nécessaires |
| N-Z5 | Phase Quantize | quantifie la phase de lecture en 2^(12 − 10·z) pas (escalier, grain numérique) | identité (12 bits ≈ transparent) | conservée | très faible ; risque d'aliasing marqué (à borner selon Quality) |
| N-Z6 | Harmonic Comb | creuse des encoches périodiques dans la série harmonique (une harmonique sur n) | identité | protégée | faible ; recouvre en partie F2 Comb |

Six propositions sont données pour permettre un choix ; le nombre cible historique (huit au total) n'impose pas d'en
retenir cinq.

## 4. Arbitrage demandé (B-01, inchangé dans son principe)

- A. Retenir certaines propositions N-Z1…N-Z6 (identifiées comme nouvelles dans l'UI, le manuel et l'état), avec un
  contrat par transformation (critères avant mesure : neutralité, fondamentale, gain, aliasing, coût).
- B. Ouvrir un QCM Z dédié pour d'autres pistes.
- C. Rester à trois transformations en V1 (cible de huit abandonnée ou reportée).
- D. Fournir le cahier d'origine (récupération historique) : seule voie qui débloque R19 sans invention.

Tant qu'aucune option n'est choisie, aucune transformation supplémentaire n'est implémentée et R19 reste BLOCKED.
