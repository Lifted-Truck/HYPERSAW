# HYPERSAW — CLAP-native, VST3/AUv2 via clap-wrapper (ADR-002).
# Build (this machine — see global notes: Unix Makefiles only, absolute paths):
#   cmake -S . -B build-release -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
#   cmake --build <abs>/build-release -j
# Perf work happens in Release only (Debug is -O0 and meaningless for ceilings).
cmake_minimum_required(VERSION 3.27)
project(HYPERSAW VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Windows: static MSVC runtime project-wide. clap-wrapper sets this for its own
# directory scope only, so without the root-level setting our targets get /MD
# and its libs get /MT — LNK2038 at link. Static runtime is also what you ship
# in a plugin (no VC redist dependency).
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if(APPLE)
  enable_language(OBJC OBJCXX)
  # Apple Silicon + Intel; hosts are arm64-first but Live still ships x86_64 users.
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
endif()

# Pinned SDKs live as submodules (libs/clap @ 1.2.10, libs/clap-wrapper @ v0.15.1).
# The VST3 SDK is CPM-fetched at configure time by clap-wrapper (v3.8.0 pinned there).
set(CLAP_WRAPPER_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "" FORCE)
add_subdirectory(libs/clap EXCLUDE_FROM_ALL)
add_subdirectory(libs/clap-wrapper EXCLUDE_FROM_ALL)

# GUI HTML is embedded at build time (self-contained bundle, no file deps).
# find_package resolves the interpreter portably — bare `python3` is absent
# from Windows CI runners' PATH (same trap family as L0003).
find_package(Python3 COMPONENTS Interpreter REQUIRED)
# HYPERSAW_GUI2: embed the greenfield multi-oscillator interface
# (src/gui/gui2.html) instead of the original single-column layout. OFF by
# default — the new interface is built up cluster by cluster on its own branch
# (human direction 2026-08-07) and swaps in only when it reaches parity.
option(HYPERSAW_GUI2 "Embed the experimental gui2.html interface" OFF)
if(HYPERSAW_GUI2)
  set(GUI_HTML_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/gui2.html)
else()
  set(GUI_HTML_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/gui.html)
endif()
set(GUI_HTML_HEADER ${CMAKE_CURRENT_BINARY_DIR}/generated/gui_html.h)
add_custom_command(
  OUTPUT ${GUI_HTML_HEADER}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_file.py
          ${GUI_HTML_SOURCE} ${GUI_HTML_HEADER} kGuiHtml
  DEPENDS ${GUI_HTML_SOURCE} tools/embed_file.py
  COMMENT "Embedding GUI: ${GUI_HTML_SOURCE}")

# clap-first idiom (see libs/clap-wrapper/tests/clap-first-example): a static
# impl lib carrying the factory, wrapped per-format by make_clapfirst_plugins.
# The webview GUI module (ADR-019) compiles on macOS only for now; elsewhere
# the shell exposes no gui extension and hosts use their generic editor.
# BUILD ID stamped into the GUI corner. "Is the plugin cached?" cost real
# debugging time twice (2026-07-30), and a hand-typed version can lie; this is
# read from git at CONFIGURE time and re-read whenever CMake reruns. Dirty tree
# gets a "+" so a local edit is never mistaken for the committed hash.
execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} rev-parse --short HEAD
                OUTPUT_VARIABLE HS_GIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET RESULT_VARIABLE HS_GIT_RC)
if(NOT HS_GIT_RC EQUAL 0)
  set(HS_GIT_HASH "nogit")
endif()
execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} status --porcelain
                OUTPUT_VARIABLE HS_GIT_DIRTY ERROR_QUIET)
if(NOT HS_GIT_DIRTY STREQUAL "")
  set(HS_GIT_HASH "${HS_GIT_HASH}+")
endif()

# Build stamp regenerated EVERY build (see tools/build_stamp.cmake for why a
# configure-time stamp is not good enough).
set(HS_STAMP_H ${CMAKE_CURRENT_BINARY_DIR}/generated/build_stamp.h)
add_custom_target(hypersaw_build_stamp ALL
  COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_CURRENT_SOURCE_DIR} -DOUT=${HS_STAMP_H}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/tools/build_stamp.cmake
  BYPRODUCTS ${HS_STAMP_H}
  COMMENT "stamping build id + time")

add_library(${PROJECT_NAME}-impl STATIC src/hypersaw_clap.cpp)
target_compile_definitions(${PROJECT_NAME}-impl PRIVATE HYPERSAW_BUILD_ID="${HS_GIT_HASH}")
add_dependencies(${PROJECT_NAME}-impl hypersaw_build_stamp)
target_include_directories(${PROJECT_NAME}-impl PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(${PROJECT_NAME}-impl PUBLIC clap clap-wrapper-extensions)
if(APPLE)
  target_sources(${PROJECT_NAME}-impl PRIVATE src/gui/hypersaw_gui.mm ${GUI_HTML_HEADER})
  target_include_directories(${PROJECT_NAME}-impl PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
  target_link_libraries(${PROJECT_NAME}-impl PUBLIC "-framework WebKit" "-framework Cocoa")
elseif(WIN32)
  # WebView2 via choc (COM declarations embedded — no external SDK).
  # CI-compile-verified; runtime validation is a recorded residual.
  target_sources(${PROJECT_NAME}-impl PRIVATE src/gui/hypersaw_gui_win.cpp ${GUI_HTML_HEADER})
  target_include_directories(${PROJECT_NAME}-impl PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
  target_link_libraries(${PROJECT_NAME}-impl PUBLIC ole32 shlwapi version)
endif()

set(HYPERSAW_PLUGIN_FORMATS CLAP VST3 AUV2)

make_clapfirst_plugins(
        TARGET_NAME ${PROJECT_NAME}
        IMPL_TARGET ${PROJECT_NAME}-impl

        OUTPUT_NAME "HYPERSAW"

        ENTRY_SOURCE "src/hypersaw_clap_entry.cpp"

        BUNDLE_IDENTIFIER "com.lifted-truck.hypersaw"
        BUNDLE_VERSION ${PROJECT_VERSION}

        COPY_AFTER_BUILD FALSE

        PLUGIN_FORMATS ${HYPERSAW_PLUGIN_FORMATS}

        ASSET_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/HYPERSAW_assets

        AUV2_MANUFACTURER_NAME "Lifted Truck"
        AUV2_MANUFACTURER_CODE "LfTk"
        AUV2_SUBTYPE_CODE "Hsaw"
        AUV2_INSTRUMENT_TYPE "aumu"
)

# SWARM-FX: the effects product line as a standalone AUDIO-EFFECT plugin
# (Track E1.3). Its own impl/entry/factory — the instrument shell is untouched
# — sharing the parity-proven cores (filter_core.h / notch_core.h) headers-only.
# No webview GUI yet (generic host params); AUv2 type is 'aufx' (effect).
add_library(SWARMFX-impl STATIC src/swarmfx_clap.cpp)
target_link_libraries(SWARMFX-impl PUBLIC clap clap-wrapper-extensions)

make_clapfirst_plugins(
        TARGET_NAME SWARMFX
        IMPL_TARGET SWARMFX-impl
        OUTPUT_NAME "SWARM-FX"
        ENTRY_SOURCE "src/swarmfx_entry.cpp"
        BUNDLE_IDENTIFIER "com.lifted-truck.swarmfx"
        BUNDLE_VERSION ${PROJECT_VERSION}
        COPY_AFTER_BUILD FALSE
        PLUGIN_FORMATS ${HYPERSAW_PLUGIN_FORMATS}
        ASSET_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/SWARMFX_assets
        AUV2_MANUFACTURER_NAME "Lifted Truck"
        AUV2_MANUFACTURER_CODE "LfTk"
        AUV2_SUBTYPE_CODE "Swfx"
        AUV2_INSTRUMENT_TYPE "aufx"
)

# L0-1 parity harness (host-side tool, not shipped): SwarmCore vs JS goldens.
add_executable(parity_check tools/parity_check.cpp)
if(NOT MSVC)
  target_compile_options(parity_check PRIVATE -O2)
endif()

# L0-2..5 + L0-13 trajectory suite (host-side tool, not shipped).
add_executable(trajectory_check tools/trajectory_check.cpp)
if(NOT MSVC)
  target_compile_options(trajectory_check PRIVATE -O2)
endif()

# Session-persistence oracle: drives the impl through the CLAP params/state
# extensions with a stub host (save -> fresh instance -> load -> identical
# values AND identical audio).
# Real-time safety oracle: the audio thread must not allocate (charter rule,
# previously enforced only by review).
# mpe_check: per-note expression must reach EVERY oscillator (2026-08-09).
# Links the plugin impl, like rtsafety_probe — it drives the real CLAP event
# path rather than a core in isolation, because the bug lived in the event
# fan-out, which a core-level test cannot see.
# samplerate_check: behaviour declared in SECONDS must not track the sample
# rate (ADR-009). Core-level, no plugin link.
add_executable(samplerate_check tools/samplerate_check.cpp)
target_include_directories(samplerate_check PRIVATE src)
if(NOT MSVC)
  target_compile_options(samplerate_check PRIVATE -O2)
endif()

# routing_check: the B23 crosspoint matrix (ADR-088) before it enters the audio
# path — core + oracle first, same order the glide and swarmalator ports used.
add_executable(routing_check tools/routing_check.cpp)
target_include_directories(routing_check PRIVATE src)
if(NOT MSVC)
  target_compile_options(routing_check PRIVATE -O2)
endif()

# subdiv_check: rendering must not depend on how a buffer is subdivided
# (ADR-086). Core-level, so no plugin link.
add_executable(subdiv_check tools/subdiv_check.cpp)
target_include_directories(subdiv_check PRIVATE src)
if(NOT MSVC)
  target_compile_options(subdiv_check PRIVATE -O2)
endif()

add_executable(mpe_check tools/mpe_check.cpp)
target_link_libraries(mpe_check PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(mpe_check PRIVATE -O2)
endif()

# mixer_check: per-oscillator mute/solo actually gate the mix (B24, 2026-08-09).
# Not yet in ./verify — that is a protected path and needs a human gate.
# registry_dump: emits our parameter tables for FOUNDATIONS' registry_conformance
# (F2 Stage 1 increment 2). Links BOTH shells — the instrument and swarmfx —
# because the conformance run covers both.
add_executable(registry_dump tools/registry_dump.cpp)
target_link_libraries(registry_dump PRIVATE ${PROJECT_NAME}-impl SWARMFX-impl)
if(NOT MSVC)
  target_compile_options(registry_dump PRIVATE -O2)
endif()

# paramscope_check: a parameter's SCOPE must be what it claims — globals reach
# every oscillator, per-osc params reach only their own (ADR-089 conditions met).
add_executable(paramscope_check tools/paramscope_check.cpp)
target_link_libraries(paramscope_check PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(paramscope_check PRIVATE -O2)
endif()

add_executable(mixer_check tools/mixer_check.cpp)
target_link_libraries(mixer_check PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(mixer_check PRIVATE -O2)
endif()

add_executable(rtsafety_probe tools/rtsafety_probe.cpp)
target_link_libraries(rtsafety_probe PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(rtsafety_probe PRIVATE -O2)
endif()

# Oscillator preset tier (B20): format + slot-agnosticism.
add_executable(preset_check tools/preset_check.cpp)
target_include_directories(preset_check PRIVATE src)
if(NOT MSVC)
  target_compile_options(preset_check PRIVATE -O2)
endif()

add_executable(state_check tools/state_check.cpp)
target_link_libraries(state_check PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(state_check PRIVATE -O2)
endif()

# Note-lifecycle oracle (ADR-038): seeded random note streams through the
# CLAP process path (poly/mono/legato + MIDI vel-0 note-off convention);
# output must decay to silence after all keys are up.
add_executable(waveshape_check tools/waveshape_check.cpp)
if(NOT MSVC)
  target_compile_options(waveshape_check PRIVATE -O2)
endif()

add_executable(endprobe tools/endprobe.cpp)
target_link_libraries(endprobe PRIVATE ${PROJECT_NAME}-impl)

add_executable(tailprobe tools/tailprobe.cpp)
target_link_libraries(tailprobe PRIVATE ${PROJECT_NAME}-impl)

add_executable(paramfunc_smoke tools/paramfunc_smoke.cpp)
target_link_libraries(paramfunc_smoke PRIVATE ${PROJECT_NAME}-impl)

add_executable(paramleak_probe tools/paramleak_probe.cpp)
target_link_libraries(paramleak_probe PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(paramleak_probe PRIVATE -O2)
endif()

# kstuck_probe: the 2026-08-12 field report — a note that fails to die at, or
# after, high coupling. Every note-lifecycle oracle we have runs at ONE value of
# K, so the coupling axis is untested for lifecycle in an instrument whose whole
# thesis is the coupling.
add_executable(kstuck_probe tools/kstuck_probe.cpp)
target_link_libraries(kstuck_probe PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(kstuck_probe PRIVATE -O2)
endif()

# steal_check: WHICH voice dies when the pool is full. notefuzz proves no voice
# HANGS and proves it whether the victim is oldest, newest or random — steal
# priority was entirely unpinned until this (FOUNDATIONS seam question 1).
add_executable(steal_check tools/steal_check.cpp)
target_link_libraries(steal_check PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(steal_check PRIVATE -O2)
endif()

add_executable(notefuzz_check tools/notefuzz_check.cpp)
target_link_libraries(notefuzz_check PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(notefuzz_check PRIVATE -O2)
endif()

# trace_check: the forensic note trace (FOUNDATIONS ask (c)). Links the impl so
# it can trigger the REAL dump path through the test hook rather than rebuilding
# the ring in the test — a check that reimplements its own subject spans the
# wrong layer (L0031).
add_executable(trace_check tools/trace_check.cpp)
target_link_libraries(trace_check PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(trace_check PRIVATE -O2)
endif()

# Track E0 force-core oracle (host-side tool, not shipped): forcecore vs the
# effect labs' population trajectories + L0-14/15/17/19/20 population criteria.
add_executable(force_check tools/force_check.cpp)
if(NOT MSVC)
  target_compile_options(force_check PRIVATE -O2)
endif()

# Phase 4 SPECTRA oracle (host-side tool, not shipped): SpectraCore parity vs
# the JS reference + L0-6/7 behavioral checks.
add_executable(spectra_check tools/spectra_check.cpp)
if(NOT MSVC)
  target_compile_options(spectra_check PRIVATE -O2)
endif()

# Track E1 resonator-bank oracle (host-side tool, not shipped): FilterCore
# audio parity vs the JS FilterLab + L0-14 collapse->Q.
add_executable(filter_check tools/filter_check.cpp)
if(NOT MSVC)
  target_compile_options(filter_check PRIVATE -O2)
endif()

# Track E1.2 notch-swarm oracle (host-side tool, not shipped): NotchCore audio
# parity vs the JS PhaserLab + L0-16 notch exactness.
add_executable(notch_check tools/notch_check.cpp)
if(NOT MSVC)
  target_compile_options(notch_check PRIVATE -O2)
endif()

# Swarmalator engine oracle (host-side tool, not shipped): SwarmalatorCore
# stereo parity vs the JS reference + SPEC-SWARMALATOR §5 acceptance anchors.
add_executable(swarmalator_check tools/swarmalator_check.cpp)
if(NOT MSVC)
  target_compile_options(swarmalator_check PRIVATE -O2)
endif()

# Track E2 time-engine oracle (host-side tool, not shipped): TimeCore audio
# parity vs the JS TimeLab (echo + room) + L0-19/20/21.
add_executable(time_check tools/time_check.cpp)
if(NOT MSVC)
  target_compile_options(time_check PRIVATE -O2)
endif()

# ADR-006 spike benchmark (host-side tool, not shipped): bank vs iFFT renderer.
# CPU bench against the E-6 envelope — runs the real SwarmCore so a min-spec
# number can replace ADR-082's derate ESTIMATE.
# Travel-law trajectory oracle (A1 fold): parity vs bend-lab's Inertia.
add_executable(glide_check tools/glide_check.cpp)
target_include_directories(glide_check PRIVATE src)
if(NOT MSVC)
  target_compile_options(glide_check PRIVATE -O2)
endif()

add_executable(cpu_bench tools/cpu_bench.cpp)
target_include_directories(cpu_bench PRIVATE src)
if(NOT MSVC)
  target_compile_options(cpu_bench PRIVATE -O3)
endif()

add_executable(renderer_bench tools/renderer_bench.cpp)
if(NOT MSVC)
  target_compile_options(renderer_bench PRIVATE -O3)
endif()
if(APPLE)
  target_link_libraries(renderer_bench PRIVATE "-framework Accelerate")
endif()
