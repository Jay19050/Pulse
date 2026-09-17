#pragma once

#include <array>
#include "VisualizerMode.h"

// A symmetrical spectrum: the live trace drawn as bars radiating both up and
// down from a centreline, mirrored top/bottom.
//
// PERSONALITY: symmetrical. A vertical centre axis (in addition to the
// existing horizontal one) makes the composition read as built around a
// cross rather than just "spectrum cut in half", and a thin envelope curve
// traces the bar tops/bottoms so the shape reads as one designed silhouette
// rather than a loose row of bars.
class ModeMirror final : public VisualizerMode
{
public:
    ModeMirror();

    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>&) override;
    juce::String getModeName() const override { return "Mirror"; }

    void paint(juce::Graphics&) override;

private:
    SpectrumAnalyzer::Snapshot snapshot;
    std::array<EnvelopeFollower, SpectrumAnalyzer::spectrumPoints> followers;
    std::array<float, SpectrumAnalyzer::spectrumPoints> displayed {};
    std::vector<float> barValuesBuf; // reused each frame, not reallocated

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeMirror)
};
