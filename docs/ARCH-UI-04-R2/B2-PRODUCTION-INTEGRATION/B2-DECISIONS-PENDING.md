# ARCH-UI-04-R2-B2 — décisions soumises au propriétaire (STOP ciblés)

Chaque point bloque seulement l'étape indiquée. Les travaux indépendants continuent (mission R2-B2 §8).

## DB2-08 — Saut de valeur des PLASMA hors canal (EC-17) : **VALIDÉ, option A** (2026-11-28)

Constat : B2-01, PB-J1 à J3 et PB-01c ([PB/README.md](PB/README.md)). Les options et l'analyse soumises sont
conservées à l'annexe A.

**Contrat définitif**, appliqué aux PLASMA verticaux et horizontaux, en STANDARD comme en COMPACT :

| Zone | Interaction | Comportement attendu |
|---|---|---|
| Canal | Clic simple | Position absolue (DB2-02) |
| Canal | Glisser | Comportement historique 0.4.1 |
| Libellé | Clic simple | Sélection et focus seulement |
| Libellé | Glisser | Aucune modification du paramètre |
| Valeur | Clic simple | Aucune écriture du paramètre |
| Valeur | Double-clic | Saisie numérique, sans modification préalable |
| Hors canal | Clic | Aucune écriture involontaire |

Les autres contrats sont conservés : clavier, molette, menu contextuel, saisie numérique, automation, retour au défaut
(sous réserve d'EC-19), Undo et Redo. **Mise en œuvre** : B2-03 (2). La zone active est la zone de manipulation
utilisée par le rendu. Un geste commencé hors du canal ne devient jamais un geste de modification. Aucun glisser
relatif n'est ajouté. Les NEXUS ne sont pas modifiés. **Tests permanents** : PB-J1, J2, J3 et PB-01c deviennent des
assertions (OUT GAIN reste à 0 dB, BALANCE reste à 0,5, la saisie s'ouvre sans saut). Ils sont complétés par les
frontières de zone, les glissers commencés hors canal et dans le canal, le clic absolu aux extrémités, la prise en main,
le focus, et l'absence d'écriture, de geste et de transaction parasites, aux trois zooms.

## DB2-09 — Double-clic dans le canal d'un PLASMA (EC-19) : **À ARBITRER avant la clôture de R2-B2**

Ce point ne bloque ni la fondation R1 ni la correction d'EC-17. Aucun comportement ne change avant l'arbitrage.

### 1. Comportement actuel et preuves

PB-01b (`PB/windows/pb-events.csv`, séquences 723 à 735 ; macOS identique), OUT GAIN au départ à 0 dB, double-clic au
centre du canal :

| Événement JUCE | Effet hôte |
|---|---|
| `mouseDown` (1 clic) | begin, **écriture 0,545 (≈ −18,6 dB : position cliquée)** |
| `mouseUp` | end : **transaction 1** |
| `mouseDown` (2 clics) | begin (même position : aucune écriture) |
| `mouseUp` | end |
| `mouseDoubleClick` | begin, écriture 0,778 (−6 dB, défaut), end : **transaction 2** |

### 2. Conséquences sur le signal audio

Le paramètre prend la valeur de la position cliquée pendant tout l'intervalle du double-clic : de l'ordre de 100 à
400 ms selon l'utilisateur, avec un délai maximal de 400 ms dans JUCE. Le lissage du moteur (20 ms) ne masque pas un
palier de cette durée. Sur OUT GAIN, partant de 0 dB, le niveau tombe à −18,6 dB, puis remonte à −6 dB. Ce creux est
audible. Sur BALANCE, on entend un basculement temporaire.

### 3. Conséquences sur l'Undo

Il y a **deux transactions**, la position cliquée puis le défaut. Un seul Undo ramène à la valeur transitoire, pas à la
valeur d'origine.

### 4. Conséquences sur l'automation

Deux gestes hôte complets sont émis. En écriture d'automation, l'hôte enregistre le creux transitoire puis le retour au
défaut. La piste d'automation garde donc une marche non voulue.

### 5. Alternatives

| Option | Description | Faisabilité | Réactivité | Compatibilité DB2-02 / DB2-08 | Évaluation |
|---|---|---|---|---|---|
| (A) Comportement historique | Inchangé, documenté | — | Inchangée | Oui | Le creux audible, la double transaction et la marche d'automation demeurent |
| **(B) Pas de retour au défaut par double-clic dans le canal** | Dans le canal, un double-clic équivaut à deux clics au même point : position absolue stable, une transaction. Le retour au défaut reste possible par Alt+clic, Suppr et le menu contextuel, chemins existants testés par PB-01d. | Simple : retirer l'appel de réinitialisation sur le double-clic dans le canal du PLASMA | Inchangée, aucune temporisation | Oui : le clic reste absolu et le double-clic sur la valeur ouvre toujours la saisie | Supprime la double transaction. Il reste le saut volontaire du clic absolu, qui est le contrat DB2-02. Perte d'un raccourci de réinitialisation sur les PLASMA ; les NEXUS gardent le double-clic. |
| (C) Option B + double-clic sur le **libellé** = retour au défaut | Réinitialisation hors canal, donc sans valeur transitoire | Simple | Inchangée | Oui : le libellé n'écrit jamais au clic simple (DB2-08) | Nouveau geste, moins découvrable : à documenter dans l'infobulle |
| (D) Clic simple temporisé | Le clic absolu n'est appliqué qu'après le délai du double-clic, ou dès qu'un glisser commence | Possible | **Dégradée** : jusqu'à 400 ms de retard pour chaque clic | Contraire à l'esprit de DB2-02 (clic immédiat) | Écartée, comme la mission le pressentait |
| (E) Fusion des deux transactions Undo | Le défaut annule et remplace la transaction du premier clic | Possible côté interface | Inchangée | Oui | Ne corrige ni le creux audible ni la marche d'automation |
| (F) Retour au défaut annulant d'abord le premier clic | Restaure la valeur d'origine dans le même geste que le double-clic | Possible | Inchangée | Oui | Le creux a déjà été joué : ni le signal ni l'automation ne sont corrigés |

### 6. Recommandation

**(B)** : retirer le retour au défaut par double-clic dans le canal des PLASMA.

- C'est la seule option qui supprime à la fois la valeur transitoire non voulue, la double transaction et la marche
  d'automation, sans temporisation et sans geste nouveau.
- Le retour au défaut reste disponible par trois chemins existants (Alt+clic, Suppr, menu).
- L'infobulle du PLASMA serait mise à jour.
- **(C)** est une variante acceptable si le propriétaire tient à conserver un retour au défaut par double-clic sur les
  PLASMA.

## Annexe A — DB2-08 tel que soumis (pour la traçabilité)

Options soumises :

- **(A)**, retenue : limiter le clic absolu et le début de geste à la zone de manipulation ;
- **(B)** : conserver le comportement historique ;
- **(C)** : option A plus un glisser relatif sur le libellé ou la valeur.

Recommandation : (A). Analyse complète dans l'historique de ce fichier (commit `b1502c8`).
