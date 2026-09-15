#pragma once

#include <array>
#include "VisualizerMode.h"

// A 360-degree radial view of the same log-frequency spectrum: one ray per
// spectrum point, low frequencies starting at 12 o'clock and running
// clockwise, magnitude mapped to ray length outward from a centre disc.
class ModeCircular final : public VisualizerMode
{
public:
    ModeCircular();

    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>&) override;
    juce::String getModeName() const override { return "Circular"; }

    void paint(juce::Graphics&) override;

private:
    SpectrumAnalyzer::Snapshot snapshot;
    std::array<EnvelopeFollower, SpectrumAnalyzer::spectrumPoints> followers;
    std::array<float, SpectrumAnalyzer::spectrumPoints> displayed {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeCircular)
};
