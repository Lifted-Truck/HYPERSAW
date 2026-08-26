# Trace — the offset horizontal stripe on ghost knob cells

**Trigger** human 2026-08-25 (screenshot): *"The horizontal lines are offset
from the solid line."*

**Cause.** `.row.kcell.ghost` reset `box-shadow` but never `border-top`, and a
knob cell is normally **both** owned and ghost — owned by one corner, ghosted
because a *different* corner is armed. Measured on the real staging: **45 of 45
cells carried both classes.** So `.row.kcell.owned`'s `border-top:3px solid
var(--own)` survived under the ghost background. Worse, it survived *above* it:
`background-origin` is the padding box, which a 3px border pushes down, so the
dashes rendered 3px below the solid line. Two stacked stripes in two different
colours — the owner's and the armed corner's.

**Why the vertical rows never showed it.** `.row.owned` marks a plain row with
an *inset box-shadow*, which `.row.ghost`'s `box-shadow:none` already clears.
The two cases were never symmetrical, which is exactly what the previous
comment ("matching `.row.kcell.owned` one property across, exactly as the row
case does") assumed without checking. The asymmetry is the bug.

**Fix.** `border-top:0` on `.row.kcell.ghost`, so ghost replaces the owner
marking the way the vertical case already did.

**Verified.** Staged the real condition — every cell owned by some corner AND
ghosted by a different armed one, which is what `paintOwners` +
`paintCornerView` produce together. Shipped: border 0, stripe offset 0.
Must-fire control re-injecting the owner border: offset 3. Screenshot with
owned-only and ghost cells side by side shows one stripe per cell, all on the
same baseline.

**Verify.** `./verify fast` exit 0. CSS only; no DSP, parity untouched.
Rebuilt, installed to both formats, re-signed after copy, seals verified, AU
cache reset, marker confirmed present in both installed binaries with `grep -a`
(L0042).
