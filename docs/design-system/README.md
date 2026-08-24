# Design system

## What is here

| file | what it is |
|---|---|
| `HORDE-UI-Spec.dc.html` | **The UI implementation spec, v1** (2026-08-23) — tokens, widget state tables, visualizer recipes, morph presentation. Normative for **values**. |
| `HORDE-Design-System.dc.html` | **The consolidated system** (2026-08-23) — page compositions for MAIN, OSC, FX, MIX and the bend+scale panels, with the reasoning. Normative for **layout**. |
| `support.js` | The Claude Design canvas runtime both `.dc.html` files load. Vendored beside them **only** so the specs render from a checkout; it is not ours and is not edited. A separately-delivered `hp-support.js` was byte-identical (same SHA-256), so there is one copy, not two. |

The spec is kept **as delivered**, not transcribed into markdown. A normative
document with two copies is a normative document that will disagree with itself,
and this one is 200-odd exact values — hexes, pixel counts, angles, timings —
which is precisely the content transcription gets wrong. Read the `.dc.html`.

## How to read it

The spec declares its own precedence, and it is worth repeating here because it
decides every future argument about it:

> §1–7 are format-neutral values … normative for both stacks. §8 is the webview
> build … §9 is the native port appendix. **Where they conflict, §1–7 win.**

So: §1–7 is the design. §8 is one implementation of it (the one we ship today),
§9 is a second implementation for a C++ painter that does not exist yet. A
question like "what colour is a knob's filled arc" is answered in §1, never in
§8's CSS.

## Status: SPECIFIED, NOT BUILT

Nothing in `src/gui/gui2.html` implements this yet. The shipped interface is a
**dark** theme; the spec is a **cream** one. This is a re-skin, not a tweak, and
it is tracked on ROADMAP (B37) rather than applied piecemeal.

Read `DECISIONS.md` ADR-116 for the intake analysis: what the spec changes, what
it specifies that does not exist yet, and the one trap that must be fixed before
any of it lands (the corner palette currently exists twice in gui2, and the two
copies already disagree).

## Which document wins

Both, on different questions, and they say so themselves:

- `HORDE-UI-Spec.dc.html` — **values**. "§1–7 are format-neutral values …
  normative for both stacks."
- `HORDE-Design-System.dc.html` — **compositions**: what goes on MAIN, OSC, FX,
  MIX and the bend+scale panels, and the reasoning for each layout. Its own
  header defers on values: *"Values are normative in HORDE UI Spec."*

So a question about a hex, a radius or a timing is answered by the UI Spec even
if the Design System shows something else; a question about what a page contains
is answered by the Design System.

### Where they actually disagree

Found by reading them against each other (ADR-117). The value conflicts are
settled by the rule above — **UI Spec wins** — and are listed so nobody
"corrects" the implementation toward the losing side:

| | UI Spec (wins) | Design System |
|---|---|---|
| window | 1180 × 820, min 1000 × 700 | 1180 × 780, min 1000 × 660 |
| shadow | 1.5px 1.5px 0 | 2px 2px 0 |
| meter fill | `#A6F219` | acid `#9BE514` |
| `well` | `#FFFFFF` (`well-alt` = `#E9E3D4`) | `#E9E3D4` |

Two conflicts are **not** value questions and are **unresolved — they need a
human ruling**, recorded on ROADMAP as B37's blocking questions:

1. **Glyphs.** UI Spec: "NO glyphs anywhere", corner identity is colour + preset
   name. Design System: every corner carries one (GLASS ◆ / GRIT ▲ / HOLLOW ● /
   BLOOM ■) and "glyph always rides with colour". This is an accessibility
   argument as much as an aesthetic one — a glyph is how corner identity
   survives colour-blindness, which is why the UI Spec's own §8 offers preset
   chips for that job instead.
2. **Re-theming.** UI Spec: "corner colours never re-theme". Design System:
   "corner identity becomes per-direction, not global." These cannot both hold
   if a second visual direction ever ships.
