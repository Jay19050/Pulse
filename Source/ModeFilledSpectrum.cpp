#include "ModeFilledSpectrum.h"
#include "PulseTheme.h"
#include "CurveUtils.h"

ModeFilledSpectrum::ModeFilledSpectrum()
{
    setOpaque(false);
}

void ModeFilledSpectrum::updateData(const SpectrumAnalyzer::Snapshot& newSnapshot, const std::vector<float>&)
{
    snapshot = newSnapshot;

    for (size_t i = 0; i < displayed.size(); ++i)
    {
        const float target = juce::jlimit(0.0f, 1.0f, snapshot.fast[i] * appearance.sensitivity);
        displayed[i] = followers[i].advance(target, appearance.smoothing);
    }
}

void ModeFilledSpectrum::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    PulseTheme::panelBackground(g, bounds, 6.0f, PulseTheme::backgroundColourFor(appearance.background));

    const auto plot = bounds.reduced(14.0f);

    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.3f));
    for (int i = 1; i < 4; ++i)
    {
        const float y = plot.getY() + plot.getHeight() * (static_cast<float>(i) / 4.0f);
        g.drawHorizontalLine(static_cast<int>(y), plot.getX(), plot.getRight());
    }

    auto mapPoint = [&](int index, float value, juce::Rectangle<float> b)
    {
        return CurveUtils::cartesianMap(index, value, b);
    };

    const auto stroke = CurveUtils::buildSmoothCurve(displayed, plot, mapPoint);

    juce::Path filled = stroke;
    filled.lineTo(plot.getRight(), plot.getBottom());
    filled.lineTo(plot.getX(), plot.getBottom());
    filled.closeSubPath();

    const juce::Colour accent(PulseTheme::Accent);
    const float b = juce::jlimit(0.3f, 1.0f, appearance.brightness);

    g.setGradientFill(juce::ColourGradient(accent.withAlpha(0.85f * b), plot.getX(), plot.getY(),
                                            accent.withAlpha(0.08f * b), plot.getX(), plot.getBottom(),
                                            false));
    g.fillPath(filled);

    g.setColour(accent.withAlpha((snapshot.active ? 1.0f : 0.4f) * juce::jlimit(0.3f, 1.0f, appearance.peakIntensity)));
    g.strokePath(stroke, juce::PathStrokeType(2.0f));
}
