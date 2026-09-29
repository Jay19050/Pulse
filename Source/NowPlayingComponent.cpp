#include "NowPlayingComponent.h"

NowPlayingComponent::NowPlayingComponent()
{
    setOpaque(false);
    setInterceptsMouseClicks(false, false);
    setVisible(false);
    startTimerHz(30);
}

void NowPlayingComponent::setInfo(const NowPlayingManager::Info& info)
{
    const bool hasMetadata = info.valid && (info.title.isNotEmpty() || info.artist.isNotEmpty());
    const bool changed = title != info.title || artist != info.artist || playing != info.playing;

    if (! changed && hasMetadata == (targetAlpha > 0.0f))
        return;

    title = info.title;
    artist = info.artist;
    playing = info.playing;
    targetAlpha = hasMetadata ? 1.0f : 0.0f;

    if (hasMetadata)
        setVisible(true);

    repaint();
}

void NowPlayingComponent::timerCallback()
{
    const float speed = 0.18f;
    alpha += (targetAlpha - alpha) * speed;

    if (std::abs(alpha - targetAlpha) < 0.01f)
        alpha = targetAlpha;

    if (alpha <= 0.0f && targetAlpha <= 0.0f)
    {
        alpha = 0.0f;
        if (isVisible())
            setVisible(false);
        return;
    }

    repaint();
}

void NowPlayingComponent::paint(juce::Graphics& g)
{
    if (alpha <= 0.0f)
        return;

    auto area = getLocalBounds().toFloat().reduced(2.0f, 2.0f);

    // The metadata is deliberately quiet: it should sit above the spectrum
    // without becoming a second focal point.
    auto titleArea = area.removeFromTop(area.getHeight() * 0.56f);
    auto artistArea = area;

    const auto titleColour = juce::Colour(PulseTheme::DefaultText).withAlpha(0.94f * alpha);
    const auto artistColour = juce::Colour(PulseTheme::MutedText).withAlpha((playing ? 0.90f : 0.55f) * alpha);

    g.setColour(titleColour);
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.drawFittedText(title.isNotEmpty() ? title : "Unknown title",
                     titleArea.toNearestInt(), juce::Justification::centredRight,
                     1, 0.82f);

    g.setColour(artistColour);
    g.setFont(juce::Font(11.0f));
    g.drawFittedText(artist.isNotEmpty() ? artist : "Unknown artist",
                     artistArea.toNearestInt(), juce::Justification::centredRight,
                     1, 0.82f);
}

void NowPlayingComponent::resized()
{
}
