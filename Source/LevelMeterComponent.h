#pragma once

#include <JuceHeader.h>

// L/R peak + RMS meter with peak-hold ticks. Fed directly from the same stereo
// blocks as SpectrumAnalyzer/GoniometerComponent - it computes its own peak/RMS
// rather than reusing SpectrumAnalyzer::Snapshot::level, since that value is a
// single mono-summed number and a level meter needs L and R separately.
class LevelMeterComponent final : public juce::Component, private juce::Timer
{
public:
    LevelMeterComponent();

    void pushStereo(const float* left, const float* right, int numSamples);
    void setActive(bool shouldBeActive);

    void paint(juce::Graphics&) override;

private:
    struct ChannelLevels
    {
        float peak = 0.0f;   // instantaneous peak this block, linear 0..1
        float rms = 0.0f;    // instantaneous rms this block, linear 0..1

        float displayedRms = 0.0f;    // smoothed bar height
        float displayedPeak = 0.0f;   // fast-attack / slow-release peak bar
        float peakHold = 0.0f;        // slow-decaying peak-hold tick
    };

    void timerCallback() override;
    void drawChannel(juce::Graphics& g, juce::Rectangle<float> area,
                     const ChannelLevels& levels, const juce::String& label) const;

    static float linearToNormalized(float linear);

    ChannelLevels left, right;
    bool active = false;

    juce::CriticalSection dataLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterComponent)
};
