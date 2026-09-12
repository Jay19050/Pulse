#include "GoniometerComponent.h"
#include "PulseTheme.h"
#include <cmath>

GoniometerComponent::GoniometerComponent()
{
    setOpaque(false);
    startTimerHz(30);
}

void GoniometerComponent::clearTrace()
{
    const juce::ScopedLock lock(dataLock);
    recent.clear();
}

void GoniometerComponent::setActive(bool shouldBeActive)
{
    if (active != shouldBeActive)
    {
        active = shouldBeActive;

        if (! active)
            clearTrace();
    }
}

void GoniometerComponent::pushStereo(const float* left, const float* right, int numSamples)
{
    if (! active || left == nullptr || right == nullptr || numSamples <= 0)
        return;

    const int keep  = juce::jmin(numSamples, kMaxDots);
    const int start = numSamples - keep;   // the newest frames in this block

    const juce::ScopedLock lock(dataLock);

    recent.resize(static_cast<size_t>(keep) * 2);

    for (int i = 0; i < keep; ++i)
    {
        recent[(size_t) i * 2]     = juce::jlimit(-1.0f, 1.0f, left[start + i]);
        recent[(size_t) i * 2 + 1] = juce::jlimit(-1.0f, 1.0f, right[start + i]);
    }
}

float GoniometerComponent::correlation(const std::vector<float>& interleavedLR)
{
    const size_t frames = interleavedLR.size() / 2;
    if (frames == 0)
        return 0.0f;

    double sumLR = 0.0, sumLL = 0.0, sumRR = 0.0;

    for (size_t i = 0; i < frames; ++i)
    {
        const double l = interleavedLR[2 * i];
        const double r = interleavedLR[2 * i + 1];
        sumLR += l * r;
        sumLL += l * l;
        sumRR += r * r;
    }

    const double denom = std::sqrt(sumLL * sumRR);
    if (denom < 1.0e-9)
        return 0.0f;   // silence: no meaningful correlation

    return static_cast<float>(juce::jlimit(-1.0, 1.0, sumLR / denom));
}

void GoniometerComponent::timerCallback()
{
    if (! active)
        clearTrace();

    repaint();
}

void GoniometerComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colour(PulseTheme::WidgetBackground));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Nothing on this widget says what it is: an unlabelled dot cloud in a circle
    // is only readable if you already know it's a goniometer.
    auto titleArea = bounds.removeFromTop(18.0f).reduced(8.0f, 2.0f);
    auto meterArea = bounds.removeFromBottom(22.0f).reduced(8.0f, 4.0f);
    auto scopeArea = bounds.reduced(8.0f);

    g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(0.75f));
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("GONIOMETER", titleArea, juce::Justification::centredLeft);

    // Reference circle and axes.
    const float r  = juce::jmin(scopeArea.getWidth(), scopeArea.getHeight()) * 0.5f;
    const float cx = scopeArea.getCentreX();
    const float cy = scopeArea.getCentreY();

    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.35f));
    g.drawEllipse(cx - r, cy - r, 2.0f * r, 2.0f * r, 1.0f);
    g.drawVerticalLine  (static_cast<int>(cx), cy - r, cy + r);
    g.drawHorizontalLine(static_cast<int>(cy), cx - r, cx + r);

    // The dot cloud: for each (L, R) pair, rotate 45 degrees so mono (L == R)
    // plots as a vertical line and out-of-phase content plots horizontally - the
    // standard goniometer convention. Note the sign: (R - L), not (L - R) - get
    // this backwards and a left-only signal plots on the wrong side.
    {
        const juce::ScopedLock lock(dataLock);

        const size_t frames = recent.size() / 2;
        const float  k = 0.70710678f;   // 1/sqrt(2)

        g.setColour(juce::Colour(PulseTheme::Accent).withAlpha(0.85f));

        for (size_t i = 0; i < frames; ++i)
        {
            const float l  = recent[2 * i];
            const float rr = recent[2 * i + 1];

            const float x = cx + (rr - l) * k * r;
            const float y = cy - (l + rr) * k * r;

            g.fillEllipse(x - 1.0f, y - 1.0f, 2.0f, 2.0f);
        }
    }

    // Axis labels around the circle's edge.
    struct Axis { float dx, dy; const char* text; };

    const float k = 0.70710678f;

    const Axis axes[] = {
        {  0.0f, -1.0f, "M" },     // straight up: mono, L == R
        {    -k, -k,    "L" },     // upper left:  left channel only
        {     k, -k,    "R" },     // upper right: right channel only
        { -1.0f,  0.0f, "-" },     // horizontal:  anti-phase
        {  1.0f,  0.0f, "-" },
    };

    g.setFont(9.0f);
    g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(0.55f));

    for (const auto& a : axes)
        g.drawText(a.text,
                   juce::Rectangle<float>(cx + a.dx * (r + 7.0f) - 6.0f,
                                          cy + a.dy * (r + 7.0f) - 6.0f, 12.0f, 12.0f),
                   juce::Justification::centred);

    // Correlation meter: +1 (mono) at the right, -1 (out of phase) at the left.
    float corr = 0.0f;

    {
        const juce::ScopedLock lock(dataLock);
        corr = correlation(recent);
    }

    g.setColour(juce::Colour(PulseTheme::Outline).withAlpha(0.4f));
    g.drawRoundedRectangle(meterArea, 3.0f, 1.0f);

    const float t = (corr + 1.0f) * 0.5f;
    const float markerX = meterArea.getX() + t * meterArea.getWidth();

    g.setColour(corr < 0.0f ? juce::Colours::orangered : juce::Colour(PulseTheme::Accent));
    g.fillRoundedRectangle(markerX - 2.0f, meterArea.getY(), 4.0f, meterArea.getHeight(), 2.0f);

    g.setFont(9.0f);
    g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(0.45f));
    g.drawText("-1", meterArea.reduced(4.0f, 0.0f), juce::Justification::centredLeft);
    g.drawText("+1", meterArea.reduced(4.0f, 0.0f), juce::Justification::centredRight);

    g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(0.8f));
    g.setFont(10.0f);
    g.drawText("corr " + juce::String(corr, 2), meterArea, juce::Justification::centred);
}
