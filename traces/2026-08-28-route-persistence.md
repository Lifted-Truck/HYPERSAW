# Trace — routes survive the session; the identity arrives before the morph

**Trigger** SESSION.md first move (human-ratified close 2026-08-28): route
persistence keyed on B72's deterministic link identity.

## What changed

`src/hypersaw_clap.cpp` only: modRoutesChunk()/applyModRoutesChunk() (one
serializer, canonical `src:dest:depth;` merged by (src,dest)); `modroutes=`
line in state_save/load; `"modRoutes"` string in stateJson/applyStateJson
(schema 2→3); param 161 and its readback find the pitch route BY DEST — the
index-0 assumption was corruptible before persistence and wrong after it.

## Evidence

routepersist_probe (scratchpad, linked against the shipped impl lib): 10/10 —
listed in ADR-138. Gates: parity 156/156 (worst 4.262e-09), state_check,
preset_check, mod_check green, ./verify fast exit 0. Routeless state emits no
line, so pre-existing chunks round-trip byte-identically.

## Open

B71 (table-side route UX + release-all) and B72's morph-transition semantics
(human decision) are the next mod-matrix moves; B73 (Echo/Room feedback,
designed not ported) and B74 (CHROME-001 visualizer) queued behind them.
