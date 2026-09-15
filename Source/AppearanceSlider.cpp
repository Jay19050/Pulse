#include "AppearanceSlider.h"
#include "PulseTheme.h"

AppearanceSlider::AppearanceSlider(juce::String labelIn, float minValueIn, float maxValueIn, float initialValue)
    : label(std::move(labelIn)), minValue(minValueIn), maxValue(maxValueIn),
      value(juce::jlimit(minValueIn, maxValueIn, initialValue))
{
    setOpaque(false);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void AppearanceSlider::setValue(float newValue, bool notify)
{
    const float clamped = juce::jlimit(minValue, maxValue, newValue);
    if (clamped == value)
        return;

    value = clamped;
    repaint();

    if (notify && onValueChanged != nullptr)
        onValueChanged(value);
}

juce::Rectangle<float> AppearanceSlider::trackBounds() const
{
    // Track sits in the lower half of the component, below the label/value
    // text row - see resized()/paint() layout.
    auto area = getLocalBounds().toFloat();
    area.removeFromTop(16.0f);
    return area.withHeight(4.0f).withY(area.getY() + (area.getHeight() - 4.0f) * 0.5f).reduced(2.0f, 0.0f);
}

void AppearanceSlider::resized()
{
    // Nothing to lay out beyond what trackBounds()/paint() compute directly -
    // this control is small enough not to warrant child components.
}

void AppearanceSlider::setValueFromMouseX(float mouseX)
{
    const auto track = trackBounds();
    const float t = track.getWidth() > 0.0f
                  ? juce::jlimit(0.0f, 1.0f, (mouseX - track.getX()) / track.getWidth())
                  : 0.0f;

    setValue(minValue + t * (maxValue - minValue), true);
}

void AppearanceSlider::mouseDown(const juce::MouseEvent& e)
{
    dragging = true;
    setValueFromMouseX(e.position.x);
}

void AppearanceSlider::mouseDrag(const juce::MouseEvent& e)
{
    setValueFromMouseX(e.position.x);
}

void AppearanceSlider::mouseMove(const juce::MouseEvent&)
{
    if (! hover)
    {
        hover = true;
        repaint();
    }
}

void AppearanceSlider::mouseExit(const juce::MouseEvent&)
{
    if (hover && ! dragging)
    {
        hover = false;
        repaint();
    }
    dragging = false;
}

void AppearanceSlider::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    auto labelRow = area.removeFromTop(16.0f);

    g.setColour(juce::Colour(PulseTheme::DefaultText).withAlpha(0.85f));
    g.setFont(12.0f);
    g.drawText(label, labelRow, juce::Justification::centredLeft);

    g.setColour(juce::Colour(PulseTheme::MutedText));
    g.setFont(11.0f);
    const float t = maxValue > minValue ? (value - minValue) / (maxValue - minValue) : 0.0f;
    g.drawText(juce::String(value, 2), labelRow, juce::Justification::centredRight);

    const auto track = trackBounds();

    g.setColour(juce::Colour(PulseTheme::Outline));
    g.fillRoundedRectangle(track, track.getHeight() * 0.5f);

    auto activeTrack = track.withWidth(track.getWidth() * t);
    g.setColour(juce::Colour(PulseTheme::Accent).withAlpha(hover || dragging ? 1.0f : 0.85f));
    g.fillRoundedRectangle(activeTrack, track.getHeight() * 0.5f);

    const float thumbSize = dragging ? 12.0f : (hover ? 11.0f : 9.0f);
    const float thumbX = track.getX() + track.getWidth() * t;

    if (hover || dragging)
    {
        g.setColour(juce::Colour(PulseTheme::Accent).withAlpha(0.18f));
        g.fillEllipse(thumbX - thumbSize, track.getCentreY() - thumbSize, thumbSize * 2.0f, thumbSize * 2.0f);
    }

    g.setColour(juce::Colour(PulseTheme::Accent));
    g.fillEllipse(thumbX - thumbSize * 0.5f, track.getCentreY() - thumbSize * 0.5f, thumbSize, thumbSize);
}
