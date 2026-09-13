#include "ModeParticles.h"
#include "PulseTheme.h"
#include <cmath>

namespace
{
    constexpr int kNumParticles = 260;
}

ModeParticles::ModeParticles()
{
    setOpaque(false);

    juce::Random rng(1);   // fixed seed: layout is stable across runs/resizes

    particles.reserve((size_t) kNumParticles);
    for (int i = 0; i < kNumParticles; ++i)
    {
        Particle p;
        p.xFraction = static_cast<float>(i) / static_cast<float>(kNumParticles - 1);
        p.seed = rng.nextFloat() * 2.0f - 1.0f;
        particles.push_back(p);
    }
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

    // Sideways drift speed is proportional to overall level - silence means
    // no drift, not a frozen frame (the per-particle scatter still responds
    // to whatever residual level there is).
    driftPhase += 0.015f + snapshot.level * 0.12f;
}

void ModeParticles::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    PulseTheme::panelBackground(g, bounds);

    const auto plot = bounds.reduced(14.0f);
    const juce::Colour accent(PulseTheme::Accent);

    for (const auto& p : particles)
    {
        // Drift each particle's sampling position sideways over time so the
        // whole field flows, rather than sampling the same bin forever.
        const float t = std::fmod(p.xFraction + driftPhase * 0.01f, 1.0f);

        const float v = juce::jlimit(0.0f, 1.0f, sampleSpectrum(snapshot.fast, t));

        const float baseX = plot.getX() + p.xFraction * plot.getWidth();
        const float baseY = plot.getBottom() - v * plot.getHeight();

        // Scatter perpendicular to the trace, magnitude-driven: louder bins
        // spray a wider, denser-looking cloud; quiet bins collapse to a
        // thin, tight line.
        const float scatter = p.seed * (6.0f + v * 34.0f);
        const float size = 1.5f + v * 3.0f;

        g.setColour(accent.withAlpha(juce::jlimit(0.08f, 0.95f, 0.15f + v * 0.8f)
                                      * (snapshot.active ? 1.0f : 0.3f)));
        g.fillEllipse(baseX - size * 0.5f, baseY + scatter - size * 0.5f, size, size);
    }
}
