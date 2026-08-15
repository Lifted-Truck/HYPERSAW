# notch-fx-slot — add NOTCH as FX rack slot type 6

- **Queue item:** parallel-streams round, stream A, first item (dispatched by
  the lead session; brief origin: HYPERSAW/synthetic-worlds, 2026-08-15).
- **Why:** `NotchCore` (Track E1.2, `src/notch_core.h`) was fully ported and
  oracle-covered (`notch_check`, parity rms 0) but unreachable from the
  shipping FX rack — `FxType` had no Notch value, the fx*type params were
  clamped 0..5, and no GUI offered it. This wires the existing, correct core
  into the rack as slot type 6 so it is actually selectable, without touching
  the core itself.
- **Evidence consulted:** `src/fx_rack.h` (Slot/FxType/processSlot shape,
  Off's bit-exact-passthrough contract), `src/notch_core.h` (constructor,
  `processExternal` vs `render`, `setParam` key list — confirmed `mix` is not
  one of the rebuild-triggering keys), `src/hypersaw_clap.cpp` (param table,
  `kFxTypeLabels`, the generic `id >= 57 && id <= 64` rack dispatch — already
  type-agnostic, no change needed there beyond the range/label), `tools/mixer_check.cpp`
  and `tools/steal_check.cpp` (CLAP-factory harness scaffold, fresh-instance-
  per-case pattern), `tools/gui_reach.py` (patch-scope/data-fixed enforcement),
  `verify` (gate ordering — placed next to `routing_check`, the other
  rack/slot-level invariant gate).
- **What changed:**
  - `src/fx_rack.h`: `FxType::Notch = 6`; one `std::unique_ptr<NotchCore>` per
    slot, (re)constructed in `setSampleRate` (main thread only — RT-safety);
    `case FxType::Notch` in `processSlot` maps the slot's `amount` to the
    core's own `mix` param and calls `processExternal` in place (verified
    alias-safe: each sample's `dry` is read before its output is written).
  - `src/hypersaw_clap.cpp`: widened the four `fx*type` param ranges 0..5 →
    0..6; added `"Notch"` to `kFxTypeLabels`.
  - `src/gui/gui.html`, `src/gui/gui2.html`: added `<option value="6">Notch</option>`
    to all four FX-slot-type `<select>`s in each file (8 total), preserving
    the existing `data-fixed="1"` on the `<select>` itself.
  - `tools/notchslot_check.cpp` (new): drives the real plugin through the CLAP
    factory. Two must-read-nothing controls (Off bit-exact to no-slot; Off
    ignores `amount` entirely) plus a Goertzel spectral-distance assertion
    (Notch@amount=0.5 vs Off) with a MEASURED floor — see below.
  - `CMakeLists.txt`: added the `notchslot_check` target (links
    `${PROJECT_NAME}-impl`, mirrors mixer_check/steal_check).
  - `verify`: added `"$build_dir/notchslot_check" || return 1` to `full()`,
    placed immediately after `routing_check` (the other rack/slot invariant
    gate) — an explicitly in-scope edit per the brief.
- **Alternatives rejected:** measuring only total RMS/level instead of a
  multi-bin spectral distance — rejected because a scalar-gain wiring bug
  (e.g. a stray gain misrouted to id 57) could pass an RMS-only check by
  accident; the spectral metric only moves if the harmonic *shape* changes.
  Giving Notch its own dedicated GUI param page (beyond the existing generic
  `tone` knob) — out of scope per the brief ("do not restructure Comb… those
  are separate ruled items" implies the same posture for a new per-slot page).
- **Oracle floor/threshold, measured not guessed:** the spectral-distance
  floor was measured from a render where the notch effect is genuinely
  absent (no-slot vs. explicit-Off, same patch) — bit-exact 0, not a rounded
  guess: this signal path has no RNG and no wall-clock read (SPEC §5.7), so
  zero is the honest number. Notch@amount=0.5 vs Off measured 0.0180 —
  four orders of magnitude clear of the 1e-6 assertion bar, nowhere near the
  "within 2x, threshold is wrong" zone.
- **Calibration (green→red→green):** planted a no-op in `case FxType::Notch`
  (comment left in place during the plant, reverted after), deleted the stale
  `build-release/CMakeFiles/HYPERSAW-impl.dir/src/hypersaw_clap.cpp.o` before
  each rebuild (the L0032 stale-binary trap), rebuilt just the
  `notchslot_check` target. Plant: spectral distance dropped to exactly 0.0,
  oracle FAILed (`notchslot_check: RED (1 failures)`, exit 1). Reverted,
  rebuilt again (object deleted first): back to
  `notchslot_check: GREEN (0 failures)`, exit 0.
- **Verify:** `./verify full`, exit 0, git `9e10dda` (pre-change HEAD; work
  left uncommitted in the worktree per the brief). `parity_check: 147/147
  scenarios within eps=1e-06 (worst 4.262e-09 @ dyn-ring.seed42)` — unchanged
  from before this change, confirming the FX rack addition did not perturb
  the goldens. Every other gate GREEN (no RED/FAIL lines in the run).
  `python3 tools/gui_reach.py`: GREEN — gui.html 102/105, gui2.html 30/105
  (both unchanged coverage counts other than the Notch option now existing
  inside the already-counted fx*type ids).
- **Open questions:** none for this item's scope. Noted but NOT acted on
  (out of scope): `mixer_check` is still not wired into `./verify` (a
  pre-existing gap, unrelated to this change) — left as-is per the brief's
  explicit scope (only `notchslot_check` was to be added to the gate set).
