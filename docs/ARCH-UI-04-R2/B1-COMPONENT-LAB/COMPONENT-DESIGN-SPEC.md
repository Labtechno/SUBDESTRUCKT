# ARCH-UI-04-R2-B1 — spécification des composants réalisés

Ce document décrit ce qui est **réalisé** dans le Component Lab (`lab/r2/`) et le compare au référentiel R2-A
(`../R2-DESIGN-SYSTEM-SPEC.md`). Les tokens et contrastes sont dans [DESIGN-TOKENS-R2.md](DESIGN-TOKENS-R2.md). Les
captures sont dans [README.md](README.md).

## 1. Architecture réutilisable

| Élément | Rôle | Intégration R2-B2 |
|---|---|---|
| `ValueSource` | Abstraction de valeur normalisée, avec le même contrat de geste que `ParamLink` 0.4.1 (`begin` / `during` / `end` / `complete`), plus Effective, texte, bipolarité et routes | Adaptateur `ParamSource` vers `ParamLink`, déjà démontré dans `lab/app/Compare.cpp` : composants R2 liés aux paramètres réels 0.4.1 |
| `GestureGuard` | Ferme le geste ouvert à la destruction, au masquage, au détachement et sur `cancelGesture()` | Repris tel quel ; la correction PB-01/PB-02 en production suit OD-10 |
| `Forced` | État visuel imposé (planches) : Normal, Hover, Focus, Pressed, Disabled | Réservé aux captures et aux tests ; sans effet en production |
| `draw::*` | Primitives : corps usiné, moletage, arcs avec lueur ≤ 0,35, touches convexes et plates, témoins encastrés, cadre optique, hachures, focus, texte sur une ou deux lignes | Bibliothèque partagée par toutes les familles |
| Domaine | WT, FM, CROSS, MOD, neutre, alerte | Remplace `Domain3` |

Les composants R2 ne dépendent ni du registre ni du processeur. Aucun composant n'est dupliqué entre le laboratoire et
l'intégration.

## 2. NEXUS Hybrid (DA-A03)

| Élément | Réalisé | Référence R2-A |
|---|---|---|
| Matière | Titane noir, face légèrement concave, biseau éclairé en haut à gauche, ombre portée | §4 |
| Périphérie | XL : moletage usiné de 56 stries ; S : biseau usiné sans moletage ; Micro : corps simple | §4.1 |
| Graduations | XL ×11, S ×5, Micro aucune ; centre marqué en bipolaire | §4.2 |
| Arc BASE | Couleur du domaine, depuis le minimum ou le centre (bipolaire), lueur maîtrisée (XL, S) | §4.2 |
| EFFECTIVE | Anneau extérieur et **losange**, indépendants de l'arc ; absent si non modulé | §4.2 |
| Marques de route | Quatre au plus, puis « +n » ; aucune en Micro | §4.2 |
| Index | Trait optique avec ombre ; point en bout d'index pour XL | §4.2 |
| Valeur | Fenêtre carbone, chiffres monospace, BASE ; contour du domaine pendant le geste | §4.2 |
| États | Normal, Hover (**halo fin**), Focus (anneau), Pressed / actif, Disabled (corps nu, « — »), INACTIVE (hachures et mention), modulé | §4.3 |
| Gestes | Ceux de 0.4.1, à l'identique ; double-clic hors valeur = défaut dans le geste ouvert | §4.4 |
| Zones | Toute la cellule pour glisser ; zone de valeur pour la saisie | §4.1 |

## 3. NEURAL CELL Hybrid (DA-A04)

| Type | Traitement | Témoin | Texte d'état |
|---|---|---|---|
| Action instantanée | Touche **convexe** `relief3`, en creux à l'appui, icône d'impulsion (créneau) | aucun | — |
| Interrupteur | Touche **plate** `relief2` | disque encastré, plein si ON | OFF / ON |
| Armement | Touche plate, **liseré ambre** | anneau (ARMED), disque (ACTIVE), croix (DENIED) | OFF / ARMED / ACTIVE / DENIED |
| Sélection exclusive | Fond **carbone**, losange plein et soulignement du domaine sur l'option active | losange | libellé |
| Sûreté (PANIC) | Rouge `alert`, convexe, double biseau, isolée | — | — |
| Désactivée | Contour seul, hachures | aucun | « — » et raison en infobulle |

FREEZE (interrupteur), ARM (armement), AUTHORIZE (action, icône d'autorisation ambre) et CAPTURE (action, icône
d'instantané) diffèrent par la forme, le témoin, l'icône, le texte et l'emplacement (planche 03). Le contrat
d'autonomie 0.4.1 n'est pas touché.

## 4. SYNAPTIC TRACK

Segments dans une gorge usinée (segment actif en relief, souligné), liste dans un champ optique (rang n/N, chevron,
« … » si trop long) et onglets soulignés. États : actif, inactif, survol, focus, désactivé. Indices et libellés hôtes
inchangés ; affichage compact R2 (planche 04).

## 5. Cybernetic Display (DA-A05, option D)

Cadre optique (verre, monture, repères d'angle), titre principal, **source obligatoire** en pied, étiquette DEMO DATA.
Variantes : valeur, oscilloscope, spectre **INDICATIVE**, wavetable (BASE en trait plein, EFFECTIVE en pointillé),
courbe de modulation, enveloppe (niveau pour AMP seulement, sans curseur d'étape), crête L/R (segments `measure`, rouge
seulement au-delà de 0 dBFS, PEAK en dBFS), état autonome, diagnostic (deux niveaux de hiérarchie).

**C03** : le spectre est dérivé de l'oscilloscope, décimé sans filtre anti-repliement. Il est indicatif **dans toute sa
bande affichée**, et pas seulement au-delà de fs/4. L'étiquette et le pied le disent.

**C04** : l'afficheur AUTONOMY sépare TRANSPORT, AUTONOMY, ARMED et OUTPUT. Avec TRANSPORT STOP et AUTONOMY SUSPENDED,
la sortie peut porter un signal ; la démonstration affiche « SIGNAL −18,4 dBFS ».

## 6. PLASMA CHANNEL Hybrid (DA-A06)

Gorge usinée, canal lumineux BASE (dégradé du domaine, depuis le minimum ou le centre), **curseur mécanique** en titane
avec stries et index, marqueur EFFECTIVE indépendant (**triangle** du côté opposé aux graduations), graduations et
unité, valeur en fenêtre optique. La crête mesurée n'est jamais dessinée dans le canal : l'afficheur PEAK est distinct
(planche 06).

## 7. Écarts avec le référentiel R2-A

| Écart | Nature | Statut |
|---|---|---|
| PLASMA vertical 64 × 180 au lieu de 48 × 150 | gabarit, pour loger graduations et unité | proposition à arbitrer |
| Survol NEXUS / PLASMA / NEURAL CELL marqué par un halo ou un contour (forme) | ajout pour différencier les états sans la couleur | proposition |
| Libellé PANIC carbone (5,08:1) au lieu de blanc (3,84:1) | contraste WCAG | **à arbitrer** ; le blanc 0.4.1 reste la référence des planches |
| `relief1` / `relief2` ajustés (`#22282D` / `#2C343A`) | seuil candidat 1,15:1 | appliqué dans le laboratoire |
| Saisie numérique visuelle seulement (caret) | le champ de 0.4.1 sera réutilisé | limite du laboratoire |
| Liste : un clic passe à l'option suivante (pas de menu déroulant) | le menu de 0.4.1 sera réutilisé | limite du laboratoire |
| Accessibilité : titre = nom complet ; pas d'interface de valeur accessible propre | les gestionnaires 0.4.1 seront repris | limite du laboratoire (AX-01 à vérifier en R2-B2) |
| Animation : aucune (planches statiques, DEMO DATA) | les cadences seront traitées en R2-G | conforme à C05 |
