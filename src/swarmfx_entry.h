/*
 * swarmfx_entry.h — extern entry methods for the SWARM-FX effect plugin,
 * implemented in swarmfx_clap.cpp, consumed by the exported clap_entry in
 * swarmfx_entry.cpp (clap-first idiom; separate factory from the instrument).
 */
#pragma once

#include <clap/clap.h>

extern "C"
{
  bool swarmfx_entry_init(const char *plugin_path);
  void swarmfx_entry_deinit(void);
  const void *swarmfx_entry_get_factory(const char *factory_id);
}
