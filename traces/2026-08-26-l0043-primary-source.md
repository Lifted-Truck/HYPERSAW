# Trace — I filed a cross-repo answer to a document I had not read

**Trigger** human 2026-08-26: *"have you seen response-seam-round2.md?"*

**No.** I had read a scout subagent's report of it and quoted the scout's
quotes. On that basis I wrote and committed a cross-repo answer into
FOUNDATIONS' mailbox. The human's question was the check I had not run on
myself.

**What reading it actually changed.** Two things, neither available from the
summary — because a summary preserves conclusions and discards the checkable
details.

1. **I had asserted as fact what the scout flagged as unresolved.** My filing
   said the Aug-25 answer "was written **without sight of the first round**".
   The scout's own report listed exactly that under *"Ambiguities I will not
   resolve by inference"*. Worse, FOUNDATIONS had explicitly declined to infer
   it — *"we are not going to decide which of your two positions you hold by
   inferring it from a filename collision"* — so I asserted the very thing my
   correspondent had been careful not to. Now re-grounded in evidence they can
   check themselves: zero references to the Aug-11 round anywhere in the Aug-25
   document; it replaced that document *in place* rather than beside it; filed
   on the respond-by date.

2. **Their premise was wrong, and checkable.** They offered the supersession
   branch conditioned on *"(ADR-083 postdates the first answer, and B38 is
   new)"*. ADR-083 landed **2026-08-07** (`22d0b1d`) — four days *before* the
   Aug-11 answer, so that round was written with it in hand. B38 is genuinely
   new (**2026-08-24**, `2ef7282`) but bears on Q2, which they have already
   adopted, not on Q1 or Q3. So there is no new information on either contested
   point, by date — which strengthens the same conclusion I had reached by
   assertion.

**Amended and disclosed.** `bbf8d1a` in the FOUNDATIONS mailbox rewrites the
answer's basis and tells them the first pass was written from a summary — since
"a session answering a document it has not read" is the same species as the
failure their §2 diagnoses, and they should be able to weight our filings
accordingly. Substance is unchanged: **Aug-11 governs on Q1 and Q3.**

**Lesson L0043.** Delegated recon is legitimate for FINDING things and
illegitimate as the basis for an irreversible act. Delegate the search; read
the document before you answer it. The asymmetry matters: reading it cost one
command, and the act was outbound.

**Note the repetition.** This is the second instance in two days of the same
root failure — trusting a carried summary over the tree. The first was
reporting the signal-graph brief as unfiled for several turns when it had been
filed, answered and ratified since 2026-08-11. L0037 already covers
"frontmatter is a claim, the tree is the fact"; L0043 extends it to my own
intermediaries.

**Verify.** `./verify fast`: `mailbox_delivery` RED — the amended filing still
awaits the human's push (B52). All other gates GREEN.
