#include "IconButton.h"
#include "PulseTheme.h"

IconButton::IconButton(Icon iconToShow)
    : juce::Button(iconToShow == Icon::Gear ? "settings" : "close"),
      icon(iconToShow)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void IconButton::paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown)
{
    const auto bounds = getLocalBounds().toFloat();
    const float d = juce::jmin(bounds.getWidth(), bounds.getHeight());
    const auto square = bounds.withSizeKeepingCentre(d, d);

    if (isMouseOver || isButtonDown)
    {
        g.setColour(juce::Colour(PulseTheme::SurfaceRaised).withAlpha(isButtonDown ? 1.0f : 0.85f));
        g.fillEllipse(square);
    }

    const auto glyph = square.reduced(d * 0.28f);
    const juce::Colour lineColour = juce::Colour(PulseTheme::DefaultText)
                                        .withAlpha(isMouseOver ? 1.0f : 0.8f);
    g.setColour(lineColour);

    if (icon == Icon::Gear)
    {
        const float cx = glyph.getCentreX(), cy = glyph.getCentreY();
        const float outerR = glyph.getWidth() * 0.5f;
        const float innerR = outerR * 0.42f;

        juce::Path gear;
        constexpr int teeth = 8;

        for (int i = 0; i < teeth; ++i)
        {
            const float a0 = juce::MathConstants<float>::twoPi * (float) i / (float) teeth;
            const float a1 = a0 + juce::MathConstants<float>::twoPi / (float) teeth * 0.5f;

            gear.addPieSegment(cx - outerR, cy - outerR, outerR * 2.0f, outerR * 2.0f, a0, a1, 0.55f);
        }

        g.fillPath(gear);
        g.setColour(juce::Colour(isMouseOver || isButtonDown ? PulseTheme::SurfaceRaised : PulseTheme::WidgetBackground));
        g.fillEllipse(cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f);
        g.setColour(lineColour);
        g.drawEllipse(cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f, 1.2f);
    }
    else // Close
    {
        g.drawLine(glyph.getX(), glyph.getY(), glyph.getRight(), glyph.getBottom(), 1.6f);
        g.drawLine(glyph.getX(), glyph.getBottom(), glyph.getRight(), glyph.getY(), 1.6f);
    }
}
