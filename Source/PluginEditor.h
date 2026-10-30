#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class UndertuneAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit UndertuneAudioProcessorEditor (UndertuneAudioProcessor&);
    ~UndertuneAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    UndertuneAudioProcessor& processor;
    juce::Image marinPanel;
    juce::ComboBox modeBox, keyBox, scaleBox;
    juce::Label title, subtitle, modeLabel, keyLabel, scaleLabel, footer;
    juce::ToggleButton bypass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment, keyAttachment, scaleAttachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UndertuneAudioProcessorEditor)
};
