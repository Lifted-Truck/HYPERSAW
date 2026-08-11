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

  /* TEST HOOK — not part of the CLAP surface and not called by the exported
     entry. It lets a headless oracle trigger the same forensic dump the GUI
     panic button triggers, so what is under test is the REAL capture path and
     not a reimplementation of it (a test that rebuilds the mechanism it is
     checking spans the wrong layer — L0031). Returns a pointer valid until the
     next call, or NULL if the dump could not be written. */
  const char *hypersaw_test_dump_forensics(const clap_plugin_t *p, const char *why);
}
