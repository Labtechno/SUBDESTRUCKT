# ARCH-UI-03 — Cartographie StableID

Base : `90e12f982688cd9513b8dd50e5457cd05b259591` (`source/core/ParamTable.inc`). Inventaire exhaustif des 695 identifiants, affectations UI candidates; aucune preuve d'accessibilité ni d'effet DSP. Les contrôles conditionnels restent accessibles en activant leur contexte; les armements sont des actions explicites, jamais des modulations ou rappels implicites. Les alias lisent/écrivent le même ID. Les actions hors registre figurent dans la spécification ARCH-UI-03. Les futurs IDs doivent être ajoutés au contrôle de couverture.

| StableID | Nom registre | Accès principal candidat | Alias candidat |
|---|---|---|---|
| `amp.attack` | Amp Attack | Sound Design / Mod / Envelope AMP | — |
| `amp.decay` | Amp Decay | Sound Design / Mod / Envelope AMP | — |
| `amp.sustain` | Amp Sustain | Sound Design / Mod / Envelope AMP | — |
| `amp.release` | Amp Release | Sound Design / Mod / Envelope AMP | — |
| `amp.velocity` | Amp Velocity | Sound Design / Mod / Envelope AMP | — |
| `out.gain` | Output Gain | Persistent strip / out | — |
| `wt.level` | WT Level | Sound Design / WT / Direct | — |
| `op1.level` | OP1 Level | Sound Design / FM / OP1 / Direct | — |
| `op2.level` | OP2 Level | Sound Design / FM / OP2 / Direct | — |
| `op1.ratio` | OP1 Ratio | Sound Design / FM / OP1 / Direct | — |
| `op2.ratio` | OP2 Ratio | Sound Design / FM / OP2 / Direct | — |
| `fm.op2_op1.depth` | FM OP2>OP1 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op1_op2.depth` | FM OP1>OP2 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op1_op1.depth` | FM OP1 Self | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op2_op2.depth` | FM OP2 Self | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `cross.wt_op1.depth` | CROSS WT>OP1 | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.wt_op2.depth` | CROSS WT>OP2 | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op1_wt.depth` | CROSS OP1>WT | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op2_wt.depth` | CROSS OP2>WT | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.master` | CROSS Master | Sound Design / CROSS / Direct | Performance / numeric controls |
| `cross.fwd` | CROSS WT>FM | Sound Design / CROSS / Direct | Performance / numeric controls |
| `cross.rev` | CROSS FM>WT | Sound Design / CROSS / Direct | Performance / numeric controls |
| `voice.mode` | Voice Mode | Engineering / Voice | Sound Design / shortcut |
| `voice.priority` | Note Priority | Engineering / Voice | Sound Design / shortcut |
| `voice.bend_up` | Bend Up | Engineering / Voice | Sound Design / shortcut |
| `voice.bend_down` | Bend Down | Engineering / Voice | Sound Design / shortcut |
| `tune.a4` | Reference A4 | Engineering / Tuning | Sound Design / shortcut |
| `tune.scale` | Scale | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc0` | Tune C | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc1` | Tune C# | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc2` | Tune D | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc3` | Tune D# | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc4` | Tune E | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc5` | Tune F | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc6` | Tune F# | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc7` | Tune G | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc8` | Tune G# | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc9` | Tune A | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc10` | Tune A# | Engineering / Tuning | Sound Design / shortcut |
| `tune.pc11` | Tune B | Engineering / Tuning | Sound Design / shortcut |
| `amp.delay` | Amp Delay | Sound Design / Mod / Envelope AMP | — |
| `amp.hold` | Amp Hold | Sound Design / Mod / Envelope AMP | — |
| `amp.curve` | Amp Curve | Sound Design / Mod / Envelope AMP | — |
| `env.wt.delay` | ENV WT Delay | Sound Design / Mod / Envelope WT | — |
| `env.wt.attack` | ENV WT Attack | Sound Design / Mod / Envelope WT | — |
| `env.wt.hold` | ENV WT Hold | Sound Design / Mod / Envelope WT | — |
| `env.wt.decay` | ENV WT Decay | Sound Design / Mod / Envelope WT | — |
| `env.wt.sustain` | ENV WT Sustain | Sound Design / Mod / Envelope WT | — |
| `env.wt.release` | ENV WT Release | Sound Design / Mod / Envelope WT | — |
| `env.wt.curve` | ENV WT Curve | Sound Design / Mod / Envelope WT | — |
| `env.wt.loop` | ENV WT Loop | Sound Design / Mod / Envelope WT | — |
| `env.fm.delay` | ENV FM Delay | Sound Design / Mod / Envelope FM | — |
| `env.fm.attack` | ENV FM Attack | Sound Design / Mod / Envelope FM | — |
| `env.fm.hold` | ENV FM Hold | Sound Design / Mod / Envelope FM | — |
| `env.fm.decay` | ENV FM Decay | Sound Design / Mod / Envelope FM | — |
| `env.fm.sustain` | ENV FM Sustain | Sound Design / Mod / Envelope FM | — |
| `env.fm.release` | ENV FM Release | Sound Design / Mod / Envelope FM | — |
| `env.fm.curve` | ENV FM Curve | Sound Design / Mod / Envelope FM | — |
| `env.fm.loop` | ENV FM Loop | Sound Design / Mod / Envelope FM | — |
| `env.aux1.delay` | ENV AUX1 Delay | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux1.attack` | ENV AUX1 Attack | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux1.hold` | ENV AUX1 Hold | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux1.decay` | ENV AUX1 Decay | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux1.sustain` | ENV AUX1 Sustain | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux1.release` | ENV AUX1 Release | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux1.curve` | ENV AUX1 Curve | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux1.loop` | ENV AUX1 Loop | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux2.delay` | ENV AUX2 Delay | Sound Design / Mod / Envelope AUX2 | — |
| `env.aux2.attack` | ENV AUX2 Attack | Sound Design / Mod / Envelope AUX2 | — |
| `env.aux2.hold` | ENV AUX2 Hold | Sound Design / Mod / Envelope AUX2 | — |
| `env.aux2.decay` | ENV AUX2 Decay | Sound Design / Mod / Envelope AUX2 | — |
| `env.aux2.sustain` | ENV AUX2 Sustain | Sound Design / Mod / Envelope AUX2 | — |
| `env.aux2.release` | ENV AUX2 Release | Sound Design / Mod / Envelope AUX2 | — |
| `env.aux2.curve` | ENV AUX2 Curve | Sound Design / Mod / Envelope AUX2 | — |
| `env.aux2.loop` | ENV AUX2 Loop | Sound Design / Mod / Envelope AUX2 | — |
| `lfo1.shape` | LFO1 Shape | Sound Design / Mod / LFO1 | — |
| `lfo1.rate` | LFO1 Rate | Sound Design / Mod / LFO1 | — |
| `lfo1.sync` | LFO1 Tempo Sync | Sound Design / Mod / LFO1 | — |
| `lfo1.div` | LFO1 Division | Sound Design / Mod / LFO1 | — |
| `lfo1.phase` | LFO1 Phase | Sound Design / Mod / LFO1 | — |
| `lfo1.retrig` | LFO1 Retrigger | Sound Design / Mod / LFO1 | — |
| `lfo2.shape` | LFO2 Shape | Sound Design / Mod / LFO2 | — |
| `lfo2.rate` | LFO2 Rate | Sound Design / Mod / LFO2 | — |
| `lfo2.sync` | LFO2 Tempo Sync | Sound Design / Mod / LFO2 | — |
| `lfo2.div` | LFO2 Division | Sound Design / Mod / LFO2 | — |
| `lfo2.phase` | LFO2 Phase | Sound Design / Mod / LFO2 | — |
| `lfo2.retrig` | LFO2 Retrigger | Sound Design / Mod / LFO2 | — |
| `lfo3.shape` | LFO3 Shape | Sound Design / Mod / LFO3 | — |
| `lfo3.rate` | LFO3 Rate | Sound Design / Mod / LFO3 | — |
| `lfo3.sync` | LFO3 Tempo Sync | Sound Design / Mod / LFO3 | — |
| `lfo3.div` | LFO3 Division | Sound Design / Mod / LFO3 | — |
| `lfo3.phase` | LFO3 Phase | Sound Design / Mod / LFO3 | — |
| `lfo3.retrig` | LFO3 Retrigger | Sound Design / Mod / LFO3 | — |
| `lfo4.shape` | LFO4 Shape | Sound Design / Mod / LFO4 | — |
| `lfo4.rate` | LFO4 Rate | Sound Design / Mod / LFO4 | — |
| `lfo4.sync` | LFO4 Tempo Sync | Sound Design / Mod / LFO4 | — |
| `lfo4.div` | LFO4 Division | Sound Design / Mod / LFO4 | — |
| `lfo4.phase` | LFO4 Phase | Sound Design / Mod / LFO4 | — |
| `lfo4.retrig` | LFO4 Retrigger | Sound Design / Mod / LFO4 | — |
| `rnd.hold.rate` | Random Hold Rate | Sound Design / Mod / Random & Chaos | — |
| `rnd.smooth.rate` | Random Smooth Rate | Sound Design / Mod / Random & Chaos | — |
| `chaos.rate` | Chaos Rate | Sound Design / Mod / Random & Chaos | — |
| `fol1.attack` | Follower1 Attack | Sound Design / Mod / Followers | — |
| `fol1.release` | Follower1 Release | Sound Design / Mod / Followers | — |
| `fol2.attack` | Follower2 Attack | Sound Design / Mod / Followers | — |
| `fol2.release` | Follower2 Release | Sound Design / Mod / Followers | — |
| `fol3.attack` | Follower3 Attack | Sound Design / Mod / Followers | — |
| `fol3.release` | Follower3 Release | Sound Design / Mod / Followers | — |
| `wt.x` | WT X | Sound Design / WT / Direct | Performance |
| `wt.y` | WT Y | Sound Design / WT / Direct | Performance |
| `wt.z` | WT Z Master | Sound Design / WT / Direct | Performance |
| `wt.x_bound` | WT X Boundary | Sound Design / WT / Morph, Sync & Character | — |
| `wt.y_bound` | WT Y Boundary | Sound Design / WT / Morph, Sync & Character | — |
| `wt.x_morph` | WT X Morph | Sound Design / WT / Morph, Sync & Character | — |
| `wt.y_morph` | WT Y Morph | Sound Design / WT / Morph, Sync & Character | — |
| `wt.interp` | WT Interpolation | Sound Design / WT / Morph, Sync & Character | — |
| `wt.render` | WT Clean/Raw | Sound Design / WT / Morph, Sync & Character | — |
| `wt.coarse` | WT Coarse | Sound Design / WT / Pitch & Phase | — |
| `wt.fine` | WT Fine | Sound Design / WT / Pitch & Phase | — |
| `wt.phase_mode` | WT Phase Mode | Sound Design / WT / Pitch & Phase | — |
| `wt.phase` | WT Start Phase | Sound Design / WT / Pitch & Phase | — |
| `wt.sync` | WT Sync | Sound Design / WT / Morph, Sync & Character | — |
| `wt.sync_src` | WT Sync Source | Sound Design / WT / Morph, Sync & Character | — |
| `wt.char` | WT Character | Sound Design / WT / Direct | — |
| `wt.char_mode` | WT Character Profile | Sound Design / WT / Morph, Sync & Character | — |
| `wt.za.type` | WT ZA Transform | Sound Design / WT / Z Transforms | — |
| `wt.za.depth` | WT ZA Depth | Sound Design / WT / Z Transforms | — |
| `wt.za.curve` | WT ZA Curve | Sound Design / WT / Z Transforms | — |
| `wt.zb.type` | WT ZB Transform | Sound Design / WT / Z Transforms | — |
| `wt.zb.depth` | WT ZB Depth | Sound Design / WT / Z Transforms | — |
| `wt.zb.curve` | WT ZB Curve | Sound Design / WT / Z Transforms | — |
| `wt.z_order` | WT Z Order | Sound Design / WT / Z Transforms | — |
| `wt.z_fund` | WT Z Keep Fundamental | Sound Design / WT / Z Transforms | — |
| `wt.z_gain` | WT Z Gain Compensation | Sound Design / WT / Z Transforms | — |
| `op1.tune` | OP1 Tuning | Sound Design / FM / OP1 / Direct | — |
| `op1.fixed` | OP1 Fixed | Sound Design / FM / OP1 / Direct | — |
| `op1.offset` | OP1 Offset | Sound Design / FM / OP1 / Advanced | — |
| `op1.coarse` | OP1 Coarse | Sound Design / FM / OP1 / Advanced | — |
| `op1.fine` | OP1 Fine | Sound Design / FM / OP1 / Advanced | — |
| `op1.wave` | OP1 Waveform | Sound Design / FM / OP1 / Direct | — |
| `op1.wt_x` | OP1 WT X | Sound Design / FM / OP1 / Advanced | — |
| `op1.wt_y` | OP1 WT Y | Sound Design / FM / OP1 / Advanced | — |
| `op1.pan` | OP1 Pan | Sound Design / FM / OP1 / Advanced | — |
| `op1.mute` | OP1 Mute | Sound Design / FM / OP1 / Direct | — |
| `op1.enable` | OP1 Enable | Sound Design / FM / OP1 / Direct | — |
| `op1.char` | OP1 Character | Sound Design / FM / OP1 / Advanced | — |
| `op1.phase_mode` | OP1 Phase Mode | Sound Design / FM / OP1 / Advanced | — |
| `op1.ac` | OP1 Mod Coupling | Sound Design / FM / OP1 / Advanced | — |
| `op1.mode` | OP1 Mod Mode | Sound Design / FM / OP1 / Advanced | — |
| `op1.sync` | OP1 Sync | Sound Design / FM / OP1 / Advanced | — |
| `op2.tune` | OP2 Tuning | Sound Design / FM / OP2 / Direct | — |
| `op2.fixed` | OP2 Fixed | Sound Design / FM / OP2 / Direct | — |
| `op2.offset` | OP2 Offset | Sound Design / FM / OP2 / Advanced | — |
| `op2.coarse` | OP2 Coarse | Sound Design / FM / OP2 / Advanced | — |
| `op2.fine` | OP2 Fine | Sound Design / FM / OP2 / Advanced | — |
| `op2.wave` | OP2 Waveform | Sound Design / FM / OP2 / Direct | — |
| `op2.wt_x` | OP2 WT X | Sound Design / FM / OP2 / Advanced | — |
| `op2.wt_y` | OP2 WT Y | Sound Design / FM / OP2 / Advanced | — |
| `op2.pan` | OP2 Pan | Sound Design / FM / OP2 / Advanced | — |
| `op2.mute` | OP2 Mute | Sound Design / FM / OP2 / Direct | — |
| `op2.enable` | OP2 Enable | Sound Design / FM / OP2 / Direct | — |
| `op2.char` | OP2 Character | Sound Design / FM / OP2 / Advanced | — |
| `op2.phase_mode` | OP2 Phase Mode | Sound Design / FM / OP2 / Advanced | — |
| `op2.ac` | OP2 Mod Coupling | Sound Design / FM / OP2 / Advanced | — |
| `op2.mode` | OP2 Mod Mode | Sound Design / FM / OP2 / Advanced | — |
| `op2.sync` | OP2 Sync | Sound Design / FM / OP2 / Advanced | — |
| `op3.level` | OP3 Level | Sound Design / FM / OP3 / Direct | — |
| `op3.ratio` | OP3 Ratio | Sound Design / FM / OP3 / Direct | — |
| `op3.tune` | OP3 Tuning | Sound Design / FM / OP3 / Direct | — |
| `op3.fixed` | OP3 Fixed | Sound Design / FM / OP3 / Direct | — |
| `op3.offset` | OP3 Offset | Sound Design / FM / OP3 / Advanced | — |
| `op3.coarse` | OP3 Coarse | Sound Design / FM / OP3 / Advanced | — |
| `op3.fine` | OP3 Fine | Sound Design / FM / OP3 / Advanced | — |
| `op3.wave` | OP3 Waveform | Sound Design / FM / OP3 / Direct | — |
| `op3.wt_x` | OP3 WT X | Sound Design / FM / OP3 / Advanced | — |
| `op3.wt_y` | OP3 WT Y | Sound Design / FM / OP3 / Advanced | — |
| `op3.pan` | OP3 Pan | Sound Design / FM / OP3 / Advanced | — |
| `op3.mute` | OP3 Mute | Sound Design / FM / OP3 / Direct | — |
| `op3.enable` | OP3 Enable | Sound Design / FM / OP3 / Direct | — |
| `op3.char` | OP3 Character | Sound Design / FM / OP3 / Advanced | — |
| `op3.phase_mode` | OP3 Phase Mode | Sound Design / FM / OP3 / Advanced | — |
| `op3.ac` | OP3 Mod Coupling | Sound Design / FM / OP3 / Advanced | — |
| `op3.mode` | OP3 Mod Mode | Sound Design / FM / OP3 / Advanced | — |
| `op3.sync` | OP3 Sync | Sound Design / FM / OP3 / Advanced | — |
| `op4.level` | OP4 Level | Sound Design / FM / OP4 / Direct | — |
| `op4.ratio` | OP4 Ratio | Sound Design / FM / OP4 / Direct | — |
| `op4.tune` | OP4 Tuning | Sound Design / FM / OP4 / Direct | — |
| `op4.fixed` | OP4 Fixed | Sound Design / FM / OP4 / Direct | — |
| `op4.offset` | OP4 Offset | Sound Design / FM / OP4 / Advanced | — |
| `op4.coarse` | OP4 Coarse | Sound Design / FM / OP4 / Advanced | — |
| `op4.fine` | OP4 Fine | Sound Design / FM / OP4 / Advanced | — |
| `op4.wave` | OP4 Waveform | Sound Design / FM / OP4 / Direct | — |
| `op4.wt_x` | OP4 WT X | Sound Design / FM / OP4 / Advanced | — |
| `op4.wt_y` | OP4 WT Y | Sound Design / FM / OP4 / Advanced | — |
| `op4.pan` | OP4 Pan | Sound Design / FM / OP4 / Advanced | — |
| `op4.mute` | OP4 Mute | Sound Design / FM / OP4 / Direct | — |
| `op4.enable` | OP4 Enable | Sound Design / FM / OP4 / Direct | — |
| `op4.char` | OP4 Character | Sound Design / FM / OP4 / Advanced | — |
| `op4.phase_mode` | OP4 Phase Mode | Sound Design / FM / OP4 / Advanced | — |
| `op4.ac` | OP4 Mod Coupling | Sound Design / FM / OP4 / Advanced | — |
| `op4.mode` | OP4 Mod Mode | Sound Design / FM / OP4 / Advanced | — |
| `op4.sync` | OP4 Sync | Sound Design / FM / OP4 / Advanced | — |
| `fm.char_master` | FM Character Master | Sound Design / FM / Common | — |
| `fm.fb_mode` | FM Feedback Reset | Sound Design / FM / Common | — |
| `fm.op1_op3.depth` | FM OP1>OP3 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op1_op4.depth` | FM OP1>OP4 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op2_op3.depth` | FM OP2>OP3 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op2_op4.depth` | FM OP2>OP4 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op3_op1.depth` | FM OP3>OP1 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op3_op2.depth` | FM OP3>OP2 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op3_op3.depth` | FM OP3 Self | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op3_op4.depth` | FM OP3>OP4 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op4_op1.depth` | FM OP4>OP1 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op4_op2.depth` | FM OP4>OP2 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op4_op3.depth` | FM OP4>OP3 | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `fm.op4_op4.depth` | FM OP4 Self | Engineering / FM Network / Matrix 4x4 | Sound Design / FM / Matrix; Performance contextual candidate |
| `cross.wt_op3.depth` | CROSS WT>OP3 | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.wt_op4.depth` | CROSS WT>OP4 | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op3_wt.depth` | CROSS OP3>WT | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op4_wt.depth` | CROSS OP4>WT | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.bus_wt.depth` | CROSS BUS>WT | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op1_wt_freq.depth` | CROSS OP1>WT Freq | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op1_wt_x.depth` | CROSS OP1>WT X | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op1_wt_y.depth` | CROSS OP1>WT Y | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op1_wt_z.depth` | CROSS OP1>WT Z | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op1_wt_level.depth` | CROSS OP1>WT Level | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op1_wt_char.depth` | CROSS OP1>WT Char | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op2_wt_freq.depth` | CROSS OP2>WT Freq | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op2_wt_x.depth` | CROSS OP2>WT X | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op2_wt_y.depth` | CROSS OP2>WT Y | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op2_wt_z.depth` | CROSS OP2>WT Z | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op2_wt_level.depth` | CROSS OP2>WT Level | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op2_wt_char.depth` | CROSS OP2>WT Char | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op3_wt_freq.depth` | CROSS OP3>WT Freq | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op3_wt_x.depth` | CROSS OP3>WT X | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op3_wt_y.depth` | CROSS OP3>WT Y | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op3_wt_z.depth` | CROSS OP3>WT Z | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op3_wt_level.depth` | CROSS OP3>WT Level | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op3_wt_char.depth` | CROSS OP3>WT Char | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op4_wt_freq.depth` | CROSS OP4>WT Freq | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op4_wt_x.depth` | CROSS OP4>WT X | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op4_wt_y.depth` | CROSS OP4>WT Y | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op4_wt_z.depth` | CROSS OP4>WT Z | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op4_wt_level.depth` | CROSS OP4>WT Level | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.op4_wt_char.depth` | CROSS OP4>WT Char | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.bus_wt_freq.depth` | CROSS BUS>WT Freq | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.bus_wt_x.depth` | CROSS BUS>WT X | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.bus_wt_y.depth` | CROSS BUS>WT Y | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.bus_wt_z.depth` | CROSS BUS>WT Z | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.bus_wt_level.depth` | CROSS BUS>WT Level | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.bus_wt_char.depth` | CROSS BUS>WT Char | Engineering / Cross Network / Depths | Sound Design / CROSS / Network |
| `cross.wt_fmode` | CROSS WT Freq Mode | Sound Design / CROSS / Advanced | — |
| `cross.bus.w1` | CROSS Bus OP1 | Sound Design / CROSS / Bus | — |
| `cross.bus.w2` | CROSS Bus OP2 | Sound Design / CROSS / Bus | — |
| `cross.bus.w3` | CROSS Bus OP3 | Sound Design / CROSS / Bus | — |
| `cross.bus.w4` | CROSS Bus OP4 | Sound Design / CROSS / Bus | — |
| `cross.bus.mode` | CROSS Bus Mode | Sound Design / CROSS / Bus | — |
| `cross.dir` | CROSS Direction | Sound Design / CROSS / Advanced | — |
| `cross.audio.mode` | Cross Audio Mode | Sound Design / CROSS / Audio | — |
| `cross.audio.amount` | Cross Audio Amount | Sound Design / CROSS / Audio | — |
| `cross.audio.am_dir` | Cross AM Direction | Sound Design / CROSS / Audio | — |
| `flt.wt.type` | FLT WT Type | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.pos` | FLT WT Position | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.stereo` | FLT WT Stereo | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.f1_mode` | FLT WT F1 Mode | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.cutoff` | FLT WT Cutoff | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.res` | FLT WT Resonance | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.key` | FLT WT Keytrack | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.drive` | FLT WT Drive | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.bias` | FLT WT Bias | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.char` | FLT WT Character | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.f3_mode` | FLT WT F3 Mode | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.comb_fb` | FLT WT Comb Feedback | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.comb_damp` | FLT WT Comb Damping | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.detune` | FLT WT Detune | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.spread` | FLT WT Stereo Spread | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.modal` | FLT WT Modal Model | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.modes` | FLT WT Modes | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.decay` | FLT WT Decay | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.damp` | FLT WT Damping | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.bright` | FLT WT Brightness | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.inharm` | FLT WT Inharmonicity | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.freeze` | FLT WT Modal Freeze | Sound Design / Process / WT / Filter / Safety & Excitation | — |
| `flt.wt.exc` | FLT WT Excitation | Sound Design / Process / WT / Filter / Safety & Excitation | — |
| `flt.wt.exc_level` | FLT WT Excitation Level | Sound Design / Process / WT / Filter / Safety & Excitation | — |
| `flt.wt.mix` | FLT WT Dry/Wet | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.out` | FLT WT Output | Sound Design / Process / WT / Filter / Algorithm / Common | — |
| `flt.wt.arm` | FLT WT Autonomous Armed | Sound Design / Process / WT / Filter / Safety & Excitation | — |
| `flt.fm.type` | FLT FM Type | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.pos` | FLT FM Position | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.stereo` | FLT FM Stereo | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.f1_mode` | FLT FM F1 Mode | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.cutoff` | FLT FM Cutoff | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.res` | FLT FM Resonance | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.key` | FLT FM Keytrack | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.drive` | FLT FM Drive | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.bias` | FLT FM Bias | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.char` | FLT FM Character | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.f3_mode` | FLT FM F3 Mode | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.comb_fb` | FLT FM Comb Feedback | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.comb_damp` | FLT FM Comb Damping | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.detune` | FLT FM Detune | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.spread` | FLT FM Stereo Spread | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.modal` | FLT FM Modal Model | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.modes` | FLT FM Modes | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.decay` | FLT FM Decay | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.damp` | FLT FM Damping | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.bright` | FLT FM Brightness | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.inharm` | FLT FM Inharmonicity | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.freeze` | FLT FM Modal Freeze | Sound Design / Process / FM / Filter / Safety & Excitation | — |
| `flt.fm.exc` | FLT FM Excitation | Sound Design / Process / FM / Filter / Safety & Excitation | — |
| `flt.fm.exc_level` | FLT FM Excitation Level | Sound Design / Process / FM / Filter / Safety & Excitation | — |
| `flt.fm.mix` | FLT FM Dry/Wet | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.out` | FLT FM Output | Sound Design / Process / FM / Filter / Algorithm / Common | — |
| `flt.fm.arm` | FLT FM Autonomous Armed | Sound Design / Process / FM / Filter / Safety & Excitation | — |
| `flt.gl.type` | FLT GLOBAL Type | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.pos` | FLT GLOBAL Position | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.stereo` | FLT GLOBAL Stereo | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.f1_mode` | FLT GLOBAL F1 Mode | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.cutoff` | FLT GLOBAL Cutoff | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.res` | FLT GLOBAL Resonance | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.key` | FLT GLOBAL Keytrack | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.drive` | FLT GLOBAL Drive | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.bias` | FLT GLOBAL Bias | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.char` | FLT GLOBAL Character | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.f3_mode` | FLT GLOBAL F3 Mode | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.comb_fb` | FLT GLOBAL Comb Feedback | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.comb_damp` | FLT GLOBAL Comb Damping | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.detune` | FLT GLOBAL Detune | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.spread` | FLT GLOBAL Stereo Spread | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.modal` | FLT GLOBAL Modal Model | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.modes` | FLT GLOBAL Modes | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.decay` | FLT GLOBAL Decay | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.damp` | FLT GLOBAL Damping | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.bright` | FLT GLOBAL Brightness | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.inharm` | FLT GLOBAL Inharmonicity | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.freeze` | FLT GLOBAL Modal Freeze | Sound Design / Process / GL / Filter / Safety & Excitation | — |
| `flt.gl.exc` | FLT GLOBAL Excitation | Sound Design / Process / GL / Filter / Safety & Excitation | — |
| `flt.gl.exc_level` | FLT GLOBAL Excitation Level | Sound Design / Process / GL / Filter / Safety & Excitation | — |
| `flt.gl.mix` | FLT GLOBAL Dry/Wet | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.out` | FLT GLOBAL Output | Sound Design / Process / GL / Filter / Algorithm / Common | — |
| `flt.gl.arm` | FLT GLOBAL Autonomous Armed | Sound Design / Process / GL / Filter / Safety & Excitation | — |
| `dst.wt1.type` | DST WT1 Type | Sound Design / Process / WT1 / Common | — |
| `dst.wt1.mix` | DST WT1 Dry/Wet | Sound Design / Process / WT1 / Common | — |
| `dst.wt1.out` | DST WT1 Output | Sound Design / Process / WT1 / Common | — |
| `dst.wt1.arm` | DST WT1 Autonomous Armed | Sound Design / Process / WT1 / Common | — |
| `dst.wt1.n1.drive` | DST WT1 N1 Drive | Sound Design / Process / WT1 / N1 | — |
| `dst.wt1.n1.fold` | DST WT1 N1 Fold | Sound Design / Process / WT1 / N1 | — |
| `dst.wt1.n1.bias` | DST WT1 N1 Bias | Sound Design / Process / WT1 / N1 | — |
| `dst.wt1.n1.sym` | DST WT1 N1 Symmetry | Sound Design / Process / WT1 / N1 | — |
| `dst.wt1.n1.stages` | DST WT1 N1 Stages | Sound Design / Process / WT1 / N1 | — |
| `dst.wt1.n1.tone` | DST WT1 N1 Tone | Sound Design / Process / WT1 / N1 | — |
| `dst.wt1.d1.bits` | DST WT1 D1 Bits | Sound Design / Process / WT1 / D1 | — |
| `dst.wt1.d1.rate` | DST WT1 D1 Rate Ratio | Sound Design / Process / WT1 / D1 | — |
| `dst.wt1.d1.jitter` | DST WT1 D1 Jitter | Sound Design / Process / WT1 / D1 | — |
| `dst.wt1.d1.mode` | DST WT1 D1 Mode | Sound Design / Process / WT1 / D1 | — |
| `dst.wt1.d1.hold` | DST WT1 D1 Hold | Sound Design / Process / WT1 / D1 | — |
| `dst.wt1.y1.thresh` | DST WT1 Y1 Threshold | Sound Design / Process / WT1 / Y1 | — |
| `dst.wt1.y1.ratio` | DST WT1 Y1 Ratio | Sound Design / Process / WT1 / Y1 | — |
| `dst.wt1.y1.attack` | DST WT1 Y1 Attack | Sound Design / Process / WT1 / Y1 | — |
| `dst.wt1.y1.release` | DST WT1 Y1 Release | Sound Design / Process / WT1 / Y1 | — |
| `dst.wt1.y1.knee` | DST WT1 Y1 Knee | Sound Design / Process / WT1 / Y1 | — |
| `dst.wt1.y1.gate` | DST WT1 Y1 Gate | Sound Design / Process / WT1 / Y1 | — |
| `dst.wt1.y1.transient` | DST WT1 Y1 Transient | Sound Design / Process / WT1 / Y1 | — |
| `dst.wt1.y1.makeup` | DST WT1 Y1 Makeup | Sound Design / Process / WT1 / Y1 | — |
| `dst.wt1.s1.mode` | DST WT1 S1 Mode | Sound Design / Process / WT1 / S1 | — |
| `dst.wt1.s1.freq` | DST WT1 S1 Frequency | Sound Design / Process / WT1 / S1 | — |
| `dst.wt1.s1.shift` | DST WT1 S1 Shift | Sound Design / Process / WT1 / S1 | — |
| `dst.wt1.s1.carrier` | DST WT1 S1 Carrier | Sound Design / Process / WT1 / S1 | — |
| `dst.wt1.s1.phase` | DST WT1 S1 Phase | Sound Design / Process / WT1 / S1 | — |
| `dst.wt1.s1.stereo` | DST WT1 S1 Stereo Phase | Sound Design / Process / WT1 / S1 | — |
| `dst.wt1.n2.fb` | DST WT1 N2 Feedback | Sound Design / Process / WT1 / N2 | — |
| `dst.wt1.n2.delay` | DST WT1 N2 Delay | Sound Design / Process / WT1 / N2 | — |
| `dst.wt1.n2.damp` | DST WT1 N2 Damping | Sound Design / Process / WT1 / N2 | — |
| `dst.wt1.n2.bias` | DST WT1 N2 Bias | Sound Design / Process / WT1 / N2 | — |
| `dst.wt1.n2.res` | DST WT1 N2 Resonance | Sound Design / Process / WT1 / N2 | — |
| `dst.wt1.n2.selfosc` | DST WT1 N2 Self-Oscillation | Sound Design / Process / WT1 / N2 | — |
| `dst.wt1.d2.size` | DST WT1 D2 Size | Sound Design / Process / WT1 / D2 | — |
| `dst.wt1.d2.speed` | DST WT1 D2 Playback | Sound Design / Process / WT1 / D2 | — |
| `dst.wt1.d2.repeat` | DST WT1 D2 Repeat | Sound Design / Process / WT1 / D2 | — |
| `dst.wt1.d2.pos` | DST WT1 D2 Position | Sound Design / Process / WT1 / D2 | — |
| `dst.wt1.d2.fb` | DST WT1 D2 Feedback | Sound Design / Process / WT1 / D2 | — |
| `dst.wt1.d2.xfade` | DST WT1 D2 Crossfade | Sound Design / Process / WT1 / D2 | — |
| `dst.wt1.d2.freeze` | DST WT1 D2 Freeze | Sound Design / Process / WT1 / D2 | — |
| `dst.wt1.y2.attack` | DST WT1 Y2 Attack | Sound Design / Process / WT1 / Y2 | — |
| `dst.wt1.y2.release` | DST WT1 Y2 Release | Sound Design / Process / WT1 / Y2 | — |
| `dst.wt1.y2.sens` | DST WT1 Y2 Sensitivity | Sound Design / Process / WT1 / Y2 | — |
| `dst.wt1.y2.curve` | DST WT1 Y2 Curve | Sound Design / Process / WT1 / Y2 | — |
| `dst.wt1.y2.depth` | DST WT1 Y2 Depth | Sound Design / Process / WT1 / Y2 | — |
| `dst.wt1.y2.hold` | DST WT1 Y2 Hold | Sound Design / Process / WT1 / Y2 | — |
| `dst.wt1.y2.detect` | DST WT1 Y2 Detector | Sound Design / Process / WT1 / Y2 | — |
| `dst.wt1.s2.fft` | DST WT1 S2 FFT Size | Sound Design / Process / WT1 / S2 | — |
| `dst.wt1.s2.freeze` | DST WT1 S2 Freeze | Sound Design / Process / WT1 / S2 | — |
| `dst.wt1.s2.decay` | DST WT1 S2 Decay | Sound Design / Process / WT1 / S2 | — |
| `dst.wt1.s2.blur` | DST WT1 S2 Blur | Sound Design / Process / WT1 / S2 | — |
| `dst.wt1.s2.tilt` | DST WT1 S2 Tilt | Sound Design / Process / WT1 / S2 | — |
| `dst.wt1.s2.drift` | DST WT1 S2 Phase Drift | Sound Design / Process / WT1 / S2 | — |
| `dst.wt1.s2.shift` | DST WT1 S2 Shift | Sound Design / Process / WT1 / S2 | — |
| `dst.wt2.type` | DST WT2 Type | Sound Design / Process / WT2 / Common | — |
| `dst.wt2.mix` | DST WT2 Dry/Wet | Sound Design / Process / WT2 / Common | — |
| `dst.wt2.out` | DST WT2 Output | Sound Design / Process / WT2 / Common | — |
| `dst.wt2.arm` | DST WT2 Autonomous Armed | Sound Design / Process / WT2 / Common | — |
| `dst.wt2.n1.drive` | DST WT2 N1 Drive | Sound Design / Process / WT2 / N1 | — |
| `dst.wt2.n1.fold` | DST WT2 N1 Fold | Sound Design / Process / WT2 / N1 | — |
| `dst.wt2.n1.bias` | DST WT2 N1 Bias | Sound Design / Process / WT2 / N1 | — |
| `dst.wt2.n1.sym` | DST WT2 N1 Symmetry | Sound Design / Process / WT2 / N1 | — |
| `dst.wt2.n1.stages` | DST WT2 N1 Stages | Sound Design / Process / WT2 / N1 | — |
| `dst.wt2.n1.tone` | DST WT2 N1 Tone | Sound Design / Process / WT2 / N1 | — |
| `dst.wt2.d1.bits` | DST WT2 D1 Bits | Sound Design / Process / WT2 / D1 | — |
| `dst.wt2.d1.rate` | DST WT2 D1 Rate Ratio | Sound Design / Process / WT2 / D1 | — |
| `dst.wt2.d1.jitter` | DST WT2 D1 Jitter | Sound Design / Process / WT2 / D1 | — |
| `dst.wt2.d1.mode` | DST WT2 D1 Mode | Sound Design / Process / WT2 / D1 | — |
| `dst.wt2.d1.hold` | DST WT2 D1 Hold | Sound Design / Process / WT2 / D1 | — |
| `dst.wt2.y1.thresh` | DST WT2 Y1 Threshold | Sound Design / Process / WT2 / Y1 | — |
| `dst.wt2.y1.ratio` | DST WT2 Y1 Ratio | Sound Design / Process / WT2 / Y1 | — |
| `dst.wt2.y1.attack` | DST WT2 Y1 Attack | Sound Design / Process / WT2 / Y1 | — |
| `dst.wt2.y1.release` | DST WT2 Y1 Release | Sound Design / Process / WT2 / Y1 | — |
| `dst.wt2.y1.knee` | DST WT2 Y1 Knee | Sound Design / Process / WT2 / Y1 | — |
| `dst.wt2.y1.gate` | DST WT2 Y1 Gate | Sound Design / Process / WT2 / Y1 | — |
| `dst.wt2.y1.transient` | DST WT2 Y1 Transient | Sound Design / Process / WT2 / Y1 | — |
| `dst.wt2.y1.makeup` | DST WT2 Y1 Makeup | Sound Design / Process / WT2 / Y1 | — |
| `dst.wt2.s1.mode` | DST WT2 S1 Mode | Sound Design / Process / WT2 / S1 | — |
| `dst.wt2.s1.freq` | DST WT2 S1 Frequency | Sound Design / Process / WT2 / S1 | — |
| `dst.wt2.s1.shift` | DST WT2 S1 Shift | Sound Design / Process / WT2 / S1 | — |
| `dst.wt2.s1.carrier` | DST WT2 S1 Carrier | Sound Design / Process / WT2 / S1 | — |
| `dst.wt2.s1.phase` | DST WT2 S1 Phase | Sound Design / Process / WT2 / S1 | — |
| `dst.wt2.s1.stereo` | DST WT2 S1 Stereo Phase | Sound Design / Process / WT2 / S1 | — |
| `dst.wt2.n2.fb` | DST WT2 N2 Feedback | Sound Design / Process / WT2 / N2 | — |
| `dst.wt2.n2.delay` | DST WT2 N2 Delay | Sound Design / Process / WT2 / N2 | — |
| `dst.wt2.n2.damp` | DST WT2 N2 Damping | Sound Design / Process / WT2 / N2 | — |
| `dst.wt2.n2.bias` | DST WT2 N2 Bias | Sound Design / Process / WT2 / N2 | — |
| `dst.wt2.n2.res` | DST WT2 N2 Resonance | Sound Design / Process / WT2 / N2 | — |
| `dst.wt2.n2.selfosc` | DST WT2 N2 Self-Oscillation | Sound Design / Process / WT2 / N2 | — |
| `dst.wt2.d2.size` | DST WT2 D2 Size | Sound Design / Process / WT2 / D2 | — |
| `dst.wt2.d2.speed` | DST WT2 D2 Playback | Sound Design / Process / WT2 / D2 | — |
| `dst.wt2.d2.repeat` | DST WT2 D2 Repeat | Sound Design / Process / WT2 / D2 | — |
| `dst.wt2.d2.pos` | DST WT2 D2 Position | Sound Design / Process / WT2 / D2 | — |
| `dst.wt2.d2.fb` | DST WT2 D2 Feedback | Sound Design / Process / WT2 / D2 | — |
| `dst.wt2.d2.xfade` | DST WT2 D2 Crossfade | Sound Design / Process / WT2 / D2 | — |
| `dst.wt2.d2.freeze` | DST WT2 D2 Freeze | Sound Design / Process / WT2 / D2 | — |
| `dst.wt2.y2.attack` | DST WT2 Y2 Attack | Sound Design / Process / WT2 / Y2 | — |
| `dst.wt2.y2.release` | DST WT2 Y2 Release | Sound Design / Process / WT2 / Y2 | — |
| `dst.wt2.y2.sens` | DST WT2 Y2 Sensitivity | Sound Design / Process / WT2 / Y2 | — |
| `dst.wt2.y2.curve` | DST WT2 Y2 Curve | Sound Design / Process / WT2 / Y2 | — |
| `dst.wt2.y2.depth` | DST WT2 Y2 Depth | Sound Design / Process / WT2 / Y2 | — |
| `dst.wt2.y2.hold` | DST WT2 Y2 Hold | Sound Design / Process / WT2 / Y2 | — |
| `dst.wt2.y2.detect` | DST WT2 Y2 Detector | Sound Design / Process / WT2 / Y2 | — |
| `dst.wt2.s2.fft` | DST WT2 S2 FFT Size | Sound Design / Process / WT2 / S2 | — |
| `dst.wt2.s2.freeze` | DST WT2 S2 Freeze | Sound Design / Process / WT2 / S2 | — |
| `dst.wt2.s2.decay` | DST WT2 S2 Decay | Sound Design / Process / WT2 / S2 | — |
| `dst.wt2.s2.blur` | DST WT2 S2 Blur | Sound Design / Process / WT2 / S2 | — |
| `dst.wt2.s2.tilt` | DST WT2 S2 Tilt | Sound Design / Process / WT2 / S2 | — |
| `dst.wt2.s2.drift` | DST WT2 S2 Phase Drift | Sound Design / Process / WT2 / S2 | — |
| `dst.wt2.s2.shift` | DST WT2 S2 Shift | Sound Design / Process / WT2 / S2 | — |
| `dst.fm1.type` | DST FM1 Type | Sound Design / Process / FM1 / Common | — |
| `dst.fm1.mix` | DST FM1 Dry/Wet | Sound Design / Process / FM1 / Common | — |
| `dst.fm1.out` | DST FM1 Output | Sound Design / Process / FM1 / Common | — |
| `dst.fm1.arm` | DST FM1 Autonomous Armed | Sound Design / Process / FM1 / Common | — |
| `dst.fm1.n1.drive` | DST FM1 N1 Drive | Sound Design / Process / FM1 / N1 | — |
| `dst.fm1.n1.fold` | DST FM1 N1 Fold | Sound Design / Process / FM1 / N1 | — |
| `dst.fm1.n1.bias` | DST FM1 N1 Bias | Sound Design / Process / FM1 / N1 | — |
| `dst.fm1.n1.sym` | DST FM1 N1 Symmetry | Sound Design / Process / FM1 / N1 | — |
| `dst.fm1.n1.stages` | DST FM1 N1 Stages | Sound Design / Process / FM1 / N1 | — |
| `dst.fm1.n1.tone` | DST FM1 N1 Tone | Sound Design / Process / FM1 / N1 | — |
| `dst.fm1.d1.bits` | DST FM1 D1 Bits | Sound Design / Process / FM1 / D1 | — |
| `dst.fm1.d1.rate` | DST FM1 D1 Rate Ratio | Sound Design / Process / FM1 / D1 | — |
| `dst.fm1.d1.jitter` | DST FM1 D1 Jitter | Sound Design / Process / FM1 / D1 | — |
| `dst.fm1.d1.mode` | DST FM1 D1 Mode | Sound Design / Process / FM1 / D1 | — |
| `dst.fm1.d1.hold` | DST FM1 D1 Hold | Sound Design / Process / FM1 / D1 | — |
| `dst.fm1.y1.thresh` | DST FM1 Y1 Threshold | Sound Design / Process / FM1 / Y1 | — |
| `dst.fm1.y1.ratio` | DST FM1 Y1 Ratio | Sound Design / Process / FM1 / Y1 | — |
| `dst.fm1.y1.attack` | DST FM1 Y1 Attack | Sound Design / Process / FM1 / Y1 | — |
| `dst.fm1.y1.release` | DST FM1 Y1 Release | Sound Design / Process / FM1 / Y1 | — |
| `dst.fm1.y1.knee` | DST FM1 Y1 Knee | Sound Design / Process / FM1 / Y1 | — |
| `dst.fm1.y1.gate` | DST FM1 Y1 Gate | Sound Design / Process / FM1 / Y1 | — |
| `dst.fm1.y1.transient` | DST FM1 Y1 Transient | Sound Design / Process / FM1 / Y1 | — |
| `dst.fm1.y1.makeup` | DST FM1 Y1 Makeup | Sound Design / Process / FM1 / Y1 | — |
| `dst.fm1.s1.mode` | DST FM1 S1 Mode | Sound Design / Process / FM1 / S1 | — |
| `dst.fm1.s1.freq` | DST FM1 S1 Frequency | Sound Design / Process / FM1 / S1 | — |
| `dst.fm1.s1.shift` | DST FM1 S1 Shift | Sound Design / Process / FM1 / S1 | — |
| `dst.fm1.s1.carrier` | DST FM1 S1 Carrier | Sound Design / Process / FM1 / S1 | — |
| `dst.fm1.s1.phase` | DST FM1 S1 Phase | Sound Design / Process / FM1 / S1 | — |
| `dst.fm1.s1.stereo` | DST FM1 S1 Stereo Phase | Sound Design / Process / FM1 / S1 | — |
| `dst.fm1.n2.fb` | DST FM1 N2 Feedback | Sound Design / Process / FM1 / N2 | — |
| `dst.fm1.n2.delay` | DST FM1 N2 Delay | Sound Design / Process / FM1 / N2 | — |
| `dst.fm1.n2.damp` | DST FM1 N2 Damping | Sound Design / Process / FM1 / N2 | — |
| `dst.fm1.n2.bias` | DST FM1 N2 Bias | Sound Design / Process / FM1 / N2 | — |
| `dst.fm1.n2.res` | DST FM1 N2 Resonance | Sound Design / Process / FM1 / N2 | — |
| `dst.fm1.n2.selfosc` | DST FM1 N2 Self-Oscillation | Sound Design / Process / FM1 / N2 | — |
| `dst.fm1.d2.size` | DST FM1 D2 Size | Sound Design / Process / FM1 / D2 | — |
| `dst.fm1.d2.speed` | DST FM1 D2 Playback | Sound Design / Process / FM1 / D2 | — |
| `dst.fm1.d2.repeat` | DST FM1 D2 Repeat | Sound Design / Process / FM1 / D2 | — |
| `dst.fm1.d2.pos` | DST FM1 D2 Position | Sound Design / Process / FM1 / D2 | — |
| `dst.fm1.d2.fb` | DST FM1 D2 Feedback | Sound Design / Process / FM1 / D2 | — |
| `dst.fm1.d2.xfade` | DST FM1 D2 Crossfade | Sound Design / Process / FM1 / D2 | — |
| `dst.fm1.d2.freeze` | DST FM1 D2 Freeze | Sound Design / Process / FM1 / D2 | — |
| `dst.fm1.y2.attack` | DST FM1 Y2 Attack | Sound Design / Process / FM1 / Y2 | — |
| `dst.fm1.y2.release` | DST FM1 Y2 Release | Sound Design / Process / FM1 / Y2 | — |
| `dst.fm1.y2.sens` | DST FM1 Y2 Sensitivity | Sound Design / Process / FM1 / Y2 | — |
| `dst.fm1.y2.curve` | DST FM1 Y2 Curve | Sound Design / Process / FM1 / Y2 | — |
| `dst.fm1.y2.depth` | DST FM1 Y2 Depth | Sound Design / Process / FM1 / Y2 | — |
| `dst.fm1.y2.hold` | DST FM1 Y2 Hold | Sound Design / Process / FM1 / Y2 | — |
| `dst.fm1.y2.detect` | DST FM1 Y2 Detector | Sound Design / Process / FM1 / Y2 | — |
| `dst.fm1.s2.fft` | DST FM1 S2 FFT Size | Sound Design / Process / FM1 / S2 | — |
| `dst.fm1.s2.freeze` | DST FM1 S2 Freeze | Sound Design / Process / FM1 / S2 | — |
| `dst.fm1.s2.decay` | DST FM1 S2 Decay | Sound Design / Process / FM1 / S2 | — |
| `dst.fm1.s2.blur` | DST FM1 S2 Blur | Sound Design / Process / FM1 / S2 | — |
| `dst.fm1.s2.tilt` | DST FM1 S2 Tilt | Sound Design / Process / FM1 / S2 | — |
| `dst.fm1.s2.drift` | DST FM1 S2 Phase Drift | Sound Design / Process / FM1 / S2 | — |
| `dst.fm1.s2.shift` | DST FM1 S2 Shift | Sound Design / Process / FM1 / S2 | — |
| `dst.fm2.type` | DST FM2 Type | Sound Design / Process / FM2 / Common | — |
| `dst.fm2.mix` | DST FM2 Dry/Wet | Sound Design / Process / FM2 / Common | — |
| `dst.fm2.out` | DST FM2 Output | Sound Design / Process / FM2 / Common | — |
| `dst.fm2.arm` | DST FM2 Autonomous Armed | Sound Design / Process / FM2 / Common | — |
| `dst.fm2.n1.drive` | DST FM2 N1 Drive | Sound Design / Process / FM2 / N1 | — |
| `dst.fm2.n1.fold` | DST FM2 N1 Fold | Sound Design / Process / FM2 / N1 | — |
| `dst.fm2.n1.bias` | DST FM2 N1 Bias | Sound Design / Process / FM2 / N1 | — |
| `dst.fm2.n1.sym` | DST FM2 N1 Symmetry | Sound Design / Process / FM2 / N1 | — |
| `dst.fm2.n1.stages` | DST FM2 N1 Stages | Sound Design / Process / FM2 / N1 | — |
| `dst.fm2.n1.tone` | DST FM2 N1 Tone | Sound Design / Process / FM2 / N1 | — |
| `dst.fm2.d1.bits` | DST FM2 D1 Bits | Sound Design / Process / FM2 / D1 | — |
| `dst.fm2.d1.rate` | DST FM2 D1 Rate Ratio | Sound Design / Process / FM2 / D1 | — |
| `dst.fm2.d1.jitter` | DST FM2 D1 Jitter | Sound Design / Process / FM2 / D1 | — |
| `dst.fm2.d1.mode` | DST FM2 D1 Mode | Sound Design / Process / FM2 / D1 | — |
| `dst.fm2.d1.hold` | DST FM2 D1 Hold | Sound Design / Process / FM2 / D1 | — |
| `dst.fm2.y1.thresh` | DST FM2 Y1 Threshold | Sound Design / Process / FM2 / Y1 | — |
| `dst.fm2.y1.ratio` | DST FM2 Y1 Ratio | Sound Design / Process / FM2 / Y1 | — |
| `dst.fm2.y1.attack` | DST FM2 Y1 Attack | Sound Design / Process / FM2 / Y1 | — |
| `dst.fm2.y1.release` | DST FM2 Y1 Release | Sound Design / Process / FM2 / Y1 | — |
| `dst.fm2.y1.knee` | DST FM2 Y1 Knee | Sound Design / Process / FM2 / Y1 | — |
| `dst.fm2.y1.gate` | DST FM2 Y1 Gate | Sound Design / Process / FM2 / Y1 | — |
| `dst.fm2.y1.transient` | DST FM2 Y1 Transient | Sound Design / Process / FM2 / Y1 | — |
| `dst.fm2.y1.makeup` | DST FM2 Y1 Makeup | Sound Design / Process / FM2 / Y1 | — |
| `dst.fm2.s1.mode` | DST FM2 S1 Mode | Sound Design / Process / FM2 / S1 | — |
| `dst.fm2.s1.freq` | DST FM2 S1 Frequency | Sound Design / Process / FM2 / S1 | — |
| `dst.fm2.s1.shift` | DST FM2 S1 Shift | Sound Design / Process / FM2 / S1 | — |
| `dst.fm2.s1.carrier` | DST FM2 S1 Carrier | Sound Design / Process / FM2 / S1 | — |
| `dst.fm2.s1.phase` | DST FM2 S1 Phase | Sound Design / Process / FM2 / S1 | — |
| `dst.fm2.s1.stereo` | DST FM2 S1 Stereo Phase | Sound Design / Process / FM2 / S1 | — |
| `dst.fm2.n2.fb` | DST FM2 N2 Feedback | Sound Design / Process / FM2 / N2 | — |
| `dst.fm2.n2.delay` | DST FM2 N2 Delay | Sound Design / Process / FM2 / N2 | — |
| `dst.fm2.n2.damp` | DST FM2 N2 Damping | Sound Design / Process / FM2 / N2 | — |
| `dst.fm2.n2.bias` | DST FM2 N2 Bias | Sound Design / Process / FM2 / N2 | — |
| `dst.fm2.n2.res` | DST FM2 N2 Resonance | Sound Design / Process / FM2 / N2 | — |
| `dst.fm2.n2.selfosc` | DST FM2 N2 Self-Oscillation | Sound Design / Process / FM2 / N2 | — |
| `dst.fm2.d2.size` | DST FM2 D2 Size | Sound Design / Process / FM2 / D2 | — |
| `dst.fm2.d2.speed` | DST FM2 D2 Playback | Sound Design / Process / FM2 / D2 | — |
| `dst.fm2.d2.repeat` | DST FM2 D2 Repeat | Sound Design / Process / FM2 / D2 | — |
| `dst.fm2.d2.pos` | DST FM2 D2 Position | Sound Design / Process / FM2 / D2 | — |
| `dst.fm2.d2.fb` | DST FM2 D2 Feedback | Sound Design / Process / FM2 / D2 | — |
| `dst.fm2.d2.xfade` | DST FM2 D2 Crossfade | Sound Design / Process / FM2 / D2 | — |
| `dst.fm2.d2.freeze` | DST FM2 D2 Freeze | Sound Design / Process / FM2 / D2 | — |
| `dst.fm2.y2.attack` | DST FM2 Y2 Attack | Sound Design / Process / FM2 / Y2 | — |
| `dst.fm2.y2.release` | DST FM2 Y2 Release | Sound Design / Process / FM2 / Y2 | — |
| `dst.fm2.y2.sens` | DST FM2 Y2 Sensitivity | Sound Design / Process / FM2 / Y2 | — |
| `dst.fm2.y2.curve` | DST FM2 Y2 Curve | Sound Design / Process / FM2 / Y2 | — |
| `dst.fm2.y2.depth` | DST FM2 Y2 Depth | Sound Design / Process / FM2 / Y2 | — |
| `dst.fm2.y2.hold` | DST FM2 Y2 Hold | Sound Design / Process / FM2 / Y2 | — |
| `dst.fm2.y2.detect` | DST FM2 Y2 Detector | Sound Design / Process / FM2 / Y2 | — |
| `dst.fm2.s2.fft` | DST FM2 S2 FFT Size | Sound Design / Process / FM2 / S2 | — |
| `dst.fm2.s2.freeze` | DST FM2 S2 Freeze | Sound Design / Process / FM2 / S2 | — |
| `dst.fm2.s2.decay` | DST FM2 S2 Decay | Sound Design / Process / FM2 / S2 | — |
| `dst.fm2.s2.blur` | DST FM2 S2 Blur | Sound Design / Process / FM2 / S2 | — |
| `dst.fm2.s2.tilt` | DST FM2 S2 Tilt | Sound Design / Process / FM2 / S2 | — |
| `dst.fm2.s2.drift` | DST FM2 S2 Phase Drift | Sound Design / Process / FM2 / S2 | — |
| `dst.fm2.s2.shift` | DST FM2 S2 Shift | Sound Design / Process / FM2 / S2 | — |
| `dst.gl1.type` | DST GL1 Type | Sound Design / Process / GL1 / Common | — |
| `dst.gl1.mix` | DST GL1 Dry/Wet | Sound Design / Process / GL1 / Common | — |
| `dst.gl1.out` | DST GL1 Output | Sound Design / Process / GL1 / Common | — |
| `dst.gl1.arm` | DST GL1 Autonomous Armed | Sound Design / Process / GL1 / Common | — |
| `dst.gl1.n1.drive` | DST GL1 N1 Drive | Sound Design / Process / GL1 / N1 | — |
| `dst.gl1.n1.fold` | DST GL1 N1 Fold | Sound Design / Process / GL1 / N1 | — |
| `dst.gl1.n1.bias` | DST GL1 N1 Bias | Sound Design / Process / GL1 / N1 | — |
| `dst.gl1.n1.sym` | DST GL1 N1 Symmetry | Sound Design / Process / GL1 / N1 | — |
| `dst.gl1.n1.stages` | DST GL1 N1 Stages | Sound Design / Process / GL1 / N1 | — |
| `dst.gl1.n1.tone` | DST GL1 N1 Tone | Sound Design / Process / GL1 / N1 | — |
| `dst.gl1.d1.bits` | DST GL1 D1 Bits | Sound Design / Process / GL1 / D1 | — |
| `dst.gl1.d1.rate` | DST GL1 D1 Rate Ratio | Sound Design / Process / GL1 / D1 | — |
| `dst.gl1.d1.jitter` | DST GL1 D1 Jitter | Sound Design / Process / GL1 / D1 | — |
| `dst.gl1.d1.mode` | DST GL1 D1 Mode | Sound Design / Process / GL1 / D1 | — |
| `dst.gl1.d1.hold` | DST GL1 D1 Hold | Sound Design / Process / GL1 / D1 | — |
| `dst.gl1.y1.thresh` | DST GL1 Y1 Threshold | Sound Design / Process / GL1 / Y1 | — |
| `dst.gl1.y1.ratio` | DST GL1 Y1 Ratio | Sound Design / Process / GL1 / Y1 | — |
| `dst.gl1.y1.attack` | DST GL1 Y1 Attack | Sound Design / Process / GL1 / Y1 | — |
| `dst.gl1.y1.release` | DST GL1 Y1 Release | Sound Design / Process / GL1 / Y1 | — |
| `dst.gl1.y1.knee` | DST GL1 Y1 Knee | Sound Design / Process / GL1 / Y1 | — |
| `dst.gl1.y1.gate` | DST GL1 Y1 Gate | Sound Design / Process / GL1 / Y1 | — |
| `dst.gl1.y1.transient` | DST GL1 Y1 Transient | Sound Design / Process / GL1 / Y1 | — |
| `dst.gl1.y1.makeup` | DST GL1 Y1 Makeup | Sound Design / Process / GL1 / Y1 | — |
| `dst.gl1.s1.mode` | DST GL1 S1 Mode | Sound Design / Process / GL1 / S1 | — |
| `dst.gl1.s1.freq` | DST GL1 S1 Frequency | Sound Design / Process / GL1 / S1 | — |
| `dst.gl1.s1.shift` | DST GL1 S1 Shift | Sound Design / Process / GL1 / S1 | — |
| `dst.gl1.s1.carrier` | DST GL1 S1 Carrier | Sound Design / Process / GL1 / S1 | — |
| `dst.gl1.s1.phase` | DST GL1 S1 Phase | Sound Design / Process / GL1 / S1 | — |
| `dst.gl1.s1.stereo` | DST GL1 S1 Stereo Phase | Sound Design / Process / GL1 / S1 | — |
| `dst.gl1.n2.fb` | DST GL1 N2 Feedback | Sound Design / Process / GL1 / N2 | — |
| `dst.gl1.n2.delay` | DST GL1 N2 Delay | Sound Design / Process / GL1 / N2 | — |
| `dst.gl1.n2.damp` | DST GL1 N2 Damping | Sound Design / Process / GL1 / N2 | — |
| `dst.gl1.n2.bias` | DST GL1 N2 Bias | Sound Design / Process / GL1 / N2 | — |
| `dst.gl1.n2.res` | DST GL1 N2 Resonance | Sound Design / Process / GL1 / N2 | — |
| `dst.gl1.n2.selfosc` | DST GL1 N2 Self-Oscillation | Sound Design / Process / GL1 / N2 | — |
| `dst.gl1.d2.size` | DST GL1 D2 Size | Sound Design / Process / GL1 / D2 | — |
| `dst.gl1.d2.speed` | DST GL1 D2 Playback | Sound Design / Process / GL1 / D2 | — |
| `dst.gl1.d2.repeat` | DST GL1 D2 Repeat | Sound Design / Process / GL1 / D2 | — |
| `dst.gl1.d2.pos` | DST GL1 D2 Position | Sound Design / Process / GL1 / D2 | — |
| `dst.gl1.d2.fb` | DST GL1 D2 Feedback | Sound Design / Process / GL1 / D2 | — |
| `dst.gl1.d2.xfade` | DST GL1 D2 Crossfade | Sound Design / Process / GL1 / D2 | — |
| `dst.gl1.d2.freeze` | DST GL1 D2 Freeze | Sound Design / Process / GL1 / D2 | — |
| `dst.gl1.y2.attack` | DST GL1 Y2 Attack | Sound Design / Process / GL1 / Y2 | — |
| `dst.gl1.y2.release` | DST GL1 Y2 Release | Sound Design / Process / GL1 / Y2 | — |
| `dst.gl1.y2.sens` | DST GL1 Y2 Sensitivity | Sound Design / Process / GL1 / Y2 | — |
| `dst.gl1.y2.curve` | DST GL1 Y2 Curve | Sound Design / Process / GL1 / Y2 | — |
| `dst.gl1.y2.depth` | DST GL1 Y2 Depth | Sound Design / Process / GL1 / Y2 | — |
| `dst.gl1.y2.hold` | DST GL1 Y2 Hold | Sound Design / Process / GL1 / Y2 | — |
| `dst.gl1.y2.detect` | DST GL1 Y2 Detector | Sound Design / Process / GL1 / Y2 | — |
| `dst.gl1.s2.fft` | DST GL1 S2 FFT Size | Sound Design / Process / GL1 / S2 | — |
| `dst.gl1.s2.freeze` | DST GL1 S2 Freeze | Sound Design / Process / GL1 / S2 | — |
| `dst.gl1.s2.decay` | DST GL1 S2 Decay | Sound Design / Process / GL1 / S2 | — |
| `dst.gl1.s2.blur` | DST GL1 S2 Blur | Sound Design / Process / GL1 / S2 | — |
| `dst.gl1.s2.tilt` | DST GL1 S2 Tilt | Sound Design / Process / GL1 / S2 | — |
| `dst.gl1.s2.drift` | DST GL1 S2 Phase Drift | Sound Design / Process / GL1 / S2 | — |
| `dst.gl1.s2.shift` | DST GL1 S2 Shift | Sound Design / Process / GL1 / S2 | — |
| `dst.gl2.type` | DST GL2 Type | Sound Design / Process / GL2 / Common | — |
| `dst.gl2.mix` | DST GL2 Dry/Wet | Sound Design / Process / GL2 / Common | — |
| `dst.gl2.out` | DST GL2 Output | Sound Design / Process / GL2 / Common | — |
| `dst.gl2.arm` | DST GL2 Autonomous Armed | Sound Design / Process / GL2 / Common | — |
| `dst.gl2.n1.drive` | DST GL2 N1 Drive | Sound Design / Process / GL2 / N1 | — |
| `dst.gl2.n1.fold` | DST GL2 N1 Fold | Sound Design / Process / GL2 / N1 | — |
| `dst.gl2.n1.bias` | DST GL2 N1 Bias | Sound Design / Process / GL2 / N1 | — |
| `dst.gl2.n1.sym` | DST GL2 N1 Symmetry | Sound Design / Process / GL2 / N1 | — |
| `dst.gl2.n1.stages` | DST GL2 N1 Stages | Sound Design / Process / GL2 / N1 | — |
| `dst.gl2.n1.tone` | DST GL2 N1 Tone | Sound Design / Process / GL2 / N1 | — |
| `dst.gl2.d1.bits` | DST GL2 D1 Bits | Sound Design / Process / GL2 / D1 | — |
| `dst.gl2.d1.rate` | DST GL2 D1 Rate Ratio | Sound Design / Process / GL2 / D1 | — |
| `dst.gl2.d1.jitter` | DST GL2 D1 Jitter | Sound Design / Process / GL2 / D1 | — |
| `dst.gl2.d1.mode` | DST GL2 D1 Mode | Sound Design / Process / GL2 / D1 | — |
| `dst.gl2.d1.hold` | DST GL2 D1 Hold | Sound Design / Process / GL2 / D1 | — |
| `dst.gl2.y1.thresh` | DST GL2 Y1 Threshold | Sound Design / Process / GL2 / Y1 | — |
| `dst.gl2.y1.ratio` | DST GL2 Y1 Ratio | Sound Design / Process / GL2 / Y1 | — |
| `dst.gl2.y1.attack` | DST GL2 Y1 Attack | Sound Design / Process / GL2 / Y1 | — |
| `dst.gl2.y1.release` | DST GL2 Y1 Release | Sound Design / Process / GL2 / Y1 | — |
| `dst.gl2.y1.knee` | DST GL2 Y1 Knee | Sound Design / Process / GL2 / Y1 | — |
| `dst.gl2.y1.gate` | DST GL2 Y1 Gate | Sound Design / Process / GL2 / Y1 | — |
| `dst.gl2.y1.transient` | DST GL2 Y1 Transient | Sound Design / Process / GL2 / Y1 | — |
| `dst.gl2.y1.makeup` | DST GL2 Y1 Makeup | Sound Design / Process / GL2 / Y1 | — |
| `dst.gl2.s1.mode` | DST GL2 S1 Mode | Sound Design / Process / GL2 / S1 | — |
| `dst.gl2.s1.freq` | DST GL2 S1 Frequency | Sound Design / Process / GL2 / S1 | — |
| `dst.gl2.s1.shift` | DST GL2 S1 Shift | Sound Design / Process / GL2 / S1 | — |
| `dst.gl2.s1.carrier` | DST GL2 S1 Carrier | Sound Design / Process / GL2 / S1 | — |
| `dst.gl2.s1.phase` | DST GL2 S1 Phase | Sound Design / Process / GL2 / S1 | — |
| `dst.gl2.s1.stereo` | DST GL2 S1 Stereo Phase | Sound Design / Process / GL2 / S1 | — |
| `dst.gl2.n2.fb` | DST GL2 N2 Feedback | Sound Design / Process / GL2 / N2 | — |
| `dst.gl2.n2.delay` | DST GL2 N2 Delay | Sound Design / Process / GL2 / N2 | — |
| `dst.gl2.n2.damp` | DST GL2 N2 Damping | Sound Design / Process / GL2 / N2 | — |
| `dst.gl2.n2.bias` | DST GL2 N2 Bias | Sound Design / Process / GL2 / N2 | — |
| `dst.gl2.n2.res` | DST GL2 N2 Resonance | Sound Design / Process / GL2 / N2 | — |
| `dst.gl2.n2.selfosc` | DST GL2 N2 Self-Oscillation | Sound Design / Process / GL2 / N2 | — |
| `dst.gl2.d2.size` | DST GL2 D2 Size | Sound Design / Process / GL2 / D2 | — |
| `dst.gl2.d2.speed` | DST GL2 D2 Playback | Sound Design / Process / GL2 / D2 | — |
| `dst.gl2.d2.repeat` | DST GL2 D2 Repeat | Sound Design / Process / GL2 / D2 | — |
| `dst.gl2.d2.pos` | DST GL2 D2 Position | Sound Design / Process / GL2 / D2 | — |
| `dst.gl2.d2.fb` | DST GL2 D2 Feedback | Sound Design / Process / GL2 / D2 | — |
| `dst.gl2.d2.xfade` | DST GL2 D2 Crossfade | Sound Design / Process / GL2 / D2 | — |
| `dst.gl2.d2.freeze` | DST GL2 D2 Freeze | Sound Design / Process / GL2 / D2 | — |
| `dst.gl2.y2.attack` | DST GL2 Y2 Attack | Sound Design / Process / GL2 / Y2 | — |
| `dst.gl2.y2.release` | DST GL2 Y2 Release | Sound Design / Process / GL2 / Y2 | — |
| `dst.gl2.y2.sens` | DST GL2 Y2 Sensitivity | Sound Design / Process / GL2 / Y2 | — |
| `dst.gl2.y2.curve` | DST GL2 Y2 Curve | Sound Design / Process / GL2 / Y2 | — |
| `dst.gl2.y2.depth` | DST GL2 Y2 Depth | Sound Design / Process / GL2 / Y2 | — |
| `dst.gl2.y2.hold` | DST GL2 Y2 Hold | Sound Design / Process / GL2 / Y2 | — |
| `dst.gl2.y2.detect` | DST GL2 Y2 Detector | Sound Design / Process / GL2 / Y2 | — |
| `dst.gl2.s2.fft` | DST GL2 S2 FFT Size | Sound Design / Process / GL2 / S2 | — |
| `dst.gl2.s2.freeze` | DST GL2 S2 Freeze | Sound Design / Process / GL2 / S2 | — |
| `dst.gl2.s2.decay` | DST GL2 S2 Decay | Sound Design / Process / GL2 / S2 | — |
| `dst.gl2.s2.blur` | DST GL2 S2 Blur | Sound Design / Process / GL2 / S2 | — |
| `dst.gl2.s2.tilt` | DST GL2 S2 Tilt | Sound Design / Process / GL2 / S2 | — |
| `dst.gl2.s2.drift` | DST GL2 S2 Phase Drift | Sound Design / Process / GL2 / S2 | — |
| `dst.gl2.s2.shift` | DST GL2 S2 Shift | Sound Design / Process / GL2 / S2 | — |
| `macro1` | Macro 1 | Persistent strip / macro | — |
| `macro2` | Macro 2 | Persistent strip / macro | — |
| `macro3` | Macro 3 | Persistent strip / macro | — |
| `macro4` | Macro 4 | Persistent strip / macro | — |
| `out.limiter` | Musical Limiter | Sound Design / Global / Output Safety | — |
| `out.ceiling` | Limiter Ceiling | Sound Design / Global / Output Safety | — |
| `perf.balance` | WT/FM Balance | Persistent strip / perf | — |
| `amp.trigger` | Amp Trigger | Sound Design / Mod / Envelope AMP | — |
| `amp.sync` | Amp Tempo Sync | Sound Design / Mod / Envelope AMP | — |
| `env.wt.trigger` | ENV WT Trigger | Sound Design / Mod / Envelope WT | — |
| `env.wt.sync` | ENV WT Tempo Sync | Sound Design / Mod / Envelope WT | — |
| `env.fm.trigger` | ENV FM Trigger | Sound Design / Mod / Envelope FM | — |
| `env.fm.sync` | ENV FM Tempo Sync | Sound Design / Mod / Envelope FM | — |
| `env.aux1.trigger` | ENV AUX1 Trigger | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux1.sync` | ENV AUX1 Tempo Sync | Sound Design / Mod / Envelope AUX1 | — |
| `env.aux2.trigger` | ENV AUX2 Trigger | Sound Design / Mod / Envelope AUX2 | — |
| `env.aux2.sync` | ENV AUX2 Tempo Sync | Sound Design / Mod / Envelope AUX2 | — |
| `dst.wt1.d2.fb_mode` | DST WT1 D2 Feedback Mode | Sound Design / Process / WT1 / D2 | — |
| `dst.wt2.d2.fb_mode` | DST WT2 D2 Feedback Mode | Sound Design / Process / WT2 / D2 | — |
| `dst.fm1.d2.fb_mode` | DST FM1 D2 Feedback Mode | Sound Design / Process / FM1 / D2 | — |
| `dst.fm2.d2.fb_mode` | DST FM2 D2 Feedback Mode | Sound Design / Process / FM2 / D2 | — |
| `dst.gl1.d2.fb_mode` | DST GL1 D2 Feedback Mode | Sound Design / Process / GL1 / D2 | — |
| `dst.gl2.d2.fb_mode` | DST GL2 D2 Feedback Mode | Sound Design / Process / GL2 / D2 | — |
| `wt.za.shape` | WT ZA Shape | Sound Design / WT / Z Transforms | — |
| `wt.zb.shape` | WT ZB Shape | Sound Design / WT / Z Transforms | — |

## Actualisation ARCH-G4.3 (branche `feature/arch-g4-full-development`, 0.3.2)

8 StableID ajoutés en fin de registre (version 4, indices 695–702) ; les 695 identifiants ci-dessus sont inchangés
(identité, ordre, plage, défaut ; identifiants VST3 inchangés, G-22w). Couverture : 703/703.

| Ajout | Contexte d'affichage | Remarque |
|---|---|---|
| `dst.<slot>.d2.fb_mode` (×6) | panneau de l'emplacement quand l'algorithme D2 est sélectionné | Choice Legacy / Regenerative ; non automatisable ; jamais un armement |
| `wt.za.shape`, `wt.zb.shape` | WT › Z Transforms | Float [−1, 1], modulable ; ignoré par Fold, Warp, Mutation |
| `wt.za.type`, `wt.zb.type` (existants) | WT › Z Transforms | énumération étendue en fin : indices 4–8 (Harmonic Tilt, Mask, Focus, Shift, Phase Scatter) ; 0–3 inchangés |

Actions et états hors registre ajoutés ou modifiés (à couvrir par le contrôle de couverture ARCH-UI-04) :

| Action / état | Accès actuel | Contrat |
|---|---|---|
| Autoriser (autorisation explicite) | barre globale, visible si états autonomes suspendus | action explicite ; jamais par navigation, automation ou restauration |
| Mise en marche d'un armement ou du Drone par clic | contrôle de l'armement / bouton Drone | vaut autorisation explicite (TA-07w) |
| État d'autorisation (accordée / suspendue / révoquée par Panic, source, transport observé) | barre d'états du Sound Design ; Engineering › Diagnostics | lecture seule (télémétrie), aucune écriture DSP |
| Journal TRANSPORT AUTHORIZATION | Engineering › Diagnostics (8 derniers événements) ; fichier `ta_log.txt` si `ta_diagnostics.on` existe | diagnostic de recette, aucune écriture disque sans le fichier d'activation |

## Actualisation ARCH-UI-04 (0.4.0) — cartographie candidate remplacée par une preuve d'implémentation

La présente table reste l'intention ARCH-UI-03 (affectations candidates, non prouvées). Elle est **supersédée** par
[V1-ARCH-UI-04-STABLEID-MAP.md](V1-ARCH-UI-04-STABLEID-MAP.md), générée depuis l'exploration réelle de l'éditeur
compilé (test UI04-COV) : 703/703 StableID atteints par une commande liée visible, chacun édité et vérifié par sa
commande (UI04-BIND). Écarts de placement par rapport aux candidats : voir `docs/V1-ARCH-UI-04-REPORT.md` §6 ; les
actions hors registre listées ci-dessus sont vérifiées par UI04-OPS.
