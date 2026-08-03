# TESTING.md — human test checklist (living document)

**Protocol (ratified 2026-07-31):** every PR that changes human-testable behavior
updates this file — items added under "Current", stale items pruned. The agent
refreshes it as part of the PR; the human checks items off in Live and reports
failures by item number. Ordered by importance; ~15 min total.

*Build under test: `main` @ post-#135 (run `./install check` to confirm currency).*

## Round-1 results (2026-07-31): 1 FAIL-partial (NOTE_END lag, see ROADMAP brief),
## 2 FAIL (amber hops — marker bug), 3 pass-with-note, 4 PASS, 5 FAIL (same as 1).
## Extras logged: hi-tame inaudible · double-click defaults wanted · retrigger
## gray-out wrong · SPECTRA voice map wanted.

## Current — untested by human

### 0a. ADR-075 oversample 2× (NEW)
- [ ] Toggle on at a bright patch: highs should open slightly (measured +2.4 dB at 15 kHz)
- [ ] Toggle OFF must sound *identical* to before this build (bit-exact by gate)
- [ ] Watch CPU in Live: expect roughly 2.5× the instrument's own load

### 0. ADR-074 super-width modes (NEW — fold just landed)
- [ ] Width 1.5, mode **wide (clean)**: as wide as the old sound, NO pulse artifact; scope stays cliff-free
- [ ] Mode **pulse**: the old ADR-025 sound, unchanged
- [ ] Mode **smear**: the allpass character from the lab's D
- [ ] Width ≤ 1: mode selector grays out; sound bit-identical to before
- [ ] Mono-fold check on wide mode: sum a track to mono — acceptable loss?

### 1. Stuck-note fix (#135) — THE experiment
- [ ] Poly mode, computer keyboard, type fast with overlapping/repeated keys for ~30 s
- [ ] **Expect:** note-monitor cells hollow out on every release; NO cell stays filled with keys up
- [ ] If any cell stays filled: screenshot the monitor + note the time — that falsifies the NOTE_END hypothesis

### 2. Voice map (#128/#131/#134)
- [ ] Default patch: map shows ±50¢ label, dots spread, amber ring at pan centre
- [ ] Root anchor → 1: amber ring sits ON the f0 line (pinned)
- [ ] Pivot → root, K → 0.8: amber dot holds still; teal dots migrate toward it
- [ ] Pan motion up: dots visibly wander L/R (mode: drift = individually, sweep = together)
- [ ] Law harmonic + reach 4 + spread up: zoom steps out to ±5/±10 oct, dots not pinned at frame edge

### 3. ADR-072 params, musical spot-check (#126/#127) — machine-verified, ears not yet
- [ ] Law → harmonic (series): coherent metallic spread; reach stretches the top
- [ ] Law → stretch (inharmonic): piano-like inharmonicity as stretch B rises
- [ ] Distribution → golden: even-but-inharmonic texture vs gaussian
- [ ] Keep phase ON: fast re-presses lose the restart click
- [ ] Freq glide ~20 ms + drift mode S&H + depth up: steps become swoops
- [ ] Tone tilt: + darkens, − thins (bipolar); Hi tame quiets upper voices
- [ ] Pan image legacy vs pitch fan; fan curve/invert reshape the image (n=7)

### 4. Visualizers restored (#133)
- [ ] Phase circle, carpet, spectrum, voice map, notes strip ALL animate

### 5. Regression spot-checks
- [ ] Mono: re-pressing a held key re-articulates (decision from #121)
- [ ] Save Live set with new params moved → reload → values persist
- [ ] FX slots Comp + Comb still behave; sub osc unchanged

## Known-good (human-verified earlier — retest only if suspicious)
Reverb lab chain (gain/meter/impulse/envelope) · ensemble character rewrite ·
lab rev-badges/fingerprints · mono note-hang under fast mono playing (#121).

## Deferred / blocked on other work
Even-fan image (n=2) — fold not yet built · GUI parse gate — awaiting ./verify ruling ·
morph corner colors — lab work.
