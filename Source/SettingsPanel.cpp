#include "SettingsPanel.h"
#include "PulseTheme.h"

namespace
{
    void drawSectionHeader(juce::Graphics& g, juce::Rectangle<float> area, const juce::String& text)
    {
        g.setColour(juce::Colour(PulseTheme::MutedText));
        g.setFont(juce::Font(11.0f, juce::Font::bold).withExtraKerningFactor(0.08f));
        g.drawText(text, area, juce::Justification::centredLeft);
    }

    void drawRow(juce::Graphics& g, juce::Rectangle<float> area, const juce::String& label,
                 const juce::String& value, bool comingSoon)
    {
        g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(comingSoon ? 0.55f : 0.85f));
        g.setFont(12.5f);
        g.drawText(label, area, juce::Justification::centredLeft);

        g.setColour(juce::Colour(PulseTheme::MutedText));
        g.setFont(12.0f);
        g.drawText(value, area, juce::Justification::centredRight);
    }
}

SettingsPanel::SettingsPanel()
{
    setOpaque(false);

    addAndMakeVisible(closeButton);
    closeButton.onClick = [this] { if (onCloseClicked != nullptr) onCloseClicked(); };

    addAndMakeVisible(deviceSelector);
    deviceSelector.onDeviceSelected = [this](juce::String id)
    {
        if (onDeviceSelected != nullptr)
            onDeviceSelected(id);
    };
    deviceSelector.onRefreshRequested = [this]
    {
        if (onRefreshDevicesRequested != nullptr)
            onRefreshDevicesRequested();
    };

    auto wireSlider = [this](AppearanceSlider& slider, float VisualizerSettings::* field)
    {
        addAndMakeVisible(slider);
        slider.onValueChanged = [this, field](float v)
        {
            appearance.*field = v;
            notifyAppearanceChanged();
        };
    };

    wireSlider(sensitivitySlider,   &VisualizerSettings::sensitivity);
    wireSlider(smoothingSlider,     &VisualizerSettings::smoothing);
    wireSlider(brightnessSlider,    &VisualizerSettings::brightness);
    wireSlider(peakIntensitySlider, &VisualizerSettings::peakIntensity);
    wireSlider(backgroundSlider,    &VisualizerSettings::background);
}

void SettingsPanel::notifyAppearanceChanged()
{
    if (onAppearanceChanged != nullptr)
        onAppearanceChanged(appearance);
}

void SettingsPanel::setDevices(std::vector<AudioEngine::DeviceInfo> devices, const juce::String& selectedId)
{
    deviceSelector.setDevices(std::move(devices), selectedId);
}

void SettingsPanel::setSelectedDevice(const juce::String& id)
{
    deviceSelector.setSelectedId(id);
}

void SettingsPanel::setSampleRate(double newSampleRate)
{
    if (sampleRate != newSampleRate)
    {
        sampleRate = newSampleRate;
        repaint();
    }
}

SettingsPanel::Layout SettingsPanel::computeLayout() const
{
    Layout l;

    auto area = getLocalBounds().toFloat().reduced(18.0f, 0.0f);

    l.header = area.removeFromTop(50.0f);
    area.removeFromTop(6.0f);
    l.rule = area.removeFromTop(1.0f);
    area.removeFromTop(16.0f);

    l.audioOutputLabel = area.removeFromTop(18.0f);
    area.removeFromTop(6.0f);
    l.deviceSelector = area.removeFromTop(34.0f + 8.0f + 20.0f);
    area.removeFromTop(12.0f);
    l.sampleRateRow = area.removeFromTop(22.0f);
    area.removeFromTop(14.0f);

    l.displayLabel = area.removeFromTop(18.0f);
    area.removeFromTop(6.0f);
    l.fullscreenRow = area.removeFromTop(24.0f);
    area.removeFromTop(14.0f);

    l.appearanceLabel = area.removeFromTop(18.0f);
    area.removeFromTop(6.0f);
    for (auto& s : l.sliders)
    {
        s = area.removeFromTop(38.0f);
        area.removeFromTop(8.0f);
    }
    area.removeFromTop(6.0f);

    l.performanceLabel = area.removeFromTop(18.0f);
    area.removeFromTop(6.0f);
    l.perfRow1 = area.removeFromTop(22.0f);
    l.perfRow2 = area.removeFromTop(22.0f);
    l.perfRow3 = area.removeFromTop(22.0f);

    return l;
}

void SettingsPanel::resized()
{
    layout = computeLayout();

    constexpr int buttonSize = 26;
    closeButton.setBounds(layout.header.toNearestInt().removeFromRight(buttonSize + 14)
                                          .withSizeKeepingCentre(buttonSize, buttonSize));

    deviceSelector.setBounds(layout.deviceSelector.toNearestInt());

    AppearanceSlider* sliders[] = { &sensitivitySlider, &smoothingSlider, &brightnessSlider,
                                     &peakIntensitySlider, &backgroundSlider };
    for (size_t i = 0; i < layout.sliders.size(); ++i)
        sliders[i]->setBounds(layout.sliders[i].toNearestInt());
}

void SettingsPanel::mouseMove(const juce::MouseEvent& e)
{
    const bool over = layout.fullscreenRow.contains(e.position);
    if (over != fullscreenRowHover)
    {
        fullscreenRowHover = over;
        setMouseCursor(over ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
        repaint();
    }
}

void SettingsPanel::mouseExit(const juce::MouseEvent&)
{
    if (fullscreenRowHover)
    {
        fullscreenRowHover = false;
        repaint();
    }
}

void SettingsPanel::mouseDown(const juce::MouseEvent& e)
{
    if (layout.fullscreenRow.contains(e.position) && onToggleFullscreenClicked != nullptr)
        onToggleFullscreenClicked();
}

void SettingsPanel::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    PulseTheme::panelBackground(g, bounds, 0.0f, PulseTheme::WindowBackground);
    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.7f));
    g.drawLine(bounds.getX() + 0.5f, bounds.getY(), bounds.getX() + 0.5f, bounds.getBottom(), 1.0f);

    g.setColour(juce::Colour(PulseTheme::DefaultText));
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText("SETTINGS", layout.header.withTrimmedRight(40.0f), juce::Justification::centredLeft);

    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.5f));
    g.drawHorizontalLine((int) layout.rule.getY(), layout.rule.getX(), layout.rule.getRight());

    // --- AUDIO OUTPUT: real and functional. deviceSelector (a child
    // component) draws its own pill + refresh row inside layout.deviceSelector. ---
    drawSectionHeader(g, layout.audioOutputLabel, "AUDIO OUTPUT");
    drawRow(g, layout.sampleRateRow, "Sample rate",
            sampleRate > 0.0 ? juce::String(sampleRate, 0) + " Hz" : juce::String("-"), false);

    // --- DISPLAY: real and functional (F11 toggles fullscreen; this row
    // does the same thing via click). ---
    drawSectionHeader(g, layout.displayLabel, "DISPLAY");
    if (fullscreenRowHover)
    {
        g.setColour(juce::Colour(PulseTheme::SurfaceRaised));
        g.fillRoundedRectangle(layout.fullscreenRow.expanded(4.0f, 0.0f), 4.0f);
    }
    drawRow(g, layout.fullscreenRow, "Fullscreen", "F11", false);

    // --- VISUALIZER APPEARANCE: real and functional. Sliders are child
    // components drawing themselves inside layout.sliders; just the section
    // label is ours to draw. ---
    drawSectionHeader(g, layout.appearanceLabel, "VISUALIZER APPEARANCE");

    // --- PERFORMANCE: still a stub. ---
    drawSectionHeader(g, layout.performanceLabel, "PERFORMANCE  \u00b7  coming soon");
    drawRow(g, layout.perfRow1, "FPS limit",       "60 FPS", true);
    drawRow(g, layout.perfRow2, "Render quality",  "High", true);
    drawRow(g, layout.perfRow3, "Show goniometer", "On", true);
}
