# Spike S0.1 — validation finale du retard causal (B-T1 contre B-T2)

Harnais de recherche, hors VST3 : à ne pas réutiliser tel quel dans le produit.
Mission : `docs/V1-S0.1-CAUSALITY-VALIDATION.md` (sur `main`). Rapport : `docs/V1-S0.1-REPORT.md`.
Il s'appuie sur S0 (`spikes/s0/`, conservé intact ; `s0/analysis.hpp` est réutilisé).

- `include/s01/routing.hpp` : classification causale sticky, StableRouteID, sérialisation/validation.
- `include/s01/engine.hpp` : réseau WT + 4 OP, PM / Linear FM / TZFM / Exponential FM, rampes linéaire/smoothstep, rendu par blocs hôte.
- `include/s01/wavetable.hpp` : tables réelles 2048 éch. mip-mappées (sine, triangle, saw, asym).
- `include/s01/resampler.hpp` : décimation temps réel 2×/4× (demi-bandes FIR Kaiser, ~100 dB).
- `include/s01/fracdelay.hpp` : T1 / T2, Lagrange3 / Linear / Thiran1, refus explicite de T2 non représentable.
- `tests/s01_tests.cpp`, `src/s01_cli.cpp` (campagnes), `bench/s01_bench.cpp` (benchmark M1), `scripts/` (agrégation).
- `data/` : CSV bruts + `summary.md` ; `audio/` : rendus aveugles X/Y, `LISTENING.md` (à remplir), `KEY*.md/csv` (clé).

```
make test                                   # macOS/Linux ; Windows : build_msvc.bat puis build\s01_tests.exe
make && ./build/s01_cli all data && ./build/s01_cli audio audio
python3 -I scripts/summarize.py data audio > data/summary.md
make bench-m1                               # Apple M1 -> data/bench_m1.csv, puis relancer summarize.py
```
