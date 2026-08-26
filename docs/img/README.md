# Screenshots

Drop PNGs here; the top-level README wires them in.

## Wanted (post-aesthetic, 2026-08-26)

**All five current shots are stale.** They were taken 2026-08-23, before the knob
pass, the light/dark chassis, the five screen schemes, the warping wordmark, the
SET page and the corner-ownership stripes. The README now says so in a note under
the hero image rather than pretending otherwise — but retaking them is the fix.

**Two extra slots the new aesthetic earns**, because the design's central claim is
that chassis and screen are *independent* axes and a single-mode screenshot set
cannot show that:

| file | what it must show |
|---|---|
| `gui-dark.png` | any page on the **dark chassis** — the same page as one of the light shots, so the pair reads as one instrument in two modes rather than two instruments |
| `gui-schemes.png` | one well under two different **screen schemes** (e.g. TUBE vs FROST) on the *same* chassis, which is the only way to show the axes are separate |

Keep the conventions below. The build hash matters more than ever now that the
interface is moving weekly.

## Wanted (GUI2, 2026-08-23 — superseded, kept for the record)

The existing `gui-overview.png` shows **GUI1**, which became the legacy interface
on 2026-08-23 — so it now illustrates the wrong instrument. Replacing it is the
one blocking shot; the rest are wanted but the README does not reference them
until they exist (a broken image is worse than a missing one).

| file | page | why it earns a slot |
|---|---|---|
| `gui-overview.png` | **MAIN** | the hero image — replaces the GUI1 shot the README currently carries |
| `gui-osc.png` | OSC | where the instrument is shaped: swarm, detune law, drift, saw shape |
| `gui-mix.png` | MIX | per-oscillator strips, meters, master — the two-oscillator story |
| `gui-fx.png` | FX | the rack and its slot types |
| `gui-morph.png` | MORPH | the XY pad with corner colours; the one page nothing else in the repo shows |

## Conventions

- **Leave the build hash visible** in the corner. It is what lets a reader say
  which code drew the picture, and it is why the caption can name a build at all.
- Capture at a **stable window size** so the pages look like one set rather than
  five unrelated screenshots. The GUI is responsive, so a differently-sized
  window re-flows the columns.
- **PNG, not JPEG** — this is a dark UI with thin lines and small text, and JPEG
  ringing on that is ugly at any quality setting.
- If a page changed since its shot, either retake it or say so in the caption.
  A dated, admitted-stale screenshot beats a confident one that quietly lies —
  the same rule the README's status line follows.

## Note on the current file

`gui-overview.png` is still the GUI1 image at time of writing, and the README's
caption already describes it as GUI2's MAIN page. That mismatch is deliberate
and short-lived: the caption describes what the slot is *for*, and the file is
being replaced. If you are reading this and the image still looks like a single
long column, the replacement has not landed yet.
