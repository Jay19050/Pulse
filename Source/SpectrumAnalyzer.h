#pragma once

#include <JuceHeader.h>

class SpectrumAnalyzer
{
public:
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int spectrumPoints = 512;

    struct Snapshot
    {
        std::array<float, spectrumPoints> fast {};
        std::array<float, spectrumPoints> slow {};
        std::array<float, spectrumPoints> peak {};
        float level = 0.0f;
        bool active = false;
    };

    SpectrumAnalyzer();

    void prepare(double newSampleRate);
    void pushStereo(const float* left, const float* right, int numSamples);

    // Clears the decaying peak-hold trace. Called when the user clicks the
    // spectrum plot - a peak hold with no way to clear it just shows the loudest
    // thing that ever happened, not the loudest thing that happened recently.
    void resetPeakHold();

    Snapshot getSnapshot() const;

private:
    void processBlock();

    double sampleRate = 48000.0;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window {
        fftSize,
        juce::dsp::WindowingFunction<float>::hann,
        false
    };

    std::array<float, fftSize> fifo {};
    std::array<float, fftSize * 2> fftData {};
    int fifoIndex = 0;

    std::array<float, spectrumPoints> fast {};
    std::array<float, spectrumPoints> slow {};
    std::array<float, spectrumPoints> peakHold {};
    float currentLevel = 0.0f;
    bool active = false;

    mutable juce::SpinLock snapshotLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};
