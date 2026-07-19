/*
 * swarmfx_entry.cpp — exports the C-linkage clap_entry for SWARM-FX, backed by
 * the swarm-fx impl static library (clap-first idiom).
 */
#include <clap/clap.h>

#include "swarmfx_entry.h"

extern "C"
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif

  const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
      CLAP_VERSION, swarmfx_entry_init, swarmfx_entry_deinit, swarmfx_entry_get_factory};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}
