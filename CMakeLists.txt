# HYPERSAW — CLAP-native, VST3/AUv2 via clap-wrapper (ADR-002).
# Build (this machine — see global notes: Unix Makefiles only, absolute paths):
#   cmake -S . -B build-release -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
#   cmake --build <abs>/build-release -j
# Perf work happens in Release only (Debug is -O0 and meaningless for ceilings).
cmake_minimum_required(VERSION 3.27)
project(HYPERSAW VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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

# clap-first idiom (see libs/clap-wrapper/tests/clap-first-example): a static
# impl lib carrying the factory, wrapped per-format by make_clapfirst_plugins.
add_library(${PROJECT_NAME}-impl STATIC src/hypersaw_clap.cpp)
target_link_libraries(${PROJECT_NAME}-impl PUBLIC clap clap-wrapper-extensions)

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

# ADR-006 spike benchmark (host-side tool, not shipped): bank vs iFFT renderer.
add_executable(renderer_bench tools/renderer_bench.cpp)
target_compile_options(renderer_bench PRIVATE -O3)
if(APPLE)
  target_link_libraries(renderer_bench PRIVATE "-framework Accelerate")
endif()
