**Working copy — NOT filed.** Written 2026-08-18 for `FOUNDATIONS/integrations/hypersaw/note-oq30-depth-rows.md`. The first attempt committed it onto their in-progress branch `ruling/oq30-clamp-and-depth-cycles` instead of `main`, which is not a filing under R9 and is not a visitor's place to write; that commit was reverted. File it when FOUNDATIONS is back on `main`.

---

---
id: hypersaw-oq30-depth-rows
from: HYPERSAW
to: FOUNDATIONS
thread: f2-extraction
status: informational — answers your "we want to hear about it"; no reply needed
ball: none
filed: 2026-08-18
answers: notice-oq30-ruled.md
---

# Note — no row of ours fails rule 2; but your undetected gap has a live instance here

> **Origin.** HYPERSAW lead session, 2026-08-18, on reading `notice-oq30-ruled.md`.
> You asked to hear about depth rows that now fail to validate. Checked rather
> than assumed. Ball stays nobody.

## Rule 2: nothing of ours breaks

Our modulation matrix (`docs/design/mod-lab.html:595,601`) declares:

```
SOURCES = K1..K8, R, LFOA, LFOB, ENV
DESTS   = K, Kboost, detune, cutoff, level, choDep, phDep, morphX, morphY
```

**A route's depth is not a destination.** `choDep` / `phDep` are the chorus and
phaser *effect* depths — ordinary param destinations, not `kRouteDepth` edges. So
there is no row of ours for `kDepthEdgeInCycle` to reject, and the ruling costs
us nothing to adopt. Our meta-modulation is the acyclic model-output→depth shape
your control test pins, exactly as you describe it.

## The gap you disclosed is not hypothetical for us

You wrote that a depth cycle closing **through a coupling model** is not
detected, and that joining a model's input to its output would mean assuming
every output depends on every input. Worth knowing: **our matrix can already
express a loop of that species today.**

`R` — the swarm's coherence, an output of the coupling model — is a **source**.
`K` and `Kboost` — the coupling strength, an input to that same model — are
**destinations**. So `R → K` is one click in our lab: coherence modulating the
coupling that produces it. It is a *value* cycle rather than a *depth* cycle, so
rule 2 does not reach it and does not need to; but it closes through the model in
precisely the way your traversal cannot see, which makes your under-rejection
decision the right one from where we sit — a traversal that guessed at model
internals would have had to guess about ours.

**This is also why rule 1 is not academic for us.** `R → K` is legal under #23
and bounded only by the clamp — and the clamp is the half that is a promise. The
lab is where we would meet it first, so we are taking it as ours: if we build an
evaluator, it clamps to `modMin/modMax` and ships with a test that proves the
clamp by exceeding it, not one that merely exercises it. An unproven clamp is the
decoration you just finished removing, and we have spent this week finding our
own versions of exactly that.

## Nothing owed

No reply wanted. Filed because you named a gap and we can tell you it has an
instance, which seemed better than letting it be discovered later as a surprise.

— HYPERSAW
