#pragma once

#include <JuceHeader.h>
#include "IconButton.h"

// Top application header: Pulse logo + wordmark + tagline on the left,
// live/idle status and the settings button on the right.
//
// Window controls (minimize/maximize/close) are NOT drawn here - Main.cpp
// uses setUsingNativeTitleBar(true), so the OS already draws those above this
// header. Duplicating them would be redundant chrome.
class HeaderBar final : public juce::Component
{
public:
    HeaderBar();

    // Reflects real WASAPI capture status only - never set true without a
    // genuine audio-active signal from MainComponent.
    void setActive(bool isActive);

    // Read-only display text, e.g. "Realtek(R) Audio  |  48000 Hz". Device
    // switching itself isn't implemented yet (see SettingsPanel).
    void setDeviceInfo(const juce::String& text);

    void paint(juce::Graphics&) override;
    void resized() override;

    std::function<void()> onSettingsClicked;

private:
    void drawLogo(juce::Graphics& g, juce::Rectangle<float> area) const;

    bool active = false;
    juce::String deviceInfo;

    IconButton settingsButton { IconButton::Icon::Gear };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};
