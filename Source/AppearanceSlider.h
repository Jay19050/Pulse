#pragma once

#include <JuceHeader.h>

// A single custom-drawn appearance slider: label, thin track, accent-filled
// active portion, a round thumb, and a small live value readout - not a
// juce::Slider (which renders with the platform LookAndFeel's default style,
// not Pulse's own visual language).
//
// Purely a UI control: it knows a label, a range, and a value. It has no
// idea what the value means or does - VisualizerSettings/SettingsPanel own
// that. Dragging fires onValueChanged immediately and continuously, so the
// visualizer updates live while the user drags.
class AppearanceSlider final : public juce::Component
{
public:
    AppearanceSlider(juce::String label, float minValue, float maxValue, float initialValue);

    float getValue() const noexcept { return value; }
    void setValue(float newValue, bool notify = false);

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;

    std::function<void(float)> onValueChanged;

private:
    void setValueFromMouseX(float mouseX);
    juce::Rectangle<float> trackBounds() const;

    juce::String label;
    float minValue, maxValue, value;
    bool hover = false, dragging = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppearanceSlider)
};
