#include "HeaderComponent.h"
#include "PulseTheme.h"
#include <cmath>

HeaderComponent::HeaderComponent()
{
    setOpaque(false);

    titleLabel.setText("PULSE", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions { 18.0f }.withStyleFlags (juce::Font::bold)));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(PulseTheme::DefaultText));
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    titleLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(titleLabel);

    taglineLabel.setText("SEE YOUR SOUND", juce::dontSendNotification);
    taglineLabel.setFont(juce::Font(juce::FontOptions { 10.0f }).withExtraKerningFactor(0.08f));
    taglineLabel.setColour(juce::Label::textColourId, juce::Colour(PulseTheme::MutedText));
    taglineLabel.setJustificationType(juce::Justification::centredLeft);
    taglineLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(taglineLabel);

    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void HeaderComponent::setAudioActive(bool isActive)
{
    if (audioActive != isActive)
    {
        audioActive = isActive;
        repaint();
    }
}

void HeaderComponent::drawLogoMark(juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    // Small drawn waveform glyph (three bars of varying height) inside a
    // rounded square - avoids needing an image asset for the app icon/mark.
    g.setColour(juce::Colour(PulseTheme::PanelBackground));
    g.fillRoundedRectangle(bounds, 5.0f);
    g.setColour(juce::Colour(PulseTheme::BorderSubtle));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

    const auto barArea = bounds.reduced(bounds.getWidth() * 0.22f);
    const float barWidth = barArea.getWidth() / 5.0f;
    const float heights[3] = { 0.45f, 1.0f, 0.65f };

    g.setColour(juce::Colour(PulseTheme::Accent));

    for (int i = 0; i < 3; ++i)
    {
        const float h = barArea.getHeight() * heights[i];
        const float x = barArea.getX() + (float) (i * 2 + 1) * barWidth * 0.9f;
        const float y = barArea.getCentreY() - h * 0.5f;

        g.fillRoundedRectangle(x, y, barWidth * 0.8f, h, barWidth * 0.35f);
    }
}

juce::Rectangle<float> HeaderComponent::settingsButtonBounds() const
{
    const float size = 26.0f;
    auto b = getLocalBounds().toFloat();
    return { b.getRight() - size - 4.0f, b.getCentreY() - size * 0.5f, size, size };
}

void HeaderComponent::resized()
{
    auto area = getLocalBounds().toFloat();

    // Logo mark occupies the left kLogoSize + 12px; text starts after it.
    area.removeFromLeft(kLogoSize + 12.0f);

    auto textArea = area.withTrimmedLeft(6.0f).withTrimmedRight(120.0f);

    titleLabel.setBounds(textArea.removeFromTop(textArea.getHeight() * 0.6f).toNearestInt());
    taglineLabel.setBounds(textArea.toNearestInt());
}

void HeaderComponent::mouseMove(const juce::MouseEvent& e)
{
    const bool nowHover = settingsButtonBounds().contains(e.position);
    if (nowHover != settingsHover)
    {
        settingsHover = nowHover;
        repaint(settingsButtonBounds().getSmallestIntegerContainer());
    }
}

void HeaderComponent::mouseExit(const juce::MouseEvent&)
{
    if (settingsHover)
    {
        settingsHover = false;
        repaint(settingsButtonBounds().getSmallestIntegerContainer());
    }
}

void HeaderComponent::mouseDown(const juce::MouseEvent& e)
{
    if (settingsButtonBounds().contains(e.position) && onSettingsClicked != nullptr)
        onSettingsClicked();
}

void HeaderComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(PulseTheme::PanelBackground));
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(juce::Colour(PulseTheme::BorderSubtle));
    g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

    drawLogoMark(g, bounds.removeFromLeft(kLogoSize + 12.0f).reduced(6.0f).withSizeKeepingCentre(kLogoSize, kLogoSize));

    // --- LIVE / IDLE status, to the left of the settings button ---
    const auto settingsArea = settingsButtonBounds();
    juce::Rectangle<float> statusArea { settingsArea.getX() - 96.0f, 0.0f, 92.0f, getHeight() * 1.0f };

    const juce::Colour dotColour = audioActive ? juce::Colour(PulseTheme::LiveGreen)
                                                : juce::Colour(PulseTheme::MutedText);

    const float dotSize = 7.0f;
    juce::Rectangle<float> dot { statusArea.getRight() - 60.0f, statusArea.getCentreY() - dotSize * 0.5f, dotSize, dotSize };
    g.setColour(dotColour);
    g.fillEllipse(dot);

    g.setColour(dotColour.withAlpha(audioActive ? 1.0f : 0.8f));
    g.setFont(juce::Font(juce::FontOptions { 11.0f }.withStyleFlags (juce::Font::bold)));
    g.drawText(audioActive ? "LIVE" : "IDLE",
               juce::Rectangle<float> { dot.getRight() + 6.0f, statusArea.getY(), 46.0f, statusArea.getHeight() },
               juce::Justification::centredLeft);

    // --- Settings button (visual shell only - no behaviour wired yet) ---
    g.setColour(juce::Colour(PulseTheme::WidgetBackground).withAlpha(settingsHover ? 1.0f : 0.0f));
    g.fillRoundedRectangle(settingsArea, 6.0f);
    g.setColour(juce::Colour(PulseTheme::BorderSubtle));
    g.drawRoundedRectangle(settingsArea, 6.0f, 1.0f);

    // Simple gear glyph: outer ring + a few teeth ticks + centre dot.
    {
        const auto c = settingsArea.getCentre();
        const float r = settingsArea.getWidth() * 0.24f;

        g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(settingsHover ? 0.95f : 0.75f));
        g.drawEllipse(c.x - r, c.y - r, r * 2.0f, r * 2.0f, 1.6f);
        g.fillEllipse(c.x - 1.5f, c.y - 1.5f, 3.0f, 3.0f);

        for (int i = 0; i < 6; ++i)
        {
            const float angle = (float) i * juce::MathConstants<float>::pi / 3.0f;
            const float x1 = c.x + std::cos(angle) * (r + 1.5f);
            const float y1 = c.y + std::sin(angle) * (r + 1.5f);
            const float x2 = c.x + std::cos(angle) * (r + 4.0f);
            const float y2 = c.y + std::sin(angle) * (r + 4.0f);
            g.drawLine(x1, y1, x2, y2, 1.6f);
        }
    }
}
