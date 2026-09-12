#pragma once

#include "VisualizerMode.h"

// A field of particles tracing the live spectrum shape. Each particle has a
// fixed x-position and a fixed per-particle scatter seed (chosen once, at
// construction, with a seeded juce::Random - not re-rolled every frame), so
// all motion you see is driven by the audio itself:
//   - vertical position follows the smoothed spectrum value at that x
//   - scatter distance around that position scales with the SAME value
//   - dot size/brightness scales with the same value
//   - the whole field drifts sideways at a speed proportional to the
//     overall signal level, so it goes still (not idle-random) with no audio
class ModeParticles final : public VisualizerMode
{
public:
    ModeParticles();

    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>&) override;
    juce::String getModeName() const override { return "Particles"; }

    void paint(juce::Graphics&) override;

private:
    struct Particle
    {
        float xFraction = 0.0f;  // 0..1 across the plot width
        float seed = 0.0f;       // -1..1, fixed scatter direction/weight
    };

    std::vector<Particle> particles;
    SpectrumAnalyzer::Snapshot snapshot;
    float driftPhase = 0.0f;

    static float sampleSpectrum(const std::array<float, SpectrumAnalyzer::spectrumPoints>& values, float t);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeParticles)
};
