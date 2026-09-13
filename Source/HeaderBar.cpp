#include "HeaderBar.h"
#include "PulseTheme.h"

HeaderBar::HeaderBar()
{
    setOpaque(false);

    addAndMakeVisible(settingsButton);
    settingsButton.onClick = [this] { if (onSettingsClicked != nullptr) onSettingsClicked(); };
}

void HeaderBar::setActive(bool isActive)
{
    if (active != isActive)
    {
        active = isActive;
        repaint();
    }
}

void HeaderBar::setDeviceInfo(const juce::String& text)
{
    if (deviceInfo != text)
    {
        deviceInfo = text;
        repaint();
    }
}

void HeaderBar::resized()
{
    constexpr int buttonSize = 30;
    settingsButton.setBounds(getLocalBounds().removeFromRight(buttonSize + 14)
                                              .withSizeKeepingCentre(buttonSize, buttonSize));
}

void HeaderBar::drawLogo(juce::Graphics& g, juce::Rectangle<float> area) const
{
    // An abstract "pulse" mark - not a literal reproduction of any external
    // logo, just a simple waveform glyph in a rounded square, matching the
    // accent colour used throughout the app.
    g.setColour(juce::Colour(PulseTheme::Accent).withAlpha(0.14f));
    g.fillRoundedRectangle(area, area.getHeight() * 0.28f);

    juce::Path p;
    const float w = area.getWidth(), h = area.getHeight();
    const float x = area.getX(), y = area.getY();

    p.startNewSubPath(x + w * 0.14f, y + h * 0.5f);
    p.lineTo(x + w * 0.32f, y + h * 0.5f);
    p.lineTo(x + w * 0.42f, y + h * 0.22f);
    p.lineTo(x + w * 0.56f, y + h * 0.78f);
    p.lineTo(x + w * 0.66f, y + h * 0.5f);
    p.lineTo(x + w * 0.86f, y + h * 0.5f);

    g.setColour(juce::Colour(PulseTheme::Accent));
    g.strokePath(p, juce::PathStrokeType(2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void HeaderBar::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();

    g.setColour(juce::Colour(PulseTheme::WindowBackground));
    g.fillRect(area);
    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.6f));
    g.drawLine(area.getX(), area.getBottom() - 0.5f, area.getRight(), area.getBottom() - 0.5f, 1.0f);

    auto content = area.reduced(18.0f, 0.0f);

    // --- Left: logo + wordmark + tagline ---
    auto logoArea = content.removeFromLeft(content.getHeight() - 8.0f).reduced(0.0f, 4.0f);
    drawLogo(g, logoArea);
    content.removeFromLeft(12.0f);

    g.setColour(juce::Colour(PulseTheme::DefaultText));
    g.setFont(juce::Font(19.0f, juce::Font::bold));
    auto titleArea = content.removeFromLeft(90.0f);
    g.drawText("PULSE", titleArea, juce::Justification::centredLeft);

    g.setColour(juce::Colour(PulseTheme::MutedText));
    g.setFont(juce::Font(10.0f).withExtraKerningFactor(0.12f));
    auto tagArea = content.removeFromLeft(140.0f);
    g.drawText("SEE YOUR SOUND", tagArea, juce::Justification::centredLeft);

    // --- Right: device info, then status dot + label, then settings button ---
    auto right = getLocalBounds().toFloat().reduced(18.0f, 0.0f);
    right.removeFromRight(30.0f + 14.0f); // reserve settings button + its own margin

    auto statusArea = right.removeFromRight(90.0f);
    const juce::Colour statusColour = juce::Colour(active ? PulseTheme::StatusLive : PulseTheme::StatusIdle);

    const float dotSize = 8.0f;
    const auto dotBounds = statusArea.removeFromLeft(dotSize + 8.0f)
                                      .withSizeKeepingCentre(dotSize, dotSize);
    g.setColour(statusColour);
    g.fillEllipse(dotBounds);

    if (active)
    {
        g.setColour(statusColour.withAlpha(0.28f));
        g.fillEllipse(dotBounds.expanded(3.0f));
    }

    g.setColour(statusColour);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(active ? "LIVE" : "IDLE", statusArea, juce::Justification::centredLeft);

    if (deviceInfo.isNotEmpty())
    {
        g.setColour(juce::Colour(PulseTheme::MutedText));
        g.setFont(11.0f);
        g.drawText(deviceInfo, right.reduced(0.0f, 0.0f).withTrimmedRight(14.0f),
                   juce::Justification::centredRight);
    }
}
