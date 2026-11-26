# SUBDESTRUCKT — Registre des paramètres (généré)

Généré par `tools/gen_params.py` depuis la table compilée. 684 paramètres. Ne pas éditer.

| # | StableID | Nom | Type | Unité | Min | Max | Défaut | Auto | Rates | Domaine | Version |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 0 | `amp.attack` | Amp Attack | Float | s | 0.001 | 5.0 | 0.005 | oui | C | Time | 1 |
| 1 | `amp.decay` | Amp Decay | Float | s | 0.001 | 5.0 | 0.25 | oui | C | Time | 1 |
| 2 | `amp.sustain` | Amp Sustain | Float | lin | 0.0 | 1.0 | 0.8 | oui | C | Lin | 1 |
| 3 | `amp.release` | Amp Release | Float | s | 0.001 | 10.0 | 0.3 | oui | C | Time | 1 |
| 4 | `amp.velocity` | Amp Velocity | Float | lin | 0.0 | 1.0 | 1.0 | oui | C | Lin | 1 |
| 5 | `out.gain` | Output Gain | Float | dB | -48.0 | 6.0 | -6.0 | oui | C | Db | 1 |
| 6 | `wt.level` | WT Level | Float | lin | 0.0 | 1.0 | 0.5 | oui | CA | Lin | 1 |
| 7 | `op1.level` | OP1 Level | Float | lin | 0.0 | 1.0 | 0.5 | oui | CA | Lin | 1 |
| 8 | `op2.level` | OP2 Level | Float | lin | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 1 |
| 9 | `op1.ratio` | OP1 Ratio | Float | x f0 | 0.25 | 16.0 | 1.0 | oui | CA | Oct | 1 |
| 10 | `op2.ratio` | OP2 Ratio | Float | x f0 | 0.25 | 16.0 | 2.0 | oui | CA | Oct | 1 |
| 11 | `fm.op2_op1.depth` | FM OP2>OP1 | Float | rad | 0.0 | 8.0 | 1.0 | oui | CA | Rad | 1 |
| 12 | `fm.op1_op2.depth` | FM OP1>OP2 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 1 |
| 13 | `fm.op1_op1.depth` | FM OP1 Self | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 1 |
| 14 | `fm.op2_op2.depth` | FM OP2 Self | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 1 |
| 15 | `cross.wt_op1.depth` | CROSS WT>OP1 | Float | rad | 0.0 | 8.0 | 0.5 | oui | CA | Rad | 1 |
| 16 | `cross.wt_op2.depth` | CROSS WT>OP2 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 1 |
| 17 | `cross.op1_wt.depth` | CROSS OP1>WT | Float | rad | 0.0 | 8.0 | 0.5 | oui | CA | Rad | 1 |
| 18 | `cross.op2_wt.depth` | CROSS OP2>WT | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 1 |
| 19 | `cross.master` | CROSS Master | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 1 |
| 20 | `cross.fwd` | CROSS WT>FM | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 1 |
| 21 | `cross.rev` | CROSS FM>WT | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 1 |
| 22 | `voice.mode` | Voice Mode | Choice (Mono / Legato) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 23 | `voice.priority` | Note Priority | Choice (Last / Low / High) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 24 | `voice.bend_up` | Bend Up | Float | st | 0.0 | 48.0 | 2.0 | oui | E | Lin | 2 |
| 25 | `voice.bend_down` | Bend Down | Float | st | 0.0 | 48.0 | 2.0 | oui | E | Lin | 2 |
| 26 | `tune.a4` | Reference A4 | Float | Hz | 400.0 | 480.0 | 440.0 | oui | C | Lin | 2 |
| 27 | `tune.scale` | Scale | Choice (Equal / Custom) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 28 | `tune.pc0` | Tune C | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 29 | `tune.pc1` | Tune C# | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 30 | `tune.pc2` | Tune D | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 31 | `tune.pc3` | Tune D# | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 32 | `tune.pc4` | Tune E | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 33 | `tune.pc5` | Tune F | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 34 | `tune.pc6` | Tune F# | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 35 | `tune.pc7` | Tune G | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 36 | `tune.pc8` | Tune G# | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 37 | `tune.pc9` | Tune A | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 38 | `tune.pc10` | Tune A# | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 39 | `tune.pc11` | Tune B | Float | ct | -100.0 | 100.0 | 0.0 | oui | E | Lin | 2 |
| 40 | `amp.delay` | Amp Delay | Float | s | 0.0 | 5.0 | 0.0 | oui | C | Lin | 2 |
| 41 | `amp.hold` | Amp Hold | Float | s | 0.0 | 5.0 | 0.0 | oui | C | Lin | 2 |
| 42 | `amp.curve` | Amp Curve | Choice (Linear / Exponential) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 43 | `env.wt.delay` | ENV WT Delay | Float | s | 0.0 | 10.0 | 0.0 | oui | C | Lin | 2 |
| 44 | `env.wt.attack` | ENV WT Attack | Float | s | 0.001 | 20.0 | 0.01 | oui | C | Time | 2 |
| 45 | `env.wt.hold` | ENV WT Hold | Float | s | 0.0 | 10.0 | 0.0 | oui | C | Lin | 2 |
| 46 | `env.wt.decay` | ENV WT Decay | Float | s | 0.001 | 20.0 | 0.3 | oui | C | Time | 2 |
| 47 | `env.wt.sustain` | ENV WT Sustain | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 48 | `env.wt.release` | ENV WT Release | Float | s | 0.001 | 20.0 | 0.3 | oui | C | Time | 2 |
| 49 | `env.wt.curve` | ENV WT Curve | Choice (Linear / Exponential) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 50 | `env.wt.loop` | ENV WT Loop | Choice (Off / Loop) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 51 | `env.fm.delay` | ENV FM Delay | Float | s | 0.0 | 10.0 | 0.0 | oui | C | Lin | 2 |
| 52 | `env.fm.attack` | ENV FM Attack | Float | s | 0.001 | 20.0 | 0.01 | oui | C | Time | 2 |
| 53 | `env.fm.hold` | ENV FM Hold | Float | s | 0.0 | 10.0 | 0.0 | oui | C | Lin | 2 |
| 54 | `env.fm.decay` | ENV FM Decay | Float | s | 0.001 | 20.0 | 0.3 | oui | C | Time | 2 |
| 55 | `env.fm.sustain` | ENV FM Sustain | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 56 | `env.fm.release` | ENV FM Release | Float | s | 0.001 | 20.0 | 0.3 | oui | C | Time | 2 |
| 57 | `env.fm.curve` | ENV FM Curve | Choice (Linear / Exponential) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 58 | `env.fm.loop` | ENV FM Loop | Choice (Off / Loop) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 59 | `env.aux1.delay` | ENV AUX1 Delay | Float | s | 0.0 | 10.0 | 0.0 | oui | C | Lin | 2 |
| 60 | `env.aux1.attack` | ENV AUX1 Attack | Float | s | 0.001 | 20.0 | 0.01 | oui | C | Time | 2 |
| 61 | `env.aux1.hold` | ENV AUX1 Hold | Float | s | 0.0 | 10.0 | 0.0 | oui | C | Lin | 2 |
| 62 | `env.aux1.decay` | ENV AUX1 Decay | Float | s | 0.001 | 20.0 | 0.3 | oui | C | Time | 2 |
| 63 | `env.aux1.sustain` | ENV AUX1 Sustain | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 64 | `env.aux1.release` | ENV AUX1 Release | Float | s | 0.001 | 20.0 | 0.3 | oui | C | Time | 2 |
| 65 | `env.aux1.curve` | ENV AUX1 Curve | Choice (Linear / Exponential) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 66 | `env.aux1.loop` | ENV AUX1 Loop | Choice (Off / Loop) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 67 | `env.aux2.delay` | ENV AUX2 Delay | Float | s | 0.0 | 10.0 | 0.0 | oui | C | Lin | 2 |
| 68 | `env.aux2.attack` | ENV AUX2 Attack | Float | s | 0.001 | 20.0 | 0.01 | oui | C | Time | 2 |
| 69 | `env.aux2.hold` | ENV AUX2 Hold | Float | s | 0.0 | 10.0 | 0.0 | oui | C | Lin | 2 |
| 70 | `env.aux2.decay` | ENV AUX2 Decay | Float | s | 0.001 | 20.0 | 0.3 | oui | C | Time | 2 |
| 71 | `env.aux2.sustain` | ENV AUX2 Sustain | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 72 | `env.aux2.release` | ENV AUX2 Release | Float | s | 0.001 | 20.0 | 0.3 | oui | C | Time | 2 |
| 73 | `env.aux2.curve` | ENV AUX2 Curve | Choice (Linear / Exponential) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 74 | `env.aux2.loop` | ENV AUX2 Loop | Choice (Off / Loop) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 75 | `lfo1.shape` | LFO1 Shape | Choice (Sine / Triangle / Saw Up / Saw Down / Square / Random Hold / Random Smooth / Custom) | — | 0 | 7 | 0 | oui | E | Disc | 2 |
| 76 | `lfo1.rate` | LFO1 Rate | Float | Hz | 0.01 | 1000.0 | 1.0 | oui | C | Oct | 2 |
| 77 | `lfo1.sync` | LFO1 Tempo Sync | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 78 | `lfo1.div` | LFO1 Division | Choice (8/1 / 4/1 / 2/1 / 1/1 / 1/2 / 1/4 / 1/8 / 1/16 / 1/32 / 1/4T / 1/8T / 1/16T / 1/4D / 1/8D / 1/16D) | — | 0 | 14 | 5 | oui | E | Disc | 2 |
| 79 | `lfo1.phase` | LFO1 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 80 | `lfo1.retrig` | LFO1 Retrigger | Choice (Free / Note) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 81 | `lfo2.shape` | LFO2 Shape | Choice (Sine / Triangle / Saw Up / Saw Down / Square / Random Hold / Random Smooth / Custom) | — | 0 | 7 | 0 | oui | E | Disc | 2 |
| 82 | `lfo2.rate` | LFO2 Rate | Float | Hz | 0.01 | 1000.0 | 1.0 | oui | C | Oct | 2 |
| 83 | `lfo2.sync` | LFO2 Tempo Sync | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 84 | `lfo2.div` | LFO2 Division | Choice (8/1 / 4/1 / 2/1 / 1/1 / 1/2 / 1/4 / 1/8 / 1/16 / 1/32 / 1/4T / 1/8T / 1/16T / 1/4D / 1/8D / 1/16D) | — | 0 | 14 | 5 | oui | E | Disc | 2 |
| 85 | `lfo2.phase` | LFO2 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 86 | `lfo2.retrig` | LFO2 Retrigger | Choice (Free / Note) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 87 | `lfo3.shape` | LFO3 Shape | Choice (Sine / Triangle / Saw Up / Saw Down / Square / Random Hold / Random Smooth / Custom) | — | 0 | 7 | 0 | oui | E | Disc | 2 |
| 88 | `lfo3.rate` | LFO3 Rate | Float | Hz | 0.01 | 1000.0 | 1.0 | oui | C | Oct | 2 |
| 89 | `lfo3.sync` | LFO3 Tempo Sync | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 90 | `lfo3.div` | LFO3 Division | Choice (8/1 / 4/1 / 2/1 / 1/1 / 1/2 / 1/4 / 1/8 / 1/16 / 1/32 / 1/4T / 1/8T / 1/16T / 1/4D / 1/8D / 1/16D) | — | 0 | 14 | 5 | oui | E | Disc | 2 |
| 91 | `lfo3.phase` | LFO3 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 92 | `lfo3.retrig` | LFO3 Retrigger | Choice (Free / Note) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 93 | `lfo4.shape` | LFO4 Shape | Choice (Sine / Triangle / Saw Up / Saw Down / Square / Random Hold / Random Smooth / Custom) | — | 0 | 7 | 0 | oui | E | Disc | 2 |
| 94 | `lfo4.rate` | LFO4 Rate | Float | Hz | 0.01 | 1000.0 | 1.0 | oui | C | Oct | 2 |
| 95 | `lfo4.sync` | LFO4 Tempo Sync | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 96 | `lfo4.div` | LFO4 Division | Choice (8/1 / 4/1 / 2/1 / 1/1 / 1/2 / 1/4 / 1/8 / 1/16 / 1/32 / 1/4T / 1/8T / 1/16T / 1/4D / 1/8D / 1/16D) | — | 0 | 14 | 5 | oui | E | Disc | 2 |
| 97 | `lfo4.phase` | LFO4 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 98 | `lfo4.retrig` | LFO4 Retrigger | Choice (Free / Note) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 99 | `rnd.hold.rate` | Random Hold Rate | Float | Hz | 0.01 | 100.0 | 2.0 | oui | C | Oct | 2 |
| 100 | `rnd.smooth.rate` | Random Smooth Rate | Float | Hz | 0.01 | 100.0 | 1.0 | oui | C | Oct | 2 |
| 101 | `chaos.rate` | Chaos Rate | Float | Hz | 0.01 | 100.0 | 0.5 | oui | C | Oct | 2 |
| 102 | `fol1.attack` | Follower1 Attack | Float | s | 0.0005 | 1.0 | 0.01 | oui | C | Time | 2 |
| 103 | `fol1.release` | Follower1 Release | Float | s | 0.005 | 5.0 | 0.2 | oui | C | Time | 2 |
| 104 | `fol2.attack` | Follower2 Attack | Float | s | 0.0005 | 1.0 | 0.01 | oui | C | Time | 2 |
| 105 | `fol2.release` | Follower2 Release | Float | s | 0.005 | 5.0 | 0.2 | oui | C | Time | 2 |
| 106 | `fol3.attack` | Follower3 Attack | Float | s | 0.0005 | 1.0 | 0.01 | oui | C | Time | 2 |
| 107 | `fol3.release` | Follower3 Release | Float | s | 0.005 | 5.0 | 0.2 | oui | C | Time | 2 |
| 108 | `wt.x` | WT X | Float | pos | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 109 | `wt.y` | WT Y | Float | pos | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 110 | `wt.z` | WT Z Master | Float | lin | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 111 | `wt.x_bound` | WT X Boundary | Choice (Clamp / Wrap / Mirror) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 112 | `wt.y_bound` | WT Y Boundary | Choice (Clamp / Wrap / Mirror) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 113 | `wt.x_morph` | WT X Morph | Choice (Linear / Stepped) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 114 | `wt.y_morph` | WT Y Morph | Choice (Linear / Stepped) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 115 | `wt.interp` | WT Interpolation | Choice (Linear / Cubic) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 116 | `wt.render` | WT Clean/Raw | Choice (Clean / Raw) | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 117 | `wt.coarse` | WT Coarse | Float | st | -48.0 | 48.0 | 0.0 | oui | CA | Semi | 2 |
| 118 | `wt.fine` | WT Fine | Float | ct | -100.0 | 100.0 | 0.0 | oui | CA | Lin | 2 |
| 119 | `wt.phase_mode` | WT Phase Mode | Choice (Free / Retrigger / Fixed / Random) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 120 | `wt.phase` | WT Start Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | E | Cyc | 2 |
| 121 | `wt.sync` | WT Sync | Choice (Off / Hard / Soft) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 122 | `wt.sync_src` | WT Sync Source | Choice (Internal / OP1 / OP2 / OP3 / OP4) | — | 0 | 4 | 0 | non | E | Disc | 2 |
| 123 | `wt.char` | WT Character | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 124 | `wt.char_mode` | WT Character Profile | Choice (Phase Quantize / Bit Reduce / Sample Hold) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 125 | `wt.za.type` | WT ZA Transform | Choice (Off / Harmonic Fold / Phase Warp / Spectral Mutation) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 126 | `wt.za.depth` | WT ZA Depth | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 127 | `wt.za.curve` | WT ZA Curve | Choice (Linear / Exponential / Logarithmic / S-Curve) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 128 | `wt.zb.type` | WT ZB Transform | Choice (Off / Harmonic Fold / Phase Warp / Spectral Mutation) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 129 | `wt.zb.depth` | WT ZB Depth | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 130 | `wt.zb.curve` | WT ZB Curve | Choice (Linear / Exponential / Logarithmic / S-Curve) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 131 | `wt.z_order` | WT Z Order | Choice (A>B / B>A) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 132 | `wt.z_fund` | WT Z Keep Fundamental | Bool | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 133 | `wt.z_gain` | WT Z Gain Compensation | Bool | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 134 | `op1.tune` | OP1 Tuning | Choice (Ratio / Fixed) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 135 | `op1.fixed` | OP1 Fixed | Float | Hz | 0.01 | 20000.0 | 440.0 | oui | CA | Oct | 2 |
| 136 | `op1.offset` | OP1 Offset | Float | Hz | -2000.0 | 2000.0 | 0.0 | oui | CA | Lin | 2 |
| 137 | `op1.coarse` | OP1 Coarse | Float | st | -48.0 | 48.0 | 0.0 | oui | CA | Semi | 2 |
| 138 | `op1.fine` | OP1 Fine | Float | ct | -100.0 | 100.0 | 0.0 | oui | CA | Lin | 2 |
| 139 | `op1.wave` | OP1 Waveform | Choice (Sine / Triangle / Saw / Pulse / Folded Sine / Asymmetric / Phase Distorted / Harmonic / Wavetable) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 140 | `op1.wt_x` | OP1 WT X | Float | pos | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 141 | `op1.wt_y` | OP1 WT Y | Float | pos | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 142 | `op1.pan` | OP1 Pan | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 143 | `op1.mute` | OP1 Mute | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 144 | `op1.enable` | OP1 Enable | Bool | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 145 | `op1.char` | OP1 Character | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 146 | `op1.phase_mode` | OP1 Phase Mode | Choice (Free / Retrigger / Random) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 147 | `op1.ac` | OP1 Mod Coupling | Choice (DC / AC) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 148 | `op1.mode` | OP1 Mod Mode | Choice (PM / Linear FM / Through-Zero FM / Exponential FM) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 149 | `op1.sync` | OP1 Sync | Choice (Off / WT) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 150 | `op2.tune` | OP2 Tuning | Choice (Ratio / Fixed) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 151 | `op2.fixed` | OP2 Fixed | Float | Hz | 0.01 | 20000.0 | 440.0 | oui | CA | Oct | 2 |
| 152 | `op2.offset` | OP2 Offset | Float | Hz | -2000.0 | 2000.0 | 0.0 | oui | CA | Lin | 2 |
| 153 | `op2.coarse` | OP2 Coarse | Float | st | -48.0 | 48.0 | 0.0 | oui | CA | Semi | 2 |
| 154 | `op2.fine` | OP2 Fine | Float | ct | -100.0 | 100.0 | 0.0 | oui | CA | Lin | 2 |
| 155 | `op2.wave` | OP2 Waveform | Choice (Sine / Triangle / Saw / Pulse / Folded Sine / Asymmetric / Phase Distorted / Harmonic / Wavetable) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 156 | `op2.wt_x` | OP2 WT X | Float | pos | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 157 | `op2.wt_y` | OP2 WT Y | Float | pos | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 158 | `op2.pan` | OP2 Pan | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 159 | `op2.mute` | OP2 Mute | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 160 | `op2.enable` | OP2 Enable | Bool | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 161 | `op2.char` | OP2 Character | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 162 | `op2.phase_mode` | OP2 Phase Mode | Choice (Free / Retrigger / Random) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 163 | `op2.ac` | OP2 Mod Coupling | Choice (DC / AC) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 164 | `op2.mode` | OP2 Mod Mode | Choice (PM / Linear FM / Through-Zero FM / Exponential FM) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 165 | `op2.sync` | OP2 Sync | Choice (Off / WT) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 166 | `op3.level` | OP3 Level | Float | lin | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 167 | `op3.ratio` | OP3 Ratio | Float | x f0 | 0.25 | 16.0 | 3.0 | oui | CA | Oct | 2 |
| 168 | `op3.tune` | OP3 Tuning | Choice (Ratio / Fixed) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 169 | `op3.fixed` | OP3 Fixed | Float | Hz | 0.01 | 20000.0 | 440.0 | oui | CA | Oct | 2 |
| 170 | `op3.offset` | OP3 Offset | Float | Hz | -2000.0 | 2000.0 | 0.0 | oui | CA | Lin | 2 |
| 171 | `op3.coarse` | OP3 Coarse | Float | st | -48.0 | 48.0 | 0.0 | oui | CA | Semi | 2 |
| 172 | `op3.fine` | OP3 Fine | Float | ct | -100.0 | 100.0 | 0.0 | oui | CA | Lin | 2 |
| 173 | `op3.wave` | OP3 Waveform | Choice (Sine / Triangle / Saw / Pulse / Folded Sine / Asymmetric / Phase Distorted / Harmonic / Wavetable) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 174 | `op3.wt_x` | OP3 WT X | Float | pos | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 175 | `op3.wt_y` | OP3 WT Y | Float | pos | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 176 | `op3.pan` | OP3 Pan | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 177 | `op3.mute` | OP3 Mute | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 178 | `op3.enable` | OP3 Enable | Bool | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 179 | `op3.char` | OP3 Character | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 180 | `op3.phase_mode` | OP3 Phase Mode | Choice (Free / Retrigger / Random) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 181 | `op3.ac` | OP3 Mod Coupling | Choice (DC / AC) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 182 | `op3.mode` | OP3 Mod Mode | Choice (PM / Linear FM / Through-Zero FM / Exponential FM) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 183 | `op3.sync` | OP3 Sync | Choice (Off / WT) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 184 | `op4.level` | OP4 Level | Float | lin | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 185 | `op4.ratio` | OP4 Ratio | Float | x f0 | 0.25 | 16.0 | 4.0 | oui | CA | Oct | 2 |
| 186 | `op4.tune` | OP4 Tuning | Choice (Ratio / Fixed) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 187 | `op4.fixed` | OP4 Fixed | Float | Hz | 0.01 | 20000.0 | 440.0 | oui | CA | Oct | 2 |
| 188 | `op4.offset` | OP4 Offset | Float | Hz | -2000.0 | 2000.0 | 0.0 | oui | CA | Lin | 2 |
| 189 | `op4.coarse` | OP4 Coarse | Float | st | -48.0 | 48.0 | 0.0 | oui | CA | Semi | 2 |
| 190 | `op4.fine` | OP4 Fine | Float | ct | -100.0 | 100.0 | 0.0 | oui | CA | Lin | 2 |
| 191 | `op4.wave` | OP4 Waveform | Choice (Sine / Triangle / Saw / Pulse / Folded Sine / Asymmetric / Phase Distorted / Harmonic / Wavetable) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 192 | `op4.wt_x` | OP4 WT X | Float | pos | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 193 | `op4.wt_y` | OP4 WT Y | Float | pos | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 194 | `op4.pan` | OP4 Pan | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 195 | `op4.mute` | OP4 Mute | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 196 | `op4.enable` | OP4 Enable | Bool | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 197 | `op4.char` | OP4 Character | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 198 | `op4.phase_mode` | OP4 Phase Mode | Choice (Free / Retrigger / Random) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 199 | `op4.ac` | OP4 Mod Coupling | Choice (DC / AC) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 200 | `op4.mode` | OP4 Mod Mode | Choice (PM / Linear FM / Through-Zero FM / Exponential FM) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 201 | `op4.sync` | OP4 Sync | Choice (Off / WT) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 202 | `fm.char_master` | FM Character Master | Float | lin | 0.0 | 1.0 | 1.0 | oui | C | Lin | 2 |
| 203 | `fm.fb_mode` | FM Feedback Reset | Choice (Continuous / Note Reset) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 204 | `fm.op1_op3.depth` | FM OP1>OP3 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 205 | `fm.op1_op4.depth` | FM OP1>OP4 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 206 | `fm.op2_op3.depth` | FM OP2>OP3 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 207 | `fm.op2_op4.depth` | FM OP2>OP4 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 208 | `fm.op3_op1.depth` | FM OP3>OP1 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 209 | `fm.op3_op2.depth` | FM OP3>OP2 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 210 | `fm.op3_op3.depth` | FM OP3 Self | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 211 | `fm.op3_op4.depth` | FM OP3>OP4 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 212 | `fm.op4_op1.depth` | FM OP4>OP1 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 213 | `fm.op4_op2.depth` | FM OP4>OP2 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 214 | `fm.op4_op3.depth` | FM OP4>OP3 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 215 | `fm.op4_op4.depth` | FM OP4 Self | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 216 | `cross.wt_op3.depth` | CROSS WT>OP3 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 217 | `cross.wt_op4.depth` | CROSS WT>OP4 | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 218 | `cross.op3_wt.depth` | CROSS OP3>WT | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 219 | `cross.op4_wt.depth` | CROSS OP4>WT | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 220 | `cross.bus_wt.depth` | CROSS BUS>WT | Float | rad | 0.0 | 8.0 | 0.0 | oui | CA | Rad | 2 |
| 221 | `cross.op1_wt_freq.depth` | CROSS OP1>WT Freq | Float | x | 0.0 | 8.0 | 0.0 | oui | CA | Lin | 2 |
| 222 | `cross.op1_wt_x.depth` | CROSS OP1>WT X | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 223 | `cross.op1_wt_y.depth` | CROSS OP1>WT Y | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 224 | `cross.op1_wt_z.depth` | CROSS OP1>WT Z | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 225 | `cross.op1_wt_level.depth` | CROSS OP1>WT Level | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 226 | `cross.op1_wt_char.depth` | CROSS OP1>WT Char | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 227 | `cross.op2_wt_freq.depth` | CROSS OP2>WT Freq | Float | x | 0.0 | 8.0 | 0.0 | oui | CA | Lin | 2 |
| 228 | `cross.op2_wt_x.depth` | CROSS OP2>WT X | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 229 | `cross.op2_wt_y.depth` | CROSS OP2>WT Y | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 230 | `cross.op2_wt_z.depth` | CROSS OP2>WT Z | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 231 | `cross.op2_wt_level.depth` | CROSS OP2>WT Level | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 232 | `cross.op2_wt_char.depth` | CROSS OP2>WT Char | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 233 | `cross.op3_wt_freq.depth` | CROSS OP3>WT Freq | Float | x | 0.0 | 8.0 | 0.0 | oui | CA | Lin | 2 |
| 234 | `cross.op3_wt_x.depth` | CROSS OP3>WT X | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 235 | `cross.op3_wt_y.depth` | CROSS OP3>WT Y | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 236 | `cross.op3_wt_z.depth` | CROSS OP3>WT Z | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 237 | `cross.op3_wt_level.depth` | CROSS OP3>WT Level | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 238 | `cross.op3_wt_char.depth` | CROSS OP3>WT Char | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 239 | `cross.op4_wt_freq.depth` | CROSS OP4>WT Freq | Float | x | 0.0 | 8.0 | 0.0 | oui | CA | Lin | 2 |
| 240 | `cross.op4_wt_x.depth` | CROSS OP4>WT X | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 241 | `cross.op4_wt_y.depth` | CROSS OP4>WT Y | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 242 | `cross.op4_wt_z.depth` | CROSS OP4>WT Z | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 243 | `cross.op4_wt_level.depth` | CROSS OP4>WT Level | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 244 | `cross.op4_wt_char.depth` | CROSS OP4>WT Char | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 245 | `cross.bus_wt_freq.depth` | CROSS BUS>WT Freq | Float | x | 0.0 | 8.0 | 0.0 | oui | CA | Lin | 2 |
| 246 | `cross.bus_wt_x.depth` | CROSS BUS>WT X | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 247 | `cross.bus_wt_y.depth` | CROSS BUS>WT Y | Float | pos | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 248 | `cross.bus_wt_z.depth` | CROSS BUS>WT Z | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 249 | `cross.bus_wt_level.depth` | CROSS BUS>WT Level | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 250 | `cross.bus_wt_char.depth` | CROSS BUS>WT Char | Float | lin | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 251 | `cross.wt_fmode` | CROSS WT Freq Mode | Choice (Linear FM / Through-Zero FM / Exponential FM) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 252 | `cross.bus.w1` | CROSS Bus OP1 | Float | bip | -1.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 253 | `cross.bus.w2` | CROSS Bus OP2 | Float | bip | -1.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 254 | `cross.bus.w3` | CROSS Bus OP3 | Float | bip | -1.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 255 | `cross.bus.w4` | CROSS Bus OP4 | Float | bip | -1.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 256 | `cross.bus.mode` | CROSS Bus Mode | Choice (Raw / Normalized) | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 257 | `cross.dir` | CROSS Direction | Choice (Both / Forward / Reverse) | — | 0 | 2 | 0 | oui | E | Disc | 2 |
| 258 | `cross.audio.mode` | Cross Audio Mode | Choice (Mix / Crossfade / Ring / AM) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 259 | `cross.audio.amount` | Cross Audio Amount | Float | lin | 0.0 | 1.0 | 0.5 | oui | CA | Lin | 2 |
| 260 | `cross.audio.am_dir` | Cross AM Direction | Choice (WT>FM / FM>WT) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 261 | `flt.wt.type` | FLT WT Type | Choice (Off / F1 Multimode / F2 Comb / F3 Nonlinear / F4 Modal) | — | 0 | 4 | 0 | non | E | Disc | 2 |
| 262 | `flt.wt.pos` | FLT WT Position | Choice (Pre / Post) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 263 | `flt.wt.stereo` | FLT WT Stereo | Choice (Linked / Dual Mono) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 264 | `flt.wt.f1_mode` | FLT WT F1 Mode | Choice (LP12 / LP24 / BP12 / BP24 / HP12 / HP24 / Notch) | — | 0 | 6 | 0 | non | E | Disc | 2 |
| 265 | `flt.wt.cutoff` | FLT WT Cutoff | Float | Hz | 20.0 | 20000.0 | 1000.0 | oui | CA | Oct | 2 |
| 266 | `flt.wt.res` | FLT WT Resonance | Float | lin | 0.0 | 1.0 | 0.2 | oui | CA | Lin | 2 |
| 267 | `flt.wt.key` | FLT WT Keytrack | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 268 | `flt.wt.drive` | FLT WT Drive | Float | dB | 0.0 | 36.0 | 0.0 | oui | CA | Db | 2 |
| 269 | `flt.wt.bias` | FLT WT Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 270 | `flt.wt.char` | FLT WT Character | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 271 | `flt.wt.f3_mode` | FLT WT F3 Mode | Choice (LP / BP / HP) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 272 | `flt.wt.comb_fb` | FLT WT Comb Feedback | Float | bip | -1.0 | 1.0 | 0.7 | oui | CA | Lin | 2 |
| 273 | `flt.wt.comb_damp` | FLT WT Comb Damping | Float | lin | 0.0 | 1.0 | 0.2 | oui | C | Lin | 2 |
| 274 | `flt.wt.detune` | FLT WT Detune | Float | ct | 0.0 | 50.0 | 0.0 | oui | C | Lin | 2 |
| 275 | `flt.wt.spread` | FLT WT Stereo Spread | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 276 | `flt.wt.modal` | FLT WT Modal Model | Choice (Harmonic / Metal / Membrane / Inharmonic) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 277 | `flt.wt.modes` | FLT WT Modes | Int | — | 1 | 16 | 8 | non | E | Disc | 2 |
| 278 | `flt.wt.decay` | FLT WT Decay | Float | s | 0.01 | 20.0 | 1.0 | oui | C | Time | 2 |
| 279 | `flt.wt.damp` | FLT WT Damping | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 280 | `flt.wt.bright` | FLT WT Brightness | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 281 | `flt.wt.inharm` | FLT WT Inharmonicity | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 282 | `flt.wt.freeze` | FLT WT Modal Freeze | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 283 | `flt.wt.exc` | FLT WT Excitation | Choice (Off / Note / Manual) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 284 | `flt.wt.exc_level` | FLT WT Excitation Level | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 285 | `flt.wt.mix` | FLT WT Dry/Wet | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 286 | `flt.wt.out` | FLT WT Output | Float | dB | -24.0 | 24.0 | 0.0 | oui | C | Db | 2 |
| 287 | `flt.wt.arm` | FLT WT Autonomous Armed | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 288 | `flt.fm.type` | FLT FM Type | Choice (Off / F1 Multimode / F2 Comb / F3 Nonlinear / F4 Modal) | — | 0 | 4 | 0 | non | E | Disc | 2 |
| 289 | `flt.fm.pos` | FLT FM Position | Choice (Pre / Post) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 290 | `flt.fm.stereo` | FLT FM Stereo | Choice (Linked / Dual Mono) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 291 | `flt.fm.f1_mode` | FLT FM F1 Mode | Choice (LP12 / LP24 / BP12 / BP24 / HP12 / HP24 / Notch) | — | 0 | 6 | 0 | non | E | Disc | 2 |
| 292 | `flt.fm.cutoff` | FLT FM Cutoff | Float | Hz | 20.0 | 20000.0 | 1000.0 | oui | CA | Oct | 2 |
| 293 | `flt.fm.res` | FLT FM Resonance | Float | lin | 0.0 | 1.0 | 0.2 | oui | CA | Lin | 2 |
| 294 | `flt.fm.key` | FLT FM Keytrack | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 295 | `flt.fm.drive` | FLT FM Drive | Float | dB | 0.0 | 36.0 | 0.0 | oui | CA | Db | 2 |
| 296 | `flt.fm.bias` | FLT FM Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 297 | `flt.fm.char` | FLT FM Character | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 298 | `flt.fm.f3_mode` | FLT FM F3 Mode | Choice (LP / BP / HP) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 299 | `flt.fm.comb_fb` | FLT FM Comb Feedback | Float | bip | -1.0 | 1.0 | 0.7 | oui | CA | Lin | 2 |
| 300 | `flt.fm.comb_damp` | FLT FM Comb Damping | Float | lin | 0.0 | 1.0 | 0.2 | oui | C | Lin | 2 |
| 301 | `flt.fm.detune` | FLT FM Detune | Float | ct | 0.0 | 50.0 | 0.0 | oui | C | Lin | 2 |
| 302 | `flt.fm.spread` | FLT FM Stereo Spread | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 303 | `flt.fm.modal` | FLT FM Modal Model | Choice (Harmonic / Metal / Membrane / Inharmonic) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 304 | `flt.fm.modes` | FLT FM Modes | Int | — | 1 | 16 | 8 | non | E | Disc | 2 |
| 305 | `flt.fm.decay` | FLT FM Decay | Float | s | 0.01 | 20.0 | 1.0 | oui | C | Time | 2 |
| 306 | `flt.fm.damp` | FLT FM Damping | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 307 | `flt.fm.bright` | FLT FM Brightness | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 308 | `flt.fm.inharm` | FLT FM Inharmonicity | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 309 | `flt.fm.freeze` | FLT FM Modal Freeze | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 310 | `flt.fm.exc` | FLT FM Excitation | Choice (Off / Note / Manual) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 311 | `flt.fm.exc_level` | FLT FM Excitation Level | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 312 | `flt.fm.mix` | FLT FM Dry/Wet | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 313 | `flt.fm.out` | FLT FM Output | Float | dB | -24.0 | 24.0 | 0.0 | oui | C | Db | 2 |
| 314 | `flt.fm.arm` | FLT FM Autonomous Armed | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 315 | `flt.gl.type` | FLT GLOBAL Type | Choice (Off / F1 Multimode / F2 Comb / F3 Nonlinear / F4 Modal) | — | 0 | 4 | 0 | non | E | Disc | 2 |
| 316 | `flt.gl.pos` | FLT GLOBAL Position | Choice (Pre / Post) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 317 | `flt.gl.stereo` | FLT GLOBAL Stereo | Choice (Linked / Dual Mono) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 318 | `flt.gl.f1_mode` | FLT GLOBAL F1 Mode | Choice (LP12 / LP24 / BP12 / BP24 / HP12 / HP24 / Notch) | — | 0 | 6 | 0 | non | E | Disc | 2 |
| 319 | `flt.gl.cutoff` | FLT GLOBAL Cutoff | Float | Hz | 20.0 | 20000.0 | 1000.0 | oui | CA | Oct | 2 |
| 320 | `flt.gl.res` | FLT GLOBAL Resonance | Float | lin | 0.0 | 1.0 | 0.2 | oui | CA | Lin | 2 |
| 321 | `flt.gl.key` | FLT GLOBAL Keytrack | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 322 | `flt.gl.drive` | FLT GLOBAL Drive | Float | dB | 0.0 | 36.0 | 0.0 | oui | CA | Db | 2 |
| 323 | `flt.gl.bias` | FLT GLOBAL Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 324 | `flt.gl.char` | FLT GLOBAL Character | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 325 | `flt.gl.f3_mode` | FLT GLOBAL F3 Mode | Choice (LP / BP / HP) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 326 | `flt.gl.comb_fb` | FLT GLOBAL Comb Feedback | Float | bip | -1.0 | 1.0 | 0.7 | oui | CA | Lin | 2 |
| 327 | `flt.gl.comb_damp` | FLT GLOBAL Comb Damping | Float | lin | 0.0 | 1.0 | 0.2 | oui | C | Lin | 2 |
| 328 | `flt.gl.detune` | FLT GLOBAL Detune | Float | ct | 0.0 | 50.0 | 0.0 | oui | C | Lin | 2 |
| 329 | `flt.gl.spread` | FLT GLOBAL Stereo Spread | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 330 | `flt.gl.modal` | FLT GLOBAL Modal Model | Choice (Harmonic / Metal / Membrane / Inharmonic) | — | 0 | 3 | 0 | non | E | Disc | 2 |
| 331 | `flt.gl.modes` | FLT GLOBAL Modes | Int | — | 1 | 16 | 8 | non | E | Disc | 2 |
| 332 | `flt.gl.decay` | FLT GLOBAL Decay | Float | s | 0.01 | 20.0 | 1.0 | oui | C | Time | 2 |
| 333 | `flt.gl.damp` | FLT GLOBAL Damping | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 334 | `flt.gl.bright` | FLT GLOBAL Brightness | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 335 | `flt.gl.inharm` | FLT GLOBAL Inharmonicity | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 336 | `flt.gl.freeze` | FLT GLOBAL Modal Freeze | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 337 | `flt.gl.exc` | FLT GLOBAL Excitation | Choice (Off / Note / Manual) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 338 | `flt.gl.exc_level` | FLT GLOBAL Excitation Level | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 339 | `flt.gl.mix` | FLT GLOBAL Dry/Wet | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 340 | `flt.gl.out` | FLT GLOBAL Output | Float | dB | -24.0 | 24.0 | 0.0 | oui | C | Db | 2 |
| 341 | `flt.gl.arm` | FLT GLOBAL Autonomous Armed | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 342 | `dst.wt1.type` | DST WT1 Type | Choice (Off / N1 Asymmetric Wavefolder / D1 Variable Rate Crusher / Y1 Transient Destroyer / S1 Ring/Frequency Shifter / N2 Feedback Waveshaper / D2 Glitch Buffer / Y2 Envelope Shaper / S2 Spectral Freeze) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 343 | `dst.wt1.mix` | DST WT1 Dry/Wet | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 344 | `dst.wt1.out` | DST WT1 Output | Float | dB | -24.0 | 24.0 | 0.0 | oui | C | Db | 2 |
| 345 | `dst.wt1.arm` | DST WT1 Autonomous Armed | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 346 | `dst.wt1.n1.drive` | DST WT1 N1 Drive | Float | dB | -24.0 | 36.0 | 0.0 | oui | CA | Db | 2 |
| 347 | `dst.wt1.n1.fold` | DST WT1 N1 Fold | Float | lin | 0.0 | 1.0 | 0.5 | oui | CA | Lin | 2 |
| 348 | `dst.wt1.n1.bias` | DST WT1 N1 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 349 | `dst.wt1.n1.sym` | DST WT1 N1 Symmetry | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 350 | `dst.wt1.n1.stages` | DST WT1 N1 Stages | Int | — | 1 | 4 | 1 | non | E | Disc | 2 |
| 351 | `dst.wt1.n1.tone` | DST WT1 N1 Tone | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 352 | `dst.wt1.d1.bits` | DST WT1 D1 Bits | Float | bit | 2.0 | 16.0 | 8.0 | oui | C | Lin | 2 |
| 353 | `dst.wt1.d1.rate` | DST WT1 D1 Rate Ratio | Float | x | 1.0 | 128.0 | 4.0 | oui | C | Oct | 2 |
| 354 | `dst.wt1.d1.jitter` | DST WT1 D1 Jitter | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 355 | `dst.wt1.d1.mode` | DST WT1 D1 Mode | Choice (Raw / Controlled) | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 356 | `dst.wt1.d1.hold` | DST WT1 D1 Hold | Choice (Step / Smooth) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 357 | `dst.wt1.y1.thresh` | DST WT1 Y1 Threshold | Float | dB | -60.0 | 0.0 | -20.0 | oui | C | Db | 2 |
| 358 | `dst.wt1.y1.ratio` | DST WT1 Y1 Ratio | Float | :1 | 1.0 | 20.0 | 4.0 | oui | C | Lin | 2 |
| 359 | `dst.wt1.y1.attack` | DST WT1 Y1 Attack | Float | ms | 0.1 | 100.0 | 5.0 | oui | C | Time | 2 |
| 360 | `dst.wt1.y1.release` | DST WT1 Y1 Release | Float | ms | 5.0 | 1000.0 | 100.0 | oui | C | Time | 2 |
| 361 | `dst.wt1.y1.knee` | DST WT1 Y1 Knee | Float | dB | 0.0 | 24.0 | 6.0 | oui | C | Lin | 2 |
| 362 | `dst.wt1.y1.gate` | DST WT1 Y1 Gate | Float | dB | -90.0 | -20.0 | -90.0 | oui | C | Db | 2 |
| 363 | `dst.wt1.y1.transient` | DST WT1 Y1 Transient | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 364 | `dst.wt1.y1.makeup` | DST WT1 Y1 Makeup | Float | dB | 0.0 | 36.0 | 0.0 | oui | C | Db | 2 |
| 365 | `dst.wt1.s1.mode` | DST WT1 S1 Mode | Choice (Ring / Shift) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 366 | `dst.wt1.s1.freq` | DST WT1 S1 Frequency | Float | Hz | 0.1 | 8000.0 | 100.0 | oui | CA | Oct | 2 |
| 367 | `dst.wt1.s1.shift` | DST WT1 S1 Shift | Float | Hz | -5000.0 | 5000.0 | 0.0 | oui | CA | Lin | 2 |
| 368 | `dst.wt1.s1.carrier` | DST WT1 S1 Carrier | Choice (Sine / Triangle / Square) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 369 | `dst.wt1.s1.phase` | DST WT1 S1 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 370 | `dst.wt1.s1.stereo` | DST WT1 S1 Stereo Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 371 | `dst.wt1.n2.fb` | DST WT1 N2 Feedback | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 372 | `dst.wt1.n2.delay` | DST WT1 N2 Delay | Float | ms | 0.1 | 50.0 | 5.0 | oui | C | Time | 2 |
| 373 | `dst.wt1.n2.damp` | DST WT1 N2 Damping | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 374 | `dst.wt1.n2.bias` | DST WT1 N2 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 375 | `dst.wt1.n2.res` | DST WT1 N2 Resonance | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 376 | `dst.wt1.n2.selfosc` | DST WT1 N2 Self-Oscillation | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 377 | `dst.wt1.d2.size` | DST WT1 D2 Size | Float | ms | 10.0 | 2000.0 | 250.0 | oui | C | Time | 2 |
| 378 | `dst.wt1.d2.speed` | DST WT1 D2 Playback | Float | x | -4.0 | 4.0 | 1.0 | oui | C | Lin | 2 |
| 379 | `dst.wt1.d2.repeat` | DST WT1 D2 Repeat | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 380 | `dst.wt1.d2.pos` | DST WT1 D2 Position | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 381 | `dst.wt1.d2.fb` | DST WT1 D2 Feedback | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 382 | `dst.wt1.d2.xfade` | DST WT1 D2 Crossfade | Float | ms | 0.0 | 50.0 | 5.0 | oui | C | Lin | 2 |
| 383 | `dst.wt1.d2.freeze` | DST WT1 D2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 384 | `dst.wt1.y2.attack` | DST WT1 Y2 Attack | Float | ms | 0.1 | 200.0 | 5.0 | oui | C | Time | 2 |
| 385 | `dst.wt1.y2.release` | DST WT1 Y2 Release | Float | ms | 5.0 | 2000.0 | 200.0 | oui | C | Time | 2 |
| 386 | `dst.wt1.y2.sens` | DST WT1 Y2 Sensitivity | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 387 | `dst.wt1.y2.curve` | DST WT1 Y2 Curve | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 388 | `dst.wt1.y2.depth` | DST WT1 Y2 Depth | Float | bip | -1.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 389 | `dst.wt1.y2.hold` | DST WT1 Y2 Hold | Float | ms | 0.0 | 500.0 | 0.0 | oui | C | Lin | 2 |
| 390 | `dst.wt1.y2.detect` | DST WT1 Y2 Detector | Choice (Peak / RMS) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 391 | `dst.wt1.s2.fft` | DST WT1 S2 FFT Size | Choice (512 / 1024 / 2048 / 4096) | — | 0 | 3 | 2 | non | E | Disc | 2 |
| 392 | `dst.wt1.s2.freeze` | DST WT1 S2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 393 | `dst.wt1.s2.decay` | DST WT1 S2 Decay | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 394 | `dst.wt1.s2.blur` | DST WT1 S2 Blur | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 395 | `dst.wt1.s2.tilt` | DST WT1 S2 Tilt | Float | dB/oct | -6.0 | 6.0 | 0.0 | oui | C | Lin | 2 |
| 396 | `dst.wt1.s2.drift` | DST WT1 S2 Phase Drift | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 397 | `dst.wt1.s2.shift` | DST WT1 S2 Shift | Float | st | -24.0 | 24.0 | 0.0 | oui | C | Lin | 2 |
| 398 | `dst.wt2.type` | DST WT2 Type | Choice (Off / N1 Asymmetric Wavefolder / D1 Variable Rate Crusher / Y1 Transient Destroyer / S1 Ring/Frequency Shifter / N2 Feedback Waveshaper / D2 Glitch Buffer / Y2 Envelope Shaper / S2 Spectral Freeze) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 399 | `dst.wt2.mix` | DST WT2 Dry/Wet | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 400 | `dst.wt2.out` | DST WT2 Output | Float | dB | -24.0 | 24.0 | 0.0 | oui | C | Db | 2 |
| 401 | `dst.wt2.arm` | DST WT2 Autonomous Armed | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 402 | `dst.wt2.n1.drive` | DST WT2 N1 Drive | Float | dB | -24.0 | 36.0 | 0.0 | oui | CA | Db | 2 |
| 403 | `dst.wt2.n1.fold` | DST WT2 N1 Fold | Float | lin | 0.0 | 1.0 | 0.5 | oui | CA | Lin | 2 |
| 404 | `dst.wt2.n1.bias` | DST WT2 N1 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 405 | `dst.wt2.n1.sym` | DST WT2 N1 Symmetry | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 406 | `dst.wt2.n1.stages` | DST WT2 N1 Stages | Int | — | 1 | 4 | 1 | non | E | Disc | 2 |
| 407 | `dst.wt2.n1.tone` | DST WT2 N1 Tone | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 408 | `dst.wt2.d1.bits` | DST WT2 D1 Bits | Float | bit | 2.0 | 16.0 | 8.0 | oui | C | Lin | 2 |
| 409 | `dst.wt2.d1.rate` | DST WT2 D1 Rate Ratio | Float | x | 1.0 | 128.0 | 4.0 | oui | C | Oct | 2 |
| 410 | `dst.wt2.d1.jitter` | DST WT2 D1 Jitter | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 411 | `dst.wt2.d1.mode` | DST WT2 D1 Mode | Choice (Raw / Controlled) | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 412 | `dst.wt2.d1.hold` | DST WT2 D1 Hold | Choice (Step / Smooth) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 413 | `dst.wt2.y1.thresh` | DST WT2 Y1 Threshold | Float | dB | -60.0 | 0.0 | -20.0 | oui | C | Db | 2 |
| 414 | `dst.wt2.y1.ratio` | DST WT2 Y1 Ratio | Float | :1 | 1.0 | 20.0 | 4.0 | oui | C | Lin | 2 |
| 415 | `dst.wt2.y1.attack` | DST WT2 Y1 Attack | Float | ms | 0.1 | 100.0 | 5.0 | oui | C | Time | 2 |
| 416 | `dst.wt2.y1.release` | DST WT2 Y1 Release | Float | ms | 5.0 | 1000.0 | 100.0 | oui | C | Time | 2 |
| 417 | `dst.wt2.y1.knee` | DST WT2 Y1 Knee | Float | dB | 0.0 | 24.0 | 6.0 | oui | C | Lin | 2 |
| 418 | `dst.wt2.y1.gate` | DST WT2 Y1 Gate | Float | dB | -90.0 | -20.0 | -90.0 | oui | C | Db | 2 |
| 419 | `dst.wt2.y1.transient` | DST WT2 Y1 Transient | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 420 | `dst.wt2.y1.makeup` | DST WT2 Y1 Makeup | Float | dB | 0.0 | 36.0 | 0.0 | oui | C | Db | 2 |
| 421 | `dst.wt2.s1.mode` | DST WT2 S1 Mode | Choice (Ring / Shift) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 422 | `dst.wt2.s1.freq` | DST WT2 S1 Frequency | Float | Hz | 0.1 | 8000.0 | 100.0 | oui | CA | Oct | 2 |
| 423 | `dst.wt2.s1.shift` | DST WT2 S1 Shift | Float | Hz | -5000.0 | 5000.0 | 0.0 | oui | CA | Lin | 2 |
| 424 | `dst.wt2.s1.carrier` | DST WT2 S1 Carrier | Choice (Sine / Triangle / Square) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 425 | `dst.wt2.s1.phase` | DST WT2 S1 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 426 | `dst.wt2.s1.stereo` | DST WT2 S1 Stereo Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 427 | `dst.wt2.n2.fb` | DST WT2 N2 Feedback | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 428 | `dst.wt2.n2.delay` | DST WT2 N2 Delay | Float | ms | 0.1 | 50.0 | 5.0 | oui | C | Time | 2 |
| 429 | `dst.wt2.n2.damp` | DST WT2 N2 Damping | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 430 | `dst.wt2.n2.bias` | DST WT2 N2 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 431 | `dst.wt2.n2.res` | DST WT2 N2 Resonance | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 432 | `dst.wt2.n2.selfosc` | DST WT2 N2 Self-Oscillation | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 433 | `dst.wt2.d2.size` | DST WT2 D2 Size | Float | ms | 10.0 | 2000.0 | 250.0 | oui | C | Time | 2 |
| 434 | `dst.wt2.d2.speed` | DST WT2 D2 Playback | Float | x | -4.0 | 4.0 | 1.0 | oui | C | Lin | 2 |
| 435 | `dst.wt2.d2.repeat` | DST WT2 D2 Repeat | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 436 | `dst.wt2.d2.pos` | DST WT2 D2 Position | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 437 | `dst.wt2.d2.fb` | DST WT2 D2 Feedback | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 438 | `dst.wt2.d2.xfade` | DST WT2 D2 Crossfade | Float | ms | 0.0 | 50.0 | 5.0 | oui | C | Lin | 2 |
| 439 | `dst.wt2.d2.freeze` | DST WT2 D2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 440 | `dst.wt2.y2.attack` | DST WT2 Y2 Attack | Float | ms | 0.1 | 200.0 | 5.0 | oui | C | Time | 2 |
| 441 | `dst.wt2.y2.release` | DST WT2 Y2 Release | Float | ms | 5.0 | 2000.0 | 200.0 | oui | C | Time | 2 |
| 442 | `dst.wt2.y2.sens` | DST WT2 Y2 Sensitivity | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 443 | `dst.wt2.y2.curve` | DST WT2 Y2 Curve | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 444 | `dst.wt2.y2.depth` | DST WT2 Y2 Depth | Float | bip | -1.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 445 | `dst.wt2.y2.hold` | DST WT2 Y2 Hold | Float | ms | 0.0 | 500.0 | 0.0 | oui | C | Lin | 2 |
| 446 | `dst.wt2.y2.detect` | DST WT2 Y2 Detector | Choice (Peak / RMS) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 447 | `dst.wt2.s2.fft` | DST WT2 S2 FFT Size | Choice (512 / 1024 / 2048 / 4096) | — | 0 | 3 | 2 | non | E | Disc | 2 |
| 448 | `dst.wt2.s2.freeze` | DST WT2 S2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 449 | `dst.wt2.s2.decay` | DST WT2 S2 Decay | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 450 | `dst.wt2.s2.blur` | DST WT2 S2 Blur | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 451 | `dst.wt2.s2.tilt` | DST WT2 S2 Tilt | Float | dB/oct | -6.0 | 6.0 | 0.0 | oui | C | Lin | 2 |
| 452 | `dst.wt2.s2.drift` | DST WT2 S2 Phase Drift | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 453 | `dst.wt2.s2.shift` | DST WT2 S2 Shift | Float | st | -24.0 | 24.0 | 0.0 | oui | C | Lin | 2 |
| 454 | `dst.fm1.type` | DST FM1 Type | Choice (Off / N1 Asymmetric Wavefolder / D1 Variable Rate Crusher / Y1 Transient Destroyer / S1 Ring/Frequency Shifter / N2 Feedback Waveshaper / D2 Glitch Buffer / Y2 Envelope Shaper / S2 Spectral Freeze) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 455 | `dst.fm1.mix` | DST FM1 Dry/Wet | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 456 | `dst.fm1.out` | DST FM1 Output | Float | dB | -24.0 | 24.0 | 0.0 | oui | C | Db | 2 |
| 457 | `dst.fm1.arm` | DST FM1 Autonomous Armed | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 458 | `dst.fm1.n1.drive` | DST FM1 N1 Drive | Float | dB | -24.0 | 36.0 | 0.0 | oui | CA | Db | 2 |
| 459 | `dst.fm1.n1.fold` | DST FM1 N1 Fold | Float | lin | 0.0 | 1.0 | 0.5 | oui | CA | Lin | 2 |
| 460 | `dst.fm1.n1.bias` | DST FM1 N1 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 461 | `dst.fm1.n1.sym` | DST FM1 N1 Symmetry | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 462 | `dst.fm1.n1.stages` | DST FM1 N1 Stages | Int | — | 1 | 4 | 1 | non | E | Disc | 2 |
| 463 | `dst.fm1.n1.tone` | DST FM1 N1 Tone | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 464 | `dst.fm1.d1.bits` | DST FM1 D1 Bits | Float | bit | 2.0 | 16.0 | 8.0 | oui | C | Lin | 2 |
| 465 | `dst.fm1.d1.rate` | DST FM1 D1 Rate Ratio | Float | x | 1.0 | 128.0 | 4.0 | oui | C | Oct | 2 |
| 466 | `dst.fm1.d1.jitter` | DST FM1 D1 Jitter | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 467 | `dst.fm1.d1.mode` | DST FM1 D1 Mode | Choice (Raw / Controlled) | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 468 | `dst.fm1.d1.hold` | DST FM1 D1 Hold | Choice (Step / Smooth) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 469 | `dst.fm1.y1.thresh` | DST FM1 Y1 Threshold | Float | dB | -60.0 | 0.0 | -20.0 | oui | C | Db | 2 |
| 470 | `dst.fm1.y1.ratio` | DST FM1 Y1 Ratio | Float | :1 | 1.0 | 20.0 | 4.0 | oui | C | Lin | 2 |
| 471 | `dst.fm1.y1.attack` | DST FM1 Y1 Attack | Float | ms | 0.1 | 100.0 | 5.0 | oui | C | Time | 2 |
| 472 | `dst.fm1.y1.release` | DST FM1 Y1 Release | Float | ms | 5.0 | 1000.0 | 100.0 | oui | C | Time | 2 |
| 473 | `dst.fm1.y1.knee` | DST FM1 Y1 Knee | Float | dB | 0.0 | 24.0 | 6.0 | oui | C | Lin | 2 |
| 474 | `dst.fm1.y1.gate` | DST FM1 Y1 Gate | Float | dB | -90.0 | -20.0 | -90.0 | oui | C | Db | 2 |
| 475 | `dst.fm1.y1.transient` | DST FM1 Y1 Transient | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 476 | `dst.fm1.y1.makeup` | DST FM1 Y1 Makeup | Float | dB | 0.0 | 36.0 | 0.0 | oui | C | Db | 2 |
| 477 | `dst.fm1.s1.mode` | DST FM1 S1 Mode | Choice (Ring / Shift) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 478 | `dst.fm1.s1.freq` | DST FM1 S1 Frequency | Float | Hz | 0.1 | 8000.0 | 100.0 | oui | CA | Oct | 2 |
| 479 | `dst.fm1.s1.shift` | DST FM1 S1 Shift | Float | Hz | -5000.0 | 5000.0 | 0.0 | oui | CA | Lin | 2 |
| 480 | `dst.fm1.s1.carrier` | DST FM1 S1 Carrier | Choice (Sine / Triangle / Square) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 481 | `dst.fm1.s1.phase` | DST FM1 S1 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 482 | `dst.fm1.s1.stereo` | DST FM1 S1 Stereo Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 483 | `dst.fm1.n2.fb` | DST FM1 N2 Feedback | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 484 | `dst.fm1.n2.delay` | DST FM1 N2 Delay | Float | ms | 0.1 | 50.0 | 5.0 | oui | C | Time | 2 |
| 485 | `dst.fm1.n2.damp` | DST FM1 N2 Damping | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 486 | `dst.fm1.n2.bias` | DST FM1 N2 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 487 | `dst.fm1.n2.res` | DST FM1 N2 Resonance | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 488 | `dst.fm1.n2.selfosc` | DST FM1 N2 Self-Oscillation | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 489 | `dst.fm1.d2.size` | DST FM1 D2 Size | Float | ms | 10.0 | 2000.0 | 250.0 | oui | C | Time | 2 |
| 490 | `dst.fm1.d2.speed` | DST FM1 D2 Playback | Float | x | -4.0 | 4.0 | 1.0 | oui | C | Lin | 2 |
| 491 | `dst.fm1.d2.repeat` | DST FM1 D2 Repeat | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 492 | `dst.fm1.d2.pos` | DST FM1 D2 Position | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 493 | `dst.fm1.d2.fb` | DST FM1 D2 Feedback | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 494 | `dst.fm1.d2.xfade` | DST FM1 D2 Crossfade | Float | ms | 0.0 | 50.0 | 5.0 | oui | C | Lin | 2 |
| 495 | `dst.fm1.d2.freeze` | DST FM1 D2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 496 | `dst.fm1.y2.attack` | DST FM1 Y2 Attack | Float | ms | 0.1 | 200.0 | 5.0 | oui | C | Time | 2 |
| 497 | `dst.fm1.y2.release` | DST FM1 Y2 Release | Float | ms | 5.0 | 2000.0 | 200.0 | oui | C | Time | 2 |
| 498 | `dst.fm1.y2.sens` | DST FM1 Y2 Sensitivity | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 499 | `dst.fm1.y2.curve` | DST FM1 Y2 Curve | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 500 | `dst.fm1.y2.depth` | DST FM1 Y2 Depth | Float | bip | -1.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 501 | `dst.fm1.y2.hold` | DST FM1 Y2 Hold | Float | ms | 0.0 | 500.0 | 0.0 | oui | C | Lin | 2 |
| 502 | `dst.fm1.y2.detect` | DST FM1 Y2 Detector | Choice (Peak / RMS) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 503 | `dst.fm1.s2.fft` | DST FM1 S2 FFT Size | Choice (512 / 1024 / 2048 / 4096) | — | 0 | 3 | 2 | non | E | Disc | 2 |
| 504 | `dst.fm1.s2.freeze` | DST FM1 S2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 505 | `dst.fm1.s2.decay` | DST FM1 S2 Decay | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 506 | `dst.fm1.s2.blur` | DST FM1 S2 Blur | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 507 | `dst.fm1.s2.tilt` | DST FM1 S2 Tilt | Float | dB/oct | -6.0 | 6.0 | 0.0 | oui | C | Lin | 2 |
| 508 | `dst.fm1.s2.drift` | DST FM1 S2 Phase Drift | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 509 | `dst.fm1.s2.shift` | DST FM1 S2 Shift | Float | st | -24.0 | 24.0 | 0.0 | oui | C | Lin | 2 |
| 510 | `dst.fm2.type` | DST FM2 Type | Choice (Off / N1 Asymmetric Wavefolder / D1 Variable Rate Crusher / Y1 Transient Destroyer / S1 Ring/Frequency Shifter / N2 Feedback Waveshaper / D2 Glitch Buffer / Y2 Envelope Shaper / S2 Spectral Freeze) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 511 | `dst.fm2.mix` | DST FM2 Dry/Wet | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 512 | `dst.fm2.out` | DST FM2 Output | Float | dB | -24.0 | 24.0 | 0.0 | oui | C | Db | 2 |
| 513 | `dst.fm2.arm` | DST FM2 Autonomous Armed | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 514 | `dst.fm2.n1.drive` | DST FM2 N1 Drive | Float | dB | -24.0 | 36.0 | 0.0 | oui | CA | Db | 2 |
| 515 | `dst.fm2.n1.fold` | DST FM2 N1 Fold | Float | lin | 0.0 | 1.0 | 0.5 | oui | CA | Lin | 2 |
| 516 | `dst.fm2.n1.bias` | DST FM2 N1 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 517 | `dst.fm2.n1.sym` | DST FM2 N1 Symmetry | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 518 | `dst.fm2.n1.stages` | DST FM2 N1 Stages | Int | — | 1 | 4 | 1 | non | E | Disc | 2 |
| 519 | `dst.fm2.n1.tone` | DST FM2 N1 Tone | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 520 | `dst.fm2.d1.bits` | DST FM2 D1 Bits | Float | bit | 2.0 | 16.0 | 8.0 | oui | C | Lin | 2 |
| 521 | `dst.fm2.d1.rate` | DST FM2 D1 Rate Ratio | Float | x | 1.0 | 128.0 | 4.0 | oui | C | Oct | 2 |
| 522 | `dst.fm2.d1.jitter` | DST FM2 D1 Jitter | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 523 | `dst.fm2.d1.mode` | DST FM2 D1 Mode | Choice (Raw / Controlled) | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 524 | `dst.fm2.d1.hold` | DST FM2 D1 Hold | Choice (Step / Smooth) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 525 | `dst.fm2.y1.thresh` | DST FM2 Y1 Threshold | Float | dB | -60.0 | 0.0 | -20.0 | oui | C | Db | 2 |
| 526 | `dst.fm2.y1.ratio` | DST FM2 Y1 Ratio | Float | :1 | 1.0 | 20.0 | 4.0 | oui | C | Lin | 2 |
| 527 | `dst.fm2.y1.attack` | DST FM2 Y1 Attack | Float | ms | 0.1 | 100.0 | 5.0 | oui | C | Time | 2 |
| 528 | `dst.fm2.y1.release` | DST FM2 Y1 Release | Float | ms | 5.0 | 1000.0 | 100.0 | oui | C | Time | 2 |
| 529 | `dst.fm2.y1.knee` | DST FM2 Y1 Knee | Float | dB | 0.0 | 24.0 | 6.0 | oui | C | Lin | 2 |
| 530 | `dst.fm2.y1.gate` | DST FM2 Y1 Gate | Float | dB | -90.0 | -20.0 | -90.0 | oui | C | Db | 2 |
| 531 | `dst.fm2.y1.transient` | DST FM2 Y1 Transient | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 532 | `dst.fm2.y1.makeup` | DST FM2 Y1 Makeup | Float | dB | 0.0 | 36.0 | 0.0 | oui | C | Db | 2 |
| 533 | `dst.fm2.s1.mode` | DST FM2 S1 Mode | Choice (Ring / Shift) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 534 | `dst.fm2.s1.freq` | DST FM2 S1 Frequency | Float | Hz | 0.1 | 8000.0 | 100.0 | oui | CA | Oct | 2 |
| 535 | `dst.fm2.s1.shift` | DST FM2 S1 Shift | Float | Hz | -5000.0 | 5000.0 | 0.0 | oui | CA | Lin | 2 |
| 536 | `dst.fm2.s1.carrier` | DST FM2 S1 Carrier | Choice (Sine / Triangle / Square) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 537 | `dst.fm2.s1.phase` | DST FM2 S1 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 538 | `dst.fm2.s1.stereo` | DST FM2 S1 Stereo Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 539 | `dst.fm2.n2.fb` | DST FM2 N2 Feedback | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 540 | `dst.fm2.n2.delay` | DST FM2 N2 Delay | Float | ms | 0.1 | 50.0 | 5.0 | oui | C | Time | 2 |
| 541 | `dst.fm2.n2.damp` | DST FM2 N2 Damping | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 542 | `dst.fm2.n2.bias` | DST FM2 N2 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 543 | `dst.fm2.n2.res` | DST FM2 N2 Resonance | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 544 | `dst.fm2.n2.selfosc` | DST FM2 N2 Self-Oscillation | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 545 | `dst.fm2.d2.size` | DST FM2 D2 Size | Float | ms | 10.0 | 2000.0 | 250.0 | oui | C | Time | 2 |
| 546 | `dst.fm2.d2.speed` | DST FM2 D2 Playback | Float | x | -4.0 | 4.0 | 1.0 | oui | C | Lin | 2 |
| 547 | `dst.fm2.d2.repeat` | DST FM2 D2 Repeat | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 548 | `dst.fm2.d2.pos` | DST FM2 D2 Position | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 549 | `dst.fm2.d2.fb` | DST FM2 D2 Feedback | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 550 | `dst.fm2.d2.xfade` | DST FM2 D2 Crossfade | Float | ms | 0.0 | 50.0 | 5.0 | oui | C | Lin | 2 |
| 551 | `dst.fm2.d2.freeze` | DST FM2 D2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 552 | `dst.fm2.y2.attack` | DST FM2 Y2 Attack | Float | ms | 0.1 | 200.0 | 5.0 | oui | C | Time | 2 |
| 553 | `dst.fm2.y2.release` | DST FM2 Y2 Release | Float | ms | 5.0 | 2000.0 | 200.0 | oui | C | Time | 2 |
| 554 | `dst.fm2.y2.sens` | DST FM2 Y2 Sensitivity | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 555 | `dst.fm2.y2.curve` | DST FM2 Y2 Curve | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 556 | `dst.fm2.y2.depth` | DST FM2 Y2 Depth | Float | bip | -1.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 557 | `dst.fm2.y2.hold` | DST FM2 Y2 Hold | Float | ms | 0.0 | 500.0 | 0.0 | oui | C | Lin | 2 |
| 558 | `dst.fm2.y2.detect` | DST FM2 Y2 Detector | Choice (Peak / RMS) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 559 | `dst.fm2.s2.fft` | DST FM2 S2 FFT Size | Choice (512 / 1024 / 2048 / 4096) | — | 0 | 3 | 2 | non | E | Disc | 2 |
| 560 | `dst.fm2.s2.freeze` | DST FM2 S2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 561 | `dst.fm2.s2.decay` | DST FM2 S2 Decay | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 562 | `dst.fm2.s2.blur` | DST FM2 S2 Blur | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 563 | `dst.fm2.s2.tilt` | DST FM2 S2 Tilt | Float | dB/oct | -6.0 | 6.0 | 0.0 | oui | C | Lin | 2 |
| 564 | `dst.fm2.s2.drift` | DST FM2 S2 Phase Drift | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 565 | `dst.fm2.s2.shift` | DST FM2 S2 Shift | Float | st | -24.0 | 24.0 | 0.0 | oui | C | Lin | 2 |
| 566 | `dst.gl1.type` | DST GL1 Type | Choice (Off / N1 Asymmetric Wavefolder / D1 Variable Rate Crusher / Y1 Transient Destroyer / S1 Ring/Frequency Shifter / N2 Feedback Waveshaper / D2 Glitch Buffer / Y2 Envelope Shaper / S2 Spectral Freeze) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 567 | `dst.gl1.mix` | DST GL1 Dry/Wet | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 568 | `dst.gl1.out` | DST GL1 Output | Float | dB | -24.0 | 24.0 | 0.0 | oui | C | Db | 2 |
| 569 | `dst.gl1.arm` | DST GL1 Autonomous Armed | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 570 | `dst.gl1.n1.drive` | DST GL1 N1 Drive | Float | dB | -24.0 | 36.0 | 0.0 | oui | CA | Db | 2 |
| 571 | `dst.gl1.n1.fold` | DST GL1 N1 Fold | Float | lin | 0.0 | 1.0 | 0.5 | oui | CA | Lin | 2 |
| 572 | `dst.gl1.n1.bias` | DST GL1 N1 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 573 | `dst.gl1.n1.sym` | DST GL1 N1 Symmetry | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 574 | `dst.gl1.n1.stages` | DST GL1 N1 Stages | Int | — | 1 | 4 | 1 | non | E | Disc | 2 |
| 575 | `dst.gl1.n1.tone` | DST GL1 N1 Tone | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 576 | `dst.gl1.d1.bits` | DST GL1 D1 Bits | Float | bit | 2.0 | 16.0 | 8.0 | oui | C | Lin | 2 |
| 577 | `dst.gl1.d1.rate` | DST GL1 D1 Rate Ratio | Float | x | 1.0 | 128.0 | 4.0 | oui | C | Oct | 2 |
| 578 | `dst.gl1.d1.jitter` | DST GL1 D1 Jitter | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 579 | `dst.gl1.d1.mode` | DST GL1 D1 Mode | Choice (Raw / Controlled) | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 580 | `dst.gl1.d1.hold` | DST GL1 D1 Hold | Choice (Step / Smooth) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 581 | `dst.gl1.y1.thresh` | DST GL1 Y1 Threshold | Float | dB | -60.0 | 0.0 | -20.0 | oui | C | Db | 2 |
| 582 | `dst.gl1.y1.ratio` | DST GL1 Y1 Ratio | Float | :1 | 1.0 | 20.0 | 4.0 | oui | C | Lin | 2 |
| 583 | `dst.gl1.y1.attack` | DST GL1 Y1 Attack | Float | ms | 0.1 | 100.0 | 5.0 | oui | C | Time | 2 |
| 584 | `dst.gl1.y1.release` | DST GL1 Y1 Release | Float | ms | 5.0 | 1000.0 | 100.0 | oui | C | Time | 2 |
| 585 | `dst.gl1.y1.knee` | DST GL1 Y1 Knee | Float | dB | 0.0 | 24.0 | 6.0 | oui | C | Lin | 2 |
| 586 | `dst.gl1.y1.gate` | DST GL1 Y1 Gate | Float | dB | -90.0 | -20.0 | -90.0 | oui | C | Db | 2 |
| 587 | `dst.gl1.y1.transient` | DST GL1 Y1 Transient | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 588 | `dst.gl1.y1.makeup` | DST GL1 Y1 Makeup | Float | dB | 0.0 | 36.0 | 0.0 | oui | C | Db | 2 |
| 589 | `dst.gl1.s1.mode` | DST GL1 S1 Mode | Choice (Ring / Shift) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 590 | `dst.gl1.s1.freq` | DST GL1 S1 Frequency | Float | Hz | 0.1 | 8000.0 | 100.0 | oui | CA | Oct | 2 |
| 591 | `dst.gl1.s1.shift` | DST GL1 S1 Shift | Float | Hz | -5000.0 | 5000.0 | 0.0 | oui | CA | Lin | 2 |
| 592 | `dst.gl1.s1.carrier` | DST GL1 S1 Carrier | Choice (Sine / Triangle / Square) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 593 | `dst.gl1.s1.phase` | DST GL1 S1 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 594 | `dst.gl1.s1.stereo` | DST GL1 S1 Stereo Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 595 | `dst.gl1.n2.fb` | DST GL1 N2 Feedback | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 596 | `dst.gl1.n2.delay` | DST GL1 N2 Delay | Float | ms | 0.1 | 50.0 | 5.0 | oui | C | Time | 2 |
| 597 | `dst.gl1.n2.damp` | DST GL1 N2 Damping | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 598 | `dst.gl1.n2.bias` | DST GL1 N2 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 599 | `dst.gl1.n2.res` | DST GL1 N2 Resonance | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 600 | `dst.gl1.n2.selfosc` | DST GL1 N2 Self-Oscillation | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 601 | `dst.gl1.d2.size` | DST GL1 D2 Size | Float | ms | 10.0 | 2000.0 | 250.0 | oui | C | Time | 2 |
| 602 | `dst.gl1.d2.speed` | DST GL1 D2 Playback | Float | x | -4.0 | 4.0 | 1.0 | oui | C | Lin | 2 |
| 603 | `dst.gl1.d2.repeat` | DST GL1 D2 Repeat | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 604 | `dst.gl1.d2.pos` | DST GL1 D2 Position | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 605 | `dst.gl1.d2.fb` | DST GL1 D2 Feedback | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 606 | `dst.gl1.d2.xfade` | DST GL1 D2 Crossfade | Float | ms | 0.0 | 50.0 | 5.0 | oui | C | Lin | 2 |
| 607 | `dst.gl1.d2.freeze` | DST GL1 D2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 608 | `dst.gl1.y2.attack` | DST GL1 Y2 Attack | Float | ms | 0.1 | 200.0 | 5.0 | oui | C | Time | 2 |
| 609 | `dst.gl1.y2.release` | DST GL1 Y2 Release | Float | ms | 5.0 | 2000.0 | 200.0 | oui | C | Time | 2 |
| 610 | `dst.gl1.y2.sens` | DST GL1 Y2 Sensitivity | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 611 | `dst.gl1.y2.curve` | DST GL1 Y2 Curve | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 612 | `dst.gl1.y2.depth` | DST GL1 Y2 Depth | Float | bip | -1.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 613 | `dst.gl1.y2.hold` | DST GL1 Y2 Hold | Float | ms | 0.0 | 500.0 | 0.0 | oui | C | Lin | 2 |
| 614 | `dst.gl1.y2.detect` | DST GL1 Y2 Detector | Choice (Peak / RMS) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 615 | `dst.gl1.s2.fft` | DST GL1 S2 FFT Size | Choice (512 / 1024 / 2048 / 4096) | — | 0 | 3 | 2 | non | E | Disc | 2 |
| 616 | `dst.gl1.s2.freeze` | DST GL1 S2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 617 | `dst.gl1.s2.decay` | DST GL1 S2 Decay | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 618 | `dst.gl1.s2.blur` | DST GL1 S2 Blur | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 619 | `dst.gl1.s2.tilt` | DST GL1 S2 Tilt | Float | dB/oct | -6.0 | 6.0 | 0.0 | oui | C | Lin | 2 |
| 620 | `dst.gl1.s2.drift` | DST GL1 S2 Phase Drift | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 621 | `dst.gl1.s2.shift` | DST GL1 S2 Shift | Float | st | -24.0 | 24.0 | 0.0 | oui | C | Lin | 2 |
| 622 | `dst.gl2.type` | DST GL2 Type | Choice (Off / N1 Asymmetric Wavefolder / D1 Variable Rate Crusher / Y1 Transient Destroyer / S1 Ring/Frequency Shifter / N2 Feedback Waveshaper / D2 Glitch Buffer / Y2 Envelope Shaper / S2 Spectral Freeze) | — | 0 | 8 | 0 | non | E | Disc | 2 |
| 623 | `dst.gl2.mix` | DST GL2 Dry/Wet | Float | lin | 0.0 | 1.0 | 1.0 | oui | CA | Lin | 2 |
| 624 | `dst.gl2.out` | DST GL2 Output | Float | dB | -24.0 | 24.0 | 0.0 | oui | C | Db | 2 |
| 625 | `dst.gl2.arm` | DST GL2 Autonomous Armed | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 626 | `dst.gl2.n1.drive` | DST GL2 N1 Drive | Float | dB | -24.0 | 36.0 | 0.0 | oui | CA | Db | 2 |
| 627 | `dst.gl2.n1.fold` | DST GL2 N1 Fold | Float | lin | 0.0 | 1.0 | 0.5 | oui | CA | Lin | 2 |
| 628 | `dst.gl2.n1.bias` | DST GL2 N1 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 629 | `dst.gl2.n1.sym` | DST GL2 N1 Symmetry | Float | bip | -1.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 630 | `dst.gl2.n1.stages` | DST GL2 N1 Stages | Int | — | 1 | 4 | 1 | non | E | Disc | 2 |
| 631 | `dst.gl2.n1.tone` | DST GL2 N1 Tone | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 632 | `dst.gl2.d1.bits` | DST GL2 D1 Bits | Float | bit | 2.0 | 16.0 | 8.0 | oui | C | Lin | 2 |
| 633 | `dst.gl2.d1.rate` | DST GL2 D1 Rate Ratio | Float | x | 1.0 | 128.0 | 4.0 | oui | C | Oct | 2 |
| 634 | `dst.gl2.d1.jitter` | DST GL2 D1 Jitter | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 635 | `dst.gl2.d1.mode` | DST GL2 D1 Mode | Choice (Raw / Controlled) | — | 0 | 1 | 1 | non | E | Disc | 2 |
| 636 | `dst.gl2.d1.hold` | DST GL2 D1 Hold | Choice (Step / Smooth) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 637 | `dst.gl2.y1.thresh` | DST GL2 Y1 Threshold | Float | dB | -60.0 | 0.0 | -20.0 | oui | C | Db | 2 |
| 638 | `dst.gl2.y1.ratio` | DST GL2 Y1 Ratio | Float | :1 | 1.0 | 20.0 | 4.0 | oui | C | Lin | 2 |
| 639 | `dst.gl2.y1.attack` | DST GL2 Y1 Attack | Float | ms | 0.1 | 100.0 | 5.0 | oui | C | Time | 2 |
| 640 | `dst.gl2.y1.release` | DST GL2 Y1 Release | Float | ms | 5.0 | 1000.0 | 100.0 | oui | C | Time | 2 |
| 641 | `dst.gl2.y1.knee` | DST GL2 Y1 Knee | Float | dB | 0.0 | 24.0 | 6.0 | oui | C | Lin | 2 |
| 642 | `dst.gl2.y1.gate` | DST GL2 Y1 Gate | Float | dB | -90.0 | -20.0 | -90.0 | oui | C | Db | 2 |
| 643 | `dst.gl2.y1.transient` | DST GL2 Y1 Transient | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 644 | `dst.gl2.y1.makeup` | DST GL2 Y1 Makeup | Float | dB | 0.0 | 36.0 | 0.0 | oui | C | Db | 2 |
| 645 | `dst.gl2.s1.mode` | DST GL2 S1 Mode | Choice (Ring / Shift) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 646 | `dst.gl2.s1.freq` | DST GL2 S1 Frequency | Float | Hz | 0.1 | 8000.0 | 100.0 | oui | CA | Oct | 2 |
| 647 | `dst.gl2.s1.shift` | DST GL2 S1 Shift | Float | Hz | -5000.0 | 5000.0 | 0.0 | oui | CA | Lin | 2 |
| 648 | `dst.gl2.s1.carrier` | DST GL2 S1 Carrier | Choice (Sine / Triangle / Square) | — | 0 | 2 | 0 | non | E | Disc | 2 |
| 649 | `dst.gl2.s1.phase` | DST GL2 S1 Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 650 | `dst.gl2.s1.stereo` | DST GL2 S1 Stereo Phase | Float | cyc | 0.0 | 1.0 | 0.0 | oui | C | Cyc | 2 |
| 651 | `dst.gl2.n2.fb` | DST GL2 N2 Feedback | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 652 | `dst.gl2.n2.delay` | DST GL2 N2 Delay | Float | ms | 0.1 | 50.0 | 5.0 | oui | C | Time | 2 |
| 653 | `dst.gl2.n2.damp` | DST GL2 N2 Damping | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 654 | `dst.gl2.n2.bias` | DST GL2 N2 Bias | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 655 | `dst.gl2.n2.res` | DST GL2 N2 Resonance | Float | lin | 0.0 | 1.0 | 0.3 | oui | C | Lin | 2 |
| 656 | `dst.gl2.n2.selfosc` | DST GL2 N2 Self-Oscillation | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 657 | `dst.gl2.d2.size` | DST GL2 D2 Size | Float | ms | 10.0 | 2000.0 | 250.0 | oui | C | Time | 2 |
| 658 | `dst.gl2.d2.speed` | DST GL2 D2 Playback | Float | x | -4.0 | 4.0 | 1.0 | oui | C | Lin | 2 |
| 659 | `dst.gl2.d2.repeat` | DST GL2 D2 Repeat | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 660 | `dst.gl2.d2.pos` | DST GL2 D2 Position | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 661 | `dst.gl2.d2.fb` | DST GL2 D2 Feedback | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 662 | `dst.gl2.d2.xfade` | DST GL2 D2 Crossfade | Float | ms | 0.0 | 50.0 | 5.0 | oui | C | Lin | 2 |
| 663 | `dst.gl2.d2.freeze` | DST GL2 D2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 664 | `dst.gl2.y2.attack` | DST GL2 Y2 Attack | Float | ms | 0.1 | 200.0 | 5.0 | oui | C | Time | 2 |
| 665 | `dst.gl2.y2.release` | DST GL2 Y2 Release | Float | ms | 5.0 | 2000.0 | 200.0 | oui | C | Time | 2 |
| 666 | `dst.gl2.y2.sens` | DST GL2 Y2 Sensitivity | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 667 | `dst.gl2.y2.curve` | DST GL2 Y2 Curve | Float | bip | -1.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 668 | `dst.gl2.y2.depth` | DST GL2 Y2 Depth | Float | bip | -1.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 669 | `dst.gl2.y2.hold` | DST GL2 Y2 Hold | Float | ms | 0.0 | 500.0 | 0.0 | oui | C | Lin | 2 |
| 670 | `dst.gl2.y2.detect` | DST GL2 Y2 Detector | Choice (Peak / RMS) | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 671 | `dst.gl2.s2.fft` | DST GL2 S2 FFT Size | Choice (512 / 1024 / 2048 / 4096) | — | 0 | 3 | 2 | non | E | Disc | 2 |
| 672 | `dst.gl2.s2.freeze` | DST GL2 S2 Freeze | Bool | — | 0 | 1 | 0 | oui | E | Disc | 2 |
| 673 | `dst.gl2.s2.decay` | DST GL2 S2 Decay | Float | lin | 0.0 | 1.0 | 0.5 | oui | C | Lin | 2 |
| 674 | `dst.gl2.s2.blur` | DST GL2 S2 Blur | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 675 | `dst.gl2.s2.tilt` | DST GL2 S2 Tilt | Float | dB/oct | -6.0 | 6.0 | 0.0 | oui | C | Lin | 2 |
| 676 | `dst.gl2.s2.drift` | DST GL2 S2 Phase Drift | Float | lin | 0.0 | 1.0 | 0.0 | oui | C | Lin | 2 |
| 677 | `dst.gl2.s2.shift` | DST GL2 S2 Shift | Float | st | -24.0 | 24.0 | 0.0 | oui | C | Lin | 2 |
| 678 | `macro1` | Macro 1 | Float | lin | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 679 | `macro2` | Macro 2 | Float | lin | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 680 | `macro3` | Macro 3 | Float | lin | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 681 | `macro4` | Macro 4 | Float | lin | 0.0 | 1.0 | 0.0 | oui | CA | Lin | 2 |
| 682 | `out.limiter` | Musical Limiter | Bool | — | 0 | 1 | 0 | non | E | Disc | 2 |
| 683 | `out.ceiling` | Limiter Ceiling | Float | dB | -24.0 | 0.0 | -0.3 | oui | C | Db | 2 |
