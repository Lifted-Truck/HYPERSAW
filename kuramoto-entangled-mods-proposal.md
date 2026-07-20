# Proposal: Emergent Mod Sources & Entanglement-Structured Coupling for the Kuramoto Supersaw

**Status:** Proposal — for review and phased adoption into ROADMAP.md
**Audience:** Claude Code agent working on the Kuramoto supersaw synth
**Origin:** Design session with Julian, 2026-07-19
**Doctrine note:** Everything below is deterministic-core. No AI in the signal or control path. All stochastic elements use seeded, named RNG streams. No wall-clock anywhere in the core. All new behavior must land behind `./verify` gates per oracle discipline.

---

## 1. Motivation

The Kuramoto bank is not just a supersaw with nicer drift — it is a dynamical system with *emergent observables* that no individual voice contains. This proposal turns those observables into first-class modulation sources, then goes one structural step further: instead of a classical mod matrix (unidirectional routing gains from generator A's output to generator B's parameters), we borrow the **structure** of entangled quantum systems — shared joint state, conservation laws, measurement-as-conditioning, and population transfer — to couple two sound generators.

Honest physics framing, for the record: a classical coupled-oscillator system cannot be literally entangled. What we implement are *structural analogues*: (a) correlations sculpted on a shared state rather than signals routed between local states, (b) conserved budgets that force anticorrelation, (c) discrete measurement events that collapse conditional states, and (d) voice membership that coherently or stochastically transfers between generators. Each mechanism below is labeled with its physics inspiration and its actual classical implementation.

---

## 2. Assumed baseline (verify against current repo state)

- N sawtooth voices with phases θᵢ, natural frequencies ωᵢ = ω₀·(1 + dᵢ) where dᵢ is the seeded detune spread.
- Mean-field Kuramoto update (O(N)):
  - Order parameter: `R·e^{iψ} = (1/N)·Σ e^{iθᵢ}`
  - Per-voice: `dθᵢ/dt = ωᵢ + K·R·sin(ψ − θᵢ)`
- Coupling K and detune spread as user parameters.

If the current implementation uses pairwise O(N²) coupling, migrate to mean-field first — it is exact for global uniform coupling and makes R/ψ free byproducts of the update.

**Key dynamical fact to preserve and exploit:** the system has a phase transition at critical coupling K_c, which scales with detune spread. Below K_c: incoherent (classic lush supersaw). Above: locked (single fat saw). Near K_c: partial synchrony, metastable breathing of R, frequent phase slips. The musically rich regime is the neighborhood of the transition. Detune and coupling are antagonists; that antagonism is a feature to surface in the UI, not hide.

---

## 3. Phase A — Observable extraction (mod sources)

Extract at control rate (recommend a fixed control tick of 1–4 kHz; decision point, see §8). All outputs published to the mod-source bus, each in raw + one-pole-smoothed form with a per-source smoothing time constant.

| Source | Definition | Character |
|---|---|---|
| `R` | Magnitude of order parameter, ∈ [0,1] | Smooth, slow; coherence meter |
| `psi` | Mean phase ψ, unwrapped | Ramp; mostly internal |
| `drift` | dψ/dt − ω̄ (co-rotating frame; ω̄ = mean of ωᵢ) | Bipolar, slow; collective drift rate |
| `direction` | sign(drift), with hysteresis band ε around 0 | Ternary gate (−1, 0, +1) |
| `R2` | Second-order parameter `|(1/N)·Σ e^{i2θᵢ}|` | Detects 2-cluster states R misses |
| `lock_ratio_i` | Per-voice: fraction of recent window with \|θᵢ − ψ\| (wrapped) < threshold | Per-voice meters |
| `slip[i]` | **Event**: voice i's unwrapped (θᵢ − ψ) crosses ±2π; emit (voice_id, sign, tick) and reset accumulator | Discrete event bus |

Implementation notes:
- Unwrap ψ and per-voice (θᵢ − ψ) with standard phase-unwrap accumulators; never diff wrapped phases.
- `drift` needs light smoothing before publication (ψ is noisy near low R — when R ≈ 0, ψ is nearly meaningless; consider gating `drift`/`direction` confidence by R).
- Slip events are the natural **measurement bus** for Phase C/D. Route them alongside MIDI note-ons and an external transient-detect input as event sources of equal rank.

**Acceptance criteria (Phase A):**
1. Deterministic replay: identical seed + parameter script → bit-identical observable log across two runs.
2. K = 0 sanity: time-averaged R ≈ N^(−1/2) scale fluctuations (tolerance band in test).
3. K ≫ K_c: R → 1 within tolerance; slip rate → 0.
4. Sweep test across K: slip-rate curve peaks near the transition region; monotone decline above it.
5. Scope harness (visual-first): rendered plot of R(t), drift(t), and slip raster for three canonical configs, written to `traces/`.

---

## 4. Phase B — Coherence budget (conservation-law coupling)

**Physics inspiration:** monogamy of entanglement / singlet anticorrelation. **Classical implementation:** a conserved budget that couples *effective couplings*, not output signals.

Two generator instances G₁ (Kuramoto bank) and G₂ (second instance, or another synchrony-capable generator — see §8 open decisions). Define:

```
K₂_eff = K₂ · clamp(C_total − w · R₁, 0, 1)
```

- `C_total` ∈ [0, 2]: shared coherence budget (macro knob).
- `w` ∈ [0, 1]: entanglement weight — the "how monogamous" knob.
- Symmetric variant (both banks draw from the budget) is Phase B.2; start asymmetric (G₁ free-runs, G₂ constrained) for testability.

The budget acts on K (structural), **not** on output gain (mixer-level). This is deliberate: constraining K changes *what the system does*, constraining gain only changes what you hear of it. Flag this as a DECISIONS.md entry when adopted.

Three-generator extension (later): pairwise weights w_AB, w_AC, w_BC with row-sum constraint Σw ≤ C_total, so tightening one coupling necessarily loosens another. This produces the mod matrix whose entries constrain each other.

**Acceptance criteria (Phase B):**
1. With w = 1, C_total = 1: drive G₁ across its transition; verify R₂ falls as R₁ rises (anticorrelation visible in traces, correlation coefficient below a set negative threshold).
2. With w = 0: G₂ statistics independent of G₁ (correlation within noise band).
3. Budget conservation logged per tick; no NaN/denormal escapes under extreme settings.

---

## 5. Phase C — Membership spinors (voice blinking / population transfer)

**Physics inspiration:** two-level systems, Rabi oscillation, tunneling, quantum-dot blinking statistics. **Classical implementation:** per-voice complex pair with normalized power split across two render paths.

Each voice i carries a membership spinor `(aᵢ, bᵢ) ∈ ℂ²`, `|aᵢ|² + |bᵢ|² = 1`. Voice output:

```
yᵢ = sqrt(|aᵢ|²) · pathA(θᵢ) + sqrt(|bᵢ|²) · pathB(θᵢ)
```

(Equal-power rendering; the sqrt is on power weights — see §8 for the amplitude-vs-power decision.)

**Critical detail — phase carry-over:** both paths read the *same* θᵢ. A voice whose membership shifts into path B arrives phase-coherent with the ensemble it left and beats against the one it joined. The audible interference is the trace of the coupling scheme. Do not give path B an independent phase.

### C.1 Tunneling (stochastic, ship first)

Jump rate per voice, evaluated at control rate:

```
λᵢ = λ₀ · exp(−β · barrier) · heatᵢ
heatᵢ = smoothed |dθᵢ/dt − dψ/dt|   (or: recent slip count in window)
```

- `barrier` ∈ [0, ∞): macro knob. High barrier → rare jumps.
- Hot voices (fighting the mean field) tunnel more; locked voices stay put.
- On jump: swap dominant membership (a↔b rotation by π on the Bloch sphere), optionally with a short seeded raised-cosine ramp on the power weights (1–20 ms) to avoid clicks. Ramp length is a parameter, not wall-clock.
- RNG: dedicated named stream `rng.tunnel`, seeded from the project seed. Jump decisions via per-tick Bernoulli with p = λᵢ·Δt (Δt = control tick).

### C.2 Heavy-tailed dwell option (blinking statistics)

Alternative scheduler: draw dwell times from a bounded Pareto on [t_min, t_max] with exponent α ≈ 1.5 (all three parameters exposed). Produces long stable stretches punctuated by bursts of chatter — the quantum-dot blinking signature, far more organic than Poisson. Same RNG stream discipline. Selectable per-voice-bank: `scheduler: poisson | pareto`.

### C.3 Rabi oscillation (coherent, second)

Deterministic spinor evolution between events:

```
daᵢ/dt = −i(Δ/2)·aᵢ − i(Ω_R/2)·bᵢ
dbᵢ/dt = +i(Δ/2)·bᵢ − i(Ω_R/2)·aᵢ
```

- `Ω_R`: Rabi rate (how fast membership sloshes). `Δ`: detuning (Δ = 0 → full transfer A↔B; large Δ → shallow slosh).
- Integrate with a norm-preserving step (exact 2×2 unitary per tick via matrix exponential of the constant Hamiltonian — it's closed-form for a 2-level system; do this rather than Euler, which leaks norm).
- Renormalize |a|² + |b|² = 1 defensively each tick; log if correction exceeds tolerance (that's an integrator bug, not a feature).

Tunneling and Rabi compose: Rabi runs continuously; tunneling events apply discrete Bloch rotations on top.

**Acceptance criteria (Phase C):**
1. Norm conservation: max per-tick correction < 1e−9 over a 10⁶-tick run.
2. Rabi with Δ = 0, no tunneling: population |b|² oscillates sinusoidally at Ω_R (spectral peak test on the logged population).
3. Tunneling statistics: over ≥ 10⁴ seeded jump events, empirical jump-rate vs. heat matches λ formula (chi-square within tolerance); Pareto scheduler's dwell histogram matches bounded power law.
4. Phase carry-over: synthetic test where path A and path B are sine oscillators — verify beat frequency on transfer equals the analytic prediction.
5. Deterministic replay across the full spinor subsystem.

---

## 6. Phase D — Measurement bus & collapse

**Physics inspiration:** projective measurement, Born rule, decoherence/recoherence. **Classical implementation:** event-triggered probabilistic state selection with seeded RNG, plus relaxation back to superposition.

Event sources (equal rank, all timestamped in ticks): slip events from §3, MIDI note-on, external transient-detect, and hysteretic threshold crossings on R₁ (e.g., R crossing 0.7 upward). Event routing is a small matrix: which sources trigger which collapse targets.

On a collapse event targeting voice i (or a voice group):
1. Draw from `rng.measure`: voice collapses to path A with probability |aᵢ|², path B with |bᵢ|² (Born rule).
2. Set spinor to the pure state (1,0) or (0,1) — through the same click-guard ramp as tunneling.
3. Rebuild: relax the spinor toward its Rabi orbit (or toward equal superposition if Ω_R = 0) at rate `γ_rebuild` — the recoherence knob. Implement as slerp toward the target state on the Bloch sphere at fixed angular rate.

Conditional collapse across generators (ties Phase B and D together): a measurement on G₁ can trigger a *conditioned* operation on G₂ — e.g., "R₁ measured high" → snap G₂'s coupling toward the anticorrelated configuration implied by the budget, rather than merely modulating it. Keep this as D.2; D.1 is per-voice collapse only.

**Acceptance criteria (Phase D):**
1. Born-rule statistics: over ≥ 10⁴ seeded collapses at known |a|², empirical selection frequency within binomial confidence bounds.
2. Event determinism: identical seed + identical event script → identical collapse outcomes.
3. Rebuild rate: measured Bloch-angle recovery time matches γ_rebuild analytically.
4. No event starvation or double-fire under simultaneous events (tick-level tie-break rule documented and tested).

---

## 7. Suggested phase ordering & sizing

- **A → B → C.1 → D.1 → C.3 → C.2 → B.2/D.2.** Each phase independently shippable and gated.
- Architecture rung: **single-thread** is right-sized for A–C (this is one DSP core with a control-rate sidecar). Revisit only if the scope harness / trace tooling grows its own legs.
- Every phase adds: `./verify fast` unit checks (analytic + replay), `./verify full` statistical checks (the 10⁴–10⁶-sample ones), and at least one visual trace artifact.

---

## 8. Open decisions to surface (DECISIONS.md candidates — do not resolve unilaterally)

1. **Identity of G₂ / path B:** second Kuramoto bank, or the modal bank from the spectral resonator (gravity lens)? The modal-bank pairing is the more interesting instrument (R₁ → orb mass, slips → measurement events) but adds a cross-project dependency; per "writes stay home," that would go through a brief→response exchange rather than direct import.
2. **Membership rendering:** power-domain (|a|², equal-power sqrt as specced) vs amplitude-domain (Re/complex mixing, which would make spinor *phase* audible as inter-path interference). Amplitude-domain is more "quantum" and more chaotic; power-domain is predictable. Spec'd power-domain as default; amplitude-domain worth a spike.
3. **Control tick rate** for the coupling/spinor ODEs (1 kHz vs 4 kHz vs audio-rate coupling). Affects CPU and slip-detection resolution.
4. **Slip definition:** ±2π accumulator crossing (spec'd) vs lock-ratio-window dropout. Accumulator is cleaner; window is more robust at very low R.
5. **Budget target:** spec'd as acting on K₂ (structural). Confirm against any existing budget/macro conventions in the repo.
6. **drift/direction confidence gating by R** (see §3 note): publish gated, ungated, or both.

---

## 9. What this is not

- Not a claim of literal quantum behavior — §1 framing stands; keep the honest labels in the README per living-README doctrine.
- Not an AI-in-the-loop system — no model calls anywhere in the control or signal path.
- Not a mixer trick — the budget and collapse mechanisms act on system structure (couplings, state), never merely on output gains, except for the explicitly-labeled click-guard ramps.
