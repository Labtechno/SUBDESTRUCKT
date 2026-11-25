# V1 — ARCH-G3 — Rapport de la première vertical slice

Date : 2026-11-25. Branche : `feature/arch-g3-vertical-slice`, créée depuis `main` `5cc21a61138740c3122741ee5d56d98de954d03a`.
`main` n'est pas modifié. Aucun merge, aucun ARCH-G4, aucune release publique.

## 1. Verdict

**GO livraison interne (candidate), dans la portée testée.** Tout ce qui pouvait être exécuté automatiquement l'a été :
compilation Release arm64, tests, validation VST3 et paquets. Tous ces contrôles réussissent sur le runner GitHub
macOS 15 arm64, qui est une machine virtuelle.

**La validation ARCH-G3 complète reste PENDING.** La recette physique sur le Mac Studio M4 Max n'a pas été faite :
installation, découverte dans une DAW, son, MIDI, CROSS, PDC, rechargement de projet, désinstallation et
réinstallation. Le M3 physique est exigé avant toute Beta.

Le résultat du run de livraison, le lien de téléchargement et l'empreinte du ZIP sont au § 9.

## 2. Gates du dossier de mission

| gate | contenu | statut |
|---|---|---|
| 1 | Plan de fichiers et contrats, stratégie de test et seuils fixés avant le code ; API d'hôte disponibles ; errata S0.1 | **FAIT** : `docs/V1-ARCH-G3-PLAN.md` (commit `4d34ae4`, antérieur à tout code). Errata E1, E2 et E3 dans `docs/V1-S0.1-ERRATA.md` (branche `spike/s0.1-causality-validation`, commit `fb669170`) ; données et verdict historiques inchangés. |
| 2 | Cœur minimal testé nativement (B-T2, Thiran1, DAG, transitions, sérialisation, déterminisme, reset) | **PASS (CI)** : 23 critères, RTSan compris. |
| 3 | Wrapper JUCE/VST3, état, identifiants stables, Base / Effective, PDC, UI de diagnostic, 4 fréquences × 5 blocs | **PASS (CI, hôte VST3 de test)**. Tests en DAW : **PENDING**. |
| 4 | CI macOS arm64 gratuite : build Release, tests, validation VST3 | **PASS** : runner `macos-15`, pluginval 1.0.4 au niveau de rigueur 10. Pas de prétention à une recette DAW. |
| 5 | ZIP versionné, Installer et Uninstaller.pkg, README_FR, CHANGELOG, SHA256SUMS, PKG-01 à PKG-08 | **PASS (VM CI)** pour tout ce qui est automatisable. La partie physique de PKG-02 à PKG-05 et l'essentiel de PKG-08 sont **PENDING**. Signature et notarisation : **bloquées**, faute de certificats (§ 7). |
| 6 | Rapport et SHA publiés, recette utilisateur | **FAIT** pour le rapport. Recette M4 Max : **PENDING** (`docs/V1-ARCH-G3-RECETTE-M4MAX.md`). |

## 3. Ce qui a été construit

**Séparation stricte des couches** (`docs/V1-ARCH-G3-PLAN.md` § 4) :

- `source/dsp` (L3), `source/core` (L1) et `source/engine` (L2) : aucune dépendance à JUCE, aucun code conditionnel de
  plateforme ;
- `plugin/` : wrapper JUCE ;
- `ui/` : UI de diagnostic ;
- `packaging/` : paquets ;
- `tests/` : tests du cœur, du wrapper, de l'hôte VST3 et contrôle RTSan.

**Contenu de la tranche :**

- MIDI mono, priorité à la dernière note, événements appliqués à leur offset ;
- ordre dans un même échantillon : sécurité, puis note-off, puis note-on ;
- CC120 déclenche le Panic, CC123 coupe toutes les notes ;
- WT minimal : dent de scie limitée en bande, mip-map ;
- OP1 et OP2 en PM ;
- CROSS WT→FM et FM→WT, avec un Master et une intensité par direction ;
- ADSR appliqué au rythme moteur, sortie stéréo, garde DC de sortie, garde numérique ;
- 22 paramètres hôte à StableID explicites ;
- état versionné v1 ;
- preset minimal : OP2→OP1 (I), WT→OP1 (I), OP1→WT (D).

**Décisions appliquées :**

- **B-T2 sticky :** une route qui ferme un cycle Immediate, ou une auto-boucle, est classée Delayed. Les classes des
  routes survivantes ne sont jamais recalculées. Les identifiants StableRouteID ne sont jamais réutilisés. Les
  transactions sont atomiques. Le chargement est validé et un DAG invalide est refusé.
- **Retard nominal de 1/48000 s,** appliqué aux seules arêtes Delayed. D = Fs·K/48000 :
  - entier exact à 48 kHz (1) et à 96 kHz (2) ;
  - fractionnaire à 44,1 kHz/2× et à 88,2 kHz (1,8375), lu par **Thiran1** (a = 0,088435).
- **Oversampling :** 2× permanent à 44,1 kHz ; 1× à 48, 88,2 et 96 kHz. Cette politique est provisionnelle : la
  politique Quality n'est pas encore implémentée.
- **Latence déclarée :** 32 échantillons à 44,1 kHz, 0 ailleurs. Elle ne change qu'au `prepareToPlay`.
- **Transitions séquentielles :**
  1. fondu de sortie de 128 échantillons hôte sur l'ancien graphe ;
  2. bascule atomique ;
  3. fondu d'entrée de 128 échantillons hôte sur le nouveau graphe.

  L'union des deux graphes n'est jamais exécutée. Les graphes sont préparés et retirés hors temps réel, par échange
  lock-free.
- **Base et Effective :** l'hôte automatise la valeur Base ; le moteur calcule la valeur Effective, affichée dans
  l'UI.

**UI de diagnostic.** Elle affiche :

- la configuration active : Fs, K, Fs moteur, D, lecteur, latence ;
- 22 curseurs pour les valeurs Base ;
- la topologie : route, classe, identifiant, profondeur Effective, gain de transition, boutons d'ajout et de retrait ;
- la télémétrie : voix, crêtes, compteurs de sûreté ;
- les boutons Panic et Preset init.

Une capture faite par la CI sur macOS est publiée avec la livraison (`ui_diagnostic_macos.png`). Le graphisme n'est
pas définitif.

## 4. Outillage et dépendances épinglés

| élément | version et vérification |
|---|---|
| JUCE | 9.0.6, commit `654bdbc7a9db0dae90849b44abe91f339e9f0768`, vérifié en CI. Licence double AGPLv3 / commerciale JUCE 9 : **choix non arbitré** (§ 8). |
| SDK VST3 | 3.8.0 (MIT), fourni avec JUCE |
| pluginval | 1.0.4, SHA-256 `3c4c533b…2b29f` vérifié |
| Compilateur | Apple Clang de l'image `macos-15` (journalisé avec Xcode, CMake et macOS dans `machine.txt` du run) |
| RTSan | clang 23.1.5 (Homebrew), job séparé |
| Cible de déploiement | **macOS 14.0 déclaré**, `minos` vérifié dans le binaire. Ce n'est pas un minimum validé. |

## 5. Tests automatisés (CI macOS arm64, VM)

Un même critère est validé à plusieurs niveaux quand c'est possible : cœur, wrapper instancié, et vrai bundle chargé
par l'interface VST3. Résultats du run n°4 (`45607779663`), identiques à ceux du run de livraison (§ 9).

| critère | résultat |
|---|---|
| G-01a/b/c/d — DAG / sticky | PASS : règles, 1200 séquences et 48 000 transactions aléatoires, validation au chargement, profondeur 0 sans retrait |
| G-02 — transactions | PASS : 6 transactions invalides rejetées sans effet partiel |
| G-03a/b — état | PASS : 300 allers-retours exacts ; 16 états invalides refusés (version future, cycle I, doublons, hors plage, NaN, etc.) |
| G-04 — causalité | PASS : sorties non calculées empoisonnées par NaN, 0 violation aux 4 fréquences ; contrôle négatif détecté |
| G-05 — retard T2 / Thiran1 | PASS : \|H\| − 1 ≤ 3,3e−16 ; τ(DC) = D ; τ(1 kHz) = 1,837606 pour D = 1,8375. Rapporté sans seuil : τ = 1,84014 / 1,84804 / 1,86110 / 1,87904 échantillons à 5 / 10 / 15 / 20 kHz (fs moteur 88,2 kHz). |
| G-06 / G-06w — latence | PASS : pic impulsionnel exactement à la latence déclarée (32 à K = 2, 35 à K = 4) ; latence déclarée à l'hôte 32 / 0 / 0 / 0 |
| G-07 — déterminisme | PASS : bit à bit pour des blocs 1 / 7 / 32 / 64 / 128 / 256 / 512 et variables, aux 4 fréquences |
| G-08 — MIDI | PASS : offset exact, note-off avant note-on dans un même échantillon, priorité dernière note |
| G-09 — WT seul | PASS : harmoniques 2 à 10 en 1/k à 0,001 dB près ; énergie inharmonique −82,8 dB ; indépendance vis-à-vis du FM, bit à bit |
| G-10 — FM seul | PASS : bandes latérales k = 0 à 3 conformes à \|J_k(1)\|, écart maximal 0,112 dB |
| G-11 — CROSS WT→FM | PASS : NMSE −5,0 / −5,1 dB et écart spectral 29 à 31 dB (route seule et cycle B-T2, 44,1 et 48 kHz). Inactif : identique bit à bit à la référence. |
| G-12 — CROSS FM→WT | PASS : NMSE −0,1 dB, écart spectral 19 dB (y compris via l'arête Delayed T2). Inactif : identique bit à bit. |
| G-13 — CROSS = 0 sans reset | PASS : après la rampe, sortie identique bit à bit à une référence sans CROSS, aux 4 fréquences |
| G-14 — transitions | PASS : 6 transactions en cours de note, dont l'union cyclique ; poison actif ; 0 échantillon de chevauchement ; 0 NaN. Métrique de clic Δ⁴, rapportée sans seuil : burst maximal +3,1 dB (48 kHz) et −0,0 dB (44,1 kHz) par rapport au régime établi. **Aucune promesse d'absence de clic.** |
| G-15 — bornes | PASS : 200 configurations aléatoires × 2 s, 0 NaN ; garde de sortie et borne PM jamais atteintes |
| G-16 / G-16w — reset | PASS : Panic donne un silence exact après 5 ms puis la note suivante est normale ; NaN injecté : 1 réinitialisation et reprise du son ; Panic transmis depuis l'UI |
| G-17 — silence | PASS : sortie exactement nulle sans note ; < −120 dBFS 1 s après la fin du relâchement |
| G-18 / G-18w / G-18rt — temps réel | PASS : 0 allocation dans `Engine::process` et dans `AudioProcessor::processBlock` ; **RTSan : 0 violation**, et le contrôle négatif (allocation volontaire) est bien détecté |
| G-19 / G-19w / G-19h — instances | PASS : instances isolées et entrelacées, au niveau du cœur, du wrapper et de deux instances VST3 hébergées |
| G-20 / G-20a — hôte VST3 | PASS : découverte (1 classe, Labtechno, instrument, 0.1.0) ; 4 fréquences × blocs 32 à 512, latence attendue, sorties identiques bit à bit entre tailles de bloc |
| G-21 / G-21w — état | PASS : état relu identique via VST3, rendu identique bit à bit ; routes, identifiants et classes restaurés ; état invalide et version future refusés |
| G-22 / G-22w — identité | PASS : identifiants VST3 des 22 paramètres égaux à la liste de référence calculée indépendamment à partir des StableID |
| G-23 — pluginval | PASS : niveau de rigueur 10, tests d'interface inclus, 44,1 / 48 / 88,2 / 96 kHz × blocs 32 à 512 |
| G-24 — bundle livré | PASS : arm64 uniquement, signature ad hoc valide, `com.labtechno.subdestruckt` 0.1.0, `minos` 14.0, mentions tierces présentes |

**Non applicables, avec justification :**

- undo/redo de topologie : non exposé dans la tranche ;
- verrouillage DC en LinFM/TZFM : seule la PM est implémentée ;
- changement de Quality : non implémenté ;
- ExpFM : dette conservée.

## 6. Paquets : PKG-01 à PKG-08

| critère | automatisé (VM CI) | physique |
|---|---|---|
| PKG-01 ZIP, fichiers, version, empreintes | PASS : 5 fichiers ; `shasum -c` OK ; identité `0.1.0-bN` cohérente entre le ZIP, `Info.plist`, les paquets (`0.1.0.N`), le README et le commit | — |
| PKG-02 installation, découverte, instanciation | PASS : `installer` réel, bundle arm64, reçu et manifeste ; découverte et rendu depuis `/Library/Audio/Plug-Ins/VST3` par l'hôte VST3 de test | **PENDING** (DAW, M4 Max) |
| PKG-03 réinstallation, mise à jour | PASS : réinstallation de la même version, contenu identique. Mise à jour depuis une version antérieure simulée (bac à sable). Mise à jour réelle : N/A, première version. | **PENDING** |
| PKG-04 désinstallation | PASS : bundle, manifeste et reçu retirés ; autre plugin et données utilisateur conservés | **PENDING** |
| PKG-05 réinstallation après désinstallation | PASS, données conservées | **PENDING** |
| PKG-06 cas d'échec et cas étrangers | PASS. **Réel :** plugin absent, désinstallateur plus ancien, élément tiers, lien symbolique, chacun refusé avec un message explicite. **Bac à sable :** échec de vérification suivi d'une restauration à l'identique, échec sans installation antérieure, plugin chargé par un processus. | — |
| PKG-07 documentation, signature | PASS pour la documentation : statut exact dans README_FR. Signature Developer ID, notarisation et Gatekeeper : **non applicables à une livraison interne, bloquées pour la distribution publique**. | — |
| PKG-08 rapport de recette | Ce rapport et les journaux CI (macOS, architecture, hôte de test, versions) | **PENDING** : DAW et Mac réels |

## 7. Signature et notarisation

- Bundle VST3 : signature **ad hoc** (`Signature=adhoc`, `TeamIdentifier=not set`).
- Paquets : **non signés** (`Status: no signature`).
- Aucune notarisation.
- Aucun certificat, secret ou identifiant n'est dans Git.

Conséquence : la livraison est **interne uniquement**. Pour installer, macOS demande d'autoriser le paquet par
Réglages Système → Confidentialité et sécurité → « Ouvrir quand même ». **Aucune désactivation globale de Gatekeeper
n'est demandée.**

## 8. Limitations et points ouverts

1. **Licence (bloquant pour toute diffusion).** JUCE 9 est sous AGPLv3 ou sous licence commerciale, et ce choix n'est
   pas arbitré. Le dépôt est public : le lien de livraison rend donc le binaire techniquement accessible à tous.
2. **Pas de DAW, pas de Mac physique.** Tous les PASS ci-dessus proviennent d'une VM Apple M1 de GitHub et d'un hôte
   VST3 de test. Aucun statut d'installation physique n'est revendiqué.
3. **Verrou interne de JUCE.** Le wrapper VST3 de JUCE prend `getCallbackLock()` autour de `processBlock`. C'est le
   comportement de JUCE ; notre code instrumenté ne contient ni verrou ni allocation.
4. **Paramètres MIDI CC.** Le wrapper JUCE expose 2103 paramètres VST3 : nos 22 paramètres plus l'émulation des
   contrôleurs MIDI. C'est le comportement par défaut de JUCE et les hôtes masquent ces paramètres.
5. **Version macOS.** 14.0 est déclaré, pas validé. M1 et M2 ne sont pas officiellement pris en charge, sans blocage
   imposé.
6. **Délai de groupe de Thiran1.** Il n'est pas constant : 1,8375 en DC, 1,879 échantillon moteur à 20 kHz. C'est
   mesuré et rapporté ; la stabilité est vérifiée.
7. **Clics.** La transition de 128 échantillons est un candidat ; elle est mesurée, sans promesse d'absence de clic.
8. **Politique de K.** K = 1 à 48, 88,2 et 96 kHz est provisoire, en attendant la politique Quality.
9. **Performances.** Aucun budget CPU ni aucune mesure de performance M3 ou M4 n'est revendiqué. Le NO-GO historique
   du benchmark M1 reste inchangé.
10. **Périmètre fonctionnel.** Pas de pitch bend, d'aftertouch ni de CC autres que 120 et 123. Topologie éditable
    seulement depuis l'UI de diagnostic.

## 9. Livraison

*(Complété après le run de livraison.)*

## 10. Suite

- Recette sur le M4 Max avec `docs/V1-ARCH-G3-RECETTE-M4MAX.md`, puis sur un M3 physique avant toute Beta.
- Arbitrage de la licence JUCE.
- Certificats Developer ID et notarisation avant toute distribution publique.
- Aucun ARCH-G4 avant la décision de la direction technique.
