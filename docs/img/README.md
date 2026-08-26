# Screenshots

Drop PNGs here; the top-level README wires them in.

## STATUS (2026-08-26)

**Tier 1 is DONE and wired into the README** — `gui-hero.png` (MAIN, dark, TUBE),
`gui-light.png` (MAIN, light, EMBER), `gui-osc.png`, `gui-morph.png`. The old
2026-08-23 set was deleted with them, and the README's dead references to
`gui-overview.png`, `gui-mix.png` and `gui-fx.png` were removed in the same
change — a broken image is worse than a missing one, and three of them were
live for a moment.

**Still open, in value order:** `labs-index.png`, `gui-schemes.png`,
`verify-green.png`, `gui-fx.png`, `gui-mix.png`. The MIX and FX *sections* still
read fine without an image, so those two are the least urgent; the labs gallery
is the most valuable thing not yet shot.

**Two notes from the shots that came in**, for whoever takes the rest:
the four Tier-1 shots were captured at slightly different window widths, so MAIN
shows four columns in the dark shot and three in the light one. It still reads
as one instrument, but a fixed width would have made the pair exact. And none of
the four has the build stamp in frame — the footer sat below the capture. Both
are worth getting right on the remaining five.

## THE SHOT LIST (2026-08-26 — for the public README)

**Context: this README is about to be linked publicly**, so the set has a job
beyond documentation — a stranger scrolling for fifteen seconds should come away
knowing this is an instrument with a real idea in it. Every current image is
dated 2026-08-23 and predates the whole aesthetic pass, so **all of them are
retakes, not edits.**

**Capture once, at one size, in one session.** The plugin's own default is
**980 × 720** (`kGuiWidth`/`kGuiHeight`) — use it. The GUI is responsive, so a
differently-sized window reflows the columns and the set stops reading as one
instrument. Capture at 2× (Retina) — GitHub downscales cleanly and the thin ink
lines survive it.

**Hold a note while you shoot.** Every visualiser falls back to *"no engine —
open in the plugin"* or an empty well when nothing is sounding, and a set full
of empty wells makes a live instrument look like a mockup. Sustain a chord, or
run an arp, so the phase circle, spectrum, carpet and voice map are all alive.

**Leave the build hash visible** (bottom-right). It is what lets a caption name
a build, and it is the difference between a screenshot and a claim.

---

### Tier 1 — the README should not go public without these

| # | file | page & state | what it has to show |
|---|---|---|---|
| 1 | `gui-hero.png` | **MAIN**, **dark** chassis, note held | The first image anyone sees. The swarm's phase circle with a live R, the spectrum lit, the wordmark at full colour. Dark because the corner hues run at full emission there and the instrument reads as an instrument. **This one replaces `gui-overview.png` as the hero.** |
| 2 | `gui-morph.png` | **MORPH**, cursor parked **off-centre** | The thing nothing else on the market has. Four corner colours on the pad, and — critically — the ownership stripes visible down the panel beside it, so a viewer sees that *parameters belong to corners*. Centre the cursor and the story disappears (every corner weighs 0.25 and nothing reads). |
| 3 | `gui-osc.png` | **OSC**, note held | Density and depth: the swarm ring, the saw-shape viewer, the knob grid. This is the "there is a lot here" shot; it answers the reflex that a one-idea plugin is a toy. |
| 4 | `gui-light.png` | **the same page as #1**, **light** chassis | The pair is the argument. Two shots of one page in two chassis prove the design is a system rather than a colour scheme — and the light/cream ground is the more unusual look for a synth, which is exactly why it is worth showing. |

### Tier 2 — strongly wanted, in this order

| # | file | page & state | what it has to show |
|---|---|---|---|
| 5 | `labs-index.png` | `docs/design/index.html` | **The strongest single image for a technical audience, and the README currently has nothing like it.** Twenty-four browser labs in one gallery says the claims were prototyped before they were built — it is the visual form of "spec-in-code". |
| 6 | `gui-schemes.png` | one well, **two screen schemes**, same chassis | Side by side or as a two-panel crop (e.g. TUBE vs FROST). The only way to show chassis and screen are *independent* axes; a single-mode set cannot say it. |
| 7 | `gui-fx.png` | **FX**, a slot doing something audible | The rack and its slot types. Worth taking now knowing B50 will replace it — caption it as the current rack. |
| 8 | `gui-mix.png` | **MIX**, both oscillators on, meters moving | The two-oscillator story and the master stage. Meters at rest look broken; hold a note. |

### Tier 3 — optional, high value for the audience you are posting to

| # | file | what it has to show |
|---|---|---|
| 9 | `verify-green.png` | A terminal running `./verify fast` with every gate GREEN. Not pretty, and unusually persuasive to engineers: the README claims oracle discipline and this is the claim being true on screen. |
| 10 | `logo.gif` | 3–4 s of the wordmark warping, ideally with the morph pad being moved so the colour follows. The only *moving* thing in the set, and the one that demonstrates "the synth plays the logo" rather than asserting it. Keep it under ~2 MB; GitHub will autoplay it inline. |

### If you want one image for the LinkedIn post itself

Post **#1 (the dark hero)** or **#5 (the labs gallery)** — they argue different
things. The hero says *this is a finished-looking instrument*; the gallery says
*this was engineered*. For a professional network the gallery is the less
expected of the two, and the more defensible.

---

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

## Note on the current files

All five PNGs here are **GUI2 shots dated 2026-08-23** — they are the right
interface, taken before the aesthetic pass. What they predate: the knob geometry,
the light/dark chassis, the five screen schemes, the warping wordmark, the SET
page and the corner-ownership stripes. The top-level README says so under the
hero image rather than pretending otherwise. Replacing them is the shot list
above.
