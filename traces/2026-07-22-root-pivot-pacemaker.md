# 2026-07-22 — Sync pivot: root/fundamental pacemaker option (lab)

Human, following the "stuck voice" explanation: the pivot voice should be the
base — the fundamental, both centered (symmetric detune) and lowest (anchored/
harmonic). Reasonable, and it's a topology choice, not a relabel.

## Framing (recorded)
"Pivot" is two things: (a) the splay PHASE anchor, and (b) the sync
FREQUENCY-collapse target. The visible "stuck voice" is (b) — mean-field sync
collapses to the swarm MEAN, and the min-detune voice sits there. Making the root
the pivot means collapsing toward the FUNDAMENTAL, not the mean — a root-pinned
"pacemaker" coupling. Musical payoff: pitch-stable collapse (fundamental holds,
upper voices fold onto it) vs. mean-field's pitch-rising collapse. This is a
DIVERGENCE from pure mean-field Kuramoto (core identity) but fits the roadmapped
topology menu (mean-field / ring / two-cluster → + pinned/star). Prototype-first;
a core port needs an ADR.

## Change (audition lab only — docs/design/detune-lab.html)
- `sync pivot` select in Coupling: `mean field` (default) / `root (fundamental)`.
- Fundamental = voice nearest f0 (`argmin|vf−f0|`) — min|x| in symmetric detune,
  the lowest voice in anchored/harmonic (one consistent "base", matching the
  human's "both centered and lowest").
- pivotMode=1: sync becomes a pacemaker — every voice entrains to the
  fundamental's phase `KsmS·sin(2π(φ_root−φ_i))` (fundamental couple=0 → it
  stays); splay anchor also moves to the fundamental. pivotMode=0 = unchanged.

## Evidence
- **Default (mean-field) Δ = 0** at K=0.5 (bit-identical; pivotMode default 0).
- Root pivot at full +K on a 1..7 harmonic stack: fundamental pins at ratio 1.0
  (holds f0) and the coupling target is the fundamental, vs mean-field scattering
  around ~4× f0 (the mean). Pitch-stable collapse confirmed.
- Browser: no console errors; `sync pivot` present + wired. `node --check` clean;
  `./verify fast` EXIT 0. No oracle/core impact, no ADR (lab only).
- git: on branch lab-detune-octave-spread (PR #70).

## Next
Human auditions mean-field vs root pacemaker by ear (esp. harmonic/anchored
modes). If root feels right, it becomes a topology option on the SAW engine —
prototype-first fold into swarmsaw.html + an ADR (it diverges from the reference
mean-field). Open tuning: the pacemaker drops the R (order-parameter) scaler that
mean-field uses, so its onset is a touch stronger — revisit if the K taper feels
off in root mode.
