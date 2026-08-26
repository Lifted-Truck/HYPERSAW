# Trace — ADR-125: topology morphs by ARGMAX (human ruling, by ear)

**Trigger** human 2026-08-26, after listening to `fx-morph-law-lab.html`:
*"As much as part of me wants to go for the blend anyway, it does create an
untenable screechy feedback; maybe we could leave it as an option but default
to argmax. Argmax does in some ways better capture the spirit of the quantum
morph anyway."*

**What changed.** DECISIONS ADR-125; ROADMAP B50 updated (its stated
must-answer-first question is now answered); tests B50-1 (human ruling with a
falsifier) and B50-2 (implementation constraint); the lab now opens on ARGMAX
with BLEND still selectable. **No engine code** — see below for why that is
correct rather than incomplete.

**Why the second clause matters more than the first.** The screech is the
proximate reason, but *"argmax better captures the spirit of the quantum
morph"* is the durable one. ADR-104's premise is a morph that DRAWS a corner
rather than averaging corners; ADR-115 opened the field at corner A rather than
the centre for the same reason (*"the middle is the messiest place on the
grid"*). A blended topology is an averaged structure — the one thing this
design was built not to do. BLEND was the odd law out on this substrate all
along; the screech only made it audible.

**No new mechanism.** ARGMAX over topology means every route coefficient draws
one corner, which is precisely what ADR-124's `morphLead` map already does.
When routing joins the field, all route ids point at one lead and the existing
picker does the rest.

**Why no param was added.** `morphTopoLaw` (ARGMAX default, BLEND opt-in) is
the natural home for "leave it as an option" — but route coefficients are not
in `morphIds` today (`routing.processBlock` runs at `hypersaw_clap.cpp:3539`,
its coefficients are not morphable), so the param would change nothing. That is
the dead-control failure `gui_reach` exists to catch (L0023): fully
implemented, fully automatable, and absent from the product's behaviour. It
lands with the routing-morph work, in that change.

**Left open, deliberately.** The screech may be partly the *cycle delay* rather
than the blend. BLEND's midpoint is the only state that closes
`drive→comb→drive`, and the lab's loop carries one render quantum — 2.9 ms —
because Web Audio requires a delay in any cycle. A 2.9 ms loop is intrinsically
metallic. If that is the dominant cause, it is ear-gathered evidence for B50's
feedback fork (block-rate unacceptable for audio; per-sample required), which
would be a stronger basis than the argument alone. The A/B that settles it is
the same test with a one-sample loop delay. **ADR-125 does not depend on it** —
the ruling stands either way, and I have not assumed which lab revision the
human listened to.

**Verify.** `./verify fast` exit 0; `lab_load_check` GREEN. No engine code;
parity untouched by construction.
