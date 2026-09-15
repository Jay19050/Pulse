#pragma once

#include <JuceHeader.h>

// Shared, purely-visual rendering parameters that every visualizer mode reads
// while painting. Nothing here ever touches captured audio, system volume,
// or Windows audio settings - it only reshapes how the already-analyzed
// spectrum/waveform data is drawn.
//
// Touched only on the message thread: Settings-panel slider drags write it,
// Component::paint() reads it. Both happen on the same (message) thread, so
// plain floats are fine here - this is NOT audio-thread-shared state like
// SpectrumAnalyzer's snapshot, and needs no locking.
struct VisualizerSettings
{
    float sensitivity   = 1.0f;  // 0.4..2.0 - scales analyzed magnitude before it's rendered
    float smoothing     = 0.35f; // 0.0..0.85 - extra mode-local damping, on top of SpectrumAnalyzer's own
    float brightness    = 1.0f;  // 0.5..1.6 - scales fill/stroke alpha
    float peakIntensity = 1.0f;  // 0.0..2.0 - scales peak-hold/hot-transient prominence
    float background    = 0.5f;  // 0.0..1.0 - panel background wash; kept in a readable range at paint time
};

// A one-pole low-pass filter, used by modes that want an extra visual
// damping pass distinct from SpectrumAnalyzer's own fast/slow smoothing -
// e.g. so "Smoothing" can visibly change Particles' drift or Circular's ray
// motion without touching the analyzer itself.
struct EnvelopeFollower
{
    float value = 0.0f;
    bool primed = false;

    float advance(float target, float smoothing) noexcept
    {
        if (! primed)
        {
            value = target;
            primed = true;
            return value;
        }

        const float coeff = juce::jlimit(0.0f, 0.95f, smoothing); // higher = slower/smoother
        value = value * coeff + target * (1.0f - coeff);
        return value;
    }
};
