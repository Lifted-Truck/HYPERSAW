/*
 * ratchet_probe — SCRATCH DIAGNOSTIC, not a gate. (B47, second report)
 *
 * Trigger: human 2026-08-26, "it happens more the longer I play, and moreso
 * when I morph partway between fairly complex patches. I wonder if there's
 * some sort of memory leak."
 *
 * Three hypotheses, one instrument, all against the REAL plugin:
 *   RATCHET  — render cost per simulated second, in buckets, under a seeded
 *              continuous note stream. A leak/accumulation shows as a rising
 *              staircase; tail pile-up shows as a plateau above the quiet
 *              baseline that RECOVERS after silence. The recovery bucket is
 *              the discriminator between "leak" and "tails".
 *   MORPH    — same stream with the real morph engine on, pinned PARTWAY
 *              between two corners authored through the shipped arm path
 *              (param 159 routes writes into the armed corner's baseline,
 *              hypersaw_clap.cpp:1427). Mode 0 + temperature makes partway a
 *              stochastic patchwork: stepped params (n, dist) can flip corner
 *              every 5.8 ms grid tick, and n/dist/width writes call rebuild().
 *              Corner-pinned (x=y=0) is the control: same engine, no churn.
 *   MEMORY   — RSS sampled every bucket. A literal leak rises monotonically;
 *              tails and churn do not move RSS at all.
 *
 * MUST-FIRE control for the meter itself: the quiet-baseline bucket must be
 * far below the playing buckets, or the timer is measuring something else.
 */
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <mach/mach.h>
#include <clap/clap.h>

#include "../src/hypersaw_clap_entry.h"

namespace
{
#include "notefuzz_scaffold.inc"

double rssMB()
{
  mach_task_basic_info info{};
  mach_msg_type_number_t cnt = MACH_TASK_BASIC_INFO_COUNT;
  if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &cnt) != KERN_SUCCESS)
    return -1;
  return (double)info.resident_size / (1024.0 * 1024.0);
}

struct Probe
{
  const clap_plugin_t *p = nullptr;
  std::vector<float> L, R;
  clap_audio_buffer_t out{};
  clap_process_t proc{};
  float *ch[2];

  void boot()
  {
    auto *f = (const clap_plugin_factory_t *)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
    p = f->create_plugin(f, &kHost, "com.lifted-truck.hypersaw");
    p->init(p);
    p->activate(p, kSR, 32, kBlock);
    p->start_processing(p);
    L.assign(kBlock, 0); R.assign(kBlock, 0);
    ch[0] = L.data(); ch[1] = R.data();
    out.data32 = ch; out.channel_count = 2;
    proc.frames_count = kBlock; proc.audio_outputs = &out;
    proc.audio_outputs_count = 1; proc.out_events = &kOut;
  }
  void step(EvList &e)
  {
    e.finalize(); proc.in_events = &e.list; p->process(p, &proc);
  }
  void setParams(const std::vector<std::pair<clap_id, double>> &kv)
  {
    EvList e; for (auto &x : kv) e.params.push_back(mkParam(x.first, x.second)); step(e);
  }
  double readParam(clap_id id)
  {
    auto *ext = (const clap_plugin_params_t *)p->get_extension(p, CLAP_EXT_PARAMS);
    double v = -1;
    if (ext) ext->get_value(p, id, &v);
    return v;
  }
  void kill() { p->stop_processing(p); p->deactivate(p); p->destroy(p); }
};

/* The two patches the human's report implies: a "fairly complex" corner and a
   plain one. Complexity picked to differ on the REBUILD-triggering stepped
   keys (n, dist, width) and the per-sample-cost keys (round, release). */
const std::vector<std::pair<clap_id, double>> kHeavy = {
  {1, 16}, {2, 2}, {4, 0.6}, {14, 1.2},          // n=16, gauss, wide detune
  {73, 0.8}, {74, 0.5},                          // round + roundHi (per-sample shape path)
  {22, 2.0},                                     // 2 s release: tails live long
  {6, 0.4}, {9, 30}, {10, 0.6},                  // some coupling + drift
};
const std::vector<std::pair<clap_id, double>> kLight = {
  {1, 7}, {2, 1}, {4, 0.28}, {14, 0.8},
  {73, 0.0}, {74, 0.0},
  {22, 0.16},
  {6, 0.0}, {9, 0}, {10, 0.4},
};

/* Seeded 6-notes/s on/off stream. Same seed every phase: identical playing. */
struct Stream
{
  uint32_t rng;
  int nextOnBlk = 0, noteId = 100;
  struct Off { int blk, noteId, key; };
  std::vector<Off> pendingOff;
  explicit Stream(uint32_t seed) : rng(seed) {}
  void tick(int blk, EvList &e)
  {
    if (blk >= nextOnBlk)
    {
      const int key = 40 + (int)(mrand(rng) % 32);
      e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON, 0, (int16_t)key, noteId));
      /* OFF carries the SAME key as its ON. First run of this probe sent key 0
         and every release matched nothing (CLAP wildcard is -1, not 0) -- the
         "plateau" was 16 stuck notes this probe created itself. */
      pendingOff.push_back({blk + (int)((0.3 + (mrand(rng) % 1000) / 833.0) * kSR) / kBlock,
                            noteId, key});
      noteId++;
      nextOnBlk = blk + (int)((0.10 + (mrand(rng) % 1000) / 7700.0) * kSR) / kBlock;
    }
    for (auto it = pendingOff.begin(); it != pendingOff.end();)
      if (it->blk <= blk)
      {
        e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF, 0, (int16_t)it->key, it->noteId));
        it = pendingOff.erase(it);
      }
      else ++it;
  }
  void drain(EvList &e)
  {
    for (auto &o : pendingOff)
      e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF, 0, (int16_t)o.key, o.noteId));
    pendingOff.clear();
  }
};

struct Bucket { double pct, rssMB; int tags, gated; };

/* Render `seconds` of the stream (or silence if stream == null), returning
   per-`bucketS` cost as % of real time plus occupancy from the test hooks. */
std::vector<Bucket> run(Probe &pr, Stream *st, double seconds, double bucketS,
                        Stream *drainFirst = nullptr)
{
  using clk = std::chrono::steady_clock;
  if (drainFirst) { EvList e; drainFirst->drain(e); pr.step(e); }
  std::vector<Bucket> out;
  const int blocksPerBucket = (int)(bucketS * kSR) / kBlock;
  const int nBuckets = (int)(seconds / bucketS);
  int blk = 0;
  for (int b = 0; b < nBuckets; b++)
  {
    auto t0 = clk::now();
    for (int i = 0; i < blocksPerBucket; i++, blk++)
    {
      EvList e;
      if (st) st->tick(blk, e);
      pr.step(e);
    }
    const double wall = std::chrono::duration<double>(clk::now() - t0).count();
    int tags = 0, gated = 0;
    for (int s2 = 0; s2 < hypersaw_test_poly(); s2++)
    {
      if (hypersaw_test_tag_at(pr.p, s2, nullptr, nullptr, nullptr, nullptr)) tags++;
      if (hypersaw_test_slot_gated(pr.p, s2)) gated++;
    }
    out.push_back({100.0 * wall / bucketS, rssMB(), tags, gated});
  }
  return out;
}

void print(const char *label, const std::vector<Bucket> &bs, double bucketS)
{
  std::printf("%s\n", label);
  std::printf("  %6s %8s %6s %6s %9s\n", "t(s)", "cost%", "tags", "gated", "RSS MB");
  for (size_t i = 0; i < bs.size(); i++)
    std::printf("  %6.0f %8.2f %6d %6d %9.1f\n",
                (i + 1) * bucketS, bs[i].pct, bs[i].tags, bs[i].gated, bs[i].rssMB);
}
}  // namespace

int main()
{
  hypersaw_entry_init("");

  /* ---- PHASE 0+1: static heavy patch. Quiet baseline (the meter control),
     then 60 s of playing, then 12 s of silence — the recovery discriminator. */
  {
    Probe pr; pr.boot();
    pr.setParams(kHeavy);
    print("\n== METER CONTROL: heavy patch, SILENCE (must be near zero) ==",
          run(pr, nullptr, 10, 5), 5);
    Stream st(1234);
    print("\n== PHASE 1: static heavy patch, 60 s continuous playing ==",
          run(pr, &st, 60, 5), 5);
    /* drainFirst releases the notes still held at the boundary -- run 2 of
       this probe read 7 voices "stuck" 12 s into silence that were simply
       notes whose OFF this probe never sent. */
    print("\n== PHASE 1b: notes stop (all offs sent); does cost RECOVER? ==",
          run(pr, nullptr, 24, 3, &st), 3);
    pr.kill();
  }

  /* ---- PHASE 1c: the SAME play+silence with the B38 cull at -40 dB. If the
     plateau is tails, both the playing cost and the recovery time halve; if it
     is a leak, the knob changes nothing. This is the discriminator AND the
     remedy in one table. */
  {
    Probe pr; pr.boot();
    pr.setParams(kHeavy);
    pr.setParams({{160, -40}});
    Stream st(1234);
    print("\n== PHASE 1c: same patch + stream, voiceCull -40 dB ==",
          run(pr, &st, 60, 5), 5);
    print("\n== PHASE 1c-silence: recovery at -40 dB ==",
          run(pr, nullptr, 24, 3, &st), 3);
    pr.kill();
  }

  /* ---- PHASE 2: the real morph engine, corners authored via the shipped arm
     path. Partway (stochastic patchwork) vs pinned at corner A (control). */
  {
    Probe pr; pr.boot();
    /* Morph ON FIRST: morphRouteEdit is a no-op while morphOn <= 0.5
       (hypersaw_clap.cpp:1417), so corners authored before enabling the field
       are silently discarded -- the authoring control caught exactly that on
       run 2 of this probe (n read 7.0 where 16 was authored). */
    pr.setParams({{151, 1}, {154, 1.0}, {157, 0}, {158, 0.05}});  // morph ON, temp 1, pick mode
    pr.setParams({{159, 1}}); pr.setParams(kHeavy);   // author corner A = heavy
    pr.setParams({{159, 2}}); pr.setParams(kLight);   // author corner B = light
    pr.setParams({{159, 0}});
    pr.setParams({{152, 0.0}, {153, 0.0}});           // pinned at A
    /* AUTHORING CONTROL: pinned at A the field must land n=16; at B, n=7.
       Without this readback a half-failed authoring makes every Phase 2
       number meaningless (first run: pinned-at-A cost was HALF of Phase 1's
       for what should be the same patch -- that discrepancy is what this
       control exists to catch). Run a second of silence so the 5.8 ms grid
       actually steps the field before reading. */
    { EvList e; for (int i = 0; i < (int)kSR / kBlock; i++) { EvList q; pr.step(q); } (void)e; }
    std::printf("\n== AUTHORING CONTROL ==\n  at A: n reads %.1f (want 16)\n", pr.readParam(1));
    pr.setParams({{152, 1.0}, {153, 0.0}});
    { for (int i = 0; i < (int)kSR / kBlock; i++) { EvList q; pr.step(q); } }
    std::printf("  at B: n reads %.1f (want 7)\n", pr.readParam(1));
    pr.setParams({{152, 0.0}, {153, 0.0}});
    { for (int i = 0; i < (int)kSR / kBlock; i++) { EvList q; pr.step(q); } }
    Stream st(1234);
    print("== PHASE 2a: morph ON pinned at corner A (control), 30 s ==",
          run(pr, &st, 30, 5), 5);
    pr.setParams({{152, 0.5}, {153, 0.0}});           // PARTWAY A<->B
    Stream st2(1234);
    print("\n== PHASE 2b: morph PARTWAY between heavy and light, 30 s ==",
          run(pr, &st2, 30, 5), 5);
    pr.setParams({{152, 0.0}, {153, 0.0}});           // back to A
    Stream st3(1234);
    print("\n== PHASE 2c: back at corner A — does cost return? ==",
          run(pr, &st3, 15, 5), 5);
    pr.kill();
  }

  hypersaw_entry_deinit();
  return 0;
}
