// NOTE_END re-press invariant: no END may be emitted for a key+channel that is
// still held (gated) in any slot. Captures out-events, plays press/release/
// re-press inside the release tail, and counts violations.
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <clap/clap.h>
#include "../src/hypersaw_clap_entry.h"
namespace {
#include "notefuzz_scaffold.inc"
std::vector<clap_event_note_t> gEnds;
// A REJECTING host: try_push returns false while gReject > 0, exactly as a
// host with a full output buffer does when a knob sweep floods it with param
// events. The plugin must not lose the NOTE_END — it must retry until accepted.
int gReject = 0;
bool cap_push(const clap_output_events_t *, const clap_event_header_t *h){
  if (gReject > 0) { gReject--; return false; }
  if (h->type == CLAP_EVENT_NOTE_END) gEnds.push_back(*(const clap_event_note_t*)h);
  return true;
}
const clap_output_events_t kCap = {nullptr, cap_push};
}
int main(){
  hypersaw_entry_init("");
  auto *f=(const clap_plugin_factory_t*)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  const clap_plugin_t *p=f->create_plugin(f,&kHost,"com.lifted-truck.hypersaw");
  p->init(p); p->activate(p,kSR,32,kBlock); p->start_processing(p);
  std::vector<float> L(kBlock),R(kBlock); float*ch[2]={L.data(),R.data()};
  clap_audio_buffer_t out{}; out.data32=ch; out.channel_count=2;
  clap_process_t pr{}; pr.frames_count=kBlock; pr.audio_outputs=&out;
  pr.audio_outputs_count=1; pr.out_events=&kCap;
  bool held=false; int viol=0, ends=0;
  auto run=[&](EvList&e){e.finalize();pr.in_events=&e.list;
    size_t before=gEnds.size(); p->process(p,&pr);
    for(size_t k=before;k<gEnds.size();k++){ends++; if(held && gEnds[k].key==60) viol++;}
  };
  auto blocks=[&](double s){int n=(int)(s*kSR)/kBlock; for(int b=0;b<n;b++){EvList e;run(e);}};
  // press C, release, re-press INSIDE the tail, hold while old slot dies
  { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,60,-1)); run(e);} held=true;
  blocks(0.10);
  held=false;   // off is at frame 0 of this block: an END within it is PROMPT, not a violation
  { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF,0,60,-1)); run(e);}
  blocks(0.05);   // inside the 0.16 s release tail
  { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,60,-1)); run(e);} held=true;
  blocks(2.0);    // old slot dies while the key is HELD — the trap window
  held=false;
  { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF,0,60,-1)); run(e);}
  blocks(2.0);    // final END must still arrive
  std::printf("NOTE_ENDs total %d, emitted-while-held %d, final END after last release: %s\n",
              ends, viol, ends>0 ? "yes" : "NO (leak)");
  // REJECTION TEST (2026-08-03): the host refuses the next 40 pushes. Every
  // NOTE_END must still arrive once the buffer frees up. Before the fix these
  // were silently destroyed and the host never learned the note ended.
  {
    gEnds.clear();
    const size_t before = gEnds.size();
    { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,64,-1)); run(e); }
    blocks(0.10);
    gReject = 40;                       // host buffer "full"
    { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF,0,64,-1)); run(e); }
    blocks(1.5);                        // plenty of blocks to retry in
    const int got = (int)(gEnds.size() - before);
    std::printf("%s rejected-push retry: %d NOTE_END delivered after 40 refusals\n",
                got > 0 ? "OK  " : "FAIL", got);
    if (got == 0) viol++;
  }
  std::printf(viol? "endprobe: FAIL (END for a held key -> wrapper poison)\n"
                  : ends? "endprobe: PASS\n" : "endprobe: FAIL (no END at all)\n");
  p->stop_processing(p); p->deactivate(p); p->destroy(p);
  return (viol||!ends)?1:0;
}
