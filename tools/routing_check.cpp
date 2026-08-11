/* routing_check — the B23 crosspoint matrix (ADR-088) before it enters the
 * audio path. Same order the glide and swarmalator ports used: core + oracle
 * first, shell integration onto proven ground second.
 *
 * These are INVARIANT assertions, not parity against the routing lab. Deliberate:
 * the lab's scheme C carries toy effects (one-pole, delay, tanh), so a
 * sample-parity test would mostly measure those and would go red for reasons
 * that are not about routing. Every slot here is a trivial deterministic
 * stand-in, so what is measured is the topology and nothing else — which is
 * also what makes these transferable (L0030/L0031: an oracle that names no
 * internals outlives the implementation it was written against).
 */
#include <cmath>
#include <cstdio>
#include <vector>

#include "../src/routing_core.h"
#include "../src/fx_rack.h"

constexpr double kPi = 3.141592653589793;

static int failures = 0;
static void check(bool ok, const char *what, const char *detail)
{
  std::printf("%-6s %s  (%s)\n", ok ? "OK" : "FAIL", what, detail);
  if (!ok) failures++;
}

using Matrix = hypersaw::RoutingMatrix<2, 4>;

// Deterministic stand-in slots: distinguishable, order-sensitive, no state.
// Slot 1 is NONLINEAR on purpose — a linear chain would make serial and
// parallel indistinguishable, and the test would pass for the wrong reason.
static double slotProc(int slot, double x)
{
  switch (slot)
  {
    case 0: return x * 2.0;
    case 1: return x * x;          // nonlinear: order matters
    case 2: return x + 1.0;
    default: return x * 0.5;
  }
}

int main()
{
  char d[192];

  // ---- 1. the default topology IS a serial chain ---------------------------
  // Computed independently rather than by running the matrix, or the test would
  // be the implementation restated.
  {
    Matrix m;
    m.setSerialChain();
    const double src[2] = {0.3, 0.0};
    const double got = m.process(src, slotProc);
    const double want = slotProc(3, slotProc(2, slotProc(1, slotProc(0, 0.3))));
    std::snprintf(d, sizeof(d), "got %.12g, hand-computed chain %.12g", got, want);
    check(std::fabs(got - want) < 1e-12, "default topology is exactly a serial chain", d);
  }

  // ---- 2. sources sum at a slot -------------------------------------------
  {
    Matrix m;
    m.setSerialChain();
    const double a[2] = {0.3, 0.0}, b[2] = {0.0, 0.4}, both[2] = {0.3, 0.4};
    // slot 0 is linear (x*2), so its input sum is observable through the chain
    // only if the whole chain were linear — it is not. Assert at the slot input
    // instead, via a matrix whose only terminal is slot 0.
    Matrix m0;
    m0.setSerialChain();
    m0.inFrom[1] = 0; m0.outAmount[3] = 0; m0.outAmount[0] = 1.0;
    const double ga = m0.process(a, slotProc), gb = m0.process(b, slotProc),
                 gboth = m0.process(both, slotProc);
    std::snprintf(d, sizeof(d), "f(a)=%.6g f(b)=%.6g f(a+b)=%.6g", ga, gb, gboth);
    check(std::fabs(gboth - (ga + gb)) < 1e-12, "both sources sum into one slot", d);
  }

  // ---- 3. a backwards edge is DROPPED BY THE READER ------------------------
  // Set directly on the model — the route a preset load, morph corner or
  // automation takes. No editor is involved and none can be relied on.
  {
    Matrix clean;
    clean.setSerialChain();
    const double src[2] = {0.3, 0.2};
    const double before = clean.process(src, slotProc);

    Matrix planted = clean;
    planted.inFrom[0] |= (1u << (2 + 3));    // slot 3 -> slot 0, backwards
    planted.coeff[2 + 3][0] = 1.0;
    const double after = planted.process(src, slotProc);

    std::snprintf(d, sizeof(d), "before %.12g, after planting slot3->slot0 %.12g", before, after);
    check(before == after, "an illegal backwards edge changes nothing", d);

    // and the control: a LEGAL edge in the same shape must change the result,
    // or the guard is simply eating everything.
    Matrix legal = clean;
    legal.inFrom[3] |= (1u << (2 + 0));      // slot 0 -> slot 3, forwards
    legal.coeff[2 + 0][3] = 1.0;
    const double legalOut = legal.process(src, slotProc);
    std::snprintf(d, sizeof(d), "legal slot0->slot3 gives %.12g vs %.12g", legalOut, before);
    check(legalOut != before, "a legal edge in the same shape DOES change it", d);
  }

  // ---- 4. terminal detection follows the edges -----------------------------
  {
    Matrix m;
    m.setSerialChain();
    bool ok = m.isTerminal(3) && !m.isTerminal(0) && !m.isTerminal(1) && !m.isTerminal(2);
    // now cut slot 3's input: slot 2 becomes a terminal too
    m.inFrom[3] = 0;
    const bool bothTerminal = m.isTerminal(2) && m.isTerminal(3);
    std::snprintf(d, sizeof(d), "chain: only slot3 terminal = %s; after cutting slot3's input, "
                                "slots 2 and 3 both terminal = %s",
                  ok ? "yes" : "no", bothTerminal ? "yes" : "no");
    check(ok && bothTerminal, "a slot nobody reads is an output", d);
  }

  // ---- 5. serial and parallel genuinely differ ----------------------------
  // If they did not, the matrix would be expressing one topology with two
  // spellings and every assertion above would be vacuous.
  {
    Matrix ser;
    ser.setSerialChain();
    ser.inFrom[2] = 0; ser.inFrom[3] = 0;
    ser.outAmount[0] = 0; ser.outAmount[1] = 1.0; ser.outAmount[3] = 0;   // src->0->1->out

    Matrix par = ser;
    par.inFrom[1] = 0;                                  // slot 1 reads the SOURCE instead
    par.inFrom[1] |= 1u; par.coeff[0][1] = 1.0;
    par.outAmount[0] = 1.0; par.outAmount[1] = 1.0;     // both terminals

    const double src[2] = {0.3, 0.0};
    const double s = ser.process(src, slotProc), p = par.process(src, slotProc);
    std::snprintf(d, sizeof(d), "serial %.6g vs parallel %.6g", s, p);
    check(std::fabs(s - p) > 1e-9, "serial and parallel are not the same topology", d);
  }

  // ---- 6. the initial value is an independent input ------------------------
  {
    Matrix m;
    m.setSerialChain();
    const double src[2] = {0.0, 0.0};
    const double silent = m.process(src, slotProc);
    m.slotInit[0] = 0.25;
    const double offset = m.process(src, slotProc);
    std::snprintf(d, sizeof(d), "silent sources give %.6g; slotInit 0.25 gives %.6g", silent, offset);
    check(silent != offset, "slotInit drives a slot with no source connected", d);
  }

  // ---- 7. the block pass over the REAL rack is bit-exactly the old chain ----
  /* This section deliberately breaks the FX-agnostic rule above, and the reason
     is worth stating: the claim under test is not "routing works", it is "the
     matrix replaced `rack.processStereo` WITHOUT CHANGING A SAMPLE", and that
     claim is about the real rack or it is about nothing.

     It needs its own assertion because the 147 parity goldens CANNOT see this.
     They render SwarmCore directly; the plugin mix stage — bass-mono, the rack,
     master volume — is downstream of everything they cover. Treating a green
     parity run as evidence for a mix-stage refactor would be assuming exactly
     the coverage that does not exist (L0031: parity certifies agreement with
     the reference, over the surface the reference actually spans). */
  {
    hypersaw::RoutingMatrix<1, hypersaw::kRackSlots> m;   // ctor = serial chain
    hypersaw::FxRack direct, viaMatrix;

    // Every slot ACTIVE and distinct — an all-Off rack would make both paths
    // trivially equal by touching no samples, and the test would pass without
    // the chain ever being exercised.
    const int types[hypersaw::kRackSlots] = {1, 2, 4, 5};
    for (int i = 0; i < hypersaw::kRackSlots; i++)
    {
      direct.setType(i, types[i]);      viaMatrix.setType(i, types[i]);
      direct.setAmount(i, 0.4 + 0.1 * i); viaMatrix.setAmount(i, 0.4 + 0.1 * i);
    }

    const int N = 1024;
    std::vector<float> aL(N), aR(N), bL(N), bR(N);
    for (int i = 0; i < N; i++)
    {
      const double t = (double)i / 44100.0;
      // kPi, not M_PI: M_PI is a POSIX extension, not standard C++, and MSVC
      // does not define it without _USE_MATH_DEFINES. The Windows leg caught
      // this; the rest of the tree already uses a local constant for exactly
      // this reason, so match it rather than add a platform define.
      aL[i] = bL[i] = (float)(0.3 * std::sin(2 * kPi * 220 * t));
      aR[i] = bR[i] = (float)(0.3 * std::sin(2 * kPi * 331 * t));
    }

    direct.processStereo(aL.data(), aR.data(), N);

    // Same chunking the shell uses, so the comparison covers the chunk seam and
    // not just one big block — slot state has to survive it.
    float sL[hypersaw::kRackSlots][256], sR[hypersaw::kRackSlots][256];
    float *pL[hypersaw::kRackSlots], *pR[hypersaw::kRackSlots];
    for (int t = 0; t < hypersaw::kRackSlots; t++) { pL[t] = sL[t]; pR[t] = sR[t]; }
    for (int off = 0; off < N; off += 256)
    {
      const int n = N - off < 256 ? N - off : 256;
      const float *srcL[1] = {bL.data() + off};
      const float *srcR[1] = {bR.data() + off};
      m.processBlock(srcL, srcR, pL, pR, bL.data() + off, bR.data() + off, n,
                     [&](int slot, float *L, float *R, int k) {
                       viaMatrix.processSlot(slot, L, R, k);
                     });
    }

    int diff = 0;
    double energy = 0;
    for (int i = 0; i < N; i++)
    {
      if (aL[i] != bL[i] || aR[i] != bR[i]) diff++;
      energy += (double)aL[i] * aL[i] + (double)aR[i] * aR[i];
    }
    std::snprintf(d, sizeof(d), "%d/%d samples differ; reference energy %.4g "
                                "(0 would mean the rack did nothing)", diff, N, energy);
    check(diff == 0 && energy > 1e-3,
          "serial-chain block pass == rack.processStereo, sample for sample", d);

    /* CALIBRATED, and the useful result is the plant that did NOT fire.
       Fires (1023/1024 samples): a gather coefficient off by 1e-6; zeroing the
       output buffer BEFORE the slots gather, which is the aliasing hazard
       processBlock's comment warns about (the shell passes the mix bus as both
       source and destination, so an early zero destroys the input).
       NO-OP: removing the `isTerminal` filter from the terminal sum. It changes
       nothing HERE because setSerialChain leaves outAmount = 0 on every
       non-terminal, so summing them adds zeros. This assertion therefore does
       not cover terminal detection at all — assertion 4 does, and that division
       is recorded rather than left for someone to assume the other way. */
  }

  /* CALIBRATION, recorded because a green suite proves nothing on its own.
     Removing the read-side acyclicity guard (`edgeLive` -> always true) makes
     assertion 3 FAIL, 1 -> 5. So that guard is load-bearing and this catches
     its loss.
     The lab's OTHER bug — a terminal test that skips the legality check — was
     planted too and is a NO-OP here: `isTerminal` loops `t > slot`, so illegal
     destinations are excluded by the loop bound rather than by the check. The
     bug is not expressible against this shape. Recorded rather than counted as
     a second successful calibration, which is what it would look like from the
     outside.
     NB: these plants must be run with the object file deleted. CMake did not
     track `src/routing_core.h` as a dependency of this target, and the first
     calibration pass read a stale binary and reported two identical failures
     that were one failure twice. */
  std::printf("routing_check: %s (%d failures)\n", failures ? "RED" : "GREEN", failures);
  return failures ? 1 : 0;
}
