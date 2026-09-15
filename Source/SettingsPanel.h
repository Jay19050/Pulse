#pragma once

#include <JuceHeader.h>
#include <array>
#include <memory>
#include "IconButton.h"
#include "DeviceSelectorControl.h"
#include "AppearanceSlider.h"
#include "AudioEngine.h"
#include "VisualizerSettings.h"

// A slide-out settings drawer.
//
// AUDIO OUTPUT, DISPLAY/Fullscreen, and VISUALIZER APPEARANCE are all real,
// functional controls. Only PERFORMANCE (FPS limit, render quality, the
// goniometer toggle) remains a static "coming soon" stub - those aren't
// implemented yet, and showing a control that doesn't move anything would be
// pretending it's functional.
//
// Layout is computed once, in computeLayout(), and both paint() and
// resized() read from the same stored rectangles - this used to be two
// independently hand-maintained offset calculations that could (and did)
// drift out of sync; now there is exactly one source of truth.
class SettingsPanel final : public juce::Component
{
public:
    SettingsPanel();

    void setDevices(std::vector<AudioEngine::DeviceInfo> devices, const juce::String& selectedId);
    void setSelectedDevice(const juce::String& id);
    void setSampleRate(double sampleRate);

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;

    std::function<void()> onCloseClicked;
    std::function<void(juce::String)> onDeviceSelected;
    std::function<void()> onRefreshDevicesRequested;
    std::function<void()> onToggleFullscreenClicked;

    // Fired whenever any appearance slider moves, with the complete current
    // settings (not just the one that changed) - simplest possible contract
    // for the receiver (MainComponent) to forward straight to
    // VisualizerModeManager::setAppearance().
    std::function<void(VisualizerSettings)> onAppearanceChanged;

private:
    struct Layout
    {
        juce::Rectangle<float> header, rule;
        juce::Rectangle<float> audioOutputLabel, deviceSelector, sampleRateRow;
        juce::Rectangle<float> displayLabel, fullscreenRow;
        juce::Rectangle<float> appearanceLabel;
        std::array<juce::Rectangle<float>, 5> sliders;
        juce::Rectangle<float> performanceLabel, perfRow1, perfRow2, perfRow3;
    };

    Layout computeLayout() const;
    void notifyAppearanceChanged();

    double sampleRate = 0.0;
    bool fullscreenRowHover = false;
    Layout layout;

    IconButton closeButton { IconButton::Icon::Close };
    DeviceSelectorControl deviceSelector;

    VisualizerSettings appearance;
    AppearanceSlider sensitivitySlider   { "Sensitivity",    0.4f, 2.0f, appearance.sensitivity };
    AppearanceSlider smoothingSlider     { "Smoothing",      0.0f, 0.85f, appearance.smoothing };
    AppearanceSlider brightnessSlider    { "Brightness",     0.5f, 1.6f, appearance.brightness };
    AppearanceSlider peakIntensitySlider { "Peak Intensity", 0.0f, 2.0f, appearance.peakIntensity };
    AppearanceSlider backgroundSlider    { "Background",     0.0f, 1.0f, appearance.background };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel)
};
