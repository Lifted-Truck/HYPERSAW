# Trace — five days of installs, one process that never let go

**Trigger** human 2026-08-29, three escalating reports: *"I think the old
version is still loading"* → *"none of the plugins have the latest updates"*
→ *"I'm not convinced you updated the actual plugin yet, just the reference"*.
All three were correct.

## The finding

Ableton's per-launch Log.txt: the session began **Aug 13**; it first mapped
horde.vst3 on **Aug 24 at 02:34**; every later "Going to create: horde"
(through 01:02 on Aug 29) reused that image. macOS maps a plugin dylib once
per process lifetime — replacing the file on disk is invisible to a running
host, and re-adding the device does not remap. Meanwhile every disk-side
check was truthful: the installed binaries embed a page byte-identical to
source (sha dbed6209, whole-page comparison, all three formats).

Both statements were true at once: the disk was current, the human was
looking at Aug 24. The gap between them was one process that had not quit.

## What changed

`./install` now lists every process holding a horde image whose HELD inode
differs from the same path's CURRENT inode (lsof -F pcni), names it loudly,
and says the fix (Cmd-Q, not close-the-set; then check the build stamp).
Detection only — killing a DAW mid-session is not the script's call.
Calibrated both directions, and the calibration caught a false-positive
design in the first draft (comparing all formats to the VST3's inode would
cry wolf on a host legitimately mapping the AU). L0045 files the lesson.

## Open

The human's actual verdicts on ADR-143 (lag) and ADR-144 (blob) are now
finally testable: neither ever ran in their DAW. Build stamp to confirm on
next launch: e759b7d or newer.
