# 2026-08-05 — COOPERATOR lab (Kuramoto FM) + spectra NaN root cause

## Changes
- `docs/design/cooperator-lab.html` — NEW: engine candidate, both architectures.
- `docs/design/spectra-lab.html` — NaN fix + ADR-032-style watchdog.
- `ROADMAP.md` — COOPERATOR section + register row B14; NaN root cause appended.

## Spectra NaN (human report: "voices at 7 caused a NaN error")
Not cloud-7-specific. My liveliness rewrite of `rebuild()` dropped its tail
responsibility — the loop resizing LIVE notes' arrays — so growing partials or cloud
mid-note indexed past the old `vf`/`phase` arrays. Typed-array OOB reads return
`undefined`; `undefined/sr` is NaN; `fsin(NaN)` poisons everything after. Static configs
were all clean (9/9); the reproduction needed simulated slider DRAGGING while notes ring
(NaN at block 52). Fixed by restoring the resize, preserving surviving phases so a drag
does not restrike the note. Verified clean over 3000 blocks of the same abuse. Watchdog
added (self-heal + counter); it fired ZERO times, confirming the root fix carries the load.
Same lesson as L0023's family: a rewrite must diff the responsibilities the old code
carried, not just the ones being changed.

## COOPERATOR (human rulings: both architectures · force system yes · the name)
CLOUD (carrier + Kuramoto-coupled modulator swarm) and NETWORK (4 ops, continuous
coupling matrix, per-edge Kuramoto↔FM morph). Force system on every ratio: seeded
spread, per-unit drift, gravity to the just lattice (kRatios), basin, K.

Measured at birth:
- Gravity captures 1.48 → 3/2: 23.2 ¢ → 0.0 ¢ in 2 s (and holds under drift).
- Cloud lock: R 0.394 / 0.933 / 0.941 at K 0 / 0.5 / 1 (12 ¢ spread); transition between
  K 0.25 and 0.5; partial lock (0.803) at 50 ¢ spread by design.
- Worst-case chaos (12 edges @ 1, full FM, index 8) finite for seconds, 0 watchdog resets.
- Network carrier bend bounded: ≤ 33.5 ¢ measured (clamp ±80), documented.

Two bugs fixed pre-ship, both caught by measurement, both mine:
1. Coupling correction written into state the next tick overwrote — R flat 0.394→0.395
   over the whole K range, a dead knob — and ~170× too weak to cancel the detune it
   fights. Rewritten as fresh-per-tick offsets with the ceiling in CENTS (40 ¢ at K=1),
   commensurate with the spread knob.
2. Network carrier `lCur` never rebuilt → coupling would random-walk the note's pitch.

Honest limit, queued as increment 2: 1:1 phase pull cannot lock ops at different ratios
(network R 0.2–0.3 is physics, not a defect); true cross-ratio lock needs n:m edges
(Arnold tongues). Stated in the lab.

## Evidence
verify fast exit 0. In-browser: script completes, 12 matrix cells, poly + per-key release
correct, locked-capture patch reads R = 0.992 / ratio error 1.1 ¢ live in the meters.
