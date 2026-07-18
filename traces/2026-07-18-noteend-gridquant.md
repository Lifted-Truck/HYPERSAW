# 2026-07-18 — NOTE_END bookkeeping (retrigger fix) + grid quantization

**Retrigger report (human).** Same-pitch retrigger before the tail ends
neither overrides nor overlaps in Live. Root cause: the plugin never sent
CLAP_EVENT_NOTE_END — a spec obligation skipped in the Phase 0 skeleton —
so the host's per-note bookkeeping believed old notes were still alive and
withheld same-pitch retriggers. The engine itself overlaps fine (alloc
gives releasing voices a fresh swarm). Fix: core.noteOn returns its slot
(parity-neutral API addition + read-only swarmAt accessor); the shell tags
each slot with the host note identity (note_id/port/channel/key) and emits
NOTE_END from process() when a tagged swarm retires (gate off AND env <
1e-4, matching render's own skip test). Needs the human's Live re-test for
final confirmation — expected result: retaps overlap the releasing tail.

**Grid quantization (human request).** Grid cycles/beat now snaps to 12
rational divisions (1/4, 1/3, 1/2, 2/3, 3/4, 1, 3/2, 2, 3, 4, 6, 8);
param 23 still stores the actual value (state-compatible), applyParam
snaps, value_to_text names the fraction ("2/3/beat"), GUI is a select.
Audibility experiments recorded as a ROADMAP item linked to the Phase 3
grid-status readout (ADR-016/017).

**Evidence.** verify full green (parity 30/30 identical worst case;
trajectories incl. L0-12 green); pluginval strictness 10 SUCCESS; auval
SUCCEEDED; seals verified; sweep clean.
