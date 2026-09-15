#pragma once

#include <JuceHeader.h>
#include "AudioEngine.h"

// A custom-drawn "which Windows output device am I listening to" control:
// a pill button showing the current device name, which opens a small
// juce::CallOutBox list of choices when clicked, plus a "Refresh Devices"
// affordance underneath. Deliberately not a juce::ComboBox - that renders
// with the platform's native dropdown chrome, which is exactly the
// "generic Windows control" look the UI brief asked to avoid.
//
// This control only presents devices and reports choices/refresh requests -
// it doesn't own the AudioEngine, doesn't decide when to enumerate, and
// doesn't itself restart audio. That's MainComponent's job.
class DeviceSelectorControl final : public juce::Component
{
public:
    DeviceSelectorControl();

    // Full refresh: replaces the device list and the current selection.
    void setDevices(std::vector<AudioEngine::DeviceInfo> newDevices, const juce::String& selectedId);

    // Cheap re-highlight after a switch, without re-enumerating devices.
    void setSelectedId(const juce::String& id);

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;

    std::function<void(juce::String)> onDeviceSelected;
    std::function<void()> onRefreshRequested;

private:
    void showDeviceMenu();
    juce::String currentDisplayName() const;

    std::vector<AudioEngine::DeviceInfo> devices;
    juce::String selectedId;

    juce::Rectangle<float> pillBounds, refreshBounds;
    bool pillHover = false, refreshHover = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceSelectorControl)
};
