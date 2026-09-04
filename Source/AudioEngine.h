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

    AudioEngine();
    ~AudioEngine();

    bool start();
    void stop();

    bool isRunning() const noexcept { return running.load(); }
    double getSampleRate() const noexcept { return sampleRate.load(); }
    int getNumInputChannels() const noexcept { return numInputChannels.load(); }
    juce::String getDeviceName() const;
    juce::String getLastError() const;

    void setBlockCallback(BlockCallback callback);

private:
    void captureThreadMain();
    bool initialiseLoopback(void*& audioClient, void*& captureClient, void*& endpointDevice, void*& sampleReadyEvent);
    void releaseLoopback(void*& audioClient, void*& captureClient, void*& endpointDevice, void*& sampleReadyEvent);
    void processPacket(const unsigned char* data, unsigned int frames, unsigned long flags);

    void* audioClient = nullptr;
    void* captureClient = nullptr;
    void* endpointDevice = nullptr;
    void* sampleReadyEvent = nullptr;

    std::thread captureThread;

    juce::String deviceName;
    juce::String lastError;
    mutable juce::CriticalSection callbackLock;
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
