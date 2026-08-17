# Dispatch brief — aesthetics lab

**Provenance.** HYPERSAW lead organ, 2026-08-17, for a scoped subagent with zero
conversation history. Motivating request (human, verbatim): *"could we start a lab
for testing alternative aesthetics? I want to play around with a lighter color
scheme, styles like skeuomorphism, rotary dials instead of sliders, etc."*

Committed per our standing commitment to FOUNDATIONS that round-2 dispatch briefs
are readable in our tree. Nothing here derives from any FOUNDATIONS path.

## Files in scope

- **CREATE** `docs/design/aesthetics-lab.html` — one self-contained file.

**OUT of scope:** `src/**` (this changes nothing about the shipping GUI),
`ROADMAP.md` (lead is the only writer), `verify`, any other lab, `tools/**`.
Do not commit. Leave the file in the working tree and report.

## What it is for

A bench for auditioning the *look and feel* of the instrument's controls, so
aesthetic direction is decided by comparing rendered options rather than by
imagining them. It is **not** connected to the plugin and must not pretend to be
— no bridge, no param ids, no fake values that imply a live instrument.

## Requirements

1. **Theme tokens, not hard-coded colour.** Every colour, radius, shadow and font
   comes from CSS custom properties on a theme class. Switching theme must be a
   *token swap*, never a re-layout. Ship at least: the current dark scheme, a
   **light** scheme, and a **skeuomorphic** scheme (bevels, gradients, inner
   shadow, tactile). Adding a fourth must mean adding one token block.
2. **Rotary dials.** Implement a `drawDial(ctx, opts)` on canvas — value arc,
   pointer, and a tick/detent option — driven by pointer drag (vertical or
   angular, your call; say which and why in a comment). Must be keyboard
   accessible (arrows adjust, visible focus).
3. **Side by side, same content.** The SAME small control set — a few continuous
   params, one enum, one toggle, one bipolar — rendered simultaneously under
   every theme and in both widget styles (slider vs dial), so the comparison is
   direct. **The point of the lab is the comparison, not any one rendering.**
4. **Enums render as enums.** A stepped parameter with labels must render as a
   real selector, not a numeric slider. (This lab exists partly because our GUI
   generator got that wrong.)
5. **Density/scale control** — a single slider that scales control size, so
   "does this survive at plugin size?" is answerable in the lab.

## Hard constraints

- **NO React, no framework, no CDN, no build step.** Every lab here is a single
  self-contained HTML file. Plain JS.
- **Must pass `node tools/labharness/lab_load_check.mjs docs/design/aesthetics-lab.html`.**
  That harness evaluates `<script>` blocks in a vm with proxy DOM globals, so:
  no `fetch` (it is undefined there — it has already broken one lab), no
  top-level access to a `const` before its declaration, and nothing that throws
  at setup.
- Follow the house lab conventions visible in `docs/design/bend-lab.html`: a
  `rev` stamp using `document.lastModified` plus a script fingerprint, a
  `tagline` explaining what the bench is for and what question it settles, and
  `hint` prose next to controls explaining *why* a control exists.
- **Comment the why, never the what** (see `CLAUDE.md`). Where you make an
  aesthetic or interaction judgement, say what you rejected and why.

## Acceptance criteria

1. The lab load check passes.
2. Opening it shows every theme at once, with the same controls, in both widget
   styles.
3. Changing theme changes no geometry — verify by asserting in a comment which
   properties are token-driven.
4. A dial is draggable and keyboard-operable; an enum is a selector.
5. Nothing in the file implies a connection to the running plugin.

## Report back

The file, what you chose for dial interaction and why, anything you could not do
within the single-file no-framework constraint, and — required — any aesthetic
decision you made that you think the human should overrule.
