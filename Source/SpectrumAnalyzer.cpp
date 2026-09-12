#include "SpectrumAnalyzer.h"

SpectrumAnalyzer::SpectrumAnalyzer()
{
    fast.fill(0.0f);
    slow.fill(0.0f);
}

void SpectrumAnalyzer::prepare(double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 48000.0;

    fifoIndex = 0;
    fifo.fill(0.0f);
    fftData.fill(0.0f);

    fast.fill(0.0f);
    slow.fill(0.0f);
    peakHold.fill(0.0f);

    currentLevel = 0.0f;
    active = false;
}

void SpectrumAnalyzer::pushStereo(const float* left,
                                  const float* right,
                                  int numSamples)
{
    if (left == nullptr || numSamples <= 0)
        return;

    const juce::SpinLock::ScopedLockType lock(snapshotLock);

    for (int i = 0; i < numSamples; ++i)
    {
        const float l = left[i];
        const float r = right != nullptr ? right[i] : l;

        // Stereo -> mono for spectrum analysis.
        const float mono = 0.5f * (l + r);

        fifo[(size_t) fifoIndex++] = mono;

        if (fifoIndex >= fftSize)
        {
            processBlock();

            // -------------------------------------------------
            // 50% overlapping FFT.
            //
            // Keep the newest half of the previous FFT and
            // collect another half before calculating again.
            //
            // This keeps 2048-point frequency resolution while
            // making the visualizer considerably more responsive.
            // -------------------------------------------------

            constexpr int overlap = fftSize / 2;

            std::copy(fifo.begin() + overlap, fifo.end(), fifo.begin());

            fifoIndex = overlap;
        }
    }
}

void SpectrumAnalyzer::processBlock()
{
    // Copy audio into FFT buffer.
    std::copy(fifo.begin(), fifo.end(), fftData.begin());
    std::fill(fftData.begin() + fftSize, fftData.end(), 0.0f);

    // Hann window.
    window.multiplyWithWindowingTable(fftData.data(), fftSize);

    // Perform FFT.
    fft.performRealOnlyForwardTransform(fftData.data());

    // ---------------------------------------------------------
    // Overall signal level.
    // ---------------------------------------------------------

    float sumSquares = 0.0f;

    for (const auto sample : fifo)
        sumSquares += sample * sample;

    const float rms = std::sqrt(sumSquares / static_cast<float>(fftSize));

    currentLevel = juce::jlimit(0.0f, 1.0f, rms * 5.0f);
    active = currentLevel > 0.0005f;

    // ---------------------------------------------------------
    // Log-frequency spectrum.
    // ---------------------------------------------------------

    for (int p = 0; p < spectrumPoints; ++p)
    {
        const float t = static_cast<float>(p) / static_cast<float>(spectrumPoints - 1);

        const float minHz = 20.0f;
        const float maxHz = static_cast<float>(sampleRate * 0.5);

        // Balanced frequency distribution.
        const float shapedT = std::pow(t, 0.72f);
        const float hz = minHz * std::pow(maxHz / minHz, shapedT);

        const float bin = hz * static_cast<float>(fftSize) / static_cast<float>(sampleRate);

        const int i0 = juce::jlimit(1, fftSize / 2 - 2, static_cast<int>(bin));
        const int i1 = i0 + 1;
        const float frac = juce::jlimit(0.0f, 1.0f, bin - static_cast<float>(i0));

        // First FFT bin.
        const float real0 = fftData[2 * i0];
        const float imag0 = fftData[2 * i0 + 1];
        const float mag0 = std::sqrt(real0 * real0 + imag0 * imag0);

        // Second FFT bin.
        const float real1 = fftData[2 * i1];
        const float imag1 = fftData[2 * i1 + 1];
        const float mag1 = std::sqrt(real1 * real1 + imag1 * imag1);

        // Interpolate between FFT bins.
        const float magnitude = mag0 + (mag1 - mag0) * frac;

        // Normalize FFT magnitude.
        const float normalizedMagnitude = magnitude * (2.0f / static_cast<float>(fftSize));

        // Convert magnitude to dB.
        const float db = 20.0f * std::log10(juce::jmax(normalizedMagnitude, 1.0e-6f));

        // Useful dynamic range mapped to 0..1.
        const float normalized = juce::jlimit(0.0f, 1.0f, (db + 70.0f) / 55.0f);

        // -----------------------------------------------------
        // Fast live spectrum.
        //
        // Slightly slower decay compensates for updating twice
        // as frequently now that the FFT overlaps 50%.
        // -----------------------------------------------------

        fast[(size_t) p] = juce::jmax(normalized, fast[(size_t) p] * 0.70f);
        fast[(size_t) p] *= 0.992f;

        // -----------------------------------------------------
        // Peak hold: instant attack, slow decay - the classic
        // "loudest thing that happened recently" ceiling trace.
        // -----------------------------------------------------

        peakHold[(size_t) p] = juce::jmax(peakHold[(size_t) p] * 0.996f, fast[(size_t) p]);

        // -----------------------------------------------------
        // Slow reference trace.
        // -----------------------------------------------------

        slow[(size_t) p] += (fast[(size_t) p] - slow[(size_t) p]) * 0.055f;
    }
}

SpectrumAnalyzer::Snapshot SpectrumAnalyzer::getSnapshot() const
{
    const juce::SpinLock::ScopedLockType lock(snapshotLock);

    Snapshot snapshot;
    snapshot.fast = fast;
    snapshot.slow = slow;
    snapshot.peak = peakHold;
    snapshot.level = currentLevel;
    snapshot.active = active;

    return snapshot;
}

void SpectrumAnalyzer::resetPeakHold()
{
    const juce::SpinLock::ScopedLockType lock(snapshotLock);
    peakHold.fill(0.0f);
}
