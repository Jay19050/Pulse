#pragma once

#include <JuceHeader.h>
#include "VisualizerModeManager.h"

// A full-width bottom bar of mode cards (icon + label), replacing the old
// compact text strip that used to live inside VisualizerModeManager itself.
// This is a separate component precisely so the mode selector can be a
// first-class, prominent piece of the layout (per the UI brief) rather than
// a detail tucked inside the visualizer's own bounds.
//
// Doesn't own any mode state - it's a thin view over VisualizerModeManager's
// public index/name API, so VisualizerModeManager stays the single source of
// truth for "which mode is active".
class ModeSelectorBar final : public juce::Component
{
public:
    explicit ModeSelectorBar(VisualizerModeManager& managerToControl);

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    void drawIcon(juce::Graphics& g, const juce::String& modeName,
                  juce::Rectangle<float> box, juce::Colour colour) const;

    VisualizerModeManager& manager;
    std::vector<juce::Rectangle<float>> cardBounds;
    int hoverIndex = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModeSelectorBar)
};
