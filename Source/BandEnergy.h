#pragma once

#include <JuceHeader.h>
#include "SpectrumAnalyzer.h"

// Reduces the already-computed 512-point log-frequency spectrum into a small
// set of band energies every mode can read - so "bass/mid/high energy" logic
// lives in exactly one place instead of being reimplemented in six files.
//
// This is a cheap O(512) reduction over data SpectrumAnalyzer has already
// produced each tick - NOT a second FFT pass, and not "redundant analysis"
// in the sense that matters (there is still exactly one FFT, in
// SpectrumAnalyzer; this just reads its output six different ways).
//
// Index ranges are expressed as percentages of SpectrumAnalyzer's existing
// log-frequency point distribution (see SpectrumAnalyzer.cpp), matching the
// bass/mid/high split every mode that needs one already agrees on:
// ~0-8% = sub-bass/bass, ~15-55% = midrange, ~82-100% = high treble.
struct BandEnergy
{
    float bass = 0.0f;    // 0..1, sensitivity-applied
    float mid = 0.0f;     // 0..1, sensitivity-applied
    float high = 0.0f;    // 0..1, sensitivity-applied
    float overall = 0.0f; // snapshot.level, sensitivity-applied
    float peak = 0.0f;    // max of the peak-hold array, sensitivity-applied
};

inline BandEnergy computeBandEnergy(const SpectrumAnalyzer::Snapshot& snapshot, float sensitivity)
{
    constexpr int n = SpectrumAnalyzer::spectrumPoints;
    constexpr int bassEnd   = n * 8  / 100;
    constexpr int midStart  = n * 15 / 100;
    constexpr int midEnd    = n * 55 / 100;
    constexpr int highStart = n * 82 / 100;

    BandEnergy e;

    for (int i = 0; i < bassEnd; ++i)
        e.bass = juce::jmax(e.bass, snapshot.fast[(size_t) i]);

    for (int i = midStart; i < midEnd; ++i)
        e.mid += snapshot.fast[(size_t) i];
    e.mid /= static_cast<float>(juce::jmax(1, midEnd - midStart));

    for (int i = highStart; i < n; ++i)
        e.high = juce::jmax(e.high, snapshot.fast[(size_t) i]);

    for (int i = 0; i < n; ++i)
        e.peak = juce::jmax(e.peak, snapshot.peak[(size_t) i]);

    e.overall = snapshot.level;

    e.bass    = juce::jlimit(0.0f, 1.0f, e.bass * sensitivity);
    e.mid     = juce::jlimit(0.0f, 1.0f, e.mid * sensitivity);
    e.high    = juce::jlimit(0.0f, 1.0f, e.high * sensitivity);
    e.overall = juce::jlimit(0.0f, 1.0f, e.overall * sensitivity);
    e.peak    = juce::jlimit(0.0f, 1.0f, e.peak * sensitivity);

    return e;
}
