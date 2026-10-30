#pragma once
#include <JuceHeader.h>

class UndertuneAudioProcessor final : public juce::AudioProcessor
{
public:
    UndertuneAudioProcessor();
    ~UndertuneAudioProcessor() override = default;
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "UNDERTUNE"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.08; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    static constexpr int ringSize = 8192;
    std::array<std::array<float, ringSize>, 2> ring{};
    std::array<int, 2> writePos { 0, 0 };
    std::array<double, 2> readDelay { 1200.0, 1200.0 };
    std::array<double, 2> headPhase { 0.0, 0.5 };
    double sr = 44100.0;
    int detectorCounter = 0;
    float detectedHz = 0.0f;
    float smoothedShift = 1.0f;
    juce::AudioBuffer<float> detectorBuffer;
    int detectorWrite = 0;

    float detectPitch (const float* data, int numSamples) const;
    float targetFrequency (float hz, int key, int scale) const;
    float readInterpolated (int channel, double delay) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UndertuneAudioProcessor)
};
