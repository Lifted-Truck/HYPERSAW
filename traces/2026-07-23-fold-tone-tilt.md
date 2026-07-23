# 2026-07-23 — First lab→core fold: tone tilt (ADR-060)

The first integration of a detune-lab finding into the real thing. Human chose
"safe superset first" as the pipeline-primer; tone tilt is inert at its default,
so it exercises the full reference→golden→core→parity loop end-to-end at lowest
risk. Off fresh main (after #72 merged the lab work + fold map).

## What changed
- **swarmsaw.html (reference/oracle, protected — human-authorized):** new `tilt:0`
  param; per-voice `vlp`/`vlpc` state (vlpc init 1); the coeff loop after σ
  (`H = tilt>0 ? 2·200^(1−|t|) : 0.1·24^|t|`, cutoff `√(f/f0)`); the one-pole in
  the render voice loop (`v = tiltHP ? v−vlp : vlp`); `vlp` reset on note-on.
- **swarm_core.h (core):** mirrored bit-for-bit — param, `Swarm.vlp/vlpc`, ctor
  init, initVoice reset, controlTick coeff (kTau, std::pow/exp/sqrt), render
  application after the ADR-058 shape block, `paramSlot("tilt")`, `tiltHP` member.
- **gen_goldens.mjs:** two scenarios — tilt-dark (+0.6, K 0.3), tilt-thin (−0.6).
- **DECISIONS.md:** ADR-060. **NOT yet a CLAP param** (public-interface change =
  next gated step); this is DSP + parity only.

## Evidence
- **Pre-edit inertness proven first:** extracted SwarmSynth from origin/main's
  swarmsaw.html vs the edited one — defaults / sync-lock / gauss-cloud / splay-jp7
  render BIT-IDENTICAL (tilt=0); tilt±0.6 changes output.
- **`./verify full` EXIT 0:** `parity_check` **60/60** within ε (was 54/54); the 6
  new tilt goldens at **rms 0.000e+00** (C++ == JS exactly); worst overall
  4.262e-9 @ dyn-ring (pre-existing, unrelated). trajectory / state / **notefuzz**
  / force / spectra / filter / notch / swarmalator / time all GREEN.
- git: on branch fold-tone-tilt (off fresh main).

## Process note (L0004)
Branch created THIS turn off fresh origin/main (checked #72 MERGED via gh pr view
before branching) — the pre-push reflex added after the 14-commit stranding.

## Next
Expose `tilt` on the CLAP param surface (hypersaw_clap.cpp id table + applyParam
+ readParam + GUI) — a frozen-id public-interface change → its own gated step.
Then continue the fold sequence (map §recommended): the other supersets, then the
new detune laws, then the divergences (root-pivot topology, pan default, saw
shape).
