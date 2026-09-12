#include "WaveformBuffer.h"

void WaveformBuffer::push(const float* left, const float* right, int numSamples)
{
    if (left == nullptr || numSamples <= 0)
        return;

    const juce::SpinLock::ScopedLockType sl(lock);

    for (int i = 0; i < numSamples; ++i)
    {
        const float l = left[i];
        const float r = right != nullptr ? right[i] : l;

        ring[writePos] = juce::jlimit(-1.0f, 1.0f, 0.5f * (l + r));
        writePos = (writePos + 1) % (size_t) capacity;
    }

    written = juce::jmin((size_t) capacity, written + (size_t) numSamples);
}

std::vector<float> WaveformBuffer::getRecent(int numSamples) const
{
    const juce::SpinLock::ScopedLockType sl(lock);

    const int available = (int) juce::jmin((size_t) numSamples, written);
    std::vector<float> out((size_t) available);

    // writePos points at the NEXT slot to be written, i.e. one past the
    // newest sample. Walk backwards from there to fill `out` oldest-first.
    size_t readPos = (writePos + (size_t) capacity - (size_t) available) % (size_t) capacity;

    for (int i = 0; i < available; ++i)
    {
        out[(size_t) i] = ring[readPos];
        readPos = (readPos + 1) % (size_t) capacity;
    }

    return out;
}

void WaveformBuffer::clear()
{
    const juce::SpinLock::ScopedLockType sl(lock);
    ring.fill(0.0f);
    writePos = 0;
    written = 0;
}
