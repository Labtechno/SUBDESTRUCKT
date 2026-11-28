# SUBDESTRUCKT — ARCH-UI-04-R2-B1 — Component Lab (projet CMake ISOLÉ).
#
# Ce projet ne fait pas partie de la construction de production : le CMakeLists.txt racine, cmake/Plugin.cmake, ui/,
# plugin/ et source/ ne sont ni modifiés ni étendus. Il réutilise en lecture seule :
#   - JUCE 9.0.6 (même commit épinglé que le plugin) ;
#   - la cible subdestruckt_core et les sources UI 0.4.1, uniquement pour l'outil de comparaison 0.4.1 / R2.
# Aucune cible de distribution : ni plugin, ni application musicale, aucun traitement audio dans le laboratoire.
#
#   cmake -S lab -B build-lab -G Ninja -DCMAKE_BUILD_TYPE=Release -DSUBDESTRUCKT_JUCE_DIR=<JUCE 9.0.6>
#   cmake --build build-lab
cmake_minimum_required(VERSION 3.22)
if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "arm64 uniquement")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "")
endif()
project(SUBDESTRUCKT_COMPONENT_LAB VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SUBDESTRUCKT_JUCE_DIR "" CACHE PATH "Chemin du depot JUCE 9.0.6 (commit 654bdbc7)")
if(NOT SUBDESTRUCKT_JUCE_DIR)
  message(FATAL_ERROR "SUBDESTRUCKT_JUCE_DIR doit pointer vers JUCE 9.0.6")
endif()
get_filename_component(SBD_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

# Cœur de production, inchangé (plugin et tests désactivés dans ce sous-projet uniquement).
set(SUBDESTRUCKT_BUILD_PLUGIN OFF CACHE BOOL "" FORCE)
set(SUBDESTRUCKT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${SBD_ROOT} sbd_root EXCLUDE_FROM_ALL)
add_subdirectory(${SUBDESTRUCKT_JUCE_DIR} JUCE)

set(SBD_LAB_DEFINITIONS JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

# Composants R2 : bibliothèque candidate à l'intégration R2-B2 (dépend de JUCE et des tokens 0.4.1, pas du registre).
set(SBD_R2_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/r2/R2Style.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/r2/R2Nexus.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/r2/R2NeuralCell.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/r2/R2SynapticTrack.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/r2/R2CyberDisplay.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/r2/R2PlasmaChannel.cpp)

# Outil 1 : laboratoire (planches, mesures typographiques PT-02, glyphes PT-05, contrastes, auto-tests de gestes,
# fenêtre interactive). Données de démonstration uniquement (DEMO DATA).
juce_add_console_app(sbd_component_lab PRODUCT_NAME "sbd_component_lab")
target_sources(sbd_component_lab PRIVATE
  ${SBD_R2_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/app/DemoData.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/app/Sheets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/app/Measure.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/app/SelfTest.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/app/Main.cpp)
target_include_directories(sbd_component_lab PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${SBD_ROOT})
target_compile_definitions(sbd_component_lab PRIVATE ${SBD_LAB_DEFINITIONS})
target_link_libraries(sbd_component_lab PRIVATE juce::juce_gui_basics juce::juce_recommended_config_flags)

# Outil 2 : comparatif 0.4.1 / R2 (rend les composants 0.4.1 réels, liés au processeur 0.4.1 non modifié, à côté des
# composants R2). Aucun traitement audio : le processeur est instancié uniquement pour ses paramètres.
juce_add_console_app(sbd_lab_compare PRODUCT_NAME "sbd_lab_compare")
target_sources(sbd_lab_compare PRIVATE
  ${SBD_R2_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/app/DemoData.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/app/Sheets.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/app/Compare.cpp
  ${SBD_ROOT}/plugin/PluginProcessor.cpp
  ${SBD_ROOT}/ui/DiagnosticEditor.cpp
  ${SBD_ROOT}/ui/Components.cpp
  ${SBD_ROOT}/ui/SymbioticLook.cpp
  ${SBD_ROOT}/ui/CurveEditors.cpp
  ${SBD_ROOT}/ui/Monitors.cpp
  ${SBD_ROOT}/ui/PerformanceView.cpp
  ${SBD_ROOT}/ui/SoundDesignView.cpp
  ${SBD_ROOT}/ui/EngineeringView.cpp
  ${SBD_ROOT}/ui/MainEditor.cpp)
target_include_directories(sbd_lab_compare PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${SBD_ROOT})
target_compile_definitions(sbd_lab_compare PRIVATE ${SBD_LAB_DEFINITIONS})
target_link_libraries(sbd_lab_compare PRIVATE subdestruckt_core juce::juce_audio_processors juce::juce_gui_basics
  juce::juce_recommended_config_flags)
