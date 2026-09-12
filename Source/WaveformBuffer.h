#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>

// A short, continuously-scrolling window of raw mono audio, for the Waveform
// (oscilloscope) mode.
//
// This is deliberately separate from SpectrumAnalyzer: the spectrum's FFT
// fifo is windowed and consumed in 50%-overlapping chunks, which is the wrong
// shape of data for "draw the last N milliseconds of the actual waveform".
// Unlike GoniometerComponent's dot cloud (replaced wholesale every callback,
// since a scope only needs "what just happened"), this is a real ring buffer
// so the trace scrolls continuously instead of jumping between blocks.
class WaveformBuffer
{
public:
    // ~1 second at 48 kHz. Plenty for any on-screen window the mode asks for;
    // memory cost is trivial (a few hundred KB of floats).
    static constexpr int capacity = 1 << 16;

    void push(const float* left, const float* right, int numSamples);

    // Returns the most recent `numSamples` samples, oldest first. If fewer
    // than `numSamples` have ever been pushed, the result is shorter.
    std::vector<float> getRecent(int numSamples) const;

    void clear();

private:
    mutable juce::SpinLock lock;
    std::array<float, capacity> ring {};
    size_t writePos = 0;
    size_t written = 0; // total samples ever pushed, saturates at capacity for size purposes
};
