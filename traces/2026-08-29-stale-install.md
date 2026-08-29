# Trace — "the old version is still loading", and it was

**Trigger** human 2026-08-29: *"I think the old version is still loading."*

## The finding

Not a GUI bug. Two defects in `./install`:

1. It never copied the CLAP. `horde.clap` is built; only the VST3 and AU were
   installed. `~/Library/Audio/Plug-Ins/CLAP/` held only `HYPERSAW.clap` from
   **Aug 25** — on a CLAP-first plugin, the format we develop against was the
   one guaranteed stale.
2. Legacy `HYPERSAW.vst3` / `.component` (Aug 27) sit beside the current
   `horde.*` bundles declaring the SAME frozen id and AU triple (ADR-002), so
   a host may resolve to either.

## Evidence

Embedded-string comparison (L0042: identifiers do not survive compilation, so
look for strings only the new source can contain):

| bundle | specimenBox | scaleMax | date |
|---|---|---|---|
| HYPERSAW.vst3 | 0 | 0 | Aug 27 |
| HYPERSAW.component | 0 | 0 | Aug 27 |
| HYPERSAW.clap | 0 | 0 | Aug 25 |
| horde.vst3 / .component / .clap (after fix) | 2 | 4 | today |

## The correction I owe

Several PRs this week closed with "installed; auval SUCCEEDED". With two
components sharing one AU triple, auval may have validated the Aug 27 bundle.
Reported in good faith; not trustworthy as written. ADR-145 records it.

## What changed

`./install` signs and installs the CLAP, creates the dirs, names every legacy
bundle with its date and the exact `rm -rf` on each run, and prints the
installed inventory with timestamps. It does NOT delete: that is the human's
call (B78).

Also: the item I first cited in the warning text (B58-4) did not exist in
ROADMAP.md — it lived only in session memory. Filed properly as B78 and the
citation corrected. A pointer to a roadmap entry that is not there is the same
failure class as a test row naming an oracle ./verify does not run.
