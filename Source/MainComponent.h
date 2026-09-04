#pragma once

#include <JuceHeader.h>
#include "AudioEngine.h"
#include "SpectrumAnalyzer.h"
#include "VisualizerComponent.h"
#include "GoniometerComponent.h"
#include "LevelMeterComponent.h"

class MainComponent final : public juce::Component, private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void startAudio();
    void stopAudio();

    AudioEngine audio;
    SpectrumAnalyzer analyzer;
    VisualizerComponent visualizer;
    GoniometerComponent goniometer;
    LevelMeterComponent levelMeter;

    juce::Label status;
    std::atomic<bool> audioActive { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
