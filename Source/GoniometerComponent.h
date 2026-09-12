#pragma once

#include <JuceHeader.h>
#include <vector>

// Stereo phase scope (Lissajous / vector scope), restyled to match the FxSound Mac
// reference: a dot-cloud plot of raw (L, R) sample pairs rotated 45 degrees, a
// reference circle + crosshair, edge axis labels, and a correlation meter.
//
// Pulse has only one signal (no dry/wet), so - unlike the reference, which plots a
// dim "dry" cloud alongside a bright "wet" cloud - this only ever plots one cloud,
// in the accent colour.
class GoniometerComponent final : public juce::Component, private juce::Timer
{
public:
    GoniometerComponent();

    void pushStereo(const float* left, const float* right, int numSamples);
    void setActive(bool shouldBeActive);

    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;
    void clearTrace();

    static float correlation(const std::vector<float>& interleavedLR);

    // Most recent interleaved (L, R) sample pairs - a rolling window REPLACED each
    // pushStereo() call, not accumulated across calls. Matches the reference: the
    // cloud shows "what just happened", not a long historical trail.
    static constexpr int kMaxDots = 2048;
    std::vector<float> recent; // interleaved L, R

    bool active = false;

    juce::CriticalSection dataLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GoniometerComponent)
};
