#pragma once

#include <JuceHeader.h>
#include "PulseTheme.h"
#include "NowPlayingManager.h"

// Small, non-interactive now-playing display for the top-right of Pulse.
class NowPlayingComponent final : public juce::Component, private juce::Timer
{
public:
    NowPlayingComponent();
    ~NowPlayingComponent() override = default;

    void setInfo(const NowPlayingManager::Info& info);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    juce::String title;
    juce::String artist;
    bool playing = false;

    float alpha = 0.0f;
    float targetAlpha = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NowPlayingComponent)
};
