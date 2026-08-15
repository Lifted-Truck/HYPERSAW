# routing-lab-modular — F-B modular routing lab (QM-3 send matrix + live graph)

- **Queue item:** F-B — modular routing page (stream B, first item; dispatched after the morph-law
  ruling and routing-view question were resolved by the human).
- **Why:** the human wants to feel out whether the QM-3 §2 pool (SRC…DRV2, MST, 36-cell
  upper-triangular send matrix) is legible as an editable matrix + live topology graph before any of
  it touches gui2, and to preview the B23 per-oscillator-source increment (OSC1/OSC2 rows) without
  implying it is already wired into the C++ audio path.
- **Evidence consulted:** `docs/design/fx-page-lab.html` (house style: CSS variable palette,
  panel/box structure, in-lab findings panel convention); `docs/received/QM-3-fx-pool-spec.md` §2
  (frozen rank order SRC..DRV2..MST, 36-cell matrix, inert-cell rule), §2.1 (matrix dimensions), §5
  (the five topology-ledger patches, used verbatim as presets); `docs/received/routing-morph-demo.html`
  (read for the bezier-cable graph-drawing approach — NOT copied, this lab uses its own single-source
  `legal()`/`effectiveGain()` derivation rather than the demo's separate `effectiveRouting()` +
  `curNorm` pair); `src/routing_core.h` (shipping semantics: `edgeLive()` — sources reach anything, a
  slot reaches only a later slot; the dense-table "0 = not connected" argument in the file's own
  header comment; `slotInit`, the crosspoint `in_i` term; `setSerialChain()`, reproduced verbatim as
  this lab's initial/reset state so it is never a fiction; the explicit warning against a second copy
  of "which edges exist," which is why `legal()` is called by both the matrix build and the graph
  draw and nothing else computes legality).
- **Alternatives rejected:** copying `routing-morph-demo.html`'s two-function split
  (`effectiveRouting()` returning `{eff,norm}`, then a separate `curNorm` lookup at render time) —
  rejected in favour of one `isNormalled(from)` / `effectiveGain(from,to)` pair with no cached
  intermediate state, since routing_core.h's own header explicitly names a second copy of "is this
  slot consumed" as the historical bug (a guard in a select handler let a backwards edge through when
  set directly on the model). Considered constraining OSC1/OSC2 to the same upper-triangular rule as
  the pool rows — rejected because routing_core.h's `edgeLive()` says sources reach anything, and
  QM-3's SRC row already sits at rank 0 where "any receiver" and "strictly later rank" coincide, so
  extending the pool's own rank rule to OSC1/OSC2 would have been an invented constraint, not a
  faithful one.
- **Verify:** `./verify fast`, exit 0, git `9e10dda`. `node tools/labharness/lab_load_check.mjs`
  (full sweep, 18 labs): GREEN, 0 broken, 0 skipped — includes the new file plus the 17 pre-existing
  labs/GUIs, confirming no regression.
- **Open questions:** (1) whether normalling should visually apply to the OSC1/OSC2 preview rows at
  all, since QM-3 never mentions them — this lab applies it for consistency with the pool rows, which
  is a design call, not a spec fact, and is flagged as such in the findings panel item 5. (2)
  Cable-colour-by-sender runs out of easily distinguished hues around the 7th–8th row (VRB/DRV2 sit
  close to CHO/DLY) — noted as an open finding (item 4) rather than fixed, since resolving it likely
  means reserving colour for corner-ownership instead (fx-page-lab's convention) and finding a
  different channel (dash pattern, curve height) for sender identity, which is out of this lab's
  scope (no corner-ownership implementation per the brief).
