# 2026-07-21 — Detune lab: octave spread + root-anchor (NI-style extreme)

Human, auditioning the NI/AG Cook supersaw walkthroughs: the detune spread goes
fluidly up to at least an octave across the whole ensemble, with the lowest
voice staying pinned near the root (asymmetric, upward fan). The lab could only
do ±1 semitone symmetric. Add the extreme as two live, default-inert controls so
it can be heard before anything is folded into the reference prototype.

## Change (audition lab only — docs/design/detune-lab.html, not a reference prototype)
- `spread` slider (1…24 st, default 1): scales the detune depth. Law math now
  uses `dep = detune · spread` in place of raw `detune`. spread=1 reproduces the
  prior behavior bit-for-bit (dep == detune); spread≈12 at detune=1 reaches an
  octave on the cents law, up to two octaves at 24.
- `anchor ↑` slider (0…1, default 0): slides the swarm up by `anchor · xmin` so
  the lowest voice sits at the root (offset 0) and the rest fan upward. anchor=0
  is the classic symmetric bipolar detune; anchor=1 is the bottom-locked stack.
  `this.xmin` (lowest voice position) stored in `rebuild()`.
- `detune` readout is now law- and layout-aware: reports the ACTUAL most-detuned
  voice after anchoring (`top = xmax − anchor·xmin`), switching to `oct` past
  1200¢ and flipping `±`→`+` when anchored. Hz/ERB laws scale by `dep` too.

## Evidence
- Superset is inert at defaults: spread=1 → `dep==detune`; anchor=0 → `x` unchanged.
  Readout at defaults `±28 ¢` (JP edge 0.9766 × 0.28 × 100), matching prior lab.
- Script parses (`vm.compileFunction`), browser pane: no console errors, controls
  render, `detune_o` tracks the extreme correctly — anchor=root↑ / spread=6 st /
  detune=1 → `+1.00 oct`; law=Hz → `+240 Hz`.
- `./verify fast` EXIT 0. Design-lab HTML — not in the build or the 9 oracle
  chains, so no parity/core impact and no ADR (no reference-prototype edit).
- git: on branch lab-detune-octave-spread (off origin/main)

## Next (detune workshop)
Human auditions spread/anchor/law/B by ear → picks the winning combination →
fold into swarmsaw.html (reference update, ADR-011/012 file-drop) → port to
swarm_core.h with parity. The anchor+spread depth mapping would become real
params on the SAW engine at that point.
