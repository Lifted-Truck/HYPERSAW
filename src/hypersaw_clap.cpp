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
#include <string>
#include <clap/clap.h>

#include "swarm_core.h"
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
static const char *const kLawLabels[] = {"cents-constant", "Hz-constant", "ERB-flat"};
static const char *const kOffOn[] = {"off", "on"};

static const ParamDef kParams[] = {
    {1, "n", "Voices", 1, 32, 7, true, nullptr},
    {2, "dist", "Distribution", 0, 3, 1, true, kDistLabels},
    {3, "seed", "Seed", 0, 999999, 1234, true, nullptr},
    {4, "detune", "Detune", 0, 1, 0.28, false, nullptr},
    {5, "law", "Detune Law", 0, 2, 0, true, kLawLabels},
    {6, "K", "Pull K", -1, 1, 0, false, nullptr},
    {7, "onset", "Onset Lock", 0, 1, 0, false, nullptr},
    {8, "dissolve", "Dissolve (s)", 0.05, 7.94, 0.63, false, nullptr},
    {9, "driftDepth", "Drift Depth (c)", 0, 25, 0, false, nullptr},
    {10, "driftRate", "Drift Rate", 0, 1, 0.4, false, nullptr},
    {11, "inertia", "Inertia", 0, 1, 0, false, nullptr},
    {12, "rtone", "R->Tone", -1, 1, 0, false, nullptr},
    {13, "normExp", "Density Comp", 0.5, 1, 0.75, false, nullptr},
    {14, "width", "Width", 0, 1, 0.8, false, nullptr},
    {15, "mono", "Mono", 0, 1, 0, true, kOffOn},
    {16, "digital", "Digital", 0, 1, 1, false, nullptr},
    {17, "vol", "Volume", 0, 1, 0.4, false, nullptr},
    {18, "retrig", "Retrigger", 0, 1, 1, true, kOffOn},
};
constexpr uint32_t kNumParams = sizeof(kParams) / sizeof(kParams[0]);

const ParamDef *findParam(clap_id id)
{
  for (const auto &d : kParams)
    if (d.id == id) return &d;
  return nullptr;
}

struct Plugin
{
  clap_plugin_t plugin{};
  const clap_host_t *host = nullptr;
  hypersaw::SwarmCore core{44100.0};
  double sampleRate = 44100.0;

  void applyParam(clap_id id, double value)
  {
    if (const ParamDef *d = findParam(id))
    {
      const double v = std::max(d->minV, std::min(d->maxV, value));
      core.setParam(d->coreKey, d->stepped ? std::round(v) : v);
    }
  }

  double readParam(clap_id id) const
  {
    if (const ParamDef *d = findParam(id))
    {
      // core.p is the single source of truth; mirror of setParam's mapping
      const hypersaw::Params &p = core.p;
      const std::string k = d->coreKey;
      if (k == "n") return p.n;
      if (k == "dist") return p.dist;
      if (k == "seed") return p.seed;
      if (k == "detune") return p.detune;
      if (k == "law") return p.law;
      if (k == "K") return p.K;
      if (k == "onset") return p.onset;
      if (k == "dissolve") return p.dissolve;
      if (k == "driftDepth") return p.driftDepth;
      if (k == "driftRate") return p.driftRate;
      if (k == "inertia") return p.inertia;
      if (k == "rtone") return p.rtone;
      if (k == "normExp") return p.normExp;
      if (k == "width") return p.width;
      if (k == "mono") return p.mono;
      if (k == "digital") return p.digital;
      if (k == "vol") return p.vol;
      if (k == "retrig") return p.retrig;
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
        core.noteOn(n->key, 440.0 * std::pow(2.0, (n->key - 69) / 12.0));
        break;
      }
      case CLAP_EVENT_NOTE_OFF:
      case CLAP_EVENT_NOTE_CHOKE:
      {
        auto *n = reinterpret_cast<const clap_event_note_t *>(ev);
        if (n->key < 0) core.allOff();
        else core.noteOff(n->key);
        break;
      }
      case CLAP_EVENT_PARAM_VALUE:
      {
        auto *pv = reinterpret_cast<const clap_event_param_value_t *>(ev);
        applyParam(pv->param_id, pv->value);
        break;
      }
      default:
        break;
    }
  }

  clap_process_status process(const clap_process_t *p)
  {
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

    if (core.focus()) return CLAP_PROCESS_CONTINUE;
    return CLAP_PROCESS_SLEEP;
  }
};

Plugin *self(const clap_plugin_t *p) { return static_cast<Plugin *>(p->plugin_data); }

/* ---- lifecycle ---- */

bool plug_init(const clap_plugin_t *) { return true; }
void plug_destroy(const clap_plugin_t *p) { delete self(p); }

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
bool plug_start_processing(const clap_plugin_t *) { return true; }
void plug_stop_processing(const clap_plugin_t *) {}
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
    const int idx = (int)std::round(value);
    const int span = (int)(d->maxV - d->minV);
    if (idx >= 0 && idx <= span) std::snprintf(out, cap, "%s", d->labels[idx]);
    else std::snprintf(out, cap, "%d", idx);
  }
  else if (d->stepped)
  {
    std::snprintf(out, cap, "%d", (int)std::round(value));
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
                  const clap_output_events_t *)
{
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
    pl->core.setParam(line.substr(0, eq), std::atof(line.c_str() + eq + 1));
  }
  return true;
}

const clap_plugin_state_t s_state = {state_save, state_load};

const void *plug_get_extension(const clap_plugin_t *, const char *id)
{
  if (!std::strcmp(id, CLAP_EXT_AUDIO_PORTS)) return &s_audio_ports;
  if (!std::strcmp(id, CLAP_EXT_NOTE_PORTS)) return &s_note_ports;
  if (!std::strcmp(id, CLAP_EXT_PARAMS)) return &s_params;
  if (!std::strcmp(id, CLAP_EXT_STATE)) return &s_state;
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
