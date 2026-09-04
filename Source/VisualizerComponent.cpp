#include "VisualizerComponent.h"
#include "PulseTheme.h"
#include <cmath>
#include <vector>

namespace
{
    constexpr float kPlotInset = 8.0f;

    const juce::Colour kAccentColour   { PulseTheme::Accent };
    const juce::Colour kPeakHoldColour { PulseTheme::PeakHold };
    const juce::Colour kOutlineColour  { PulseTheme::Outline };
    const juce::Colour kTextColour     { PulseTheme::DefaultText };
    const juce::Colour kPanelColour    { PulseTheme::WidgetBackground };

    // Log-spaced frequency gridlines - the values an engineer actually looks for.
    const float kGridFreqs[] = { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f,
                                 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };

    const float kMinFreqHz = 20.0f;
    const float kMaxFreqHz = 20000.0f;

    juce::String labelForFreq(float f)
    {
        return f >= 1000.0f ? juce::String(static_cast<int>(f / 1000.0f)) + "k"
                            : juce::String(static_cast<int>(f));
    }

    juce::String preciseFreq(int pointIndex)
    {
        const float t = static_cast<float>(pointIndex)
                      / static_cast<float>(SpectrumAnalyzer::spectrumPoints - 1);
        const float logMin = std::log10(kMinFreqHz);
        const float logMax = std::log10(kMaxFreqHz);
        const float f = std::pow(10.0f, logMin + t * (logMax - logMin));

        return f >= 1000.0f ? juce::String(f / 1000.0f, 2) + " kHz"
                            : juce::String(f, 1) + " Hz";
    }

    // SpectrumAnalyzer stores everything as a normalised 0..1 magnitude, not raw
    // dB. This is the inverse of the mapping it uses internally ((db + 70) / 55,
    // clamped), purely so the hover readout can show something in familiar units.
    // It is an approximation near the clamped ends of the 0..1 range - fine for a
    // readout, not meant for precise metering.
    float approxDb(float normalisedValue)
    {
        return normalisedValue * 55.0f - 70.0f;
    }

    struct LegendEntry
    {
        juce::Colour colour;
        const char*  text;
    };

    std::array<LegendEntry, VisualizerComponent::kNumTraces> legendEntries()
    {
        return { {
            { juce::Colour(PulseTheme::Accent),                    "LIVE - real-time spectrum" },
            { juce::Colours::white.withAlpha(0.85f),               "AVG  - slow reference" },
            { juce::Colour(PulseTheme::PeakHold).withAlpha(0.75f), "PEAK HOLD" },
        } };
    }
}

VisualizerComponent::VisualizerComponent()
{
    setOpaque(false);
    setMouseCursor(juce::MouseCursor::CrosshairCursor);

    // Runs independently of MainComponent's 30 Hz audio-analysis timer: this just
    // repaints at display rate so the curve and the hover readout stay smooth.
    startTimerHz(60);
}

void VisualizerComponent::setSnapshot(const SpectrumAnalyzer::Snapshot& newSnapshot)
{
    snapshot = newSnapshot;
}

void VisualizerComponent::timerCallback()
{
    repaint();
}

juce::Rectangle<float> VisualizerComponent::plotArea() const
{
    return getLocalBounds().toFloat()
             .reduced(kPlotInset)
             .withTrimmedBottom(16.0f)      // frequency axis labels
             .withTrimmedLeft(kLeftGutter); // dB-ish scale
}

float VisualizerComponent::xForPoint(int pointIndex, juce::Rectangle<float> plot) const
{
    const float t = static_cast<float>(pointIndex)
                  / static_cast<float>(SpectrumAnalyzer::spectrumPoints - 1);
    return plot.getX() + t * plot.getWidth();
}

float VisualizerComponent::yForValue(float normalisedValue, juce::Rectangle<float> plot) const
{
    return plot.getBottom() - juce::jlimit(0.0f, 1.0f, normalisedValue) * plot.getHeight();
}

int VisualizerComponent::pointForX(float x, juce::Rectangle<float> plot) const
{
    if (plot.getWidth() <= 0.0f)
        return 0;

    const float t = (x - plot.getX()) / plot.getWidth();
    const int   i = juce::roundToInt(t * static_cast<float>(SpectrumAnalyzer::spectrumPoints - 1));

    return juce::jlimit(0, SpectrumAnalyzer::spectrumPoints - 1, i);
}

juce::Path VisualizerComponent::buildCurve(const std::array<float, SpectrumAnalyzer::spectrumPoints>& values,
                                           juce::Rectangle<float> plot) const
{
    juce::Path p;

    constexpr int n = SpectrumAnalyzer::spectrumPoints;

    auto pointAt = [&](int i)
    {
        const int c = juce::jlimit(0, n - 1, i);   // clamp the ends, so no phantom slope
        return juce::Point<float>(xForPoint(c, plot), yForValue(values[(size_t) c], plot));
    };

    p.startNewSubPath(pointAt(0));

    // Catmull-Rom through every point, converted to cubic Beziers - straight
    // segments between 512 points read as visibly faceted, not analog.
    for (int i = 0; i < n - 1; ++i)
    {
        const auto p0 = pointAt(i - 1);
        const auto p1 = pointAt(i);
        const auto p2 = pointAt(i + 1);
        const auto p3 = pointAt(i + 2);

        auto c1 = p1 + (p2 - p0) * (1.0f / 6.0f);
        auto c2 = p2 - (p3 - p1) * (1.0f / 6.0f);

        // Catmull-Rom overshoots at a sharp change of direction, which reads as a
        // phantom dip beside every peak. Clamp each control point inside its own
        // segment's range so the curve stays smooth but never leaves the data.
        const float lo = juce::jmin(p1.y, p2.y);
        const float hi = juce::jmax(p1.y, p2.y);

        c1.y = juce::jlimit(lo, hi, c1.y);
        c2.y = juce::jlimit(lo, hi, c2.y);

        p.cubicTo(c1, c2, p2);
    }

    return p;
}

void VisualizerComponent::drawGrid(juce::Graphics& g, juce::Rectangle<float> plot) const
{
    g.setColour(kOutlineColour.withAlpha(0.35f));

    const float logMin = std::log10(kMinFreqHz);
    const float logMax = std::log10(kMaxFreqHz);

    for (const float f : kGridFreqs)
    {
        const float t = (std::log10(f) - logMin) / (logMax - logMin);
        const float x = plot.getX() + t * plot.getWidth();
        g.drawVerticalLine(static_cast<int>(x), plot.getY(), plot.getBottom());
    }

    for (int i = 1; i < 4; ++i)
    {
        const float y = plot.getY() + plot.getHeight() * (static_cast<float>(i) / 4.0f);
        g.drawHorizontalLine(static_cast<int>(y), plot.getX(), plot.getRight());
    }

    g.setColour(kTextColour.withAlpha(0.5f));
    g.setFont(11.0f);

    for (const float f : kGridFreqs)
    {
        const float t = (std::log10(f) - logMin) / (logMax - logMin);
        const float x = plot.getX() + t * plot.getWidth();
        g.drawText(labelForFreq(f),
                   juce::Rectangle<float>(x - 20.0f, plot.getBottom() + 2.0f, 40.0f, 14.0f),
                   juce::Justification::centred);
    }

    g.setFont(10.0f);

    for (int i = 0; i <= 4; ++i)
    {
        const float norm = 1.0f - static_cast<float>(i) / 4.0f;
        const float y = yForValue(norm, plot);

        g.drawText(juce::String(static_cast<int>(approxDb(norm))),
                   juce::Rectangle<float>(plot.getX() - kLeftGutter, y - 7.0f,
                                          kLeftGutter - 5.0f, 14.0f),
                   juce::Justification::centredRight);
    }
}

void VisualizerComponent::mouseMove(const juce::MouseEvent& e)
{
    const auto plot = plotArea();
    const float x = static_cast<float>(e.position.x);
    const float next = plot.contains(e.position) ? x : -1.0f;

    if (next != hoverX)
    {
        hoverX = next;
        repaint();
    }
}

void VisualizerComponent::mouseExit(const juce::MouseEvent&)
{
    if (hoverX >= 0.0f)
    {
        hoverX = -1.0f;
        repaint();
    }
}

void VisualizerComponent::setTraceVisible(Trace t, bool visible)
{
    auto& flag = traceVisible[(size_t) t];

    if (flag == visible)
        return;

    flag = visible;
    repaint();
}

void VisualizerComponent::setLegendCollapsed(bool collapsed)
{
    if (legendCollapsed == collapsed)
        return;

    legendCollapsed = collapsed;
    repaint();
}

juce::Rectangle<float> VisualizerComponent::legendHandleBounds() const
{
    const auto plot = plotArea();
    return { plot.getX() + 8.0f, plot.getY() + 6.0f, 220.0f, 14.0f };
}

juce::Rectangle<float> VisualizerComponent::legendRowBounds(int index) const
{
    return legendHandleBounds().translated(0.0f, 15.0f * static_cast<float>(index + 1));
}

juce::Rectangle<float> VisualizerComponent::legendPanelBounds() const
{
    const auto handle = legendHandleBounds();
    const int  rows   = legendCollapsed ? 1 : (kNumTraces + 1);

    return { handle.getX() - 5.0f, handle.getY() - 4.0f,
             handle.getWidth() + 10.0f, 15.0f * static_cast<float>(rows) + 6.0f };
}

int VisualizerComponent::legendRowAt(juce::Point<float> p) const
{
    if (legendCollapsed)
        return -1;

    const auto panel = legendPanelBounds();

    if (! panel.contains(p))
        return -1;

    if (legendHandleBounds().contains(p))
        return -1;

    for (int i = 0; i < kNumTraces; ++i)
        if (legendRowBounds(i).contains(p))
            return i;

    const auto  row0 = legendRowBounds(0);
    const float rel  = (p.y - row0.getY()) / 15.0f;

    return juce::jlimit(0, kNumTraces - 1, static_cast<int>(std::round(rel)));
}

void VisualizerComponent::mouseDown(const juce::MouseEvent& e)
{
    // Handle first: it is the outermost control, and this is the only thing left
    // to click once the legend is collapsed.
    if (legendHandleBounds().contains(e.position))
    {
        setLegendCollapsed(! legendCollapsed);
        return;
    }

    // The legend is the control surface: a row is a switch for its trace.
    const int row = legendRowAt(e.position);

    if (row >= 0)
    {
        const auto trace = static_cast<Trace>(row);
        setTraceVisible(trace, ! isTraceVisible(trace));
        return;
    }

    // Dead space inside the legend panel must not fall through to onResetPeaks -
    // a click aimed at the legend that missed a row by a couple of pixels
    // shouldn't throw away the peak hold you were reading.
    if (legendPanelBounds().contains(e.position))
        return;

    if (onResetPeaks != nullptr)
        onResetPeaks();
}

void VisualizerComponent::drawLegendBackdrop(juce::Graphics& g) const
{
    // Painted BEFORE any trace, so it only darkens the fill underneath for
    // contrast - it does not occlude anything, since everything it could occlude
    // is drawn afterwards.
    g.setColour(juce::Colour(PulseTheme::WindowBackground).withAlpha(0.78f));
    g.fillRoundedRectangle(legendPanelBounds(), 4.0f);
}

void VisualizerComponent::drawLegendForeground(juce::Graphics& g) const
{
    const auto entries = legendEntries();

    // A second, much fainter wash over the traces just before the text: a trace
    // crossing the legend keeps most of its colour and stays visible, rather than
    // being erased by an opaque backdrop drawn on top of it.
    g.setColour(juce::Colour(PulseTheme::WindowBackground).withAlpha(0.35f));
    g.fillRoundedRectangle(legendPanelBounds(), 4.0f);

    g.setFont(11.0f);

    g.setColour(kTextColour.withAlpha(0.85f));
    g.drawText(legendCollapsed ? "legend  >" : "legend  v",
               legendHandleBounds(), juce::Justification::centredLeft);

    if (legendCollapsed)
        return;

    for (int i = 0; i < kNumTraces; ++i)
    {
        const auto& e   = entries[(size_t) i];
        const auto  row = legendRowBounds(i);
        const bool  on  = traceVisible[(size_t) i];

        g.setColour(e.colour.withMultipliedAlpha(on ? 1.0f : 0.22f));
        g.fillRect(row.getX(), row.getCentreY() - 1.5f, 14.0f, 3.0f);

        juce::String text = e.text;

        if (i == TracePeakHold && on)
            text += " - click plot to clear";

        g.setColour(kTextColour.withAlpha(on ? 0.85f : 0.35f));
        g.drawText(text, row.withTrimmedLeft(20.0f), juce::Justification::centredLeft);
    }
}

void VisualizerComponent::paint(juce::Graphics& g)
{
    const auto plot = plotArea();

    g.setColour(kPanelColour);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 6.0f);

    drawGrid(g, plot);

    drawLegendBackdrop(g);

    // LIVE: filled, in the accent colour. This is what's happening right now.
    if (traceVisible[(size_t) TraceLive])
    {
        const juce::Path liveStroke = buildCurve(snapshot.fast, plot);

        juce::Path liveFilled = liveStroke;
        liveFilled.lineTo(plot.getRight(), plot.getBottom());
        liveFilled.lineTo(plot.getX(),     plot.getBottom());
        liveFilled.closeSubPath();

        g.setGradientFill(juce::ColourGradient(kAccentColour.withAlpha(0.55f), plot.getX(), plot.getY(),
                                               kAccentColour.withAlpha(0.05f), plot.getX(), plot.getBottom(),
                                               false));
        g.fillPath(liveFilled);

        g.setColour(kAccentColour.withAlpha(snapshot.active ? 0.95f : 0.4f));
        g.strokePath(liveStroke, juce::PathStrokeType(1.6f));
    }

    // Peak hold, drawn above the fill: a ceiling sitting over the live spectrum,
    // not another curve competing with it.
    if (traceVisible[(size_t) TracePeakHold])
    {
        g.setColour(kPeakHoldColour.withAlpha(0.75f));
        g.strokePath(buildCurve(snapshot.peak, plot), juce::PathStrokeType(1.0f));
    }

    // AVG: unfilled ghost, drawn last so it stays readable on top of the fill.
    if (traceVisible[(size_t) TraceAverage])
    {
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.strokePath(buildCurve(snapshot.slow, plot), juce::PathStrokeType(1.6f));
    }

    // Foreground last: the legend and hover readout ride on top of every trace.
    drawLegendForeground(g);
    drawHoverReadout(g, plot);
}

void VisualizerComponent::drawHoverReadout(juce::Graphics& g, juce::Rectangle<float> plot) const
{
    if (hoverX < 0.0f)
        return;

    const int  i   = pointForX(hoverX, plot);
    const auto idx = (size_t) i;
    const float x  = xForPoint(i, plot);   // snap to the point, not the raw cursor

    g.setColour(juce::Colours::white.withAlpha(0.35f));
    g.drawVerticalLine(static_cast<int>(x), plot.getY(), plot.getBottom());

    struct Row { juce::Colour colour; juce::String text; };
    std::vector<Row> rows;

    rows.push_back({ kTextColour, preciseFreq(i) });

    if (traceVisible[(size_t) TraceLive])
    {
        g.setColour(kAccentColour);
        g.fillEllipse(x - 2.5f, yForValue(snapshot.fast[idx], plot) - 2.5f, 5.0f, 5.0f);
        rows.push_back({ kAccentColour, "LIVE " + juce::String(approxDb(snapshot.fast[idx]), 1) + " dB" });
    }

    if (traceVisible[(size_t) TraceAverage])
    {
        g.setColour(juce::Colours::white.withAlpha(0.9f));
        g.fillEllipse(x - 2.5f, yForValue(snapshot.slow[idx], plot) - 2.5f, 5.0f, 5.0f);
        rows.push_back({ juce::Colours::white, "AVG  " + juce::String(approxDb(snapshot.slow[idx]), 1) + " dB" });
    }

    if (traceVisible[(size_t) TracePeakHold])
    {
        g.setColour(kPeakHoldColour);
        g.fillEllipse(x - 2.5f, yForValue(snapshot.peak[idx], plot) - 2.5f, 5.0f, 5.0f);
        rows.push_back({ kPeakHoldColour, "PEAK " + juce::String(approxDb(snapshot.peak[idx]), 1) + " dB" });
    }

    // Row 0 is just the frequency, which belongs to the cursor rather than any
    // trace. If it's the only row, every value-bearing trace is off - nothing
    // worth boxing, so leave the crosshair and stop.
    if (rows.size() < 2)
        return;

    constexpr float kBoxW = 118.0f;
    const float     kBoxH = 12.0f + 15.0f * static_cast<float>(rows.size());

    // Flips to the other side of the crosshair near the right edge so it stays
    // on screen.
    const bool  flip = x + 10.0f + kBoxW > plot.getRight();
    const float boxX = flip ? x - 10.0f - kBoxW : x + 10.0f;

    auto box = juce::Rectangle<float>(boxX, plot.getBottom() - kBoxH - 6.0f, kBoxW, kBoxH);

    g.setColour(juce::Colour(PulseTheme::WindowBackground).withAlpha(0.92f));
    g.fillRoundedRectangle(box, 4.0f);

    g.setColour(kOutlineColour.withAlpha(0.6f));
    g.drawRoundedRectangle(box, 4.0f, 1.0f);

    auto row = box.reduced(8.0f, 6.0f).removeFromTop(15.0f);

    g.setFont(11.0f);

    for (const auto& r : rows)
    {
        g.setColour(r.colour);
        g.drawText(r.text, row, juce::Justification::centredLeft);
        row.translate(0.0f, 15.0f);
    }
}
