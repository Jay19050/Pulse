#pragma once

#include "VisualizerMode.h"

// Raw mono waveform, drawn as a continuously scrolling oscilloscope trace -
// the only mode that uses the raw sample window rather than spectrum data.
class ModeWaveform final : public VisualizerMode
{
public:
    ModeWaveform();

    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>& waveform) override;
    juce::String getModeName() const override { return "Waveform"; }

    void paint(juce::Graphics&) override;

private:
    std::vector<float> samples;
    bool active = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeWaveform)
};
