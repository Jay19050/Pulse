#include "ModeMirror.h"
#include "PulseTheme.h"

ModeMirror::ModeMirror()
{
    setOpaque(false);
}

void ModeMirror::updateData(const SpectrumAnalyzer::Snapshot& newSnapshot, const std::vector<float>&)
{
    snapshot = newSnapshot;
}

void ModeMirror::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    PulseTheme::panelBackground(g, bounds);

    const auto plot = bounds.reduced(14.0f);
    const float midY = plot.getCentreY();

    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.35f));
    g.drawHorizontalLine(static_cast<int>(midY), plot.getX(), plot.getRight());

    constexpr int n = SpectrumAnalyzer::spectrumPoints;

    // Aggregate the 512-point spectrum down into dense, evenly spaced bars -
    // a bar per ~4px reads as a proper bar-graph rather than 512 hairlines.
    const int numBars = juce::jmax(1, (int) (plot.getWidth() / 4.0f));
    const float barSpan = plot.getWidth() / static_cast<float>(numBars);
    const float barWidth = juce::jmax(1.0f, barSpan * 0.72f);

    const juce::Colour accent(PulseTheme::Accent);
    const float alpha = snapshot.active ? 1.0f : 0.35f;

    for (int b = 0; b < numBars; ++b)
    {
        const int i0 = (b * n) / numBars;
        const int i1 = juce::jmax(i0 + 1, ((b + 1) * n) / numBars);

        float v = 0.0f;
        for (int i = i0; i < juce::jmin(i1, n); ++i)
            v = juce::jmax(v, snapshot.fast[(size_t) i]);

        v = juce::jlimit(0.0f, 1.0f, v);

        const float x = plot.getX() + static_cast<float>(b) * barSpan + (barSpan - barWidth) * 0.5f;
        const float halfHeight = v * plot.getHeight() * 0.5f;

        g.setColour(accent.withAlpha(juce::jmap(v, 0.0f, 1.0f, 0.35f, 0.95f) * alpha));
        g.fillRect(x, midY - halfHeight, barWidth, halfHeight * 2.0f);
    }
}
