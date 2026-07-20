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

namespace hypersaw
{

constexpr int kRackSlots = 4;

// Placeholder effect types (increment 1). Off is the inert default; the other
// three are chosen so processing order is AUDIBLE immediately (drive→filter ≠
// filter→drive). Real cores replace these as slot types in later increments.
enum class FxType : int
{
  Off = 0,
  Drive = 1,   // dry/wet tanh — amount 0 = passthrough, 1 = tanh(4x)
  Filter = 2,  // one-pole LP  — amount 0 = open (passthrough), 1 = heavy
  Gain = 3,    // level        — amount 0.5 = unity, 0 = silence, 1 = +6 dB
};

class FxRack
{
 public:
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
  Slot slots[kRackSlots];
};

}  // namespace hypersaw
