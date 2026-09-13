#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "VisualizerMode.h"

// Owns one instance of every visualizer mode and shows exactly one at a time,
// filling its entire bounds.
//
// The mode-selector UI used to live inside this class (a text strip painted
// along the top). It has since moved out to the standalone ModeSelectorBar,
// so the main visualizer area is 100% visualization - the selector is now a
// first-class sibling in MainComponent's layout instead of eating into this
// component's bounds. This class kept its name and its "own every mode,
// switch which is visible" responsibility; it just no longer draws UI chrome
// itself.
//
// Every mode receives updateData() on every tick regardless of whether it's
// currently visible, so switching modes shows current audio immediately
// rather than a stale frame from whenever it was last active.
class VisualizerModeManager final : public juce::Component
{
public:
    VisualizerModeManager();

    void prepare(double sampleRate);
    void updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>& waveform);

    int getNumModes() const { return (int) modes.size(); }
    juce::String getModeName(int index) const;
    int getActiveIndex() const { return activeIndex; }
    void setActiveIndex(int index);

    void resized() override;

    // Forwarded from MainComponent so the Spectrum mode's "click to clear
    // peak hold" behaviour keeps working when it's the active mode.
    std::function<void()> onResetPeaks;

    // Fired whenever setActiveIndex() actually changes the active mode, so
    // ModeSelectorBar (or anything else showing the current mode) can repaint
    // without polling.
    std::function<void(int)> onActiveModeChanged;

private:
    std::vector<std::unique_ptr<VisualizerMode>> modes;
    int activeIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerModeManager)
};
