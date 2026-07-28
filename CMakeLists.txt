cmake_minimum_required(VERSION 3.22)
project(MasterLintVST3 VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)

# JUCE: use a local checkout if given, otherwise fetch from GitHub
if(DEFINED JUCE_SOURCE_DIR AND EXISTS ${JUCE_SOURCE_DIR}/CMakeLists.txt)
    add_subdirectory(${JUCE_SOURCE_DIR} juce-build)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.8
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(MasterLint
    COMPANY_NAME "Fancy-Studio"
    PLUGIN_MANUFACTURER_CODE Fncy
    PLUGIN_CODE Mlnt
    FORMATS VST3
    PRODUCT_NAME "MasterLint"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Analyzer Fx)

target_sources(MasterLint PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp)

target_compile_definitions(MasterLint PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(MasterLint PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

juce_add_console_app(dsp_test PRODUCT_NAME "dsp_test")
target_sources(dsp_test PRIVATE tests/dsp_test.cpp)
target_compile_definitions(dsp_test PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(dsp_test PRIVATE juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags)
