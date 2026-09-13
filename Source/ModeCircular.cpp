#include "ModeCircular.h"
#include "PulseTheme.h"
#include <cmath>

ModeCircular::ModeCircular()
{
    setOpaque(false);
}

void ModeCircular::updateData(const SpectrumAnalyzer::Snapshot& newSnapshot, const std::vector<float>&)
{
    snapshot = newSnapshot;
}

void ModeCircular::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    PulseTheme::panelBackground(g, bounds);

    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float outerR = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 16.0f;
    const float innerR = outerR * 0.28f;

    if (outerR <= 4.0f)
        return;

    const juce::Colour accent(PulseTheme::Accent);

    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.3f));
    g.drawEllipse(cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f, 1.0f);

    constexpr int n = SpectrumAnalyzer::spectrumPoints;
    // Skip points to keep ray count readable (one ray every ~2 degrees).
    constexpr int step = 4;

    for (int i = 0; i < n; i += step)
    {
        const float t = static_cast<float>(i) / static_cast<float>(n - 1);
        const float angle = t * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;

        const float v = juce::jlimit(0.0f, 1.0f, snapshot.fast[(size_t) i]);
        const float rayLen = v * (outerR - innerR);

        const float x0 = cx + innerR * std::cos(angle);
        const float y0 = cy + innerR * std::sin(angle);
        const float x1 = cx + (innerR + rayLen) * std::cos(angle);
        const float y1 = cy + (innerR + rayLen) * std::sin(angle);

        g.setColour(accent.withAlpha(juce::jmap(v, 0.0f, 1.0f, 0.25f, 0.95f) * (snapshot.active ? 1.0f : 0.3f)));
        g.drawLine(x0, y0, x1, y1, 2.0f);
    }

    // Peak-hold ring, drawn as a faint dotted outline at each ray's held ceiling.
    juce::Path peakRing;
    for (int i = 0; i <= n; i += step)
    {
        const int idx = juce::jmin(i, n - 1);
        const float t = static_cast<float>(idx) / static_cast<float>(n - 1);
        const float angle = t * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
        const float v = juce::jlimit(0.0f, 1.0f, snapshot.peak[(size_t) idx]);
        const float r = innerR + v * (outerR - innerR);

        const auto pt = juce::Point<float>(cx + r * std::cos(angle), cy + r * std::sin(angle));
        if (i == 0) peakRing.startNewSubPath(pt); else peakRing.lineTo(pt);
    }
    g.setColour(juce::Colour(PulseTheme::PeakHold).withAlpha(0.5f));
    g.strokePath(peakRing, juce::PathStrokeType(1.0f));

    // Centre glow, brightens with overall level.
    g.setColour(accent.withAlpha(0.15f + snapshot.level * 0.5f));
    g.fillEllipse(cx - innerR * 0.7f, cy - innerR * 0.7f, innerR * 1.4f, innerR * 1.4f);
}
