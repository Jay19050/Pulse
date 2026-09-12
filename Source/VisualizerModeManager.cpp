#include "VisualizerModeManager.h"
#include "PulseTheme.h"
#include "VisualizerComponent.h"
#include "ModeFilledSpectrum.h"
#include "ModeWaveform.h"
#include "ModeCircular.h"
#include "ModeMirror.h"
#include "ModeParticles.h"

VisualizerModeManager::VisualizerModeManager()
{
    setOpaque(false);

    modes.push_back(std::make_unique<VisualizerComponent>());
    modes.push_back(std::make_unique<ModeFilledSpectrum>());
    modes.push_back(std::make_unique<ModeWaveform>());
    modes.push_back(std::make_unique<ModeCircular>());
    modes.push_back(std::make_unique<ModeMirror>());
    modes.push_back(std::make_unique<ModeParticles>());

    // Spectrum mode owns the "click plot to clear peak hold" gesture.
    if (auto* spectrum = dynamic_cast<VisualizerComponent*>(modes.front().get()))
        spectrum->onResetPeaks = [this]
        {
            if (onResetPeaks != nullptr)
                onResetPeaks();
        };

    for (auto& m : modes)
    {
        addChildComponent(*m);
        m->setVisible(false);
    }

    modes[(size_t) activeIndex]->setVisible(true);
    itemBounds.resize(modes.size());

    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void VisualizerModeManager::prepare(double sampleRate)
{
    for (auto& m : modes)
        m->prepare(sampleRate);
}

void VisualizerModeManager::updateData(
    const SpectrumAnalyzer::Snapshot& snapshot,
    const std::vector<float>& waveform)
{
    for (auto& m : modes)
        m->updateData(snapshot, waveform);
}

void VisualizerModeManager::setActiveIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(modes.size()) || index == activeIndex)
        return;

    modes[(size_t) activeIndex]->setVisible(false);

    activeIndex = index;

    modes[(size_t) activeIndex]->setVisible(true);

    repaint();
}

juce::Rectangle<float> VisualizerModeManager::selectorStripBounds() const
{
    return getLocalBounds().toFloat().removeFromTop(kStripHeight);
}

void VisualizerModeManager::resized()
{
    const auto strip = selectorStripBounds();

    juce::Font font { juce::FontOptions {}.withHeight(12.0f) };

    float x = strip.getX() + 10.0f;

    for (size_t i = 0; i < modes.size(); ++i)
    {
        const float textWidth =
    juce::GlyphArrangement::getStringWidth(
        font,
        modes[i]->getModeName());

        const float itemWidth = textWidth + 18.0f;

        itemBounds[i] =
        {
            x,
            strip.getY(),
            itemWidth,
            strip.getHeight()
        };

        x += itemWidth;
    }

    const auto content =
        getLocalBounds()
            .toFloat()
            .withTrimmedTop(kStripHeight + 4.0f);

    for (auto& m : modes)
        m->setBounds(content.toNearestInt());
}

void VisualizerModeManager::mouseMove(const juce::MouseEvent& e)
{
    int next = -1;

    for (size_t i = 0; i < itemBounds.size(); ++i)
    {
        if (itemBounds[i].contains(e.position))
        {
            next = static_cast<int>(i);
            break;
        }
    }

    if (next != hoverIndex)
    {
        hoverIndex = next;
        repaint(selectorStripBounds().getSmallestIntegerContainer());
    }
}

void VisualizerModeManager::mouseExit(const juce::MouseEvent&)
{
    if (hoverIndex != -1)
    {
        hoverIndex = -1;
        repaint(selectorStripBounds().getSmallestIntegerContainer());
    }
}

void VisualizerModeManager::mouseDown(const juce::MouseEvent& e)
{
    for (size_t i = 0; i < itemBounds.size(); ++i)
    {
        if (itemBounds[i].contains(e.position))
        {
            setActiveIndex(static_cast<int>(i));
            return;
        }
    }
}

void VisualizerModeManager::paint(juce::Graphics& g)
{
    const auto strip = selectorStripBounds();

    g.setColour(juce::Colour(PulseTheme::WidgetBackground));
    g.fillRoundedRectangle(strip, 6.0f);

    g.setFont(juce::FontOptions {}.withHeight(12.0f));

    for (size_t i = 0; i < modes.size(); ++i)
    {
        const auto item = itemBounds[i];

        const bool active =
            static_cast<int>(i) == activeIndex;

        const bool hover =
            static_cast<int>(i) == hoverIndex;

        if (hover && !active)
        {
            g.setColour(
                juce::Colour(PulseTheme::Outline)
                    .withAlpha(0.4f));

            g.fillRoundedRectangle(
                item.reduced(2.0f, 4.0f),
                4.0f);
        }

        g.setColour(
            active
                ? juce::Colour(PulseTheme::Accent)
                : juce::Colour(PulseTheme::DefaultText)
                      .withAlpha(hover ? 0.9f : 0.65f));

        g.drawText(
            modes[i]->getModeName(),
            item,
            juce::Justification::centred);

        if (active)
        {
            const float underlineY =
                item.getBottom() - 3.0f;

            g.setColour(
                juce::Colour(PulseTheme::Accent));

            g.fillRoundedRectangle(
                item.getX() + 6.0f,
                underlineY,
                item.getWidth() - 12.0f,
                2.0f,
                1.0f);
        }
    }
}