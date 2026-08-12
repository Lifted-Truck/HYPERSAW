# gui2 integration checklist — what gui.html reaches that gui2 does not

**Generated 2026-08-12 from source**, not from memory: params come from the table in
`src/hypersaw_clap.cpp`, reachability from `data-p` attributes in each GUI, and the groupings are
gui.html's OWN section headings — so these are the feature boundaries already designed, not ones
invented here. Regenerate with `tools/gui_reach.py` (coverage counts) plus this file's method.

**Why this exists.** The human deliberately paused the GUI build to set up the FOUNDATIONS plumbing
first, reasoning that integrating features one at a time beats retrofitting them and letting some be
silently dropped. That reasoning is sound and this list is its missing half: the inventory that makes
"one at a time" auditable rather than hopeful. gui2's 18/105 is an in-progress state, not a defect.

**Coverage today:** `gui.html` 102/105 · `gui2.html` 18/105. Gated by `tools/gui_reach.py`, which
fails the build only when a param is reachable in NO gui, and prints per-GUI coverage every run.

**Note the asymmetry:** gui2 is not a strict subset. It already has mute/solo (104/105), per-oscillator
meters, the oscillator selector, the MPE hint and key-focus passthrough, which gui.html lacks. Those
must survive the migration in the other direction.

---

## Non-parameter features

| feature | gui.html | gui2 | note |
|---|---|---|---|
| Scope (stereo L/R) | yes | **no** | the whole point is watching L against R; super-width polarity is invisible in a sum |
| Preset load/save | yes | **no** | |
| Scale picker (`hzScalePicker`) | yes | **no** | portable pitch-class-set control, built 2026-08-10 |
| Fine-drag (shift) | yes | partial | gui2 has 4 refs vs 11; verify shift+drag works on every control |
| Spectrum | yes | yes | |
| Phase circle | yes | yes | |
| XY pad | yes | yes | |
| Meters | yes | **better** | gui2 has per-oscillator meters |
| Mute / solo | **no** | yes | gui2-only |
| Oscillator selector | partial | yes | gui2's role-addressed version is the correct one (L0028) |
| PANIC + Escape | yes | yes | added to gui2 2026-08-12 |
| MPE-off hint | **no** | yes | gui2-only |
| Key-focus passthrough | **no** | yes | gui2-only, and it is the fix for the lingering notes |

Also missing and not yet built anywhere: **master level meter** (roadmapped K2).

---

## Parameters, grouped by gui.html's own sections


### Envelope — 13 params

**Do this first.** ADSR being unreachable actively cost us: during the 2026-08-12 Expressive Chords investigation the envelope could not be inspected or adjusted, and we compared against another synth without noticing. Nothing else on this list has already distorted a diagnosis.

- [ ] `19` **attack** — Attack (s)
- [ ] `20` **decay** — Decay (s)
- [ ] `21` **sustain** — Sustain
- [ ] `22` **release** — Release (s)
- [ ] `65` **sAttack** — S.Attack (s)
- [ ] `66` **sDecay** — S.Decay (s)
- [ ] `67` **sSustain** — S.Sustain
- [ ] `68` **sRelease** — S.Release (s)
- [ ] `94` **voiceEnv** — Per-Partial Env
- [ ] `91` **onsetScatter** — Onset Scatter (ms)
- [ ] `92` **onsetAlpha** — Timing Correction
- [ ] `93` **attackScatter** — Attack Scatter
- [ ] `95` **relScatter** — Release Scatter


### Voice — 6 params

Mono/legato/glide — the note-lifecycle surface. Second because the stuck-note and lingering-note work all turned on these, and they are still unreachable.

- [ ] `38` **pitchBend** — Pitch
- [ ] `33` **glide** — Glide (s)
- [ ] `90` **glideMode** — Glide From
- [ ] `32` **voiceMono** — Mono
- [ ] `34` **voiceLegato** — Legato
- [ ] `89` **polyGlide** — Poly Glide


### FX rack — 12 params

Blocks the ratified bass-mono-as-a-slot work outright: a slot type in a rack with no picker is unreachable by construction (L0023). 12 params, and Comp/Comb are already-built cores nobody can select.

- [ ] `57` **fx1type** — FX1 Type
- [ ] `58` **fx1amt** — FX1 Amount
- [ ] `96` **fx1tone** — FX1 Tone
- [ ] `59` **fx2type** — FX2 Type
- [ ] `60` **fx2amt** — FX2 Amount
- [ ] `97` **fx2tone** — FX2 Tone
- [ ] `61` **fx3type** — FX3 Type
- [ ] `62` **fx3amt** — FX3 Amount
- [ ] `98` **fx3tone** — FX3 Tone
- [ ] `63` **fx4type** — FX4 Type
- [ ] `64` **fx4amt** — FX4 Amount
- [ ] `99` **fx4tone** — FX4 Tone


### Output & perception — 17 params

Contains `bassMono`/`bassMonoHz` (40/41), so the ratified slot work needs this or the FX rack or both. Also the pan/width surface, where A4/A5 ear A/Bs are waiting.

- [ ] `12` **rtone** — R->Tone
- [ ] `13` **normExp** — Density Comp
- [ ] `42` **panScatter** — Pan Scatter
- [ ] `87` **superMode** — Super-Width Mode
- [ ] `84` **panLayout** — Pan Image
- [ ] `85` **panCurve** — Fan Curve
- [ ] `76` **panMotion** — Pan Motion
- [ ] `77` **panMode** — Pan Motion Mode
- [ ] `71` **toneTilt** — Tone Tilt
- [ ] `72` **hiTame** — Hi Tame
- [ ] `86` **panInvert** — Fan Invert
- [ ] `41` **bassMonoHz** — Bass XOver (Hz)
- [ ] `88` **oversample** — Oversample 2x
- [ ] `16` **digital** — Digital
- [ ] `15` **mono** — Mono Fold
- [ ] `40` **bassMono** — Bass Mono
- [ ] `31` **absK** — Absolute K


### Dynamics — 10 params

Includes `grav` — the known L0023 instance, a shipped feature with no control since before this list existed.

- [ ] `83` **pivotMode** — Pivot
- [ ] `24` **topo** — Topology
- [ ] `25` **reach** — Ring Reach
- [ ] `26` **mu** — Cluster Link
- [ ] `56` **balance** — A/B Balance
- [ ] `27` **alpha** — Phase Lag
- [ ] `28` **poles** — Poles q
- [ ] `29` **grav** — Gravity
- [ ] `30` **basin** — Basin (c)
- [ ] `31` **absK** — Absolute K


### The swarm — 9 params

Distribution, detune law, and the fold laws (harmReach/stretchB/spread/anchor) — the instrument's core identity.

- [ ] `43` **engine** — Engine
- [ ] `2` **dist** — Distribution
- [ ] `5` **law** — Detune Law
- [ ] `79` **harmReach** — Harmonic Reach
- [ ] `80` **stretchB** — Stretch B
- [ ] `81` **spread** — Octave Spread
- [ ] `82` **anchor** — Root Anchor
- [ ] `69` **shape** — Saw Shape
- [ ] `23` **beatMult** — Grid Cycles/Beat


### Drift — 6 params

Self-contained, 6 params, no dependencies. Good candidate whenever a small win is wanted.

- [ ] `9` **driftDepth** — Drift Depth (c)
- [ ] `10` **driftRate** — Drift Rate
- [ ] `73` **driftMode** — Drift Mode
- [ ] `75` **freqGlide** — Freq Glide (s)
- [ ] `78` **motionCenter** — Centre Pin
- [ ] `74` **keepPhase** — Keep Phase


### The coupling — 3 params

Only 3 missing; `inertiaCurve` is dev-only and exempt from the gate.

- [ ] `11` **inertia** — Inertia
- [ ] `70` **inertiaCurve** — Inertia Curve (dev)
- [ ] `39` **scatter** — Phase Scatter


### Spectra — 12 params

**Deprioritised, not cancelled.** The ROADMAP already records SPECTRA-facing items as behind the renovation — they matter when SPECTRA returns to the UI. 12 params, safe to do last.

- [ ] `44` **partials** — Partials
- [ ] `45` **tilt** — Amp Tilt
- [ ] `46` **stretch** — Stretch
- [ ] `47` **cloud** — Cloud Voices
- [ ] `48` **cwidth** — Cloud Width
- [ ] `49` **wtilt** — Width Tilt
- [ ] `50` **wlaw** — Width Law
- [ ] `51` **cascade** — Cascade
- [ ] `53` **subVol** — Sub Level
- [ ] `54` **subWave** — Sub Wave
- [ ] `55` **subOct** — Sub Octave
- [ ] `52` **subOn** — Sub Osc


---

## Suggested order, and the reason for it

1. **Envelope** — the only cluster that has already distorted a diagnosis
2. **Voice** — where three field investigations lived
3. **FX rack** — unblocks the ratified bass-mono slot work
4. **Output & perception** — carries `bassMono` itself, plus the waiting A4/A5 ear A/Bs
5. **Dynamics** → **The swarm** → **Drift** → **The coupling**
6. **Spectra** last, per the ROADMAP's existing deprioritisation

Two standing rules for each increment, both earned the hard way here:

- **A control lands with its param, in the same change** (L0023, three occurrences, now gated).
- **Check the gui2-only features survive** — mute/solo, per-oscillator meters, the MPE hint and the
  key-focus passthrough exist ONLY in gui2, so a migration that treats gui.html as the source of
  truth would delete the fix for the lingering notes.
