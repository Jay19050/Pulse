#pragma once

#include <JuceHeader.h>
#include "PulseTheme.h"
#include "NowPlayingManager.h"
#include "TrackAnalysis.h"

class TrackDetailsComponent final : public juce::Component, private juce::Timer
{
public:
    TrackDetailsComponent();
    ~TrackDetailsComponent() override = default;

    void setNowPlaying(const NowPlayingManager::Info& info);
    void setAnalysis(const TrackAnalysis::Snapshot& snapshot);

    void paint(juce::Graphics&) override;
    void resized() override;
    bool hitTest(int x, int y) override;
    void mouseUp(const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void toggleExpanded();
    void drawMetric(juce::Graphics&, juce::Rectangle<float>, const juce::String&, const juce::String&, float) const;

    juce::String title;
    juce::String artist;
    TrackAnalysis::Snapshot analysis;
    bool metadataValid = false;
    bool expanded = false;
    float visibility = 0.0f;
    float targetVisibility = 0.0f;
    float expansion = 0.0f;
    float targetExpansion = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackDetailsComponent)
};
