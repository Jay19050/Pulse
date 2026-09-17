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

    // Subtle center reference line - a fixed point to read the waveform
    // against, kept faint so it never competes with the trace itself.
    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.3f));
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
        secondaryHiFollowers.assign((size_t) columns, {});
        secondaryLoFollowers.assign((size_t) columns, {});
    }

    const float sensitivity = juce::jlimit(0.4f, 2.0f, appearance.sensitivity);
    // The secondary/ghost trace is always notably smoother than the main
    // one, regardless of the user's Smoothing setting - it's meant to read
    // as a slow reference layer (the waveform's equivalent of Spectrum
    // mode's AVG trace), not a second copy of the same line.
    const float secondarySmoothing = juce::jlimit(0.0f, 0.97f, appearance.smoothing * 0.5f + 0.55f);

    juce::Path mainPath, secondaryPath;
    hiPointsBuf.clear();
    loPointsBuf.clear();
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

        const float rawHi = juce::jlimit(-1.0f, 1.0f, hi * sensitivity);
        const float rawLo = juce::jlimit(-1.0f, 1.0f, lo * sensitivity);

        // Sensitivity scales the swing; Smoothing damps column-to-column
        // (i.e. frame-to-frame, since columns are re-sampled fresh each
        // paint) jumps in that swing, independent of the waveform's own
        // timing/shape - "Smooth" mode doesn't distort what the signal
        // looks like, just how eagerly the display chases sudden changes.
        hi = hiFollowers[(size_t) col].advance(rawHi, appearance.smoothing);
        lo = loFollowers[(size_t) col].advance(rawLo, appearance.smoothing);

        const float secHi = secondaryHiFollowers[(size_t) col].advance(rawHi, secondarySmoothing);
        const float secLo = secondaryLoFollowers[(size_t) col].advance(rawLo, secondarySmoothing);

        const float x = plot.getX() + static_cast<float>(col);
        const float yHi = plot.getCentreY() - hi * plot.getHeight() * 0.5f;
        const float yLo = plot.getCentreY() - lo * plot.getHeight() * 0.5f;
        const float secYHi = plot.getCentreY() - secHi * plot.getHeight() * 0.5f;
        const float secYLo = plot.getCentreY() - secLo * plot.getHeight() * 0.5f;

        if (!started)
        {
            mainPath.startNewSubPath(x, yHi);
            secondaryPath.startNewSubPath(x, secYHi);
            started = true;
        }
        else
        {
            mainPath.lineTo(x, yHi);
            secondaryPath.lineTo(x, secYHi);
        }
        mainPath.lineTo(x, yLo);
        secondaryPath.lineTo(x, secYLo);

        hiPointsBuf.emplace_back(x, yHi);
        loPointsBuf.emplace_back(x, yLo);
    }

    const float brightness = juce::jlimit(0.3f, 1.6f, appearance.brightness);
    const float activeFade = active ? 1.0f : 0.35f;

    // Secondary/ghost trace first, so the main trace reads on top of it.
    g.setColour(juce::Colours::white.withAlpha(0.22f * brightness * activeFade));
    g.strokePath(secondaryPath, juce::PathStrokeType(1.2f));

    // Cheap glow: a wider, low-alpha stroke of the same path drawn just
    // behind the crisp main stroke - no blur filter needed.
    g.setColour(accent.withAlpha(0.18f * brightness * activeFade));
    g.strokePath(mainPath, juce::PathStrokeType(4.0f));

    // Translucent body fill: the TOP contour forward, then the BOTTOM
    // contour backward, closed into one silhouette - not mainPath itself
    // closed (that zigzags hi/lo every column and would self-intersect if
    // closed directly).
    if (! hiPointsBuf.empty())
    {
        juce::Path filled;
        filled.startNewSubPath(hiPointsBuf.front());
        for (size_t i = 1; i < hiPointsBuf.size(); ++i)
            filled.lineTo(hiPointsBuf[i]);
        for (size_t i = loPointsBuf.size(); i-- > 0; )
            filled.lineTo(loPointsBuf[i]);
        filled.closeSubPath();

        g.setGradientFill(juce::ColourGradient(accent.withAlpha(0.16f * brightness), plot.getCentreX(), plot.getY(),
                                                accent.withAlpha(0.02f * brightness), plot.getCentreX(), plot.getBottom(),
                                                false));
        g.fillPath(filled);
    }

    g.setColour(accent.withAlpha(0.95f * brightness * activeFade));
    g.strokePath(mainPath, juce::PathStrokeType(1.6f));
}
