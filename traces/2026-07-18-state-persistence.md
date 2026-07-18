# 2026-07-18 — Session persistence proven; state_load race fixed; Windows deferred

**Human directives.** (1) Windows runtime work deferred until desktop
coordination — recorded in ROADMAP (moves out of the Phase 2 gate). (2)
"Make sure state is persistent across saved sessions" — answered with an
oracle, not an assurance.

**state_check (new, in verify full).** Links the impl statically, drives it
with a stub CLAP host: every param set to a non-default value through the
params extension → save → FRESH instance → load → (a) every get_value
round-trips exactly, (b) the restored instance renders BIT-IDENTICAL audio
for the same note (sessions restore sound, not just numbers), (c) a partial
/forward blob (old session, unknown keys) loads with defaults intact. 8/8
GREEN first run. Wrapper layers remain covered by pluginval (VST3 setState)
and auval (AU).

**Race fixed.** state_load previously called core.setParam directly from
the main thread — racing rebuild() against an active render(). Now
processing-aware (atomic flag set in start/stop_processing): idle → direct
apply (host reads values back immediately); processing → routed through the
param queue, with drainQueue's outgoing param events informing the host.

**Evidence.** verify full green (parity 30/30 identical worst case;
trajectories; state_check 8/8); pluginval strictness 10 SUCCESS; auval
SUCCEEDED; seals verified; sweep clean.
