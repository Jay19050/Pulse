#pragma once

#include "VisualizerMode.h"

// A single smooth filled curve of the live spectrum - no legend, no
// peak-hold/average traces, no hover readout. The clean, modern counterpart
// to Spectrum mode's fuller instrumentation.
class ModeFilledSpectrum final : public VisualizerMode
{
public:
    ModeFilledSpectrum();

    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>&) override;
    juce::String getModeName() const override { return "Filled Spectrum"; }

    void paint(juce::Graphics&) override;

private:
    SpectrumAnalyzer::Snapshot snapshot;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeFilledSpectrum)
};
