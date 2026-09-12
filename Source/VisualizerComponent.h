#pragma once

#include <JuceHeader.h>
#include <functional>
#include "SpectrumAnalyzer.h"
#include "VisualizerMode.h"

// Real-time spectrum display, restyled to match the FxSound Mac reference design:
// dark panel, log-frequency grid, a collapsible legend with per-trace visibility
// toggles, a hover crosshair + readout box, and a slow-decaying peak-hold trace.
//
// Pulse has no DSP stage yet, so there is no dry/wet distinction to draw - unlike
// the reference, which plots pre- and post-processing spectra together. What
// SpectrumAnalyzer calls "fast" and "slow" are shown here as LIVE (the real-time
// curve, filled) and AVG (a slower trailing reference, unfilled ghost), plus PEAK
// HOLD (a decaying ceiling on LIVE).
//
// This is the "Spectrum" visualizer mode - the original (and still default)
// Pulse view. It predates VisualizerMode and has its own richer feature set
// (legend, hover readout, per-trace toggles) that the other modes don't need,
// so it keeps its existing name and API rather than being renamed to fit them.
class VisualizerComponent final : public VisualizerMode, private juce::Timer
{
public:
    enum Trace
    {
        TraceLive = 0,
        TraceAverage,
        TracePeakHold,
        kNumTraces
    };

    VisualizerComponent();

    void setSnapshot(const SpectrumAnalyzer::Snapshot& newSnapshot);

    // VisualizerMode
    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>&) override
    {
        setSnapshot(snapshot); // this mode doesn't use the raw waveform
    }
    juce::String getModeName() const override { return "Spectrum"; }

    void paint(juce::Graphics&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;

    // Wired by MainComponent to SpectrumAnalyzer::resetPeakHold() - clicking the
    // plot (outside the legend) clears the peak-hold trace.
    std::function<void()> onResetPeaks;

private:
    void timerCallback() override;

    juce::Rectangle<float> plotArea() const;

    float xForPoint(int pointIndex, juce::Rectangle<float> plot) const;
    float yForValue(float normalisedValue, juce::Rectangle<float> plot) const;
    int   pointForX(float x, juce::Rectangle<float> plot) const;

    juce::Path buildCurve(const std::array<float, SpectrumAnalyzer::spectrumPoints>& values,
                          juce::Rectangle<float> plot) const;

    void drawGrid(juce::Graphics& g, juce::Rectangle<float> plot) const;
    void drawHoverReadout(juce::Graphics& g, juce::Rectangle<float> plot) const;

    void setTraceVisible(Trace t, bool visible);
    bool isTraceVisible(Trace t) const { return traceVisible[(size_t) t]; }
    void setLegendCollapsed(bool collapsed);

    juce::Rectangle<float> legendHandleBounds() const;
    juce::Rectangle<float> legendRowBounds(int index) const;
    juce::Rectangle<float> legendPanelBounds() const;
    int legendRowAt(juce::Point<float> p) const;

    void drawLegendBackdrop(juce::Graphics& g) const;
    void drawLegendForeground(juce::Graphics& g) const;

    SpectrumAnalyzer::Snapshot snapshot;

    std::array<bool, kNumTraces> traceVisible { true, true, true };
    bool legendCollapsed = false;

    float hoverX = -1.0f;

    static constexpr float kLeftGutter = 34.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerComponent)
};
