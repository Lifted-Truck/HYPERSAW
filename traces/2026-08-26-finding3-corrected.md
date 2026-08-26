# Trace — B41 finding 3 explained, and corrected in two places

**Trigger** human 2026-08-26: *"Can you remind me what the order-parameter trig
was in the B41 CPU audit? I assume I'm going to ratify this recommendation but
I would like to understand it better first."*

Answered from the code rather than from the audit's own summary (L0043,
written the same day). That reading found two errors in the finding.

## What it is

`controlTick` runs every `kTick = 16` samples — 2756/s at 44.1 kHz, **per
voice** — and contains two loops over all n oscillators:

```cpp
for (i<n) { a = phase[i]*TAU;   sx += cos(a); sy += sin(a); }   // -> R, psi (+atan2)
for (i<n) { a = phase[i]*TAU*n; nx += cos(a); ny += sin(a); }   // -> RN
```

`R` and `psi` are the Kuramoto order parameter: treat each oscillator's phase as
a unit vector, `R` is the length of their average (0 = evenly splayed, 1 =
locked) and `psi` its direction — the mean phase the swarm clusters around. It
exists so the coupling can be **mean-field**: every oscillator pulls toward one
summary vector, O(n), instead of n² pairwise interactions. The use site is
`:1636` — `KsmS * s.R * sin(psi - phase[i] - alpha)`.

**The waste:** `KsmS` derives from `km = 4·K·|K|`, so at K = 0 the entire
coupling term is multiplied by exactly zero. The trig runs and feeds nothing.

## The two corrections

**1. "Skipping is output-identical for audio" is false when `rtone` ≠ 0.**
R→Tone (param 12) reads `s.R` at `:1734` to set a filter cutoff,
`fc = 16000·2^(-6·rtone·R)`, and that path is **not** gated by K. The recorded
gate condition (`K != 0 || absK != 0 || Kenv > eps`) is therefore incomplete —
it needs `|rtone| > eps` too. `rtone` defaults to 0, so a gate written and
tested against defaults would have passed every oracle and changed the sound of
any patch using R→Tone at K = 0. Exactly the shape of bug this repo keeps
catching: correct at defaults, wrong where the feature is used.

**2. The RN loop has no audio consumer at all — at any K.** `s.RN` is written
at `:1582`, copied to the viz snapshot (`hypersaw_clap.cpp:1989`), drawn in
`gui.html`, read by `trajectory_check`. It never reaches the DSP. So half this
cost is **unconditionally** non-audio, not merely wasted at K = 0, and gating it
needs no condition at all. Caveat: `trajectory_check` asserts on RN through the
snapshot, so moving it to viz rate changes when it is sampled — an oracle
question to settle, not a blocker.

**Net:** the K = 0 half is a narrower win than recorded; the RN half is a wider
one.

## The coupling the human already flagged

B46 option 3 would derive the density compensation from R (|Σe^{iθ}| = n·R
exactly). If adopted, **R becomes load-bearing for gain at every K** and the
K = 0 skip closes permanently. Note this does *not* touch RN, which stays
skippable either way. A possible reconciliation, offered as untested: at K = 0
with splayed phases R tends to ≈ 1/√n, so a gain law could use that closed form
where R is not computed — which would preserve both. That is reasoning, not a
measurement, and the fluctuation of R under detune is exactly what would have to
be measured before believing it.

**Corrected in both places** that carried the wrong claim:
`docs/research/2026-08-24-cpu-audit.md` finding 3, and ROADMAP's own
order-parameter entry. No code changed; nothing ratified.
