# Trace — morph-on was destroying edited patches; repo renamed; three rulings filed

**Trigger** human 2026-08-26, five items in one message.

## 1. The repo rename ran

`gh repo rename horde`. Verified end to end: `Lifted-Truck/horde` resolves, the
remote is updated, the old `HYPERSAW` URLs still redirect (`git ls-remote`
succeeds against them), fetch works, all six gates green, **and the frozen
plugin id is untouched** — which was the whole constraint. The local directory
stays `HYPERSAW/` deliberately: renaming it mid-session would invalidate the
Bash cwd, the scratchpad, `$CLAUDE_PROJECT_DIR` for the hooks, and the memory
directory. The plugin descriptor's display URL is updated to the new name here
(it was redirecting, but reading stale).

## 2. ADR-127 — morph-on no longer overwrites an edited patch

**Reproduced before fixing, 5 of 5 parameters destroyed.** Edit voices→16,
detune→0.75, K→0.80, release→2.50, width→1.30; switch morph on; let the 5.8 ms
grid tick. Every one snapped back to its default.

**Mechanism.** `morphInit()` seeds all four corners with `defaultFor(...)`,
reasoning it is *"silence-safe: every corner agrees"* — true of a **fresh**
instance where live == default. But it is guarded by `if (!morphIds.empty())
return;` and called from readParam/state paths, so it runs at startup, long
before editing. The corners then hold stale defaults and the first grid tick
after morph-on writes them over the sound. The comment was right about the case
it was written for and silently wrong about every other.

**Fix.** Morph-on adopts the live value into **all four** corners when
`morphCornersAuthored` is false — all four, so morphInit's silence-safe property
is preserved exactly. The flag is set by capture, armed edits, exempt writes,
corner-preset apply, and a state chunk that carried corners, so a **loaded
preset is never clobbered either**: the destructive direction is closed both
ways.

**Chose the fix over the human's other option (a warning dialog) deliberately.**
A warning that says *"you are about to lose your sound"* is worse than not
losing it — the dialog exists only because the behaviour is wrong. And
adopt-live is already this codebase's recorded lean, at `morphToggleExempt`:
*"the corners honestly record what was playing."*

**Control fired:** with the fix stashed and rebuilt, the same probe reports
**5 of 5 destroyed**; with it, 0 of 5. Parity 156/156 unchanged — `morphOn`
ships off, so no golden reaches the new branch.

## 3. ADR-128 — audio feedback is per-sample

Human took the recommendation. Block-rate was rejected on one number: 2.9 ms at
128 @ 44.1 kHz is a flanger, and it **varies with host buffer size**, which
violates our determinism rule the way a wall-clock read would. FOUNDATIONS'
OQ-23 is not overruled — it was ruled for the *modulation* graph. **That
divergence is worth filing to them**, since their register watches for exactly
this: a second consumer finding a doorframe's rate assumption does not transfer
across domains.

## 4. B55 and B56 filed

**B55, control-order pass.** Cheap to do because the order is *generated*:
`param_presentation.tsv` (241 rows) is the source and `gen_gui_controls.py`
regenerates the markup, gated by verify — so a reorder is a TSV edit, with no
hand-editing and no drift. It is a session rather than a ticket because the
right order is a judgement about workflow. Bring the 34 undesigned and 5
ungrouped rows `presentation_check` already reports.

**B56, the manual agent.** The notification design should reuse three signals
this repo already maintains: `param_presentation.tsv` diffs (a control moved),
`feature_tests.tsv` rows (a new documented behaviour), and `traces/` (what
changed and why). No new watcher needed, and nothing that went through the
normal process can be missed. **Honest gap:** none of them capture sound or
feel, which is most of a synth manual — so the hook says *what* changed, never
*how it behaves*. Constraints fixed now: the agent is read-only here and files
via the INTEGRATIONS mailbox (a second writer in ROADMAP breaks the
single-writer rule), inherits the build-hash screenshot convention, and needs
ADR-014 alias discipline since the manual is public-facing. **B55 first** — a
manual written against the current order documents a layout we intend to change.

## Verify

`./verify fast` exit 0; `parity_check` 156/156 (worst 4.262e-09). Built,
installed to both formats, hash-verified against the build **before** signing,
signed, seals verified, AU cache reset.
