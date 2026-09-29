#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class AudioEngine final
{
public:
    using BlockCallback = std::function<void(const float* const*, int, int, double)>;

    struct DeviceInfo
    {
        juce::String id;    // WASAPI endpoint ID - stable, used to reopen this exact device
        juce::String name;  // human-readable friendly name, for display only
        bool isDefault = false;
    };

    AudioEngine();
    ~AudioEngine();

    bool start();
    void stop();

    bool isRunning() const noexcept { return running.load(); }
    double getSampleRate() const noexcept { return sampleRate.load(); }
    int getNumInputChannels() const noexcept { return numInputChannels.load(); }
    juce::String getDeviceName() const;
    juce::String getLastError() const;

    // Windows render/output devices Pulse can loopback-capture from. Static
    // and self-contained (own COM lifetime) so it can be called from the UI
    // thread without an AudioEngine instance - e.g. before the first start().
    static std::vector<DeviceInfo> enumerateOutputDevices();

    // Which endpoint to open on the next start()/restart. Empty string means
    // "the current Windows default render device". Does not itself restart
    // capture - callers (MainComponent) call stop()+start() (or just start(),
    // which already stops first) to apply it.
    void setOutputDeviceId(const juce::String& id);
    juce::String getOutputDeviceId() const;

    // MUST be called before start() (and never again afterward - not
    // reassignable while the capture thread may be running). This one-shot
    // contract is what lets the hot audio-callback path read blockCallback
    // with zero locking (see processPacket()): thread-creation in start()
    // is itself a synchronisation point, so a callback set beforehand is
    // guaranteed visible to the capture thread without a mutex.
    void setBlockCallback(BlockCallback callback);

private:
    void captureThreadMain();
    bool initialiseLoopback(void*& audioClient, void*& captureClient, void*& endpointDevice, void*& sampleReadyEvent);
    void releaseLoopback(void*& audioClient, void*& captureClient, void*& endpointDevice, void*& sampleReadyEvent);
    void processPacket(const unsigned char* data, unsigned int frames, unsigned long flags);

    // lastError/deviceName are written from the capture thread only in cold
    // paths (once during initialiseLoopback, or rarely on a disconnect) and
    // read from the UI thread via the locked getters above - these helpers
    // are what make that locking actually meaningful (previously the reads
    // were locked but the writes weren't, which protects nothing).
    void setLastError(juce::String message);
    void setDeviceName(juce::String name);

    void* audioClient = nullptr;
    void* captureClient = nullptr;
    void* endpointDevice = nullptr;
    void* sampleReadyEvent = nullptr;

    std::thread captureThread;

    juce::String deviceName;
    juce::String lastError;
    juce::String requestedDeviceId; // "" = system default; guarded by callbackLock
    mutable juce::CriticalSection callbackLock;

    // NOT guarded by callbackLock - see setBlockCallback()'s contract above.
    // processPacket() (the per-audio-buffer hot path) reads this directly,
    // with no lock, by design.
    BlockCallback blockCallback;

    std::atomic<bool> running { false };
    std::atomic<bool> initComplete { false };
    std::atomic<bool> initSucceeded { false };
    mutable std::mutex initMutex;
    std::condition_variable initCv;

    std::atomic<double> sampleRate { 48000.0 };
    std::atomic<int> numInputChannels { 0 };

    int bitsPerSample = 32;
    int bytesPerSample = 4;
    bool isFloatFormat = true;

    std::vector<float> leftBuffer;
    std::vector<float> rightBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
