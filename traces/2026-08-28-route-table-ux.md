# Trace — the menu shrinks to one verb; the table gains the modulator

**Trigger** human 2026-08-28: *"it should just be 'send to mod matrix' and it
sends it to a table and from there you can decide which modulator to link it
to (or x to release it)"* + *"a 'release all modulators' option"*. Built from
the design in `docs/plans/2026-08-28-day-plan.md` §1.

## What changed

Shell: `modSetSource(idx, srcSlot)` (refuses synthetic dests and out-of-range
slots) + hostIf lambda + `hzModSource` bind; deleted modSetDepth's stale
index-0 comment. GUI: PARAM_MENU's three mod entries → two (`modSend`,
`modRelease`), the `keep:true` submenu mechanism deleted with its last user;
route rows gain a source `<select>` (disabled on the pitch route); MODROUTES
refreshed at right-click so the guards are truthful; row relaid to two lines;
MOD note rewritten. Test rows B69-7/B69-9 retargeted off the retired menu.

## Evidence

In-browser with a mutating stub table: unrouted knob shows
["Reset to default", "Send to mod matrix"]; routed shows ["Reset to default",
"Release modulator"]; two routes to one dest shows "Release all modulators
(2)"; **release-all removal order [2, 0]** with the unrelated `3->9` route
surviving — the descending-order proof. Matrix's own knob (166) shows only
Reset. Row render: three routes, labels 121px wide (they were 0px in the
five-across layout — the defect that forced the two-line row), pitch select
disabled, sources read ENV 2 / Macro 3 / Macro 1, no horizontal overflow.
parity 156/156, mod_check GREEN, verify fast exit 0, lab_load_check GREEN.

## Process note

The browser pane reported `visibilityState: hidden` / innerWidth 0 mid-run and
every measurement came back garbage (clusterW 23) — a screenshot forces the
paint. Second sighting today; it is in the day plan's process notes.

## Open

B72's morph-transition ruling (framed for the human, not built); B73's Delay
module is the next build; B70 depth-mod now has a seat on line two of the row.
