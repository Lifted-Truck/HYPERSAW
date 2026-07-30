// Over-hold probe at REALISTIC settings (default release, poly), driven like a
// human typing fast: overlapping notes, re-presses, same-frame events, held
// chords under a moving line. The notefuzz gate runs at the 5 ms release floor
// to make hangs unmistakable; this probe instead asks the question the human
// is actually asking — "does anything ring LONGER than the envelope says it
// should?" — with the envelope's own tail as the yardstick.
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>
#include <algorithm>
#include <clap/clap.h>
#include "../src/hypersaw_clap_entry.h"
namespace {
#include "notefuzz_scaffold.inc"
}
int main()
{
  hypersaw_entry_init("");
  auto *factory=(const clap_plugin_factory_t*)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  int bad=0;
  struct M { const char*name; double mono, legato, glide; };
  const M modes[] = {{"poly",0,0,0},{"mono",1,0,0},{"mono+legato",1,1,0},
                     {"mono+legato+glide",1,1,0.08}};
  for (const auto &mode : modes)
  for (uint32_t seed=2000; seed<2015; seed++)
  {
    const clap_plugin_t *p=factory->create_plugin(factory,&kHost,"com.lifted-truck.hypersaw");
    p->init(p); p->activate(p,kSR,32,kBlock); p->start_processing(p);
    std::vector<float> L(kBlock),R(kBlock);
    float*chans[2]={L.data(),R.data()};
    clap_audio_buffer_t out{}; out.data32=chans; out.channel_count=2;
    clap_process_t proc{}; proc.frames_count=kBlock; proc.audio_outputs=&out;
    proc.audio_outputs_count=1; proc.out_events=&kOut;
    auto process=[&](EvList&e){e.finalize();proc.in_events=&e.list;p->process(p,&proc);};
    { EvList e; e.params.push_back(mkParam(32,mode.mono)); e.params.push_back(mkParam(34,mode.legato));
      e.params.push_back(mkParam(33,mode.glide)); process(e); }
    uint32_t rng=seed;
    std::vector<int16_t> held; int32_t id=1;
    // ~4 s of fast overlapping playing: every ~70 ms an on or off, re-press
    // allowed, all events at frame 0 (live typing), chords accumulate to 6.
    for (int b=0; b<Math_blocks(4.0); b++)
    {
      EvList e;
      if (b % Math_blocks(0.07) == 0)
      {
        const bool off = !held.empty() && ((mrand(rng) & 3u) == 0);   // 25% offs: notes overlap
        if (off || held.size() >= 6)
        {
          const size_t k = mrand(rng) % held.size();
          e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF, 0, held[k], -1));
          held.erase(held.begin()+k);
        }
        else
        {
          const int16_t key = (int16_t)(48 + (mrand(rng) % 25));      // dup = re-press
          e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON, 0, key, id++));
          if (std::find(held.begin(),held.end(),key)==held.end()) held.push_back(key);
        }
      }
      process(e);
    }
    { EvList e; uint32_t t=0; for(int16_t k:held) e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF,t++,k,-1)); held.clear(); process(e); }
    // Default release is 0.16 s; a one-pole tail is at -60 dB by ~7 tau = 1.1 s.
    // Measure when the output actually falls below -60 dBFS of its own peak.
    double peak=1e-12; int lastLoud=-1; EvList e0;
    std::vector<double> env;
    for (int b=0; b<Math_blocks(3.0); b++)
    {
      process(e0);
      double m=0; for(int i=0;i<kBlock;i++) m=std::max(m,(double)std::fabs(L[i])+std::fabs(R[i]));
      env.push_back(m); peak=std::max(peak,m);
    }
    for (size_t b=0;b<env.size();b++) if (env[b] > peak*1e-3) lastLoud=(int)b;
    const double tailS = (lastLoud+1)*(double)kBlock/kSR;
    const bool over = tailS > 1.4;   // envelope math says ~1.1 s to -60 dB
    if (over) bad++;
    if (over)
      std::printf("%s seed %u: tail to -60 dB = %.2f s  <-- OVER-HOLD\n", mode.name, seed, tailS);
    p->stop_processing(p); p->deactivate(p); p->destroy(p);
  }
  std::printf(bad ? "tailprobe: %d/60 runs OVER-HOLD\n" : "tailprobe: all 60 runs (4 modes x 15 seeds) within envelope tail\n", bad);
  return bad?1:0;
}
