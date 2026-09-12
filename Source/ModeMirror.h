#pragma once

#include "VisualizerMode.h"

// A symmetrical spectrum: the live trace drawn as bars radiating both up and
// down from a centreline, mirrored top/bottom.
class ModeMirror final : public VisualizerMode
{
public:
    ModeMirror();

    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>&) override;
    juce::String getModeName() const override { return "Mirror"; }

    void paint(juce::Graphics&) override;

private:
    SpectrumAnalyzer::Snapshot snapshot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeMirror)
};
