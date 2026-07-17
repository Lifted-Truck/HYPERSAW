/*
 * hypersaw_clap_entry.cpp — exports the C-linkage clap_entry, backed by the
 * impl static library. Kept separate per the clap-first idiom so the wrapper
 * formats (VST3/AUv2) can link the impl without a second exported entry.
 */

#include <clap/clap.h>

#include "hypersaw_clap_entry.h"

extern "C"
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif

  const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
      CLAP_VERSION, hypersaw_entry_init, hypersaw_entry_deinit, hypersaw_entry_get_factory};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}
