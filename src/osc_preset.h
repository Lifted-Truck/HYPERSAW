/*
 * osc_preset.h — one oscillator's parameters as a slot-agnostic preset (B20,
 * the bottom of the three preset tiers).
 *
 * This tier is almost free, and that is the interesting part: ADR-082 gave
 * every per-oscillator parameter the state key `o<k>.name`, so ONE
 * OSCILLATOR'S PRESET IS JUST THE SUBSET OF STATE KEYS CARRYING ONE PREFIX.
 * Saving is a filter; loading into another slot is a prefix rewrite; "copy
 * oscillator 1 to 2" needs no new format at all. That fell out of the id
 * scheme rather than being designed for presets — some evidence the scheme is
 * the right shape.
 *
 * The logic lives here, free of the plugin object, for one reason: inside the
 * plugin's anonymous namespace it would be unreachable from an oracle, and
 * untested shipped code is the thing this project is organised against. The
 * caller supplies read/write callbacks; the format and the filtering are all
 * that is tested, because they are all there is.
 *
 * Format: `hypersaw-osc 1` then UNPREFIXED `key=value` lines. Unprefixed is the
 * whole point — a preset saved from oscillator 2 must load into oscillator 1
 * without editing. Global parameters are excluded: an oscillator preset that
 * carried the FX rack or the master image would silently redecorate whatever
 * patch you dropped it into.
 */
#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>

namespace hypersaw
{
namespace oscpreset
{

inline const char *header() { return "hypersaw-osc 1\n"; }

// ForEachParam: void(cb) where cb is void(const char *key, bool isGlobal)
// ReadFn:       double(const char *key)
template <class ForEachParam, class ReadFn>
std::string save(ForEachParam forEach, ReadFn read)
{
  std::string blob = header();
  char line[96];
  forEach([&](const char *key, bool isGlobal) {
    if (isGlobal) return;                      // globals belong to the patch
    std::snprintf(line, sizeof(line), "%s=%.17g\n", key, read(key));
    blob += line;
  });
  return blob;
}

// WriteFn: bool(const char *key, double v) — returns false for a key it will
// not accept (unknown, or global), so `applied` counts only real writes.
// Returns the number applied, or -1 if the blob is not an oscillator preset.
// Counting rather than bool: "loaded nothing" and "loaded values that happened
// to match" are different failures and a caller should be able to tell them
// apart.
template <class WriteFn>
int load(const std::string &blob, WriteFn write)
{
  if (blob.rfind(header(), 0) != 0) return -1;
  size_t pos = blob.find('\n');
  if (pos == std::string::npos) return -1;
  pos++;
  int applied = 0;
  while (pos < blob.size())
  {
    const size_t eol = blob.find('\n', pos);
    const std::string ln =
        blob.substr(pos, eol == std::string::npos ? std::string::npos : eol - pos);
    pos = eol == std::string::npos ? blob.size() : eol + 1;
    const size_t eq = ln.find('=');
    if (eq == std::string::npos) continue;     // blank or malformed: skip, do not fail
    const std::string key = ln.substr(0, eq);
    const double val = std::atof(ln.c_str() + eq + 1);
    if (write(key.c_str(), val)) applied++;
  }
  return applied;
}

}  // namespace oscpreset
}  // namespace hypersaw
