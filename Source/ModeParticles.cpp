#include "ModeParticles.h"
#include "PulseTheme.h"
#include <cmath>

namespace
{
    constexpr int kDustCount   = 320;
    constexpr int kFlowCount   = 420;
    constexpr int kEnergyCount = 160;
    constexpr int kSparkCount  = 180;
}

ModeParticles::ModeParticles()
{
    setOpaque(false);
    buildParticles();
}

void ModeParticles::buildParticles()
{
    juce::Random rng(7); // fixed seed: layout is stable across runs/resizes

    particles.clear();
    particles.reserve((size_t) (kDustCount + kFlowCount + kEnergyCount + kSparkCount));

    auto addLayer = [&](int count, Layer layer)
    {
        for (int i = 0; i < count; ++i)
        {
            Particle p;
            const float even = static_cast<float>(i) / static_cast<float>(juce::jmax(1, count - 1));
            // Small jitter off the even spacing so a layer doesn't read as a
            // grid - the brief explicitly calls out "avoid grid-like
            // patterns".
            const float jitter = (rng.nextFloat() - 0.5f) * (1.4f / static_cast<float>(juce::jmax(1, count)));
            p.xFraction   = juce::jlimit(0.0f, 1.0f, even + jitter);
            p.phase       = rng.nextFloat() * juce::MathConstants<float>::twoPi;
            p.speed       = 0.6f + rng.nextFloat() * 0.8f;
            p.scatterSeed = rng.nextFloat() * 2.0f - 1.0f;
            p.sizeSeed    = rng.nextFloat();
            p.layer       = layer;
            particles.push_back(p);
        }
    };

    addLayer(kDustCount,   Layer::Dust);
    addLayer(kFlowCount,   Layer::Flow);
    addLayer(kEnergyCount, Layer::Energy);
    addLayer(kSparkCount,  Layer::Spark);
}

float ModeParticles::sampleSpectrum(const std::array<float, SpectrumAnalyzer::spectrumPoints>& values, float t)
{
    const float pos = juce::jlimit(0.0f, 1.0f, t) * static_cast<float>(SpectrumAnalyzer::spectrumPoints - 1);
    const int i0 = static_cast<int>(pos);
    const int i1 = juce::jmin(i0 + 1, SpectrumAnalyzer::spectrumPoints - 1);
    const float frac = pos - static_cast<float>(i0);
    return values[(size_t) i0] + (values[(size_t) i1] - values[(size_t) i0]) * frac;
}

void ModeParticles::updateData(const SpectrumAnalyzer::Snapshot& newSnapshot, const std::vector<float>&)
{
    snapshot = newSnapshot;

    // Bass/mid/high bands, in index-space of the existing log-frequency
    // mapping (SpectrumAnalyzer already distributes its 512 points
    // perceptually across the audible range, so "first ~8%" is sub-bass/
    // bass, "15-55%" is midrange, "top ~18%" is high treble - matching the
    // log mapping every other mode already relies on).
    constexpr int n = SpectrumAnalyzer::spectrumPoints;
    constexpr int bassEnd   = n * 8  / 100;
    constexpr int midStart  = n * 15 / 100;
    constexpr int midEnd    = n * 55 / 100;
    constexpr int highStart = n * 82 / 100;

    float bassRaw = 0.0f, midRaw = 0.0f, highRaw = 0.0f;
    for (int i = 0; i < bassEnd; ++i)
        bassRaw = juce::jmax(bassRaw, snapshot.fast[(size_t) i]);
    for (int i = midStart; i < midEnd; ++i)
        midRaw += snapshot.fast[(size_t) i];
    midRaw /= static_cast<float>(juce::jmax(1, midEnd - midStart));
    for (int i = highStart; i < n; ++i)
        highRaw = juce::jmax(highRaw, snapshot.fast[(size_t) i]);

    const float sens = appearance.sensitivity;
    bassEnergy   = bassFollower.advance(juce::jlimit(0.0f, 1.0f, bassRaw * sens), appearance.smoothing);
    midEnergy    = midFollower.advance(juce::jlimit(0.0f, 1.0f, midRaw * sens), appearance.smoothing);
    highEnergy   = highFollower.advance(juce::jlimit(0.0f, 1.0f, highRaw * sens), appearance.smoothing);
    overallLevel = levelFollower.advance(juce::jlimit(0.0f, 1.0f, snapshot.level * sens), appearance.smoothing);

    // The entire idle/no-fake-movement guarantee lives here: flowPhase only
    // moves while snapshot.active is true (SpectrumAnalyzer's own, already-
    // debounced activity flag), and even then at a rate tied to the current
    // level. At true silence this line is a no-op every tick, so every wave,
    // drift, and vortex calculation downstream simply stops.
    flowPhase += snapshot.active ? (0.01f + overallLevel * 0.05f) : 0.0f;
}

juce::Point<float> ModeParticles::computePosition(const Particle& p, juce::Rectangle<float> plot, float phaseOffset) const
{
    const float t = flowPhase - phaseOffset;

    float xFrac = std::fmod(p.xFraction + t * 0.15f * p.speed, 1.0f);
    if (xFrac < 0.0f)
        xFrac += 1.0f;

    const float x = plot.getX() + xFrac * plot.getWidth();
    const float centerY = plot.getCentreY();

    // Two layered waves at different spatial/temporal frequencies, per
    // particle phase-offset, so the field reads as organic flow rather than
    // one rigid sine ripple.
    const float wave1 = std::sin(xFrac * juce::MathConstants<float>::twoPi * 2.3f + t * 0.9f + p.phase);
    const float wave2 = std::sin(xFrac * juce::MathConstants<float>::twoPi * 4.7f - t * 1.6f + p.phase * 1.7f);

    switch (p.layer)
    {
        case Layer::Energy:
        {
            // Curves around a central vortex whose *strength* is driven by
            // overall level - not a free-running rotation. At overallLevel
            // == 0 this reduces to a fixed point (angle/radius stop
            // changing because t is frozen too), matching every other
            // layer's idle behaviour.
            const float dx = xFrac - 0.5f;
            const float baseY = p.scatterSeed * 0.22f;
            const float dy = baseY;

            const float dist = std::sqrt(dx * dx + dy * dy);
            const float vortexPull = juce::jlimit(0.0f, 1.0f, overallLevel);
            const float swirl = vortexPull * (1.0f - juce::jlimit(0.0f, 1.0f, dist * 1.4f)) * 2.4f;

            const float angle = std::atan2(dy, dx) + swirl + t * 0.12f * vortexPull;
            const float radius = juce::jmax(0.1f, dist) * (1.0f - vortexPull * 0.25f);

            const float ex = juce::jlimit(0.0f, 1.0f, 0.5f + std::cos(angle) * radius);
            const float ey = std::sin(angle) * radius;

            return { plot.getX() + ex * plot.getWidth(), centerY + ey * plot.getHeight() * 0.9f };
        }

        case Layer::Dust:
        {
            const float y = centerY + p.scatterSeed * plot.getHeight() * 0.42f
                                     + wave1 * plot.getHeight() * 0.025f * (0.3f + bassEnergy);
            return { x, y };
        }

        case Layer::Spark:
        {
            const float y = centerY + p.scatterSeed * plot.getHeight() * 0.38f
                                     + wave2 * plot.getHeight() * 0.05f * highEnergy;
            return { x, y };
        }

        case Layer::Flow:
        default:
        {
            const float spectrumV = sampleSpectrum(snapshot.fast, xFrac) * appearance.sensitivity;
            const float amplitude = plot.getHeight() * 0.28f * (0.35f + midEnergy);
            const float y = centerY + (wave1 * 0.6f + wave2 * 0.4f) * amplitude
                                     + juce::jlimit(-1.0f, 1.0f, spectrumV - 0.5f) * plot.getHeight() * 0.45f
                                     + p.scatterSeed * plot.getHeight() * 0.05f;
            return { x, y };
        }
    }
}

void ModeParticles::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    PulseTheme::panelBackground(g, bounds, 6.0f, PulseTheme::backgroundColourFor(appearance.background));

    const auto plot = bounds.reduced(14.0f);
    if (plot.getWidth() <= 0.0f || plot.getHeight() <= 0.0f)
        return;

    const juce::Colour accent(PulseTheme::Accent);
    const juce::Colour hot = accent.brighter(0.5f);
    const float brightness = juce::jlimit(0.3f, 1.6f, appearance.brightness);
    const float activeFade = snapshot.active ? 1.0f : 0.35f;
    const float peakBoost = juce::jlimit(0.3f, 2.0f, appearance.peakIntensity);

    for (const auto& p : particles)
    {
        switch (p.layer)
        {
            case Layer::Dust:
            {
                const auto pos = computePosition(p, plot, 0.0f);
                const float size = 1.0f + p.sizeSeed * 1.2f;
                g.setColour(accent.withAlpha(juce::jlimit(0.02f, 0.5f, 0.06f * brightness) * activeFade));
                g.fillEllipse(pos.x - size * 0.5f, pos.y - size * 0.5f, size, size);
                break;
            }

            case Layer::Flow:
            {
                // Two-step trail: current position plus one fainter "ghost"
                // slightly behind it - both from computePosition() at
                // different phase offsets, no stored history.
                for (int step = 1; step >= 0; --step)
                {
                    const auto pos = computePosition(p, plot, (float) step * 0.35f);
                    const float v = juce::jlimit(0.0f, 1.0f, sampleSpectrum(snapshot.fast, p.xFraction) * appearance.sensitivity);
                    const float size = (2.0f + v * 2.6f + p.sizeSeed) * (step == 0 ? 1.0f : 0.7f);
                    const float alpha = juce::jlimit(0.02f, 0.95f, (0.12f + v * 0.65f) * brightness * activeFade
                                                                     * (step == 0 ? 1.0f : 0.35f));

                    g.setColour(accent.withAlpha(alpha));
                    g.fillEllipse(pos.x - size * 0.5f, pos.y - size * 0.5f, size, size);
                }
                break;
            }

            case Layer::Energy:
            {
                for (int step = 1; step >= 0; --step)
                {
                    const auto pos = computePosition(p, plot, (float) step * 0.3f);
                    const float v = juce::jlimit(0.0f, 1.0f, overallLevel + p.sizeSeed * 0.2f);
                    const float size = 2.0f + (3.5f + v * 4.5f) * (step == 0 ? 1.0f : 0.65f) * peakBoost * 0.7f;
                    const float coreAlpha = juce::jlimit(0.05f, 1.0f, (0.25f + v * 0.7f) * brightness * activeFade
                                                                        * (step == 0 ? 1.0f : 0.3f));

                    if (step == 0)
                    {
                        // Cheap glow: a larger, low-alpha ellipse behind the
                        // bright core - no blur filter, just layered alpha.
                        const float haloSize = size * 2.4f;
                        g.setColour(hot.withAlpha(coreAlpha * 0.22f));
                        g.fillEllipse(pos.x - haloSize * 0.5f, pos.y - haloSize * 0.5f, haloSize, haloSize);
                    }

                    g.setColour(hot.withAlpha(coreAlpha));
                    g.fillEllipse(pos.x - size * 0.5f, pos.y - size * 0.5f, size, size);
                }
                break;
            }

            case Layer::Spark:
            {
                // Sparks sample the genuine HIGH-frequency region of the
                // real spectrum, regardless of their visual xFraction -
                // that's what makes them flicker with actual treble/
                // transient content rather than just being small flow dots.
                const int idx = juce::jlimit(0, SpectrumAnalyzer::spectrumPoints - 1,
                                              (int) (SpectrumAnalyzer::spectrumPoints * (0.82f + p.xFraction * 0.18f)));
                const float v = juce::jlimit(0.0f, 1.0f, snapshot.fast[(size_t) idx] * appearance.sensitivity);

                constexpr float sparkThreshold = 0.32f;
                if (v < sparkThreshold)
                    break;

                const auto pos = computePosition(p, plot, 0.0f);
                const float strength = (v - sparkThreshold) / (1.0f - sparkThreshold);
                const float size = 1.0f + (1.5f + strength * 2.5f) * peakBoost * 0.6f;

                g.setColour(hot.withAlpha(juce::jlimit(0.1f, 1.0f, strength * brightness * activeFade)));
                g.fillEllipse(pos.x - size * 0.5f, pos.y - size * 0.5f, size, size);
                break;
            }
        }
    }
}
