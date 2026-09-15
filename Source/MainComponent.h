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
    bool keyPressed(const juce::KeyPress& key) override;

    // Wired up from Main.cpp's DocumentWindow, which is the thing that
    // actually knows how to go fullscreen. MainComponent only decides *when*
    // (F11/Esc/the settings row) - the window decides *how*.
    std::function<void()> onToggleFullscreenRequested;
    std::function<void()> onExitFullscreenRequested;

private:
    void timerCallback() override;
    void startAudio();
    void stopAudio();
    void refreshDeviceList();
    void switchOutputDevice(const juce::String& id);
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

    // Reconnect watchdog: if the capture thread dies mid-session (device
    // unplugged etc), retry at a throttled interval rather than hammering
    // WASAPI every 33ms. See timerCallback().
    int reconnectCooldownTicks = 0;
    static constexpr int kReconnectIntervalTicks = 60; // ~2s at the 30Hz timer

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
