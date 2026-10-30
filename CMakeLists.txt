cmake_minimum_required(VERSION 3.22)
project(UNDERTUNE VERSION 1.0.0)

# JUCE is fetched at configure time. Requires internet on the build computer.
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(UNDERTUNE_Assets SOURCES Assets/undertune_marin_panel.png)

juce_add_plugin(UNDERTUNE
    COMPANY_NAME "UNDERTUNE Audio"
    BUNDLE_ID "com.undertune.audio"
    PLUGIN_MANUFACTURER_CODE Undr
    PLUGIN_CODE Utrn
    FORMATS VST3
    PRODUCT_NAME "UNDERTUNE"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(UNDERTUNE PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)
target_compile_features(UNDERTUNE PRIVATE cxx_std_17)
target_compile_definitions(UNDERTUNE PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(UNDERTUNE PRIVATE UNDERTUNE_Assets juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)
