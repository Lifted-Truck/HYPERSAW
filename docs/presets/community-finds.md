# Community-found presets

Patches discovered in play worth keeping — candidates for the Phase 5 demo set
(ROADMAP). Load via the GUI: paste the JSON into the state box, click **PASTE
STATE**. Provenance-tracked; the JSON is the full state blob (SPEC §5.7 format).

---

## "Strings / Noise / Alien Metal" — found 2026-07-19 (human, in play)

**What it does (human's words):** with a very wet hall reverb and a lot of
playing around with the XY, it transforms between orchestral strings, dark
noise, and some sort of alien metal.

**Why it's interesting (engineering read):** SAW engine deep in splay
(K = −0.94) with q=3 Daido poles, full gravity (grav 1, basin 35¢) and a strong
Sakaguchi lag (α = −50°) over a Cauchy distribution (dist 3, Hz law) — so the
voices are simultaneously multiplying harmonically, clustering into triads, and
settling onto ratios, which is why small XY moves (detune × K) swing it across
those three timbres. R→tone at −1 and full phase + pan scatter (scatter 1,
panScatter 1) keep it wide and unstable; bass-mono (236 Hz) anchors the low end
under the external hall. The wet reverb is host-side (not in the patch).

```json
{"plugin":"HYPERSAW","schema":1,"params":{"n":32,"dist":3,"seed":0,"detune":0.15358361774744028,"law":1,"K":-0.94419319054526829,"onset":0,"dissolve":0.72443596007499,"driftDepth":3.7000000000000002,"driftRate":0.059999999999999998,"inertia":0,"rtone":-1,"normExp":0.68000000000000005,"width":0.96999999999999997,"mono":0,"digital":0,"vol":0.72999999999999998,"retrig":0,"attack":0.0030000000000000001,"decay":0.16,"sustain":1,"release":0.16,"beatMult":1,"topo":0,"reach":2,"mu":1,"alpha":-50,"poles":3,"grav":1,"basin":35,"absK":0,"voiceMono":0,"glide":0,"voiceLegato":1,"octave":-1,"semi":0,"fineCents":0,"pitchBend":0,"scatter":1,"bassMono":1,"bassMonoHz":236,"panScatter":1,"engine":0,"partials":32,"tilt":0.77000000000000002,"stretch":0,"cloud":3,"cwidth":0.73999999999999999,"wtilt":-0.40999999999999998,"wlaw":1,"cascade":0.78498293515358364,"subOn":1,"subVol":0.34000000000000002,"subWave":0.73999999999999999}}
```

*Note: `engine:0` = SAW active, so the SPECTRA-only fields (partials/cloud/sub*)
ride along in the state but don't sound until you switch to SPECTRA — worth a
try there too.*
