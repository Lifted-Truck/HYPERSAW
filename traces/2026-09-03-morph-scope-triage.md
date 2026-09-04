# 2026-09-03 — B93 triage: the ADR-108 hold wearing a corner colour

**What changed.** New diagnostic `tools/morphscope_probe.cpp` (edit a param
under mode × arm × puck position, read every corner back by pinning the puck).
Its truth table isolated the human's "globals act like corner params" report:
quantum-mode edits land in exactly ONE corner whenever the parameter's
enabling dependency is satisfied in the winning corner; when it is not, the
ADR-108 hold means the field applies no corner at all, so the live value shows
everywhere while the owners JSON still painted the winner's colour. Landed:
`morphOwnersJson` reports −2 (held) via `depLiveInCorner`; the GUI marks the
row "· held" with no corner colour. B93 filed with the measurements and the
ruling the human owns (keep vs drop the hold; lead recommends drop). B94 filed
for the mass-spring + quantise report with a ranked hypothesis list and a
GlideCore-direct reproduction recipe as its first move.

**Evidence consulted.** morphscope_probe runs (bendTime/bendRate/bendLaw/detune
across 8 contexts each, with and without the dependency authored); the dep
rules in `src/depends_graph.h` (107-113 gated per-law on 106); morphRouteEdit
and morphStep read in place; no morphIds duplicates (checked).

**Verify.** `./verify full` exit 0; lab_load 26/0; morphscope_probe is a
diagnostic, not a gate (prints, never judges).
