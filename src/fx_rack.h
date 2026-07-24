/*
 * fx_rack.h — post-oscillator internal FX rack (ADR-054, increment 1).
 *
 * A fixed series of kRackSlots slots processed IN ORDER (slot 0 → 1 → …), each
 * slot running one user-selectable effect in place on the stereo bus. Order is
 * the slot index, so "reordering" = reassigning effect types to slots — the
 * simplest thing that makes FX order audible (the human's #1 requirement). A
 * true parallel/matrix grid and the real engine cores (filter/notch/time +
 * saturation) come in later increments; this increment is the routing/param/UX
 * skeleton with TRIVIAL placeholder effects to get the feel right.
 *
 * Parity contract (ADR-054): the default slot type is Off, and an all-Off rack
 * MUST be a bit-exact passthrough — processStereo touches no sample — so every
 * existing instrument golden stays green. That is why Off is `continue`, not a
 * gain of 1.0 (a multiply would still be exact, but not touching the buffer is
 * the unambiguous guarantee).
 *
 * Real-time rules (charter): allocation-free after construction, no locks, no
 * wall-clock. All state is preallocated per slot; setType/setAmount are plain
 * stores. Placeholder DSP is per-sample arithmetic only.
 */
#pragma once

#include <cmath>
#include <cstring>
#include <vector>

namespace hypersaw
{

constexpr int kRackSlots = 4;

// Effect types. Off is the inert default; Drive/Filter/Gain are the increment-1
// placeholders (kept — order is audible with them); Comp and Comb (ADR-071) are
// the first REAL cores, transcribed from the detune-lab prototype.
enum class FxType : int
{
  Off = 0,
  Drive = 1,   // dry/wet tanh — amount 0 = passthrough, 1 = tanh(4x)
  Filter = 2,  // one-pole LP  — amount 0 = open (passthrough), 1 = heavy
  Gain = 3,    // level        — amount 0.5 = unity, 0 = silence, 1 = +6 dB
  Comp = 4,    // lab comp+limiter: amount = strength; 0.98 brickwall always on
  Comb = 5,    // polyphonic per-note Karplus-Strong comb: amount = wet mix
};

// Comb bank size: one tuned line per sounding note, stolen oldest-first past 8.
constexpr int kCombLines = 8;

class FxRack
{
 public:
  // Construction is main-thread: allocate at the default rate immediately so a
  // shell that never calls setSampleRate still has valid comb buffers (an empty
  // buffer would make `% len` divide by zero in process).
  FxRack() { setSampleRate(44100); }

  // Called from activate (main thread — allocation is allowed there, never in
  // process). Sizes each comb line to the lowest musical note (~20 Hz) at sr,
  // and derives the comp coefficients from their SECONDS constants (ADR-009 —
  // the lab's 0.3 / 0.0015 were per-sample at 44.1 kHz; expressed as time
  // constants they are sr-independent: atk 63.58 us, rel 15.11 ms).
  void setSampleRate(double sampleRate)
  {
    sr = sampleRate;
    const int len = (int)std::ceil(sr / 20.0) + 4;
    for (auto &c : combs)
    {
      c.bufL.assign(len, 0.0f);
      c.bufR.assign(len, 0.0f);
      c.key = -1;
      c.w = 0;
      c.lpL = c.lpR = 0;
      c.dly = 100;
    }
    compAtk = 1.0 - std::exp(-1.0 / (6.3576e-5 * sr));
    compRel = 1.0 - std::exp(-1.0 / (1.5106e-2 * sr));
    compEnv = 0;
  }

  // Note feed for note-context slots (ADR-071 comb). Called from the shell's
  // note handlers on the audio thread — bounded work, no allocation (the
  // buffers exist since setSampleRate; clearing one line is a fixed memset).
  void noteOn(int key, double freq)
  {
    Comb *line = nullptr;
    for (auto &c : combs)
      if (c.key == key) { line = &c; break; }                 // retrigger same key
    if (!line)
      for (auto &c : combs)
        if (c.key < 0) { line = &c; break; }                  // free line
    if (!line)
    {
      line = &combs[0];                                       // steal oldest
      for (auto &c : combs)
        if (c.age < line->age) line = &c;
    }
    const int len = (int)line->bufL.size();
    line->dly = std::max(2, std::min(len - 1, (int)std::lround(sr / std::max(20.0, freq))));
    std::memset(line->bufL.data(), 0, sizeof(float) * len);
    std::memset(line->bufR.data(), 0, sizeof(float) * len);
    line->w = 0;
    line->lpL = line->lpR = 0;
    line->key = key;
    line->age = ageCounter++;
  }
  void noteOff(int key)
  {
    // The line keeps ringing (natural KS decay) and becomes steal-preferred by
    // age; no state change needed beyond forgetting the key binding on steal.
    (void)key;
  }

  void setType(int slot, int type)
  {
    if (slot < 0 || slot >= kRackSlots) return;
    slots[slot].type = (FxType)type;
  }
  void setAmount(int slot, double amount)
  {
    if (slot < 0 || slot >= kRackSlots) return;
    slots[slot].amount = amount < 0 ? 0 : (amount > 1 ? 1 : amount);
  }

  // Readback for state_save / host get_value (the shell keys these by id).
  int getType(int slot) const
  {
    return (slot < 0 || slot >= kRackSlots) ? 0 : (int)slots[slot].type;
  }
  double getAmount(int slot) const
  {
    return (slot < 0 || slot >= kRackSlots) ? 0 : slots[slot].amount;
  }

  // Reset filter memory (e.g. on transport discontinuity). Parity-neutral.
  void reset()
  {
    for (auto &s : slots) { s.zL = 0; s.zR = 0; }
  }

  // Process the stereo bus in place, slot 0 → kRackSlots-1. All-Off is a
  // no-op (bit-exact passthrough — the parity gate).
  void processStereo(float *L, float *R, int n)
  {
    for (auto &s : slots)
    {
      switch (s.type)
      {
        case FxType::Off:
          break;  // bit-exact passthrough — never touch the buffer
        case FxType::Drive:
        {
          const double w = s.amount;                 // dry/wet
          const double pre = 1.0 + s.amount * 3.0;    // up to 4x into tanh
          for (int i = 0; i < n; i++)
          {
            L[i] = (float)((1 - w) * L[i] + w * std::tanh(L[i] * pre));
            R[i] = (float)((1 - w) * R[i] + w * std::tanh(R[i] * pre));
          }
          break;
        }
        case FxType::Filter:
        {
          // one-pole LP: coef 1 = passthrough, → 0 = heavy. amount 0 → open.
          const double coef = 1.0 - s.amount * 0.99;
          for (int i = 0; i < n; i++)
          {
            s.zL += coef * (L[i] - s.zL);
            s.zR += coef * (R[i] - s.zR);
            L[i] = (float)s.zL;
            R[i] = (float)s.zR;
          }
          break;
        }
        case FxType::Gain:
        {
          const double g = s.amount * 2.0;  // 0.5 → unity
          for (int i = 0; i < n; i++) { L[i] = (float)(L[i] * g); R[i] = (float)(R[i] * g); }
          break;
        }
        case FxType::Comp:
        {
          // Lab comp + limiter as ONE dynamics slot (ADR-071): amount = comp
          // strength (lab's `comp` knob); the 0.98 brickwall is always engaged
          // while the slot is active — "optional" is the slot being Off. Peak
          // follower with fast attack / slow release (seconds-derived coeffs,
          // set in setSampleRate); soft knee above 0.4, ratio 1 + 4*amount.
          const double amt = s.amount;
          for (int i = 0; i < n; i++)
          {
            double l = L[i], r = R[i];
            const double a = std::max(std::fabs(l), std::fabs(r));
            compEnv += (a > compEnv ? compAtk : compRel) * (a - compEnv);
            if (amt > 0.005 && compEnv > 0.4)
            {
              const double gr = (0.4 + (compEnv - 0.4) / (1 + amt * 4)) / compEnv;
              l *= gr;
              r *= gr;
            }
            const double pk = std::max(std::fabs(l), std::fabs(r));
            if (pk > 0.98) { const double lg = 0.98 / pk; l *= lg; r *= lg; }
            L[i] = (float)l;
            R[i] = (float)r;
          }
          break;
        }
        case FxType::Comb:
        {
          // Polyphonic per-note Karplus-Strong comb (ADR-071): one tuned
          // feedback line per sounding note (fed by the shell's note events),
          // each resonating its own pitch out of the shared bus — the lab's
          // per-voice comb re-hosted bus-side, sympathetic-string style.
          // amount = wet mix; resonance fixed at the lab default (fb = 0.79,
          // damp 0.5) until the rack grows per-slot param pages.
          const double mix = s.amount;
          if (mix <= 0.001) break;
          int act = 0;
          for (auto &c : combs)
            if (c.key >= 0) act++;
          if (!act) break;
          const double norm = 1.0 / act, fb = 0.79, damp = 0.5;
          for (int i = 0; i < n; i++)
          {
            const double l = L[i], r = R[i];
            double wl = 0, wr = 0;
            for (auto &c : combs)
            {
              if (c.key < 0) continue;
              const int len = (int)c.bufL.size();
              const int rd = (c.w - c.dly + len) % len;
              double dl = c.bufL[rd], dr = c.bufR[rd];
              c.lpL += (1 - damp) * (dl - c.lpL);
              dl = c.lpL;
              c.lpR += (1 - damp) * (dr - c.lpR);
              dr = c.lpR;
              c.bufL[c.w] = (float)(l + fb * dl);
              c.bufR[c.w] = (float)(r + fb * dr);
              c.w = (c.w + 1) % len;
              wl += dl;
              wr += dr;
            }
            L[i] = (float)(l * (1 - mix) + wl * norm * mix);
            R[i] = (float)(r * (1 - mix) + wr * norm * mix);
          }
          break;
        }
      }
    }
  }

 private:
  struct Slot
  {
    FxType type = FxType::Off;
    double amount = 0.5;
    double zL = 0, zR = 0;  // one-pole filter memory (Filter type)
  };
  struct Comb
  {
    std::vector<float> bufL, bufR;  // sized at setSampleRate (main thread)
    int key = -1, dly = 100, w = 0;
    long age = -1;
    double lpL = 0, lpR = 0;
  };
  Slot slots[kRackSlots];
  Comb combs[kCombLines];
  double sr = 44100;
  // Comp state — coefficients derived from SECONDS constants in setSampleRate
  // (ADR-009); these defaults are the 44.1 kHz values so a shell that never
  // calls setSampleRate still behaves.
  double compEnv = 0, compAtk = 0.3, compRel = 0.0015;
  long ageCounter = 0;
};

}  // namespace hypersaw
