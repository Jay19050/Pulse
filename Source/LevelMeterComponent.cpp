#include "LevelMeterComponent.h"
#include "PulseTheme.h"
#include <cmath>

namespace
{
    constexpr float kMinDb = -60.0f;
}

LevelMeterComponent::LevelMeterComponent()
{
    setOpaque(false);
    startTimerHz(30);
}

void LevelMeterComponent::pushStereo(const float* leftData, const float* rightData, int numSamples)
{
    if (leftData == nullptr || numSamples <= 0)
        return;

    float peakL = 0.0f, peakR = 0.0f;
    double sumSquaresL = 0.0, sumSquaresR = 0.0;

    for (int i = 0; i < numSamples; ++i)
    {
        const float l = juce::jlimit(-1.0f, 1.0f, leftData[i]);
        const float r = rightData != nullptr ? juce::jlimit(-1.0f, 1.0f, rightData[i]) : l;

        peakL = juce::jmax(peakL, std::abs(l));
        peakR = juce::jmax(peakR, std::abs(r));

        sumSquaresL += static_cast<double>(l) * l;
        sumSquaresR += static_cast<double>(r) * r;
    }

    const juce::ScopedLock lock(dataLock);

    left.peak  = peakL;
    right.peak = peakR;
    left.rms   = static_cast<float>(std::sqrt(sumSquaresL / numSamples));
    right.rms  = static_cast<float>(std::sqrt(sumSquaresR / numSamples));
}

void LevelMeterComponent::setActive(bool shouldBeActive)
{
    active = shouldBeActive;
}

float LevelMeterComponent::linearToNormalized(float linear)
{
    const float db = 20.0f * std::log10(juce::jmax(linear, 1.0e-6f));
    return juce::jlimit(0.0f, 1.0f, (db - kMinDb) / -kMinDb);
}

void LevelMeterComponent::timerCallback()
{
    const juce::ScopedLock lock(dataLock);

    auto updateChannel = [](ChannelLevels& ch, bool isActive)
    {
        const float rmsNorm  = linearToNormalized(ch.rms);
        const float peakNorm = linearToNormalized(ch.peak);

        // RMS bar: gentle smoothing both ways.
        ch.displayedRms += (rmsNorm - ch.displayedRms) * 0.35f;

        // Peak bar: instant attack, fast-ish release.
        if (peakNorm > ch.displayedPeak)
            ch.displayedPeak = peakNorm;
        else
            ch.displayedPeak *= 0.85f;

        // Peak-hold tick: instant attack, very slow release - the classic
        // "loudest thing that happened recently" indicator.
        if (peakNorm > ch.peakHold)
            ch.peakHold = peakNorm;
        else
            ch.peakHold *= 0.97f;

        if (! isActive)
        {
            ch.displayedRms  *= 0.85f;
            ch.displayedPeak *= 0.85f;
        }
    };

    updateChannel(left, active);
    updateChannel(right, active);
}

void LevelMeterComponent::drawChannel(juce::Graphics& g, juce::Rectangle<float> area,
                                      const ChannelLevels& levels, const juce::String& label) const
{
    auto labelArea = area.removeFromTop(14.0f);

    g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(0.7f));
    g.setFont(11.0f);
    g.drawText(label, labelArea, juce::Justification::centredLeft);

    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.4f));
    g.drawRoundedRectangle(area, 3.0f, 1.0f);

    auto fillArea = area.reduced(2.0f);

    // RMS as a solid filled bar.
    const float rmsWidth = levels.displayedRms * fillArea.getWidth();
    g.setColour(juce::Colour(PulseTheme::Accent).withAlpha(0.75f));
    g.fillRoundedRectangle(fillArea.withWidth(rmsWidth), 2.0f);

    // Peak as a thin vertical line on top - orange once it's effectively clipping.
    const float peakX   = fillArea.getX() + levels.displayedPeak * fillArea.getWidth();
    const bool  clipping = levels.peak >= 0.977f; // approx -0.1 dBFS

    g.setColour(clipping ? juce::Colours::orangered
                         : juce::Colour(PulseTheme::DefaultText).withAlpha(1.0f));
    g.drawLine(peakX, fillArea.getY(), peakX, fillArea.getBottom(), 2.0f);

    g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(0.85f));
    g.setFont(10.0f);

    const float peakDb = 20.0f * std::log10(juce::jmax(levels.peak, 1.0e-6f));
    g.drawText(juce::String(peakDb, 1) + " dB", area, juce::Justification::centredRight);
}

void LevelMeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(PulseTheme::WidgetBackground));
    g.fillRoundedRectangle(bounds, 6.0f);

    auto area = bounds.reduced(10.0f);

    const juce::ScopedLock lock(dataLock);

    drawChannel(g, area.removeFromTop(38.0f), left, "L");
    area.removeFromTop(6.0f);
    drawChannel(g, area.removeFromTop(38.0f), right, "R");
}
