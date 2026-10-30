#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float minHz = 70.0f;
    constexpr float maxHz = 1000.0f;
    constexpr float twoPi = juce::MathConstants<float>::twoPi;
}

UndertuneAudioProcessor::UndertuneAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "UNDERTUNE_STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout UndertuneAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    p.add (std::make_unique<juce::AudioParameterChoice> ("MODE", "Mode", juce::StringArray { "FEID", "under" }, 1));
    p.add (std::make_unique<juce::AudioParameterChoice> ("KEY", "Key", juce::StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
    p.add (std::make_unique<juce::AudioParameterChoice> ("SCALE", "Scale", juce::StringArray { "Major", "Minor" }, 0));
    p.add (std::make_unique<juce::AudioParameterBool> ("BYPASS", "Bypass", false));
    return p;
}

void UndertuneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    detectorBuffer.setSize (1, juce::jmax (4096, samplesPerBlock * 4));
    detectorBuffer.clear(); detectorWrite = 0; detectorCounter = 0;
    detectedHz = 0.0f; smoothedShift = 1.0f;
    for (auto& channel : ring) channel.fill (0.0f);
    writePos = { 0, 0 }; readDelay = { sr * 0.028, sr * 0.028 }; headPhase = { 0.0, 0.5 };
}

void UndertuneAudioProcessor::releaseResources() {}

bool UndertuneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
        && out == layouts.getMainInputChannelSet();
}

float UndertuneAudioProcessor::detectPitch (const float* data, int n) const
{
    if (n < 512) return 0.0f;
    const int minLag = juce::jmax (1, (int) (sr / maxHz));
    const int maxLag = juce::jmin (n / 2, (int) (sr / minHz));
    float best = 0.0f; int bestLag = 0;
    // Decimated autocorrelation with mean removal; intentionally lightweight for real-time use.
    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        double corr = 0.0, energy = 1.0e-12;
        for (int i = lag; i < n; i += 2)
        {
            const float a = data[i], b = data[i - lag];
            corr += (double) a * b; energy += (double) a * a;
        }
        const float score = (float) (corr / energy);
        if (score > best) { best = score; bestLag = lag; }
    }
    if (bestLag == 0 || best < 0.25f) return 0.0f;
    return (float) (sr / bestLag);
}

float UndertuneAudioProcessor::targetFrequency (float hz, int key, int scale) const
{
    if (hz < minHz || hz > maxHz) return hz;
    // Semitone classes relative to C. Major: W W H W W W H; Minor: W H W W H W W.
    static constexpr int major[] = { 0, 2, 4, 5, 7, 9, 11 };
    static constexpr int minor[] = { 0, 2, 3, 5, 7, 8, 10 };
    const int* intervals = scale == 0 ? major : minor;
    int midi = (int) std::round (69.0 + 12.0 * std::log2 (hz / 440.0));
    float bestDistance = 1000.0f; int bestMidi = midi;
    for (int candidate = midi - 12; candidate <= midi + 12; ++candidate)
    {
        int pc = (candidate % 12 + 12) % 12;
        int relative = (pc - key + 12) % 12;
        bool allowed = false;
        for (int i = 0; i < 7; ++i) if (relative == intervals[i]) { allowed = true; break; }
        if (allowed)
        {
            float distance = std::abs ((float) candidate - (69.0f + 12.0f * std::log2 (hz / 440.0f)));
            if (distance < bestDistance) { bestDistance = distance; bestMidi = candidate; }
        }
    }
    return (float) (440.0 * std::pow (2.0, (bestMidi - 69) / 12.0));
}

float UndertuneAudioProcessor::readInterpolated (int channel, double delay) const
{
    const double pos = (double) writePos[(size_t) channel] - delay;
    double wrapped = std::fmod (pos, (double) ringSize);
    if (wrapped < 0.0) wrapped += ringSize;
    const int i0 = (int) wrapped;
    const int i1 = (i0 + 1) % ringSize;
    const float frac = (float) (wrapped - i0);
    return ring[(size_t) channel][(size_t) i0] * (1.0f - frac) + ring[(size_t) channel][(size_t) i1] * frac;
}

void UndertuneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels(), n = buffer.getNumSamples();
    if (*parameters.getRawParameterValue ("BYPASS") > 0.5f) return;
    const int mode = (int) *parameters.getRawParameterValue ("MODE");
    const int key = (int) *parameters.getRawParameterValue ("KEY");
    const int scale = (int) *parameters.getRawParameterValue ("SCALE");
    const float correctionStrength = mode == 0 ? 0.72f : 1.0f;
    const float maxShiftSemitones = mode == 0 ? 4.0f : 12.0f;

    // Analyse a mono sum in short windows. The selected target note updates every ~20 ms.
    for (int i = 0; i < n; ++i)
    {
        float mono = 0.0f;
        for (int ch = 0; ch < channels; ++ch) mono += buffer.getReadPointer (ch)[i];
        mono /= (float) juce::jmax (1, channels);
        detectorBuffer.setSample (0, detectorWrite, mono);
        detectorWrite = (detectorWrite + 1) % detectorBuffer.getNumSamples();
        if (++detectorCounter >= (int) (sr * 0.020))
        {
            detectorCounter = 0;
            const int window = juce::jmin (2048, detectorBuffer.getNumSamples());
            juce::HeapBlock<float> temp ((size_t) window);
            for (int j = 0; j < window; ++j)
            {
                int idx = detectorWrite - window + j;
                while (idx < 0) idx += detectorBuffer.getNumSamples();
                temp[(size_t) j] = detectorBuffer.getSample (0, idx);
            }
            const float f = detectPitch (temp.get(), window);
            if (f > 0.0f)
            {
                detectedHz = f;
                const float target = targetFrequency (f, key, scale);
                float semitones = 12.0f * std::log2 (target / f) * correctionStrength;
                semitones = juce::jlimit (-maxShiftSemitones, maxShiftSemitones, semitones);
                const float ratio = std::pow (2.0f, semitones / 12.0f);
                // Smooth mode mimics a 20 ms response; under snaps to the target as quickly as the detector permits.
                smoothedShift = mode == 1 ? ratio : (0.65f * smoothedShift + 0.35f * ratio);
            }
        }

        for (int ch = 0; ch < channels; ++ch)
        {
            const int c = juce::jmin (ch, 1);
            const float input = buffer.getReadPointer (ch)[i];
            ring[(size_t) c][(size_t) writePos[(size_t) c]] = input;
            // Two crossfaded delay heads create a granular pitch shift. Small shifts are cleaner;
            // abrupt note changes intentionally produce the hard robotic character in under mode.
            const double range = sr * 0.018;
            const double rate = std::abs (1.0 - (double) smoothedShift);
            if (rate < 0.0005)
            {
                buffer.getWritePointer (ch)[i] = input;
            }
            else
            {
                const double phaseInc = rate / juce::jmax (1.0, range);
                headPhase[(size_t) c] += phaseInc;
                headPhase[(size_t) c] -= std::floor (headPhase[(size_t) c]);
                const double phaseA = headPhase[(size_t) c];
                const double phaseB = std::fmod (phaseA + 0.5, 1.0);
                const double delayA = sr * 0.010 + phaseA * range;
                const double delayB = sr * 0.010 + phaseB * range;
                const float a = readInterpolated (c, delayA);
                const float b = readInterpolated (c, delayB);
                const float gainA = 0.5f - 0.5f * std::cos (twoPi * (float) phaseA);
                const float gainB = 0.5f - 0.5f * std::cos (twoPi * (float) phaseB);
                const float sum = gainA + gainB;
                buffer.getWritePointer (ch)[i] = sum > 1.0e-5f ? (a * gainA + b * gainB) / sum : input;
            }
            writePos[(size_t) c] = (writePos[(size_t) c] + 1) % ringSize;
        }
    }
}

juce::AudioProcessorEditor* UndertuneAudioProcessor::createEditor() { return new UndertuneAudioProcessorEditor (*this); }
void UndertuneAudioProcessor::getStateInformation (juce::MemoryBlock& dest) { auto state = parameters.copyState(); std::unique_ptr<juce::XmlElement> xml (state.createXml()); copyXmlToBinary (*xml, dest); }
void UndertuneAudioProcessor::setStateInformation (const void* data, int size) { std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size)); if (xml && xml->hasTagName (parameters.state.getType())) parameters.replaceState (juce::ValueTree::fromXml (*xml)); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new UndertuneAudioProcessor(); }
