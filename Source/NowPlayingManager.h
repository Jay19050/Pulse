#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

// Retrieves metadata from the Windows system media session layer.
//
// This class deliberately has no dependency on AudioEngine/SpectrumAnalyzer:
// metadata retrieval is completely separate from the real-time audio path.
class NowPlayingManager final
{
public:
    struct Info
    {
        juce::String title;
        juce::String artist;
        bool playing = false;
        bool valid = false;

        bool operator== (const Info& other) const noexcept
        {
            return title == other.title
                && artist == other.artist
                && playing == other.playing
                && valid == other.valid;
        }

        bool operator!= (const Info& other) const noexcept { return ! (*this == other); }
    };

    NowPlayingManager();
    ~NowPlayingManager();

    // Starts a lightweight worker which checks Windows media-session state.
    // The callback is always delivered on JUCE's message thread.
    void start();
    void stop();

    std::function<void(const Info&)> onChanged;

private:
    void workerMain();
    void publish(const Info& info);

    std::atomic<bool> running { false };
    std::thread worker;
    std::shared_ptr<std::atomic<bool>> alive;
    Info lastPublished;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NowPlayingManager)
};
