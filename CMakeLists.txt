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
set(GUI_HTML_HEADER ${CMAKE_CURRENT_BINARY_DIR}/generated/gui_html.h)
add_custom_command(
  OUTPUT ${GUI_HTML_HEADER}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/generated
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_file.py
          ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/gui.html ${GUI_HTML_HEADER} kGuiHtml
  DEPENDS src/gui/gui.html tools/embed_file.py
  COMMENT "Embedding gui.html")

# clap-first idiom (see libs/clap-wrapper/tests/clap-first-example): a static
# impl lib carrying the factory, wrapped per-format by make_clapfirst_plugins.
# The webview GUI module (ADR-019) compiles on macOS only for now; elsewhere
# the shell exposes no gui extension and hosts use their generic editor.
add_library(${PROJECT_NAME}-impl STATIC src/hypersaw_clap.cpp)
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
add_executable(state_check tools/state_check.cpp)
target_link_libraries(state_check PRIVATE ${PROJECT_NAME}-impl)
if(NOT MSVC)
  target_compile_options(state_check PRIVATE -O2)
endif()

# Track E0 force-core oracle (host-side tool, not shipped): forcecore vs the
# effect labs' population trajectories + L0-14/15/17/19/20 population criteria.
add_executable(force_check tools/force_check.cpp)
if(NOT MSVC)
  target_compile_options(force_check PRIVATE -O2)
endif()

# ADR-006 spike benchmark (host-side tool, not shipped): bank vs iFFT renderer.
add_executable(renderer_bench tools/renderer_bench.cpp)
if(NOT MSVC)
  target_compile_options(renderer_bench PRIVATE -O3)
endif()
if(APPLE)
  target_link_libraries(renderer_bench PRIVATE "-framework Accelerate")
endif()
