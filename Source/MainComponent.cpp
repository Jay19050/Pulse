#include "MainComponent.h"
#include "PulseTheme.h"

namespace
{
    constexpr int kHeaderHeight   = 56;
    constexpr int kModeBarHeight  = 84;
    constexpr int kStereoRowHeight = 140;
    constexpr int kSettingsWidth  = 300;
    constexpr int kGap = 12;
}

MainComponent::MainComponent()
{
    setSize(1180, 720);

    header.setDeviceInfo("starting audio capture...");
    header.onSettingsClicked = [this] { setSettingsPanelOpen(! settingsOpen); };

    settingsPanel.onCloseClicked = [this] { setSettingsPanelOpen(false); };

    visualizer.onResetPeaks = [this] { analyzer.resetPeakHold(); };

    audio.setBlockCallback([this](const float* const* channels,
                                   int numChannels,
                                   int numSamples,
                                   double /*sampleRate*/)
    {
        if (numChannels <= 0 || channels == nullptr || channels[0] == nullptr)
            return;

        const float* left = channels[0];
        const float* right = numChannels > 1 && channels[1] != nullptr
                           ? channels[1]
                           : channels[0];

        analyzer.pushStereo(left, right, numSamples);
        waveform.push(left, right, numSamples);
        goniometer.pushStereo(left, right, numSamples);
        levelMeter.pushStereo(left, right, numSamples);
    });

    addAndMakeVisible(header);
    addAndMakeVisible(visualizer);
    addAndMakeVisible(goniometer);
    addAndMakeVisible(levelMeter);
    addAndMakeVisible(modeSelector);
    // Settings panel is added on demand in setSettingsPanelOpen() so it never
    // eats mouse events over the visualizer while closed.

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
        header.setDeviceInfo(audio.getLastError());
        return;
    }

    analyzer.prepare(audio.getSampleRate());
    visualizer.prepare(audio.getSampleRate());
    audioActive.store(true);

    const juce::String deviceName = audio.getDeviceName();
    header.setDeviceInfo(deviceName + "  |  " + juce::String(audio.getSampleRate(), 0) + " Hz");
    settingsPanel.setAudioInfo(deviceName, audio.getSampleRate());
}

void MainComponent::stopAudio()
{
    audio.stop();
    audioActive.store(false);
}

void MainComponent::timerCallback()
{
    auto snapshot = analyzer.getSnapshot();

    // ~50ms window - enough to read as a real waveform, short enough that
    // Waveform mode's per-pixel min/max columns stay dense on screen.
    const int waveformSamples = juce::jmax(256, (int) (audio.getSampleRate() * 0.05));
    visualizer.updateData(snapshot, waveform.getRecent(waveformSamples));

    goniometer.setActive(snapshot.active);
    levelMeter.setActive(snapshot.active);
    header.setActive(snapshot.active);

    if (!snapshot.active && audioActive.load())
        header.setDeviceInfo("listening for audio...");
    else if (snapshot.active && audioActive.load())
        header.setDeviceInfo(audio.getDeviceName() + "  |  " + juce::String(audio.getSampleRate(), 0) + " Hz");

    repaint();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(PulseTheme::WindowBackground));
}

juce::Rectangle<int> MainComponent::settingsPanelBounds() const
{
    return { getWidth() - kSettingsWidth, kHeaderHeight, kSettingsWidth, getHeight() - kHeaderHeight };
}

void MainComponent::setSettingsPanelOpen(bool open)
{
    if (settingsOpen == open)
        return;

    settingsOpen = open;

    auto& animator = juce::Desktop::getInstance().getAnimator();
    const auto target = settingsPanelBounds();

    if (open)
    {
        addAndMakeVisible(settingsPanel);
        settingsPanel.setBounds(target.withX(getWidth()));
        animator.animateComponent(&settingsPanel, target, 1.0f, 160, false, 3.0, 0.0);
    }
    else
    {
        animator.animateComponent(&settingsPanel, target.withX(getWidth()), 0.0f, 140, false, 3.0, 1.0);
        // Removed from the hierarchy once the slide-out finishes, via a timer
        // one-shot rather than a ComponentAnimator callback (kept simple -
        // this is a rare, deliberate UI action, not a hot path). Guarded with
        // a SafePointer in case MainComponent is destroyed mid-animation.
        juce::Component::SafePointer<MainComponent> safeThis(this);
        juce::Timer::callAfterDelay(150, [safeThis]
        {
            if (safeThis != nullptr && ! safeThis->settingsOpen)
                safeThis->removeChildComponent(&safeThis->settingsPanel);
        });
    }
}

void MainComponent::resized()
{
    auto area = getLocalBounds();

    header.setBounds(area.removeFromTop(kHeaderHeight));

    auto modeBarArea = area.removeFromBottom(kModeBarHeight);
    modeBarArea.reduce(18, 0);
    modeBarArea.removeFromBottom(14);
    modeSelector.setBounds(modeBarArea);

    area = area.reduced(18, 0);
    area.removeFromTop(14);
    area.removeFromBottom(14);

    auto stereoRow = area.removeFromBottom(kStereoRowHeight);
    area.removeFromBottom(kGap);

    auto gonArea = stereoRow.removeFromLeft(kStereoRowHeight);
    stereoRow.removeFromLeft(kGap);
    goniometer.setBounds(gonArea);
    levelMeter.setBounds(stereoRow);

    visualizer.setBounds(area);

    if (settingsOpen)
        settingsPanel.setBounds(settingsPanelBounds());
}
