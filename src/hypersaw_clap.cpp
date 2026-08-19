/*
 * hypersaw_clap.cpp — HYPERSAW CLAP plugin impl (Phase 2: SwarmCore wired in).
 *
 * The DSP is src/swarm_core.h — the parity-proven SAW core (L0-1) — untouched
 * here; this file is the CLAP adapter: note/param events in, audio out, state
 * save/load. Parameter IDs are frozen once shipped (host automation lanes and
 * saved sessions reference them); append new params, never renumber. Ranges
 * mirror the prototype UI (swarmsaw.html) — notably dissolve is exposed in
 * SECONDS (the prototype knob is log10 s), driftDepth in cents.
 *
 * Real-time rules (charter): process() allocates nothing, no locks, no
 * wall-clock. setParam/rebuild are fixed-array math — safe on the audio
 * thread. params.flush is audio-thread while active per CLAP, main-thread
 * only when inactive, so touching the core there is race-free.
 */

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <atomic>
#include <string>
#include <filesystem>
#include <clap/clap.h>
#include <clapwrapper/vst3.h>

#include "swarm_core.h"
#include "gui/hypersaw_gui.h"
#include "spectra_core.h"
#include "glide_core.h"
#include "fx_rack.h"
#include "routing_core.h"
#include "hypersaw_clap_entry.h"
#include "build_stamp.h"   // generated every build (CMake target)

namespace
{

static const char *s_features[] = {CLAP_PLUGIN_FEATURE_INSTRUMENT, CLAP_PLUGIN_FEATURE_SYNTHESIZER,
                                   CLAP_PLUGIN_FEATURE_STEREO, nullptr};

static const clap_plugin_descriptor_t s_desc = {
    CLAP_VERSION_INIT,
    "com.lifted-truck.hypersaw",
    "HYPERSAW",
    "Lifted Truck",
    "https://github.com/Lifted-Truck/HYPERSAW",
    "",
    "",
    "0.1.0",
    "Coupled-oscillator swarm synthesizer (SAW core)",
    &s_features[0]};

/* ---- parameter table (IDs frozen; append-only) ---- */

struct ParamDef
{
  clap_id id;
  const char *coreKey;  // SwarmCore setParam key
  const char *name;
  double minV, maxV, defV;
  bool stepped;
  const char *const *labels;  // for enum-ish stepped params, else nullptr
};

static const char *const kDistLabels[] = {"even spread", "JP-8000 curve", "gaussian (seeded)",
                                          "cauchy (seeded)", "golden (irrational)"};
static const char *const kLawLabels[] = {"cents-constant", "Hz-constant", "ERB-flat",
                                         "tempo-grid", "harmonic (series)",
                                         "stretch (inharmonic)"};
static const char *const kDriftModeLabels[] = {"walk (1/f)", "sine (per-voice)",
                                               "sample & hold"};
static const char *const kPanModeLabels[] = {"drift (per-voice)", "sweep (whole image)"};
static const char *const kPivotLabels[] = {"mean field", "root (fundamental)"};
static const char *const kPanLayoutLabels[] = {"pitch fan", "legacy (x-position)"};
static const char *const kSuperModeLabels[] = {"wide (clean)", "pulse (M/S)", "smear (allpass)"};
static const char *const kGlideModeLabels[] = {"held note (legato)", "last note (memory)"};
static const char *const kOffOn[] = {"off", "on"};
static const char *const kNoteNames[] = {"C", "C#", "D", "D#", "E", "F",
                                        "F#", "G", "G#", "A", "A#", "B"};
static const char *const kBendLawLabels[] = {"off (instant)", "constant time", "constant rate",
                                            "lag (one-pole)", "mass-spring"};
static const char *const kBendQuantLabels[] = {"off", "chromatic", "scale"};
static const char *const kTopoLabels[] = {"mean-field", "ring", "two-cluster"};
static const char *const kPolesLabels[] = {"1 — classic", "2 — pair", "3 — triad", "4 — quad"};
static const char *const kFxTypeLabels[] = {"Off",  "Drive", "Filter", "Gain",
                                            "Comp", "Comb",  "Notch"};
// Display names for the gravity ratio readout (indices match core kRatios)
static const char *const kRatioNames[13] = {"1/1", "16/15", "9/8", "6/5", "5/4", "4/3", "7/5",
                                            "3/2", "8/5", "5/3", "16/9", "15/8", "2/1"};

static const char *const kEngineLabels[] = {"HYPERSAW", "SPECTRA"};   // engine renamed SAW -> HYPERSAW 2026-08-17 (ADR-091); value 0 and state key unchanged
static const char *const kWlawLabels[] = {"cents", "Hz"};
static const ParamDef kParams[] = {
    {1, "n", "Voices", 1, 32, 7, true, nullptr},
    {2, "dist", "Distribution", 0, 4, 1, true, kDistLabels},
    {3, "seed", "Seed", 0, 999999, 1234, true, nullptr},
    {4, "detune", "Detune", 0, 1, 0.28, false, nullptr},
    {5, "law", "Detune Law", 0, 5, 0, true, kLawLabels},
    {6, "K", "Pull K", -1, 1, 0, false, nullptr},
    {7, "onset", "Onset Lock", -1, 1, 0, false, nullptr},  // ADR-056: bipolar (<0 = splay onset)
    {8, "dissolve", "Dissolve (s)", 0.05, 7.94, 0.63, false, nullptr},
    {9, "driftDepth", "Drift Depth (c)", 0, 100, 0, false, nullptr},  // widened from the
    // prototype's 25c at human request (ADR-020); core takes any cents value
    {10, "driftRate", "Drift Rate", 0, 1, 0.4, false, nullptr},
    {11, "inertia", "Inertia", 0, 1, 0, false, nullptr},
    {12, "rtone", "R->Tone", -1, 1, 0, false, nullptr},
    {13, "normExp", "Density Comp", 0.5, 1, 0.75, false, nullptr},
    {14, "width", "Width", 0, 1.5, 0.8, false, nullptr},  // >1 = super-width (ADR-025)
    {15, "mono", "Mono Fold", 0, 1, 0, true, kOffOn},
    {16, "digital", "Digital", 0, 1, 1, false, nullptr},
    {17, "vol", "Volume", 0, 1, 0.4, false, nullptr},
    {18, "retrig", "Retrigger", 0, 1, 1, true, kOffOn},
    // ADR-021 envelope: defaults reproduce the reference AR bit-exactly
    {19, "attack", "Attack (s)", 0.001, 2.0, 0.003, false, nullptr},
    {20, "decay", "Decay (s)", 0.005, 4.0, 0.16, false, nullptr},
    {21, "sustain", "Sustain", 0, 1, 1.0, false, nullptr},
    {22, "release", "Release (s)", 0.005, 8.0, 0.16, false, nullptr},
    // Tempo-grid law (ADR-022): bpm is host-owned (transport), not a param
    {23, "beatMult", "Grid Cycles/Beat", 0.25, 8.0, 1.0, false, nullptr},
    // Dynamics surface (Phase 3 increment 2; engine per ADR-023)
    {24, "topo", "Topology", 0, 2, 0, true, kTopoLabels},
    {25, "reach", "Ring Reach", 1, 8, 5, true, nullptr},
    {26, "mu", "Cluster Link", 0, 1, 0.6, false, nullptr},
    {27, "alpha", "Phase Lag", -90, 90, 0, false, nullptr},
    {28, "poles", "Poles q", 1, 4, 1, true, kPolesLabels},
    {29, "grav", "Gravity", 0, 1, 0, false, nullptr},
    {30, "basin", "Basin (c)", 10, 50, 35, false, nullptr},
    {31, "absK", "Absolute K", 0, 1, 0, true, kOffOn},
    // Voice mode (ADR-026): mono/glide/legato are SHELL note-routing plus the
    // core's glide param; octave is a pure shell transpose.
    {32, "voiceMono", "Mono", 0, 1, 0, true, kOffOn},
    {33, "glide", "Glide (s)", 0, 2.0, 0, false, nullptr},
    {34, "voiceLegato", "Legato", 0, 1, 1, true, kOffOn},
    {35, "octave", "Octave", -2, 2, 0, true, nullptr},
    // Transposition suite (ADR-027): all four combine into ONE live core tune
    // factor — the pitch knob bends sounding notes, not just new ones.
    {36, "semi", "Semitones", -12, 12, 0, true, nullptr},
    {37, "fineCents", "Fine (c)", -100, 100, 0, false, nullptr},
    {38, "pitchBend", "Pitch", -12, 12, 0, false, nullptr},
    {39, "scatter", "Phase Scatter", 0, 1, 0, false, nullptr},  // ADR-033
    // Output stage + pan order (ADR-035). bassMono/bassMonoHz are SHELL
    // post-processing (side-channel high-pass, M/S); panScatter is core.
    {40, "bassMono", "Bass Mono", 0, 1, 0, true, kOffOn},
    {41, "bassMonoHz", "Bass XOver (Hz)", 60, 500, 120, false, nullptr},
    {42, "panScatter", "Pan Scatter", 0, 1, 0, false, nullptr},
    // Phase 4 (ADR-037): engine select + SPECTRA surface. Shared knobs
    // (K/onset/dissolve/seed/vol/retrig) are mirrored into both cores by
    // applyParam; ids 44-51 are SPECTRA-only.
    {43, "engine", "Engine", 0, 1, 0, true, kEngineLabels},
    {44, "partials", "Partials", 1, 32, 12, true, nullptr},
    {45, "tilt", "Amp Tilt", 0.5, 2, 1, false, nullptr},
    {46, "stretch", "Stretch", 0, 1, 0, false, nullptr},
    {47, "cloud", "Cloud Voices", 1, 7, 5, true, nullptr},
    {48, "cwidth", "Cloud Width", 0, 1, 0.25, false, nullptr},
    {49, "wtilt", "Width Tilt", -1, 1, 0, false, nullptr},
    {50, "wlaw", "Width Law", 0, 1, 0, true, kWlawLabels},
    {51, "cascade", "Cascade", 0, 1, 0, false, nullptr},
    // SPECTRA sub-oscillator (ADR-042; SPECTRA-only, ids route to spectra core)
    {52, "subOn", "Sub Osc", 0, 1, 0, true, kOffOn},
    {53, "subVol", "Sub Level", 0, 1, 0, false, nullptr},
    {54, "subWave", "Sub Wave", 0, 1, 0, false, nullptr},
    {55, "subOct", "Sub Octave", -3, -1, -1, true, nullptr},
    // Two-cluster A/B balance (ADR-051): sweeps cluster B from synced (0) to
    // splayed (1); default 0 is bit-inert. Two-cluster topology only.
    {56, "balance", "A/B Balance", 0, 1, 0, false, nullptr},
    // Internal FX rack (ADR-054, increment 1): 4 series slots, each a type +
    // amount, processed in slot order. Default type Off = bit-exact passthrough
    // (the parity gate). coreKeys are unique non-core strings — used only as
    // state-blob keys; apply/readParam intercept these ids and route to `rack`.
    {57, "fx1type", "FX1 Type", 0, 6, 0, true, kFxTypeLabels},
    {58, "fx1amt", "FX1 Amount", 0, 1, 0.5, false, nullptr},
    {59, "fx2type", "FX2 Type", 0, 6, 0, true, kFxTypeLabels},
    {60, "fx2amt", "FX2 Amount", 0, 1, 0.5, false, nullptr},
    {61, "fx3type", "FX3 Type", 0, 6, 0, true, kFxTypeLabels},
    {62, "fx3amt", "FX3 Amount", 0, 1, 0.5, false, nullptr},
    {63, "fx4type", "FX4 Type", 0, 6, 0, true, kFxTypeLabels},
    {64, "fx4amt", "FX4 Amount", 0, 1, 0.5, false, nullptr},
    // SPECTRA ADSR (ADR-055; SPECTRA-only, ids route to the spectra core).
    // SEPARATE from the SAW ADSR (ids 19-22): the two references have
    // different reference AR constants (SAW 3 ms/160 ms, SPECTRA 4 ms/180 ms),
    // so each engine carries its own envelope defaulting to ITS reference —
    // the plugin default must be reference-exact, not just the golden harness.
    // Renumbered 57-60 → 65-68 on the merge with the FX rack (ADR-054 owns 57-64).
    {65, "sAttack", "S.Attack (s)", 0.001, 2.0, 0.004, false, nullptr},
    {66, "sDecay", "S.Decay (s)", 0.005, 4.0, 0.18, false, nullptr},
    {67, "sSustain", "S.Sustain", 0, 1, 1.0, false, nullptr},
    {68, "sRelease", "S.Release (s)", 0.005, 8.0, 0.18, false, nullptr},
    // SAW waveshape morph (ADR-058): 0 = saw, 1 = square. SAW-core key, routed
    // by the applyParam fallback; default 0 is bit-inert (spectra no-ops "shape").
    {69, "shape", "Saw Shape", 0, 1, 0, false, nullptr},
    // ADR-072 batched param pass (task #18): the fold-campaign features.
    // Ids START AT 71: id 70 is a GHOST — the ADR-059 dev inertia-taper
    // exponent is intercepted by number in applyParam/readParam without a row
    // in this table, so "max id in the table + 1" is NOT the next free id.
    // (Found the hard way: toneTilt landed on 70 first and its writes were
    // silently swallowed by the taper hook — the functional smoke caught it.)
    // (ADR-060..070) made host-reachable. Ranges are the AUDITIONED lab ranges
    // (detune-lab sliders / fold ADRs), not invented. All defaults are the
    // core's bit-inert defaults, so an unautomated session sounds identical.
    // "toneTilt", not "tilt": id 45 already uses the key "tilt" for SPECTRA's
    // amp tilt, and applyParam mirrors unguarded ids into BOTH cores by key —
    // a new id named "tilt" would write both. The core carries the alias.
    {71, "toneTilt", "Tone Tilt", -1, 1, 0, false, nullptr},        // ADR-060
    {72, "hiTame", "Hi Tame", 0, 1, 0, false, nullptr},             // ADR-061
    {73, "driftMode", "Drift Mode", 0, 2, 0, true, kDriftModeLabels},  // ADR-062
    {74, "keepPhase", "Keep Phase", 0, 1, 0, true, kOffOn},         // ADR-062
    {75, "freqGlide", "Freq Glide (s)", 0, 0.1, 0, false, nullptr}, // ADR-063 seconds (ADR-009)
    {76, "panMotion", "Pan Motion", 0, 1, 0, false, nullptr},       // ADR-064
    {77, "panMode", "Pan Motion Mode", 0, 1, 0, true, kPanModeLabels},  // ADR-064
    {78, "motionCenter", "Centre Pin", 0, 1, 0, false, nullptr},    // ADR-064
    {79, "harmReach", "Harmonic Reach", 0.25, 4, 1, false, nullptr},  // ADR-065
    {80, "stretchB", "Stretch B", 0, 6, 0, false, nullptr},         // ADR-066
    {81, "spread", "Octave Spread", 1, 24, 1, false, nullptr},      // ADR-068
    {82, "anchor", "Root Anchor", 0, 1, 0, false, nullptr},         // ADR-068
    {83, "pivotMode", "Pivot", 0, 1, 0, true, kPivotLabels},        // ADR-069
    {84, "panLayout", "Pan Image", 0, 1, 0, true, kPanLayoutLabels},  // ADR-070
    {85, "panCurve", "Fan Curve", 0, 1, 0.5, false, nullptr},       // ADR-070
    {86, "panInvert", "Fan Invert", 0, 1, 0, true, kOffOn},         // ADR-070
    // ADR-074 super-width mode: active only at width > 1. Default 0 = mode F
    // (clean ITD+steepening) — a deliberate default-output change at width > 1
    // versus the old always-M/S behavior, per the human's ratified ship list.
    {87, "superMode", "Super-Width Mode", 0, 2, 0, true, kSuperModeLabels},
    // ADR-075: opt-in 2x oscillator oversampling. Default 0 keeps every
    // existing session and all 147 goldens bit-identical; on costs ~2.5x the
    // core's CPU (measured 2.5% -> 6.3% of one core at 8 notes x 16 voices).
    {88, "oversample", "Oversample 2x", 0, 1, 0, true, kOffOn},
    // ADR-076: poly glide reuses the existing Glide TIME knob (id 33), which
    // therefore stops being mono-only in the GUI gating.
    {89, "polyGlide", "Poly Glide", 0, 1, 0, true, kOffOn},
    {90, "glideMode", "Glide From", 0, 1, 0, true, kGlideModeLabels},
    // ADR-077 ensemble onset timing. onsetScatter is the master switch (0 = off
    // = bit-exact); alpha is the mutual-correction gain that carries the serial
    // structure listeners judge (LIBRARY L0019).
    {91, "onsetScatter", "Onset Scatter (ms)", 0, 80, 0, false, nullptr},
    {92, "onsetAlpha", "Timing Correction", 0, 1.5, 0.25, false, nullptr},
    {93, "attackScatter", "Attack Scatter", 0, 1, 0, false, nullptr},
    // ADR-078 per-voice envelopes. Off = one shared envelope (reference path).
    {94, "voiceEnv", "Per-Partial Env", 0, 1, 0, true, kOffOn},
    {95, "relScatter", "Release Scatter", 0, 1, 0, false, nullptr},
    // Per-slot SECOND axis for the FX rack (2026-08-03) — ADR-071 deferred the
    // comb's resonance "until the rack grows per-slot param pages"; this is it.
    // Deliberately ONE generic knob per slot rather than a comb-specific param,
    // so the next slot type that wants a second control costs no new ids.
    // Comb reads it as resonance (fb = 0.6 + 0.38*tone); 0.5 reproduces the
    // previously hardcoded 0.79 exactly, so every existing state loads unchanged.
    {96, "fx1tone", "FX1 Tone", 0, 1, 0.5, false, nullptr},
    {97, "fx2tone", "FX2 Tone", 0, 1, 0.5, false, nullptr},
    {98, "fx3tone", "FX3 Tone", 0, 1, 0.5, false, nullptr},
    {99, "fx4tone", "FX4 Tone", 0, 1, 0.5, false, nullptr},
    // ADR-059 DEV tune-then-lock: inertia knob taper exponent (0.5 == the sqrt
    // default). Shell-owned; re-derives inertia from the stored knob. Removed
    // once the human locks a value. coreKey is a non-core state key.
    {70, "inertiaCurve", "Inertia Curve (dev)", 0.3, 5, 0.5, false, nullptr},
    // MASTER VOLUME (B24 mixer, 2026-08-07) — the first id above 99, allocated
    // under Amendment 1's stride-1000 scheme. Needed because Amendment 1 made
    // `vol` (17) per-oscillator: after that there was NO patch-level fader at
    // all. Default 1.0 = unity, and the render skips the multiply at exactly
    // 1.0, so every existing patch is bit-identical. GLOBAL (in kGlobalIds).
    {100, "masterVol", "Master Volume", 0, 1.5, 1.0, false, nullptr},
    // GLOBAL PITCH (human, 2026-08-07): patch-level transpose summed with each
    // oscillator's own. UI range is the honest playing range (+/-12 st); the
    // MOD MATRIX is intended to drive pitch harder (to +/-48, clamped) when it
    // folds into the shell — recorded in ROADMAP so the widened drive does not
    // become an invisible feature (L0023).
    {101, "gSemi", "Pitch", -12, 12, 0, true, nullptr},
    {102, "gFine", "Fine", -100, 100, 0, false, nullptr},
    {103, "gOct", "Master Octave", -2, 2, 0, true, nullptr},
    // MUTE / SOLO (B24 mixer remainder, 2026-08-09) — PARAMS, not GUI state,
    // because the human asked for automation to reach them. Per-oscillator, so
    // oscillator 2 is 1104/1105. Shell-owned: they gate the mix stage and never
    // enter SwarmCore, so the parity goldens cannot see them.
    // Defaults 0/0 mean every gain is exactly 1.0 and the render skips the
    // multiply, so an untouched patch stays bit-identical.
    {104, "oscMute", "Mute", 0, 1, 0, true, kOffOn},
    {105, "oscSolo", "Solo", 0, 1, 0, true, kOffOn},
    /* BEND TRAVEL LAW (id 106+, ADR pending; folded 2026-08-19). GLOBAL — the
       wheel bends the patch, so these are not per-oscillator. Ranges and defaults
       are the REFERENCE's, read from docs/design/bend-lab.html's own controls, so
       a value set here means what it meant on the bench glide_check's goldens were
       sliced from.
       `bendLaw` ships OFF: the core calls kConstRate its "ratified default", but
       that is the bench's default for AUDITIONING, and shipping it would change how
       every existing patch bends (human ruling 2026-08-19). */
    {106, "bendLaw", "Bend Law", 0, 4, 0, true, kBendLawLabels},
    {107, "bendTime", "Bend Time (ms)", 5, 1500, 120, false, nullptr},
    {108, "bendRate", "Bend Rate (st/s)", 0.5, 200, 24, false, nullptr},
    {109, "bendTau", "Bend Lag (ms)", 1, 400, 60, false, nullptr},
    {110, "bendSpringF", "Spring (Hz)", 0.5, 20, 4, false, nullptr},
    {111, "bendDamp", "Damping", 0, 1, 0.6, false, nullptr},
    {112, "bendDistOver", "Distance Curve", 0, 2, 1, false, nullptr},
    // BEND LANE ONLY, and the core enforces it: a note has no home pitch to
    // spring back to, so retMul is meaningless on the note-pitch lane.
    {113, "bendReturn", "Return x", 0.2, 3, 1, false, nullptr},
    {114, "bendQuant", "Bend Quantise", 0, 2, 0, true, kBendQuantLabels},
    {115, "bendHyst", "Quantise Hyst (c)", 0, 50, 8, false, nullptr},
    /* GLOBAL SCALE (ids 116-128). THE MASK IS THE TRUTH, THE NAME IS UI — the
       standing ruling. Consumers store and transmit `{root, mask}` only, never a
       scale ID, which is what keeps `glide_core.h` free of a scale table: adding
       a named scale becomes a UI-table edit with no core change and no parity
       surface, and a hand-drawn set is first-class rather than a degraded mode.
       Hence twelve honest booleans instead of one packed 0..4095 integer, which
       no host could automate meaningfully and no user could read. The named-scale
       dropdown lives in the GUI and WRITES these thirteen; it is not a parameter.
       Global because four consumers are already visible — the bend quantiser, the
       note-pitch lane, the chord layer, any arp — and two modules disagreeing
       about the scale produce notes in neither key. */
    {116, "scaleRoot", "Scale Root", 0, 11, 0, true, kNoteNames},
    {117, "scaleDeg0", "Degree 1 (root)", 0, 1, 1, true, kOffOn},
    {118, "scaleDeg1", "Degree b2", 0, 1, 0, true, kOffOn},
    {119, "scaleDeg2", "Degree 2", 0, 1, 1, true, kOffOn},
    {120, "scaleDeg3", "Degree b3", 0, 1, 0, true, kOffOn},
    {121, "scaleDeg4", "Degree 3", 0, 1, 1, true, kOffOn},
    {122, "scaleDeg5", "Degree 4", 0, 1, 1, true, kOffOn},
    {123, "scaleDeg6", "Degree b5", 0, 1, 0, true, kOffOn},
    {124, "scaleDeg7", "Degree 5", 0, 1, 1, true, kOffOn},
    {125, "scaleDeg8", "Degree b6", 0, 1, 0, true, kOffOn},
    {126, "scaleDeg9", "Degree 6", 0, 1, 1, true, kOffOn},
    {127, "scaleDeg10", "Degree b7", 0, 1, 0, true, kOffOn},
    {128, "scaleDeg11", "Degree 7", 0, 1, 1, true, kOffOn},
};

// THE DEFAULT OF A PARAMETER, DEFINED ONCE. Both CLAP (`clap_param_info.
// default_value`) and the GUI bridge ask here, so a host's "reset to default"
// and the GUI's double-click cannot disagree. They already could: oscillators
// above the first default to SILENT, and a GUI reading the default out of its
// own markup restored 0.4 to a parameter CLAP reports as 0.0. File scope
// deliberately — it depends on nothing but the row and the oscillator index.
static double defaultFor(const ParamDef &d, uint32_t osc)
{
  return (osc > 0 && d.id == 17) ? 0.0 : d.defV;
}
constexpr uint32_t kNumParams = sizeof(kParams) / sizeof(kParams[0]);

/* ---- ADR-082 multi-oscillator namespace (increment 1: mechanism only) -----
   id(P, osc k) = id(P, osc 0) + 100k.  Oscillator 0 keeps every id it has, so
   every existing session, automation lane and patch survives untouched. CLAP
   ids are APPEND-ONLY: this mapping is designed once or lived with forever.

   kNumOsc is 1 here ON PURPOSE. Increment 1 lands the id/state mechanism with
   the oscillator count unchanged, so params_count(), the id list and the saved
   state bytes are all bit-identical to before — which is exactly what makes
   the parity/state oracles a proof that the refactor is inert. Increment 2
   raises it to 2 (the ratified slot count) and adds the second core. */
// STRIDE 1000, NOT 100 (amendment, 2026-08-06 — see ADR-082 Amendment 1).
// The stride is also the CAPACITY of oscillator 0's block, and at stride 100
// that block was ids 1..99 with ZERO free slots: the instrument already had 99
// params, so it could never gain another one. A new param at id 100 is not
// merely cramped, it is UNREACHABLE — findParam computes osc = id/kOscStride,
// so 100 resolves to oscillator 1 / base 0 and is never found. 1000 leaves 900
// free slots and costs nothing to adopt today, because increment 1 shipped at
// kNumOsc == 1 and no id >= 100 has ever been exposed to a host.
// Chunk the extra oscillators' render through a fixed stack buffer. Small
// enough to be free on the stack, large enough that the loop overhead is
// irrelevant against a render of the same length.
// ---- BEND TRAVEL GRID (ADR-086 Amendment 1's construction, reused) ----------
// The bend glide advances on a fixed TIME grid, not a fixed sample count. The
// amendment exists because the first version of that idea (kGravGrid = 256
// SAMPLES) was a duration that shrank as the sample rate rose, so the trajectory
// tracked the rate; expressed in seconds it obeys ADR-009 like every other time
// constant. The value is exactly 16/44100 so the grid is EXACTLY 16 samples at
// 44.1 kHz — which is the rate `bend-lab.html` was benched at and therefore the
// rate glide_check's goldens encode. Any other value silently invalidates them.
constexpr double kBendGridSeconds = 16.0 / 44100.0;   // 0.363 ms

constexpr int kMixChunk = 256;
constexpr uint32_t kOscStride = 1000;
constexpr uint32_t kMaxOsc = 2;   // ratified 2026-08-06; 2000-2999 stays free for a third
constexpr uint32_t kNumOsc = 2;   // ADR-082 increment 2: the ratified slot count
static_assert(kNumOsc >= 1 && kNumOsc <= kMaxOsc, "kNumOsc outside the ratified range");

/* GLOBAL params — one instance no matter how many oscillators exist. Everything
   NOT listed is per-oscillator. Itemised in ADR-082; the three judgement calls
   (amp env global; transpose per-osc; retrig/keepPhase per-osc) are recorded
   there rather than buried here, because a param in the wrong class is wrong
   permanently. */
constexpr clap_id kGlobalIds[] = {
    // A12 (human-ratified 2026-08-11): the amp envelope (19-22) and beatMult
    // (23) LEFT this list. Envelope, because a fast-attack oscillator layered
    // against a slow swell is a basic two-oscillator move and one shared
    // envelope makes the second oscillator a timbre-only layer. beatMult,
    // because it is a parameter OF the tempo-grid detune law and `detune`/`law`
    // are already per-oscillator — so an oscillator could pick the law but not
    // its own grid. bpm stays host-owned and global; beatMult is the per-source
    // ratio to it.
    15, 40, 41,                                  // output & image
    // NB: 14 "width" left this list 2026-08-07 (A12, human-ruled: "oscillators
    // will independently need their own width controls"). It is a SwarmCore
    // param, so each oscillator always had its own copy — global classification
    // just made oscillator 2's unreachable. mono (15) stays global pending the
    // rest of the A12 ruling.
    // NB: 17 "vol" is NOT here. It is the swarm's own output gain, computed
    // inside SwarmCore::render — so it is PER-OSCILLATOR, and it is what lets
    // two oscillators be balanced against each other. A patch-level master
    // volume, if wanted, is a separate new param (the stride-1000 amendment
    // leaves room for one).
                                 // amp envelope (voice-level, not per-osc)
    32, 33, 34, 38, 75, 89, 90, 11, 70,          // voice & glide behaviour
    57, 58, 59, 60, 61, 62, 63, 64, 96, 97, 98, 99,  // FX rack
    88,                                      // tempo grid, oversampling
    100, 101, 102, 103,                          // masterVol + global pitch
    106, 107, 108, 109, 110, 111, 112, 113, 114, 115,  // bend travel law (global: the wheel bends the patch)
    116, 117, 118, 119, 120, 121, 122, 123, 124,     // global scale: root + twelve degrees
    125, 126, 127, 128,                          // (the mask is the truth; the name is UI)
};
constexpr bool isGlobalId(clap_id id)
{
  for (clap_id g : kGlobalIds)
    if (g == id) return true;
  return false;
}
// How many of the 99 are per-oscillator — the size of each additional block.
inline uint32_t perOscParamCount()
{
  uint32_t n = 0;
  for (const auto &d : kParams)
    if (!isGlobalId(d.id)) n++;
  return n;
}

// Which oscillator an id addresses, and the osc-0 id it mirrors. Global ids
// always resolve to oscillator 0 — they have no counterpart in the higher
// blocks, which is why those slots are never allocated.
inline uint32_t oscOfId(clap_id id) { return (uint32_t)id / kOscStride; }
inline clap_id baseIdOf(clap_id id) { return (clap_id)((uint32_t)id % kOscStride); }

const ParamDef *findParam(clap_id id)
{
  const uint32_t osc = oscOfId(id);
  if (osc == 0)
  {
    for (const auto &d : kParams)
      if (d.id == id) return &d;
    return nullptr;
  }
  if (osc >= kNumOsc) return nullptr;          // block exists only up to kNumOsc
  const clap_id base = baseIdOf(id);
  if (isGlobalId(base)) return nullptr;        // globals have no per-osc mirror
  for (const auto &d : kParams)
    if (d.id == base) return &d;
  return nullptr;
}

// Grid cycles/beat quantizes to musical (rational) divisions — the param
// stores the actual cycles-per-beat value (state stays forward-compatible),
// but applyParam snaps and value_to_text names the fraction.
static const double kGridSteps[] = {0.25, 1.0 / 3, 0.5, 2.0 / 3, 0.75, 1, 1.5, 2, 3, 4, 6, 8};
static const char *const kGridStepNames[] = {"1/4", "1/3", "1/2", "2/3", "3/4", "1",
                                             "3/2", "2",   "3",   "4",   "6",   "8"};
constexpr int kNumGridSteps = 12;

double snapGridStep(double v)
{
  double best = kGridSteps[0], bd = 1e9;
  for (double s : kGridSteps)
    if (std::fabs(v - s) < bd)
    {
      bd = std::fabs(v - s);
      best = s;
    }
  return best;
}

const char *gridStepName(double v)
{
  for (int i = 0; i < kNumGridSteps; i++)
    if (std::fabs(v - kGridSteps[i]) < 1e-6) return kGridStepNames[i];
  return nullptr;
}

struct Plugin
{
  clap_plugin_t plugin{};
  const clap_host_t *host = nullptr;
  const clap_host_params_t *hostParams = nullptr;
  // ADR-082 increment 2: N SAW cores. `core` stays a reference to oscillator 0
  // so the 52 existing call sites keep meaning exactly what they meant — this
  // change adds an oscillator, it does not rewrite the first one.
  hypersaw::SwarmCore cores[kMaxOsc] = {hypersaw::SwarmCore{44100.0},
                                        hypersaw::SwarmCore{44100.0}};
  hypersaw::SwarmCore &core = cores[0];

  /* FAN-OUT SEAM (2026-08-09). Every per-voice and lifecycle operation means
     "all oscillators", never "oscillator 0" — route them through these and
     never through `core`.

     The `core` alias exists so the multi-oscillator port (ADR-082) did not have
     to touch every legacy call site. That convenience is exactly what hid this:
     eight sites read as correct C++ and were correct with one oscillator, and
     with two they addressed half the instrument. PRESSURE fanned out while
     TUNING did not, so a bend split the pair mid-gesture; every allOff() —
     mono/poly toggle, engine switch, MIDI all-notes-off, reset, GUI panic —
     silenced oscillator 0 and left the rest ringing, which is a stuck note.

     This is L0028's shape: an operation whose intent is a ROLE ("every
     oscillator") written against an INSTANCE. Covered by tools/mpe_check.cpp;
     the alias itself is the root cause and its removal is queued behind a
     human gate, since `core` still has legitimately-oscillator-0 readers. */
  void allOffAll()
  {
    for (uint32_t k = 0; k < kNumOsc; k++) cores[k].allOff();
  }
  void noteOffAll(int key)
  {
    for (uint32_t k = 0; k < kNumOsc; k++) cores[k].noteOff(key);
  }
  /* ONE LOGICAL NOTE, N PHYSICAL VOICES — and the mapping is now CONSTRUCTED,
     not assumed. Every helper below used to apply oscillator 0's slot index to
     every core, on the strength of a comment ("note fan-out keeps slot indices
     aligned"). Nothing enforced it, and it is false: `alloc()`'s tiers 1 and 2
     read `s.env`, and the amp envelope is PER-OSCILLATOR (A12), so the moment
     two cores' envelopes differ their tails fade on different schedules, the
     same note lands on different slots, and a retarget gates the WRONG voice in
     core k while the real one is orphaned — gated, under a key whose note-off
     has already been and gone. It never releases. That is the human's
     intermittent stuck-note report, and FOUNDATIONS' 2026-08-11 brief §2 called
     the mechanism before it was measured.

     `slotOf[s][k]` is core k's slot for the logical voice that oscillator 0
     holds at slot s; `slotOf[s][0] == s` by definition. Recorded at note-on,
     which is the only place a core allocates. */
  int slotOf[hypersaw::kPoly][kNumOsc];
  void bindSlots(int slot0, uint32_t k, int slotK)
  {
    if (slot0 >= 0 && slot0 < (int)hypersaw::kPoly && k < kNumOsc) slotOf[slot0][k] = slotK;
  }
  void retargetAll(int slot, int key, double freq, bool keepPhase)
  {
    if (slot < 0) return;
    for (uint32_t k = 0; k < kNumOsc; k++)
      cores[k].retargetNote(slotOf[slot][k], key, freq, keepPhase);
  }
  void setNoteExprAll(int slot, double v)
  {
    if (slot < 0) return;
    for (uint32_t k = 0; k < kNumOsc; k++) cores[k].setNoteExpr(slotOf[slot][k], v);
  }
  void setNotePressureAll(int slot, double v)
  {
    if (slot < 0) return;
    for (uint32_t k = 0; k < kNumOsc; k++) cores[k].setNotePressure(slotOf[slot][k], v);
  }
  // constructed state matches the reported default above
  struct SilenceHigherOscillators
  {
    explicit SilenceHigherOscillators(hypersaw::SwarmCore *c)
    {
      for (uint32_t k = 1; k < kNumOsc; k++) c[k].setParam("vol", 0.0);
    }
  } silenceHigher{cores};
  hypersaw::SpectraCore spectra{44100.0};
  /* FORENSIC NOTE TRACE (FOUNDATIONS brief ask (c), 2026-08-11).
     The stuck-note bug took weeks because it could not be REPRODUCED, and no
     generator was ever going to reproduce it: a fuzzer emits the event stream
     it imagines, and ours deliberately excludes shapes no host can produce
     (notefuzz_check.cpp:14-17). So it can never model a stream the host
     actually delivered. Capture instead of simulate — then a field report
     becomes a replayable regression case instead of an anecdote.

     Written from the AUDIO THREAD: plain stores into a fixed array plus one
     release store. No allocation, no lock, no wall-clock — the charter and
     rtsafety_probe both forbid all three. Read from the GUI thread on panic;
     a torn read of a single record is acceptable here, because this is a
     diagnostic and making it exact would cost the audio thread something real.
     kTraceLen is a power of two so the index is a mask, not a modulo. */
  struct NoteTrace
  {
    uint64_t pos;      // absolute sample position: block steady time + offset
    uint16_t type;     // CLAP event type
    int16_t key;
    int32_t noteId;
    int16_t channel, port;
    float velocity;
  };
  static constexpr uint32_t kTraceLen = 512;   // a few seconds of dense play
  static_assert((kTraceLen & (kTraceLen - 1)) == 0, "kTraceLen must be a power of two");
  NoteTrace trace[kTraceLen] = {};
  std::atomic<uint64_t> traceWrite{0};         // total ever written; & (len-1) indexes
  uint64_t blockPos = 0;                       // steady time of the block in flight
  uint64_t tracePos = 0;                       // local monotonic sample count, never host-supplied

  /* HOST-MPE DETECTION. Live gates MPE behind a PER-DEVICE toggle the plugin
     cannot set and cannot read. With it off, an expressive device's stream
     arrives FLATTENED — every note on channel 0, no note expressions — and the
     result is retriggered blips where the player expects sustain. That cost two
     multi-round investigations here (2026-07-19 bend, 2026-08-12 Expressive
     Chords), and the human found it both times, not the oracle.

     We cannot turn the toggle on. We CAN notice its absence: notes arriving with
     zero note expressions AND never leaving channel 0 is the signature. Plain
     single-channel MIDI looks identical, which is why the hint is phrased as a
     possibility and never as an error — a diagnosis the user can dismiss beats a
     defect they cannot find. Relaxed stores; these are counters, not state. */
  std::atomic<uint32_t> sawNotes{0}, sawExprs{0}, sawNonZeroChan{0};
  std::string lastDumpPath;

  void recordNote(const clap_event_header_t *ev, const clap_event_note_t *n)
  {
    const uint64_t w = traceWrite.load(std::memory_order_relaxed);
    NoteTrace &r = trace[w & (kTraceLen - 1)];
    r.pos = blockPos + ev->time;
    r.type = (uint16_t)ev->type;
    r.key = (int16_t)n->key;
    r.noteId = n->note_id;
    r.channel = (int16_t)n->channel;
    r.port = (int16_t)n->port_index;
    r.velocity = (float)n->velocity;
    traceWrite.store(w + 1, std::memory_order_release);
  }

  /* Write the trace and the live voice tables to a file, and return its path.
     MAIN/GUI THREAD ONLY — this opens a file, which the audio thread may never
     do. It reads state the audio thread is concurrently writing and does not
     lock: a diagnostic that stalls the audio thread to describe it is worse
     than a diagnostic with one torn row.

     The path is derived at RUNTIME, never baked in — a machine-absolute path in
     a tracked file is both an identity leak and wrong on any other machine. */
/* Empty string = nothing to say. Deliberately silent until enough notes have
     arrived to be sure: a hint that fires on the first note would fire on every
     load, and a warning that is usually wrong gets ignored when it is right. */
  std::string hostHint() const
  {
    const uint32_t n = sawNotes.load(std::memory_order_relaxed);
    if (n < 24) return {};
    if (sawExprs.load(std::memory_order_relaxed) > 0) return {};
    if (sawNonZeroChan.load(std::memory_order_relaxed) > 0) return {};
    return "No note expressions received on any channel. If you are playing an "
           "MPE controller or an expressive device, MPE is probably OFF for this "
           "plugin in your host - per-note pitch and pressure will be flattened.";
  }

    std::string dumpForensics(const char *why)
  {
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path dir;
#ifdef __APPLE__
    if (const char *home = std::getenv("HOME")) dir = fs::path(home) / "Library" / "Logs" / "HYPERSAW";
#endif
    if (dir.empty()) dir = fs::temp_directory_path(ec) / "HYPERSAW";
    fs::create_directories(dir, ec);
    // Named by the trace counter, not by a clock: the charter bans wall-clock
    // reads in the core, and a monotonic counter also sorts correctly.
    const uint64_t w = traceWrite.load(std::memory_order_acquire);
    const fs::path out = dir / ("panic-" + std::to_string(w) + ".txt");
    std::FILE *f = std::fopen(out.string().c_str(), "w");
    if (!f) return {};

    std::fprintf(f, "HYPERSAW forensic dump\nreason: %s\nbuild: %s\nsample rate: %.1f\n",
                 why, HYPERSAW_BUILD_STAMP, sampleRate);
    std::fprintf(f, "engine: %s  mono: %d  legato: %d  monoSlot: %d  heldCount: %d\n",
                 spectraMode() ? "SPECTRA" : "SAW", (int)voiceMono, (int)voiceLegato,
                 monoSlot, heldCount);

    /* THE PATCH, because "the envelope sounds wrong" is unanswerable without it.
       The 2026-08-12 Expressive Chords report needed attack/decay/sustain/release
       to separate "the host sent short notes" from "our envelope mis-renders long
       ones", and the dump did not carry them — so the capture settled the note
       STREAM and left the sound unexplained. A forensic dump that records the
       input but not the configuration answers only half of any question. */
    std::fprintf(f, "\n-- patch (the params that shape what you hear) --\n");
    {
      static const int kWanted[] = {1, 4, 6, 8, 14, 17, 19, 20, 21, 22, 32, 34, 42, 94};
      for (int id : kWanted)
      {
        const ParamDef *d0 = findParam((clap_id)id);
        if (!d0) continue;
        std::fprintf(f, "  %-14s", d0->coreKey);
        for (uint32_t k = 0; k < kNumOsc; k++)
          std::fprintf(f, "  osc%u %-9.4f", k, cores[k].getParam(d0->coreKey));
        std::fprintf(f, "\n");
      }
    }

    std::fprintf(f, "\n-- held stack --\n");
    for (int i = 0; i < heldCount; i++) std::fprintf(f, "  [%d] key %d\n", i, heldStack[i].key);

    /* The voice tables per core, side by side with slotOf. This is the exact
       view that would have shown the stuck-note orphan at a glance: a gated
       voice in core 1 whose key appears in no held stack, at a slot the shell
       is not addressing. */
    std::fprintf(f, "\n-- voices (shell slot -> each core's own slot) --\n");
    for (int i = 0; i < hypersaw::kPoly; i++)
    {
      bool any = false;
      for (uint32_t k = 0; k < kNumOsc; k++)
        if (cores[k].voiceAt(i).gate || cores[k].voiceAt(i).env > 1e-4) any = true;
      if (!any && !tags[i].active) continue;
      std::fprintf(f, "  %2d:", i);
      for (uint32_t k = 0; k < kNumOsc; k++)
      {
        const auto &v = cores[k].voiceAt(slotOf[i][k]);
        std::fprintf(f, "  core%u[slot %d] midi %3d %s env %.4f |", k, slotOf[i][k],
                     v.midi, v.gate ? "GATED" : "  off", v.env);
      }
      std::fprintf(f, "  tag %s key %d note_id %d\n", tags[i].active ? "active" : "  --",
                   tags[i].key, tags[i].noteId);
    }

    std::fprintf(f, "\n-- last %u note events, oldest first (pos = absolute sample) --\n",
                 (unsigned)(w < kTraceLen ? w : kTraceLen));
    const uint64_t first = w > kTraceLen ? w - kTraceLen : 0;
    for (uint64_t n = first; n < w; n++)
    {
      const NoteTrace &r = trace[n & (kTraceLen - 1)];
      const char *t = r.type == CLAP_EVENT_NOTE_ON ? "ON   "
                    : r.type == CLAP_EVENT_NOTE_OFF ? "OFF  "
                    : r.type == CLAP_EVENT_NOTE_CHOKE ? "CHOKE" : "?????";
      std::fprintf(f, "  pos %10llu  %s key %3d  note_id %5d  ch %d  port %d  vel %.3f\n",
                   (unsigned long long)r.pos, t, r.key, r.noteId, r.channel, r.port, r.velocity);
    }
    std::fclose(f);
    return out.string();
  }

  /* Panic: capture, THEN clear. The ordering is the whole feature — a dump
     taken after the clear faithfully records a synth in perfect health and
     proves nothing, and panic is precisely the human's tell that the bug just
     happened. Extracted from the GUI lambda so the ordering is reachable from a
     headless oracle; when it lived inline it was guarded only by a comment,
     which trace_check recorded as a known coverage boundary rather than
     pretending to cover. */
  void panicWithDump()
  {
    lastDumpPath = dumpForensics("panic");
    /* RETIRE the outstanding notes; do not DISCARD them. This used to do
       `pendingEndCount = 0` and clear every tag directly, which destroyed every
       NOTE_END the host was owed — a host tracking `note_id`s was left holding
       identities that never end, and nothing downstream could recover them
       because the tag carrying the identity was already gone.

       Same class as L0022 (an END obligation destroyed rather than delivered),
       reached through a different door: there the host REFUSED the push and the
       tag was retired anyway; here the tag was dropped before a push was ever
       attempted. Found 2026-08-11 while answering FOUNDATIONS' question about
       which END cases their seam had not modeled — the question forced a read
       of this function and the defect was sitting in it.

       retireTag() moves each active tag into pendingEnds (respecting its cap)
       and clears `active`, so the blanket clear this replaced is redundant as
       well as wrong. emitNoteEnds then delivers them on following blocks, with
       the try_push retry L0022 installed. */
    for (int i = 0; i < hypersaw::kPoly; i++) retireTag(i);
    allOffAll();
    spectra.allOff();
    rack.reset();
    heldCount = 0;
    monoSlot = -1;
  }

  hypersaw::FxRack rack;  // ADR-054 internal FX rack (post-oscillator)
  // B23 crosspoint topology over those slots (ADR-088). One source for now —
  // the summed, post-bass-mono bus — so this increment is purely "the matrix is
  // in the audio path and inert". Per-oscillator sources are a later increment
  // and carry their own decision, because sources upstream of bass-mono is
  // exactly the ordering question this increment declined to force.
  hypersaw::RoutingMatrix<1, hypersaw::kRackSlots> routing;
  double engineSel = 0;  // 0 SAW, 1 SPECTRA (ADR-037; shell dispatch)
  bool spectraMode() const { return engineSel != 0; }
  double sampleRate = 44100.0;

  // GUI -> audio param queue (producer: GUI main thread; consumer: process on
  // the audio thread, or flush on main when inactive — never concurrent per
  // the CLAP threading contract).
  struct ParamMsg
  {
    uint32_t id;
    double value;
    uint8_t kind;  // 0=value, 1=gesture begin, 2=gesture end
  };
  static constexpr uint32_t kQCap = 256;
  ParamMsg queue[kQCap];
  std::atomic<uint32_t> qHead{0}, qTail{0};

  // Engine -> GUI viz feed: classic double buffer; writer alternates, reader
  // only ever copies the published side.
  hypersaw::VizSnapshot vizBuf[2];
  std::atomic<int> vizPublished{0};

  hypersaw::HypersawGui *gui = nullptr;
  // Spectrum feed: mono ring written on the audio thread (write-only, cheap);
  // the FFT runs on the GUI thread on demand — zero audio-thread analysis
  // cost, torn reads are cosmetic-only (visualizer).
  float specRing[4096] = {0};
  std::atomic<uint32_t> specPos{0};
  // Scope feed (2026-08-03): STEREO, unlike specRing's mono sum — the whole
  // point of a scope here is watching L against R (super-width's polarity
  // modes are invisible in a sum). Write-only on the audio thread.
  double outPeakViz = 0;   // peak since the last viz publish (see publishViz)
  float scopeL[2048] = {0}, scopeR[2048] = {0};
  std::atomic<uint32_t> scopePos{0};
  uint32_t guiW = 980, guiH = 720;  // resizable (clamped in gui_adjust_size)
  std::atomic<bool> processing{false};
  // ADR-024: the inertia KNOB value (params/state domain). The core holds
  // sqrt(knob) — squaring the core value back is not bit-exact, and
  // state_check demands exact round-trips, so the knob domain gets this one
  // documented slot. Everything else stays core.p-authoritative.
  double inertiaKnob = 0;
  // ADR-059 tune-then-lock: taper exponent for the inertia knob. 0.5 == the
  // ADR-024 sqrt taper (bit-inert default). Higher = gentler onset just after 0
  // (the low-detune+retrigger steepness). DEV control — dial by ear, then the
  // chosen value gets hardcoded and this param + slider removed.
  double inertiaCurve = 0.5;
  // ADR-026 shell voice-mode state (audio-thread only)
  double voiceMono = 0, voiceLegato = 1;
  // ADR-082 classified transpose (35/36/37) PER-OSCILLATOR — "an octave down
  // replaces what a sub would do" — but increment 2 left this shell state as a
  // single copy, so editing osc 2's pitch was silently dropped and the GUI
  // poll snapped the control back (human report 2026-08-07). One copy per
  // oscillator; pitchBend stays global (the wheel bends the patch).
  double octaveA[kMaxOsc] = {0}, semiA[kMaxOsc] = {0}, fineCentsA[kMaxOsc] = {0};
  // B24: mute/solo targets, and the smoothed gain the mix actually applies.
  // Smoothed because a hard 1->0 on a ringing oscillator is a click; same
  // one-pole the master fader uses.
  double oscMute[kMaxOsc] = {0}, oscSolo[kMaxOsc] = {0};
  double oscGainSm[kMaxOsc] = {1.0, 1.0};
  double oscPeakViz[kMaxOsc] = {0};   // per-oscillator meter, drained by publishViz

  // Mute wins over solo; any solo anywhere silences every non-soloed
  // oscillator. Computed from the targets, never stored, so the two params
  // remain the single source of truth (a cached "anySolo" flag is one more
  // thing to forget to update).
  /* Gate one oscillator's block and take its meter reading.
     `chunked` says whether this buffer is a slice of a larger render (the
     temp-chunk path): the smoothing coefficient is per-sample either way, so
     the only difference is that a chunked call must NOT reset the peak.
     Gain 1.0 with nothing to smooth skips the multiply entirely, which is what
     keeps an untouched patch bit-identical to a pre-mixer build. */
  void applyOscGainAndMeter(uint32_t k, float *bL, float *bR, int n, bool chunked)
  {
    const double target = oscGainTarget(k);
    const double c = gainSmoothCoef();
    double g = oscGainSm[k];
    double peak = chunked ? oscPeakViz[k] : 0.0;
    const bool settled = g == target;
    for (int i = 0; i < n; i++)
    {
      if (!settled)
      {
        g += (target - g) * c;
        if (std::fabs(g - target) < 1e-6) g = target;
      }
      if (g != 1.0)
      {
        bL[i] = (float)(bL[i] * g);
        bR[i] = (float)(bR[i] * g);
      }
      const double a = std::fabs((double)bL[i]) > std::fabs((double)bR[i])
                           ? std::fabs((double)bL[i]) : std::fabs((double)bR[i]);
      if (a > peak) peak = a;
    }
    oscGainSm[k] = g;
    oscPeakViz[k] = peak;
  }

  // Same ~8 ms one-pole the master fader uses. Shared so the two faders in the
  // mixer cannot drift apart in feel, and so there is one place to change it.
  double gainSmoothCoef() const
  {
    return 1.0 - std::exp(-1.0 / (0.008 * sampleRate));
  }

  double oscGainTarget(uint32_t k) const
  {
    if (oscMute[k] != 0) return 0.0;
    bool anySolo = false;
    for (uint32_t i = 0; i < kNumOsc; i++)
      if (oscSolo[i] != 0) { anySolo = true; break; }
    return (!anySolo || oscSolo[k] != 0) ? 1.0 : 0.0;
  }
  double pitchBend = 0, gSemi = 0, gFine = 0, gOct = 0;   // global transpose (101/102/103)

  /* BEND TRAVEL LAW (glide_core, folded 2026-08-19). `pitchBend` is now the
     SOUNDING bend; `bendTarget` is where the wheel asked it to go. With the law
     OFF they are the same value and the code below is a pass-through — kOff sets
     `x = target; vel = 0; y = target`, which is the property that lets this land
     in the audio path with parity provably unmoved (147/147 + subdiv + the
     sample-rate probe) BEFORE any law is exposed.
     The law params do not exist yet, so `bendActive()` is false everywhere today
     and the render takes exactly the path it took before this change. That is
     deliberate: wire first, prove inert, expose second. */
  hypersaw::GlideCore bendGlide{44100.0 / 16.0, /*bendLane=*/true};
  // The core calls kConstRate its "ratified default" — that is the BENCH's default
  // for auditioning. Shipping it would change how every existing patch bends, so
  // the PLUGIN ships kOff (human ruling 2026-08-19), matching the precedent that
  // oscillators above the first default to silent: a default must not rewrite a
  // sound that already exists.
  hypersaw::GlideCore::Params bendLaw = [] {
    hypersaw::GlideCore::Params q;
    q.model = hypersaw::GlideCore::kOff;
    return q;
  }();
  double bendTarget = 0;
  int bendAccum = 0;                     // samples owed to the bend grid
  int bendGridSamples() const
  {
    const int g = (int)std::lround(sampleRate * kBendGridSeconds);
    return g < 1 ? 1 : g;
  }
  bool bendActive() const { return (int)bendLaw.model != hypersaw::GlideCore::kOff; }
  // ADR-035 bass-mono output stage: ONE 2nd-order TPT SVF high-pass on the
  // SIDE channel (L = M + HP(S), R = M − HP(S)) — lows collapse to mid with
  // no crossover phase mismatch, the classic vinyl-elliptic routing.
  double bassMonoOn = 0, bassMonoHz = 120;
  double masterVol = 1.0, masterVolSm = 1.0;   // B24: target + smoothed
  // Which oscillator the visuals describe. GUI-owned (follows the OSC tab),
  // audio-thread-read. The visuals were hardwired to oscillator 0 — the
  // intermediary the human asked for is this one index.
  std::atomic<uint32_t> vizOsc{0};
  double bmIc1 = 0, bmIc2 = 0;

  void updateTune(uint32_t k)
  {
    const double st = 12.0 * (octaveA[k] + gOct) + semiA[k] + gSemi + pitchBend +
                      (fineCentsA[k] + gFine) / 100.0;
    const double factor = st == 0.0 ? 1.0 : std::pow(2.0, st / 12.0);
    cores[k].setParam("tune", factor);
    if (k == 0)
      spectra.setParam("tune", factor);  // ADR-057: SPECTRA rides osc 0's transpose (legacy path)
  }
  void updateTuneAll()
  {
    for (uint32_t k = 0; k < kNumOsc; k++) updateTune(k);
  }
  struct Held
  {
    int16_t key;
    double freq;
  };
  Held heldStack[16];
  int heldCount = 0;
  int monoSlot = -1;
  /* Identity-initialised: a slot that was never bound behaves exactly as the
     old code did rather than indexing on uninitialised memory. The map is a
     correction to an assumption, so its unset state must be that assumption. */
  struct InitSlotMap {
    explicit InitSlotMap(int (*m)[kNumOsc]) {
      for (uint32_t s = 0; s < hypersaw::kPoly; s++)
        for (uint32_t k = 0; k < kNumOsc; k++) m[s][k] = (int)s;
    }
  } initSlotMap{slotOf};

  // Host note identity per swarm slot, for CLAP NOTE_END: hosts use note-end
  // to retire per-note bookkeeping, and without it some (Live via the VST3
  // wrapper) withhold retriggering a pitch until they believe the previous
  // note ended — the 2026-07-18 "retrigger doesn't overlap" report.
  struct NoteTag
  {
    int32_t noteId = -1;
    int16_t port = -1, channel = -1, key = -1;
    bool active = false;
  };
  NoteTag tags[hypersaw::kPoly];
  // RETIRED TAGS AWAITING NOTE_END (2026-07-31, the mono-poison bug). A mono
  // retarget — and a poly voice steal — OVERWRITES tags[slot] with the new
  // note, so the old note's identity is gone before emitNoteEnds could ever
  // end it. The wrapper's table then carries that note as sounding FOREVER:
  // Live withholds retriggering its key, the damage survives switching modes
  // (nothing re-ends it), and a fast arpeggiator "fixes" it by cycling every
  // key through a fresh on/off/END — the human's exact diagnostic. Every
  // overwrite of an active tag now queues the old identity here; emitNoteEnds
  // flushes the queue unconditionally each block.
  NoteTag pendingEnds[2 * hypersaw::kPoly];
  int pendingEndCount = 0;
  void retireTag(int slot)
  {
    if (!tags[slot].active) return;
    if (pendingEndCount < (int)(sizeof(pendingEnds) / sizeof(pendingEnds[0])))
      pendingEnds[pendingEndCount++] = tags[slot];
    tags[slot].active = false;
  }

  // ADR-038: latched per-channel MPE pitch bend, in semitones. MPE hosts
  // send member-channel bend BEFORE the note-on it modifies, so the latch —
  // not the event — is what a fresh strike must read. Channel index 0 is the
  // MPE manager / plain single-channel MIDI and is deliberately excluded:
  // member channels are 2-16 (indices 1-15), and applying the ±48 st MPE
  // range to a normal ±2 st bend wheel on channel 1 would be wildly wrong.
  double mpeBendSemis[16] = {0};

  void emitNoteEnds(const clap_output_events_t *out, uint32_t time)
  {
    int kept = 0;
    for (int k = 0; k < pendingEndCount; k++)
    {
      clap_event_note_t ev{};
      ev.header.size = sizeof(ev);
      ev.header.time = time;
      ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
      ev.header.type = CLAP_EVENT_NOTE_END;
      ev.note_id = pendingEnds[k].noteId;
      ev.port_index = pendingEnds[k].port;
      ev.channel = pendingEnds[k].channel;
      ev.key = pendingEnds[k].key;
      ev.velocity = 0;
      // KEEP IT IF THE PUSH IS REJECTED. try_push CAN fail — the host's output
      // buffer is finite and drainQueue floods it with param events whenever a
      // knob moves. Ignoring the return value silently DESTROYED the NOTE_END,
      // so Live never learned the note ended and withheld retriggering that
      // pitch: "stuck for longer than it should, most when I've recently
      // changed the K value" (human, 2026-08-03). Survivors are compacted and
      // retried next block.
      if (out->try_push(out, &ev.header)) continue;
      pendingEnds[kept++] = pendingEnds[k];
    }
    pendingEndCount = kept;
    for (int i = 0; i < hypersaw::kPoly; i++)
    {
      if (!tags[i].active) continue;
      // EMIT ON RELEASE, NOT ENV DEATH (2026-07-31 redesign, test round 1).
      // Live gates RETRIGGERING a pitch on receiving this note's END — the
      // 2026-07-18 finding that motivated emission. Emitting at env death made
      // the host wait on an invisible ~1.1 s tail: inconsistent minimum note
      // durations, laggy release, mono re-press blocked until the tail died.
      // gate==0 is the moment the musical note ended; the DSP tail keeps
      // sounding regardless (hosts do not gate our audio). The re-press guard
      // below still covers the one residual ordering hazard: an off and a
      // re-press of the SAME key landing in the same block.
      const bool dead = spectraMode() ? !spectra.voiceAt(i).gate : !core.voiceAt(i).gate;
      if (!dead) continue;
      // RE-PRESS GUARD (2026-07-31, the stuck-note bug): if this key+channel is
      // still HELD in another slot, do NOT end it yet. Hosts without real note
      // ids (Live via the VST3/AU wrappers sends note_id -1) match NOTE_END by
      // key+channel, so ending the DYING old instance of a re-pressed key
      // poisons the wrapper's bookkeeping for the NEW held instance — its
      // eventual note-off is swallowed and the gate sticks on forever. Fast
      // typing re-presses keys inside the previous release tail constantly
      // ("almost every note is getting stuck", poly + computer keyboard); a
      // piano roll never overlaps a key with its own tail, which is why it was
      // immune. Deferring is safe for id-matching hosts too: the END still
      // fires once the LAST instance of the key dies.
      bool keyStillHeld = false;
      for (int j = 0; j < hypersaw::kPoly; j++)
      {
        if (j == i || !tags[j].active) continue;
        if (tags[j].key != tags[i].key || tags[j].channel != tags[i].channel) continue;
        const bool jDead = spectraMode()
                               ? (!spectra.voiceAt(j).gate && spectra.voiceAt(j).env < 1e-4)
                               : (!core.voiceAt(j).gate && core.voiceAt(j).env < 1e-4);
        if (!jDead) { keyStillHeld = true; break; }
      }
      if (keyStillHeld) continue;
      clap_event_note_t ev{};
      ev.header.size = sizeof(ev);
      ev.header.time = time;
      ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
      ev.header.type = CLAP_EVENT_NOTE_END;
      ev.note_id = tags[i].noteId;
      ev.port_index = tags[i].port;
      ev.channel = tags[i].channel;
      ev.key = tags[i].key;
      ev.velocity = 0;
      // Same rule: only retire the tag once the host has ACCEPTED the end.
      // A rejected push leaves the tag active so the next block tries again —
      // the note is resolved late rather than never.
      if (out->try_push(out, &ev.header)) tags[i].active = false;
    }
  }

  void enqueueParam(uint32_t id, double value, uint8_t kind)
  {
    const uint32_t head = qHead.load(std::memory_order_relaxed);
    if (head - qTail.load(std::memory_order_acquire) >= kQCap) return;  // drop on overflow
    queue[head % kQCap] = {id, value, kind};
    qHead.store(head + 1, std::memory_order_release);
    if (hostParams && hostParams->request_flush) hostParams->request_flush(host);
  }

  void drainQueue(const clap_output_events_t *out)
  {
    uint32_t tail = qTail.load(std::memory_order_relaxed);
    const uint32_t head = qHead.load(std::memory_order_acquire);
    while (tail != head)
    {
      const ParamMsg &m = queue[tail % kQCap];
      if (m.kind == 0)
      {
        applyParam(m.id, m.value);
        if (out)
        {
          clap_event_param_value_t ev{};
          ev.header.size = sizeof(ev);
          ev.header.time = 0;
          ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
          ev.header.type = CLAP_EVENT_PARAM_VALUE;
          ev.param_id = m.id;
          ev.cookie = nullptr;
          ev.note_id = -1;
          ev.port_index = -1;
          ev.channel = -1;
          ev.key = -1;
          ev.value = m.value;
          out->try_push(out, &ev.header);
        }
      }
      else if (out)
      {
        clap_event_param_gesture_t ev{};
        ev.header.size = sizeof(ev);
        ev.header.time = 0;
        ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
        ev.header.type =
            m.kind == 1 ? CLAP_EVENT_PARAM_GESTURE_BEGIN : CLAP_EVENT_PARAM_GESTURE_END;
        ev.param_id = m.id;
        out->try_push(out, &ev.header);
      }
      tail++;
    }
    qTail.store(tail, std::memory_order_release);
  }

  void publishViz()
  {
    // THE INTERMEDIARY (human, 2026-08-07): every per-swarm visual reads the
    // oscillator the GUI is editing, not oscillator 0. Slot indices stay
    // aligned across cores because noteOn/noteOff fan out in order.
    const uint32_t vo = vizOsc.load(std::memory_order_relaxed);
    hypersaw::SwarmCore &vc = cores[vo < kNumOsc ? vo : 0];
    const int writeIdx = 1 - vizPublished.load(std::memory_order_relaxed);
    hypersaw::VizSnapshot &v = vizBuf[writeIdx];
    // NOTE MONITOR — built here, BEFORE the engine branch, and from whichever
    // engine is sounding. It used to live inside the SAW-only path after the
    // SPECTRA early-return, so in SPECTRA mode there was no monitor AT ALL and
    // a stuck voice there was invisible by construction (human, 2026-08-03:
    // "a properly stuck note that isn't expressing on the notes tab").
    int nm = 0;
    for (int i = 0; i < hypersaw::kPoly && nm < 16; i++)
    {
      const int gate = spectraMode() ? spectra.voiceAt(i).gate : vc.voiceAt(i).gate;
      const double env = spectraMode() ? spectra.voiceAt(i).env : vc.voiceAt(i).env;
      // 1e-9, not 1e-4: the render skip-test also uses 1e-4, so a voice just
      // under it was invisible to the monitor while still being rendered. The
      // human hit a note that was audible and ABSENT from the tab (2026-08-03,
      // SAW, no FX), so the monitor must never be the thing that is silent.
      if (!gate && env < 1e-9) continue;
      v.nmMidi[nm] = spectraMode() ? spectra.voiceAt(i).midi : vc.voiceAt(i).midi;
      v.nmGate[nm] = gate;
      v.nmEnv[nm] = env;
      nm++;
    }
    v.nmCount = nm;
    // OUTPUT PEAK, published alongside the monitor. If sound continues while
    // this reads silence, the plugin is not the source — a question that has
    // cost real debugging time twice now and should be answerable at a glance.
    v.outPeak = outPeakViz;
    outPeakViz = 0;
    for (uint32_t k = 0; k < kNumOsc && k < 4; k++)
    {
      v.oscPeak[k] = oscPeakViz[k];
      oscPeakViz[k] = 0;
    }
    if (spectraMode())
    {
      // SPECTRA viz: partial-0's cloud drives the phase circle (v.R/psi/phase),
      // and the per-partial strip feed (v.partR/partAmp/partPhase) carries the
      // whole harmonic series — the cascade lock-front made visible.
      const auto *fs = spectra.focus();
      { const int keep = v.nmCount;
        int km[16]; int kg[16]; double ke[16];
        for (int i = 0; i < keep; i++) { km[i] = v.nmMidi[i]; kg[i] = v.nmGate[i]; ke[i] = v.nmEnv[i]; }
        v = hypersaw::VizSnapshot{};
        v.nmCount = keep;
        for (int i = 0; i < keep; i++) { v.nmMidi[i] = km[i]; v.nmGate[i] = kg[i]; v.nmEnv[i] = ke[i]; } }
      if (fs)
      {
        v.active = true;
        v.spectra = true;
        const int P = (int)spectra.p.partials, M = (int)spectra.p.cloud;
        v.partials = P;
        v.cloud = M;
        v.n = M;
        v.R = fs->R[0];
        v.psi = fs->psi[0];
        v.sigma = fs->sigma[0];
        v.KsmS = fs->KsmS[0];
        v.KsmP = fs->KsmP[0];
        for (int i = 0; i < M && i < 32; i++) v.phase[i] = fs->phase[i];
        for (int k = 0; k < P && k < 32; k++)
        {
          v.partR[k] = fs->R[k];
          v.partAmp[k] = spectra.partialAmp(k);
          for (int m = 0; m < M && m < 7; m++)
            v.partPhase[k * 7 + m] = fs->phase[k * hypersaw::SpectraCore::kMMax + m];
        }
      }
      vizPublished.store(writeIdx, std::memory_order_release);
      return;
    }
    const auto *s = vc.focus();
    if (!s)
    {
      v = hypersaw::VizSnapshot{};
    }
    else
    {
      v.active = true;
      v.n = (int)vc.p.n;
      v.centerIdx = vc.centerIndex();
      v.R = s->R;
      v.RN = s->RN;
      v.psi = s->psi;
      v.sigma = s->sigma;
      v.KsmS = s->KsmS;
      v.KsmP = s->KsmP;
      for (int i = 0; i < v.n && i < 32; i++) v.phase[i] = s->phase[i];
      // voice map: focus swarm's placement vs actual, plus its pan seats
      v.sampleRate = sampleRate;
      v.vmF0 = s->f0cur;
      for (int i = 0; i < v.n && i < 32; i++)
      {
        v.vmVf[i] = s->vf[i];
        v.vmEff[i] = s->eff[i];
        v.vmPan[i] = vc.panEffAt(i);
      }
      // Per-voice envelope shape (ADR-077/078 scatter made visible). Coefficients
      // are one-poles, so the time constant is the inverse of the derivation in
      // the core: c = 1 - exp(-1/(t*sr))  ->  t = -1/(sr*ln(1-c)). Published
      // from the coefficients the core is ACTUALLY using, so the display cannot
      // disagree with the sound.
      {
        const bool perVoice = vc.p.onsetScatter > 0 || vc.p.voiceEnv > 0.5;
        v.envCount = perVoice ? (v.n < 32 ? v.n : 32) : 0;
        const auto tOf = [&](double c) {
          return (c > 0 && c < 1) ? -1000.0 / (sampleRate * std::log(1.0 - c)) : 0.0;
        };
        for (int i = 0; i < v.envCount; i++)
        {
          v.envOnsetMs[i] = s->onsD0[i] / sampleRate * 1000.0;
          v.envAtkMs[i] = tOf(s->onsC[i]);
          v.envRelMs[i] = tOf(s->relC[i]);
        }
      }
      // dynamics layer
      v.topo = (int)vc.p.topo;
      v.poles = (int)vc.p.poles;
      v.RA = s->RA;
      v.RB = s->RB;
      v.RQ = s->RQ;
      v.gravCount = vc.gravCount < 4 ? vc.gravCount : 4;
      for (int i = 0; i < v.gravCount; i++)
      {
        v.gravRatio[i] = vc.gravPairs[i][0];
        v.gravOct[i] = vc.gravPairs[i][1];
        v.gravErr[i] = vc.gravErr[i];
      }
      // note monitor: every slot, gated or ringing
      // grid status (ADR-016/017): unit, occupied rungs, cause-AND-state lock
      v.gridActive = ((int)vc.p.law == 3);
      if (v.gridActive)
      {
        v.gridU = (vc.p.bpm / 60.0) * vc.p.beatMult;
        int rungCount = 0;
        double seen[32];
        for (int i = 0; i < v.n && i < 32; i++)
        {
          const double rung = std::round((s->vf[i] - s->f0cur * vc.p.tune) / v.gridU);
          bool dup = false;
          for (int j = 0; j < rungCount; j++)
            if (seen[j] == rung) dup = true;
          if (!dup && rungCount < 32) seen[rungCount++] = rung;
        }
        v.gridRungs = rungCount;
        const bool coupled = s->KsmS > 0.05;
        const bool coherent = s->R > 0.8 || s->RQ > 0.8 || (v.topo == 2 && s->RA > 0.8 && s->RB > 0.8);
        v.gridLockWarn = coupled && coherent;
      }
    }
    vizPublished.store(writeIdx, std::memory_order_release);
  }

  // GUI-thread spectrum: last 2048 ring samples, Hann, radix-2 FFT, then
  // 96 log-spaced bins 30 Hz..16 kHz normalized from a -80 dB floor.
  void computeSpectrum(float *out, int nBins)
  {
    constexpr int N = 2048;
    static thread_local double re[N], im[N];
    const uint32_t w = specPos.load(std::memory_order_acquire);
    for (int i = 0; i < N; i++)
    {
      const double hann = 0.5 - 0.5 * std::cos(2 * 3.141592653589793 * i / N);
      re[i] = (double)specRing[(w - N + i) & 4095] * hann;
      im[i] = 0;
    }
    // iterative radix-2
    for (int i = 1, j = 0; i < N; i++)
    {
      int bit = N >> 1;
      for (; j & bit; bit >>= 1) j ^= bit;
      j ^= bit;
      if (i < j)
      {
        std::swap(re[i], re[j]);
        std::swap(im[i], im[j]);
      }
    }
    for (int len = 2; len <= N; len <<= 1)
    {
      const double ang = -2 * 3.141592653589793 / len;
      const double wr = std::cos(ang), wi = std::sin(ang);
      for (int i = 0; i < N; i += len)
      {
        double cr = 1, ci = 0;
        for (int k = 0; k < len / 2; k++)
        {
          const double ur = re[i + k], ui = im[i + k];
          const double vr = re[i + k + len / 2] * cr - im[i + k + len / 2] * ci;
          const double vi = re[i + k + len / 2] * ci + im[i + k + len / 2] * cr;
          re[i + k] = ur + vr;
          im[i + k] = ui + vi;
          re[i + k + len / 2] = ur - vr;
          im[i + k + len / 2] = ui - vi;
          const double ncr = cr * wr - ci * wi;
          ci = cr * wi + ci * wr;
          cr = ncr;
        }
      }
    }
    const double binHz = sampleRate / N;
    for (int b = 0; b < nBins; b++)
    {
      const double f = 30.0 * std::pow(16000.0 / 30.0, (double)b / (nBins - 1));
      int bin = (int)(f / binHz);
      if (bin < 1) bin = 1;
      if (bin > N / 2 - 1) bin = N / 2 - 1;
      const double mag = std::hypot(re[bin], im[bin]) / (N / 4);
      const double db = 20 * std::log10(mag + 1e-9);
      double v = (db + 80.0) / 80.0;
      out[b] = (float)(v < 0 ? 0 : (v > 1 ? 1 : v));
    }
  }

  // Defaults for EVERY id, same shape and same loop as paramsJson so the two
  // cannot disagree about which ids exist. This is what makes the defaults
  // survive the GUI: they live in the shell and are served to whatever asks —
  // webview today, anything else later — rather than living in HTML attributes
  // that vanish with the markup.
  std::string defaultsJson() const
  {
    std::string out = "{";
    char buf[48];
    for (uint32_t k = 0; k < kNumOsc; k++)
      for (const auto &d : kParams)
      {
        if (k > 0 && isGlobalId(d.id)) continue;
        const clap_id id = (clap_id)(d.id + k * kOscStride);
        std::snprintf(buf, sizeof(buf), "%s\"%u\":%.6g", out.size() > 1 ? "," : "", id,
                      defaultFor(d, k));
        out += buf;
      }
    out += "}";
    return out;
  }

  std::string paramsJson() const
  {
    // ADR-082: emit EVERY oscillator's block, not just oscillator 0. Without
    // this the GUI cannot see — let alone edit — the second oscillator, which
    // is the whole point of increment 2.
    //
    // It also lets the GUI DERIVE which params are global instead of carrying
    // a copy of kGlobalIds: a base id with no `+kOscStride` sibling in this
    // JSON is global. A hand-maintained second list would drift from this one
    // within a release, and the drift would be silent — the GUI would simply
    // edit the wrong oscillator.
    std::string out = "{";
    char buf[48];
    for (uint32_t k = 0; k < kNumOsc; k++)
      for (const auto &d : kParams)
      {
        if (k > 0 && isGlobalId(d.id)) continue;   // globals exist once
        const clap_id id = (clap_id)(d.id + k * kOscStride);
        std::snprintf(buf, sizeof(buf), "%s\"%u\":%.6g", out.size() > 1 ? "," : "", id,
                      readParam(id));
        out += buf;
      }
    return out + "}";
  }

  std::string stateJson() const
  {
    // The debug dump IS the preset format (ROADMAP Phase 2 design position):
    // one schema, provenance included (SPEC §5.7).
    std::string out = "{\"plugin\":\"HYPERSAW\",\"schema\":1,\"params\":{";
    char buf[64];
    bool first = true;
    for (const auto &d : kParams)
    {
      std::snprintf(buf, sizeof(buf), "%s\"%s\":%.17g", first ? "" : ",", d.coreKey,
                    readParam(d.id));
      out += buf;
      first = false;
    }
    return out + "}}";
  }

  bool applyStateJson(const std::string &json)
  {
    // Tolerant flat scan of our own schema: for each known coreKey, find
    // "key" and parse the number after the colon. Queued to the audio
    // thread — never applied directly from the GUI thread.
    if (json.find("\"params\"") == std::string::npos) return false;
    bool any = false;
    for (const auto &d : kParams)
    {
      const std::string needle = "\"" + std::string(d.coreKey) + "\"";
      size_t pos = json.find(needle);
      if (pos == std::string::npos) continue;
      pos = json.find(':', pos + needle.size());
      if (pos == std::string::npos) continue;
      enqueueParam(d.id, std::atof(json.c_str() + pos + 1), 0);
      any = true;
    }
    return any;
  }

  void applyParam(clap_id id, double value)
  {
    if (const ParamDef *d = findParam(id))
    {
      double v = std::max(d->minV, std::min(d->maxV, value));
      if (id == 23) v = snapGridStep(v);  // rational beat increments only
      // Inertia knob taper (ADR-024): core w = sqrt(knob) spreads the useful
      // heavy range across the knob (measured: the raw map leaves w in
      // 0.02..0.3 a dead plateau at musical K). Core DSP untouched — the
      // taper lives here; readParam inverts it.
      if (id == 11)
      {
        inertiaKnob = v;
        // ADR-059: 0.5 uses sqrt EXACTLY (bit-identical to the ADR-024 default);
        // other exponents use pow. Default knob feel is unchanged.
        v = inertiaCurve == 0.5 ? std::sqrt(v) : std::pow(v, inertiaCurve);
      }
      if (id == 70)  // ADR-059 dev: inertia taper exponent; re-derive inertia now
      {
        inertiaCurve = v;
        core.setParam("inertia",
                      inertiaCurve == 0.5 ? std::sqrt(inertiaKnob) : std::pow(inertiaKnob, inertiaCurve));
        return;
      }
      const double applied = d->stepped ? std::round(v) : v;
      if (id == 32)
      {
        if (applied != voiceMono)
        {
          allOffAll();
          heldCount = 0;
          monoSlot = -1;
        }
        voiceMono = applied;
        return;
      }
      if (id == 34)
      {
        voiceLegato = applied;
        return;
      }
      if (baseIdOf(id) == 104 || baseIdOf(id) == 105)
      {
        const uint32_t osc = oscOfId(id);
        if (osc < kNumOsc)
        {
          if (baseIdOf(id) == 104) oscMute[osc] = applied;
          else oscSolo[osc] = applied;
        }
        return;
      }
      if (baseIdOf(id) == 35 || baseIdOf(id) == 36 || baseIdOf(id) == 37)
      {
        const uint32_t osc = oscOfId(id);
        if (osc < kNumOsc)
        {
          const clap_id base = baseIdOf(id);
          if (base == 35) octaveA[osc] = applied;
          else if (base == 36) semiA[osc] = applied;
          else fineCentsA[osc] = applied;
          updateTune(osc);
        }
        return;
      }
      /* BEND LAW. Routed here rather than through a core setParam() because the
         law lives in the SHELL's GlideCore, not in an oscillator: bend is global.
         Switching the law resets the filter to the current sounding bend so a
         change of law cannot make the pitch jump — the state carries over, only
         the trajectory changes. */
      if (id >= 106 && id <= 115)
      {
        switch (id)
        {
          case 106:
            if ((int)applied != (int)bendLaw.model)
            {
              bendLaw.model = applied;
              bendGlide.reset(pitchBend);
              bendAccum = 0;
              // Leaving a law re-arrives instantly: with kOff the target IS the
              // value, so settle now rather than at the next grid boundary.
              if (!bendActive() && pitchBend != bendTarget)
              {
                pitchBend = bendTarget;
                updateTuneAll();
              }
            }
            break;
          case 107: bendLaw.gtime = applied; break;
          case 108: bendLaw.rate = applied; break;
          case 109: bendLaw.tau = applied; break;
          case 110: bendLaw.springF = applied; break;
          case 111: bendLaw.damp = applied; break;
          case 112: bendLaw.distOver = applied; break;
          case 113: bendLaw.retMul = applied; break;
          case 114: bendLaw.quant = applied; break;
          case 115: bendLaw.qhyst = applied; break;
          default: break;
        }
        return;
      }
      /* GLOBAL SCALE -> the mask the quantiser actually reads. Written straight
         into `bendLaw` because bend is the only consumer today; when the chord
         layer or an arp arrives this becomes a shared struct they all read, which
         is the whole reason the surface is global rather than bend's property. */
      if (id >= 116 && id <= 128)
      {
        if (id == 116) bendLaw.scaleRoot = applied;
        else bendLaw.scaleMask[id - 117] = applied >= 0.5 ? 1 : 0;
        return;
      }
      if (id == 38)
      {
        // The wheel sets a TARGET. With the law off the glide is a pass-through,
        // so this stays the instant write it has always been — byte-identical,
        // not merely equivalent. With a law on, the render advances toward it on
        // the bend grid.
        bendTarget = applied;
        if (!bendActive())
        {
          pitchBend = applied;
          bendGlide.reset(applied);
          updateTuneAll();
        }
        return;
      }
      if (id == 40)
      {
        if (applied != 0 && bassMonoOn == 0) bmIc1 = bmIc2 = 0;  // clean engage
        bassMonoOn = applied;
        return;
      }
      if (id == 41)
      {
        bassMonoHz = applied;
        return;
      }
      if (id == 100)
      {
        masterVol = applied;   // smoothing happens in process()
        return;
      }
      if (id == 101) { gSemi = applied; updateTuneAll(); return; }
      if (id == 102) { gFine = applied; updateTuneAll(); return; }
      if (id == 103) { gOct = applied; updateTuneAll(); return; }
      if (id == 43)
      {
        if (applied != engineSel)
        {
          allOffAll();
          spectra.allOff();
          heldCount = 0;
          monoSlot = -1;
          for (auto &t : tags) t.active = false;
        }
        engineSel = applied;
        return;
      }
      if ((id >= 44 && id <= 55) || (id >= 65 && id <= 68))  // 65-68: SPECTRA ADSR (ADR-055)
      {
        spectra.setParam(d->coreKey, applied);
        return;
      }
      if (id >= 57 && id <= 64)  // ADR-054 FX rack: type/amount pairs → rack
      {
        const int slot = (int)(id - 57) / 2;
        if (((id - 57) & 1) == 0) rack.setType(slot, (int)applied);
        else rack.setAmount(slot, applied);
        return;
      }
      if (id >= 96 && id <= 99)  // per-slot second axis (comb resonance today)
      {
        rack.setTone((int)(id - 96), applied);
        return;
      }
      // Width: the SAW core calls it "width", SPECTRA calls it "swidth" — same
      // stereo-spread control, so one slider (id 14) drives both.
      if (id == 14) spectra.setParam("swidth", applied);
      // ADR-082: ids in a higher block address that oscillator's core. Osc 0
      // keeps every id it had, so this line is unchanged for existing patches.
      const uint32_t osc = oscOfId(id);
      // A GLOBAL core param means "the same value in every oscillator", not
      // "oscillator 0's value". oscOfId() returns 0 for every global id, so
      // this line used to write the Attack knob into cores[0] and nowhere else
      // — measured: with attack at 1.5 s, oscillator 1 reached 90% at 0.955 s
      // while oscillator 2 sat at 0.007 s, its compiled-in default. Every
      // global core param behaved that way, so a two-oscillator patch was half
      // configured and the second half silently ignored the panel.
      // Third instance of the same shape (after the note/lifecycle fan-out and
      // pan motion): an operation whose intent is "every oscillator" written
      // against one. See L0028.
      if (isGlobalId(id))
        for (uint32_t k = 0; k < kNumOsc; k++) cores[k].setParam(d->coreKey, applied);
      else if (osc < kNumOsc)
        cores[osc].setParam(d->coreKey, applied);
      spectra.setParam(d->coreKey, applied);  // shared-name knobs mirror; unknown keys no-op
    }
  }

  double readParam(clap_id id) const
  {
    if (const ParamDef *d = findParam(id))
    {
      // Shell-domain params first; everything else reads the core through the
      // SAME key map setParam uses — no parallel chain to drift (the
      // 2026-07-18 state bug: dynamics params were missing from a duplicated
      // read chain, so get_value fell through to 0 and state saved lies).
      if (d->id == 11) return inertiaKnob;  // ADR-024 knob domain
      if (d->id == 70) return inertiaCurve;  // ADR-059 dev taper exponent
      if (d->id == 32) return voiceMono;
      if (d->id == 34) return voiceLegato;
      if (d->id == 104) return oscMute[oscOfId(id) < kNumOsc ? oscOfId(id) : 0];
      if (d->id == 105) return oscSolo[oscOfId(id) < kNumOsc ? oscOfId(id) : 0];
      if (d->id == 35) return octaveA[oscOfId(id) < kNumOsc ? oscOfId(id) : 0];
      if (d->id == 36) return semiA[oscOfId(id) < kNumOsc ? oscOfId(id) : 0];
      if (d->id == 37) return fineCentsA[oscOfId(id) < kNumOsc ? oscOfId(id) : 0];
      // The wheel's TARGET is the parameter; `pitchBend` is where the glide has
      // currently reached. Reporting the sounding value would make a host read
      // back something the user never set, and would fight automation mid-glide.
      if (d->id == 38) return bendTarget;
      if (d->id >= 106 && d->id <= 115)
      {
        switch (d->id)
        {
          case 106: return bendLaw.model;
          case 107: return bendLaw.gtime;
          case 108: return bendLaw.rate;
          case 109: return bendLaw.tau;
          case 110: return bendLaw.springF;
          case 111: return bendLaw.damp;
          case 112: return bendLaw.distOver;
          case 113: return bendLaw.retMul;
          case 114: return bendLaw.quant;
          case 115: return bendLaw.qhyst;
          default: break;
        }
      }
      if (d->id >= 116 && d->id <= 128)
        return d->id == 116 ? bendLaw.scaleRoot : (double)bendLaw.scaleMask[d->id - 117];
      if (d->id == 40) return bassMonoOn;
      if (d->id == 41) return bassMonoHz;
      if (d->id == 100) return masterVol;
      if (d->id == 101) return gSemi;
      if (d->id == 102) return gFine;
      if (d->id == 103) return gOct;
      if (d->id == 43) return engineSel;
      if ((d->id >= 44 && d->id <= 55) || (d->id >= 65 && d->id <= 68))  // SPECTRA (44-55) + SPECTRA ADSR (ADR-055, 65-68)
        return const_cast<Plugin *>(this)->spectra.getParam(d->coreKey);
      if (d->id >= 57 && d->id <= 64)  // ADR-054 FX rack readback (state/get_value)
      {
        const int slot = (int)(d->id - 57) / 2;
        return ((d->id - 57) & 1) == 0 ? (double)rack.getType(slot) : rack.getAmount(slot);
      }
      if (d->id >= 96 && d->id <= 99) return rack.getTone((int)(d->id - 96));
      // ADR-082: read from the oscillator the id addresses. applyParam was
      // routed by oscillator and this was not, so state_save wrote every
      // `o<k>.` key by reading OSCILLATOR 0 — and state_check's
      // "every param round-trips exactly" passed anyway, because it compares
      // two reads through the same broken accessor. Only the audio comparison
      // caught it. Write path and read path must be routed together.
      const uint32_t osc = oscOfId(id);
      return osc < kNumOsc ? cores[osc].getParam(d->coreKey)
                           : core.getParam(d->coreKey);
    }
    return 0;
  }

  // Shared by NOTE_OFF, NOTE_CHOKE, and the MIDI 1.0 vel-0 convention below.
  void handleNoteOff(const clap_event_note_t *n)
  {
    if (n->key < 0)
    {
      allOffAll();
      spectra.allOff();
      heldCount = 0;
      return;
    }
    if (spectraMode())
    {
      spectra.noteOff(n->key);
      return;
    }
    if (voiceMono != 0)
    {
      // Remove EVERY entry for this key, not just the first. The old loop
      // `break`s on the first match, so a duplicated entry survived a note-off
      // and became a PHANTOM held key — see the note-on guard for how one got
      // in and why that hung the voice. With that guard in place duplicates
      // cannot occur, so this is an invariant restore rather than a second fix:
      // if one ever slips in (a 16-entry overflow drop, or a host sending an
      // off for a key we never saw an on for), a leftover entry is exactly what
      // hangs the voice. Order is preserved, so last-note priority is unchanged.
      {
        int w = 0;
        for (int i = 0; i < heldCount; i++)
          if (heldStack[i].key != n->key) heldStack[w++] = heldStack[i];
        heldCount = w;
      }
      if (monoSlot >= 0 && core.voiceAt(monoSlot).midi == n->key)
      {
        if (heldCount > 0)
        {
          const Held &top = heldStack[heldCount - 1];
          retargetAll(monoSlot, top.key, top.freq, voiceLegato != 0);
          tags[monoSlot].key = top.key;
        }
        else
        {
          noteOffAll(n->key);
        }
      }
    }
    else
    {
      noteOffAll(n->key);
    }
  }

  void handleEvent(const clap_event_header_t *ev)
  {
    if (ev->space_id != CLAP_CORE_EVENT_SPACE_ID) return;
    switch (ev->type)
    {
      case CLAP_EVENT_NOTE_ON:
      {
        auto *n = reinterpret_cast<const clap_event_note_t *>(ev);
        recordNote(ev, n);
        sawNotes.fetch_add(1, std::memory_order_relaxed);
        if (n->channel > 0) sawNonZeroChan.fetch_add(1, std::memory_order_relaxed);
        // MIDI 1.0: note-on velocity 0 IS a note-off, and the AU wrapper
        // forwards controller 0x90-vel-0 releases verbatim (ADR-038). This
        // synth ignores velocity, so without the remap such a release struck
        // a fresh full-gain voice that no note-off ever ends — the
        // 2026-07-18 "doesn't stop when you let go" hang.
        if (n->velocity <= 0.0)
        {
          handleNoteOff(n);
          break;
        }
        const double freq = 440.0 * std::pow(2.0, (n->key - 69) / 12.0);
        // ADR-071: note-context feed for the rack's per-note comb — common to
        // both engines (the comb resonates whatever is played, SAW or SPECTRA).
        rack.noteOn(n->key, freq);
        if (spectraMode())
        {
          // SPECTRA v1: plain poly (mono/glide are SAW-side features; ADR-037).
          // No MPE bend re-apply here — SpectraCore has no noteTune (ADR-038's
          // per-note pitch is SAW-side until the kernel unification).
          const int slot = spectra.noteOn(n->key, freq);
          retireTag(slot);
          tags[slot] = {n->note_id, n->port_index, n->channel, n->key, true};
          break;
        }
        int struck;
        if (voiceMono != 0)
        {
          // Glide/legato engage only when another key is still HELD (human
          // clarification 2026-07-18) — a ringing release tail alone gets a
          // fresh strike on a new slot, overlapping the tail naturally.
          // A mono held-stack is the set of keys currently DOWN, so a key
          // cannot appear in it twice. This used to push unconditionally, so a
          // duplicate NOTE_ON for an already-held key — which a computer
          // keyboard played fast produces and a piano roll never does — pushed a
          // second entry. The note-off path then removed only one of them and
          // saw heldCount > 0, so it RETARGETED the voice to the phantom key
          // instead of releasing it, and the note hung forever. That is the
          // human's 2026-07-26 report ("notes get stuck for longer than they
          // ought to when I play quickly ... hasn't happened with preprogrammed
          // MIDI in the piano roll") — it read as finite only because a later
          // press-and-release of the same key cleared the phantom.
          // Measured: mono+restrike went 0/25 seeds silent -> 25/25.
          // A re-press is therefore "move to the top" (last-note priority), and
          // `anotherHeld` is evaluated AFTER that removal, so re-pressing the
          // ONLY held key is a fresh strike rather than a retarget to itself.
          int dupAt = -1;
          for (int i = 0; i < heldCount; i++)
            if (heldStack[i].key == n->key) { dupAt = i; break; }
          if (dupAt >= 0)
          {
            for (int j = dupAt; j < heldCount - 1; j++) heldStack[j] = heldStack[j + 1];
            heldCount--;
          }
          const bool anotherHeld = heldCount > 0;
          if (heldCount < 16) heldStack[heldCount++] = {n->key, freq};
          const bool voiceGated = monoSlot >= 0 && core.voiceAt(monoSlot).gate;
          if (anotherHeld && voiceGated)
          {
            const bool keep = voiceLegato != 0;
            retargetAll(monoSlot, n->key, freq, keep);
          }
          else
          {
            // MONO INVARIANT: at most ONE gated voice. Taking the fresh-strike
            // path while the previous mono voice is still GATED orphans it —
            // every release path keys off monoSlot's current midi, so once
            // monoSlot moves on, nothing can ever release the orphan.
            // Minimal repro found by notefuzz_check --minimal:
            //   on(61) on(61) on(60) off(61) off(60)  -> voice on 61 hangs.
            // The re-press sends the second on down this path, the on(60)
            // retargets monoSlot away, and the off(61) then finds
            // monoSlot.midi != 61 and does nothing at all.
            // Only a GATED voice is force-released here: a ringing RELEASE tail
            // has gate == 0, so the intended tail-overlap behaviour above is
            // untouched.
            if (monoSlot >= 0 && core.voiceAt(monoSlot).gate)
              noteOffAll(core.voiceAt(monoSlot).midi);
            monoSlot = core.noteOn(n->key, freq);
            core.setNoteVelocity(monoSlot, n->velocity);
            bindSlots(monoSlot, 0, monoSlot);
            for (uint32_t k = 1; k < kNumOsc; k++)
            {
              const int sk = cores[k].noteOn(n->key, freq);
              cores[k].setNoteVelocity(sk, n->velocity);
              bindSlots(monoSlot, k, sk);   // sk may differ from monoSlot
            }
          }
          retireTag(monoSlot);
          tags[monoSlot] = {n->note_id, n->port_index, n->channel, n->key, true};
          struck = monoSlot;
        }
        else
        {
          const int slot = core.noteOn(n->key, freq);
          core.setNoteVelocity(slot, n->velocity);
          bindSlots(slot, 0, slot);
          for (uint32_t k = 1; k < kNumOsc; k++)
          {
            const int sk = cores[k].noteOn(n->key, freq);
            cores[k].setNoteVelocity(sk, n->velocity);
            bindSlots(slot, k, sk);   // sk may differ from slot
          }
          retireTag(slot);
          tags[slot] = {n->note_id, n->port_index, n->channel, n->key, true};
          struck = slot;
        }
        // ADR-038: a fresh strike resets noteTune (ADR-036), so re-apply the
        // channel's latched MPE bend — MPE hosts sent it before this note-on.
        if (n->channel >= 1 && n->channel < 16 && mpeBendSemis[n->channel] != 0.0)
          setNoteExprAll(struck, mpeBendSemis[n->channel]);
        break;
      }
      case CLAP_EVENT_NOTE_OFF:
      case CLAP_EVENT_NOTE_CHOKE:
      {
        // Single note-off path (spectra dispatch lives inside handleNoteOff;
        // the vel-0 NOTE_ON remap above routes through the same code).
        recordNote(ev, reinterpret_cast<const clap_event_note_t *>(ev));
        handleNoteOff(reinterpret_cast<const clap_event_note_t *>(ev));
        break;
      }
      case CLAP_EVENT_NOTE_EXPRESSION:
        sawExprs.fetch_add(1, std::memory_order_relaxed);
      {
        // MPE per-note pitch (ADR-036): hosts deliver per-note bend as the
        // TUNING expression in relative semitones; CLAP wildcard matching
        // (-1) applies. Reaches the core through the ADR-027 live-tune seam.
        auto *x = reinterpret_cast<const clap_event_note_expression_t *>(ev);
        // ADR-084: PRESSURE -> per-voice gain (default mapping the human asked
        // for). Same tag-matching as TUNING; fan out to every oscillator, since
        // note fan-out keeps slot indices aligned.
        if (x->expression_id == CLAP_NOTE_EXPRESSION_PRESSURE)
        {
          for (int i = 0; i < hypersaw::kPoly; i++)
            if (tags[i].active &&
                (x->note_id == -1 || tags[i].noteId == x->note_id) &&
                (x->key == -1 || tags[i].key == x->key) &&
                (x->channel == -1 || tags[i].channel == x->channel))
            {
              setNotePressureAll(i, x->value);
            }
          break;
        }
        if (x->expression_id != CLAP_NOTE_EXPRESSION_TUNING) break;
        for (int i = 0; i < hypersaw::kPoly; i++)
        {
          if (!tags[i].active) continue;
          const NoteTag &t = tags[i];
          if ((x->note_id == -1 || x->note_id == t.noteId) &&
              (x->port_index == -1 || x->port_index == t.port) &&
              (x->channel == -1 || x->channel == t.channel) &&
              (x->key == -1 || x->key == t.key))
            setNoteExprAll(i, x->value);
        }
        break;
      }
      case CLAP_EVENT_MIDI:
      {
        // MPE member-channel pitch bend (ADR-038). Live (VST3, via the
        // wrapper's IMidiMapping params) and Logic (AU, raw MIDI) deliver
        // MPE bend as per-channel 0xE0 on rotating member channels 2-16 —
        // NOT as note expressions — at the MPE default range of ±48 st.
        // Channel 1 (index 0) is excluded: see mpeBendSemis.
        auto *m = reinterpret_cast<const clap_event_midi_t *>(ev);
        const int ch = m->data[0] & 0x0F;
        if ((m->data[0] & 0xF0) != 0xE0 || ch == 0) break;
        const int v14 = (int)m->data[1] | ((int)m->data[2] << 7);
        const double semis = (v14 - 8192) * (48.0 / 8192.0);
        mpeBendSemis[ch] = semis;
        for (int i = 0; i < hypersaw::kPoly; i++)
          if (tags[i].active && tags[i].channel == ch) setNoteExprAll(i, semis);
        break;
      }
      case CLAP_EVENT_PARAM_VALUE:
      {
        auto *pv = reinterpret_cast<const clap_event_param_value_t *>(ev);
        applyParam(pv->param_id, pv->value);
        break;
      }
      case CLAP_EVENT_TRANSPORT:
      {
        auto *tr = reinterpret_cast<const clap_event_transport_t *>(ev);
        if (tr->flags & CLAP_TRANSPORT_HAS_TEMPO) core.p.bpm = tr->tempo;
        break;
      }
      default:
        break;
    }
  }

  /* ONE span of oscillator rendering, extracted so the bend grid can cut a block
     into grid-sized pieces without a second copy of this logic. Two copies of a
     mix stage is how they disagree — the same reason the generated GUI derives
     its controls instead of hand-placing them. */
  void renderSpan(float *outL, float *outR, uint32_t at, uint32_t count)
  {
    const int n = (int)count;
    core.render(outL + at, outR + at, n);
    // Oscillator 0 renders STRAIGHT into the output, so its mute/solo gain
    // and meter are applied in place afterwards rather than during a sum.
    applyOscGainAndMeter(0, outL + at, outR + at, n, false);
    // Oscillators 1..N-1 render into a FIXED STACK buffer, in chunks, and
    // sum. At their default vol = 0 they add exact zeros, so a patch that
    // never touches them is bit-identical to a one-oscillator build — which
    // is what keeps the 147 parity goldens green.
    //
    // Stack, not a heap scratch. The first version sized a std::vector at
    // activate() and skipped the oscillator when the buffer was too small;
    // that made AUDIBLE OUTPUT conditional on activate() having run, so a
    // restored instance silently lost oscillator 1 (state_check caught it:
    // "restored instance renders bit-identical audio" went red). A chunk
    // loop over a fixed buffer cannot allocate, cannot depend on block
    // size, and cannot silently drop a voice.
    for (uint32_t k = 1; k < kNumOsc; k++)
    {
      float tL[kMixChunk], tR[kMixChunk];
      for (int off = 0; off < n; off += kMixChunk)
      {
        const int m = n - off < kMixChunk ? n - off : kMixChunk;
        cores[k].render(tL, tR, m);
        applyOscGainAndMeter(k, tL, tR, m, true);
        for (int i = 0; i < m; i++)
        {
          outL[at + off + i] += tL[i];
          outR[at + off + i] += tR[i];
        }
      }
    }
  }

  clap_process_status process(const clap_process_t *p)
  {
    // Host tempo drives the grid law (ADR-022); fallback stays at the last
    // known (or default 120) when the host provides none.
    if (p->transport && (p->transport->flags & CLAP_TRANSPORT_HAS_TEMPO))
      core.p.bpm = p->transport->tempo;

    drainQueue(p->out_events);

    float *outL = p->audio_outputs[0].data32[0];
    float *outR = p->audio_outputs[0].data32[1];
    const uint32_t nframes = p->frames_count;
    /* Absolute sample position for the forensic trace. NEVER derived from
       steady_time alone: the first real field dump (2026-08-12, Live via the
       VST3 wrapper) came back with every pos under 512 and NON-MONOTONIC —
       327, 146, 451, 17 — because the host reports steady_time as 0 every
       block, so `pos` was just the in-block offset and events from different
       blocks interleaved meaninglessly. The one column a replay depends on was
       the one that was wrong, and it was wrong in the only environment that
       matters. Count blocks locally and ALWAYS advance; use steady_time only
       as a bonus when the host supplies something plausible. */
    tracePos += nframes;
    blockPos = p->steady_time > 0 ? (uint64_t)p->steady_time : tracePos;
    const uint32_t nev = p->in_events->size(p->in_events);

    uint32_t frame = 0, evIndex = 0;
    while (frame < nframes)
    {
      uint32_t until = nframes;
      while (evIndex < nev)
      {
        const clap_event_header_t *ev = p->in_events->get(p->in_events, evIndex);
        if (ev->time > frame)
        {
          until = ev->time < nframes ? ev->time : nframes;
          break;
        }
        handleEvent(ev);
        ++evIndex;
      }
      /* BEND GRID. Subdividing is deliberately conditional: with no law engaged
         the render takes exactly the span it always took, so this fold cannot
         move a single sample of existing output — the parity claim is by
         CONSTRUCTION, not by measurement agreeing afterwards. When a law IS
         engaged the span is cut on the fixed grid and the tune factor is
         recomputed at each boundary, which is where the bench measured it. */
      if (bendActive() && !spectraMode())
      {
        const int grid = bendGridSamples();
        while (frame < until)
        {
          const uint32_t take = (uint32_t)std::min<int>(grid - bendAccum, (int)(until - frame));
          renderSpan(outL, outR, frame, take);
          frame += take;
          bendAccum += (int)take;
          if (bendAccum >= grid)
          {
            bendAccum = 0;
            const double v = bendGlide.step(bendTarget, bendLaw);
            if (v != pitchBend) { pitchBend = v; updateTuneAll(); }
          }
        }
        continue;   // `frame` is already at `until`
      }
      if (spectraMode())
        spectra.render(outL + frame, outR + frame, (int)(until - frame));
      else
        renderSpan(outL, outR, frame, (uint32_t)(until - frame));
      frame = until;
    }

    // ADR-035 bass mono: runs BEFORE the spectrum feed so the visualizer
    // shows what actually leaves the plugin.
    if (bassMonoOn != 0)
    {
      constexpr double kPi = 3.141592653589793;
      const double fc = std::min(bassMonoHz, 0.45 * sampleRate);
      const double g = std::tan(kPi * fc / sampleRate);
      const double k = 1.4142135623730951;  // Butterworth 2nd order
      const double a0 = 1.0 / (1.0 + g * (g + k));
      for (uint32_t i = 0; i < nframes; i++)
      {
        const double m = 0.5 * (outL[i] + outR[i]);
        const double sIn = 0.5 * (outL[i] - outR[i]);
        const double hp = (sIn - (g + k) * bmIc1 - bmIc2) * a0;
        const double v1 = g * hp;
        const double bp = v1 + bmIc1;
        bmIc1 = bp + v1;
        const double v2 = g * bp;
        bmIc2 = v2 + bmIc2 + v2;
        outL[i] = (float)(m + hp);
        outR[i] = (float)(m - hp);
      }
    }

    // Internal FX rack (ADR-054), now driven THROUGH the B23 crosspoint matrix
    // (ADR-088) rather than as a hardcoded series. Post-oscillator,
    // post-bass-mono; runs before the spectrum feed so the visualizer reflects
    // post-FX output.
    //
    // Bass-mono stays UPSTREAM of the rack. The reorder was considered and
    // dropped: the argument for moving it was that a decorrelating slot
    // downstream could undo the mono guarantee, and measurement refuted it —
    // Comb at amount 0.9 scales the sub-crossover channel difference by 2.2x
    // whether bass-mono is on or off, leaving the same ~11% residual either
    // way, because it is a stereo-SYMMETRIC filter. No current slot type
    // decorrelates, so there is no correctness case, and an audible reorder
    // with no oracle behind it is not one to make on taste.
    //
    // The default topology is setSerialChain(), which reproduces the old
    // `rack.processStereo` chain BIT-EXACTLY: every live edge carries a
    // coefficient of exactly 1.0, so each gather is `0.0f + 1.0*x` and the
    // terminal sum is `0.0f + 1.0*slot3` — both exact in float. That inertness
    // is what keeps the 147 goldens as this change's regression proof, and
    // routing_check asserts it against the real rack rather than trusting it.
    //
    // Fixed stack scratch + chunk loop, matching the oscillator sum above and
    // for the same reason: a heap buffer sized at activate() once made audible
    // output conditional on activate() having run.
    {
      float sL[hypersaw::kRackSlots][kMixChunk], sR[hypersaw::kRackSlots][kMixChunk];
      float *slotL[hypersaw::kRackSlots], *slotR[hypersaw::kRackSlots];
      for (int t = 0; t < hypersaw::kRackSlots; t++) { slotL[t] = sL[t]; slotR[t] = sR[t]; }
      for (uint32_t off = 0; off < nframes; off += (uint32_t)kMixChunk)
      {
        const uint32_t left = nframes - off;
        const int m = (int)(left < (uint32_t)kMixChunk ? left : (uint32_t)kMixChunk);
        const float *srcL[1] = {outL + off};
        const float *srcR[1] = {outR + off};
        routing.processBlock(srcL, srcR, slotL, slotR, outL + off, outR + off, m,
                             [&](int slot, float *L, float *R, int n) {
                               rack.processSlot(slot, L, R, n);
                             });
      }
    }

    // MASTER VOLUME (B24): last in the chain, before the visualizer feed so
    // the meters show what leaves the plugin. One-pole smoothed (~8 ms) with a
    // snap once within 1e-6 of target — the snap is load-bearing: it makes
    // unity EXACTLY 1.0, and the skip below keeps every pre-mixer patch
    // byte-identical rather than "identical up to a converging one-pole".
    {
      const double c = 1.0 - std::exp(-1.0 / (0.008 * sampleRate));
      for (uint32_t i = 0; i < nframes; i++)
      {
        masterVolSm += (masterVol - masterVolSm) * c;
        if (std::fabs(masterVolSm - masterVol) < 1e-6) masterVolSm = masterVol;
        if (masterVolSm != 1.0)
        {
          outL[i] = (float)(outL[i] * masterVolSm);
          outR[i] = (float)(outR[i] * masterVolSm);
        }
      }
    }

    publishViz();
    {
      uint32_t w = specPos.load(std::memory_order_relaxed);
      for (uint32_t i = 0; i < nframes; i++)
        specRing[(w + i) & 4095] = outL[i] + outR[i];
      specPos.store(w + nframes, std::memory_order_release);
      for (uint32_t i = 0; i < nframes; i++)
      {
        const double a = std::fabs((double)outL[i]) + std::fabs((double)outR[i]);
        if (a > outPeakViz) outPeakViz = a;
      }
      uint32_t sw = scopePos.load(std::memory_order_relaxed);
      for (uint32_t i = 0; i < nframes; i++)
      { scopeL[(sw + i) & 2047] = outL[i]; scopeR[(sw + i) & 2047] = outR[i]; }
      scopePos.store(sw + nframes, std::memory_order_release);
    }
    emitNoteEnds(p->out_events, nframes > 0 ? nframes - 1 : 0);

    if (spectraMode() ? (spectra.focus() != nullptr) : (core.focus() != nullptr))
      return CLAP_PROCESS_CONTINUE;
    return CLAP_PROCESS_SLEEP;
  }
};

Plugin *self(const clap_plugin_t *p) { return static_cast<Plugin *>(p->plugin_data); }

/* ---- lifecycle ---- */

bool plug_init(const clap_plugin_t *p)
{
  auto *pl = self(p);
  if (pl->host)
    pl->hostParams = static_cast<const clap_host_params_t *>(
        pl->host->get_extension(pl->host, CLAP_EXT_PARAMS));
  return true;
}

void plug_destroy(const clap_plugin_t *p)
{
#if defined(__APPLE__) || defined(_WIN32)
  delete self(p)->gui;
  self(p)->gui = nullptr;
#endif
  delete self(p);
}

bool plug_activate(const clap_plugin_t *p, double sr, uint32_t, uint32_t)
{
  auto *pl = self(p);
  pl->sampleRate = sr;
  // Recreate the core at the host rate, preserving params (constructor cost
  // is trivial; activate is main-thread and never concurrent with process).
  for (uint32_t k = 0; k < kNumOsc; k++)
  {
    hypersaw::Params saved = pl->cores[k].p;
    pl->cores[k] = hypersaw::SwarmCore(sr);
    pl->cores[k].p = saved;
    pl->cores[k].setParam("seed", saved.seed);  // re-trigger rebuild() with saved state
  }
  hypersaw::SpectraCore::SParams sp = pl->spectra.p;
  pl->spectra = hypersaw::SpectraCore(sr);
  pl->spectra.p = sp;
  pl->spectra.rebuild();
  pl->rack.setSampleRate(sr);  // ADR-071: size comb lines + derive comp coeffs at sr
  return true;
}

void plug_deactivate(const clap_plugin_t *) {}
bool plug_start_processing(const clap_plugin_t *p)
{
  self(p)->processing.store(true, std::memory_order_release);
  return true;
}
void plug_stop_processing(const clap_plugin_t *p)
{
  self(p)->processing.store(false, std::memory_order_release);
}
void plug_reset(const clap_plugin_t *p)
{
  // The host-MPE counters describe the CURRENT note stream, so a reset clears
  // them: after a transport reset the evidence for "no expressions have arrived"
  // has to be re-earned, or the hint would report a stream that is over.
  self(p)->sawNotes.store(0, std::memory_order_relaxed);
  self(p)->sawExprs.store(0, std::memory_order_relaxed);
  self(p)->sawNonZeroChan.store(0, std::memory_order_relaxed);
  auto *pl = self(p);
  pl->allOffAll();
  for (double &b : pl->mpeBendSemis) b = 0.0;
}

clap_process_status plug_process(const clap_plugin_t *p, const clap_process_t *proc)
{
  return self(p)->process(proc);
}

/* ---- audio/note ports (unchanged from Phase 0) ---- */

uint32_t aports_count(const clap_plugin_t *, bool is_input) { return is_input ? 0 : 1; }

bool aports_get(const clap_plugin_t *, uint32_t index, bool is_input, clap_audio_port_info_t *info)
{
  if (is_input || index != 0) return false;
  info->id = 0;
  std::snprintf(info->name, sizeof(info->name), "%s", "Main Out");
  info->flags = CLAP_AUDIO_PORT_IS_MAIN;
  info->channel_count = 2;
  info->port_type = CLAP_PORT_STEREO;
  info->in_place_pair = CLAP_INVALID_ID;
  return true;
}

const clap_plugin_audio_ports_t s_audio_ports = {aports_count, aports_get};

uint32_t nports_count(const clap_plugin_t *, bool is_input) { return is_input ? 1 : 0; }

bool nports_get(const clap_plugin_t *, uint32_t index, bool is_input, clap_note_port_info_t *info)
{
  if (!is_input || index != 0) return false;
  info->id = 0;
  std::snprintf(info->name, sizeof(info->name), "%s", "Note In");
  info->supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI;
  info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
  return true;
}

const clap_plugin_note_ports_t s_note_ports = {nports_count, nports_get};

/* ---- params extension ---- */

// Osc 0 occupies indices [0, kNumParams) EXACTLY as before, so at kNumOsc == 1
// the enumeration a host sees is unchanged, index for index and id for id.
// Higher oscillators append their per-osc params after it.
uint32_t params_count(const clap_plugin_t *)
{
  return kNumParams + (kNumOsc - 1) * perOscParamCount();
}

bool params_get_info(const clap_plugin_t *, uint32_t index, clap_param_info_t *info)
{
  uint32_t osc = 0;
  const ParamDef *dp = nullptr;
  if (index < kNumParams) { dp = &kParams[index]; }
  else
  {
    uint32_t rest = index - kNumParams;
    const uint32_t per = perOscParamCount();
    if (per == 0) return false;
    osc = 1 + rest / per;
    if (osc >= kNumOsc) return false;
    uint32_t want = rest % per;
    for (const auto &d : kParams)
      if (!isGlobalId(d.id) && want-- == 0) { dp = &d; break; }
    if (!dp) return false;
  }
  const ParamDef &d = *dp;
  info->id = (clap_id)(d.id + osc * kOscStride);
  info->flags = CLAP_PARAM_IS_AUTOMATABLE;
  if (d.stepped) info->flags |= CLAP_PARAM_IS_STEPPED;
  info->cookie = nullptr;
  if (osc == 0)
    std::snprintf(info->name, sizeof(info->name), "%s", d.name);
  else
    std::snprintf(info->name, sizeof(info->name), "Osc%u %s", osc + 1, d.name);
  std::snprintf(info->module, sizeof(info->module), "%s",
                osc == 0 ? "" : (osc == 1 ? "Osc 2" : "Osc 3"));
  info->min_value = d.minV;
  info->max_value = d.maxV;
  // Oscillators above the first default to SILENT (vol = 0). Without this the
  // second oscillator would sound the instant kNumOsc rose, changing every
  // existing patch — a host "reset to defaults" must give silence too, not
  // just our constructor.
  info->default_value = defaultFor(d, osc);
  return true;
}

bool params_get_value(const clap_plugin_t *p, clap_id id, double *out)
{
  if (!findParam(id)) return false;
  *out = self(p)->readParam(id);
  return true;
}

bool params_value_to_text(const clap_plugin_t *, clap_id id, double value, char *out,
                          uint32_t cap)
{
  const ParamDef *d = findParam(id);
  if (!d) return false;
  if (d->labels)
  {
    const int idx = (int)std::round(value) - (int)d->minV;
    const int span = (int)(d->maxV - d->minV);
    if (idx >= 0 && idx <= span) std::snprintf(out, cap, "%s", d->labels[idx]);
    else std::snprintf(out, cap, "%d", (int)std::round(value));
  }
  else if (d->stepped)
  {
    std::snprintf(out, cap, "%d", (int)std::round(value));
  }
  else if (id == 8)  // dissolve: seconds
  {
    std::snprintf(out, cap, "%.2f s", value);
  }
  else if (id == 19 || id == 20 || id == 22)  // envelope times
  {
    if (value < 0.01) std::snprintf(out, cap, "%.1f ms", value * 1000);
    else std::snprintf(out, cap, "%.2f s", value);
  }
  else if (id == 33)  // glide seconds
  {
    if (value < 0.001) std::snprintf(out, cap, "off");
    else if (value < 0.01) std::snprintf(out, cap, "%.1f ms", value * 1000);
    else std::snprintf(out, cap, "%.2f s", value);
  }
  else if (baseIdOf(id) == 35)  // octave (any oscillator block)
  {
    std::snprintf(out, cap, "%+d oct", (int)std::round(value));
  }
  else if (baseIdOf(id) == 36)
  {
    std::snprintf(out, cap, "%+d st", (int)std::round(value));
  }
  else if (id == 27)
  {
    std::snprintf(out, cap, "%+.0f deg", value);
  }
  else if (baseIdOf(id) == 37)
  {
    std::snprintf(out, cap, "%+.1f c", value);
  }
  else if (id == 38)
  {
    std::snprintf(out, cap, "%+.2f st", value);
  }
  else if (id == 23)  // grid cycles/beat: named rational division
  {
    const char *name = gridStepName(snapGridStep(value));
    std::snprintf(out, cap, "%s/beat", name ? name : "?");
  }
  else if (id == 9)  // drift depth: cents
  {
    std::snprintf(out, cap, "%.1f c", value);
  }
  else if (id == 10)  // drift rate knob 0..1 -> walk speed 0.2..8.2 per second
  {
    std::snprintf(out, cap, "%.1f /s", 0.2 + value * 8);
  }
  else
  {
    std::snprintf(out, cap, "%.3f", value);
  }
  return true;
}

bool params_text_to_value(const clap_plugin_t *, clap_id id, const char *text, double *out)
{
  const ParamDef *d = findParam(id);
  if (!d) return false;
  if (d->labels)
  {
    const int span = (int)(d->maxV - d->minV);
    for (int i = 0; i <= span; i++)
      if (!std::strcmp(text, d->labels[i]))
      {
        *out = i;
        return true;
      }
  }
  *out = std::atof(text);
  return true;
}

void params_flush(const clap_plugin_t *p, const clap_input_events_t *in,
                  const clap_output_events_t *out)
{
  self(p)->drainQueue(out);
  const uint32_t nev = in->size(in);
  for (uint32_t i = 0; i < nev; i++) self(p)->handleEvent(in->get(in, i));
}

const clap_plugin_params_t s_params = {params_count, params_get_info, params_get_value,
                                       params_value_to_text, params_text_to_value, params_flush};

/* ---- state extension: versioned key=value text ---- */

/* ---- OSCILLATOR PRESETS (B20) -------------------------------------------
   The format and filtering live in src/osc_preset.h and are gated by
   tools/preset_check.cpp. The plugin-side wiring (bind read/write to
   readParam/applyParam with the +kOscStride offset) is NOT here yet, on
   purpose: it would have no caller until the osc-page GUI exists, and
   unreachable code rots quietly — it compiles forever while the surface it
   assumed drifts underneath it. It lands with the GUI that calls it, in the
   same change, so it is exercised the day it ships. */

bool state_save(const clap_plugin_t *p, const clap_ostream_t *stream)
{
  // ADR-082: oscillator 0's keys are UNCHANGED, so every existing patch keeps
  // loading bit-identically and state_check stays the regression proof. Higher
  // oscillators prefix `o<k>.`. At kNumOsc == 1 this emits exactly the old
  // bytes, header included — which is the point of increment 1.
  std::string blob = kNumOsc > 1 ? "hypersaw-state 2\n" : "hypersaw-state 1\n";
  char line[80];
  for (const auto &d : kParams)
  {
    std::snprintf(line, sizeof(line), "%s=%.17g\n", d.coreKey, self(p)->readParam(d.id));
    blob += line;
  }
  for (uint32_t k = 1; k < kNumOsc; k++)
    for (const auto &d : kParams)
    {
      if (isGlobalId(d.id)) continue;
      std::snprintf(line, sizeof(line), "o%u.%s=%.17g\n", k, d.coreKey,
                    self(p)->readParam((clap_id)(d.id + k * kOscStride)));
      blob += line;
    }
  int64_t written = 0;
  while (written < (int64_t)blob.size())
  {
    const int64_t n =
        stream->write(stream, blob.data() + written, (uint64_t)(blob.size() - written));
    if (n <= 0) return false;
    written += n;
  }
  return true;
}

bool state_load(const clap_plugin_t *p, const clap_istream_t *stream)
{
  std::string blob;
  char buf[512];
  int64_t n;
  while ((n = stream->read(stream, buf, sizeof(buf))) > 0) blob.append(buf, (size_t)n);
  if (n < 0) return false;
  // Version 2 adds `o<k>.` keys; version 1 is still accepted and simply leaves
  // the higher oscillators at their defaults (i.e. silent) — forward and
  // backward compatible, which append-only ids buy us for free.
  const bool v1 = blob.rfind("hypersaw-state 1\n", 0) == 0;
  const bool v2 = blob.rfind("hypersaw-state 2\n", 0) == 0;
  if (!v1 && !v2) return false;
  size_t pos = blob.find('\n') + 1;
  auto *pl = self(p);
  while (pos < blob.size())
  {
    const size_t eol = blob.find('\n', pos);
    const std::string line = blob.substr(pos, eol == std::string::npos ? std::string::npos
                                                                       : eol - pos);
    pos = eol == std::string::npos ? blob.size() : eol + 1;
    const size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string key = line.substr(0, eq);
    const double val = std::atof(line.c_str() + eq + 1);
    // ADR-082: split an `o<k>.` prefix off the key and resolve it to that
    // oscillator's block. A prefix naming an oscillator this build does not
    // have falls through to the existing unknown-key path (ignored), which is
    // how a 2-osc patch stays loadable by a 1-osc build.
    uint32_t keyOsc = 0;
    if (key.size() > 2 && key[0] == 'o' && key.find('.') != std::string::npos)
    {
      const size_t dot = key.find('.');
      bool digits = dot > 1;
      for (size_t i = 1; i < dot && digits; i++) digits = key[i] >= '0' && key[i] <= '9';
      if (digits)
      {
        keyOsc = (uint32_t)std::atoi(key.c_str() + 1);
        key = key.substr(dot + 1);
      }
    }
    if (keyOsc >= kNumOsc && keyOsc != 0) continue;   // block this build lacks
    const clap_id idOff = (clap_id)(keyOsc * kOscStride);
    // Thread safety (2026-07-18): state_load is main-thread and MAY run while
    // the audio thread is in process() — a direct setParam would race
    // rebuild() against render(). Idle: apply directly (hosts read values
    // back immediately after setState). Processing: route through the param
    // queue; the audio thread applies next block and drainQueue's outgoing
    // param events tell the host the new values.
    if (pl->processing.load(std::memory_order_acquire))
    {
      for (const auto &d : kParams)
        if (key == d.coreKey)
        {
          if (keyOsc && isGlobalId(d.id)) break;   // globals have no per-osc mirror
          pl->enqueueParam((clap_id)(d.id + idOff), val, 0);
          break;
        }
    }
    else
    {
      // Route through applyParam (not core.setParam) so layer mappings like
      // the ADR-024 inertia taper apply identically on both load paths.
      bool known = false;
      for (const auto &d : kParams)
        if (key == d.coreKey)
        {
          if (keyOsc && isGlobalId(d.id)) break;   // globals have no per-osc mirror
          pl->applyParam((clap_id)(d.id + idOff), val);
          known = true;
          break;
        }
      if (!known) continue;  // unknown/future keys ignored (state_check pins this)
    }
  }
  return true;
}

const clap_plugin_state_t s_state = {state_save, state_load};

/* ---- gui extension (macOS/cocoa + Windows/win32 via the seam; the win32
       backend is CI-compile-verified, runtime validation is a recorded
       residual — see the Phase 2 trace) ---- */
#if defined(__APPLE__) || defined(_WIN32)

#ifdef __APPLE__
#define HYPERSAW_WINDOW_API CLAP_WINDOW_API_COCOA
#else
#define HYPERSAW_WINDOW_API CLAP_WINDOW_API_WIN32
#endif

bool gui_is_api_supported(const clap_plugin_t *, const char *api, bool is_floating)
{
  return !is_floating && !std::strcmp(api, HYPERSAW_WINDOW_API);
}

bool gui_get_preferred_api(const clap_plugin_t *, const char **api, bool *is_floating)
{
  *api = HYPERSAW_WINDOW_API;
  *is_floating = false;
  return true;
}

bool gui_create(const clap_plugin_t *p, const char *api, bool is_floating)
{
  if (!gui_is_api_supported(p, api, is_floating)) return false;
  auto *pl = self(p);
  if (pl->gui) return true;
  hypersaw::GuiHost hostIf;
  hostIf.getViz = [pl]() {
    return pl->vizBuf[pl->vizPublished.load(std::memory_order_acquire)];
  };
  hostIf.getSpectrum = [pl](float *out, int n) { pl->computeSpectrum(out, n); };
  hostIf.getScope = [pl](float *l, float *r, int n) {
    const uint32_t w = pl->scopePos.load(std::memory_order_acquire);
    for (int i = 0; i < n; i++)
    { const uint32_t k = (w - (uint32_t)n + (uint32_t)i) & 2047;
      l[i] = pl->scopeL[k]; r[i] = pl->scopeR[k]; }
  };
  hostIf.getParamsJson = [pl]() { return pl->paramsJson(); };
  hostIf.getDefaultsJson = [pl]() { return pl->defaultsJson(); };
  hostIf.setParam = [pl](uint32_t id, double v) { pl->enqueueParam(id, v, 0); };
  hostIf.gesture = [pl](uint32_t id, bool begin) { pl->enqueueParam(id, 0, begin ? 1 : 2); };
  // Stamp carries hash AND build time: a hash alone cannot distinguish "the
  // binary I just built" from "a binary built from the same commit last week",
  // which is precisely the stale-install question (L0020).
  // PANIC: kill everything a stuck note could be hiding in. There was no such
  // control at all before 2026-08-03, so a stuck voice meant deleting the
  // device. Clears both engines, every note tag (including the pending-END
  // queue), the mono held-stack, and the FX rack's tails.
  hostIf.panic = [pl]() { pl->panicWithDump(); };
  hostIf.getBuildId = []() { return std::string(HYPERSAW_BUILD_STAMP); };
  hostIf.getHostHint = [pl]() { return pl->hostHint(); };
  hostIf.setVizOsc = [pl](uint32_t k) { pl->vizOsc.store(k, std::memory_order_relaxed); };
  hostIf.getStateJson = [pl]() { return pl->stateJson(); };
  hostIf.applyStateJson = [pl](const std::string &s) { return pl->applyStateJson(s); };
  pl->gui = new hypersaw::HypersawGui(std::move(hostIf));
  return true;
}

void gui_destroy(const clap_plugin_t *p)
{
  auto *pl = self(p);
  delete pl->gui;
  pl->gui = nullptr;
}

bool gui_set_scale(const clap_plugin_t *, double) { return true; }

bool gui_get_size(const clap_plugin_t *p, uint32_t *w, uint32_t *h)
{
  *w = self(p)->guiW;
  *h = self(p)->guiH;
  return true;
}

bool gui_can_resize(const clap_plugin_t *) { return true; }

bool gui_get_resize_hints(const clap_plugin_t *, clap_gui_resize_hints_t *hints)
{
  hints->can_resize_horizontally = true;
  hints->can_resize_vertically = true;
  hints->preserve_aspect_ratio = false;
  hints->aspect_ratio_width = 0;
  hints->aspect_ratio_height = 0;
  return true;
}

bool gui_adjust_size(const clap_plugin_t *, uint32_t *w, uint32_t *h)
{
  *w = std::max(720u, std::min(1600u, *w));
  *h = std::max(440u, std::min(1000u, *h));
  return true;
}

bool gui_set_size(const clap_plugin_t *p, uint32_t w, uint32_t h)
{
  auto *pl = self(p);
  pl->guiW = w;
  pl->guiH = h;
  return true;  // the webview child autoresizes with the reparented view
}

bool gui_set_parent(const clap_plugin_t *p, const clap_window_t *window)
{
  auto *pl = self(p);
  if (!pl->gui || !window) return false;
#ifdef __APPLE__
  return pl->gui->attachToParent(window->cocoa);
#else
  return pl->gui->attachToParent(window->win32);
#endif
}

bool gui_set_transient(const clap_plugin_t *, const clap_window_t *) { return false; }
void gui_suggest_title(const clap_plugin_t *, const char *) {}
bool gui_show(const clap_plugin_t *) { return true; }
bool gui_hide(const clap_plugin_t *) { return true; }

const clap_plugin_gui_t s_gui = {gui_is_api_supported, gui_get_preferred_api, gui_create,
                                 gui_destroy,          gui_set_scale,         gui_get_size,
                                 gui_can_resize,       gui_get_resize_hints,  gui_adjust_size,
                                 gui_set_size,         gui_set_parent,        gui_set_transient,
                                 gui_suggest_title,    gui_show,              gui_hide};

#endif  // __APPLE__ || _WIN32

/* ---- clap-wrapper VST3 specifics (ADR-038) ----
 * Without this extension the VST3 wrapper advertises only PRESSURE through
 * INoteExpressionController (its CLAP_SUPPORTS_ALL_NOTE_EXPRESSIONS compile
 * flag defaults OFF and make_clapfirst_plugins never forwards it), so
 * note-expression-speaking hosts never send the per-note TUNING stream
 * ADR-036 listens for. PRESSURE is kept to match the wrapper's default. */
uint32_t v3spec_num_midi_channels(const clap_plugin *, uint32_t) { return 16; }
uint32_t v3spec_note_expressions(const clap_plugin *)
{
  return AS_VST3_NOTE_EXPRESSION_TUNING | AS_VST3_NOTE_EXPRESSION_PRESSURE;
}
const clap_plugin_as_vst3_t s_vst3_specifics = {v3spec_num_midi_channels, v3spec_note_expressions};

const void *plug_get_extension(const clap_plugin_t *, const char *id)
{
  if (!std::strcmp(id, CLAP_EXT_AUDIO_PORTS)) return &s_audio_ports;
  if (!std::strcmp(id, CLAP_EXT_NOTE_PORTS)) return &s_note_ports;
  if (!std::strcmp(id, CLAP_EXT_PARAMS)) return &s_params;
  if (!std::strcmp(id, CLAP_EXT_STATE)) return &s_state;
  if (!std::strcmp(id, CLAP_PLUGIN_AS_VST3)) return &s_vst3_specifics;
#if defined(__APPLE__) || defined(_WIN32)
  if (!std::strcmp(id, CLAP_EXT_GUI)) return &s_gui;
#endif
  return nullptr;
}

void plug_on_main_thread(const clap_plugin_t *) {}

/* ---- factory ---- */

uint32_t factory_get_plugin_count(const clap_plugin_factory *) { return 1; }

const clap_plugin_descriptor_t *factory_get_plugin_descriptor(const clap_plugin_factory *,
                                                              uint32_t index)
{
  return index == 0 ? &s_desc : nullptr;
}

const clap_plugin_t *factory_create_plugin(const clap_plugin_factory *, const clap_host_t *host,
                                           const char *plugin_id)
{
  if (std::strcmp(plugin_id, s_desc.id) != 0) return nullptr;
  auto *pl = new Plugin();
  pl->host = host;
  pl->plugin.desc = &s_desc;
  pl->plugin.plugin_data = pl;
  pl->plugin.init = plug_init;
  pl->plugin.destroy = plug_destroy;
  pl->plugin.activate = plug_activate;
  pl->plugin.deactivate = plug_deactivate;
  pl->plugin.start_processing = plug_start_processing;
  pl->plugin.stop_processing = plug_stop_processing;
  pl->plugin.reset = plug_reset;
  pl->plugin.process = plug_process;
  pl->plugin.get_extension = plug_get_extension;
  pl->plugin.on_main_thread = plug_on_main_thread;
  return &pl->plugin;
}

const clap_plugin_factory_t s_factory = {factory_get_plugin_count, factory_get_plugin_descriptor,
                                         factory_create_plugin};

}  // namespace

extern "C"
{
  const char *hypersaw_test_host_hint(const clap_plugin_t *p)
{
  static std::string held;
  held = self(p)->hostHint();
  return held.c_str();
}

const char *hypersaw_test_panic(const clap_plugin_t *p)
{
  static std::string held;
  self(p)->panicWithDump();
  held = self(p)->lastDumpPath;
  return held.empty() ? nullptr : held.c_str();
}

const char *hypersaw_test_dump_forensics(const clap_plugin_t *p, const char *why)
{
  static std::string held;
  held = self(p)->dumpForensics(why ? why : "test");
  return held.empty() ? nullptr : held.c_str();
}

/* ---- note-bookkeeping introspection, for the FOUNDATIONS conformance suite --
   These are READ-ONLY windows plus ONE shipped mutator (retireTag). They exist
   so an external suite can assert our tag tables without the adapter
   reimplementing any of the behaviour under test: the notes themselves still
   arrive as real CLAP events through the real process() path, and the steal
   decision still happens where it lives (swarm_core.h alloc()). An adapter that
   recomputed "who should have been stolen" would be an oracle checking its own
   copy of the rule (L0031). */

int hypersaw_test_poly(void) { return (int)hypersaw::kPoly; }

bool hypersaw_test_tag_at(const clap_plugin_t *p, int slot, int32_t *note_id, int16_t *port,
                          int16_t *channel, int16_t *key)
{
  if (slot < 0 || slot >= (int)hypersaw::kPoly) return false;
  const auto &t = self(p)->tags[slot];
  if (note_id) *note_id = t.noteId;
  if (port) *port = t.port;
  if (channel) *channel = t.channel;
  if (key) *key = t.key;
  return t.active;
}

/* Calls the SHIPPED retireTag() — the same function a steal and a mono retarget
   call — and reports the identity it took. Returns false when the slot held
   nothing, which is what makes the suite's no-double-END case meaningful: the
   second call must find an inactive tag and yield no identity. */
bool hypersaw_test_retire_slot(const clap_plugin_t *p, int slot, int32_t *note_id, int16_t *port,
                               int16_t *channel, int16_t *key)
{
  if (slot < 0 || slot >= (int)hypersaw::kPoly) return false;
  auto *s = self(p);
  const auto before = s->tags[slot];
  s->retireTag(slot);
  if (!before.active) return false;
  if (note_id) *note_id = before.noteId;
  if (port) *port = before.port;
  if (channel) *channel = before.channel;
  if (key) *key = before.key;
  return true;
}

/* Gate state of the logical voice at `slot`, read from oscillator 0's voice —
   `slotOf[slot][0] == slot` by definition. "Released" for the steal cases means
   gate == 0, which is exactly the predicate alloc()'s tiers read. */
bool hypersaw_test_slot_gated(const clap_plugin_t *p, int slot)
{
  if (slot < 0 || slot >= (int)hypersaw::kPoly) return false;
  return self(p)->core.voiceAt(slot).gate != 0;
}

bool hypersaw_entry_init(const char *) { return true; }
  void hypersaw_entry_deinit(void) {}
  const void *hypersaw_entry_get_factory(const char *factory_id)
  {
    if (!std::strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID)) return &s_factory;
    return nullptr;
  }
}
