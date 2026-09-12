#pragma once

#include <JuceHeader.h>
#include <array>
#include "SpectrumAnalyzer.h"

// Turns a 512-point normalised (0..1) spectrum trace into a smooth juce::Path
// across the given rectangle. Shared by every mode that draws a spectrum
// curve (Filled Spectrum, Circular, Mirror) so the "faceted 512 straight
// segments reads as digital, not analog" fix lives in one place.
//
// mapPoint(index, value, bounds) places each sample; callers control axis
// orientation (e.g. Circular maps to polar coordinates instead of x/y).
namespace CurveUtils
{
    template <typename MapPointFn>
    juce::Path buildSmoothCurve(const std::array<float, SpectrumAnalyzer::spectrumPoints>& values,
                                 juce::Rectangle<float> bounds,
                                 MapPointFn&& mapPoint)
    {
        juce::Path p;
        constexpr int n = SpectrumAnalyzer::spectrumPoints;

        auto pointAt = [&](int i)
        {
            const int c = juce::jlimit(0, n - 1, i);
            return mapPoint(c, values[(size_t) c], bounds);
        };

        p.startNewSubPath(pointAt(0));

        // Catmull-Rom through every point, converted to cubic Beziers, with
        // each control point clamped inside its own segment's y-range so a
        // sharp peak doesn't overshoot into a phantom dip beside it.
        for (int i = 0; i < n - 1; ++i)
        {
            const auto p0 = pointAt(i - 1);
            const auto p1 = pointAt(i);
            const auto p2 = pointAt(i + 1);
            const auto p3 = pointAt(i + 2);

            auto c1 = p1 + (p2 - p0) * (1.0f / 6.0f);
            auto c2 = p2 - (p3 - p1) * (1.0f / 6.0f);

            const float loX = juce::jmin(p1.x, p2.x), hiX = juce::jmax(p1.x, p2.x);
            const float loY = juce::jmin(p1.y, p2.y), hiY = juce::jmax(p1.y, p2.y);

            c1.x = juce::jlimit(loX, hiX, c1.x); c1.y = juce::jlimit(loY, hiY, c1.y);
            c2.x = juce::jlimit(loX, hiX, c2.x); c2.y = juce::jlimit(loY, hiY, c2.y);

            p.cubicTo(c1, c2, p2);
        }

        return p;
    }

    // Standard x-across / y-up mapping, used by Filled Spectrum and Mirror.
    inline juce::Point<float> cartesianMap(int index, float value, juce::Rectangle<float> plot,
                                            bool invertY = false)
    {
        const float t = static_cast<float>(index) / static_cast<float>(SpectrumAnalyzer::spectrumPoints - 1);
        const float x = plot.getX() + t * plot.getWidth();
        const float v = juce::jlimit(0.0f, 1.0f, value);
        const float y = invertY ? plot.getY() + v * plot.getHeight()
                                 : plot.getBottom() - v * plot.getHeight();
        return { x, y };
    }
}
