# SUBDESTRUCKT — Contrat des transformations Z (ARCH-G4.3, 0.3.2)

Catalogue validé par le propriétaire (huit transformations). Numérotation fonctionnelle Z1–Z8 ≠ indices persistés.
Formalisation et critères écrits avant mesure : `docs/V1-ARCH-G4.3-SPEC.md` §7 ; code : `source/dsp/ZSpectral.h`
(fonctions pures), `source/engine/Engine.cpp` (`spectralUpdate`, lecture WT).

| Catalogue | Transformation | Indice persisté (`wt.za.type`, `wt.zb.type`) | Domaine | Statut |
|---|---|---|---|---|
| Z1 | Harmonic Tilt | 4 | spectral (table) | ajoutée ARCH-G4.3 |
| Z2 | Harmonic Fold | 1 | forme d'onde (après lecture, ADAA1) | historique, inchangée |
| Z3 | Phase Warp | 2 | phase (avant lecture) | historique, inchangée |
| Z4 | Harmonic Mask | 5 | spectral | ajoutée |
| Z5 | Harmonic Focus | 6 | spectral | ajoutée |
| Z6 | Harmonic Shift | 7 | spectral | ajoutée |
| Z7 | Phase Scatter | 8 | spectral | ajoutée |
| Z8 | Spectral Mutation | 3 | phase et amplitude (par cycle) | historique, inchangée |

## 1. Paramètres

Par emplacement : type, profondeur, courbe, **Shape** (`wt.za.shape`, `wt.zb.shape`, [−1, 1], ajoutés en version 4) ;
communs : Z Master, ordre A→B / B→A, Keep Fundamental, Gain Compensation. v = profondeur × courbe(Z Master) ∈ [0, 1] ;
σ = Shape ; λ(σ) = 0,5 + 1,5σ.

## 2. Fonctions (coefficients harmoniques complexes cₕ, h = 1…1024)

| Z | Fonction | Shape |
|---|---|---|
| Tilt | gₕ = h^(v·λ) (6,02·v·λ dB par octave de rang) | −1 : sombre ; 0 : +3 dB/oct ; +1 : +12 dB/oct |
| Mask | gₕ = 1 − v(1 − mₕ), mₕ = ½ − ½cos(2πh/p), p = 3·(8/3)^σ (σ ≥ 0) ou 3·(2/3)^(−σ) | p ∈ [2 ; 8] continu |
| Focus | gₕ = (1 − v) + v·exp(−log₂(h/C)²/(2b²)), C = 2^(3,5(σ+1)), b = 0,25 + 1,25(1 − v) octave | centre C ∈ [1 ; 128] |
| Shift | c′ₕ = interpolation (amplitude linéaire, phase circulaire pondérée) de c en h − k, k = 8·v·λ | sens et étendue (−8 … +16 rangs) |
| Scatter | c′ₕ = cₕ·e^(i·v·π·ρₕ(σ)), ρ = hachage déterministe, 8 motifs interpolés | motif |

## 3. Règles communes

- Z = 0 ou profondeur 0 : chemin spectral non engagé, rendu bit à bit inchangé (ZC-02).
- Composition : spectral (table) → phase (Warp, Mutation) → lecture → forme d'onde (Fold). L'ordre A→B / B→A s'applique
  entre transformations d'un même domaine.
- Morph : Linear/Stepped = combinaison complexe linéaire des formes (identique à l'interpolation temporelle) ; Spectral
  = morph spectral existant. Limitation de bande appliquée **après** transformation (profil des niveaux mip ; coupure à
  H en morph Spectral ; Raw : 1024 harmoniques). DC = 0.
- Keep Fundamental : c₁ de sortie = c₁ d'entrée. Gain Compensation : énergie de Parseval conservée sur la bande lue
  (fondamentale préservée exclue), gain borné [0,25 ; 4].
- Resynthèse : FFT inverse 2048 toutes les 128 échantillons moteur, fondu entre tables, combinaison des formes mise en
  cache (une modulation de Z seule ne la refait pas) ; aucune allocation, aucune FFT par échantillon. La modulation
  audio de Z (CROSS) est suivie à cette cadence par les transformations spectrales (limitation documentée).
- Déterminisme : aucune dépendance au temps, à la graine d'instance ni à un tirage par échantillon.

## 4. Points ouverts (arbitrage du propriétaire)

- **A-G43-01 — niveau de Harmonic Tilt brillant** (ZC-16) : la formule spécifiée n'est pas bornée ; sans compensation,
  +78,9 dBFS avant ELP (Tilt v = 1, σ = 1, Metal) ; avec compensation, la butée [0,25 ; 4] laisse jusqu'à +30,3 dBFS.
  La sortie reste finie (ELP, garde 0) mais l'ELP ne doit pas servir de correction. Options : (a) Tilt à énergie
  constante par construction (redistribution pure, Gain Compensation sans effet pour Tilt) ; (b) borne de gain par
  rang ; (c) butée de compensation élargie pour les transformations spectrales (compensation exacte, sans risque
  d'instabilité) ; (d) inchangé, documenté.
- **A-G43-02 — progressivité de Harmonic Shift** (ZC-05) : sur un spectre creux et irrégulier (Metal), la distance au
  spectre d'origine n'est pas monotone avec la profondeur (coïncidences périodiques de partiels). Options : (a) garder
  (balayage de rang, comportement attendu d'un déplacement) ; (b) variante « mélange » |c′| = (1 − v)|c| + v|shift_K(c)|
  avec K fixé par Shape (progressivité garantie, timbre différent).
- **ZC-13 (c)** : facteur de crête de Phase Scatter +0,91 dB (< 1 dB) sur dent de scie ; l'intérêt musical est démontré
  par (a) et (b). Voir le rapport G4.3 §7.
