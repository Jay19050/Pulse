#pragma once

#include <JuceHeader.h>

// A minimal, custom-painted icon button - a circular hover/pressed background
// behind a small vector glyph drawn directly with juce::Path. Deliberately not
// a juce::ImageButton (no bitmap assets in this project) or the default
// juce::TextButton look (which reads as "generic Windows control", not part
// of Pulse's own visual language).
class IconButton final : public juce::Button
{
public:
    enum class Icon { Gear, Close };

    explicit IconButton(Icon iconToShow);

    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override;

private:
    Icon icon;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IconButton)
};
