# ARCH-UI-04-R2-B2-01 — reproduction des défauts de geste (PB-01, PB-02, PB-N, PB-J)

Exécutée sur le code de production **0.4.1, avant toute modification de composant** (base `9e0d5a9`). Aucun fichier de
`source/`, `plugin/` ni `ui/` n'est modifié par B2-01.

## 1. Banc de reproduction

Le banc est dans `tests/wrapper/GestureTests.cpp`, exécuté par `subdestruckt_wrapper_tests`, donc aussi par la CI de
production. Ses tests sont **permanents** (CE-B2-01).

| Élément | Méthode |
|---|---|
| Éditeur | `MainEditor` réel, créé par le processeur réel (`createEditorIfNeeded`), détruit selon le protocole de l'hôte (`editorBeingDeleted`) |
| Événements | Injectés par `ComponentPeer::handleMouseEvent` et `handleMouseWheel`. **Tout le dispatch est celui de JUCE 9.0.6** : comptage des clics, ordre `mouseDown` / `mouseUp` / `mouseDoubleClick`, ciblage, contrôles modaux. |
| Couche fenêtre | Remplacée par un pair de test (`TestPeer`) : empilement des fenêtres et rendu à l'écran seulement. Motif : avec une vraie fenêtre, `ComponentPeer::contains` interroge l'OS (`WindowFromPoint`) et les clics se perdaient dès qu'une autre fenêtre recouvrait celle du test. Un premier essai l'a montré : 0 événement reçu. Ce résultat a été **rejeté, pas publié comme « NON REPRODUIT »**. |
| Touches clavier | Posées dans `ModifierKeys::currentModifiers`, comme le font les pairs natifs à partir du clavier de l'OS. JUCE retire ces touches des modificateurs d'un événement souris injecté. |
| Mesures | Sonde `AudioProcessorListener` : begin et end par paramètre, profondeur courante et maximale, écritures hors geste. Journal `MouseListener` des événements effectivement reçus. Transactions Undo comptées par l'API publique. Valeurs de départ posées par l'hôte, hors geste. |
| Validité | Un scénario sans événement reçu par le composant est **BLOCKED**, jamais NON REPRODUIT |
| Limite | Le minuteur de fin de geste molette (450 ms) exige la boucle de messages, absente du banc. PB-01e mesure donc l'état au clic ; le geste molette est refermé par la destruction de l'éditeur. |

## 2. Résultats

| Plateforme | Résultats |
|---|---|
| Windows 11 (poste de développement) | [`windows/`](windows) |
| macOS 15.8.2 arm64 (VM GitHub, workflow `r2b2-evidence.yml`, run 1 sur `b1502c8`) | [`macos/`](macos) : **statuts identiques à Windows pour les 19 lignes**, barrière PASS (75 critères, AUT-02 connu) |

Fichiers : `pb-summary.csv` (synthèse), `pb-events.csv` (journal complet), `wrapper-results.txt`, `barrier.txt`,
`machine.txt`, `pb-platform.txt`.

Les mesures ci-dessous sont celles de Windows ; macOS donne les mêmes statuts et les mêmes compteurs (profondeur, résidus,
transactions Undo, conséquence sur l'Undo de la session).

| Scénario | Action | Statut (Windows et macOS) | Mesure | Suite |
|---|---|---|---|---|
| **PB-01a** | NEXUS `wt.x` (départ 0,8) : double-clic hors valeur | **NON REPRODUIT** | 3 begin / 3 end, profondeur max **1**, défaut atteint, 1 transaction Undo | Aucune correction (OD-10) ; test permanent BE |
| **PB-01b** | PLASMA vertical `out.gain` (départ 0 dB) : double-clic dans le canal | **NON REPRODUIT** | profondeur max 1, défaut atteint, **2 transactions Undo** (voir EC-19) | Test permanent BE |
| **PB-01c** | PLASMA horizontal `perf.balance` (départ 0,5) : double-clic sur la valeur | **REPRODUIT (saut)** | profondeur 1, mais la valeur **passe de 0,5 à 1,0 (maximum)** avant l'ouverture de la saisie | EC-17, **DB2-08** |
| **PB-01d** | NEXUS : Alt+clic, Suppr, menu « défaut », saisie numérique | **NON REPRODUIT** | 4 begin / 4 end, profondeur 1, 4 transactions | Test permanent BE |
| **PB-01e** | NEXUS : molette puis clic avant 450 ms | **REPRODUIT** | profondeur max **2** (gestes hôte imbriqués sur `wt.x`) | Correction autorisée en B2-03 (1) (NEXUS et PLASMA, même classe) |
| **PB-01m** | Deux instances : double-clic dans chacune | **NON REPRODUIT** | instances isolées, profondeur 1 chacune | Test permanent |
| **PB-02a** | Core : glisser puis destruction de l'éditeur | **REPRODUIT** | **2 gestes ouverts** (`cross.fwd`, `cross.rev`), jamais refermés ; l'édition suivante donne **0 transaction Undo**, l'historique de la session est bloqué | Documenté ; correction **R2-C** |
| **PB-02b** | Core : glisser, changement d'espace, relâchement | **NON REPRODUIT** | JUCE délivre le `mouseUp` au Core masqué ; gestes refermés ; édition suivante : 1 transaction | — |
| **PB-02c** | Core : composant modal pendant le glisser, relâchement | **NON REPRODUIT** | gestes refermés | La perte de capture au niveau de l'OS (fenêtre désactivée) n'est pas simulable par le banc : non couverte, à vérifier en recette physique |
| **PB-02d** | Enveloppe : glisser d'une poignée puis destruction | **REPRODUIT** | **1 geste ouvert** ; édition suivante : **0 transaction Undo** | Documenté ; correction **R2-D** |
| **PB-02e** | Core : redimensionnement et zoom 75 % pendant le glisser | **NON REPRODUIT** | gestes refermés au relâchement (RS-01 sans défaut de geste) | — |
| **PB-N1** | NEXUS : glisser puis destruction de l'éditeur | **NON REPRODUIT** | refermé par le destructeur (BE-05) | Test permanent BE (assertion) |
| **PB-N2** | NEXUS : glisser, changement d'espace, relâchement | **NON REPRODUIT** | refermé au relâchement | Test permanent |
| **PB-N3** | NEXUS de panneau contextuel : type DST changé par l'hôte pendant le glisser | **NON REPRODUIT** | commande détruite par la reconstruction ; geste refermé par le destructeur | Test permanent |
| **PB-J1** | PLASMA vertical `out.gain` : clic sur le **libellé** | **REPRODUIT (saut)** | **0 dB → +6 dB** (maximum) | EC-17, **DB2-08** |
| **PB-J2** | PLASMA horizontal `perf.balance` : clic sur le **libellé** | **REPRODUIT (saut)** | **0,5 → −1,0** (minimum) | EC-17, **DB2-08** |
| **PB-J3** | PLASMA horizontal `perf.balance` : clic sur la **valeur** | **REPRODUIT (saut)** | **0,5 → +1,0** (maximum) | EC-17, **DB2-08** |
| **PB-J4** | PLASMA vertical : clic dans le canal, puis reprise au même point | **NON REPRODUIT** | position absolue au premier clic (comportement DB2-02), **aucun saut** à la reprise | Test permanent |

**Hypothèse H-04** (ligne « INFORMATION » de la synthèse). Les gestionnaires sont appelés directement dans l'ordre
supposé par l'audit statique : down, up, down, `mouseDoubleClick`, up. On obtient alors une profondeur de 2. Mais JUCE
9.0.6 n'émet pas cet ordre : `mouseDoubleClick` est appelé dans `Component::internalMouseUp`, **après** le `mouseUp` du
second clic (`juce_Component.cpp:2667-2676`). Le constat statique était donc correct pour un ordre qui ne se produit pas.

## 3. Conclusions

1. **PB-01 (double-clic) : NON REPRODUIT.** Aucune correction (OD-10). Les scénarios restent en tests permanents.
2. **PB-01e (molette puis clic) : REPRODUIT.** Ce sont des gestes hôte imbriqués sur un NEXUS ou un PLASMA. La
   correction est autorisée en B2-03 (1) : refermer le geste molette au `mouseDown`. Le scénario devient alors une
   assertion BE (profondeur 1, aucun résidu, une transaction).
3. **PB-02 Core et enveloppe : REPRODUIT** à la destruction pendant un glisser. Les gestes restent ouverts côté hôte, et
   **l'Undo de la session est bloqué** (0 transaction pour les éditions suivantes). Corrections réservées à R2-C et R2-D,
   comme le prévoit la mission. La gravité est consignée en EC-18 pour ces lots. Le masquage, le modal et le
   redimensionnement ne reproduisent pas le défaut.
4. **NEXUS (PB-N) : aucun défaut** de destruction, de masquage ou de reconstruction.
5. **Nouveau défaut PLASMA (PB-J1 à J3, PB-01c) : saut à une borne** quand on clique sur le libellé ou sur la valeur
   (hors canal). Le clic en position absolue est calculé sur la zone de manipulation, mais il est accepté sur toute la
   commande, sauf la valeur du PLASMA vertical. C'est un défaut du comportement historique. Conformément à DB2-02, il
   est **soumis avant toute modification : DB2-08**.

## 4. Rejouer

```bash
SBD_B2_OUT=out/pb build/subdestruckt_wrapper_tests_artefacts/Release/subdestruckt_wrapper_tests tests/host/expected_vst3_param_ids.txt
bash tools/check_results.sh tests/expected_results.txt tests/known_failures.txt "wrapper=<journal>:<code>"
```

## 5. Vérification

- **CI de production** `arch-g4.yml`, run 56 sur `b1502c8` : **SUCCESS**. Elle comprend les 18 scénarios et la barrière.
- **Preuves macOS** : `r2b2-evidence.yml` run 1, versées en `43ed70e`.
- **Verdict B2-01** : critères satisfaits. Toutes les reproductions sont exécutées par le dispatch réel sur les deux
  plateformes, avec statuts individuels et tests permanents. Seul blocage : **DB2-08**, ciblé sur la correction du saut
  PLASMA. Il n'empêche pas B2-02.

