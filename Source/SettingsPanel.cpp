#include "SettingsPanel.h"
#include "PulseTheme.h"

namespace
{
    struct Row { juce::String label; juce::String value; };

    void drawSectionHeader(juce::Graphics& g, juce::Rectangle<float> area, const juce::String& text)
    {
        g.setColour(juce::Colour(PulseTheme::MutedText));
        g.setFont(juce::Font(11.0f, juce::Font::bold).withExtraKerningFactor(0.08f));
        g.drawText(text, area, juce::Justification::centredLeft);
    }

    void drawRow(juce::Graphics& g, juce::Rectangle<float> area, const Row& row, bool comingSoon)
    {
        g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(comingSoon ? 0.55f : 0.85f));
        g.setFont(12.5f);
        g.drawText(row.label, area, juce::Justification::centredLeft);

        g.setColour(juce::Colour(PulseTheme::MutedText));
        g.setFont(12.0f);
        g.drawText(row.value, area, juce::Justification::centredRight);
    }
}

SettingsPanel::SettingsPanel()
{
    setOpaque(false);

    addAndMakeVisible(closeButton);
    closeButton.onClick = [this] { if (onCloseClicked != nullptr) onCloseClicked(); };
}

void SettingsPanel::setAudioInfo(const juce::String& newDeviceName, double newSampleRate)
{
    if (deviceName != newDeviceName || sampleRate != newSampleRate)
    {
        deviceName = newDeviceName;
        sampleRate = newSampleRate;
        repaint();
    }
}

void SettingsPanel::resized()
{
    constexpr int buttonSize = 26;
    closeButton.setBounds(getLocalBounds().removeFromTop(50).removeFromRight(buttonSize + 14)
                                            .withSizeKeepingCentre(buttonSize, buttonSize));
}

void SettingsPanel::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    PulseTheme::panelBackground(g, bounds, 0.0f, PulseTheme::WindowBackground);
    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.7f));
    g.drawLine(bounds.getX() + 0.5f, bounds.getY(), bounds.getX() + 0.5f, bounds.getBottom(), 1.0f);

    auto area = bounds.reduced(18.0f, 0.0f);

    auto header = area.removeFromTop(50.0f);
    g.setColour(juce::Colour(PulseTheme::DefaultText));
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText("SETTINGS", header.withTrimmedRight(40.0f), juce::Justification::centredLeft);

    area.removeFromTop(6.0f);
    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.5f));
    g.drawHorizontalLine((int) area.getY(), area.getX(), area.getRight());
    area.removeFromTop(16.0f);

    constexpr float rowH = 26.0f;

    // --- AUDIO: this section is real, read-only info - the device Pulse is
    // actually capturing from right now, not a placeholder. ---
    drawSectionHeader(g, area.removeFromTop(18.0f), "AUDIO");
    area.removeFromTop(6.0f);
    drawRow(g, area.removeFromTop(rowH),
            { "Device", deviceName.isNotEmpty() ? deviceName : juce::String("-") }, false);
    drawRow(g, area.removeFromTop(rowH),
            { "Sample rate", sampleRate > 0.0 ? juce::String(sampleRate, 0) + " Hz" : juce::String("-") }, false);

    area.removeFromTop(20.0f);

    // --- Everything below is a stub: real values shown, but not editable
    // yet, and clearly marked as such. ---
    drawSectionHeader(g, area.removeFromTop(18.0f), "VISUALIZER  \u00b7  coming soon");
    area.removeFromTop(6.0f);
    drawRow(g, area.removeFromTop(rowH), { "Sensitivity",   "1.0" }, true);
    drawRow(g, area.removeFromTop(rowH), { "Smoothing",     "0.7" }, true);
    drawRow(g, area.removeFromTop(rowH), { "Brightness",    "1.0" }, true);
    drawRow(g, area.removeFromTop(rowH), { "Peak intensity","1.2" }, true);

    area.removeFromTop(20.0f);

    drawSectionHeader(g, area.removeFromTop(18.0f), "PERFORMANCE  \u00b7  coming soon");
    area.removeFromTop(6.0f);
    drawRow(g, area.removeFromTop(rowH), { "FPS limit",      "60 FPS" }, true);
    drawRow(g, area.removeFromTop(rowH), { "Render quality", "High" }, true);
    drawRow(g, area.removeFromTop(rowH), { "Show goniometer","On" }, true);
}
