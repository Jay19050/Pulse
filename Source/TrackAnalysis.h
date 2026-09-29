#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <array>
#include <cmath>

// Lightweight real-time track analysis. It never allocates, locks, or touches
// the JUCE message thread from the audio callback.
class TrackAnalysis final
{
public:
    struct Snapshot
    {
        float bpm = 0.0f;
        float energy = 0.0f;       // 0..1
        float peakDb = -100.0f;
        float stereoWidth = 0.0f; // 0..1
        float bass = 0.0f;        // 0..1 relative band energy
        float mid = 0.0f;
        float treble = 0.0f;
        float beatPulse = 0.0f;   // decaying 0..1 pulse
        bool beatDetected = false;
        bool active = false;
        bool hasBpm = false;
    };

    TrackAnalysis() = default;

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void pushStereo(const float* left, const float* right, int numSamples, double sampleRate) noexcept;
    Snapshot getSnapshot() const noexcept;

private:
    static float clamp01(float value) noexcept;
    static float toDb(float linear) noexcept;

    std::atomic<float> bpm { 0.0f };
    std::atomic<float> energy { 0.0f };
    std::atomic<float> peakDb { -100.0f };
    std::atomic<float> stereoWidth { 0.0f };
    std::atomic<float> bass { 0.0f };
    std::atomic<float> mid { 0.0f };
    std::atomic<float> treble { 0.0f };
    std::atomic<float> beatPulse { 0.0f };
    std::atomic<bool> beatDetected { false };
    std::atomic<bool> active { false };
    std::atomic<bool> hasBpm { false };

    // All state below is owned by the audio callback thread.
    double currentSampleRate = 48000.0;
    double sampleClock = 0.0;
    float envelope = 0.0f;
    float previousEnvelope = 0.0f;
    float lowState = 0.0f;
    float midState = 0.0f;
    float highState = 0.0f;
    float beatCooldown = 0.0f;
    float pulse = 0.0f;

    std::array<double, 12> beatTimes {};
    int beatCount = 0;
    int beatWrite = 0;
    double lastBeatTime = -100.0;
    double stableBpm = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackAnalysis)
};
