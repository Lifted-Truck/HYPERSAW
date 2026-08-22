// DIAGNOSTIC: drive the REAL plugin through the CLAP factory like a DAW —
// param events then overlapping MIDI notes — and measure rendered pitch by
// autocorrelation. Runs every scenario at 44.1k AND 48k: every existing glide
// oracle is core-level and 44.1k-only, and the human's DAW is neither.
#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>
#include <clap/clap.h>
#include "../src/hypersaw_clap_entry.h"
namespace {
#include "notefuzz_scaffold.inc"
// The scaffold has no MIDI events — no tool in the repo sends CLAP_EVENT_MIDI,
// which is exactly how a dead wheel shipped. A local list that carries one.
struct MidiEvList
{
  clap_input_events_t list;
  std::vector<clap_event_midi_t> midi;
  std::vector<clap_event_note_t> notes;
  void finalize()
  {
    list.ctx = this;
    list.size = [](const clap_input_events_t *l) -> uint32_t {
      auto*e=(MidiEvList*)l->ctx; return (uint32_t)(e->midi.size()+e->notes.size());
    };
    list.get = [](const clap_input_events_t *l, uint32_t i) -> const clap_event_header_t * {
      auto*e=(MidiEvList*)l->ctx;
      if(i<e->notes.size()) return &e->notes[i].header;
      return &e->midi[i-e->notes.size()].header;
    };
  }
};
clap_event_midi_t mkWheel(int v14)
{
  clap_event_midi_t ev{};
  ev.header.size=sizeof(ev); ev.header.time=0;
  ev.header.space_id=CLAP_CORE_EVENT_SPACE_ID; ev.header.type=CLAP_EVENT_MIDI;
  ev.port_index=0;
  ev.data[0]=0xE0; ev.data[1]=(uint8_t)(v14&0x7F); ev.data[2]=(uint8_t)((v14>>7)&0x7F);
  return ev;
}
}
static double pitchOf(const std::vector<float>&w, double sr)
{
  size_t best=0; double bestv=-1;
  for(size_t lag=(size_t)(sr/500); lag<=(size_t)(sr/100); lag++)
  {
    double s=0; for(size_t i=0;i+lag<w.size();i++) s+=w[i]*w[i+lag];
    if(s>bestv){bestv=s;best=lag;}
  }
  return best? sr/(double)best : 0;
}
int main()
{
  hypersaw_entry_init("");
  auto *factory=(const clap_plugin_factory_t*)hypersaw_entry_get_factory(CLAP_PLUGIN_FACTORY_ID);
  struct Sc { const char*name; std::vector<std::pair<clap_id,double>> params; };
  const Sc scenarios[] = {
    {"mono+legato, lag glide=0.5s",   {{32,1},{34,1},{33,0.5},{137,0},{138,3}}},
    {"mono+legato, const-rate 12",    {{32,1},{34,1},{137,0},{138,2},{140,12}}},
    // legato OFF: the re-strike path. It restores the pre-strike pitch only
    // when `glide > 0`, so a law carrying its own time re-inits AT the target
    // and travels zero distance — the human's "mono on, legato off doesn't
    // apply glide on the retrigger".
    {"mono, legato OFF, const-rate 12", {{32,1},{34,0},{137,0},{138,2},{140,12}}},
    {"mono, legato OFF, lag 300ms",     {{32,1},{34,0},{33,0.3},{137,0},{138,3}}},
    // FOLLOW must take bendTau (109), not glide (33). If the render site
    // overrides tau from `glide` regardless, this reads as instant.
    {"mono+legato, FOLLOW bend lag 400ms", {{32,1},{34,1},{137,1},{106,3},{109,400}}},
    {"mono+legato, FOLLOW bend const-rate 12", {{32,1},{34,1},{137,1},{106,2},{108,12}}},
  };
  for (double sr : {44100.0, 48000.0})
  {
    std::printf("==== %d Hz ====\n", (int)sr);
    for (const auto&sc : scenarios)
    {
      const clap_plugin_t *p=factory->create_plugin(factory,&kHost,"com.lifted-truck.hypersaw");
      p->init(p); p->activate(p,sr,32,kBlock); p->start_processing(p);
      std::vector<float> L(kBlock),R(kBlock);
      float*chans[2]={L.data(),R.data()};
      clap_audio_buffer_t out{}; out.data32=chans; out.channel_count=2;
      clap_process_t proc{}; proc.frames_count=kBlock; proc.audio_outputs=&out;
      proc.audio_outputs_count=1; proc.out_events=&kOut;
      auto process=[&](EvList&e){e.finalize();proc.in_events=&e.list;p->process(p,&proc);};
      auto blocks=[&](double s){ return (int)(s*sr)/kBlock; };
      { EvList e; e.params.push_back(mkParam(1,1)); e.params.push_back(mkParam(9,0));
        for(auto&pv:sc.params) e.params.push_back(mkParam(pv.first,pv.second)); process(e); }
      { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,57,1)); process(e); }
      for(int b=1;b<blocks(0.5);b++){ EvList e; process(e); }
      std::vector<float> tape;
      { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,69,2)); process(e);
        tape.insert(tape.end(),L.begin(),L.end()); }
      for(int b=1;b<blocks(2.0);b++){ EvList e; process(e);
        tape.insert(tape.end(),L.begin(),L.end()); }
      std::printf("%-42s ", sc.name);
      const size_t win=(size_t)(0.1*sr);
      for(size_t off=0; off+win<=tape.size() && off<(size_t)(1.6*sr); off+=win)
      {
        std::vector<float> w(tape.begin()+off, tape.begin()+off+win);
        std::printf("%4.0f ", pitchOf(w,sr));
      }
      std::printf("\n");
      p->stop_processing(p); p->deactivate(p); p->destroy(p);
    }
    {
      const clap_plugin_t *p=factory->create_plugin(factory,&kHost,"com.lifted-truck.hypersaw");
      p->init(p); p->activate(p,sr,32,kBlock); p->start_processing(p);
      std::vector<float> L(kBlock),R(kBlock);
      float*chans[2]={L.data(),R.data()};
      clap_audio_buffer_t out{}; out.data32=chans; out.channel_count=2;
      clap_process_t proc{}; proc.frames_count=kBlock; proc.audio_outputs=&out;
      proc.audio_outputs_count=1; proc.out_events=&kOut;
      auto blocks=[&](double s){ return (int)(s*sr)/kBlock; };
      { EvList e; e.params.push_back(mkParam(1,1)); e.params.push_back(mkParam(9,0));
        e.params.push_back(mkParam(106,1)); e.params.push_back(mkParam(107,800));
        e.finalize(); proc.in_events=&e.list; p->process(p,&proc); }
      { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,57,1));
        e.finalize(); proc.in_events=&e.list; p->process(p,&proc); }
      for(int b=1;b<blocks(0.3);b++){ EvList e; e.finalize(); proc.in_events=&e.list; p->process(p,&proc); }
      std::vector<float> tape;
      { MidiEvList e; e.midi.push_back(mkWheel(16383));
        e.finalize(); proc.in_events=&e.list; p->process(p,&proc);
        tape.insert(tape.end(),L.begin(),L.end()); }
      for(int b=1;b<blocks(1.6);b++){ EvList e; e.finalize(); proc.in_events=&e.list; p->process(p,&proc);
        tape.insert(tape.end(),L.begin(),L.end()); }
      std::printf("%-42s ", "WHEEL ch0 -> +2st, const-time 800ms");
      const size_t win=(size_t)(0.1*sr);
      for(size_t off=0; off+win<=tape.size() && off<(size_t)(1.4*sr); off+=win)
      {
        std::vector<float> w(tape.begin()+off, tape.begin()+off+win);
        std::printf("%4.0f ", pitchOf(w,sr));
      }
      std::printf("  (want 220 ramping to ~247)\n");
      p->stop_processing(p); p->deactivate(p); p->destroy(p);
    }
    /* ADR-111 — drag vs anchored, asserted on RENDERED pitch. The claim is a
       polyphonic one no core-level oracle can see (L0031): with Bend Quantise
       = scale, striking an OUT-OF-SCALE note emits that note's correction into
       the GLOBAL pitchBend, transposing every held voice. Value 2 (drag) keeps
       that; value 3 (anchored) admits the anchor's own class so the rest
       correction is exactly zero.
       Gesture: hold C4 · strike F#4 (F# ties F/G at distance 1; the tie breaks
       toward the previous emitted step 60, so F wins and the correction is -1)
       · release F#4 (5 ms release) · measure the held C4 alone.
       The drag case doubles as the must-discriminate control: if the gesture or
       the measurement cannot show the shift, drag fails FIRST, and the anchored
       "stays put" result is known to be meaningless. */
    int bad111 = 0;
    for (int mode = 2; mode <= 3; mode++)
    {
      const clap_plugin_t *p=factory->create_plugin(factory,&kHost,"com.lifted-truck.hypersaw");
      p->init(p); p->activate(p,sr,32,kBlock); p->start_processing(p);
      std::vector<float> L(kBlock),R(kBlock);
      float*chans[2]={L.data(),R.data()};
      clap_audio_buffer_t out{}; out.data32=chans; out.channel_count=2;
      clap_process_t proc{}; proc.frames_count=kBlock; proc.audio_outputs=&out;
      proc.audio_outputs_count=1; proc.out_events=&kOut;
      auto blocks=[&](double sec){ return (int)(sec*sr)/kBlock; };
      auto run=[&](EvList&e){ e.finalize(); proc.in_events=&e.list; p->process(p,&proc); };
      { EvList e; e.params.push_back(mkParam(1,1)); e.params.push_back(mkParam(9,0));
        e.params.push_back(mkParam(114,(double)mode));
        e.params.push_back(mkParam(22,0.005));           // release: kill the F# tail
        run(e); }
      { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,60,1)); run(e); }
      for(int b=1;b<blocks(0.3);b++){ EvList e; run(e); }
      { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_ON,0,66,2)); run(e); }
      for(int b=1;b<blocks(0.4);b++){ EvList e; run(e); }
      { EvList e; e.notes.push_back(mkNote(CLAP_EVENT_NOTE_OFF,0,66,2)); run(e); }
      std::vector<float> tape;
      for(int b=0;b<blocks(0.4);b++){ EvList e; run(e);
        tape.insert(tape.end(),L.begin(),L.end()); }
      std::vector<float> w(tape.end()-(size_t)(0.2*sr), tape.end());
      const double hz = pitchOf(w,sr);
      const double want = mode==2 ? 246.94 : 261.63;    // held C4: dragged to B3 / held at C4
      const bool ok = std::fabs(hz-want) < 5.0;
      std::printf("ADR-111 %-22s held C4 -> %6.1f Hz (want %6.1f)  %s\n",
                  mode==2?"scale (drag), F#4 hit:":"scale (anchor), F#4 hit:",
                  hz, want, ok?"OK":"FAIL");
      if(!ok) bad111++;
      p->stop_processing(p); p->deactivate(p); p->destroy(p);
    }
    if (bad111) return 1;
  }
  return 0;
}
