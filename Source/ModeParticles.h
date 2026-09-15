#pragma once

#include "VisualizerMode.h"

// A layered, flowing particle field: dense background dust, flowing "ribbon"
// streams, brighter energy clusters near a central vortex, and fine
// high-frequency sparks - driven by SpectrumAnalyzer's bass/mid/high energy
// bands and overall level.
//
// Deliberately NOT a simulated particle system - there is no per-frame
// position integration or stored velocity/trail-history state. Every
// particle's position is a pure function of (its fixed, index-derived
// parameters, a shared flowPhase that only advances while there's real
// signal, and the current spectrum). That's what makes the no-audio idle
// behaviour trivial and correct: freeze flowPhase, and every particle's
// position freezes with it - there's no separate "stop animating" path that
// could be forgotten or get out of sync with the rest of the field. Trails
// are the same trick applied twice: computePosition() evaluated at a
// slightly earlier phase IS the trail position, with no history buffer.
class ModeParticles final : public VisualizerMode
{
public:
    ModeParticles();

    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>&) override;
    juce::String getModeName() const override { return "Particles"; }

    void paint(juce::Graphics&) override;

private:
    enum class Layer { Dust, Flow, Energy, Spark };

    struct Particle
    {
        float xFraction = 0.0f;   // 0..1 anchor position across the plot width
        float phase = 0.0f;       // per-particle wave phase offset, radians
        float speed = 1.0f;       // per-particle drift-speed multiplier
        float scatterSeed = 0.0f; // -1..1 perpendicular scatter direction/weight
        float sizeSeed = 0.0f;    // 0..1 size/brightness variation
        Layer layer = Layer::Dust;
    };

    static float sampleSpectrum(const std::array<float, SpectrumAnalyzer::spectrumPoints>& values, float t);
    void buildParticles();
    juce::Point<float> computePosition(const Particle& p, juce::Rectangle<float> plot, float phaseOffset) const;

    std::vector<Particle> particles;
    SpectrumAnalyzer::Snapshot snapshot;

    // Shared time basis for every wave/drift/vortex calculation. Only ever
    // incremented while snapshot.active is true, and even then scaled by the
    // current level - see updateData(). At true silence this simply stops
    // changing, which is the entire idle mechanism.
    float flowPhase = 0.0f;

    EnvelopeFollower bassFollower, midFollower, highFollower, levelFollower;
    float bassEnergy = 0.0f, midEnergy = 0.0f, highEnergy = 0.0f, overallLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeParticles)
};
