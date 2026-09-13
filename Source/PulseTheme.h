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

    // --- UI chrome tokens (header, mode selector, settings drawer) ---------
    // Added for the UI polish pass; the visualizer panels above stay on
    // WidgetBackground/Outline/DefaultText/Accent so this doesn't touch their look.

    // Slightly lighter than WidgetBackground - used for surfaces that sit "on top
    // of" the base panels: the mode-selector bar, the settings drawer, hover fills.
    constexpr juce::uint32 SurfaceRaised    = 0xff1f1f1f;

    // Dimmer than DefaultText - secondary/muted labels (taglines, section tags,
    // inactive mode captions).
    constexpr juce::uint32 MutedText        = 0xff707070;

    // Audio-activity dot: green while WASAPI has real signal, muted grey when idle.
    constexpr juce::uint32 StatusLive       = 0xff34d399;
    constexpr juce::uint32 StatusIdle       = 0xff5a5a5a;

    // Fills + strokes a standard rounded panel background. Centralises the
    // "WidgetBackground fill + thin Outline border" look shared by every panel
    // (visualizer modes, goniometer, level meter, header, mode bar, settings
    // drawer) so they read as one consistent design system rather than six
    // separately-tuned rectangles.
    inline void panelBackground(juce::Graphics& g, juce::Rectangle<float> bounds,
                                 float cornerRadius = 6.0f, juce::uint32 fill = WidgetBackground)
    {
        g.setColour(juce::Colour(fill));
        g.fillRoundedRectangle(bounds, cornerRadius);

        g.setColour(juce::Colour(Outline).withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerRadius, 1.0f);
    }
}
