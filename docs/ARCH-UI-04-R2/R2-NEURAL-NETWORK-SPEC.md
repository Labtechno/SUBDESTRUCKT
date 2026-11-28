# ARCH-UI-04-R2-A — réseaux : Adaptive Neural Network et Dual View (DA-A12, A13, A14)

Périmètre : **FM Network** (16 emplacements), **CROSS Network** (39 emplacements) et **Global Modulation Matrix**
(64 liens au plus). Les faits structurels proviennent de `source/core/Topology.h` et `source/core/ModMatrix.h` (lecture
seule). Les faits de télémétrie proviennent de `source/engine/Telemetry.h`. Aucune de ces sources n'est modifiée.

## 1. Modèle (inchangé)

| Réseau | Nœuds | Emplacements | Profondeur | Identité de route | Classe causale | Transactions |
|---|---|---|---|---|---|---|
| FM Network | OP1…OP4 | 16 (4 × 4, dont 4 auto-rétroactions `OPn FB`) | `fm.opX_opY.depth`, 0…8 rad | `StableRouteID` (`Route::id`) | I / D (T2), figée à la création (B-T2) | Ajout et retrait validés hors temps réel, avec Undo. 10 topologies préréglées. |
| CROSS Network | Sources WT, OP1…OP4, BUS ; ports WT (phase, FREQ, X, Y, Z, LVL, CHAR) et OP1…OP4 | 39 (WT→OPn : 4 ; {OP1…OP4, BUS}→7 ports WT : 35) | `cross.*.depth` : 0…8 rad (WT→OPn et ports de phase), 0…8 « x » (port FREQ), −1…+1 (ports X, Y, Z, LVL, CHAR). Poids du bus `cross.bus.w1…w4` (−1…+1). | idem | idem | idem |
| Global Mod Matrix | 33 sources (`ModSource`), destinations = paramètres modulables | 64 liens au plus (`kMaxModRoutes`), dont 16 à cadence audio au plus | −1…+1 × échelle de la destination ; courbe (7), polarité, inversion, offset, source « via », lissage, cadence, activation, verrou | `StableModRouteID`, jamais réutilisé | — | Ajout, retrait et édition validés, avec Undo |

Les 55 emplacements FM/CROSS sont fixes (`kMaxRoutes = 55`). Une paire non autorisée (`routeSlot = −1`) **n'existe
pas** et ne peut pas être créée (INV-05).

## 2. États de route (NN-01)

| État | Définition | Représentation NEURAL | Représentation TABLE | Texte |
|---|---|---|---|---|
| Inexistante | Paire non autorisée par la topologie, ou destination non modulable | Aucune liaison. Pas de port d'accroche. | Cellule vide, non focalisable, hachurée | — |
| Disponible inactive | Emplacement autorisé, route absente. La profondeur est conservée sans effet. | Liaison fantôme en pointillé fin, au survol et au focus seulement | Cellule « OFF », profondeur grisée | OFF · ADD: I / ADD: D |
| Active, profondeur 0 | Route présente, profondeur nulle | Trait continu fin, couleur du domaine à 30 % | « 0.00 » | ON · 0 |
| Positive | Route présente, profondeur > 0 | Trait continu, épaisseur ∝ \|profondeur\|, flèche pleine | Valeur positive | — |
| Négative | Profondeur < 0 (domaines bipolaires seulement) | Trait continu à tirets longs, flèche creuse | Valeur avec `−` | — |
| Retard causal | Classe D (T2) | Marque « D » sur la liaison et pastille de délai au port d'arrivée | Colonne CLASS = D | D (T2) |
| En transition | `edgeGain` entre 0 et 1 (fondu d'ajout ou de retrait), `transitionPhase` ≠ 0 | Opacité = `edgeGain` | Badge « FADE » | — |
| Lien inactif (matrice) | `ModInactive` ≠ Active (Disabled, NoSource, DestinationNotModulatable, AudioRateUnsupported, AudioRateBudget, FollowerCycle) | Trait pointillé, gris `inactive` | Colonne STATUS avec la raison | Raison en anglais |

La couleur n'est jamais le seul porteur (INV-12) : chaque état combine un style de trait, une forme de flèche et un texte.

**Polarités (matrice globale).** Le signe de la profondeur est représenté comme ci-dessus. La polarité de la route
(`Centered` ou `Free`) est marquée à l'origine de la liaison : losange plein pour Centered, losange creux pour Free.
L'inversion est marquée « INV » et l'offset (mode Free) est affiché au focus. Le réseau FM n'a que des profondeurs
positives (0…8 rad). Le réseau CROSS a des ports bipolaires (X, Y, Z, LVL, CHAR).

**Accès aux routes inactives (NN-27).** Le filtre SHOW AVAILABLE affiche toutes les liaisons fantômes autorisées d'un
nœud ou de tout le réseau. Une liaison fantôme s'active par Entrée ou double-clic, via la transaction validée existante
(classe causale calculée et affichée avant validation : « ADD: I » ou « ADD: D (T2) »). Pour la matrice, l'ajout passe
par le sélecteur de source et de destination modulable existant.

## 3. Profondeur configurée et activité (NN-02)

| Grandeur | Disponible | Source | Usage |
|---|---|---|---|
| Profondeur configurée (Base) | Oui | Paramètre `*.depth`, `ModRoute::depth` | Épaisseur du trait, valeur |
| Profondeur effective FM/CROSS | Oui, par bloc | `effectiveDepth[55]` (après Master/direction et lissage) | Second trait intérieur `effective` quand il diffère de la Base |
| Gain de transition | Oui, par bloc | `edgeGain[55]`, `transitionPhase` | Opacité pendant un fondu |
| Valeur instantanée des sources de modulation | Oui, par bloc | `sources[kNumModSources]` | Activité du **nœud source** (pulsation ∝ valeur) |
| Valeur effective de la destination | Oui, si modulée | `effective[703]`, `modulated[703]` | Anneau Effective du nœud destination |
| Contribution instantanée d'un lien global | **Non publiée** | — | Approximation côté UI possible (source × profondeur), mais elle ignore la courbe, « via », le lissage et la polarité. Elle est interdite, sauf mention ESTIMATE (OD-07). |
| Niveau de signal par route FM/CROSS | **Non publié** | — | Interdit sans évolution du moteur, sous autorisation distincte (T-24) |

La vue NEURAL DOIT distinguer visuellement la **profondeur configurée** (structure) de l'**activité** (source,
Effective). L'activité n'est jamais déduite de la seule profondeur.

## 4. Disposition et adaptativité (DA-A13)

| ID | Exigence |
|---|---|
| NN-09 | Positions de nœuds **stables**, définies par une grille fixe et non par une simulation physique. FM : OP1…OP4 en losange, ou selon la topologie préréglée sélectionnée à titre de présentation seulement. CROSS : colonne WT (ports dans l'ordre phase, FREQ, X, Y, Z, LVL, CHAR) face à la colonne OP1…OP4 et BUS. Matrice : sources groupées par famille (MIDI, ENV, LFO, RANDOM/CHAOS, MACRO, FOLLOWER, Y2) à gauche, destinations groupées par domaine (WT, FM, CROSS, DST, FLT, ENV/LFO, OUT) à droite. |
| NN-10 | Aucun nœud ne se déplace pendant un geste, ni à la suite d'un ajout ou d'un retrait de route. Seules les liaisons apparaissent et disparaissent. |
| NN-11 | Filtres : par moteur ou domaine, par source, par destination, par état (actives, inactives, retard causal, modulées) et par texte (filtre 0.4.1 conservé). Un filtre n'écrit rien (INV-03). |
| NN-12 | Focus : sélectionner un nœud met en évidence ses liaisons entrantes et sortantes et estompe les autres à 25 %, seuil candidat. |
| NN-13 | Densité maximale lisible : 16 liaisons FM, 39 CROSS, 64 matrice simultanées. Au-delà de 24 liaisons visibles (seuil candidat), les destinations d'un même groupe sont regroupées en faisceau avec compteur, et dépliées au focus. |
| NN-14 | Pas de chevauchement de libellés : les libellés de liaison n'apparaissent qu'au survol et au focus. Les libellés de nœud sont fixes. |
| NN-15 | Clavier : flèches pour aller de nœud en nœud, Tab pour parcourir les liaisons d'un nœud, Entrée pour éditer la profondeur, Suppr pour retirer (transaction validée), Échap pour sortir |
| NN-16 | Au zoom 75 %, les libellés de nœud restent ≥ au seuil typographique critique (OD-06) |

## 5. Dual View NEURAL / TABLE (DA-A14)

| ID | Exigence |
|---|---|
| NN-17 | Les deux vues lisent le même modèle (`Topology`, `ModMatrix`, paramètres) et écrivent par les mêmes chemins (`ParamLink`, transactions validées). Aucun état propre à une vue n'est persisté. |
| NN-18 | Les identités de route (`StableRouteID`, `StableModRouteID`) et les StableID de profondeur sont identiques et visibles dans les deux vues (rang 8 de l'Inspector). |
| NN-19 | La sélection est synchronisée : une route ou un nœud sélectionné dans une vue l'est dans l'autre et dans l'Inspector. |
| NN-20 | Basculer de vue n'écrit aucun paramètre, ne crée aucune entrée Undo et ne marque pas le preset comme modifié (INV-03). |
| NN-21 | Une même action produit la même transaction dans les deux vues : même entrée Undo, mêmes gestes hôte (un `begin`/`end` par glisser de profondeur). |
| NN-22 | TABLE reste la vue de référence pour l'édition fine : colonnes SOURCE→DEST, DEPTH, CLASS, STATUS et, pour la matrice, CURVE, VIA, RATE, LOCK. |
| NN-23 | Le choix de vue est mémorisé par outil (état d'interface, hors preset). La vue par défaut est NEURAL (DA-A12). |
| NN-24 | Les deux vues affichent la restriction ou la classe causale à l'identique (texte « D (T2) »). |

## 6. Ce que le réseau ne fait pas (NN-03..NN-08)

| ID | Interdit |
|---|---|
| NN-03 | Créer une connexion libre : seules les 55 paires FM/CROSS autorisées et les liens de matrice valides (source, destination modulable) sont créables |
| NN-04 | Modifier une classe causale existante ou la topologie hors transaction validée |
| NN-05 | Réordonner le signal ou les nœuds du moteur |
| NN-06 | Afficher une activité non publiée sans mention ESTIMATE (§3) |
| NN-07 | Ouvrir un second chemin d'écriture (autre que `ParamLink` ou les transactions existantes) |
| NN-08 | Supprimer la vue TABLE |
| NN-26 | Imposer un modèle de routage identique aux trois réseaux : FM (graphe fixe 4 × 4 avec auto-rétroactions), CROSS (graphe biparti fixe à ports typés) et matrice (liens configurables, sources × destinations modulables) gardent leurs dispositions, états et transactions propres |
| NN-27 | Rendre une route inactive autorisée inaccessible : elle reste atteignable pour édition ou activation (filtre SHOW AVAILABLE, clavier) |

## 7. Faisabilité

| Sujet | Analyse | Risque | Vérification |
|---|---|---|---|
| Saturation | Matrice : 64 liens au plus vers 703 destinations potentielles. Les destinations réellement liées sont au plus 64, et le regroupement par domaine (NN-13) borne le nombre de faisceaux visibles. FM et CROSS : nombre fixe et faible (16, 39). | Moyen (matrice) | Prototype avec 64 liens vers 64 destinations distinctes |
| Chevauchements | Grille fixe et routage orthogonal ou en arc par colonne. Les croisements sont inévitables dans la matrice. Ils sont atténués par le focus (NN-12) et les faisceaux. | Moyen | Revue visuelle à 64 liens |
| Clavier | Modèle de navigation NN-15. Aucune commande nouvelle n'est requise côté moteur. | Faible | Test automatique de focus |
| Gestes d'automation | Glisser la profondeur sur une liaison = un geste hôte sur `*.depth` via `ParamLink`. Les liens globaux ne sont pas des paramètres hôtes : ils passent par les transactions de la matrice (Undo). | Faible | Test de comptage begin/end, comme UI04 |
| Undo/Redo | Inchangé : transactions existantes (`Topology`, `ModMatrix`) | Faible | Test Undo NEURAL ≡ TABLE |
| Inspector | Sélection d'une liaison → Inspector sur `*.depth` (FM/CROSS) ou sur le lien global (vue de lien existante) | Faible | Test de synchronisation |
| Coût de rendu | Au plus 64 + 55 chemins redessinés à 30 Hz (`tok::refreshHz`) quand l'activité est affichée. Le coût n'est **pas mesuré**. Il DOIT l'être en prototype, avec un budget candidat ≤ 2 ms par image sur la machine de recette. Repli : n'animer que les nœuds, pas les liaisons. | Moyen | Mesure en prototype (R2-F) |
| Topologie et causalité | Lecture seule hors transactions validées existantes. Aucun changement moteur. | Nul si NN-03/NN-04 sont respectés | Test de non-régression B-T2 existant |
| Réseau fixe et matrice configurable | FM et CROSS : 55 emplacements connus à l'avance, donc positions et liaisons fantômes calculables une fois. Matrice : liens créés et supprimés, destinations parmi 703, donc la disposition des destinations doit suivre les liens existants **sans déplacer** les nœuds déjà placés (insertion en fin de groupe, compactage seulement sur action explicite). | Moyen (matrice) | Test de stabilité des positions après ajout et retrait |
| Lisibilité des routes multiples | Plusieurs liens globaux vers une même destination, ou d'une même source : faisceau avec compteur, dépliage au focus | Moyen | Prototype à 64 liens |

Sous-lots proposés (brief §19, PR-10, OD-19) : **R2-F1** réseaux fixes FM et CROSS (risque faible), **R2-F2** matrice
globale (risques de saturation et de stabilité des positions), chacun ouvert par le prototype PT-03.

Conclusion : la vue NEURAL est faisable sans modification du moteur pour la structure, la profondeur configurée, la
profondeur effective FM/CROSS, les fondus et l'activité des sources. L'activité par route n'est pas disponible
(T-23, T-24).
