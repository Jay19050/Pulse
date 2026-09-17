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

    // Back is always notably smoother than the user's own Smoothing setting
    // (a broad, slow-moving mass), front always notably sharper (near-raw
    // transient detail) - that's what makes them read as distinct layers
    // rather than three copies of the same curve at different smoothing.
    const float backSmoothing  = juce::jlimit(0.0f, 0.97f, appearance.smoothing * 0.5f + 0.55f);
    const float frontSmoothing = juce::jlimit(0.0f, 0.9f, appearance.smoothing * 0.35f);

    for (size_t i = 0; i < midLayer.size(); ++i)
    {
        const float target = juce::jlimit(0.0f, 1.0f, snapshot.fast[i] * appearance.sensitivity);
        backLayer[i]  = backFollowers[i].advance(target, backSmoothing);
        midLayer[i]   = midFollowers[i].advance(target, appearance.smoothing);
        frontLayer[i] = frontFollowers[i].advance(target, frontSmoothing);
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

    const juce::Colour accent(PulseTheme::Accent);
    const float brightness = juce::jlimit(0.3f, 1.6f, appearance.brightness);
    const float activeFade = snapshot.active ? 1.0f : 0.4f;

    auto fillUnderCurve = [&](const juce::Path& stroke, juce::Colour top, juce::Colour bottom)
    {
        juce::Path filled = stroke;
        filled.lineTo(plot.getRight(), plot.getBottom());
        filled.lineTo(plot.getX(), plot.getBottom());
        filled.closeSubPath();

        g.setGradientFill(juce::ColourGradient(top, plot.getX(), plot.getY(),
                                                bottom, plot.getX(), plot.getBottom(),
                                                false));
        g.fillPath(filled);
    };

    // BACK: a faint, heavily-smoothed silhouette - the broad mass of the
    // sound, sitting behind everything else.
    const auto backStroke = CurveUtils::buildSmoothCurve(backLayer, plot, mapPoint);
    fillUnderCurve(backStroke, accent.withAlpha(0.30f * brightness * activeFade),
                               accent.withAlpha(0.02f * brightness * activeFade));

    // MIDDLE: the main filled body - unchanged from before, still the
    // dominant shape.
    const auto midStroke = CurveUtils::buildSmoothCurve(midLayer, plot, mapPoint);
    fillUnderCurve(midStroke, accent.withAlpha(0.85f * brightness * activeFade),
                              accent.withAlpha(0.08f * brightness * activeFade));

    // FRONT: a bright, lightly-smoothed trace riding the top edge - crisp
    // transient detail, boosted by Peak Intensity.
    const auto frontStroke = CurveUtils::buildSmoothCurve(frontLayer, plot, mapPoint);
    const float peakAlpha = juce::jlimit(0.0f, 1.0f, appearance.peakIntensity);
    g.setColour(juce::Colours::white.withAlpha(0.55f * peakAlpha * activeFade));
    g.strokePath(frontStroke, juce::PathStrokeType(1.6f));

    g.setColour(accent.withAlpha(activeFade * juce::jlimit(0.3f, 1.0f, appearance.peakIntensity)));
    g.strokePath(midStroke, juce::PathStrokeType(2.0f));
}
