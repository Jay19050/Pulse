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
}

void ModeFilledSpectrum::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(PulseTheme::WidgetBackground));
    g.fillRoundedRectangle(bounds, 6.0f);

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

    const auto stroke = CurveUtils::buildSmoothCurve(snapshot.fast, plot, mapPoint);

    juce::Path filled = stroke;
    filled.lineTo(plot.getRight(), plot.getBottom());
    filled.lineTo(plot.getX(), plot.getBottom());
    filled.closeSubPath();

    const juce::Colour accent(PulseTheme::Accent);

    g.setGradientFill(juce::ColourGradient(accent.withAlpha(0.85f), plot.getX(), plot.getY(),
                                            accent.withAlpha(0.08f), plot.getX(), plot.getBottom(),
                                            false));
    g.fillPath(filled);

    g.setColour(accent.withAlpha(snapshot.active ? 1.0f : 0.4f));
    g.strokePath(stroke, juce::PathStrokeType(2.0f));
}
