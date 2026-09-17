#pragma once

#include "VisualizerMode.h"

// Raw mono waveform, drawn as a continuously scrolling oscilloscope trace -
// the only mode that uses the raw sample window rather than spectrum data.
//
// PERSONALITY: organic. Three layers give it depth without turning it into a
// spectrum mode in disguise: a translucent filled body (the "main"
// waveform), a fainter, more heavily-smoothed ghost trace riding on top of
// it (the "secondary" waveform - always smoother than the main trace,
// analogous to Spectrum mode's AVG layer), and a subtle glow on the main
// stroke.
class ModeWaveform final : public VisualizerMode
{
public:
    ModeWaveform();

    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>& waveform) override;
    juce::String getModeName() const override { return "Waveform"; }

    void paint(juce::Graphics&) override;

private:
    std::vector<float> samples;
    std::vector<EnvelopeFollower> hiFollowers, loFollowers;
    std::vector<EnvelopeFollower> secondaryHiFollowers, secondaryLoFollowers;
    std::vector<juce::Point<float>> hiPointsBuf, loPointsBuf; // reused each frame, not reallocated
    bool active = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeWaveform)
};
