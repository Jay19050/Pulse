#pragma once

#include <JuceHeader.h>
#include <vector>
#include "SpectrumAnalyzer.h"
#include "VisualizerSettings.h"

// Common interface for every visualizer rendering mode (Spectrum, Filled
// Spectrum, Waveform, Circular, Mirror, Particles). VisualizerModeManager owns
// one instance of each and shows only the active one.
//
// Every mode is fed the SAME shared audio-analysis snapshot each tick - no mode
// performs its own FFT or touches AudioEngine directly. This keeps "shared
// analysis -> N renderers" instead of "N audio pipelines".
class VisualizerMode : public juce::Component
{
public:
    ~VisualizerMode() override = default;

    // Called once whenever the audio engine (re)starts, with the live sample
    // rate. Most modes don't need this (SpectrumAnalyzer already normalises
    // its output to 0..1), but it's here for modes that want it (e.g. mapping
    // waveform sample count to a fixed on-screen time window).
    virtual void prepare(double /*sampleRate*/) {}

    // Called every analysis tick (30 Hz, same cadence as MainComponent's
    // timer) with the shared spectrum snapshot and a short rolling window of
    // raw mono waveform samples, oldest first. Modes that don't need the raw
    // waveform (Spectrum, Filled Spectrum, Circular, Mirror) simply ignore it.
    virtual void updateData(const SpectrumAnalyzer::Snapshot& snapshot,
                             const std::vector<float>& waveform) = 0;

    // Visual-only rendering parameters (sensitivity/smoothing/brightness/
    // peak intensity/background), set from the Settings panel. Stored once
    // here rather than duplicated per mode; subclasses read `appearance.*`
    // directly in paint()/updateData(). Never affects captured audio.
    virtual void setAppearance(const VisualizerSettings& newAppearance) { appearance = newAppearance; }

    // Short label shown in the mode-selector strip, e.g. "Spectrum".
    virtual juce::String getModeName() const = 0;

protected:
    VisualizerSettings appearance;
};
