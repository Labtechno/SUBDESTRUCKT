# ARCH-UI-04 — Cartographie StableID (preuve d'implémentation)

Source : `dafa6be+ (0.4.0)`, registre compilé (`source/core/ParamTable.inc`). Générée par `tools/ui04/make_stableid_map.py` depuis l'artefact du test **UI04-COV** (`subdestruckt_wrapper_tests`, 3e argument) : chaque ligne indique le premier chemin de navigation où une commande **liée et visible** du StableID a été trouvée dans l'éditeur réel, avec les sélections d'algorithme ou de mode nécessaires (« ; »). Le test UI04-BIND édite ensuite chaque StableID par cette commande et vérifie la valeur Base hôte. Remplace la cartographie candidate ARCH-UI-03 (695 IDs, affectations documentaires).

Total : **703** StableID ; atteints : **703** ; non atteints : **0** ; accès contextuel (algorithme/mode requis) : 440.

| Espace d'accès principal | StableID |
|---|---|
| ENGINEERING | 18 |
| PERFORMANCE | 13 |
| SOUND DESIGN | 672 |

`états` = nombre d'états de navigation explorés où une commande du StableID est visible (alias et contextes compatibles compris).

| # | StableID | Nom registre | Type | Accès vérifié (contexte ; sélections) | états |
|---|---|---|---|---|---|
| 0 | `amp.attack` | Amp Attack | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 1 | `amp.decay` | Amp Decay | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 2 | `amp.sustain` | Amp Sustain | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 3 | `amp.release` | Amp Release | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 4 | `amp.velocity` | Amp Velocity | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 5 | `out.gain` | Output Gain | Float | PERFORMANCE / PERFORMANCE | 1 |
| 6 | `wt.level` | WT Level | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 7 | `op1.level` | OP1 Level | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 8 | `op2.level` | OP2 Level | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 9 | `op1.ratio` | OP1 Ratio | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 20 |
| 10 | `op2.ratio` | OP2 Ratio | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 29 |
| 11 | `fm.op2_op1.depth` | FM OP2>OP1 | Float | PERFORMANCE / PERFORMANCE | 14 |
| 12 | `fm.op1_op2.depth` | FM OP1>OP2 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 13 | `fm.op1_op1.depth` | FM OP1 Self | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 14 | `fm.op2_op2.depth` | FM OP2 Self | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 15 | `cross.wt_op1.depth` | CROSS WT>OP1 | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 32 |
| 16 | `cross.wt_op2.depth` | CROSS WT>OP2 | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 32 |
| 17 | `cross.op1_wt.depth` | CROSS OP1>WT | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 18 | `cross.op2_wt.depth` | CROSS OP2>WT | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 19 | `cross.master` | CROSS Master | Float | PERFORMANCE / PERFORMANCE | 22 |
| 20 | `cross.fwd` | CROSS WT>FM | Float | PERFORMANCE / PERFORMANCE | 22 |
| 21 | `cross.rev` | CROSS FM>WT | Float | PERFORMANCE / PERFORMANCE | 22 |
| 22 | `voice.mode` | Voice Mode | Choice | ENGINEERING / VOICE / TUNING | 6 |
| 23 | `voice.priority` | Note Priority | Choice | ENGINEERING / VOICE / TUNING | 6 |
| 24 | `voice.bend_up` | Bend Up | Float | ENGINEERING / VOICE / TUNING | 6 |
| 25 | `voice.bend_down` | Bend Down | Float | ENGINEERING / VOICE / TUNING | 6 |
| 26 | `tune.a4` | Reference A4 | Float | ENGINEERING / VOICE / TUNING | 6 |
| 27 | `tune.scale` | Scale | Choice | ENGINEERING / VOICE / TUNING | 6 |
| 28 | `tune.pc0` | Tune C | Float | ENGINEERING / VOICE / TUNING | 6 |
| 29 | `tune.pc1` | Tune C# | Float | ENGINEERING / VOICE / TUNING | 6 |
| 30 | `tune.pc2` | Tune D | Float | ENGINEERING / VOICE / TUNING | 6 |
| 31 | `tune.pc3` | Tune D# | Float | ENGINEERING / VOICE / TUNING | 6 |
| 32 | `tune.pc4` | Tune E | Float | ENGINEERING / VOICE / TUNING | 6 |
| 33 | `tune.pc5` | Tune F | Float | ENGINEERING / VOICE / TUNING | 6 |
| 34 | `tune.pc6` | Tune F# | Float | ENGINEERING / VOICE / TUNING | 6 |
| 35 | `tune.pc7` | Tune G | Float | ENGINEERING / VOICE / TUNING | 6 |
| 36 | `tune.pc8` | Tune G# | Float | ENGINEERING / VOICE / TUNING | 6 |
| 37 | `tune.pc9` | Tune A | Float | ENGINEERING / VOICE / TUNING | 6 |
| 38 | `tune.pc10` | Tune A# | Float | ENGINEERING / VOICE / TUNING | 6 |
| 39 | `tune.pc11` | Tune B | Float | ENGINEERING / VOICE / TUNING | 6 |
| 40 | `amp.delay` | Amp Delay | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 41 | `amp.hold` | Amp Hold | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 42 | `amp.curve` | Amp Curve | Choice | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 43 | `env.wt.delay` | ENV WT Delay | Float | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 44 | `env.wt.attack` | ENV WT Attack | Float | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 45 | `env.wt.hold` | ENV WT Hold | Float | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 46 | `env.wt.decay` | ENV WT Decay | Float | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 47 | `env.wt.sustain` | ENV WT Sustain | Float | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 48 | `env.wt.release` | ENV WT Release | Float | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 49 | `env.wt.curve` | ENV WT Curve | Choice | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 50 | `env.wt.loop` | ENV WT Loop | Choice | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 51 | `env.fm.delay` | ENV FM Delay | Float | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 52 | `env.fm.attack` | ENV FM Attack | Float | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 53 | `env.fm.hold` | ENV FM Hold | Float | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 54 | `env.fm.decay` | ENV FM Decay | Float | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 55 | `env.fm.sustain` | ENV FM Sustain | Float | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 56 | `env.fm.release` | ENV FM Release | Float | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 57 | `env.fm.curve` | ENV FM Curve | Choice | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 58 | `env.fm.loop` | ENV FM Loop | Choice | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 59 | `env.aux1.delay` | ENV AUX1 Delay | Float | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 29 |
| 60 | `env.aux1.attack` | ENV AUX1 Attack | Float | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 29 |
| 61 | `env.aux1.hold` | ENV AUX1 Hold | Float | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 29 |
| 62 | `env.aux1.decay` | ENV AUX1 Decay | Float | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 29 |
| 63 | `env.aux1.sustain` | ENV AUX1 Sustain | Float | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 29 |
| 64 | `env.aux1.release` | ENV AUX1 Release | Float | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 29 |
| 65 | `env.aux1.curve` | ENV AUX1 Curve | Choice | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 29 |
| 66 | `env.aux1.loop` | ENV AUX1 Loop | Choice | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 29 |
| 67 | `env.aux2.delay` | ENV AUX2 Delay | Float | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 29 |
| 68 | `env.aux2.attack` | ENV AUX2 Attack | Float | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 29 |
| 69 | `env.aux2.hold` | ENV AUX2 Hold | Float | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 29 |
| 70 | `env.aux2.decay` | ENV AUX2 Decay | Float | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 29 |
| 71 | `env.aux2.sustain` | ENV AUX2 Sustain | Float | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 29 |
| 72 | `env.aux2.release` | ENV AUX2 Release | Float | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 29 |
| 73 | `env.aux2.curve` | ENV AUX2 Curve | Choice | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 29 |
| 74 | `env.aux2.loop` | ENV AUX2 Loop | Choice | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 29 |
| 75 | `lfo1.shape` | LFO1 Shape | Choice | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 76 | `lfo1.rate` | LFO1 Rate | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 13 |
| 77 | `lfo1.sync` | LFO1 Tempo Sync | Bool | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 78 | `lfo1.div` | LFO1 Division | Choice | SOUND DESIGN / MODULATION / ENV AMP / LFO1 ; lfo1.sync=On | 15 |
| 79 | `lfo1.phase` | LFO1 Phase | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 80 | `lfo1.retrig` | LFO1 Retrigger | Choice | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 81 | `lfo2.shape` | LFO2 Shape | Choice | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 82 | `lfo2.rate` | LFO2 Rate | Float | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 14 |
| 83 | `lfo2.sync` | LFO2 Tempo Sync | Bool | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 84 | `lfo2.div` | LFO2 Division | Choice | SOUND DESIGN / MODULATION / ENV WT / LFO2 ; lfo2.sync=On | 15 |
| 85 | `lfo2.phase` | LFO2 Phase | Float | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 86 | `lfo2.retrig` | LFO2 Retrigger | Choice | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 87 | `lfo3.shape` | LFO3 Shape | Choice | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 88 | `lfo3.rate` | LFO3 Rate | Float | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 14 |
| 89 | `lfo3.sync` | LFO3 Tempo Sync | Bool | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 90 | `lfo3.div` | LFO3 Division | Choice | SOUND DESIGN / MODULATION / ENV FM / LFO3 ; lfo3.sync=On | 15 |
| 91 | `lfo3.phase` | LFO3 Phase | Float | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 92 | `lfo3.retrig` | LFO3 Retrigger | Choice | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 93 | `lfo4.shape` | LFO4 Shape | Choice | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 58 |
| 94 | `lfo4.rate` | LFO4 Rate | Float | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 28 |
| 95 | `lfo4.sync` | LFO4 Tempo Sync | Bool | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 58 |
| 96 | `lfo4.div` | LFO4 Division | Choice | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 ; lfo4.sync=On | 30 |
| 97 | `lfo4.phase` | LFO4 Phase | Float | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 58 |
| 98 | `lfo4.retrig` | LFO4 Retrigger | Choice | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 58 |
| 99 | `rnd.hold.rate` | Random Hold Rate | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 144 |
| 100 | `rnd.smooth.rate` | Random Smooth Rate | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 144 |
| 101 | `chaos.rate` | Chaos Rate | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 144 |
| 102 | `fol1.attack` | Follower1 Attack | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 144 |
| 103 | `fol1.release` | Follower1 Release | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 144 |
| 104 | `fol2.attack` | Follower2 Attack | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 144 |
| 105 | `fol2.release` | Follower2 Release | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 144 |
| 106 | `fol3.attack` | Follower3 Attack | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 144 |
| 107 | `fol3.release` | Follower3 Release | Float | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 144 |
| 108 | `wt.x` | WT X | Float | PERFORMANCE / PERFORMANCE | 22 |
| 109 | `wt.y` | WT Y | Float | PERFORMANCE / PERFORMANCE | 22 |
| 110 | `wt.z` | WT Z Master | Float | PERFORMANCE / PERFORMANCE | 22 |
| 111 | `wt.x_bound` | WT X Boundary | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 112 | `wt.y_bound` | WT Y Boundary | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 113 | `wt.x_morph` | WT X Morph | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 114 | `wt.y_morph` | WT Y Morph | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 115 | `wt.interp` | WT Interpolation | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 116 | `wt.render` | WT Clean/Raw | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 117 | `wt.coarse` | WT Coarse | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 118 | `wt.fine` | WT Fine | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 119 | `wt.phase_mode` | WT Phase Mode | Choice | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 120 | `wt.phase` | WT Start Phase | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 ; wt.phase_mode=Fixed | 1 |
| 121 | `wt.sync` | WT Sync | Choice | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 122 | `wt.sync_src` | WT Sync Source | Choice | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 ; wt.sync=Hard | 10 |
| 123 | `wt.char` | WT Character | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 23 |
| 124 | `wt.char_mode` | WT Character Profile | Choice | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 23 |
| 125 | `wt.za.type` | WT ZA Transform | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 126 | `wt.za.depth` | WT ZA Depth | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.za.type=Harmonic Fold | 32 |
| 127 | `wt.za.curve` | WT ZA Curve | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.za.type=Harmonic Fold | 32 |
| 128 | `wt.zb.type` | WT ZB Transform | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 129 | `wt.zb.depth` | WT ZB Depth | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.zb.type=Harmonic Fold | 32 |
| 130 | `wt.zb.curve` | WT ZB Curve | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.zb.type=Harmonic Fold | 32 |
| 131 | `wt.z_order` | WT Z Order | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 132 | `wt.z_fund` | WT Z Keep Fundamental | Bool | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 133 | `wt.z_gain` | WT Z Gain Compensation | Bool | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 134 | `op1.tune` | OP1 Tuning | Choice | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 135 | `op1.fixed` | OP1 Fixed | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 ; op1.tune=Fixed | 1 |
| 136 | `op1.offset` | OP1 Offset | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 137 | `op1.coarse` | OP1 Coarse | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 138 | `op1.fine` | OP1 Fine | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 139 | `op1.wave` | OP1 Waveform | Choice | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 140 | `op1.wt_x` | OP1 WT X | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 ; op1.wave=Wavetable | 1 |
| 141 | `op1.wt_y` | OP1 WT Y | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 ; op1.wave=Wavetable | 1 |
| 142 | `op1.pan` | OP1 Pan | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 143 | `op1.mute` | OP1 Mute | Bool | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 144 | `op1.enable` | OP1 Enable | Bool | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 42 |
| 145 | `op1.char` | OP1 Character | Float | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 146 | `op1.phase_mode` | OP1 Phase Mode | Choice | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 147 | `op1.ac` | OP1 Mod Coupling | Choice | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 148 | `op1.mode` | OP1 Mod Mode | Choice | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 149 | `op1.sync` | OP1 Sync | Choice | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 150 | `op2.tune` | OP2 Tuning | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 151 | `op2.fixed` | OP2 Fixed | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 ; op2.tune=Fixed | 1 |
| 152 | `op2.offset` | OP2 Offset | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 153 | `op2.coarse` | OP2 Coarse | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 154 | `op2.fine` | OP2 Fine | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 155 | `op2.wave` | OP2 Waveform | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 156 | `op2.wt_x` | OP2 WT X | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 ; op2.wave=Wavetable | 1 |
| 157 | `op2.wt_y` | OP2 WT Y | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 ; op2.wave=Wavetable | 1 |
| 158 | `op2.pan` | OP2 Pan | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 159 | `op2.mute` | OP2 Mute | Bool | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 160 | `op2.enable` | OP2 Enable | Bool | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 54 |
| 161 | `op2.char` | OP2 Character | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 162 | `op2.phase_mode` | OP2 Phase Mode | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 163 | `op2.ac` | OP2 Mod Coupling | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 164 | `op2.mode` | OP2 Mod Mode | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 165 | `op2.sync` | OP2 Sync | Choice | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 30 |
| 166 | `op3.level` | OP3 Level | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 167 | `op3.ratio` | OP3 Ratio | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 32 |
| 168 | `op3.tune` | OP3 Tuning | Choice | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 169 | `op3.fixed` | OP3 Fixed | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 ; op3.tune=Fixed | 1 |
| 170 | `op3.offset` | OP3 Offset | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 171 | `op3.coarse` | OP3 Coarse | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 172 | `op3.fine` | OP3 Fine | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 173 | `op3.wave` | OP3 Waveform | Choice | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 174 | `op3.wt_x` | OP3 WT X | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 ; op3.wave=Wavetable | 1 |
| 175 | `op3.wt_y` | OP3 WT Y | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 ; op3.wave=Wavetable | 1 |
| 176 | `op3.pan` | OP3 Pan | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 177 | `op3.mute` | OP3 Mute | Bool | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 178 | `op3.enable` | OP3 Enable | Bool | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 99 |
| 179 | `op3.char` | OP3 Character | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 180 | `op3.phase_mode` | OP3 Phase Mode | Choice | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 181 | `op3.ac` | OP3 Mod Coupling | Choice | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 182 | `op3.mode` | OP3 Mod Mode | Choice | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 183 | `op3.sync` | OP3 Sync | Choice | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 33 |
| 184 | `op4.level` | OP4 Level | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 185 | `op4.ratio` | OP4 Ratio | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 92 |
| 186 | `op4.tune` | OP4 Tuning | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 187 | `op4.fixed` | OP4 Fixed | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; op4.tune=Fixed | 1 |
| 188 | `op4.offset` | OP4 Offset | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 189 | `op4.coarse` | OP4 Coarse | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 190 | `op4.fine` | OP4 Fine | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 191 | `op4.wave` | OP4 Waveform | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 192 | `op4.wt_x` | OP4 WT X | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; op4.wave=Wavetable | 1 |
| 193 | `op4.wt_y` | OP4 WT Y | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; op4.wave=Wavetable | 1 |
| 194 | `op4.pan` | OP4 Pan | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 195 | `op4.mute` | OP4 Mute | Bool | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 196 | `op4.enable` | OP4 Enable | Bool | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 129 |
| 197 | `op4.char` | OP4 Character | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 198 | `op4.phase_mode` | OP4 Phase Mode | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 199 | `op4.ac` | OP4 Mod Coupling | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 200 | `op4.mode` | OP4 Mod Mode | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 201 | `op4.sync` | OP4 Sync | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 93 |
| 202 | `fm.char_master` | FM Character Master | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM COMMUN | 12 |
| 203 | `fm.fb_mode` | FM Feedback Reset | Choice | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM COMMUN | 12 |
| 204 | `fm.op1_op3.depth` | FM OP1>OP3 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 205 | `fm.op1_op4.depth` | FM OP1>OP4 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 206 | `fm.op2_op3.depth` | FM OP2>OP3 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 207 | `fm.op2_op4.depth` | FM OP2>OP4 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 208 | `fm.op3_op1.depth` | FM OP3>OP1 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 209 | `fm.op3_op2.depth` | FM OP3>OP2 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 210 | `fm.op3_op3.depth` | FM OP3 Self | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 211 | `fm.op3_op4.depth` | FM OP3>OP4 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 212 | `fm.op4_op1.depth` | FM OP4>OP1 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 213 | `fm.op4_op2.depth` | FM OP4>OP2 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 214 | `fm.op4_op3.depth` | FM OP4>OP3 | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 215 | `fm.op4_op4.depth` | FM OP4 Self | Float | SOUND DESIGN / MOTEURS / WT CARACTÈRE / CROSS BUS / AUDIO / FM MATRICE | 13 |
| 216 | `cross.wt_op3.depth` | CROSS WT>OP3 | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 32 |
| 217 | `cross.wt_op4.depth` | CROSS WT>OP4 | Float | SOUND DESIGN / MOTEURS / WT LECTURE / CROSS WT › FM / FM OP2 | 32 |
| 218 | `cross.op3_wt.depth` | CROSS OP3>WT | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 219 | `cross.op4_wt.depth` | CROSS OP4>WT | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 220 | `cross.bus_wt.depth` | CROSS BUS>WT | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 221 | `cross.op1_wt_freq.depth` | CROSS OP1>WT Freq | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 222 | `cross.op1_wt_x.depth` | CROSS OP1>WT X | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 223 | `cross.op1_wt_y.depth` | CROSS OP1>WT Y | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 224 | `cross.op1_wt_z.depth` | CROSS OP1>WT Z | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 225 | `cross.op1_wt_level.depth` | CROSS OP1>WT Level | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 226 | `cross.op1_wt_char.depth` | CROSS OP1>WT Char | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 227 | `cross.op2_wt_freq.depth` | CROSS OP2>WT Freq | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 228 | `cross.op2_wt_x.depth` | CROSS OP2>WT X | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 229 | `cross.op2_wt_y.depth` | CROSS OP2>WT Y | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 230 | `cross.op2_wt_z.depth` | CROSS OP2>WT Z | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 231 | `cross.op2_wt_level.depth` | CROSS OP2>WT Level | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 232 | `cross.op2_wt_char.depth` | CROSS OP2>WT Char | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 233 | `cross.op3_wt_freq.depth` | CROSS OP3>WT Freq | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 234 | `cross.op3_wt_x.depth` | CROSS OP3>WT X | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 235 | `cross.op3_wt_y.depth` | CROSS OP3>WT Y | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 236 | `cross.op3_wt_z.depth` | CROSS OP3>WT Z | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 237 | `cross.op3_wt_level.depth` | CROSS OP3>WT Level | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 238 | `cross.op3_wt_char.depth` | CROSS OP3>WT Char | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 239 | `cross.op4_wt_freq.depth` | CROSS OP4>WT Freq | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 240 | `cross.op4_wt_x.depth` | CROSS OP4>WT X | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 241 | `cross.op4_wt_y.depth` | CROSS OP4>WT Y | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 242 | `cross.op4_wt_z.depth` | CROSS OP4>WT Z | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 243 | `cross.op4_wt_level.depth` | CROSS OP4>WT Level | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 244 | `cross.op4_wt_char.depth` | CROSS OP4>WT Char | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 245 | `cross.bus_wt_freq.depth` | CROSS BUS>WT Freq | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 246 | `cross.bus_wt_x.depth` | CROSS BUS>WT X | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 247 | `cross.bus_wt_y.depth` | CROSS BUS>WT Y | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 248 | `cross.bus_wt_z.depth` | CROSS BUS>WT Z | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 249 | `cross.bus_wt_level.depth` | CROSS BUS>WT Level | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 250 | `cross.bus_wt_char.depth` | CROSS BUS>WT Char | Float | SOUND DESIGN / MOTEURS / WT PHASE / SYNC / CROSS FM › WT / FM OP3 | 35 |
| 251 | `cross.wt_fmode` | CROSS WT Freq Mode | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 116 |
| 252 | `cross.bus.w1` | CROSS Bus OP1 | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 116 |
| 253 | `cross.bus.w2` | CROSS Bus OP2 | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 116 |
| 254 | `cross.bus.w3` | CROSS Bus OP3 | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 116 |
| 255 | `cross.bus.w4` | CROSS Bus OP4 | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 116 |
| 256 | `cross.bus.mode` | CROSS Bus Mode | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 116 |
| 257 | `cross.dir` | CROSS Direction | Choice | SOUND DESIGN / MOTEURS / WT XYZ / CROSS DIRECT / FM OP1 | 21 |
| 258 | `cross.audio.mode` | Cross Audio Mode | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 116 |
| 259 | `cross.audio.amount` | Cross Audio Amount | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 | 116 |
| 260 | `cross.audio.am_dir` | Cross AM Direction | Choice | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; cross.audio.mode=AM | 6 |
| 261 | `flt.wt.type` | FLT WT Type | Choice | SOUND DESIGN / TRAITEMENTS | 293 |
| 262 | `flt.wt.pos` | FLT WT Position | Choice | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 36 |
| 263 | `flt.wt.stereo` | FLT WT Stereo | Choice | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 36 |
| 264 | `flt.wt.f1_mode` | FLT WT F1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 12 |
| 265 | `flt.wt.cutoff` | FLT WT Cutoff | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 36 |
| 266 | `flt.wt.res` | FLT WT Resonance | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 20 |
| 267 | `flt.wt.key` | FLT WT Keytrack | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 36 |
| 268 | `flt.wt.drive` | FLT WT Drive | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F3 Nonlinear | 8 |
| 269 | `flt.wt.bias` | FLT WT Bias | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F3 Nonlinear | 8 |
| 270 | `flt.wt.char` | FLT WT Character | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F3 Nonlinear | 8 |
| 271 | `flt.wt.f3_mode` | FLT WT F3 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F3 Nonlinear | 8 |
| 272 | `flt.wt.comb_fb` | FLT WT Comb Feedback | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F2 Comb | 6 |
| 273 | `flt.wt.comb_damp` | FLT WT Comb Damping | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F2 Comb | 6 |
| 274 | `flt.wt.detune` | FLT WT Detune | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F2 Comb | 6 |
| 275 | `flt.wt.spread` | FLT WT Stereo Spread | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 36 |
| 276 | `flt.wt.modal` | FLT WT Modal Model | Choice | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F4 Modal | 10 |
| 277 | `flt.wt.modes` | FLT WT Modes | Int | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F4 Modal | 10 |
| 278 | `flt.wt.decay` | FLT WT Decay | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F4 Modal | 10 |
| 279 | `flt.wt.damp` | FLT WT Damping | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F4 Modal | 10 |
| 280 | `flt.wt.bright` | FLT WT Brightness | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F4 Modal | 10 |
| 281 | `flt.wt.inharm` | FLT WT Inharmonicity | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F4 Modal | 10 |
| 282 | `flt.wt.freeze` | FLT WT Modal Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F4 Modal | 10 |
| 283 | `flt.wt.exc` | FLT WT Excitation | Choice | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 36 |
| 284 | `flt.wt.exc_level` | FLT WT Excitation Level | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 36 |
| 285 | `flt.wt.mix` | FLT WT Dry/Wet | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 36 |
| 286 | `flt.wt.out` | FLT WT Output | Float | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 36 |
| 287 | `flt.wt.arm` | FLT WT Autonomous Armed | Bool | SOUND DESIGN / TRAITEMENTS ; flt.wt.type=F1 Multimode | 48 |
| 288 | `flt.fm.type` | FLT FM Type | Choice | SOUND DESIGN / TRAITEMENTS | 293 |
| 289 | `flt.fm.pos` | FLT FM Position | Choice | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 37 |
| 290 | `flt.fm.stereo` | FLT FM Stereo | Choice | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 37 |
| 291 | `flt.fm.f1_mode` | FLT FM F1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 13 |
| 292 | `flt.fm.cutoff` | FLT FM Cutoff | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 37 |
| 293 | `flt.fm.res` | FLT FM Resonance | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 21 |
| 294 | `flt.fm.key` | FLT FM Keytrack | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 37 |
| 295 | `flt.fm.drive` | FLT FM Drive | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F3 Nonlinear | 8 |
| 296 | `flt.fm.bias` | FLT FM Bias | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F3 Nonlinear | 8 |
| 297 | `flt.fm.char` | FLT FM Character | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F3 Nonlinear | 8 |
| 298 | `flt.fm.f3_mode` | FLT FM F3 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F3 Nonlinear | 8 |
| 299 | `flt.fm.comb_fb` | FLT FM Comb Feedback | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F2 Comb | 6 |
| 300 | `flt.fm.comb_damp` | FLT FM Comb Damping | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F2 Comb | 6 |
| 301 | `flt.fm.detune` | FLT FM Detune | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F2 Comb | 6 |
| 302 | `flt.fm.spread` | FLT FM Stereo Spread | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 37 |
| 303 | `flt.fm.modal` | FLT FM Modal Model | Choice | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F4 Modal | 10 |
| 304 | `flt.fm.modes` | FLT FM Modes | Int | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F4 Modal | 10 |
| 305 | `flt.fm.decay` | FLT FM Decay | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F4 Modal | 10 |
| 306 | `flt.fm.damp` | FLT FM Damping | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F4 Modal | 10 |
| 307 | `flt.fm.bright` | FLT FM Brightness | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F4 Modal | 10 |
| 308 | `flt.fm.inharm` | FLT FM Inharmonicity | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F4 Modal | 10 |
| 309 | `flt.fm.freeze` | FLT FM Modal Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F4 Modal | 10 |
| 310 | `flt.fm.exc` | FLT FM Excitation | Choice | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 37 |
| 311 | `flt.fm.exc_level` | FLT FM Excitation Level | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 37 |
| 312 | `flt.fm.mix` | FLT FM Dry/Wet | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 37 |
| 313 | `flt.fm.out` | FLT FM Output | Float | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 37 |
| 314 | `flt.fm.arm` | FLT FM Autonomous Armed | Bool | SOUND DESIGN / TRAITEMENTS ; flt.fm.type=F1 Multimode | 49 |
| 315 | `flt.gl.type` | FLT GLOBAL Type | Choice | SOUND DESIGN / TRAITEMENTS | 293 |
| 316 | `flt.gl.pos` | FLT GLOBAL Position | Choice | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 37 |
| 317 | `flt.gl.stereo` | FLT GLOBAL Stereo | Choice | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 37 |
| 318 | `flt.gl.f1_mode` | FLT GLOBAL F1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 13 |
| 319 | `flt.gl.cutoff` | FLT GLOBAL Cutoff | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 37 |
| 320 | `flt.gl.res` | FLT GLOBAL Resonance | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 21 |
| 321 | `flt.gl.key` | FLT GLOBAL Keytrack | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 37 |
| 322 | `flt.gl.drive` | FLT GLOBAL Drive | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F3 Nonlinear | 8 |
| 323 | `flt.gl.bias` | FLT GLOBAL Bias | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F3 Nonlinear | 8 |
| 324 | `flt.gl.char` | FLT GLOBAL Character | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F3 Nonlinear | 8 |
| 325 | `flt.gl.f3_mode` | FLT GLOBAL F3 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F3 Nonlinear | 8 |
| 326 | `flt.gl.comb_fb` | FLT GLOBAL Comb Feedback | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F2 Comb | 6 |
| 327 | `flt.gl.comb_damp` | FLT GLOBAL Comb Damping | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F2 Comb | 6 |
| 328 | `flt.gl.detune` | FLT GLOBAL Detune | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F2 Comb | 6 |
| 329 | `flt.gl.spread` | FLT GLOBAL Stereo Spread | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 37 |
| 330 | `flt.gl.modal` | FLT GLOBAL Modal Model | Choice | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F4 Modal | 10 |
| 331 | `flt.gl.modes` | FLT GLOBAL Modes | Int | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F4 Modal | 10 |
| 332 | `flt.gl.decay` | FLT GLOBAL Decay | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F4 Modal | 10 |
| 333 | `flt.gl.damp` | FLT GLOBAL Damping | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F4 Modal | 10 |
| 334 | `flt.gl.bright` | FLT GLOBAL Brightness | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F4 Modal | 10 |
| 335 | `flt.gl.inharm` | FLT GLOBAL Inharmonicity | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F4 Modal | 10 |
| 336 | `flt.gl.freeze` | FLT GLOBAL Modal Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F4 Modal | 10 |
| 337 | `flt.gl.exc` | FLT GLOBAL Excitation | Choice | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 37 |
| 338 | `flt.gl.exc_level` | FLT GLOBAL Excitation Level | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 37 |
| 339 | `flt.gl.mix` | FLT GLOBAL Dry/Wet | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 37 |
| 340 | `flt.gl.out` | FLT GLOBAL Output | Float | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 37 |
| 341 | `flt.gl.arm` | FLT GLOBAL Autonomous Armed | Bool | SOUND DESIGN / TRAITEMENTS ; flt.gl.type=F1 Multimode | 52 |
| 342 | `dst.wt1.type` | DST WT1 Type | Choice | SOUND DESIGN / TRAITEMENTS | 293 |
| 343 | `dst.wt1.mix` | DST WT1 Dry/Wet | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N1 Asymmetric Wavefolder | 30 |
| 344 | `dst.wt1.out` | DST WT1 Output | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N1 Asymmetric Wavefolder | 30 |
| 345 | `dst.wt1.arm` | DST WT1 Autonomous Armed | Bool | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N2 Feedback Waveshaper | 14 |
| 346 | `dst.wt1.n1.drive` | DST WT1 N1 Drive | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N1 Asymmetric Wavefolder | 2 |
| 347 | `dst.wt1.n1.fold` | DST WT1 N1 Fold | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N1 Asymmetric Wavefolder | 2 |
| 348 | `dst.wt1.n1.bias` | DST WT1 N1 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N1 Asymmetric Wavefolder | 2 |
| 349 | `dst.wt1.n1.sym` | DST WT1 N1 Symmetry | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N1 Asymmetric Wavefolder | 2 |
| 350 | `dst.wt1.n1.stages` | DST WT1 N1 Stages | Int | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N1 Asymmetric Wavefolder | 2 |
| 351 | `dst.wt1.n1.tone` | DST WT1 N1 Tone | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N1 Asymmetric Wavefolder | 2 |
| 352 | `dst.wt1.d1.bits` | DST WT1 D1 Bits | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D1 Variable Rate Crusher | 4 |
| 353 | `dst.wt1.d1.rate` | DST WT1 D1 Rate Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D1 Variable Rate Crusher | 4 |
| 354 | `dst.wt1.d1.jitter` | DST WT1 D1 Jitter | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D1 Variable Rate Crusher | 4 |
| 355 | `dst.wt1.d1.mode` | DST WT1 D1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D1 Variable Rate Crusher | 4 |
| 356 | `dst.wt1.d1.hold` | DST WT1 D1 Hold | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D1 Variable Rate Crusher | 4 |
| 357 | `dst.wt1.y1.thresh` | DST WT1 Y1 Threshold | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y1 Transient Destroyer | 2 |
| 358 | `dst.wt1.y1.ratio` | DST WT1 Y1 Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y1 Transient Destroyer | 2 |
| 359 | `dst.wt1.y1.attack` | DST WT1 Y1 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y1 Transient Destroyer | 2 |
| 360 | `dst.wt1.y1.release` | DST WT1 Y1 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y1 Transient Destroyer | 2 |
| 361 | `dst.wt1.y1.knee` | DST WT1 Y1 Knee | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y1 Transient Destroyer | 2 |
| 362 | `dst.wt1.y1.gate` | DST WT1 Y1 Gate | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y1 Transient Destroyer | 2 |
| 363 | `dst.wt1.y1.transient` | DST WT1 Y1 Transient | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y1 Transient Destroyer | 2 |
| 364 | `dst.wt1.y1.makeup` | DST WT1 Y1 Makeup | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y1 Transient Destroyer | 2 |
| 365 | `dst.wt1.s1.mode` | DST WT1 S1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S1 Ring/Frequency Shifter | 5 |
| 366 | `dst.wt1.s1.freq` | DST WT1 S1 Frequency | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S1 Ring/Frequency Shifter | 5 |
| 367 | `dst.wt1.s1.shift` | DST WT1 S1 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S1 Ring/Frequency Shifter | 5 |
| 368 | `dst.wt1.s1.carrier` | DST WT1 S1 Carrier | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S1 Ring/Frequency Shifter | 5 |
| 369 | `dst.wt1.s1.phase` | DST WT1 S1 Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S1 Ring/Frequency Shifter | 5 |
| 370 | `dst.wt1.s1.stereo` | DST WT1 S1 Stereo Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S1 Ring/Frequency Shifter | 5 |
| 371 | `dst.wt1.n2.fb` | DST WT1 N2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N2 Feedback Waveshaper | 4 |
| 372 | `dst.wt1.n2.delay` | DST WT1 N2 Delay | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N2 Feedback Waveshaper | 4 |
| 373 | `dst.wt1.n2.damp` | DST WT1 N2 Damping | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N2 Feedback Waveshaper | 4 |
| 374 | `dst.wt1.n2.bias` | DST WT1 N2 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N2 Feedback Waveshaper | 4 |
| 375 | `dst.wt1.n2.res` | DST WT1 N2 Resonance | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N2 Feedback Waveshaper | 4 |
| 376 | `dst.wt1.n2.selfosc` | DST WT1 N2 Self-Oscillation | Bool | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=N2 Feedback Waveshaper | 5 |
| 377 | `dst.wt1.d2.size` | DST WT1 D2 Size | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | 4 |
| 378 | `dst.wt1.d2.speed` | DST WT1 D2 Playback | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | 4 |
| 379 | `dst.wt1.d2.repeat` | DST WT1 D2 Repeat | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | 4 |
| 380 | `dst.wt1.d2.pos` | DST WT1 D2 Position | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | 4 |
| 381 | `dst.wt1.d2.fb` | DST WT1 D2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | 4 |
| 382 | `dst.wt1.d2.xfade` | DST WT1 D2 Crossfade | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | 4 |
| 383 | `dst.wt1.d2.freeze` | DST WT1 D2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | 8 |
| 384 | `dst.wt1.y2.attack` | DST WT1 Y2 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y2 Envelope Shaper | 3 |
| 385 | `dst.wt1.y2.release` | DST WT1 Y2 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y2 Envelope Shaper | 3 |
| 386 | `dst.wt1.y2.sens` | DST WT1 Y2 Sensitivity | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y2 Envelope Shaper | 3 |
| 387 | `dst.wt1.y2.curve` | DST WT1 Y2 Curve | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y2 Envelope Shaper | 3 |
| 388 | `dst.wt1.y2.depth` | DST WT1 Y2 Depth | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y2 Envelope Shaper | 3 |
| 389 | `dst.wt1.y2.hold` | DST WT1 Y2 Hold | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y2 Envelope Shaper | 3 |
| 390 | `dst.wt1.y2.detect` | DST WT1 Y2 Detector | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=Y2 Envelope Shaper | 3 |
| 391 | `dst.wt1.s2.fft` | DST WT1 S2 FFT Size | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S2 Spectral Freeze | 6 |
| 392 | `dst.wt1.s2.freeze` | DST WT1 S2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S2 Spectral Freeze | 21 |
| 393 | `dst.wt1.s2.decay` | DST WT1 S2 Decay | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S2 Spectral Freeze | 6 |
| 394 | `dst.wt1.s2.blur` | DST WT1 S2 Blur | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S2 Spectral Freeze | 6 |
| 395 | `dst.wt1.s2.tilt` | DST WT1 S2 Tilt | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S2 Spectral Freeze | 6 |
| 396 | `dst.wt1.s2.drift` | DST WT1 S2 Phase Drift | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S2 Spectral Freeze | 6 |
| 397 | `dst.wt1.s2.shift` | DST WT1 S2 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=S2 Spectral Freeze | 6 |
| 398 | `dst.wt2.type` | DST WT2 Type | Choice | SOUND DESIGN / TRAITEMENTS | 293 |
| 399 | `dst.wt2.mix` | DST WT2 Dry/Wet | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N1 Asymmetric Wavefolder | 30 |
| 400 | `dst.wt2.out` | DST WT2 Output | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N1 Asymmetric Wavefolder | 30 |
| 401 | `dst.wt2.arm` | DST WT2 Autonomous Armed | Bool | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N2 Feedback Waveshaper | 14 |
| 402 | `dst.wt2.n1.drive` | DST WT2 N1 Drive | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N1 Asymmetric Wavefolder | 2 |
| 403 | `dst.wt2.n1.fold` | DST WT2 N1 Fold | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N1 Asymmetric Wavefolder | 2 |
| 404 | `dst.wt2.n1.bias` | DST WT2 N1 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N1 Asymmetric Wavefolder | 2 |
| 405 | `dst.wt2.n1.sym` | DST WT2 N1 Symmetry | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N1 Asymmetric Wavefolder | 2 |
| 406 | `dst.wt2.n1.stages` | DST WT2 N1 Stages | Int | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N1 Asymmetric Wavefolder | 2 |
| 407 | `dst.wt2.n1.tone` | DST WT2 N1 Tone | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N1 Asymmetric Wavefolder | 2 |
| 408 | `dst.wt2.d1.bits` | DST WT2 D1 Bits | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D1 Variable Rate Crusher | 4 |
| 409 | `dst.wt2.d1.rate` | DST WT2 D1 Rate Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D1 Variable Rate Crusher | 4 |
| 410 | `dst.wt2.d1.jitter` | DST WT2 D1 Jitter | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D1 Variable Rate Crusher | 4 |
| 411 | `dst.wt2.d1.mode` | DST WT2 D1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D1 Variable Rate Crusher | 4 |
| 412 | `dst.wt2.d1.hold` | DST WT2 D1 Hold | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D1 Variable Rate Crusher | 4 |
| 413 | `dst.wt2.y1.thresh` | DST WT2 Y1 Threshold | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y1 Transient Destroyer | 2 |
| 414 | `dst.wt2.y1.ratio` | DST WT2 Y1 Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y1 Transient Destroyer | 2 |
| 415 | `dst.wt2.y1.attack` | DST WT2 Y1 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y1 Transient Destroyer | 2 |
| 416 | `dst.wt2.y1.release` | DST WT2 Y1 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y1 Transient Destroyer | 2 |
| 417 | `dst.wt2.y1.knee` | DST WT2 Y1 Knee | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y1 Transient Destroyer | 2 |
| 418 | `dst.wt2.y1.gate` | DST WT2 Y1 Gate | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y1 Transient Destroyer | 2 |
| 419 | `dst.wt2.y1.transient` | DST WT2 Y1 Transient | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y1 Transient Destroyer | 2 |
| 420 | `dst.wt2.y1.makeup` | DST WT2 Y1 Makeup | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y1 Transient Destroyer | 2 |
| 421 | `dst.wt2.s1.mode` | DST WT2 S1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S1 Ring/Frequency Shifter | 5 |
| 422 | `dst.wt2.s1.freq` | DST WT2 S1 Frequency | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S1 Ring/Frequency Shifter | 5 |
| 423 | `dst.wt2.s1.shift` | DST WT2 S1 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S1 Ring/Frequency Shifter | 5 |
| 424 | `dst.wt2.s1.carrier` | DST WT2 S1 Carrier | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S1 Ring/Frequency Shifter | 5 |
| 425 | `dst.wt2.s1.phase` | DST WT2 S1 Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S1 Ring/Frequency Shifter | 5 |
| 426 | `dst.wt2.s1.stereo` | DST WT2 S1 Stereo Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S1 Ring/Frequency Shifter | 5 |
| 427 | `dst.wt2.n2.fb` | DST WT2 N2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N2 Feedback Waveshaper | 4 |
| 428 | `dst.wt2.n2.delay` | DST WT2 N2 Delay | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N2 Feedback Waveshaper | 4 |
| 429 | `dst.wt2.n2.damp` | DST WT2 N2 Damping | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N2 Feedback Waveshaper | 4 |
| 430 | `dst.wt2.n2.bias` | DST WT2 N2 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N2 Feedback Waveshaper | 4 |
| 431 | `dst.wt2.n2.res` | DST WT2 N2 Resonance | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N2 Feedback Waveshaper | 4 |
| 432 | `dst.wt2.n2.selfosc` | DST WT2 N2 Self-Oscillation | Bool | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=N2 Feedback Waveshaper | 5 |
| 433 | `dst.wt2.d2.size` | DST WT2 D2 Size | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | 4 |
| 434 | `dst.wt2.d2.speed` | DST WT2 D2 Playback | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | 4 |
| 435 | `dst.wt2.d2.repeat` | DST WT2 D2 Repeat | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | 4 |
| 436 | `dst.wt2.d2.pos` | DST WT2 D2 Position | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | 4 |
| 437 | `dst.wt2.d2.fb` | DST WT2 D2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | 4 |
| 438 | `dst.wt2.d2.xfade` | DST WT2 D2 Crossfade | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | 4 |
| 439 | `dst.wt2.d2.freeze` | DST WT2 D2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | 8 |
| 440 | `dst.wt2.y2.attack` | DST WT2 Y2 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y2 Envelope Shaper | 3 |
| 441 | `dst.wt2.y2.release` | DST WT2 Y2 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y2 Envelope Shaper | 3 |
| 442 | `dst.wt2.y2.sens` | DST WT2 Y2 Sensitivity | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y2 Envelope Shaper | 3 |
| 443 | `dst.wt2.y2.curve` | DST WT2 Y2 Curve | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y2 Envelope Shaper | 3 |
| 444 | `dst.wt2.y2.depth` | DST WT2 Y2 Depth | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y2 Envelope Shaper | 3 |
| 445 | `dst.wt2.y2.hold` | DST WT2 Y2 Hold | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y2 Envelope Shaper | 3 |
| 446 | `dst.wt2.y2.detect` | DST WT2 Y2 Detector | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=Y2 Envelope Shaper | 3 |
| 447 | `dst.wt2.s2.fft` | DST WT2 S2 FFT Size | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S2 Spectral Freeze | 6 |
| 448 | `dst.wt2.s2.freeze` | DST WT2 S2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S2 Spectral Freeze | 21 |
| 449 | `dst.wt2.s2.decay` | DST WT2 S2 Decay | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S2 Spectral Freeze | 6 |
| 450 | `dst.wt2.s2.blur` | DST WT2 S2 Blur | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S2 Spectral Freeze | 6 |
| 451 | `dst.wt2.s2.tilt` | DST WT2 S2 Tilt | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S2 Spectral Freeze | 6 |
| 452 | `dst.wt2.s2.drift` | DST WT2 S2 Phase Drift | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S2 Spectral Freeze | 6 |
| 453 | `dst.wt2.s2.shift` | DST WT2 S2 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=S2 Spectral Freeze | 6 |
| 454 | `dst.fm1.type` | DST FM1 Type | Choice | SOUND DESIGN / TRAITEMENTS | 293 |
| 455 | `dst.fm1.mix` | DST FM1 Dry/Wet | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N1 Asymmetric Wavefolder | 30 |
| 456 | `dst.fm1.out` | DST FM1 Output | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N1 Asymmetric Wavefolder | 30 |
| 457 | `dst.fm1.arm` | DST FM1 Autonomous Armed | Bool | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N2 Feedback Waveshaper | 14 |
| 458 | `dst.fm1.n1.drive` | DST FM1 N1 Drive | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N1 Asymmetric Wavefolder | 2 |
| 459 | `dst.fm1.n1.fold` | DST FM1 N1 Fold | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N1 Asymmetric Wavefolder | 2 |
| 460 | `dst.fm1.n1.bias` | DST FM1 N1 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N1 Asymmetric Wavefolder | 2 |
| 461 | `dst.fm1.n1.sym` | DST FM1 N1 Symmetry | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N1 Asymmetric Wavefolder | 2 |
| 462 | `dst.fm1.n1.stages` | DST FM1 N1 Stages | Int | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N1 Asymmetric Wavefolder | 2 |
| 463 | `dst.fm1.n1.tone` | DST FM1 N1 Tone | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N1 Asymmetric Wavefolder | 2 |
| 464 | `dst.fm1.d1.bits` | DST FM1 D1 Bits | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D1 Variable Rate Crusher | 4 |
| 465 | `dst.fm1.d1.rate` | DST FM1 D1 Rate Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D1 Variable Rate Crusher | 4 |
| 466 | `dst.fm1.d1.jitter` | DST FM1 D1 Jitter | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D1 Variable Rate Crusher | 4 |
| 467 | `dst.fm1.d1.mode` | DST FM1 D1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D1 Variable Rate Crusher | 4 |
| 468 | `dst.fm1.d1.hold` | DST FM1 D1 Hold | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D1 Variable Rate Crusher | 4 |
| 469 | `dst.fm1.y1.thresh` | DST FM1 Y1 Threshold | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y1 Transient Destroyer | 2 |
| 470 | `dst.fm1.y1.ratio` | DST FM1 Y1 Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y1 Transient Destroyer | 2 |
| 471 | `dst.fm1.y1.attack` | DST FM1 Y1 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y1 Transient Destroyer | 2 |
| 472 | `dst.fm1.y1.release` | DST FM1 Y1 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y1 Transient Destroyer | 2 |
| 473 | `dst.fm1.y1.knee` | DST FM1 Y1 Knee | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y1 Transient Destroyer | 2 |
| 474 | `dst.fm1.y1.gate` | DST FM1 Y1 Gate | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y1 Transient Destroyer | 2 |
| 475 | `dst.fm1.y1.transient` | DST FM1 Y1 Transient | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y1 Transient Destroyer | 2 |
| 476 | `dst.fm1.y1.makeup` | DST FM1 Y1 Makeup | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y1 Transient Destroyer | 2 |
| 477 | `dst.fm1.s1.mode` | DST FM1 S1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S1 Ring/Frequency Shifter | 5 |
| 478 | `dst.fm1.s1.freq` | DST FM1 S1 Frequency | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S1 Ring/Frequency Shifter | 5 |
| 479 | `dst.fm1.s1.shift` | DST FM1 S1 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S1 Ring/Frequency Shifter | 5 |
| 480 | `dst.fm1.s1.carrier` | DST FM1 S1 Carrier | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S1 Ring/Frequency Shifter | 5 |
| 481 | `dst.fm1.s1.phase` | DST FM1 S1 Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S1 Ring/Frequency Shifter | 5 |
| 482 | `dst.fm1.s1.stereo` | DST FM1 S1 Stereo Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S1 Ring/Frequency Shifter | 5 |
| 483 | `dst.fm1.n2.fb` | DST FM1 N2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N2 Feedback Waveshaper | 4 |
| 484 | `dst.fm1.n2.delay` | DST FM1 N2 Delay | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N2 Feedback Waveshaper | 4 |
| 485 | `dst.fm1.n2.damp` | DST FM1 N2 Damping | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N2 Feedback Waveshaper | 4 |
| 486 | `dst.fm1.n2.bias` | DST FM1 N2 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N2 Feedback Waveshaper | 4 |
| 487 | `dst.fm1.n2.res` | DST FM1 N2 Resonance | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N2 Feedback Waveshaper | 4 |
| 488 | `dst.fm1.n2.selfosc` | DST FM1 N2 Self-Oscillation | Bool | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=N2 Feedback Waveshaper | 5 |
| 489 | `dst.fm1.d2.size` | DST FM1 D2 Size | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | 4 |
| 490 | `dst.fm1.d2.speed` | DST FM1 D2 Playback | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | 4 |
| 491 | `dst.fm1.d2.repeat` | DST FM1 D2 Repeat | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | 4 |
| 492 | `dst.fm1.d2.pos` | DST FM1 D2 Position | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | 4 |
| 493 | `dst.fm1.d2.fb` | DST FM1 D2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | 4 |
| 494 | `dst.fm1.d2.xfade` | DST FM1 D2 Crossfade | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | 4 |
| 495 | `dst.fm1.d2.freeze` | DST FM1 D2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | 8 |
| 496 | `dst.fm1.y2.attack` | DST FM1 Y2 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y2 Envelope Shaper | 3 |
| 497 | `dst.fm1.y2.release` | DST FM1 Y2 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y2 Envelope Shaper | 3 |
| 498 | `dst.fm1.y2.sens` | DST FM1 Y2 Sensitivity | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y2 Envelope Shaper | 3 |
| 499 | `dst.fm1.y2.curve` | DST FM1 Y2 Curve | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y2 Envelope Shaper | 3 |
| 500 | `dst.fm1.y2.depth` | DST FM1 Y2 Depth | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y2 Envelope Shaper | 3 |
| 501 | `dst.fm1.y2.hold` | DST FM1 Y2 Hold | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y2 Envelope Shaper | 3 |
| 502 | `dst.fm1.y2.detect` | DST FM1 Y2 Detector | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=Y2 Envelope Shaper | 3 |
| 503 | `dst.fm1.s2.fft` | DST FM1 S2 FFT Size | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S2 Spectral Freeze | 6 |
| 504 | `dst.fm1.s2.freeze` | DST FM1 S2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S2 Spectral Freeze | 21 |
| 505 | `dst.fm1.s2.decay` | DST FM1 S2 Decay | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S2 Spectral Freeze | 6 |
| 506 | `dst.fm1.s2.blur` | DST FM1 S2 Blur | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S2 Spectral Freeze | 6 |
| 507 | `dst.fm1.s2.tilt` | DST FM1 S2 Tilt | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S2 Spectral Freeze | 6 |
| 508 | `dst.fm1.s2.drift` | DST FM1 S2 Phase Drift | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S2 Spectral Freeze | 6 |
| 509 | `dst.fm1.s2.shift` | DST FM1 S2 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=S2 Spectral Freeze | 6 |
| 510 | `dst.fm2.type` | DST FM2 Type | Choice | SOUND DESIGN / TRAITEMENTS | 293 |
| 511 | `dst.fm2.mix` | DST FM2 Dry/Wet | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N1 Asymmetric Wavefolder | 30 |
| 512 | `dst.fm2.out` | DST FM2 Output | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N1 Asymmetric Wavefolder | 30 |
| 513 | `dst.fm2.arm` | DST FM2 Autonomous Armed | Bool | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N2 Feedback Waveshaper | 14 |
| 514 | `dst.fm2.n1.drive` | DST FM2 N1 Drive | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N1 Asymmetric Wavefolder | 2 |
| 515 | `dst.fm2.n1.fold` | DST FM2 N1 Fold | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N1 Asymmetric Wavefolder | 2 |
| 516 | `dst.fm2.n1.bias` | DST FM2 N1 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N1 Asymmetric Wavefolder | 2 |
| 517 | `dst.fm2.n1.sym` | DST FM2 N1 Symmetry | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N1 Asymmetric Wavefolder | 2 |
| 518 | `dst.fm2.n1.stages` | DST FM2 N1 Stages | Int | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N1 Asymmetric Wavefolder | 2 |
| 519 | `dst.fm2.n1.tone` | DST FM2 N1 Tone | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N1 Asymmetric Wavefolder | 2 |
| 520 | `dst.fm2.d1.bits` | DST FM2 D1 Bits | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D1 Variable Rate Crusher | 4 |
| 521 | `dst.fm2.d1.rate` | DST FM2 D1 Rate Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D1 Variable Rate Crusher | 4 |
| 522 | `dst.fm2.d1.jitter` | DST FM2 D1 Jitter | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D1 Variable Rate Crusher | 4 |
| 523 | `dst.fm2.d1.mode` | DST FM2 D1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D1 Variable Rate Crusher | 4 |
| 524 | `dst.fm2.d1.hold` | DST FM2 D1 Hold | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D1 Variable Rate Crusher | 4 |
| 525 | `dst.fm2.y1.thresh` | DST FM2 Y1 Threshold | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y1 Transient Destroyer | 2 |
| 526 | `dst.fm2.y1.ratio` | DST FM2 Y1 Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y1 Transient Destroyer | 2 |
| 527 | `dst.fm2.y1.attack` | DST FM2 Y1 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y1 Transient Destroyer | 2 |
| 528 | `dst.fm2.y1.release` | DST FM2 Y1 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y1 Transient Destroyer | 2 |
| 529 | `dst.fm2.y1.knee` | DST FM2 Y1 Knee | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y1 Transient Destroyer | 2 |
| 530 | `dst.fm2.y1.gate` | DST FM2 Y1 Gate | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y1 Transient Destroyer | 2 |
| 531 | `dst.fm2.y1.transient` | DST FM2 Y1 Transient | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y1 Transient Destroyer | 2 |
| 532 | `dst.fm2.y1.makeup` | DST FM2 Y1 Makeup | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y1 Transient Destroyer | 2 |
| 533 | `dst.fm2.s1.mode` | DST FM2 S1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S1 Ring/Frequency Shifter | 5 |
| 534 | `dst.fm2.s1.freq` | DST FM2 S1 Frequency | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S1 Ring/Frequency Shifter | 5 |
| 535 | `dst.fm2.s1.shift` | DST FM2 S1 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S1 Ring/Frequency Shifter | 5 |
| 536 | `dst.fm2.s1.carrier` | DST FM2 S1 Carrier | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S1 Ring/Frequency Shifter | 5 |
| 537 | `dst.fm2.s1.phase` | DST FM2 S1 Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S1 Ring/Frequency Shifter | 5 |
| 538 | `dst.fm2.s1.stereo` | DST FM2 S1 Stereo Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S1 Ring/Frequency Shifter | 5 |
| 539 | `dst.fm2.n2.fb` | DST FM2 N2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N2 Feedback Waveshaper | 4 |
| 540 | `dst.fm2.n2.delay` | DST FM2 N2 Delay | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N2 Feedback Waveshaper | 4 |
| 541 | `dst.fm2.n2.damp` | DST FM2 N2 Damping | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N2 Feedback Waveshaper | 4 |
| 542 | `dst.fm2.n2.bias` | DST FM2 N2 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N2 Feedback Waveshaper | 4 |
| 543 | `dst.fm2.n2.res` | DST FM2 N2 Resonance | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N2 Feedback Waveshaper | 4 |
| 544 | `dst.fm2.n2.selfosc` | DST FM2 N2 Self-Oscillation | Bool | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=N2 Feedback Waveshaper | 5 |
| 545 | `dst.fm2.d2.size` | DST FM2 D2 Size | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | 4 |
| 546 | `dst.fm2.d2.speed` | DST FM2 D2 Playback | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | 4 |
| 547 | `dst.fm2.d2.repeat` | DST FM2 D2 Repeat | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | 4 |
| 548 | `dst.fm2.d2.pos` | DST FM2 D2 Position | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | 4 |
| 549 | `dst.fm2.d2.fb` | DST FM2 D2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | 4 |
| 550 | `dst.fm2.d2.xfade` | DST FM2 D2 Crossfade | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | 4 |
| 551 | `dst.fm2.d2.freeze` | DST FM2 D2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | 8 |
| 552 | `dst.fm2.y2.attack` | DST FM2 Y2 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y2 Envelope Shaper | 3 |
| 553 | `dst.fm2.y2.release` | DST FM2 Y2 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y2 Envelope Shaper | 3 |
| 554 | `dst.fm2.y2.sens` | DST FM2 Y2 Sensitivity | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y2 Envelope Shaper | 3 |
| 555 | `dst.fm2.y2.curve` | DST FM2 Y2 Curve | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y2 Envelope Shaper | 3 |
| 556 | `dst.fm2.y2.depth` | DST FM2 Y2 Depth | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y2 Envelope Shaper | 3 |
| 557 | `dst.fm2.y2.hold` | DST FM2 Y2 Hold | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y2 Envelope Shaper | 3 |
| 558 | `dst.fm2.y2.detect` | DST FM2 Y2 Detector | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=Y2 Envelope Shaper | 3 |
| 559 | `dst.fm2.s2.fft` | DST FM2 S2 FFT Size | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S2 Spectral Freeze | 6 |
| 560 | `dst.fm2.s2.freeze` | DST FM2 S2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S2 Spectral Freeze | 21 |
| 561 | `dst.fm2.s2.decay` | DST FM2 S2 Decay | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S2 Spectral Freeze | 6 |
| 562 | `dst.fm2.s2.blur` | DST FM2 S2 Blur | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S2 Spectral Freeze | 6 |
| 563 | `dst.fm2.s2.tilt` | DST FM2 S2 Tilt | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S2 Spectral Freeze | 6 |
| 564 | `dst.fm2.s2.drift` | DST FM2 S2 Phase Drift | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S2 Spectral Freeze | 6 |
| 565 | `dst.fm2.s2.shift` | DST FM2 S2 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=S2 Spectral Freeze | 6 |
| 566 | `dst.gl1.type` | DST GL1 Type | Choice | SOUND DESIGN / TRAITEMENTS | 293 |
| 567 | `dst.gl1.mix` | DST GL1 Dry/Wet | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N1 Asymmetric Wavefolder | 30 |
| 568 | `dst.gl1.out` | DST GL1 Output | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N1 Asymmetric Wavefolder | 30 |
| 569 | `dst.gl1.arm` | DST GL1 Autonomous Armed | Bool | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N2 Feedback Waveshaper | 14 |
| 570 | `dst.gl1.n1.drive` | DST GL1 N1 Drive | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N1 Asymmetric Wavefolder | 2 |
| 571 | `dst.gl1.n1.fold` | DST GL1 N1 Fold | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N1 Asymmetric Wavefolder | 2 |
| 572 | `dst.gl1.n1.bias` | DST GL1 N1 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N1 Asymmetric Wavefolder | 2 |
| 573 | `dst.gl1.n1.sym` | DST GL1 N1 Symmetry | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N1 Asymmetric Wavefolder | 2 |
| 574 | `dst.gl1.n1.stages` | DST GL1 N1 Stages | Int | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N1 Asymmetric Wavefolder | 2 |
| 575 | `dst.gl1.n1.tone` | DST GL1 N1 Tone | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N1 Asymmetric Wavefolder | 2 |
| 576 | `dst.gl1.d1.bits` | DST GL1 D1 Bits | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D1 Variable Rate Crusher | 4 |
| 577 | `dst.gl1.d1.rate` | DST GL1 D1 Rate Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D1 Variable Rate Crusher | 4 |
| 578 | `dst.gl1.d1.jitter` | DST GL1 D1 Jitter | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D1 Variable Rate Crusher | 4 |
| 579 | `dst.gl1.d1.mode` | DST GL1 D1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D1 Variable Rate Crusher | 4 |
| 580 | `dst.gl1.d1.hold` | DST GL1 D1 Hold | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D1 Variable Rate Crusher | 4 |
| 581 | `dst.gl1.y1.thresh` | DST GL1 Y1 Threshold | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y1 Transient Destroyer | 2 |
| 582 | `dst.gl1.y1.ratio` | DST GL1 Y1 Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y1 Transient Destroyer | 2 |
| 583 | `dst.gl1.y1.attack` | DST GL1 Y1 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y1 Transient Destroyer | 2 |
| 584 | `dst.gl1.y1.release` | DST GL1 Y1 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y1 Transient Destroyer | 2 |
| 585 | `dst.gl1.y1.knee` | DST GL1 Y1 Knee | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y1 Transient Destroyer | 2 |
| 586 | `dst.gl1.y1.gate` | DST GL1 Y1 Gate | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y1 Transient Destroyer | 2 |
| 587 | `dst.gl1.y1.transient` | DST GL1 Y1 Transient | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y1 Transient Destroyer | 2 |
| 588 | `dst.gl1.y1.makeup` | DST GL1 Y1 Makeup | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y1 Transient Destroyer | 2 |
| 589 | `dst.gl1.s1.mode` | DST GL1 S1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S1 Ring/Frequency Shifter | 5 |
| 590 | `dst.gl1.s1.freq` | DST GL1 S1 Frequency | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S1 Ring/Frequency Shifter | 5 |
| 591 | `dst.gl1.s1.shift` | DST GL1 S1 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S1 Ring/Frequency Shifter | 5 |
| 592 | `dst.gl1.s1.carrier` | DST GL1 S1 Carrier | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S1 Ring/Frequency Shifter | 5 |
| 593 | `dst.gl1.s1.phase` | DST GL1 S1 Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S1 Ring/Frequency Shifter | 5 |
| 594 | `dst.gl1.s1.stereo` | DST GL1 S1 Stereo Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S1 Ring/Frequency Shifter | 5 |
| 595 | `dst.gl1.n2.fb` | DST GL1 N2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N2 Feedback Waveshaper | 4 |
| 596 | `dst.gl1.n2.delay` | DST GL1 N2 Delay | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N2 Feedback Waveshaper | 4 |
| 597 | `dst.gl1.n2.damp` | DST GL1 N2 Damping | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N2 Feedback Waveshaper | 4 |
| 598 | `dst.gl1.n2.bias` | DST GL1 N2 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N2 Feedback Waveshaper | 4 |
| 599 | `dst.gl1.n2.res` | DST GL1 N2 Resonance | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N2 Feedback Waveshaper | 4 |
| 600 | `dst.gl1.n2.selfosc` | DST GL1 N2 Self-Oscillation | Bool | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=N2 Feedback Waveshaper | 5 |
| 601 | `dst.gl1.d2.size` | DST GL1 D2 Size | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | 4 |
| 602 | `dst.gl1.d2.speed` | DST GL1 D2 Playback | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | 4 |
| 603 | `dst.gl1.d2.repeat` | DST GL1 D2 Repeat | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | 4 |
| 604 | `dst.gl1.d2.pos` | DST GL1 D2 Position | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | 4 |
| 605 | `dst.gl1.d2.fb` | DST GL1 D2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | 4 |
| 606 | `dst.gl1.d2.xfade` | DST GL1 D2 Crossfade | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | 4 |
| 607 | `dst.gl1.d2.freeze` | DST GL1 D2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | 8 |
| 608 | `dst.gl1.y2.attack` | DST GL1 Y2 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y2 Envelope Shaper | 3 |
| 609 | `dst.gl1.y2.release` | DST GL1 Y2 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y2 Envelope Shaper | 3 |
| 610 | `dst.gl1.y2.sens` | DST GL1 Y2 Sensitivity | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y2 Envelope Shaper | 3 |
| 611 | `dst.gl1.y2.curve` | DST GL1 Y2 Curve | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y2 Envelope Shaper | 3 |
| 612 | `dst.gl1.y2.depth` | DST GL1 Y2 Depth | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y2 Envelope Shaper | 3 |
| 613 | `dst.gl1.y2.hold` | DST GL1 Y2 Hold | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y2 Envelope Shaper | 3 |
| 614 | `dst.gl1.y2.detect` | DST GL1 Y2 Detector | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=Y2 Envelope Shaper | 3 |
| 615 | `dst.gl1.s2.fft` | DST GL1 S2 FFT Size | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S2 Spectral Freeze | 6 |
| 616 | `dst.gl1.s2.freeze` | DST GL1 S2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S2 Spectral Freeze | 21 |
| 617 | `dst.gl1.s2.decay` | DST GL1 S2 Decay | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S2 Spectral Freeze | 6 |
| 618 | `dst.gl1.s2.blur` | DST GL1 S2 Blur | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S2 Spectral Freeze | 6 |
| 619 | `dst.gl1.s2.tilt` | DST GL1 S2 Tilt | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S2 Spectral Freeze | 6 |
| 620 | `dst.gl1.s2.drift` | DST GL1 S2 Phase Drift | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S2 Spectral Freeze | 6 |
| 621 | `dst.gl1.s2.shift` | DST GL1 S2 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=S2 Spectral Freeze | 6 |
| 622 | `dst.gl2.type` | DST GL2 Type | Choice | SOUND DESIGN / TRAITEMENTS | 293 |
| 623 | `dst.gl2.mix` | DST GL2 Dry/Wet | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N1 Asymmetric Wavefolder | 30 |
| 624 | `dst.gl2.out` | DST GL2 Output | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N1 Asymmetric Wavefolder | 30 |
| 625 | `dst.gl2.arm` | DST GL2 Autonomous Armed | Bool | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N2 Feedback Waveshaper | 14 |
| 626 | `dst.gl2.n1.drive` | DST GL2 N1 Drive | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N1 Asymmetric Wavefolder | 2 |
| 627 | `dst.gl2.n1.fold` | DST GL2 N1 Fold | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N1 Asymmetric Wavefolder | 2 |
| 628 | `dst.gl2.n1.bias` | DST GL2 N1 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N1 Asymmetric Wavefolder | 2 |
| 629 | `dst.gl2.n1.sym` | DST GL2 N1 Symmetry | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N1 Asymmetric Wavefolder | 2 |
| 630 | `dst.gl2.n1.stages` | DST GL2 N1 Stages | Int | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N1 Asymmetric Wavefolder | 2 |
| 631 | `dst.gl2.n1.tone` | DST GL2 N1 Tone | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N1 Asymmetric Wavefolder | 2 |
| 632 | `dst.gl2.d1.bits` | DST GL2 D1 Bits | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D1 Variable Rate Crusher | 4 |
| 633 | `dst.gl2.d1.rate` | DST GL2 D1 Rate Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D1 Variable Rate Crusher | 4 |
| 634 | `dst.gl2.d1.jitter` | DST GL2 D1 Jitter | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D1 Variable Rate Crusher | 4 |
| 635 | `dst.gl2.d1.mode` | DST GL2 D1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D1 Variable Rate Crusher | 4 |
| 636 | `dst.gl2.d1.hold` | DST GL2 D1 Hold | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D1 Variable Rate Crusher | 4 |
| 637 | `dst.gl2.y1.thresh` | DST GL2 Y1 Threshold | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y1 Transient Destroyer | 2 |
| 638 | `dst.gl2.y1.ratio` | DST GL2 Y1 Ratio | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y1 Transient Destroyer | 2 |
| 639 | `dst.gl2.y1.attack` | DST GL2 Y1 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y1 Transient Destroyer | 2 |
| 640 | `dst.gl2.y1.release` | DST GL2 Y1 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y1 Transient Destroyer | 2 |
| 641 | `dst.gl2.y1.knee` | DST GL2 Y1 Knee | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y1 Transient Destroyer | 2 |
| 642 | `dst.gl2.y1.gate` | DST GL2 Y1 Gate | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y1 Transient Destroyer | 2 |
| 643 | `dst.gl2.y1.transient` | DST GL2 Y1 Transient | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y1 Transient Destroyer | 2 |
| 644 | `dst.gl2.y1.makeup` | DST GL2 Y1 Makeup | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y1 Transient Destroyer | 2 |
| 645 | `dst.gl2.s1.mode` | DST GL2 S1 Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S1 Ring/Frequency Shifter | 5 |
| 646 | `dst.gl2.s1.freq` | DST GL2 S1 Frequency | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S1 Ring/Frequency Shifter | 5 |
| 647 | `dst.gl2.s1.shift` | DST GL2 S1 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S1 Ring/Frequency Shifter | 5 |
| 648 | `dst.gl2.s1.carrier` | DST GL2 S1 Carrier | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S1 Ring/Frequency Shifter | 5 |
| 649 | `dst.gl2.s1.phase` | DST GL2 S1 Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S1 Ring/Frequency Shifter | 5 |
| 650 | `dst.gl2.s1.stereo` | DST GL2 S1 Stereo Phase | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S1 Ring/Frequency Shifter | 5 |
| 651 | `dst.gl2.n2.fb` | DST GL2 N2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N2 Feedback Waveshaper | 4 |
| 652 | `dst.gl2.n2.delay` | DST GL2 N2 Delay | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N2 Feedback Waveshaper | 4 |
| 653 | `dst.gl2.n2.damp` | DST GL2 N2 Damping | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N2 Feedback Waveshaper | 4 |
| 654 | `dst.gl2.n2.bias` | DST GL2 N2 Bias | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N2 Feedback Waveshaper | 4 |
| 655 | `dst.gl2.n2.res` | DST GL2 N2 Resonance | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N2 Feedback Waveshaper | 4 |
| 656 | `dst.gl2.n2.selfosc` | DST GL2 N2 Self-Oscillation | Bool | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=N2 Feedback Waveshaper | 5 |
| 657 | `dst.gl2.d2.size` | DST GL2 D2 Size | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | 4 |
| 658 | `dst.gl2.d2.speed` | DST GL2 D2 Playback | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | 4 |
| 659 | `dst.gl2.d2.repeat` | DST GL2 D2 Repeat | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | 4 |
| 660 | `dst.gl2.d2.pos` | DST GL2 D2 Position | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | 4 |
| 661 | `dst.gl2.d2.fb` | DST GL2 D2 Feedback | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | 4 |
| 662 | `dst.gl2.d2.xfade` | DST GL2 D2 Crossfade | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | 4 |
| 663 | `dst.gl2.d2.freeze` | DST GL2 D2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | 8 |
| 664 | `dst.gl2.y2.attack` | DST GL2 Y2 Attack | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y2 Envelope Shaper | 3 |
| 665 | `dst.gl2.y2.release` | DST GL2 Y2 Release | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y2 Envelope Shaper | 3 |
| 666 | `dst.gl2.y2.sens` | DST GL2 Y2 Sensitivity | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y2 Envelope Shaper | 3 |
| 667 | `dst.gl2.y2.curve` | DST GL2 Y2 Curve | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y2 Envelope Shaper | 3 |
| 668 | `dst.gl2.y2.depth` | DST GL2 Y2 Depth | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y2 Envelope Shaper | 3 |
| 669 | `dst.gl2.y2.hold` | DST GL2 Y2 Hold | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y2 Envelope Shaper | 3 |
| 670 | `dst.gl2.y2.detect` | DST GL2 Y2 Detector | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=Y2 Envelope Shaper | 3 |
| 671 | `dst.gl2.s2.fft` | DST GL2 S2 FFT Size | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S2 Spectral Freeze | 6 |
| 672 | `dst.gl2.s2.freeze` | DST GL2 S2 Freeze | Bool | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S2 Spectral Freeze | 6 |
| 673 | `dst.gl2.s2.decay` | DST GL2 S2 Decay | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S2 Spectral Freeze | 6 |
| 674 | `dst.gl2.s2.blur` | DST GL2 S2 Blur | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S2 Spectral Freeze | 6 |
| 675 | `dst.gl2.s2.tilt` | DST GL2 S2 Tilt | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S2 Spectral Freeze | 6 |
| 676 | `dst.gl2.s2.drift` | DST GL2 S2 Phase Drift | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S2 Spectral Freeze | 6 |
| 677 | `dst.gl2.s2.shift` | DST GL2 S2 Shift | Float | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=S2 Spectral Freeze | 6 |
| 678 | `macro1` | Macro 1 | Float | PERFORMANCE / PERFORMANCE | 1 |
| 679 | `macro2` | Macro 2 | Float | PERFORMANCE / PERFORMANCE | 1 |
| 680 | `macro3` | Macro 3 | Float | PERFORMANCE / PERFORMANCE | 1 |
| 681 | `macro4` | Macro 4 | Float | PERFORMANCE / PERFORMANCE | 1 |
| 682 | `out.limiter` | Musical Limiter | Bool | SOUND DESIGN / TRAITEMENTS | 302 |
| 683 | `out.ceiling` | Limiter Ceiling | Float | SOUND DESIGN / TRAITEMENTS ; out.limiter=On | 1 |
| 684 | `perf.balance` | WT/FM Balance | Float | PERFORMANCE / PERFORMANCE | 1 |
| 685 | `amp.trigger` | Amp Trigger | Choice | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 28 |
| 686 | `amp.sync` | Amp Tempo Sync | Bool | SOUND DESIGN / MODULATION / ENV AMP / LFO1 | 38 |
| 687 | `env.wt.trigger` | ENV WT Trigger | Choice | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 29 |
| 688 | `env.wt.sync` | ENV WT Tempo Sync | Bool | SOUND DESIGN / MODULATION / ENV WT / LFO2 | 39 |
| 689 | `env.fm.trigger` | ENV FM Trigger | Choice | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 29 |
| 690 | `env.fm.sync` | ENV FM Tempo Sync | Bool | SOUND DESIGN / MODULATION / ENV FM / LFO3 | 39 |
| 691 | `env.aux1.trigger` | ENV AUX1 Trigger | Choice | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 29 |
| 692 | `env.aux1.sync` | ENV AUX1 Tempo Sync | Bool | SOUND DESIGN / MODULATION / ENV AUX1 / LFO4 | 39 |
| 693 | `env.aux2.trigger` | ENV AUX2 Trigger | Choice | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 29 |
| 694 | `env.aux2.sync` | ENV AUX2 Tempo Sync | Bool | SOUND DESIGN / MODULATION / ENV AUX2 / LFO4 | 53 |
| 695 | `dst.wt1.d2.fb_mode` | DST WT1 D2 Feedback Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt1.type=D2 Glitch Buffer | 4 |
| 696 | `dst.wt2.d2.fb_mode` | DST WT2 D2 Feedback Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.wt2.type=D2 Glitch Buffer | 4 |
| 697 | `dst.fm1.d2.fb_mode` | DST FM1 D2 Feedback Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm1.type=D2 Glitch Buffer | 4 |
| 698 | `dst.fm2.d2.fb_mode` | DST FM2 D2 Feedback Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.fm2.type=D2 Glitch Buffer | 4 |
| 699 | `dst.gl1.d2.fb_mode` | DST GL1 D2 Feedback Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl1.type=D2 Glitch Buffer | 4 |
| 700 | `dst.gl2.d2.fb_mode` | DST GL2 D2 Feedback Mode | Choice | SOUND DESIGN / TRAITEMENTS ; dst.gl2.type=D2 Glitch Buffer | 4 |
| 701 | `wt.za.shape` | WT ZA Shape | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.za.type=Harmonic Fold | 32 |
| 702 | `wt.zb.shape` | WT ZB Shape | Float | SOUND DESIGN / MOTEURS / WT Z / CROSS BUS / AUDIO / FM OP4 ; wt.zb.type=Harmonic Fold | 32 |
