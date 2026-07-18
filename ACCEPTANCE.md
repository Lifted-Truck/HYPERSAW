# SWARM✱ — acceptance criteria

Two layers, per ATTEST discipline. **Layer-0**: deterministic, CI-blocking, runs in `./verify fast`. **Layer-E**: behavioral/listening checks, `./verify full` + human sign-off. Every number below was measured on the JS reference implementations in this design cycle; tolerances are stated where drift is acceptable.

Standard test note: A3 = 220 Hz, 44.1 kHz, defaults unless stated. "R" = first-harmonic order parameter of the focus swarm. Time bases in seconds of rendered audio.

## L0-1 · Parity oracle (the master test)

C++ core vs JS reference (`SwarmSynth` / `SpectraSynth` / `DynSynth`): identical mulberry32 streams, identical seeding scheme, identical control-tick ordering → sample outputs match within ε = 1e-6 RMS over 4 s renders, for a matrix of {3 seeds} × {per-engine param vectors covering each subsystem}. Any intentional divergence requires an ADR.

## L0-2 · Sync phase transition (SAW core)

From retriggered start, K in ×σ units, R sampled ~1/s:

| K setting | Required behavior | Reference measurement |
|---|---|---|
| 4.0×σ (knob +1.0) | full lock, R ≥ 0.95 sustained | R = 0.97 steady |
| 2.9×σ (+0.85) | steady partial lock | R ≈ 0.92 steady |
| ~2.0×σ (+0.7) | oscillating partial lock (shimmer) | R oscillates ≈ 0.24 ↔ 0.78 |
| 1.4×σ (+0.6) | subcritical dissolve from coherent start | R: 0.97 → 0.16 over ~3 s |

Tolerance: regime classification must match; R values ±0.08.

## L0-3 · Splay attractor (saw kernel)

K = −1.0, JP distribution, M = 7, 4 s settle: R₁ ≤ 0.10 (measured 0.04); splay order R_N ≥ 0.75 (measured 0.84); spectral projection at 7·f0 ≥ 5× the uncoupled baseline (measured 9×: 0.0017 → 0.0153, approaching f0's 0.0226). Intermediate check: K = −0.7 → R_N ≈ 0.44 ±0.1 (the continuum exists).

**L0-3 extension — R₁/R_N regime classification** (added 2026-07-17 per the splay-legibility change note, archived at `docs/change-notes/2026-07-17-splay-legibility.md`; measured on the reference at A3, defaults, settled; values ±0.08). R_N is computed per SPEC §5.6 and is parity-relevant (same loop order, same n as the reference):

| Regime | K | R₁ | R_N |
|---|---|---|---|
| Sync | +1.0 | 0.97 | 0.07 |
| Free | 0.0 | see erratum | see erratum |
| Splay | −1.0 | 0.04 | 0.84 |

*Erratum (ratified at the Phase 1 gate, 2026-07-18 — see traces/2026-07-17-phase1-swarmcore-parity.md):* the free-row single values originally recorded here (R₁ 0.39 / R_N 0.16) were snapshots of a fluctuating observable — for n = 7 free oscillators both orders fluctuate around ~1/√n ≈ 0.38 — and do not reproduce even on the reference. Free is classified by time average: dense-sampled ⟨R₁⟩ and ⟨R_N⟩ over a ≥5 s settled window both in (0.1, 0.6) (reference: ≈0.33/0.33). Protocol notes for the other rows: the shimmer pair (L0-2, K=+0.7) is the dense-sampled min/max — 1/s sampling aliases it flat; the K=+0.6 dissolve figures describe the initial transient, with "subcritical" meaning no sustained lock afterwards.

## L0-4 · Inertia

ζ = 0.45. K = +0.85 with inertia 0.5: R hunts in ≈ 0.74–0.85 band without settling to steady lock over 8 s. Inertia 0.8: sustained chaotic hunting (R excursions below 0.4). No NaN, no unbounded momentum, for the full {K × inertia} grid.

## L0-5 · R→tone routing

rtone = +1.0 at full lock: one-pole coefficient falls from bypass (≈0.9) to ≈0.04 (cutoff ~16 kHz → ~280 Hz). Sign flip inverts the mapping against (1−R).

## L0-6 · Cascade zipper (SPECTRA)

K = +0.9, cascade = 1.0, retrigger off, P = 12: lock front (R_k > 0.85) advances monotonically low→high, completing in 7–10 s (reference: 8 s, with turbulence ahead of the front). Cascade = 0.5: completes in ≤ 3 s (reference 2 s). Monotonicity of the front, not exact timing, is the blocking criterion; timing within ±30%.

## L0-7 · Interference gate (SPECTRA)

K = −1.0 vs K = 0, defaults, 4 s settle: RMS reduction ≥ 12 dB (reference −14.8 dB), with per-partial stereo narrowing active (w = K_splay/(K_splay + 2σ)). Regression guard: without narrowing, reference leaked to −1.7 dB — a result near that value means the narrowing path broke.

## L0-8 · Consonance gravity (DYNAMICS)

Notes 220 Hz + 331 Hz (fifth +5.2¢ sharp), gravity 0.7, ~3 s: settled ratio within ±0.5¢ of 3/2 (reference: 1.50001, +0.01¢). Pairs outside basin (default ±35¢) move 0¢. Per-pair readout reports captured ratio name, octave fold, live cents error.

## L0-9 · Sakaguchi lag

Mean-field, K = +0.9, locked: α = 60° shifts locked mean frequency down ≈ 1.8 Hz from 220 (reference 218.19). Direction and monotonicity in α blocking; magnitude ±25%.

## L0-10 · Two-cluster broken symmetry

n = 32, detune 2¢, K = +1.0, μ = 0.6, retrigger off, 25 s, averaged over final 10 s: at α = 78°, sustained cluster asymmetry with ⟨R_A⟩ ≈ 0.82 / ⟨R_B⟩ ≈ 0.67 (±0.08), consistent across seeds {1234, 777, 42}. At α = 86°: seed-dependent regimes (variance across seeds ≥ 2× the α=78° variance). Note: chimera-like states under 2¢ heterogeneity; strict-chimera experiments require absolute-K mode (ADR-004).

## L0-11 · Ring spatial coexistence

Ring, reach 5, α = 80°, K = +0.9, retrigger off, 6 s: local order parameter (±3-neighbor window) spans min ≤ 0.2 and max ≥ 0.8 around the ring (reference 0.08 / 0.87).

## L0-12 · Tempo-grid law

120 BPM, 1 cycle/beat (u = 2 Hz): all pairwise voice-frequency gaps are exact multiples of u (machine precision). Spread scales with detune knob: 10¢ → 4 Hz / 3 rungs; 60¢ → 16 Hz / 9 rungs (M = 16). Envelope periodicity: DFT of squared signal shows ≥ 6× power at u vs an incommensurate probe (reference 10.2× at 2.00 vs 1.37 Hz). Coherence reachable: K = +0.85 over grid law locks to R ≥ 0.88 within 2 s (reference 0.92 in ~1 s).

## L0-13 · Determinism & stability

Same seed + note sequence → bit-identical control-path state and ε-identical audio across runs. Full param-grid fuzz (including stretch × wtilt × poly chords): no NaN, no denormal stall, master peak ≤ 1.0 post-tanh.

## Layer-E (behavioral, non-blocking, human-verified per release)

- E-1: K sweep at ~2×σ is *audibly* a journey (cloud → herd → shimmer → comb → saw), not a switch.
- E-2: ERB-flat law: same patch at C2 vs C6 keeps subjectively comparable roughness; cents law audibly does not.
- E-3: Gravity settling on a held ET major third is audible as decelerating-then-stopped beating within ~2 s.
- E-4: Splay erasure residue (sine kernel) reads as "shimmering silence," not artifact.
- E-5: Mono fold-down of a wide patch is usable, not comb-hollow.
- E-6: 8-voice poly at M = 32 (SAW) sustains without dropouts on the target hardware envelope (ratified 2026-07-17 at the Phase 0 gate): min-spec = Apple M1 base / 4-core 2018-class Intel ultrabook; Windows x64 AVX2; 44.1 kHz at 128-sample buffer; the patch must hold < 50% of one core on min-spec.
