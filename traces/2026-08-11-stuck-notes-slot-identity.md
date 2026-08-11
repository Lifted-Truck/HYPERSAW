# Stuck notes — oracle blindness, diagnosis, fix

**Trigger.** FOUNDATIONS brief `brief-stuck-notes-oracle-blindness.md` (2026-08-11), §1 verified
fact, §2 hypothesis.

**§1 confirmed.** `notefuzz_check` gates on rendered audio; the plugin constructor leaves oscillator
2 at vol 0; `vol` is per-oscillator (A12) so raising it needs id 1017, never sent. A hang in
oscillator 2 rendered nothing. The oracle could not fail on the class of hang that needs two
oscillators.

**§2 confirmed by measurement.** Six two-oscillator notefuzz modes added. With oscillator 2 audible
and its envelope diverged from oscillator 1's: permanent hang, peak 0.451876, 1498 ms tail,
deterministic at seed 1001, in `mono+2osc` and `mono+legato+2osc`. Control (`*-same`: oscillator 2
equally audible, envelope MATCHED) is clean at 41 ms — which is what isolates envelope divergence
rather than volume.

**Mechanism.** `alloc()` tiers 1-2 read `s.env`; the amp envelope is per-oscillator; divergent
envelopes fade tails on different schedules; the same note lands on different slots per core; the
fan-out helpers index every core by oscillator 0's slot and hit the wrong voice; the real voice is
orphaned gated under a key whose note-off has passed.

**Fix.** `slotOf[s][k]` recorded at note-on, used by `retargetAll` / `setNoteExprAll` /
`setNotePressureAll`. Identity-initialised so an unbound slot degrades to the previous behaviour.

**Oracle.** `./verify full` GREEN, fifteen gates. parity 147/147 worst 4.262e-09. notefuzz GREEN
across 21 modes. Calibration is inherent: RED before the fix, GREEN after, same binary and seeds.

**Not done.** Brief ask (c) — forensic ring buffer of the last N note events dumped on panic. Pays
regardless of who was right; queued.
