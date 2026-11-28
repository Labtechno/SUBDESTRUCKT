# SUBDESTRUCKT · ARCH-UI-04-DA

Revue artistique et ergonomique indépendante du VST3 **0.4.1-b263320802** dans **Reason 14.1d100** sur Mac Studio M4 Max, le **28 novembre 2026**. Révision compilée `d2d08ac1bb6ed09ee3ea1b5e5d5d409c41b586af`. Le rapport accompagne le Design System et le contre-audit R1.

**Verdict 6,7/10.** Performance a une identité claire et une façade convaincante au nominal. La lecture critique à la taille minimale, les doublons de nomenclature et le comportement des viewports doivent être réglés avant d’intégrer le Core 3D.

## Rapports

| Document | Contenu |
|---|---|
| [Revue](REVIEW.md) | Verdict, parcours de création, constats mesurés, appréciations et limites |
| [Scorecard DA-01 à DA-15](SCORECARD.md) | Notes, preuves, propositions, priorités et complexité |
| [Audit DA-09](CONTROL-LABELS-AUDIT.md) | Méthode, exemples chiffrés, zooms, règles par famille et lexique |
| [Propositions de libellés](CONTROL-LABELS-PROPOSALS.csv) | 1 102 lignes de changements identifiés, références et longueurs de texte |
| [Mesures de libellés](CONTROL-LABELS-MEASUREMENTS.csv) · [Mesures des valeurs de choix](CONTROL-LABELS-CHOICES.csv) | Contextes conditionnels, largeurs logiques et budgets mesurés |
| [Recommandations](DESIGN-RECOMMENDATIONS.md) · [Backlog par phase](PRIORITIZED-BACKLOG.md) | Contrat avant ARCH-UI-05 et améliorations pouvant attendre ARCH-UI-06 |

Télécharger les CSV depuis GitHub pour consulter toutes les lignes si l’aperçu du site est partiel.

## Planches et captures

Les planches réduites facilitent le repérage. Les PNG de la colonne de droite affichent la définition de la capture. Les annotations et le recadrage du Core sont des copies.

| Planche | Vues de référence |
|---|---|
| [Performance](CONTACT-SHEETS/PERFORMANCE.png) | [Inspector ouvert](CAPTURES/PERFORMANCE/01-nominal-inspector.png) · [Façade avec Inspector fermé](CAPTURES/PERFORMANCE/02-inspector-ferme.png) · [Core statique](CAPTURES/CORE/01-core-statique.png) |
| [Sound Design](CONTACT-SHEETS/SOUND-DESIGN.png) | [Moteurs WT, CROSS, FM](CAPTURES/SOUND-DESIGN/01-moteurs-wt-fm-cross.png) · [Transformations Z, CROSS et matrice FM](CAPTURES/SOUND-DESIGN/02-z-cross-phase-matrice.png) · [Filtres et traitements](CAPTURES/SOUND-DESIGN/03-traitements.png) · [Enveloppes et LFO](CAPTURES/SOUND-DESIGN/04-enveloppes-lfo.png) · [D2](CAPTURES/SOUND-DESIGN/05-d2-legacy.png) · [D2 après défilement](CAPTURES/SOUND-DESIGN/06-d2-bas-de-chaine.png) · [S2](CAPTURES/SOUND-DESIGN/07-s2-spectral.png) |
| [Engineering](CONTACT-SHEETS/ENGINEERING.png) | [Réseau FM](CAPTURES/ENGINEERING/01-fm-network.png) · [Réseau CROSS](CAPTURES/ENGINEERING/02-cross-network.png) · [Matrice de modulation](CAPTURES/ENGINEERING/03-mod-matrix.png) · [Bank Lab](CAPTURES/ENGINEERING/04-bank-lab.png) · [Voix et accord](CAPTURES/ENGINEERING/05-voice-tuning.png) · [Presets et état](CAPTURES/ENGINEERING/06-presets-state.png) · [Randomisation](CAPTURES/ENGINEERING/07-randomize.png) · [MIDI Learn](CAPTURES/ENGINEERING/08-midi-learn.png) · [Diagnostics](CAPTURES/ENGINEERING/09-diagnostics.png) · [Captures et ressources](CAPTURES/ENGINEERING/10-captures-assets.png) |
| [Dimensions et zoom](CONTACT-SHEETS/SIZE-ZOOM.png) | [Performance 75 %](CAPTURES/SIZE-ZOOM/01-performance-75.png) · [Performance 150 %](CAPTURES/SIZE-ZOOM/02-performance-150.png) · [Minimum 100 % avec Inspector](CAPTURES/SIZE-ZOOM/03-minimum-100-inspector.png) · [Minimum 100 % sans Inspector](CAPTURES/SIZE-ZOOM/04-minimum-100-sans-inspector.png) · [Sound Design 75 %](CAPTURES/SIZE-ZOOM/05-sound-75.png) · [Sound Design 150 %](CAPTURES/SIZE-ZOOM/06-sound-150.png) |
| [Annotations](ANNOTATIONS/) | [WT Z et CROSS](ANNOTATIONS/01-reperes-x-y-z.png) · [Fenêtre minimale](ANNOTATIONS/02-lecture-taille-minimale.png) · [Gel, armement et capture](ANNOTATIONS/03-gel-armement-capture.png) · [Matrice CROSS](ANNOTATIONS/04-matrice-cross.png) |

La campagne conserve 25 originaux JPEG de Computer Use dans l’espace local de recette, avec une liste d’empreintes. Les 25 copies publiées en PNG conservent les dimensions de capture, sans réencodage avec perte. Deux copies occultent un chemin local ou des données d’exécution dans Presets / State et Diagnostics, puis servent de source à leur planche. Les dimensions nominales et minimales du plug-in sont 1440 × 900 et 960 × 640. Les captures de fenêtre Reason incluent aussi la barre de l’hôte. Le 100 % est illustré par les captures nominales et minimales.

## Références

[Design Baseline](../V1-DESIGN-BASELINE.md) · [Design UI/UX](../V1-UI-UX-DESIGN.md) · [Spécification ARCH-UI-03](../V1-ARCH-UI-03-SPEC.md) · [Design System](../V1-ARCH-UI-04-DESIGN-SYSTEM.md) · [Rapport ARCH-UI-04](../V1-ARCH-UI-04-REPORT.md) · [Rapport R1](../V1-ARCH-UI-04-R1-REPORT.md)
