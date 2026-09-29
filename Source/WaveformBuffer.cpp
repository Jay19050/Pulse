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

void WaveformBuffer::getRecent(int numSamples, std::vector<float>& out) const
{
    const juce::SpinLock::ScopedLockType sl(lock);

    const int available = (int) juce::jmin((size_t) numSamples, written);

    // resize() only reallocates when growing past the vector's current
    // capacity; since MainComponent passes the same persistent vector every
    // tick and `available` stabilises quickly, this is a no-op after the
    // first couple of calls - no heap traffic in steady state.
    out.resize((size_t) available);

    // writePos points at the NEXT slot to be written, i.e. one past the
    // newest sample. Walk backwards from there to fill `out` oldest-first.
    size_t readPos = (writePos + (size_t) capacity - (size_t) available) % (size_t) capacity;

    for (int i = 0; i < available; ++i)
    {
        out[(size_t) i] = ring[readPos];
        readPos = (readPos + 1) % (size_t) capacity;
    }
}

void WaveformBuffer::clear()
{
    const juce::SpinLock::ScopedLockType sl(lock);
    ring.fill(0.0f);
    writePos = 0;
    written = 0;
}
