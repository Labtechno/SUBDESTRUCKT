# Spike S0 — Causalité FM/CROSS

Harnais de recherche, hors VST3 : à ne pas réutiliser tel quel dans le produit.
Mission : `docs/V1-S0-CAUSALITY-SPIKE.md`. Résultats et recommandation : `docs/V1-S0-REPORT.md`.

- `include/s0/engine.hpp` : réseau PM à 5 nœuds, stratégies A / B / Bc, références T1 / T2, transactions.
- `include/s0/analysis.hpp` : FFT, Welch par bandes de tiers d'octave, décimation, mesure de retard (hors ligne).
- `include/s0/configs.hpp` : les 10 configurations de la mission.
- `tests/s0_tests.cpp` : invariants automatisés (`make test`).
- `src/s0_cli.cpp` : campagnes `matrix`, `delay`, `transition`, `perf`.
- `scripts/` : agrégation des CSV et réponse du délai fractionnaire.
- `data/` : données brutes (CSV) et tableaux comparatifs.

Build : `make` (C++17, g++ ou clang, aucune dépendance). Voir §11 du rapport pour la reproduction complète.
