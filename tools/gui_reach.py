#!/usr/bin/env python3
"""gui_reach — every declared param must be REACHABLE in at least one shipped GUI.

L0023, third occurrence and the one that finally became a gate. The lesson says
a parameter range widened without its UI control ships a feature that is fully
implemented, fully tested, host-automatable, and absent from the product — and
that NO audio oracle can see it, because the audio path is correct. It has now
cost: FX types 0..5 shipping with dropdowns offering 0..3 for two weeks; a panic
button that existed everywhere except the interface that ships; and the discovery
(2026-08-12) that gui2 reaches 18 of 105 params while being someone's daily
driver.

Prose did not prevent any of those. This is the enforcement.

WHAT IS GATED: a param unreachable in EVERY GUI is a build failure. That is the
original bug and it is unambiguous.

WHAT IS REPORTED, NOT GATED: per-GUI coverage. gui2 is an in-progress renovation
and failing the build over its gaps would block all work — but the number is
printed on every run so the gap cannot be forgotten again, which is precisely how
it got this far. A number nobody sees is prose with extra steps.
"""
import re, sys, pathlib

root = pathlib.Path(__file__).resolve().parent.parent
decl = (root / "src/hypersaw_clap.cpp").read_text()
rows = re.findall(r'\{\s*(\d+),\s*"([A-Za-z0-9_]+)",\s*"([^"]*)"', decl)
params = {int(i): (k, n) for i, k, n in rows if int(i) < 1000}

# Deliberately not user-facing, each with its reason. L0036: a deliberate absence
# needs a test, and this list IS that test — adding to it is a visible decision,
# not a silent omission.
EXEMPT = {
    70: "inertiaCurve — dev-only, labelled (dev) in the param table",
}

guis = {}
for p in sorted((root / "src/gui").glob("*.html")):
    guis[p.name] = {int(x) for x in re.findall(r'data-p="(\d+)"', p.read_text())}

reachable_anywhere = set().union(*guis.values()) if guis else set()
orphans = sorted(set(params) - reachable_anywhere - set(EXEMPT))

for name, ids in sorted(guis.items()):
    hit = len(ids & set(params))
    print("  %-12s reaches %3d / %d params" % (name, hit, len(params)))
if EXEMPT:
    print("  exempt: %s" % ", ".join(EXEMPT.values()))

if orphans:
    print("gui_reach: RED — %d param(s) reachable in NO gui:" % len(orphans), file=sys.stderr)
    for i in orphans:
        print("    %4d %-16s %s" % (i, params[i][0], params[i][1]), file=sys.stderr)
    sys.exit(1)
print("gui_reach: GREEN (every declared param is reachable in some GUI)")
