#include "ModeSelectorBar.h"
#include "PulseTheme.h"
#include <cmath>

ModeSelectorBar::ModeSelectorBar(VisualizerModeManager& managerToControl)
    : manager(managerToControl)
{
    setOpaque(false);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);

    manager.onActiveModeChanged = [this](int) { repaint(); };

    cardBounds.resize((size_t) manager.getNumModes());
}

void ModeSelectorBar::resized()
{
    const auto area = getLocalBounds().toFloat().reduced(10.0f, 8.0f);
    const int n = manager.getNumModes();
    if (n <= 0)
        return;

    constexpr float gap = 8.0f;
    const float cardWidth = (area.getWidth() - gap * (float) (n - 1)) / (float) n;

    for (int i = 0; i < n; ++i)
    {
        cardBounds[(size_t) i] = { area.getX() + (float) i * (cardWidth + gap), area.getY(),
                                    cardWidth, area.getHeight() };
    }
}

void ModeSelectorBar::mouseMove(const juce::MouseEvent& e)
{
    int next = -1;
    for (size_t i = 0; i < cardBounds.size(); ++i)
        if (cardBounds[i].contains(e.position))
            next = (int) i;

    if (next != hoverIndex)
    {
        hoverIndex = next;
        repaint();
    }
}

void ModeSelectorBar::mouseExit(const juce::MouseEvent&)
{
    if (hoverIndex != -1)
    {
        hoverIndex = -1;
        repaint();
    }
}

void ModeSelectorBar::mouseDown(const juce::MouseEvent& e)
{
    for (size_t i = 0; i < cardBounds.size(); ++i)
        if (cardBounds[i].contains(e.position))
            manager.setActiveIndex((int) i);
}

void ModeSelectorBar::drawIcon(juce::Graphics& g, const juce::String& modeName,
                                juce::Rectangle<float> box, juce::Colour colour) const
{
    g.setColour(colour);
    juce::Path p;

    if (modeName == "Spectrum")
    {
        constexpr int bars = 5;
        const float heights[bars] = { 0.4f, 0.75f, 1.0f, 0.6f, 0.85f };
        const float bw = box.getWidth() / (float) bars * 0.6f;
        const float gap = box.getWidth() / (float) bars * 0.4f;

        for (int i = 0; i < bars; ++i)
        {
            const float h = box.getHeight() * heights[i];
            const float x = box.getX() + (float) i * (bw + gap);
            g.fillRoundedRectangle(x, box.getBottom() - h, bw, h, 1.0f);
        }
    }
    else if (modeName == "Waveform")
    {
        p.startNewSubPath(box.getX(), box.getCentreY());
        const int steps = 24;
        for (int i = 1; i <= steps; ++i)
        {
            const float t = (float) i / (float) steps;
            const float x = box.getX() + t * box.getWidth();
            const float y = box.getCentreY() - std::sin(t * juce::MathConstants<float>::twoPi * 1.5f)
                                                * box.getHeight() * 0.42f
                                                * std::sin(t * juce::MathConstants<float>::pi); // taper ends
            p.lineTo(x, y);
        }
        g.strokePath(p, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    else if (modeName == "Filled Spectrum")
    {
        p.startNewSubPath(box.getBottomLeft());
        p.lineTo(box.getX(), box.getBottom() - box.getHeight() * 0.3f);
        p.lineTo(box.getX() + box.getWidth() * 0.35f, box.getY());
        p.lineTo(box.getX() + box.getWidth() * 0.65f, box.getBottom() - box.getHeight() * 0.5f);
        p.lineTo(box.getRight(), box.getY() + box.getHeight() * 0.15f);
        p.lineTo(box.getBottomRight());
        p.closeSubPath();
        g.fillPath(p);
    }
    else if (modeName == "Circular")
    {
        const float r = juce::jmin(box.getWidth(), box.getHeight()) * 0.5f;
        g.drawEllipse(box.getCentreX() - r, box.getCentreY() - r, r * 2.0f, r * 2.0f, 1.8f);
    }
    else if (modeName == "Mirror")
    {
        constexpr int bars = 5;
        const float heights[bars] = { 0.3f, 0.65f, 1.0f, 0.5f, 0.7f };
        const float bw = box.getWidth() / (float) bars * 0.6f;
        const float gap = box.getWidth() / (float) bars * 0.4f;

        for (int i = 0; i < bars; ++i)
        {
            const float h = box.getHeight() * 0.5f * heights[i];
            const float x = box.getX() + (float) i * (bw + gap);
            g.fillRoundedRectangle(x, box.getCentreY() - h, bw, h * 2.0f, 1.0f);
        }
    }
    else // Particles
    {
        juce::Random rng(42);
        for (int i = 0; i < 9; ++i)
        {
            const float x = box.getX() + rng.nextFloat() * box.getWidth();
            const float y = box.getY() + rng.nextFloat() * box.getHeight();
            const float s = 1.6f + rng.nextFloat() * 1.6f;
            g.fillEllipse(x - s * 0.5f, y - s * 0.5f, s, s);
        }
    }
}

void ModeSelectorBar::paint(juce::Graphics& g)
{
    PulseTheme::panelBackground(g, getLocalBounds().toFloat(), 8.0f, PulseTheme::SurfaceRaised);

    const int active = manager.getActiveIndex();

    for (size_t i = 0; i < cardBounds.size(); ++i)
    {
        const auto card = cardBounds[i];
        const bool isActive = (int) i == active;
        const bool isHover  = (int) i == hoverIndex;
        const juce::String fullName = manager.getModeName((int) i);
        const juce::String label = fullName == "Filled Spectrum" ? "Filled" : fullName;

        if (isActive)
        {
            g.setColour(juce::Colour(PulseTheme::Accent).withAlpha(0.14f));
            g.fillRoundedRectangle(card, 8.0f);
            g.setColour(juce::Colour(PulseTheme::Accent).withAlpha(0.55f));
            g.drawRoundedRectangle(card.reduced(0.5f), 8.0f, 1.2f);
        }
        else if (isHover)
        {
            g.setColour(juce::Colour(PulseTheme::WidgetBackground));
            g.fillRoundedRectangle(card, 8.0f);
        }

        const juce::Colour colour = isActive ? juce::Colour(PulseTheme::Accent)
                                              : juce::Colour(PulseTheme::DefaultText)
                                                    .withAlpha(isHover ? 0.9f : 0.6f);

        auto content = card.reduced(6.0f);
        auto iconBox = content.removeFromTop(content.getHeight() * 0.55f).withSizeKeepingCentre(20.0f, 20.0f);
        drawIcon(g, fullName, iconBox, colour);

        g.setColour(colour);
        g.setFont(juce::Font(11.5f, isActive ? juce::Font::bold : juce::Font::plain));
        g.drawText(label, content, juce::Justification::centred);

        if (isActive)
        {
            const float underlineY = card.getBottom() - 3.0f;
            g.setColour(juce::Colour(PulseTheme::Accent));
            g.fillRoundedRectangle(card.getCentreX() - 12.0f, underlineY, 24.0f, 2.0f, 1.0f);
        }
    }
}
