#pragma once

#include <JuceHeader.h>
#include "AudioEngine.h"
#include "SpectrumAnalyzer.h"
#include "VisualizerModeManager.h"
#include "WaveformBuffer.h"
#include "GoniometerComponent.h"
#include "LevelMeterComponent.h"
#include "HeaderBar.h"
#include "ModeSelectorBar.h"
#include "SettingsPanel.h"

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
    void setSettingsPanelOpen(bool open);
    juce::Rectangle<int> settingsPanelBounds() const;

    AudioEngine audio;
    SpectrumAnalyzer analyzer;
    WaveformBuffer waveform;
    VisualizerModeManager visualizer;
    GoniometerComponent goniometer;
    LevelMeterComponent levelMeter;

    HeaderBar header;
    ModeSelectorBar modeSelector { visualizer };
    SettingsPanel settingsPanel;
    bool settingsOpen = false;

    std::atomic<bool> audioActive { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
