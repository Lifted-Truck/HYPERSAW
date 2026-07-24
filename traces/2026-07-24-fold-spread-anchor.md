# 2026-07-24 — Ninth fold: octave spread + root anchor (ADR-068)

The placement-block rewrite the fold map warned about: `dep = detune·spread`
and `x − anchor·xmin` thread through EVERY law — six laws touched, not one
branch added. Off fresh main after #84 merged.

## Changes (reference-first, ADR-003)
- swarmsaw.html: `spread:1, anchor:0` params; xmin tracked in rebuildDist;
  dep hoisted before the freq loop; anchored x; detune→dep in laws 0/1/4/5/ERB.
- swarm_core.h: mirrored (+ paramSlot spread/anchor, `double xmin` member).
  Tempo-grid law 3 (core-only; the lab is silent) gets dep + anchored x too —
  uniform placement semantics, recorded as a ruling in the ADR.
- gen_goldens.mjs: spread-octave, anchor-root (seeded cauchy → xmin varies
  per seed), spread-anchor-ni. Wide scenarios uncoupled per the ADR-065
  domain limit.
- DECISIONS.md: ADR-068.

## Rulings (the lab couldn't decide these)
- Harmonic law: ignores anchor (inherently root-anchored), spread scales it.
- Stretch law: x² term uses the ANCHORED x (matches the lab).
- rootWeight: OUT — gain feature entangled with anchor in the lab, not
  placement; its own fold later.

## Evidence
- Inertness by manifest diff: all **108 pre-existing renders sha256-identical**
  between the edited reference and origin/main's.
- Geometry probed on the reference first: spread 24·det 0.5 → bottom voice
  exactly 0.5000·f0; anchor 1 → lowest voice exactly 1.0000·f0, all others
  above (the NI unipolar fan). Interior offsets are density-comp (normExp),
  the established layer.
- ./verify full EXIT 0: parity 108/108 → **117/117**, the 9 new at
  **rms 0.000e+00**; all chains + notefuzz GREEN. anchor-root's three seed
  hashes all differ (cauchy xmin varies) — the anchor path is exercised for
  real, not just at a fixed layout.
- git: on branch fold-spread-anchor (off fresh main).

## Next
The fold map's superset+laws phase is COMPLETE (9 folds, ADR-060..068,
parity 54 → 117). Remaining: comp/limiter decision (#16), divergence ADRs
(#17: root-pivot topology, pan default image, saw retarget), KS comb as an
FX-rack slot, batched CLAP pass (#18: law 0..5, dist 0..4, spread/anchor +
the rest). rootWeight fold when wanted.
