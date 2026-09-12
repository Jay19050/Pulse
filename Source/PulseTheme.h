#pragma once

#include <JuceHeader.h>

// Central colour palette, matched exactly to the FxSound Mac reference app's theme
// (its FxTheme.cpp colour table). Kept as plain hex constants rather than a
// LookAndFeel/colour-scheme system, since Pulse does not have enough UI surface
// yet to justify one - if that changes, these are the values to carry forward.
//
// Stored pre-multiplied with a full alpha byte (0xff......) so every constant is
// usable directly as `juce::Colour(PulseTheme::X)` without a separate
// `.withAlpha(1.0f)` call; call `.withAlpha(...)` again wherever a translucent
// draw is needed.
namespace PulseTheme
{
    constexpr juce::uint32 WindowBackground = 0xff181818;
    constexpr juce::uint32 WidgetBackground = 0xff181818;
    constexpr juce::uint32 Outline          = 0xff2b2b2b;
    constexpr juce::uint32 DefaultText      = 0xffb1b1b1;

    // The one dominant accent colour used throughout - "EqStart" in the reference.
    constexpr juce::uint32 Accent           = 0xffef4b65;

    // A pale wash of Accent, reserved for the peak-hold trace/line so it always
    // reads as "a ceiling on the live signal" rather than a sixth competing hue.
    constexpr juce::uint32 PeakHold         = 0xffffb3be;
}
