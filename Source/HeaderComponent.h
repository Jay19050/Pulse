#pragma once

#include <JuceHeader.h>
#include <functional>

// Compact top header bar: a small drawn waveform mark + "PULSE" wordmark +
// tagline on the left, a LIVE/IDLE status dot and a settings button on the
// right. Purely presentational - setAudioActive() reflects the SAME
// audio-active state MainComponent already derives from
// SpectrumAnalyzer::Snapshot::active, it does not invent a new one.
//
// The settings button is a visual shell only for this pass (per the current
// task's scope): onSettingsClicked fires so a later task can attach real
// behaviour, but nothing is wired to it yet, and clicking it today does
// nothing visible beyond its own hover/press feedback.
class HeaderComponent final : public juce::Component
{
public:
    HeaderComponent();

    void setAudioActive(bool isActive);

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;

    std::function<void()> onSettingsClicked;

private:
    void drawLogoMark(juce::Graphics& g, juce::Rectangle<float> bounds) const;

    juce::Rectangle<float> settingsButtonBounds() const;

    bool audioActive = false;
    bool settingsHover = false;

    juce::Label titleLabel;
    juce::Label taglineLabel;

    static constexpr float kLogoSize = 22.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderComponent)
};
