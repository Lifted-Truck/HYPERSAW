# 2026-07-19 — Rescue stranded sub-osc + add octave selector; roadmap mod matrix

**Rescue.** PR #38 (sub-osc) merged into its stacked base spectra-reach-xy,
not main — GitHub didn't retarget after #37 merged, so fa34eac stranded
(the stacked-PR variant of L0004). Cherry-picked fa34eac onto main clean.

**Octave selector.** subOct param (id 55, stepped -1/-2/-3, default -1);
sub freq = f0 * 2^subOct (pow(2,-1)=0.5 bit-exact = prior behavior).
Verified: -1→110 Hz, -2→55 Hz at A3. Parity 51/51, spectra rms 0,
state GREEN (sub off by default → inert; on → round-trips).

**Roadmap.** Mod matrix design session flagged (human has ideas), with
the Kuramoto-LFO concept recorded: a coupled phase population where every
routed parameter is a member — connected params sync/desync as a group
under a pull-K. Novel signature source vs N independent LFOs.

**MPE.** Three Sonnet research agents dispatched (advertisement path /
inbound-event+voice-matching / Live behavior); fix tabled per human.

Evidence: verify full green; pluginval 10 SUCCESS; installed.
