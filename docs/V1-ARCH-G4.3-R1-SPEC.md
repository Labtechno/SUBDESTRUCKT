# SUBDESTRUCKT — ARCH-G4.3-R1 — DSP FINAL CORRECTION : contrats et critères (écrits AVANT toute mesure)

Date : 2026-11-27. Base : `cbca78db9c1b60a0400c5df1e4c415d000548ca1` (ARCH-G4.3 final ; code livré `97100b0`, CI run 43
SUCCESS). Branche unique `feature/arch-g4-full-development`. `main` : `9a07d7654cfce951900cda9e8bec7303b8007a15`.
Version interne proposée : **0.3.3** (aucune étiquette, aucune édition ni chaîne `0.3.3` existante). Format d'état 4
inchangé (aucun paramètre ajouté). Mission corrective ciblée : aucune nouvelle fonctionnalité, ARCH-UI-04 interdit.

Règles : critères fixés ici avant mesure, jamais assouplis ensuite ; les critères et résultats historiques sont
conservés. Un critère historique dont le contrat est **révisé par un arbitrage validé** reste consigné avec son résultat
et est marqué « remplacé par » le critère prospectif correspondant ; il n'est pas présenté comme un échec ouvert.

## 1. Arbitrages validés intégrés

| ID | Décision | Effet |
|---|---|---|
| A-G43-01 | Harmonic Tilt à énergie constante par construction + contrôle des crêtes | contrat §2 ; HT-01..08 ; ZC-15 (clause Tilt « gains = h^(vλ) ») remplacé par HT-02/HT-03 |
| A-G43-02 | Harmonic Shift : balayage conservé | ZC-05 FAIL historique conservé ; ZC-05R (§3) |
| A-G43-03 | Phase Scatter accepté ; critère de crête révisé | ZC-13 (c) FAIL historique conservé ; ZC-13R (§4) |
| A-G43-04 | Captures R2 : correctif inter-Fs conservé, recette complétée | R2-02 FAIL historique conservé ; R2-02R (§5) |

## 2. Harmonic Tilt — contrat révisé (A-G43-01)

Notations : étage Tilt d'un emplacement Z, coefficients d'entrée de l'étage cₕ (sortie de l'étage précédent, pas
nécessairement la forme d'origine), gains tₕ = h^(v·λ(σ)) (inchangés : caractère sombre/brillant conservé), gain de bande
lue bₕ (profil mip, coupure H en morph Spectral, 1 en Raw), domaine D = {h ≥ 2} si Keep Fundamental, sinon {h ≥ 1}.

1. **Pondération** : c′ₕ = tₕ·cₕ.
2. **Normalisation intrinsèque (toujours active, indépendante de Gain Compensation)** : Eᵢₙ = Σ_{h∈D} bₕ²|cₕ|²,
   Eₜ = Σ_{h∈D} bₕ²|c′ₕ|² ; si Eᵢₙ > ε·(Eᵢₙ+Eₜ) et Eₜ > 0 : c″ₕ = s·c′ₕ pour h ∈ D avec s = √(Eᵢₙ/Eₜ) ; sinon (énergie
   nulle ou négligeable, ε = 1e−24) : c″ = c (identité, aucune énergie créée). c₁ (fondamentale protégée) inchangé.
3. **Contrôle des crêtes** : pᵢₙ = crête du cycle à bande lue resynthétisé depuis c, pₒᵤₜ = crête depuis c″ ; si
   pₒᵤₜ > κ·pᵢₙ avec **κ = 2 (+6,02 dB)**, réduction de gain continue des rangs de D : facteur unique α ∈ ]0,1] tel que
   la crête soit ramenée à κ·pᵢₙ (calcul direct sans fondamentale protégée ; recherche monotone bornée sinon, la crête
   étant convexe en α). Aucun écrêtage, aucune saturation. L'énergie n'est alors plus conservée (exception mesurée et
   comptée). Justification de κ : à énergie égale, un spectre plus brillant est naturellement plus « pointu » (facteur
   de crête plus élevé) ; +6 dB autorisent l'éclaircissement musical usuel tout en bornant l'élévation de crête par
   étage, indépendamment du spectre et de la compensation globale.
4. **Composition** : chaque étage Tilt est normalisé et contrôlé par rapport à SON entrée (deux Tilt : deux étages).
   **Gain Compensation globale** (inchangée pour les autres transformations) : son énergie cible est l'énergie d'origine
   multipliée par les facteurs énergétiques des réductions de crête des étages Tilt, afin qu'elle ne réintroduise
   jamais une crête retirée (pas de double compensation) ; avec Tilt seul et sans réduction de crête, son gain vaut 1.
5. Profondeur effective nulle : étage non exécuté (identité bit à bit). Déterminisme, temps réel : aucune allocation,
   aucun verrou ; deux FFT inverses de 2048 points supplémentaires par étage Tilt et par resynthèse (cadence inchangée,
   au plus une resynthèse toutes les 128 échantillons moteur), tampons préalloués.

| ID | Critère (seuils fixés avant mesure) |
|---|---|
| HT-01 | Neutralité : Z Master = 0 ou Depth = 0 (Keep Fundamental et Gain Compensation 0/1, deux emplacements) : rendu bit à bit identique au moteur sans Z |
| HT-02 | Énergie : Tilt seul, 4 banques × 4 formes × v ∈ {0,05 ; 0,25 ; 0,5 ; 0,75 ; 1} × σ ∈ {−1 ; −0,5 ; 0 ; 0,5 ; 1} × Keep Fundamental 0/1 × 4 profils de bande (1024, H = 64, H = 8, profil mip partiel w = 0,4) : sans réduction de crête, \|Eₒᵤₜ/Eᵢₙ − 1\| ≤ 1e−9 sur D ; avec réduction : Eₒᵤₜ ≤ Eᵢₙ ; nombre de réductions rapporté |
| HT-03 | Fondamentale : ON : c₁ de sortie == c₁ d'entrée (égalité exacte) ; OFF : c″ₕ = s·tₕ·cₕ avec un facteur s commun à tous les rangs (écart relatif ≤ 1e−9), hors réduction de crête |
| HT-04 | Cas limites : spectre nul (sortie nulle exacte), quasi nul (1e−150), sinus seul, dent de scie, spectre creux (Metal), spectre dense aléatoire de 1024 harmoniques (forme utilisateur), bande H = 1 et H = 2, rang 1024 : 0 NaN/Inf, Eₒᵤₜ ≤ Eᵢₙ·(1 + 1e−9), crête ≤ κ·pᵢₙ·(1 + 1e−9) |
| HT-05 | Crêtes : (a) pour tous les cas HT-02/HT-04, crête du cycle de sortie ≤ κ·pᵢₙ·(1 + 1e−9) ; (b) scénario isolé de référence (Tilt seul, v = 1, σ = 1, sans Gain Compensation, Metal, note 45, 2 s) : crête avant ELP ≤ crête du même rendu Z désactivé + 6,02 dB (+0,1 dB de marge de lecture) et **0 engagement ELP** ; (c) rejeu des protocoles historiques ZC-16 (Metal, Basic) et « compensation forcée » : **0 engagement ELP, garde 0**, crêtes avant ELP rapportées (historique : +40,9 / +46,8 / +30,3 / +78,9 dBFS) |
| HT-06 | Composition : Tilt (A) + chacune des 7 autres et Tilt + Tilt, dans les deux ordres (z_order 0/1), v = 0,8, σ ∈ {−1, 1} : finis, 0 engagement ELP, crête avant ELP ≤ crête de la même configuration avec la profondeur du Tilt à 0 + 6,02 dB par étage Tilt (+0,1 dB) ; équivalence des ordres rapportée (§6) |
| HT-07 | Modulation : balayages LFO de Depth (0,5 Hz, 0→1), Shape (−1→1), Z Master, route CROSS OP1→WT.Z (audio), transitions rapides (LFO 20 Hz) : finis ; saut maximal entre échantillons ≤ 1,5 × le maximum des rendus statiques (profondeurs 0 ; 0,25 ; 0,5 ; 0,75 ; 1 × σ −1/0/1) ; rapport d'énergie entre fenêtres consécutives de 5 ms ≤ 2 (6 dB) pendant la tenue |
| HT-08 | Coût (mesure, sans seuil ; même machine, mêmes conditions) : ns/échantillon P50/P95 Tilt statique, profondeur modulée, Tilt + Shift ; µs par resynthèse ; comparaison avec 0.3.2 (`cbca78d`) reconstruit sur la même machine ; écart significatif (> 20 %) documenté |

## 3. Harmonic Shift — ZC-05R (A-G43-02)

Balayage conservé : c′ₕ = interpolation de c en h − k, k = 8·v·λ(σ). Une correction minimale n'est admise que si un
défaut de continuité est démontré par ZC-05R (c)–(e) ; elle doit préserver le principe du balayage et l'exactitude des
déplacements entiers.

| ID | Critère |
|---|---|
| ZC-05R | (a) k entier : c′ₕ = c_{h−k} exact (k = 1, 2, 5, 16, −3, −8) ; (b) k fractionnaire : \|c′ₕ\| = (1−f)\|cᵢ\| + f\|cᵢ₊₁\| (≤ 1e−12 relatif) ; (c) **continuité en k** : pour 2000 valeurs de k ∈ [−8 ; 16] et δ = 1e−6, max_h \|c′ₕ(k+δ) − c′ₕ(k)\| ≤ 1e−3 · max_h \|cₕ\| (dent de scie, Metal, Basic) ; (d) **bornes du domaine** : continuité de (c) aussi quand le rang source franchit 1 et 1024 ; (e) près de zéro : \|c′(k) − c\| → 0 quand k → 0 (k = ±1e−6 : ≤ 1e−5 · max\|c\|) ; neutralité exacte à v = 0 (ZC-02) ; (f) déterminisme ; (g) aucune composante créée : c′ₕ = 0 lorsque la source sort de ]0 ; 1025[, aucune énergie au-delà de la bande lue, DC = 0 ; (h) modulation (LFO 20 Hz sur la profondeur) : fini, saut maximal entre échantillons ≤ 1,5 × le maximum statique |

## 4. Phase Scatter — ZC-13R (A-G43-03)

Algorithme inchangé (empreintes des rendus Scatter conservées). Facteur de crête : descriptif.

| ID | Critère |
|---|---|
| ZC-13R | (a) déterminisme (deux rendus identiques ; indépendant de la graine d'instance) ; (b) spectre d'amplitude inchangé (≤ 0,01 dB par harmonique) ; (c) forme temporelle modifiée : écart du cycle ≥ −20 dB à v = 1 (seuil historique Z7 de ZC-05) ; (d) FM via CROSS WT>OP1 : écart ≥ −30 dB (seuil historique ZC-13 a) ; (e) non-linéarités : après Harmonic Fold (B) ET après N1 (drive 18 dB) : ≥ 3 bandes de tiers d'octave à ≥ 1 dB (seuil historique ZC-13 b) ; (f) modulation LFO 20 Hz de la profondeur : fini, saut ≤ 1,5 × statique ; (g) neutralité exacte à profondeur nulle ; facteur de crête rapporté (descriptif) |

## 5. Captures R2 — R2-02R (A-G43-04)

Protocole : capture S2 (Decay = 1) réalisée à Fc ∈ {44,1 ; 48 ; 88,2 ; 96 kHz}, restituée à Fr ∈ {44,1 ; 48 ; 88,2 ;
96 kHz} (16 couples), deux sources (dent de scie note 45 ; spectre dense Metal note 52). Énergies par bande de tiers
d'octave (200 Hz – 0,45·min(Fc, Fr)) d'une trame de 100 ms à temps écoulé égal, normalisées par la durée (Parseval).
Bande « résoluble » : largeur ≥ 8 bins de la grille la plus grossière (Fs moteur / taille FFT) des deux fréquences.
Seuils (choix perceptifs fixés a priori : ~1 dB = ordre de grandeur du seuil de discrimination de niveau) :

| ID | Critère |
|---|---|
| R2-02R | (a) énergie totale restituée ±1,0 dB (16 couples, 2 sources) ; (b) écart pondéré par l'énergie sur les bandes résolubles ≤ 0,5 dB ; (c) chaque bande résoluble à ≥ −20 dB du maximum : \|écart\| ≤ 1,0 dB ; (d) bandes non résolubles et bandes faibles (−20 à −60 dB) : **listées** avec niveau, largeur en bins et écart (descriptif, jamais ignorées) ; (e) Fc = Fr : spectre tenu = capture (≤ 1e−6) ; (f) WAV A/B du moteur réel, même gain pour les fichiers comparés (aucune normalisation indépendante), identifiés par Fc → Fr |

R50 reste IMPLEMENTED tant que l'écoute A/B et la recette physique ne sont pas validées par le propriétaire.

## 6. Audit des restrictions Z (documentation)

| ID | Contenu |
|---|---|
| ZA-01 | Cadence effective de resynthèse et latence de réaction selon Fs (44,1 / 48 / 88,2 / 96) et Quality (Eco/Normal/High) : tableau calculé et vérifié par compteur ; réponse à un échelon de profondeur (temps jusqu'à 90 % du changement spectral) ; atténuation d'une modulation sinusoïdale de profondeur (1, 10, 50, 200, 1000 Hz) — mesures descriptives et rendus |
| ZA-02 | Matrice 8 × 8 (types A × B, z_order 0/1) : rendu A>B vs B>A — identique bit à bit, différent en niveau seulement, ou différent ; ordre effectif et restriction par paire |

## 7. Non-régression et livraison

TRANSPORT AUTHORIZATION inchangé (TA 24/24, TA-xxw 9/9 attendus) ; empreintes 0.3.1 (ZC-03, D2R-03) ; tous les
exemples audio du manuel non concernés par Tilt ou Shift identiques octet par octet à 0.3.2 ; LB-01..13, INV-01 ;
pluginval rigueur 10 ; RTSan ; Playwright. Échecs historiques conservés : ELP-01, D2-04, D2-04c, AUT-02 (c), R2-02,
ZC-05, ZC-13 ; critère remplacé : ZC-15 (clause Tilt), par HT-02/HT-03.
