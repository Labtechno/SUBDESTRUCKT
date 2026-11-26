# SUBDESTRUCKT — ARCH-G4 (developpement integre P0/P1, base vertical slice ARCH-G3).
# Cible produit : VST3 instrument macOS arm64 natif (M3 cible minimale, M4 Max recette). Cœur DSP sans JUCE,
# testable sans hôte ; wrapper JUCE 9.0.6 séparé (voir docs/V1-ARCH-G3-PLAN.md).
cmake_minimum_required(VERSION 3.22)

if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Architecture produit : arm64 uniquement")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "Cible de deploiement declaree (non validee, voir plan §2)")
endif()

project(SUBDESTRUCKT VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SUBDESTRUCKT_BUILD_PLUGIN "Construire le plugin VST3 (necessite JUCE)" ON)
option(SUBDESTRUCKT_BUILD_TESTS "Construire les tests" ON)
set(SUBDESTRUCKT_JUCE_DIR "" CACHE PATH "Chemin du depot JUCE 9.0.6 (commit 654bdbc7)")
set(SUBDESTRUCKT_BUILD_NUMBER "0" CACHE STRING "Numero de build (run CI)")
set(SUBDESTRUCKT_COMMIT "unknown" CACHE STRING "Commit source")

set(SUBDESTRUCKT_BUILD_ID "${PROJECT_VERSION}-b${SUBDESTRUCKT_BUILD_NUMBER}")
configure_file(source/core/BuildInfo.h.in ${CMAKE_CURRENT_BINARY_DIR}/generated/BuildInfo.h @ONLY)

# ---------------------------------------------------------------- cœur (L1/L2/L3), sans JUCE
add_library(subdestruckt_core STATIC
  source/dsp/Wavetable.cpp
  source/dsp/WtBank.cpp
  source/dsp/Decimator.cpp
  source/dsp/Filters.cpp
  source/dsp/Destruction.cpp
  source/core/Parameters.cpp
  source/core/Topology.cpp
  source/core/ModMatrix.cpp
  source/core/StateCodec.cpp
  source/core/PresetOps.cpp
  source/engine/ModRuntime.cpp
  source/engine/Chain.cpp
  source/engine/Engine.cpp)
target_include_directories(subdestruckt_core PUBLIC source ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_compile_features(subdestruckt_core PUBLIC cxx_std_17)
if(MSVC)
  target_compile_options(subdestruckt_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(subdestruckt_core PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wno-sign-conversion)
endif()
set_target_properties(subdestruckt_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(SUBDESTRUCKT_BUILD_TESTS)
  enable_testing()
  # Oracle de non-régression : copie figée du moteur 0.1.0-b5 (namespace sbd_b5), jamais modifiée.
  add_library(subdestruckt_b5_oracle STATIC
    tests/regression/b5/Parameters.cpp
    tests/regression/b5/Topology.cpp
    tests/regression/b5/Decimator.cpp
    tests/regression/b5/Wavetable.cpp
    tests/regression/b5/StateCodec.cpp
    tests/regression/b5/Engine.cpp)
  target_compile_features(subdestruckt_b5_oracle PUBLIC cxx_std_17)
  add_executable(subdestruckt_core_tests
    tests/core/main.cpp
    tests/core/test_dsp.cpp
    tests/core/test_topology.cpp
    tests/core/test_state.cpp
    tests/core/test_engine.cpp
    tests/core/test_g4.cpp
    tests/core/test_g4_features.cpp
    tests/core/test_g4_filters.cpp
    tests/core/test_g4_destruction.cpp
    tests/core/test_g4_state.cpp)
  target_include_directories(subdestruckt_core_tests PRIVATE tests)
  target_link_libraries(subdestruckt_core_tests PRIVATE subdestruckt_core subdestruckt_b5_oracle)
  add_test(NAME core_tests COMMAND subdestruckt_core_tests)
endif()

# ---------------------------------------------------------------- plugin VST3 (L0) + UI de diagnostic
if(SUBDESTRUCKT_BUILD_PLUGIN)
  if(NOT SUBDESTRUCKT_JUCE_DIR)
    message(FATAL_ERROR "SUBDESTRUCKT_JUCE_DIR doit pointer vers JUCE 9.0.6")
  endif()
  add_subdirectory(${SUBDESTRUCKT_JUCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
  include(cmake/Plugin.cmake)
endif()
