#include "MainComponent.h"
#include "PulseTheme.h"

MainComponent::MainComponent()
{
    setSize(1100, 680);

    status.setText("PULSE  |  starting audio capture...", juce::dontSendNotification);
    status.setColour(juce::Label::textColourId, juce::Colour(PulseTheme::DefaultText));
    status.setFont(juce::Font(13.0f));
    status.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(status);

    visualizer.onResetPeaks = [this] { analyzer.resetPeakHold(); };

    audio.setBlockCallback([this](const float* const* channels,
                                   int numChannels,
                                   int numSamples,
                                   double sampleRate)
    {
        if (numChannels <= 0 || channels == nullptr || channels[0] == nullptr)
            return;

        const float* left = channels[0];
        const float* right = numChannels > 1 && channels[1] != nullptr
                           ? channels[1]
                           : channels[0];

        analyzer.pushStereo(left, right, numSamples);
        goniometer.pushStereo(left, right, numSamples);
        levelMeter.pushStereo(left, right, numSamples);
    });

    addAndMakeVisible(visualizer);
    addAndMakeVisible(goniometer);
    addAndMakeVisible(levelMeter);

    startAudio();
    startTimerHz(30);
}

MainComponent::~MainComponent()
{
    stopTimer();
    stopAudio();
}

void MainComponent::startAudio()
{
    if (!audio.start())
    {
        audioActive.store(false);
        status.setText("PULSE  |  " + audio.getLastError(), juce::dontSendNotification);
        return;
    }

    analyzer.prepare(audio.getSampleRate());
    audioActive.store(true);

    status.setText("PULSE  |  " + audio.getDeviceName()
                 + "  |  " + juce::String(audio.getSampleRate(), 0) + " Hz",
                   juce::dontSendNotification);
}

void MainComponent::stopAudio()
{
    audio.stop();
    audioActive.store(false);
}

void MainComponent::timerCallback()
{
    auto snapshot = analyzer.getSnapshot();

    visualizer.setSnapshot(snapshot);
    goniometer.setActive(snapshot.active);
    levelMeter.setActive(snapshot.active);

    if (!snapshot.active)
        status.setText("PULSE  |  listening for audio...", juce::dontSendNotification);

    repaint();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(PulseTheme::WindowBackground));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(18);

    auto footer = area.removeFromBottom(28);
    status.setBounds(footer);

    auto main = area;

    // The large spectrum dominates; a fixed-height bottom strip holds the
    // square goniometer on the left and the L/R level meter filling the rest.
    auto bottomStrip = main.removeFromBottom(150);
    main.removeFromBottom(12);

    auto gonArea = bottomStrip.removeFromLeft(150);
    bottomStrip.removeFromLeft(12);
    goniometer.setBounds(gonArea);
    levelMeter.setBounds(bottomStrip);

    visualizer.setBounds(main);
}