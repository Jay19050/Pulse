#include "ModeWaveform.h"
#include "PulseTheme.h"

ModeWaveform::ModeWaveform()
{
    setOpaque(false);
}

void ModeWaveform::updateData(const SpectrumAnalyzer::Snapshot& snapshot, const std::vector<float>& waveform)
{
    samples = waveform;
    active = snapshot.active;
}

void ModeWaveform::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(PulseTheme::WidgetBackground));
    g.fillRoundedRectangle(bounds, 6.0f);

    const auto plot = bounds.reduced(14.0f);

    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.35f));
    g.drawHorizontalLine(static_cast<int>(plot.getCentreY()), plot.getX(), plot.getRight());

    if (samples.empty())
        return;

    const juce::Colour accent(PulseTheme::Accent);
    const int n = (int) samples.size();

    // One vertical min/max column per pixel of width, so the trace stays
    // sharp instead of aliasing when there are many more samples than
    // pixels (the common case: ~1000+ samples across ~1000px).
    const int columns = juce::jmax(1, (int) plot.getWidth());

    juce::Path fillPath;
    bool started = false;

    for (int col = 0; col < columns; ++col)
    {
        const int i0 = (col * n) / columns;
        const int i1 = juce::jmax(i0 + 1, ((col + 1) * n) / columns);

        float lo = 1.0f, hi = -1.0f;
        for (int i = i0; i < juce::jmin(i1, n); ++i)
        {
            lo = juce::jmin(lo, samples[(size_t) i]);
            hi = juce::jmax(hi, samples[(size_t) i]);
        }
        if (hi < lo) { lo = 0.0f; hi = 0.0f; }

        const float x  = plot.getX() + static_cast<float>(col);
        const float yHi = plot.getCentreY() - hi * plot.getHeight() * 0.5f;
        const float yLo = plot.getCentreY() - lo * plot.getHeight() * 0.5f;

        if (!started)
        {
            fillPath.startNewSubPath(x, yHi);
            started = true;
        }
        else
        {
            fillPath.lineTo(x, yHi);
        }
        fillPath.lineTo(x, yLo);
    }

    g.setColour(accent.withAlpha(active ? 0.95f : 0.35f));
    g.strokePath(fillPath, juce::PathStrokeType(1.4f));
}
