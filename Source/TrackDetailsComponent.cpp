#include "TrackDetailsComponent.h"

namespace
{
    constexpr float kRadius = 8.0f;
    constexpr float kCompactWidth = 150.0f;
    constexpr float kHeaderHeight = 30.0f;
    constexpr float kExpandedMaxWidth = 340.0f;
    constexpr float kExpandedHeight = 132.0f;

    const juce::Colour panel  = juce::Colour(PulseTheme::SurfaceRaised);
    const juce::Colour border = juce::Colour(PulseTheme::Outline);
    const juce::Colour accent = juce::Colour(PulseTheme::Accent);
}

TrackDetailsComponent::TrackDetailsComponent()
{
    setOpaque(false);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    setVisible(false);
    startTimerHz(30);
}

void TrackDetailsComponent::setNowPlaying(const NowPlayingManager::Info& info)
{
    const bool valid = info.valid && (info.title.isNotEmpty() || info.artist.isNotEmpty());
    const bool trackChanged = title != info.title || artist != info.artist;

    title = info.title;
    artist = info.artist;
    metadataValid = valid;

    if (trackChanged)
    {
        analysis = {};
        expanded = false;
        targetExpansion = 0.0f;
    }

    targetVisibility = valid ? 1.0f : 0.0f;

    if (valid)
        setVisible(true);

    repaint();
}

void TrackDetailsComponent::setAnalysis(const TrackAnalysis::Snapshot& snapshot)
{
    analysis = snapshot;
    repaint();
}

void TrackDetailsComponent::timerCallback()
{
    visibility += (targetVisibility - visibility) * 0.20f;
    expansion += (targetExpansion - expansion) * 0.20f;

    if (std::abs(visibility - targetVisibility) < 0.01f)
        visibility = targetVisibility;

    if (std::abs(expansion - targetExpansion) < 0.01f)
        expansion = targetExpansion;

    if (visibility <= 0.0f && targetVisibility <= 0.0f)
    {
        visibility = 0.0f;
        expanded = false;
        targetExpansion = 0.0f;

        if (isVisible())
            setVisible(false);
    }

    repaint();
}

void TrackDetailsComponent::toggleExpanded()
{
    if (! metadataValid)
        return;

    expanded = ! expanded;
    targetExpansion = expanded ? 1.0f : 0.0f;
    repaint();
}

bool TrackDetailsComponent::hitTest(int x, int y)
{
    if (! metadataValid || visibility <= 0.01f)
        return false;

    // The component can occupy the full upper-right layout area, but the
    // collapsed card itself remains narrow. This keeps only the visible card
    // clickable instead of making the whole invisible layout area a hit target.
    const float expandedWidth = juce::jmin(kExpandedMaxWidth, (float) getWidth());
    const float width = juce::jmap(expansion, 0.0f, 1.0f,
                                   juce::jmin(kCompactWidth, expandedWidth),
                                   expandedWidth);

    const float height = juce::jmap(expansion, 0.0f, 1.0f,
                                    kHeaderHeight,
                                    juce::jmin(kExpandedHeight, (float) getHeight()));

    const int left = getWidth() - juce::roundToInt(width);

    return juce::Rectangle<int>(left, 0,
                                juce::roundToInt(width),
                                juce::roundToInt(height)).contains(x, y);
}

void TrackDetailsComponent::mouseUp(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    toggleExpanded();
}

void TrackDetailsComponent::drawMetric(juce::Graphics& g,
                                        juce::Rectangle<float> bounds,
                                        const juce::String& label,
                                        const juce::String& value,
                                        float amount) const
{
    auto bar = bounds.removeFromBottom(3.0f);

    g.setColour(border.withAlpha(0.70f));
    g.fillRoundedRectangle(bar, 1.5f);

    g.setColour(accent.withAlpha(0.72f));
    g.fillRoundedRectangle(
        bar.withWidth(bar.getWidth() * juce::jlimit(0.0f, 1.0f, amount)), 1.5f);

    g.setColour(juce::Colour(PulseTheme::MutedText));
    g.setFont(juce::Font(9.0f));
    g.drawText(label, bounds.toNearestInt(),
               juce::Justification::topLeft, false);

    g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(0.92f));
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText(value, bounds.toNearestInt(),
               juce::Justification::topRight, false);
}

void TrackDetailsComponent::paint(juce::Graphics& g)
{
    if (visibility <= 0.0f)
        return;

    const float a = visibility;

    // The parent component may be wide enough for the expanded card.
    // The actual visible card is deliberately narrow while collapsed.
    const float expandedWidth = juce::jmin(kExpandedMaxWidth, (float) getWidth());
    const float compactWidth = juce::jmin(kCompactWidth, expandedWidth);

    const float cardWidth = juce::jmap(expansion, 0.0f, 1.0f,
                                       compactWidth, expandedWidth);

    const float availableHeight = (float) getHeight();
    const float expandedHeight = juce::jmin(kExpandedHeight, availableHeight);

    const float cardHeight = juce::jmap(expansion, 0.0f, 1.0f,
                                        kHeaderHeight, expandedHeight);

    auto card = juce::Rectangle<float>(
        getWidth() - cardWidth,
        0.0f,
        cardWidth,
        cardHeight).reduced(1.0f);

    g.setColour(panel.withAlpha(0.96f * a));
    g.fillRoundedRectangle(card, kRadius);

    g.setColour(border.withAlpha(0.80f * a));
    g.drawRoundedRectangle(card, kRadius, 1.0f);

    auto headerArea = card.removeFromTop(kHeaderHeight).reduced(9.0f, 0.0f);

    g.setColour(juce::Colour(PulseTheme::MutedText).withAlpha(a));
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.drawText("TRACK DETAILS",
               headerArea.toNearestInt(),
               juce::Justification::centredLeft,
               false);

    // Draw the chevron ourselves so it is consistent on every Windows font setup.
    const float cx = headerArea.getRight() - 4.0f;
    const float cy = headerArea.getCentreY();

    juce::Path chevron;

    if (expanded || expansion > 0.5f)
    {
        chevron.startNewSubPath(cx - 4.0f, cy + 2.0f);
        chevron.lineTo(cx, cy - 2.0f);
        chevron.lineTo(cx + 4.0f, cy + 2.0f);
    }
    else
    {
        chevron.startNewSubPath(cx - 4.0f, cy - 2.0f);
        chevron.lineTo(cx, cy + 2.0f);
        chevron.lineTo(cx + 4.0f, cy - 2.0f);
    }

    g.setColour(accent.withAlpha(a));
    g.strokePath(chevron,
                 juce::PathStrokeType(1.4f,
                                      juce::PathStrokeType::curved,
                                      juce::PathStrokeType::rounded));

    // In the compact state, only the small header is visible.
    if (cardHeight <= kHeaderHeight + 8.0f)
        return;

    auto content = card.reduced(9.0f, 5.0f);
    content.removeFromTop(1.0f);

    const float gap = 8.0f;

    auto split = [gap](juce::Rectangle<float> r)
    {
        const float w = (r.getWidth() - gap) * 0.5f;
        const float x = r.getX();

        return std::array<juce::Rectangle<float>, 2>{
            juce::Rectangle<float>(x, r.getY(), w, r.getHeight()),
            juce::Rectangle<float>(x + w + gap, r.getY(), w, r.getHeight())
        };
    };

    auto row1 = content.removeFromTop(28.0f);
    auto row2 = content.removeFromTop(28.0f);
    auto row3 = content.removeFromTop(28.0f);

    const auto r1 = split(row1);
    const auto r2 = split(row2);
    const auto r3 = split(row3);

    const juce::String bpmText = analysis.hasBpm
        ? juce::String(juce::roundToInt(analysis.bpm)) + " BPM"
        : "Analyzing...";

    drawMetric(g, r1[0], "BPM", bpmText,
               analysis.hasBpm ? 1.0f : 0.18f);

    drawMetric(g, r1[1], "ENERGY",
               juce::String(juce::roundToInt(analysis.energy * 100.0f)) + "%",
               analysis.energy);

    drawMetric(g, r2[0], "STEREO",
               juce::String(juce::roundToInt(analysis.stereoWidth * 100.0f)) + "%",
               analysis.stereoWidth);

    drawMetric(g, r2[1], "PEAK",
               juce::String(analysis.peakDb, 1) + " dB",
               juce::jmap(analysis.peakDb, -60.0f, 0.0f, 0.0f, 1.0f));

    drawMetric(g, r3[0], "BASS",
               juce::String(juce::roundToInt(analysis.bass * 100.0f)) + "%",
               analysis.bass);

    drawMetric(g, r3[1], "MID / TREBLE",
               juce::String(juce::roundToInt(analysis.mid * 100.0f)) + "% / "
                   + juce::String(juce::roundToInt(analysis.treble * 100.0f)) + "%",
               juce::jmax(analysis.mid, analysis.treble));

    if (analysis.beatPulse > 0.01f)
    {
        g.setColour(accent.withAlpha(0.12f * analysis.beatPulse * a));
        g.fillRoundedRectangle(card.reduced(2.0f), kRadius - 2.0f);
    }
}

void TrackDetailsComponent::resized()
{
}
