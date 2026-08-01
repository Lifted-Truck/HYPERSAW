# 2026-07-31 — ADR-073: even-voice symmetric pan fan

Human direction 2026-07-30 ("even numbers of voices should have no voice centered
— right now 2 with any width is unlistenable"), which is also the protected-path
gate approval for the swarmsaw.html edit. Off fresh main after #145.

## Change (reference-first, ADR-003)
- swarmsaw.html: the fan's distance law forks on PARITY. Odd n unchanged
  (d = r/(n−1), rank 0 dead centre — ADR-070's requested image). Even n:
  d = (floor(r/2) + 0.5)/(n/2), so pairs sit symmetrically and NO voice takes
  the centre seat. Sides still alternate; ranking, curve, invert untouched.
- swarm_core.h: mirrored expression-for-expression.
- gen_goldens.mjs: even-fan-2 and even-fan-4 scenarios, width EXPLICIT (the fan
  is inert at width 0, so a golden without width would prove nothing).

## Why n=2 was unlistenable
Old law at n=2: d = r/(n−1) → 0 and 1, i.e. one voice dead centre and one voice
HARD side. New: ±0.5·width.

## Evidence
- INERTNESS by file hash, old reference vs new: **144 renders bit-identical**,
  exactly **4 changed** — cauchy-cloud (n=16), gauss-cloud (n=16), hi-tame (n=12),
  stretch-bell (n=12). Every changed scenario is genuinely even-n; every odd-n
  render untouched.
- PARITY: 147/147 within ε (worst 4.262e-09, pre-existing dyn-ring). The new
  even-fan goldens: n=2 rms 0.000e+00, n=4 rms 2.393e-17.
- BALANCE: sum of pan seats is exactly 0.000000000000 for n = 2,4,6,8,12,16, and
  no voice lands on the centre seat at any even n.
- ./verify full green, all nine chains.

## Measurement discipline note (cost three false readings here)
The first "inertness proof" compared manifest.tsv columns — that file carries NO
hash column, so it reported 0 changed and proved nothing. Two scenario parsers
(regex across object boundaries; split on `},{` which nested `p:{…}` breaks) both
misreported which scenarios were even-n. The real proof is sha256 over the .f32
renders, generated from the stashed reference and the edited one. Same family as
L0016/L0017: a detector must be calibrated on a case where it MUST fire.
