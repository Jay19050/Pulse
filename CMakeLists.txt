cmake_minimum_required(VERSION 3.22)

project(Pulse VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Keep JUCE's optional examples/extras disabled: Pulse only needs the core framework.
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.1
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(Pulse
    PRODUCT_NAME "Pulse"
    COMPANY_NAME "Pulse"
    BUNDLE_ID "com.pulse.visualizer"
    VERSION "0.1.0"
)

juce_generate_juce_header(Pulse)

target_sources(Pulse
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.h
        Source/MainComponent.cpp
        Source/AudioEngine.h
        Source/AudioEngine.cpp
        Source/SpectrumAnalyzer.h
        Source/SpectrumAnalyzer.cpp
        Source/PulseTheme.h
        Source/VisualizerComponent.h
        Source/VisualizerComponent.cpp
        Source/GoniometerComponent.h
        Source/GoniometerComponent.cpp
        Source/LevelMeterComponent.h
        Source/LevelMeterComponent.cpp
)

target_compile_definitions(Pulse
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(Pulse
    PRIVATE
        juce::juce_audio_devices
        juce::juce_dsp
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
        ole32
        avrt
)
