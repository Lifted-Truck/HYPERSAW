/*
 * hypersaw_clap_entry.h — extern entry methods implemented by the impl static
 * library (src/hypersaw_clap.cpp) and consumed by the exported clap_entry in
 * hypersaw_clap_entry.cpp, per the clap-first idiom (see
 * libs/clap-wrapper/tests/clap-first-example).
 */
#pragma once

#include <clap/clap.h>

extern "C"
{
  bool hypersaw_entry_init(const char *plugin_path);
  void hypersaw_entry_deinit(void);
  const void *hypersaw_entry_get_factory(const char *factory_id);
}
