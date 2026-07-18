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
#include <clap/clap.h>

#include "swarm_core.h"
#include "gui/hypersaw_gui.h"
#include "hypersaw_clap_entry.h"

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
                                          "cauchy (seeded)"};
static const char *const kLawLabels[] = {"cents-constant", "Hz-constant", "ERB-flat",
                                         "tempo-grid"};
static const char *const kOffOn[] = {"off", "on"};
static const char *const kTopoLabels[] = {"mean-field", "ring", "two-cluster"};
static const char *const kPolesLabels[] = {"1 — classic", "2 — pair", "3 — triad", "4 — quad"};
// Display names for the gravity ratio readout (indices match core kRatios)
static const char *const kRatioNames[13] = {"1/1", "16/15", "9/8", "6/5", "5/4", "4/3", "7/5",
                                            "3/2", "8/5", "5/3", "16/9", "15/8", "2/1"};

static const ParamDef kParams[] = {
    {1, "n", "Voices", 1, 32, 7, true, nullptr},
    {2, "dist", "Distribution", 0, 3, 1, true, kDistLabels},
    {3, "seed", "Seed", 0, 999999, 1234, true, nullptr},
    {4, "detune", "Detune", 0, 1, 0.28, false, nullptr},
    {5, "law", "Detune Law", 0, 3, 0, true, kLawLabels},
    {6, "K", "Pull K", -1, 1, 0, false, nullptr},
    {7, "onset", "Onset Lock", 0, 1, 0, false, nullptr},
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
    {32, "voiceMono", "Voice: Mono", 0, 1, 0, true, kOffOn},
    {33, "glide", "Glide (s)", 0, 2.0, 0, false, nullptr},
    {34, "voiceLegato", "Voice: Legato", 0, 1, 1, true, kOffOn},
    {35, "octave", "Octave", -2, 2, 0, true, nullptr},
    // Transposition suite (ADR-027): all four combine into ONE live core tune
    // factor — the pitch knob bends sounding notes, not just new ones.
    {36, "semi", "Semitones", -12, 12, 0, true, nullptr},
    {37, "fineCents", "Fine (c)", -100, 100, 0, false, nullptr},
    {38, "pitchBend", "Pitch", -12, 12, 0, false, nullptr},
    {39, "scatter", "Phase Scatter", 0, 1, 0, false, nullptr},  // ADR-033
};
constexpr uint32_t kNumParams = sizeof(kParams) / sizeof(kParams[0]);

const ParamDef *findParam(clap_id id)
{
  for (const auto &d : kParams)
    if (d.id == id) return &d;
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
  hypersaw::SwarmCore core{44100.0};
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
  uint32_t guiW = 980, guiH = 720;  // resizable (clamped in gui_adjust_size)
  std::atomic<bool> processing{false};
  // ADR-024: the inertia KNOB value (params/state domain). The core holds
  // sqrt(knob) — squaring the core value back is not bit-exact, and
  // state_check demands exact round-trips, so the knob domain gets this one
  // documented slot. Everything else stays core.p-authoritative.
  double inertiaKnob = 0;
  // ADR-026 shell voice-mode state (audio-thread only)
  double voiceMono = 0, voiceLegato = 1, octave = 0;
  double semi = 0, fineCents = 0, pitchBend = 0;

  void updateTune()
  {
    const double st = 12.0 * octave + semi + pitchBend + fineCents / 100.0;
    core.setParam("tune", st == 0.0 ? 1.0 : std::pow(2.0, st / 12.0));
  }
  struct Held
  {
    int16_t key;
    double freq;
  };
  Held heldStack[16];
  int heldCount = 0;
  int monoSlot = -1;

  // Host note identity per swarm slot, for CLAP NOTE_END: hosts use note-end
  // to retire per-note bookkeeping, and without it some (Live via the VST3
  // wrapper) withhold retriggering a pitch until they believe the previous
  // note ended — the 2026-07-18 "retrigger doesn't overlap" report.
  struct VoiceTag
  {
    int32_t noteId = -1;
    int16_t port = -1, channel = -1, key = -1;
    bool active = false;
  };
  VoiceTag tags[hypersaw::kPoly];

  void emitNoteEnds(const clap_output_events_t *out, uint32_t time)
  {
    for (int i = 0; i < hypersaw::kPoly; i++)
    {
      if (!tags[i].active) continue;
      const auto &s = core.swarmAt(i);
      const bool nowActive = s.gate || s.env >= 1e-4;  // matches render's skip test
      if (nowActive) continue;
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
      out->try_push(out, &ev.header);
      tags[i].active = false;
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
    const int writeIdx = 1 - vizPublished.load(std::memory_order_relaxed);
    hypersaw::VizSnapshot &v = vizBuf[writeIdx];
    const auto *s = core.focus();
    if (!s)
    {
      v = hypersaw::VizSnapshot{};
    }
    else
    {
      v.active = true;
      v.n = (int)core.p.n;
      v.centerIdx = core.centerIndex();
      v.R = s->R;
      v.RN = s->RN;
      v.psi = s->psi;
      v.sigma = s->sigma;
      v.KsmS = s->KsmS;
      v.KsmP = s->KsmP;
      for (int i = 0; i < v.n && i < 32; i++) v.phase[i] = s->phase[i];
      // dynamics layer
      v.topo = (int)core.p.topo;
      v.poles = (int)core.p.poles;
      v.RA = s->RA;
      v.RB = s->RB;
      v.RQ = s->RQ;
      v.gravCount = core.gravCount < 4 ? core.gravCount : 4;
      for (int i = 0; i < v.gravCount; i++)
      {
        v.gravRatio[i] = core.gravPairs[i][0];
        v.gravOct[i] = core.gravPairs[i][1];
        v.gravErr[i] = core.gravErr[i];
      }
      // grid status (ADR-016/017): unit, occupied rungs, cause-AND-state lock
      v.gridActive = ((int)core.p.law == 3);
      if (v.gridActive)
      {
        v.gridU = (core.p.bpm / 60.0) * core.p.beatMult;
        int rungCount = 0;
        double seen[32];
        for (int i = 0; i < v.n && i < 32; i++)
        {
          const double rung = std::round((s->vf[i] - s->f0cur * core.p.tune) / v.gridU);
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

  std::string paramsJson() const
  {
    std::string out = "{";
    char buf[48];
    for (const auto &d : kParams)
    {
      std::snprintf(buf, sizeof(buf), "%s\"%u\":%.6g", out.size() > 1 ? "," : "", d.id,
                    readParam(d.id));
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
        v = std::sqrt(v);
      }
      const double applied = d->stepped ? std::round(v) : v;
      if (id == 32)
      {
        if (applied != voiceMono)
        {
          core.allOff();
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
      if (id == 35)
      {
        octave = applied;
        updateTune();
        return;
      }
      if (id == 36)
      {
        semi = applied;
        updateTune();
        return;
      }
      if (id == 37)
      {
        fineCents = applied;
        updateTune();
        return;
      }
      if (id == 38)
      {
        pitchBend = applied;
        updateTune();
        return;
      }
      core.setParam(d->coreKey, applied);
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
      if (d->id == 32) return voiceMono;
      if (d->id == 34) return voiceLegato;
      if (d->id == 35) return octave;
      if (d->id == 36) return semi;
      if (d->id == 37) return fineCents;
      if (d->id == 38) return pitchBend;
      return core.getParam(d->coreKey);
    }
    return 0;
  }

  void handleEvent(const clap_event_header_t *ev)
  {
    if (ev->space_id != CLAP_CORE_EVENT_SPACE_ID) return;
    switch (ev->type)
    {
      case CLAP_EVENT_NOTE_ON:
      {
        auto *n = reinterpret_cast<const clap_event_note_t *>(ev);
        const double freq = 440.0 * std::pow(2.0, (n->key - 69) / 12.0);
        if (voiceMono != 0)
        {
          // Glide/legato engage only when another key is still HELD (human
          // clarification 2026-07-18) — a ringing release tail alone gets a
          // fresh strike on a new slot, overlapping the tail naturally.
          const bool anotherHeld = heldCount > 0;
          if (heldCount < 16) heldStack[heldCount++] = {n->key, freq};
          const bool voiceGated = monoSlot >= 0 && core.swarmAt(monoSlot).gate;
          if (anotherHeld && voiceGated)
          {
            const bool keep = voiceLegato != 0;
            core.retargetNote(monoSlot, n->key, freq, keep);
          }
          else
          {
            monoSlot = core.noteOn(n->key, freq);
          }
          tags[monoSlot] = {n->note_id, n->port_index, n->channel, n->key, true};
        }
        else
        {
          const int slot = core.noteOn(n->key, freq);
          tags[slot] = {n->note_id, n->port_index, n->channel, n->key, true};
        }
        break;
      }
      case CLAP_EVENT_NOTE_OFF:
      case CLAP_EVENT_NOTE_CHOKE:
      {
        auto *n = reinterpret_cast<const clap_event_note_t *>(ev);
        if (n->key < 0)
        {
          core.allOff();
          heldCount = 0;
          break;
        }
        if (voiceMono != 0)
        {
          for (int i = 0; i < heldCount; i++)
            if (heldStack[i].key == n->key)
            {
              for (int j = i; j < heldCount - 1; j++) heldStack[j] = heldStack[j + 1];
              heldCount--;
              break;
            }
          if (monoSlot >= 0 && core.swarmAt(monoSlot).midi == n->key)
          {
            if (heldCount > 0)
            {
              const Held &top = heldStack[heldCount - 1];
              core.retargetNote(monoSlot, top.key, top.freq, voiceLegato != 0);
              tags[monoSlot].key = top.key;
            }
            else
            {
              core.noteOff(n->key);
            }
          }
        }
        else
        {
          core.noteOff(n->key);
        }
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
      core.render(outL + frame, outR + frame, (int)(until - frame));
      frame = until;
    }

    publishViz();
    {
      uint32_t w = specPos.load(std::memory_order_relaxed);
      for (uint32_t i = 0; i < nframes; i++)
        specRing[(w + i) & 4095] = outL[i] + outR[i];
      specPos.store(w + nframes, std::memory_order_release);
    }
    emitNoteEnds(p->out_events, nframes > 0 ? nframes - 1 : 0);

    if (core.focus()) return CLAP_PROCESS_CONTINUE;
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
  hypersaw::Params saved = pl->core.p;
  pl->core = hypersaw::SwarmCore(sr);
  pl->core.p = saved;
  pl->core.setParam("seed", saved.seed);  // re-trigger rebuild() with saved state
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
void plug_reset(const clap_plugin_t *p) { self(p)->core.allOff(); }

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

uint32_t params_count(const clap_plugin_t *) { return kNumParams; }

bool params_get_info(const clap_plugin_t *, uint32_t index, clap_param_info_t *info)
{
  if (index >= kNumParams) return false;
  const ParamDef &d = kParams[index];
  info->id = d.id;
  info->flags = CLAP_PARAM_IS_AUTOMATABLE;
  if (d.stepped) info->flags |= CLAP_PARAM_IS_STEPPED;
  info->cookie = nullptr;
  std::snprintf(info->name, sizeof(info->name), "%s", d.name);
  std::snprintf(info->module, sizeof(info->module), "%s", "");
  info->min_value = d.minV;
  info->max_value = d.maxV;
  info->default_value = d.defV;
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
  else if (id == 35)  // octave
  {
    std::snprintf(out, cap, "%+d oct", (int)std::round(value));
  }
  else if (id == 36)
  {
    std::snprintf(out, cap, "%+d st", (int)std::round(value));
  }
  else if (id == 27)
  {
    std::snprintf(out, cap, "%+.0f deg", value);
  }
  else if (id == 37)
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

bool state_save(const clap_plugin_t *p, const clap_ostream_t *stream)
{
  std::string blob = "hypersaw-state 1\n";
  char line[64];
  for (const auto &d : kParams)
  {
    std::snprintf(line, sizeof(line), "%s=%.17g\n", d.coreKey, self(p)->readParam(d.id));
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
  if (blob.rfind("hypersaw-state 1\n", 0) != 0) return false;
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
    const std::string key = line.substr(0, eq);
    const double val = std::atof(line.c_str() + eq + 1);
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
          pl->enqueueParam(d.id, val, 0);
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
          pl->applyParam(d.id, val);
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
  hostIf.getParamsJson = [pl]() { return pl->paramsJson(); };
  hostIf.setParam = [pl](uint32_t id, double v) { pl->enqueueParam(id, v, 0); };
  hostIf.gesture = [pl](uint32_t id, bool begin) { pl->enqueueParam(id, 0, begin ? 1 : 2); };
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

const void *plug_get_extension(const clap_plugin_t *, const char *id)
{
  if (!std::strcmp(id, CLAP_EXT_AUDIO_PORTS)) return &s_audio_ports;
  if (!std::strcmp(id, CLAP_EXT_NOTE_PORTS)) return &s_note_ports;
  if (!std::strcmp(id, CLAP_EXT_PARAMS)) return &s_params;
  if (!std::strcmp(id, CLAP_EXT_STATE)) return &s_state;
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
  bool hypersaw_entry_init(const char *) { return true; }
  void hypersaw_entry_deinit(void) {}
  const void *hypersaw_entry_get_factory(const char *factory_id)
  {
    if (!std::strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID)) return &s_factory;
    return nullptr;
  }
}
