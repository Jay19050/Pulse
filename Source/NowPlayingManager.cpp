#include "NowPlayingManager.h"

#if JUCE_WINDOWS
 #define WIN32_LEAN_AND_MEAN
 #include <windows.h>
 #include <winrt/Windows.Foundation.h>
 #include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Control.h>
 #include <winrt/base.h>
#endif

namespace
{
#if JUCE_WINDOWS
    using winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSession;
    using winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionManager;
    using winrt::Windows::Media::Control::GlobalSystemMediaTransportControlsSessionPlaybackStatus;

    juce::String toJuceString(const winrt::hstring& value)
    {
        return value.empty() ? juce::String() : juce::String(value.c_str());
    }

    bool isPlaying(const GlobalSystemMediaTransportControlsSession& session)
    {
        const auto playback = session.GetPlaybackInfo();
        return playback != nullptr
            && playback.PlaybackStatus() == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
    }

    bool isAppleMusic(const GlobalSystemMediaTransportControlsSession& session)
    {
        try
        {
            return juce::String(session.SourceAppUserModelId().c_str())
                .containsIgnoreCase("applemusic");
        }
        catch (...)
        {
            return false;
        }
    }

    bool isActiveSession(const GlobalSystemMediaTransportControlsSession& session)
    {
        const auto playback = session.GetPlaybackInfo();
        if (playback == nullptr)
            return false;

        const auto status = playback.PlaybackStatus();
        return status == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing
            || status == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Paused;
    }

    GlobalSystemMediaTransportControlsSession chooseSession(
        const GlobalSystemMediaTransportControlsSessionManager& manager)
    {
        // Prefer Apple Music explicitly when its SMTC session is available.
        // Apple does not document the exact AUMID, so match the stable
        // 'applemusic' portion rather than hard-coding a package suffix.
        if (auto current = manager.GetCurrentSession())
        {
            try
            {
                if (isAppleMusic(current) && isActiveSession(current))
                    return current;
            }
            catch (...) {}
        }

        try
        {
            for (const auto& candidate : manager.GetSessions())
            {
                try
                {
                    if (candidate && isAppleMusic(candidate) && isActiveSession(candidate))
                        return candidate;
                }
                catch (...) {}
            }

            // If Apple Music is not present, fall back to Windows' current
            // media session so the feature remains useful with other SMTC
            // players without adding another code path.
            if (auto current = manager.GetCurrentSession())
            {
                try
                {
                    if (isActiveSession(current))
                        return current;
                }
                catch (...) {}
            }

            // Last fallback: any playing session.
            for (const auto& candidate : manager.GetSessions())
            {
                try
                {
                    if (candidate && isPlaying(candidate))
                        return candidate;
                }
                catch (...) {}
            }
        }
        catch (...) {}

        return nullptr;
    }
#endif
}

NowPlayingManager::NowPlayingManager()
    : alive(std::make_shared<std::atomic<bool>>(true))
{
}

NowPlayingManager::~NowPlayingManager()
{
    if (alive != nullptr)
        alive->store(false);

    stop();
}

void NowPlayingManager::start()
{
    if (running.exchange(true))
        return;

    worker = std::thread([this]
    {
        workerMain();
    });
}

void NowPlayingManager::stop()
{
    running.store(false);

    if (worker.joinable())
        worker.join();
}

void NowPlayingManager::publish(const Info& info)
{
    if (info == lastPublished)
        return;

    lastPublished = info;

    auto aliveToken = alive;
    juce::MessageManager::callAsync([this, aliveToken, info]
    {
        if (!aliveToken->load())
            return;

        if (onChanged != nullptr)
            onChanged(info);
    });
}

void NowPlayingManager::workerMain()
{
#if JUCE_WINDOWS
    try
    {
        // Keep all WinRT/COM work on this dedicated worker. JUCE owns the
        // message thread and other parts of Pulse may already initialize COM
        // there with a different apartment model.
        winrt::init_apartment(winrt::apartment_type::multi_threaded);

        auto manager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
        Info lastObserved;

        while (running.load())
        {
            Info current;

            try
            {
                auto session = chooseSession(manager);

                if (session != nullptr)
                {
                    const auto playback = session.GetPlaybackInfo();
                    const auto status = playback != nullptr
                                      ? playback.PlaybackStatus()
                                      : GlobalSystemMediaTransportControlsSessionPlaybackStatus::Closed;

                    const bool playing = status == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
                    const bool paused = status == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Paused;

                    if (playing || paused)
                    {
                        const auto properties = session.TryGetMediaPropertiesAsync().get();

                        if (properties != nullptr)
                        {
                            current.title = toJuceString(properties.Title());
                            current.artist = toJuceString(properties.Artist());
                            current.playing = playing;
                            current.valid = current.title.isNotEmpty() || current.artist.isNotEmpty();
                        }
                    }
                }
            }
            catch (const winrt::hresult_error&)
            {
                // Media sessions can disappear while being queried. Treat that
                // as a transient condition and retry on the next pass.
                current = {};
            }
            catch (...)
            {
                current = {};
            }

            if (current != lastObserved)
            {
                lastObserved = current;
                publish(current);
            }

            // Song metadata changes slowly compared with the visualizer. A
            // 500 ms cadence keeps detection responsive without touching the
            // audio or rendering hot paths.
            for (int i = 0; i < 10 && running.load(); ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        winrt::uninit_apartment();
    }
    catch (const winrt::hresult_error&)
    {
        publish({});
        try { winrt::uninit_apartment(); } catch (...) {}
    }
    catch (...)
    {
        publish({});
        try { winrt::uninit_apartment(); } catch (...) {}
    }
#else
    publish({});
#endif
}
