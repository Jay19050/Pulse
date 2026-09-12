#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "VisualizerMode.h"

// Owns one instance of every visualizer mode and shows exactly one at a time,
// with a slim, custom-painted selector strip along the top (deliberately NOT
// a native juce::ComboBox / Windows dropdown - it's a row of text labels in
// Pulse's own dark/minimal language, consistent with VisualizerComponent's
// legend panel elsewhere in the app).
//
// Every mode receives updateData() on every tick regardless of whether it's
// currently visible, so switching modes shows current audio immediately
// rather than a stale frame from whenever it was last active. This costs
// six cheap array copies per tick (30 Hz) - negligible next to the FFT that
// produced the snapshot in the first place.
class VisualizerModeManager final : public juce::Component
{
public:
    VisualizerModeManager();

    void prepare(double sampleRate);
    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>& waveform);

    void resized() override;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

    // Forwarded from MainComponent so the Spectrum mode's "click to clear
    // peak hold" behaviour keeps working when it's the active mode.
    std::function<void()> onResetPeaks;

private:
    void setActiveIndex(int index);
    juce::Rectangle<float> selectorStripBounds() const;

    std::vector<std::unique_ptr<VisualizerMode>> modes;
    std::vector<juce::Rectangle<float>> itemBounds; // recomputed in resized()
    int activeIndex = 0;
    int hoverIndex = -1;

    static constexpr float kStripHeight = 28.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerModeManager)
};
