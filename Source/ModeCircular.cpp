#include "ModeCircular.h"
#include "PulseTheme.h"
#include "BandEnergy.h"
#include <cmath>

ModeCircular::ModeCircular()
{
    setOpaque(false);
}

void ModeCircular::updateData(const SpectrumAnalyzer::Snapshot& newSnapshot, const std::vector<float>&)
{
    snapshot = newSnapshot;

    for (size_t i = 0; i < displayed.size(); ++i)
    {
        const float target = juce::jlimit(0.0f, 1.0f, newSnapshot.fast[i] * appearance.sensitivity);
        displayed[i] = followers[i].advance(target, appearance.smoothing);
    }

    const auto bands = computeBandEnergy(newSnapshot, appearance.sensitivity);
    bassEnergy = bassFollower.advance(bands.bass, appearance.smoothing);
}

void ModeCircular::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    PulseTheme::panelBackground(g, bounds, 6.0f, PulseTheme::backgroundColourFor(appearance.background));

    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    // Bass gives the whole ring a subtle overall radius pulse - "bass
    // influences the overall radius", per the brief - kept gentle so it
    // reads as breathing rather than throbbing.
    const float bassPulse = 1.0f + bassEnergy * 0.05f;
    const float outerR = (juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 16.0f) * bassPulse;
    const float innerR = outerR * 0.28f;

    if (outerR <= 4.0f)
        return;

    const juce::Colour accent(PulseTheme::Accent);
    const float brightness = juce::jlimit(0.3f, 1.6f, appearance.brightness);
    const float peakAlpha = juce::jlimit(0.0f, 1.0f, appearance.peakIntensity * 0.5f);
    const float activeFade = snapshot.active ? 1.0f : 0.3f;

    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.3f));
    g.drawEllipse(cx - innerR, cy - innerR, innerR * 2.0f, innerR * 2.0f, 1.0f);

    constexpr int n = SpectrumAnalyzer::spectrumPoints;

    // Inner ring: fine, dense HIGH-frequency spikes packed close to the
    // centre disc - "fine spikes/detail" from actual treble content, not a
    // decorative texture. Thin, bright, and short, so it reads as detail
    // rather than competing with the main ray ring.
    constexpr int highStart = n * 82 / 100;
    constexpr int spikeStep = 2;
    const float spikeBaseR = innerR * 1.08f;
    const float spikeMaxLen = (outerR - innerR) * 0.22f;

    for (int i = highStart; i < n; i += spikeStep)
    {
        const float t = static_cast<float>(i) / static_cast<float>(n - 1);
        const float angle = t * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
        const float v = displayed[(size_t) i];

        if (v < 0.08f)
            continue;

        const float len = v * spikeMaxLen;
        const float x0 = cx + spikeBaseR * std::cos(angle);
        const float y0 = cy + spikeBaseR * std::sin(angle);
        const float x1 = cx + (spikeBaseR + len) * std::cos(angle);
        const float y1 = cy + (spikeBaseR + len) * std::sin(angle);

        g.setColour(juce::Colours::white.withAlpha(juce::jlimit(0.0f, 0.8f, v * 0.9f) * brightness * activeFade));
        g.drawLine(x0, y0, x1, y1, 1.0f);
    }

    // Main ray ring - one ray every ~2 degrees.
    constexpr int step = 4;

    for (int i = 0; i < n; i += step)
    {
        const float t = static_cast<float>(i) / static_cast<float>(n - 1);
        const float angle = t * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;

        const float v = displayed[(size_t) i];
        const float rayLen = v * (outerR - innerR);

        const float x0 = cx + innerR * std::cos(angle);
        const float y0 = cy + innerR * std::sin(angle);
        const float x1 = cx + (innerR + rayLen) * std::cos(angle);
        const float y1 = cy + (innerR + rayLen) * std::sin(angle);

        g.setColour(accent.withAlpha(juce::jmap(v, 0.0f, 1.0f, 0.25f, 0.95f) * brightness * activeFade));
        g.drawLine(x0, y0, x1, y1, 2.0f);
    }

    // Peak-hold ring, drawn as a faint dotted outline at each ray's held ceiling.
    juce::Path peakRing;
    for (int i = 0; i <= n; i += step)
    {
        const int idx = juce::jmin(i, n - 1);
        const float t = static_cast<float>(idx) / static_cast<float>(n - 1);
        const float angle = t * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
        const float v = juce::jlimit(0.0f, 1.0f, snapshot.peak[(size_t) idx] * appearance.sensitivity);
        const float r = innerR + v * (outerR - innerR);

        const auto pt = juce::Point<float>(cx + r * std::cos(angle), cy + r * std::sin(angle));
        if (i == 0) peakRing.startNewSubPath(pt); else peakRing.lineTo(pt);
    }
    g.setColour(juce::Colour(PulseTheme::PeakHold).withAlpha(peakAlpha));
    g.strokePath(peakRing, juce::PathStrokeType(1.0f));

    // Centre glow, brightens with overall level.
    g.setColour(accent.withAlpha((0.15f + snapshot.level * appearance.sensitivity * 0.5f) * brightness));
    g.fillEllipse(cx - innerR * 0.7f, cy - innerR * 0.7f, innerR * 1.4f, innerR * 1.4f);
}
