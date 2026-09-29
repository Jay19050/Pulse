#include "TrackAnalysis.h"

namespace
{
    constexpr float kEpsilon = 1.0e-9f;
    constexpr double kMinBeatInterval = 60.0 / 200.0;
    constexpr double kMaxBeatInterval = 60.0 / 60.0;
}

void TrackAnalysis::prepare(double sampleRate) noexcept
{
    currentSampleRate = juce::jmax(8000.0, sampleRate);
    reset();
}

void TrackAnalysis::reset() noexcept
{
    sampleClock = 0.0;
    envelope = 0.0f;
    previousEnvelope = 0.0f;
    lowState = 0.0f;
    midState = 0.0f;
    highState = 0.0f;
    beatCooldown = 0.0f;
    pulse = 0.0f;
    beatCount = 0;
    beatWrite = 0;
    lastBeatTime = -100.0;
    stableBpm = 0.0;

    for (auto& t : beatTimes)
        t = 0.0;

    bpm.store(0.0f);
    energy.store(0.0f);
    peakDb.store(-100.0f);
    stereoWidth.store(0.0f);
    bass.store(0.0f);
    mid.store(0.0f);
    treble.store(0.0f);
    beatPulse.store(0.0f);
    beatDetected.store(false);
    active.store(false);
    hasBpm.store(false);
}

float TrackAnalysis::clamp01(float value) noexcept
{
    return juce::jlimit(0.0f, 1.0f, value);
}

float TrackAnalysis::toDb(float linear) noexcept
{
    return 20.0f * std::log10(juce::jmax(linear, kEpsilon));
}

void TrackAnalysis::pushStereo(const float* left, const float* right, int numSamples, double sampleRate) noexcept
{
    if (left == nullptr || right == nullptr || numSamples <= 0)
        return;

    if (sampleRate > 8000.0 && std::abs(sampleRate - currentSampleRate) > 1.0)
    {
        currentSampleRate = sampleRate;
        sampleClock = 0.0;
        beatCount = 0;
        beatWrite = 0;
        lastBeatTime = -100.0;
        stableBpm = 0.0;
        hasBpm.store(false, std::memory_order_relaxed);
    }

    double sumSquares = 0.0;
    double peak = 0.0;
    double sideEnergy = 0.0;
    double midEnergy = 0.0;

    // Very cheap band approximation using one-pole trackers. This is not
    // audio processing; it only derives visual statistics from the capture.
    for (int i = 0; i < numSamples; ++i)
    {
        const float l = left[i];
        const float r = right[i];
        const float mono = 0.5f * (l + r);
        const float side = 0.5f * (l - r);
        const float absMono = std::abs(mono);
        const float absSide = std::abs(side);

        sumSquares += static_cast<double>(mono) * mono;
        peak = juce::jmax(peak, static_cast<double>(std::max(std::abs(l), std::abs(r))));
        sideEnergy += static_cast<double>(side) * side;
        midEnergy += static_cast<double>(mono) * mono;

        const float lowInput = std::abs(mono);
        lowState += 0.015f * (lowInput - lowState);
        midState += 0.055f * (lowInput - midState);
        highState += 0.22f * (absMono - highState);
        (void) absSide;
    }

    const float rms = std::sqrt(static_cast<float>(sumSquares / static_cast<double>(numSamples)));
    const float blockPeak = static_cast<float>(peak);
    const float width = clamp01(static_cast<float>(std::sqrt(sideEnergy / juce::jmax(midEnergy, 1.0e-12))));

    // Smooth envelope for a stable energy display.
    envelope += 0.22f * (rms - envelope);
    const float energyValue = clamp01(envelope * 4.0f);
    const float peakValue = toDb(blockPeak);

    // Relative spectral-shape indicators. Normalised independently so they
    // remain useful across different master volumes.
    const float totalBands = lowState + midState + highState + kEpsilon;
    const float bassValue = clamp01((lowState / totalBands) * 2.4f);
    const float midValue = clamp01((midState / totalBands) * 2.0f);
    const float trebleValue = clamp01((highState / totalBands) * 2.8f);

    // Onset detector: a rising envelope above a small adaptive floor.
    const float rise = envelope - previousEnvelope;
    previousEnvelope = envelope;
    const float threshold = juce::jmax(0.008f, envelope * 0.085f);
    bool beat = false;

    beatCooldown = juce::jmax(0.0f, beatCooldown - static_cast<float>(numSamples / currentSampleRate));
    const double now = sampleClock / currentSampleRate;

    if (rise > threshold && energyValue > 0.025f && beatCooldown <= 0.0f)
    {
        beat = true;
        pulse = 1.0f;
        beatCooldown = 0.18f;

        if (lastBeatTime > 0.0)
        {
            const double interval = now - lastBeatTime;
            if (interval >= kMinBeatInterval && interval <= kMaxBeatInterval)
            {
                beatTimes[beatWrite] = interval;
                beatWrite = (beatWrite + 1) % static_cast<int>(beatTimes.size());
                beatCount = juce::jmin(beatCount + 1, static_cast<int>(beatTimes.size()));

                double average = 0.0;
                for (int i = 0; i < beatCount; ++i)
                    average += beatTimes[static_cast<std::size_t>(i)];
                average /= static_cast<double>(beatCount);

                double candidate = 60.0 / average;
                while (candidate < 70.0)
                    candidate *= 2.0;
                while (candidate > 180.0)
                    candidate *= 0.5;

                if (candidate >= 60.0 && candidate <= 200.0)
                {
                    stableBpm = stableBpm <= 0.0 ? candidate
                                                  : stableBpm + 0.20 * (candidate - stableBpm);
                    if (beatCount >= 4)
                        hasBpm.store(true, std::memory_order_relaxed);
                }
            }
        }

        lastBeatTime = now;
    }

    pulse *= std::exp(-static_cast<float>(numSamples / currentSampleRate) * 5.0f);
    sampleClock += static_cast<double>(numSamples);

    bpm.store(static_cast<float>(stableBpm), std::memory_order_relaxed);
    energy.store(energyValue, std::memory_order_relaxed);
    peakDb.store(juce::jlimit(-100.0f, 3.0f, peakValue), std::memory_order_relaxed);
    stereoWidth.store(width, std::memory_order_relaxed);
    bass.store(bassValue, std::memory_order_relaxed);
    mid.store(midValue, std::memory_order_relaxed);
    treble.store(trebleValue, std::memory_order_relaxed);
    beatPulse.store(clamp01(pulse), std::memory_order_relaxed);
    beatDetected.store(beat, std::memory_order_relaxed);
    active.store(energyValue > 0.012f, std::memory_order_relaxed);
}

TrackAnalysis::Snapshot TrackAnalysis::getSnapshot() const noexcept
{
    Snapshot s;
    s.bpm = bpm.load(std::memory_order_relaxed);
    s.energy = energy.load(std::memory_order_relaxed);
    s.peakDb = peakDb.load(std::memory_order_relaxed);
    s.stereoWidth = stereoWidth.load(std::memory_order_relaxed);
    s.bass = bass.load(std::memory_order_relaxed);
    s.mid = mid.load(std::memory_order_relaxed);
    s.treble = treble.load(std::memory_order_relaxed);
    s.beatPulse = beatPulse.load(std::memory_order_relaxed);
    s.beatDetected = beatDetected.load(std::memory_order_relaxed);
    s.active = active.load(std::memory_order_relaxed);
    s.hasBpm = hasBpm.load(std::memory_order_relaxed);
    return s;
}
