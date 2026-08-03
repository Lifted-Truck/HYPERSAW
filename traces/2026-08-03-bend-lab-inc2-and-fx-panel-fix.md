# 2026-08-03 — bend lab increment 2 + FX panel: Comp/Comb were unreachable

## Changes
- `src/gui/gui.html`: all four FX slot dropdowns widened 0..3 → 0..5 (Comp, Comb).
- `docs/design/bend-lab.html`: constant-time glide as its own model; dist→overshoot
  control; note-pitch lane promoted to default; per-note (MPE) bend lane; per-key
  note release with a real held stack.
- `ROADMAP.md`: FX fold-status record + bend increment 2.

## The GUI bug — shipped but invisible
The human asked to record that the comb "was only in a lab". Checking rather than
recalling found a third answer: the **Karplus-Strong comb has been shipped since
ADR-071** as FX rack slot type 5 (`src/fx_rack.h`, ids 57/59/61/63) — but when
ADR-071 widened those params 0..3 → 0..5, **the four GUI dropdowns were never
widened with them**. Comp and Comb were automatable from the host and absent from
the plugin's own panel. Fixed.

Neither party's recollection was right, and the wrong recollection is what
surfaced the defect. Worth generalising: **a param range widened without its GUI
control ships an invisible feature**, and nothing in the oracle suite can see it —
every gate tests the audio path, and the audio path was correct.

Still true and still recorded: the E1 swarm filter + notch (`filter_core.h` /
`notch_core.h`) are NOT in HYPERSAW's rack; they live in the separate SWARM-FX
plugin. That is the lab that has not been folded.

## Bench increment 2
Human's set: constant-time glide, constant-cents glide, lag, spring (+ a slider for
how much distance influences overshoot); note pitch is "the whole crux"; apply to
MPE too.

**Constant time was genuinely missing.** The old "lag" is a one-pole — asymptotic,
never arrives. Constant-time portamento latches velocity from the move distance and
arrives on schedule. These are different instruments and conflating them would have
answered the human's request with something that isn't what they asked for.

**dist→overshoot.** Their "if that isn't already how it works" is *partly yes*: a
linear spring overshoots by a fixed percentage, so overshoot in cents already scales
with distance — that is k=1. The knob generalises to overshoot ¢ ∝ distance^k,
implemented by inverting the closed-form ζ↔overshoot relation to find the damping
that *produces* the wanted overshoot, rather than scaling the output (which would
have broken the physics while looking right).

**MPE.** Each note carries its own bend inertia and its own latched target. It maps
naturally at fold time: `setNoteExpr` already writes per-voice `noteTune`
(ADR-036/038), so this is one filter instance per voice, no new plumbing.

## Evidence
Calibration slices the characterisation code **out of the lab** and runs it in node,
so it cannot pass while the shipped lab is wrong.

| property | measured | closed form |
|---|---|---|
| const-time T=200, 2 st → 50 % | 99.77 ms | T/2 = 100 |
| const-time T=200, **12 st** → 50 % | 99.77 ms | T/2 = 100 (distance-independent) |
| const-rate 24 st/s, 2 st vs 12 st | 41.36 / 249.98 ms | 41.67 / 250.0 (6×) |
| lag τ=60 → 50 % | 41.36 ms | τ·ln2 = 41.59 |
| lag settle ±5 ¢ | 221.32 ms | τ·ln40 = 221.33 |
| lag vibrato depth @ 5 Hz | 46.89 % | 46.86 |
| spring ζ=0.6 overshoot | 18.76 ¢ | 18.96 |
| spring ζ=1.0 | 0.00 ¢ | must not overshoot |
| dist→overshoot k=1, 6× distance | ratio 6.00 | 6 |
| dist→overshoot k=0, 6× distance | ratio 0.97 | 1 (constant absolute) |
| dist→overshoot k=2, 3× distance | ratio 9.07 | 9 |

Planted bad (ζ=0.06): 5 reversals, 166 ¢, never settles.

Note lifecycle driven through real key events in the page: poly 3 held → release
middle → other two still gated; mono hold A then G → release G → falls back to A,
still gated → release A → silent. MPE latch driven through `render()`: bend A to
+2, play B, wheel to −1 → A holds +2, B follows.

`./verify full` exit 0 at 78055b8. parity_check 147/147 (worst 4.262e-09 @
dyn-ring.seed42); trajectory/state/notefuzz/waveshape/force/spectra/filter/notch/
swarmalator/time all GREEN. Installed 78055b8.

## Bugs found by the calibration
1. **Move-distance rearm.** The distance tracker rearmed the instant the error
   crossed zero — but a spring crosses its target *at full speed on the way to
   overshooting*, so it re-derived its own damping mid-overshoot. dist→overshoot
   measured distance^2.5 instead of distance^2. Rearm now requires arrived AND
   stopped. This is the kind of error that reads as "close enough" by ear and only
   a closed-form check catches.
2. **Note-off gated everything.** Human-reported. Releasing any key silenced all
   sounding notes, which made the glide question unauditionable — glide is defined
   by what "still held" means. Replaced with a held stack (per-key release in poly;
   last-note priority with fallback in mono), avoiding the 2026-07-29 phantom-key
   bug by construction rather than rediscovering it.
