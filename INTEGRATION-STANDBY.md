# INTEGRATION-STANDBY — plugin-skeleton library (passive)

> Standby artifact per the library's FOUNDATIONS notice (recorded in
> DECISIONS.md, 2026-08-08). Kept cheap and current; becomes the first brief
> when the mediator calls. No refactoring toward the library has occurred or
> will occur before the brief→response dialogue opens. Writes stay home.
> Last touched: 2026-08-08.

## (a) Friction list — infrastructure problems this project actually hit

Params / addressing:
- Flat id space filled silently: 99 params, ids 1..99, ZERO free slots on the
  day the multi-osc stride was ratified; stride 100 would have capped the
  instrument forever (fixed to 1000 pre-exposure — ADR-082 Amendment 1).
- Per-source vs patch scope cannot be decided in the abstract: 13 of 31
  "global" params were per-oscillator by construction (the core owned the key;
  the second core's copy existed but was unreachable).
- Two param-map idioms (`k == "x"` vs `eq(k,"x")`) across cores made a scope
  audit silently report 0 findings until the detector was fixed.
- Shell-domain params (transpose) kept ONE copy after the ADR classified them
  per-osc; writes to the second block silently dropped, and the GUI poll
  snapped controls back. Write path and read path wrong symmetrically.
- UI range ≠ modulation range: pitch knob honestly ±12, mods should reach ±48
  clamped — needs first-class support, else widened ranges ship invisible.

Modulation:
- No shell mod matrix exists; everything lives in a lab. Sources are coupled
  (K1..K8 are one rotor's voices, not independent LFOs) — a matrix design that
  assumes independent sources cannot express this.
- Source polarity is a property of the SOURCE (R is unipolar-negative below
  the coupling knee; ENV positive-only) and destinations may rectify —
  per-route polarity/markers needed, not per-source.
- Depth-of-depth (mod-on-mod) requested: each active routing's depth is itself
  a destination. Threshold-crossing enables need hysteresis (flip chatter).
- Parameter-sharing vs dynamics-sharing are different features (master K vs
  `link`); the registry must be able to express both without conflation.

Presets / state:
- Versioned text state (`hypersaw-state 1|2`), key=value; per-osc keys carry an
  `o<k>.` prefix. The per-osc preset TIER fell out of the prefix scheme nearly
  free — evidence that scoped presets should be derived from addressing, not
  designed separately.
- Three tiers ruled: patch / morph-corner (corners are GLOBAL across sources) /
  per-oscillator. Corner-tier format depends on the scope ruling — decided
  before implementation or saved presets get rewritten.

Voices / events:
- Velocity was ignored ENTIRELY for a month; pressure expressions unhandled.
  Per-voice gain seams (vel, smoothed pressure) had to exist before mapping.
- Voice steal: naive steal-oldest sacrifices deliberately-held notes under an
  arpeggio (release tails occupy slots ~1.1 s at env<1e-3). Three-tier policy
  (free → quietest releasing → oldest gated) fixed it; the ORACLE (Goertzel at
  the held note's own f0 vs a no-arp control floor) is the reusable half.
- NOTE_END bookkeeping must survive rejected pushes (ADR-079); tags key on
  note_id/port/channel/key.
- MPE note expressions currently reach only oscillator 0 (fan-out gap, open).

Host/GUI boundary:
- Consumers (visuals, meters, labels, preset scopes, mod destinations) must
  address a ROLE resolved per read, not an instance captured at wire time —
  and must RENDER which instance they resolved to, or a mis-resolution is
  unobservable. Three same-shaped incidents here (publishViz on osc 0, XY pad
  on raw base ids, the unregistered bind). This is the class FOUNDATIONS is
  explicitly being built to mitigate (human, 2026-08-09). See L0028.
- A callback registered in the host struct and assigned by the plugin but never
  BOUND in the webview layer is invisible to the compiler: the feature ships
  dead (`setVizOsc`, 2026-08-08 — visuals stayed pinned to oscillator 0 for two
  PRs). A registry that pairs member and binding in ONE declaration, or a
  startup assertion that every host member has a binding, removes the class.

Signal routing:
- Oscillator sum was hardcoded before the mixer existed; master volume did not
  exist at all once `vol` went per-osc. The audio context (mixer/master)
  should exist before the second source does — L0027, the headline lesson.
- Kuro-synced FX are a CLASS (any FX with N steerable parallel elements), with
  `link` (0..1 entrainment to a master swarm) as the shared idiom.

## (b) Component inventory

See `docs/integrations/corelib-insights.md` for the full donor manifest with
readiness grades and PR provenance. Compact form:
- Clean exports today: glide/travel core (+trajectory oracle), oscillator-preset
  format (+oracle), force system (already shared by five engines).
- Travel with their references: SAW swarm core, humanization/ensemble timing,
  three-tier voice steal (+arp-sustain oracle).
- Rulings portable, code lab-bound: quantum morph (ownership/territory/
  per-corner depths/hysteresis), mod-matrix semantics (scope vocabulary,
  polarity markers, depth-of-depth), Kuro LFO rotor.

## (c) Parameter + modulation architecture, half a page

- **Registry:** one static table (`kParams[]`: id, coreKey, name, range,
  stepped, labels). Ids are frozen and append-only. Osc 0 owns ids 1..999;
  oscillator k mirrors per-osc ids at `id + 1000k` (`kOscStride`); globals
  (~31, listed in `kGlobalIds[]`) exist once and their slots in higher blocks
  are never allocated. Params 100+ are post-amendment allocations (masterVol,
  global pitch).
- **Dispatch:** `applyParam(id)` clamps, handles shell-domain ids (taper maps,
  tune recompute, FX rack, engine gates), else routes `cores[osc].setParam(
  coreKey)`. `readParam` mirrors it (symmetric routing is load-bearing — a
  one-sided fix passed the round-trip oracle while broken). GUI receives ALL
  blocks as JSON and DERIVES the global set (an id with no +1000 sibling is
  global) — no duplicated list to drift.
- **State:** versioned text blob; osc-0 keys unprefixed forever (old sessions
  bit-identical), `o<k>.` prefixes for higher blocks; unknown keys skipped.
- **Modulation today:** engine-internal only — the Kuramoto coupling itself,
  per-voice envelopes with scatter, velocity + smoothed MPE pressure as
  per-voice gains, drift/onset systems. The user-facing matrix (sources
  K1..K8/R/LFOA/LFOB/ENV → 9 dests, scope vocabulary system/corner/morph-owned,
  per-corner depths, hysteresis) is fully specified and measured in
  `docs/design/mod-lab.html` but not yet in the shell. Glide module (4 travel
  laws + quantise, per-destination linked-by-default) has its core + oracle
  shipped, shell wiring pending.
