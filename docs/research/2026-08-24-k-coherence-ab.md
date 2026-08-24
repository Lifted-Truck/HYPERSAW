# K → coherence: A/B against the pre-change build (2026-08-24)

**Trigger:** human, 2026-08-24 — "It feels like the range on K has been reduced;
suddenly I'm struggling to get voices to cohere at many positions along the XY."

**Method:** scratch probe through the REAL plugin (CLAP factory): hold A3,
sweep K at two detunes, drift depth 0, settle 1.2 s, then measure periodicity
CLARITY of 0.5 s of rendered audio (autocorrelation peak near the f0 lag over
zero-lag energy — a locked swarm is near-periodic, clarity → 1). Identical
binary probe run against today's build and a worktree build of `6a97f04`
(2026-08-22 morning — before ADR-109 A1 / 111 / 112 / 115 and the entire
re-skin).

**Result: identical to four decimal places at every point.**

| K | det 0.28 (now) | det 0.28 (old) | det 0.60 (now) | det 0.60 (old) |
|---|---|---|---|---|
| 0.00 | 0.9624 | 0.9624 | 0.9101 | 0.9101 |
| 0.15 | 0.9621 | 0.9621 | 0.9102 | 0.9102 |
| 0.30 | 0.9603 | 0.9603 | 0.9109 | 0.9109 |
| 0.45 | 0.9560 | 0.9560 | 0.8948 | 0.8948 |
| 0.60 | 0.9743 | 0.9743 | 0.9305 | 0.9305 |
| 0.80 | 0.9899 | 0.9899 | 0.9900 | 0.9900 |
| 1.00 | 0.9902 | 0.9902 | 0.9903 | 0.9903 |

**Reading.** The K→coherence response of the plugin is unchanged — which parity
implied (the core is pinned to the reference) and this confirms end-to-end
through the shell's parameter path at the exact control named. The inertia
taper (ADR-024 A1) was also examined and cleared analytically: it LOWERED
effective inertia for every stored knob position, and in this core inertia is a
mass-spring that slows locking — so the taper change could only have made
coherence easier, not harder.

**What DID change in the same window: the coherence instruments.** The voice
netting was invisible for a stretch (grid-violet on a violet tube, fixed
2026-08-24); the R readout had shrunk to a bare number; the phosphor blooms
visually smear dot alignment. The interface round shipped alongside this note
restores the detailed fixed-format readout and makes the waveform's hue ride R
— i.e. it rebuilds the instruments the judgement was being made with.

**Standing offer:** if a specific patch + XY position still feels wrong, name
them and this probe reruns with those exact values — the harness is in the
session scratchpad and takes minutes.
