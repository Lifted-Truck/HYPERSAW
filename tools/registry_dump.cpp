/* registry_dump — emit the parameter tables for FOUNDATIONS'
 * `registry_conformance` (F2 Stage 1 increment 2).
 *
 * EMITTER A, deliberately. The brief offered a cheap Emitter B that transcribes
 * `state_save`'s format strings into the dump loop, and flagged its weakness
 * itself: B compares their address scheme against a *transcription* of our
 * serializer, so it cannot catch a case where `state_save` does something the
 * format strings do not say. That is the same defect our own `state_check`
 * ruling names — a corpus that reads state through the accessor under test
 * agrees with itself — so `patch_key` here comes from the BYTES `state_save`
 * actually writes: instantiate the plugin, call the real `state_save` into a
 * memory stream, split each line at the first '=', and zip that key sequence
 * against the same iteration order the serializer uses.
 *
 * If the zip ever misaligns, the run aborts rather than emitting a plausible
 * table — a dump that silently pairs the wrong patch_key with an id would make
 * C4 assert something no one checked.
 *
 * Usage:  registry_dump hypersaw > hypersaw.tsv
 *         registry_dump swarmfx  > swarmfx.tsv
 * Columns: id  key  osc  global  patch_key  min  max  default
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "../src/hypersaw_clap_entry.h"
#include "../src/swarmfx_entry.h"

namespace
{
const clap_host_t kHost = {CLAP_VERSION_INIT, nullptr, "registry_dump", "-", "-", "1.0",
                           [](const clap_host_t *, const char *) -> const void * { return nullptr; },
                           [](const clap_host_t *) {}, [](const clap_host_t *) {},
                           [](const clap_host_t *) {}};

struct OStr
{
  clap_ostream_t s;
  std::string data;
};
int64_t ostrWrite(const clap_ostream_t *s, const void *buf, uint64_t n)
{
  auto *o = (OStr *)s;
  o->data.append((const char *)buf, (size_t)n);
  return (int64_t)n;
}

// Every key `state_save` wrote, in the order it wrote them, header skipped.
std::vector<std::string> savedKeys(const clap_plugin_t *p)
{
  OStr o{};
  o.s.ctx = nullptr;
  o.s.write = ostrWrite;
  auto *st = (const clap_plugin_state_t *)p->get_extension(p, CLAP_EXT_STATE);
  if (!st || !st->save(p, &o.s)) { std::fprintf(stderr, "state_save failed\n"); std::exit(2); }
  std::vector<std::string> keys;
  size_t pos = 0;
  bool first = true;
  while (pos < o.data.size())
  {
    const size_t nl = o.data.find('\n', pos);
    const std::string line = o.data.substr(pos, nl == std::string::npos ? nl : nl - pos);
    pos = (nl == std::string::npos) ? o.data.size() : nl + 1;
    if (first) { first = false; continue; }          // "hypersaw-state N" / "swarmfx-state N"
    if (line.empty()) continue;
    const size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    keys.push_back(line.substr(0, eq));
  }
  return keys;
}

void header()
{
  std::printf("# emitted by tools/registry_dump.cpp (Emitter A: patch_key read from the\n");
  std::printf("# bytes state_save writes, not transcribed from its format strings)\n");
  std::printf("# id\tkey\tosc\tglobal\tpatch_key\tmin\tmax\tdefault\n");
}
}  // namespace

// hypersaw_clap.cpp's table is file-static, so the dump is driven from the
// public param extension rather than from the array — which is also the honest
// source, since it is what a host sees.
static void dumpHypersaw()
{
  auto *f = (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p = f->create_plugin(f, &kHost, "com.lifted-truck.hypersaw");
  p->init(p);
  auto *px = (const clap_plugin_params_t *)p->get_extension(p, CLAP_EXT_PARAMS);
  const uint32_t n = px->count(p);
  const std::vector<std::string> keys = savedKeys(p);
  if (keys.size() != n)
  {
    std::fprintf(stderr, "ABORT: state_save wrote %zu keys, params report %u — the zip would "
                         "pair the wrong patch_key with an id\n", keys.size(), n);
    std::exit(3);
  }
  // TWO INDEPENDENT SOURCES, on purpose. `patch_key` comes from the bytes
  // state_save wrote (behaviour); `key`, ranges and the global flag come from
  // the DECLARATION, supplied on stdin as "id<TAB>key<TAB>global" parsed from
  // kParams. Deriving `key` by stripping the prefix off `patch_key` — the
  // obvious shortcut, and what this file did first — would make C4 compare a
  // reconstruction against a string built from the same source. The brief warns
  // about exactly that for the address column; the same trap has a second door.
  std::vector<std::string> declKey(4096);
  std::vector<int> declGlobal(4096, -1);
  {
    char ln[256];
    while (std::fgets(ln, sizeof(ln), stdin))
    {
      unsigned did = 0; char k[128]; int g = 0;
      if (std::sscanf(ln, "%u\t%127[^\t]\t%d", &did, k, &g) == 3 && did < declKey.size())
      { declKey[did] = k; declGlobal[did] = g; }
    }
  }
  header();
  for (uint32_t i = 0; i < n; i++)
  {
    clap_param_info_t info{};
    px->get_info(p, i, &info);
    const uint32_t osc = info.id / 1000;
    const uint32_t base = info.id % 1000;
    if (base >= declKey.size() || declKey[base].empty())
    { std::fprintf(stderr, "ABORT: no declared key for base id %u\n", base); std::exit(4); }
    std::printf("%u\t%s\t%u\t%d\t%s\t%.17g\t%.17g\t%.17g\n", (unsigned)info.id,
                declKey[base].c_str(), osc, (osc == 0 && declGlobal[base] == 1) ? 1 : 0,
                keys[i].c_str(), info.min_value, info.max_value, info.default_value);
  }
  p->destroy(p);
}

static void dumpSwarmfx()
{
  // No coreKey exists here and state is keyed on the numeric id, so these names
  // are PROPOSED, not frozen (the brief is explicit that a green report does not
  // ratify them). Chosen to match HYPERSAW's existing coreKey for the same
  // concept wherever one exists — convergence is the point, and inventing a
  // second vocabulary for `grav`/`basin`/`driftDepth` would be the mistake.
  static const char *const kProposed[] = {
      "engine", "K", "grav", "basin", "driftDepth", "driftRate", "inertia",
      "center", "spread", "dist", "placement", "bands", "resonance",
      "feedback", "mix", "vol", "width"};
  auto *f = (const clap_plugin_factory_t *)swarmfx_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p = f->create_plugin(f, &kHost, "com.lifted-truck.swarmfx");
  p->init(p);
  auto *px = (const clap_plugin_params_t *)p->get_extension(p, CLAP_EXT_PARAMS);
  const uint32_t n = px->count(p);
  const std::vector<std::string> keys = savedKeys(p);
  if (keys.size() != n || n != sizeof(kProposed) / sizeof(kProposed[0]))
  {
    std::fprintf(stderr, "ABORT: swarmfx saved %zu keys, params %u, proposed names %zu\n",
                 keys.size(), n, sizeof(kProposed) / sizeof(kProposed[0]));
    std::exit(3);
  }
  header();
  for (uint32_t i = 0; i < n; i++)
  {
    clap_param_info_t info{};
    px->get_info(p, i, &info);
    std::printf("%u\t%s\t0\t1\t%s\t%.17g\t%.17g\t%.17g\n", (unsigned)info.id, kProposed[i],
                keys[i].c_str(), info.min_value, info.max_value, info.default_value);
  }
  p->destroy(p);
}

int main(int argc, char **argv)
{
  const std::string which = argc > 1 ? argv[1] : "";
  if (which == "hypersaw") dumpHypersaw();
  else if (which == "swarmfx") dumpSwarmfx();
  else { std::fprintf(stderr, "usage: registry_dump hypersaw|swarmfx\n"); return 64; }
  return 0;
}
