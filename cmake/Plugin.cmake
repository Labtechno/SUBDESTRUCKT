# SUBDESTRUCKT — cible VST3 (L0 + UI de diagnostic) et exécutables de test liés à JUCE.
# Identité stable (docs/V1-ARCH-G3-PLAN.md §3) : ne jamais modifier BUNDLE_ID, codes fabricant/plugin ni StableID.

set(SUBDESTRUCKT_PLIST_MERGE
  "<plist><dict><key>SUBDESTRUCKTBuild</key><string>${SUBDESTRUCKT_BUILD_ID}</string><key>SUBDESTRUCKTCommit</key><string>${SUBDESTRUCKT_COMMIT}</string></dict></plist>")

juce_add_plugin(SUBDESTRUCKT
  PRODUCT_NAME "SUBDESTRUCKT"
  COMPANY_NAME "Labtechno"
  COMPANY_COPYRIGHT "Labtechno"
  BUNDLE_ID "com.labtechno.subdestruckt"
  PLUGIN_MANUFACTURER_CODE Lbtn
  PLUGIN_CODE Sbdk
  DESCRIPTION "SUBDESTRUCKT ARCH-G4 (build interne)"
  VERSION ${PROJECT_VERSION}
  BUILD_VERSION ${SUBDESTRUCKT_BUILD_NUMBER}
  PLIST_TO_MERGE "${SUBDESTRUCKT_PLIST_MERGE}"
  FORMATS VST3
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  VST3_CATEGORIES Instrument Synth
  COPY_PLUGIN_AFTER_BUILD FALSE)

set(SUBDESTRUCKT_WRAPPER_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/plugin/PluginProcessor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/DiagnosticEditor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/Components.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/SymbioticLook.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/CurveEditors.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/Monitors.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/PerformanceView.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/SoundDesignView.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/EngineeringView.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/MainEditor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/r2/R2Style.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/r2/R2Painters.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/r2/R2Hud.cpp)

set(SUBDESTRUCKT_JUCE_DEFINITIONS
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_sources(SUBDESTRUCKT PRIVATE ${SUBDESTRUCKT_WRAPPER_SOURCES})
target_include_directories(SUBDESTRUCKT PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(SUBDESTRUCKT PUBLIC ${SUBDESTRUCKT_JUCE_DEFINITIONS})
target_link_libraries(SUBDESTRUCKT
  PRIVATE subdestruckt_core juce::juce_audio_processors juce::juce_gui_basics
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

if(SUBDESTRUCKT_BUILD_TESTS)
  # Tests du wrapper : le processeur est instancié directement (sans client VST3) pour instrumenter processBlock.
  juce_add_console_app(subdestruckt_wrapper_tests PRODUCT_NAME "subdestruckt_wrapper_tests")
  target_sources(subdestruckt_wrapper_tests PRIVATE tests/wrapper/WrapperTests.cpp tests/wrapper/Ui04Tests.cpp tests/wrapper/TaR1Tests.cpp tests/wrapper/GestureTests.cpp tests/wrapper/R2B2Tests.cpp tests/wrapper/NexusTests.cpp ${SUBDESTRUCKT_WRAPPER_SOURCES})
  target_include_directories(subdestruckt_wrapper_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} tests)
  target_compile_definitions(subdestruckt_wrapper_tests PRIVATE ${SUBDESTRUCKT_JUCE_DEFINITIONS})
  target_link_libraries(subdestruckt_wrapper_tests PRIVATE subdestruckt_core juce::juce_audio_processors juce::juce_gui_basics
    juce::juce_recommended_config_flags)

  # ARCH-UI-04-R2-B2 : banc de performances graphiques GC-01 (non exécuté par la CI de production ; workflow r2b2-evidence.yml).
  juce_add_console_app(subdestruckt_gc01_bench PRODUCT_NAME "subdestruckt_gc01_bench")
  target_sources(subdestruckt_gc01_bench PRIVATE tests/perf/Gc01Bench.cpp ${SUBDESTRUCKT_WRAPPER_SOURCES})
  target_include_directories(subdestruckt_gc01_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} tests)
  target_compile_definitions(subdestruckt_gc01_bench PRIVATE ${SUBDESTRUCKT_JUCE_DEFINITIONS})
  target_link_libraries(subdestruckt_gc01_bench PRIVATE subdestruckt_core juce::juce_audio_processors juce::juce_gui_basics
    juce::juce_recommended_config_flags)

  # Tests d'hôte : chargement du .vst3 compilé par l'hébergement VST3 de JUCE (sans interface graphique).
  juce_add_console_app(subdestruckt_host_tests PRODUCT_NAME "subdestruckt_host_tests")
  target_sources(subdestruckt_host_tests PRIVATE tests/host/HostTests.cpp)
  target_include_directories(subdestruckt_host_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} tests)
  target_compile_definitions(subdestruckt_host_tests PRIVATE JUCE_PLUGINHOST_VST3=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
  target_link_libraries(subdestruckt_host_tests PRIVATE subdestruckt_core juce::juce_audio_processors_headless
    juce::juce_recommended_config_flags)
endif()
