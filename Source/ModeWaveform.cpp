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

    PulseTheme::panelBackground(g, bounds, 6.0f, PulseTheme::backgroundColourFor(appearance.background));

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

    if ((int) hiFollowers.size() != columns)
    {
        hiFollowers.assign((size_t) columns, {});
        loFollowers.assign((size_t) columns, {});
    }

    const float sensitivity = juce::jlimit(0.4f, 2.0f, appearance.sensitivity);

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

        // Sensitivity scales the swing; Smoothing damps column-to-column
        // (i.e. frame-to-frame, since columns are re-sampled fresh each
        // paint) jumps in that swing, independent of the waveform's own
        // timing/shape - "Smooth" mode doesn't distort what the signal
        // looks like, just how eagerly the display chases sudden changes.
        hi = hiFollowers[(size_t) col].advance(juce::jlimit(-1.0f, 1.0f, hi * sensitivity), appearance.smoothing);
        lo = loFollowers[(size_t) col].advance(juce::jlimit(-1.0f, 1.0f, lo * sensitivity), appearance.smoothing);

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

    const float brightness = juce::jlimit(0.3f, 1.0f, appearance.brightness);
    g.setColour(accent.withAlpha((active ? 0.95f : 0.35f) * brightness));
    g.strokePath(fillPath, juce::PathStrokeType(1.4f));
}
