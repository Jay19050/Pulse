#pragma once

#include <JuceHeader.h>
#include "IconButton.h"

// A slide-out settings drawer. Per this phase's brief, none of the controls
// shown here are wired to anything real yet - Output device, Sensitivity,
// Smoothing, Brightness, Peak Intensity, FPS, Render Quality, and the
// goniometer toggle are all drawn as static rows with a "Coming soon" tag,
// not draggable sliders or working switches. This is intentional: showing a
// slider that doesn't move anything would be pretending it's functional.
// Wiring each of these up is later, incremental work.
class SettingsPanel final : public juce::Component
{
public:
    SettingsPanel();

    // Display-only - shown in the Audio section.
    void setAudioInfo(const juce::String& deviceName, double sampleRate);

    void paint(juce::Graphics&) override;
    void resized() override;

    std::function<void()> onCloseClicked;

private:
    juce::String deviceName;
    double sampleRate = 0.0;

    IconButton closeButton { IconButton::Icon::Close };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPanel)
};
