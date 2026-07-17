/*
 * hypersaw_clap.cpp — HYPERSAW CLAP plugin impl (Phase 0 skeleton).
 *
 * Phase 0 scope (ROADMAP): an empty-but-loadable instrument — note ports in,
 * stereo audio out, silence-safe processing. A minimal deterministic sine
 * placeholder voice is included so a host load is audibly verifiable end to
 * end; SwarmCore replaces render() internals in Phase 1 (parity per
 * ACCEPTANCE L0-1). No allocations, no locks, no wall-clock in process().
 */

#include <cstring>
#include <cstdio>
#include <cmath>
#include <clap/clap.h>

#include "hypersaw_clap_entry.h"

namespace
{

constexpr int kMaxVoices = 8;
constexpr double kTau = 6.283185307179586;

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
    "0.0.1",
    "Coupled-oscillator swarm synthesizer (Phase 0 skeleton)",
    &s_features[0]};

struct Voice
{
  bool active = false;
  bool releasing = false;
  int16_t port = 0;
  int16_t channel = 0;
  int16_t key = -1;
  int32_t noteId = -1;
  double phase = 0.0;
  double inc = 0.0;
  double env = 0.0;
  double gain = 0.0;
  uint32_t age = 0;
};

struct Plugin
{
  clap_plugin_t plugin{};
  const clap_host_t *host = nullptr;
  double sampleRate = 44100.0;
  double attackCoef = 0.0;   // per-sample one-pole coefficients, set in activate()
  double releaseCoef = 0.0;
  uint32_t ageCounter = 0;
  Voice voices[kMaxVoices];

  void computeEnvCoefs()
  {
    // AR per SPEC §6.5: ~3-4 ms attack, ~160-180 ms release.
    attackCoef = 1.0 - std::exp(-1.0 / (0.0035 * sampleRate));
    releaseCoef = std::exp(-1.0 / (0.170 * sampleRate));
  }

  Voice *allocVoice()
  {
    // Stealing per SPEC §6.5: free-and-faded first, else oldest by age.
    for (auto &v : voices)
      if (!v.active) return &v;
    Voice *oldest = &voices[0];
    for (auto &v : voices)
      if (v.age < oldest->age) oldest = &v;
    return oldest;
  }

  void noteOn(int16_t port, int16_t channel, int16_t key, int32_t noteId, double velocity)
  {
    Voice *v = allocVoice();
    v->active = true;
    v->releasing = false;
    v->port = port;
    v->channel = channel;
    v->key = key;
    v->noteId = noteId;
    v->phase = 0.0;
    v->inc = 440.0 * std::pow(2.0, (key - 69) / 12.0) / sampleRate;
    v->env = 0.0;
    v->gain = 0.25 * (0.2 + 0.8 * velocity);
    v->age = ageCounter++;
  }

  void noteOff(int16_t port, int16_t channel, int16_t key, int32_t noteId)
  {
    for (auto &v : voices)
    {
      if (!v.active || v.releasing) continue;
      bool match = (key < 0 || v.key == key) && (channel < 0 || v.channel == channel) &&
                   (port < 0 || v.port == port) && (noteId < 0 || v.noteId == noteId);
      if (match) v.releasing = true;
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
        noteOn(n->port_index, n->channel, n->key, n->note_id, n->velocity);
        break;
      }
      case CLAP_EVENT_NOTE_OFF:
      case CLAP_EVENT_NOTE_CHOKE:
      {
        auto *n = reinterpret_cast<const clap_event_note_t *>(ev);
        noteOff(n->port_index, n->channel, n->key, n->note_id);
        break;
      }
      default:
        break;
    }
  }

  void render(float *outL, float *outR, uint32_t start, uint32_t end)
  {
    for (uint32_t i = start; i < end; ++i)
    {
      double s = 0.0;
      for (auto &v : voices)
      {
        if (!v.active) continue;
        if (v.releasing)
        {
          v.env *= releaseCoef;
          if (v.env < 1e-4)
          {
            v.active = false;
            continue;
          }
        }
        else
        {
          v.env += attackCoef * (1.0 - v.env);
        }
        s += v.gain * v.env * std::sin(v.phase * kTau);
        v.phase += v.inc;
        if (v.phase >= 1.0) v.phase -= 1.0;
      }
      // tanh guard per SPEC §6.5 (master bus safety, not tone — headroom is ample here)
      float out = static_cast<float>(std::tanh(s));
      outL[i] = out;
      outR[i] = out;
    }
  }

  clap_process_status process(const clap_process_t *p)
  {
    float *outL = p->audio_outputs[0].data32[0];
    float *outR = p->audio_outputs[0].data32[1];
    const uint32_t nframes = p->frames_count;
    const uint32_t nev = p->in_events->size(p->in_events);

    // Split rendering at event times so note starts/stops are sample-accurate.
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
      render(outL, outR, frame, until);
      frame = until;
    }

    for (const auto &v : voices)
      if (v.active) return CLAP_PROCESS_CONTINUE;
    return CLAP_PROCESS_SLEEP;
  }
};

/* ---- clap_plugin vtable ---- */

Plugin *self(const clap_plugin_t *p) { return static_cast<Plugin *>(p->plugin_data); }

bool plug_init(const clap_plugin_t *) { return true; }

void plug_destroy(const clap_plugin_t *p) { delete self(p); }

bool plug_activate(const clap_plugin_t *p, double sr, uint32_t, uint32_t)
{
  auto *pl = self(p);
  pl->sampleRate = sr;
  pl->computeEnvCoefs();
  return true;
}

void plug_deactivate(const clap_plugin_t *) {}
bool plug_start_processing(const clap_plugin_t *) { return true; }
void plug_stop_processing(const clap_plugin_t *) {}

void plug_reset(const clap_plugin_t *p)
{
  for (auto &v : self(p)->voices) v = Voice{};
}

clap_process_status plug_process(const clap_plugin_t *p, const clap_process_t *proc)
{
  return self(p)->process(proc);
}

/* audio ports: one stereo out */
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

/* note ports: one in */
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

const void *plug_get_extension(const clap_plugin_t *, const char *id)
{
  if (!std::strcmp(id, CLAP_EXT_AUDIO_PORTS)) return &s_audio_ports;
  if (!std::strcmp(id, CLAP_EXT_NOTE_PORTS)) return &s_note_ports;
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
