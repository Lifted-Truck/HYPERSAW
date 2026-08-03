# Regenerates the build stamp header on EVERY build (not just configure).
# Configure-time stamps go stale exactly when they matter — the human is
# looking at a plugin and asking "is this the code I just merged?".
execute_process(COMMAND git -C ${SRC} rev-parse --short HEAD
                OUTPUT_VARIABLE H OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET RESULT_VARIABLE RC)
if(NOT RC EQUAL 0)
  set(H "nogit")
endif()
execute_process(COMMAND git -C ${SRC} status --porcelain OUTPUT_VARIABLE D ERROR_QUIET)
if(NOT D STREQUAL "")
  set(H "${H}+")
endif()
string(TIMESTAMP T "%Y-%m-%d %H:%M" UTC)
set(BODY "#pragma once\n#define HYPERSAW_BUILD_STAMP \"${H} · ${T}Z\"\n")
if(EXISTS ${OUT})
  file(READ ${OUT} OLD)
else()
  set(OLD "")
endif()
if(NOT OLD STREQUAL BODY)
  file(WRITE ${OUT} "${BODY}")   # only rewrite on change, so we don't force relinks
endif()
