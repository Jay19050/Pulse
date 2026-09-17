#pragma once

#include <array>
#include "VisualizerMode.h"

// A layered filled spectrum - no legend, no hover readout, the clean, modern
// counterpart to Spectrum mode's fuller instrumentation.
//
// PERSONALITY: powerful. Three layers give it mass and depth rather than a
// single flat fill: a faint, heavily-smoothed BACK layer (the broad shape of
// the sound), the main filled MIDDLE body, and a bright, lightly-smoothed
// FRONT trace riding the top edge for crisp transient detail.
class ModeFilledSpectrum final : public VisualizerMode
{
public:
    ModeFilledSpectrum();

    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>&) override;
    juce::String getModeName() const override { return "Filled Spectrum"; }

    void paint(juce::Graphics&) override;

private:
    SpectrumAnalyzer::Snapshot snapshot;

    std::array<EnvelopeFollower, SpectrumAnalyzer::spectrumPoints> backFollowers;
    std::array<EnvelopeFollower, SpectrumAnalyzer::spectrumPoints> midFollowers;
    std::array<EnvelopeFollower, SpectrumAnalyzer::spectrumPoints> frontFollowers;

    std::array<float, SpectrumAnalyzer::spectrumPoints> backLayer {};
    std::array<float, SpectrumAnalyzer::spectrumPoints> midLayer {};
    std::array<float, SpectrumAnalyzer::spectrumPoints> frontLayer {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeFilledSpectrum)
};
