const void *host_get_extension(const clap_host_t *, const char *) { return nullptr; }
void host_noop(const clap_host_t *) {}
const clap_host_t kHost = {CLAP_VERSION, nullptr, "notefuzz_check", "", "", "1.0",
                           host_get_extension, host_noop, host_noop, host_noop};

bool oev_try_push(const clap_output_events_t *, const clap_event_header_t *) { return true; }
const clap_output_events_t kOut = {nullptr, oev_try_push};

struct EvList
{
  clap_input_events_t list;
  std::vector<clap_event_note_t> notes;
  std::vector<clap_event_param_value_t> params;
  std::vector<const clap_event_header_t *> order;  // time-sorted view
  void finalize()
  {
    order.clear();
    for (auto &n : notes) order.push_back(&n.header);
    for (auto &p : params) order.push_back(&p.header);
    std::stable_sort(order.begin(), order.end(),
                     [](const clap_event_header_t *a, const clap_event_header_t *b) {
                       return a->time < b->time;
                     });
    list.ctx = this;
    list.size = [](const clap_input_events_t *l) -> uint32_t {
      return (uint32_t)((EvList *)l->ctx)->order.size();
    };
    list.get = [](const clap_input_events_t *l, uint32_t i) -> const clap_event_header_t * {
      return ((EvList *)l->ctx)->order[i];
    };
  }
};

clap_event_note_t mkNote(uint16_t type, uint32_t time, int16_t key, int32_t noteId,
                         double velocity = 1.0)
{
  clap_event_note_t ev{};
  ev.header.size = sizeof(ev);
  ev.header.time = time;
  ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
  ev.header.type = type;
  ev.note_id = noteId;
  ev.port_index = 0;
  ev.channel = 0;
  ev.key = key;
  ev.velocity = velocity;
  return ev;
}

clap_event_param_value_t mkParam(clap_id id, double value)
{
  clap_event_param_value_t ev{};
  ev.header.size = sizeof(ev);
  ev.header.time = 0;
  ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
  ev.header.type = CLAP_EVENT_PARAM_VALUE;
  ev.param_id = id;
  ev.cookie = nullptr;
  ev.note_id = -1;
  ev.port_index = -1;
  ev.channel = -1;
  ev.key = -1;
  ev.value = value;
  return ev;
}

// mulberry32 — deterministic, same family the core uses
uint32_t mrand(uint32_t &s)
{
  s += 0x6D2B79F5u;
  uint32_t t = s;
  t = (t ^ (t >> 15)) * (t | 1u);
  t ^= t + (t ^ (t >> 7)) * (t | 61u);
  return t ^ (t >> 14);
}

constexpr int kBlock = 256;
constexpr double kSR = 44100.0;


static int Math_blocks(double s) { return (int)(s * kSR) / kBlock; }
