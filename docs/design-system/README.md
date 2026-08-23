# Design system

## What is here

| file | what it is |
|---|---|
| `HORDE-UI-Spec.dc.html` | **The UI implementation spec, v1** (2026-08-23). Normative. Open it in a browser. |
| `support.js` | The Claude Design canvas runtime the `.dc.html` loads. Vendored beside it **only** so the spec renders from a checkout; it is not ours and is not edited. |

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

## Not yet delivered

The spec's own last line points at a companion:

> Page compositions live in the Design Directions file, turns 6a–10a.

That file has not arrived, and neither has `HORDE Design System.dc.html`. So we
have the **tokens and widget behaviour** but not the **page layouts**. That is
the right half to have first — layouts can be re-derived from tokens, tokens
cannot be re-derived from layouts — but B37's page work is blocked until they
land.
