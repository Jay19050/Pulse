#include "AudioEngine.h"

#define NOMINMAX
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <propkey.h>
#include <functiondiscoverykeys_devpkey.h>
#include <ksmedia.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>

#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Avrt.lib")

namespace
{
    template <typename T>
    void safeRelease(T*& p)
    {
        if (p != nullptr)
        {
            p->Release();
            p = nullptr;
        }
    }

    const WAVEFORMATEXTENSIBLE* asExtensible(const WAVEFORMATEX* format) noexcept
    {
        if (format != nullptr && format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
            return reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(format);
        return nullptr;
    }
}

AudioEngine::AudioEngine()
{
    leftBuffer.reserve(8192);
    rightBuffer.reserve(8192);
}

AudioEngine::~AudioEngine()
{
    stop();
}

bool AudioEngine::start()
{
    stop();
    lastError.clear();
    deviceName.clear();
    initComplete.store(false);
    initSucceeded.store(false);
    running.store(true);

    captureThread = std::thread([this] { captureThreadMain(); });

    std::unique_lock<std::mutex> lock(initMutex);
    initCv.wait(lock, [this]
    {
        return initComplete.load();
    });

    if (!initSucceeded.load())
    {
        running.store(false);
        lock.unlock();
        if (captureThread.joinable())
            captureThread.join();
        return false;
    }

    return true;
}

void AudioEngine::stop()
{
    running.store(false);

    if (sampleReadyEvent != nullptr)
        SetEvent(static_cast<HANDLE>(sampleReadyEvent));

    if (captureThread.joinable())
        captureThread.join();

    // The capture thread owns and releases all WASAPI/COM objects.
    audioClient = nullptr;
    captureClient = nullptr;
    endpointDevice = nullptr;
    sampleReadyEvent = nullptr;
    numInputChannels.store(0);
}

bool AudioEngine::initialiseLoopback(void*& outAudioClient,
                                     void*& outCaptureClient,
                                     void*& outEndpointDevice,
                                     void*& outEvent)
{
    outAudioClient = nullptr;
    outCaptureClient = nullptr;
    outEndpointDevice = nullptr;
    outEvent = nullptr;

    IMMDeviceEnumerator* enumerator = nullptr;
    IMMDevice* endpoint = nullptr;
    IAudioClient* client = nullptr;
    IAudioCaptureClient* capture = nullptr;
    WAVEFORMATEX* mixFormat = nullptr;
    HANDLE eventHandle = nullptr;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                                  CLSCTX_ALL, IID_PPV_ARGS(&enumerator));
    if (FAILED(hr))
    {
        lastError = "CoCreateInstance(MMDeviceEnumerator) failed";
        goto fail;
    }

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &endpoint);
    if (FAILED(hr))
    {
        lastError = "Could not find the default Windows output device";
        goto fail;
    }

    {
        IPropertyStore* properties = nullptr;
        if (SUCCEEDED(endpoint->OpenPropertyStore(STGM_READ, &properties)))
        {
            PROPVARIANT value;
            PropVariantInit(&value);
            if (SUCCEEDED(properties->GetValue(PKEY_Device_FriendlyName, &value))
               && value.vt == VT_LPWSTR && value.pwszVal != nullptr)
            {
                deviceName = juce::String(value.pwszVal);
            }
            PropVariantClear(&value);
            safeRelease(properties);
        }
    }

    if (deviceName.isEmpty())
        deviceName = "Default Windows output";

    hr = endpoint->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
                            nullptr, reinterpret_cast<void**>(&client));
    if (FAILED(hr))
    {
        lastError = "Could not activate the Windows audio client";
        goto fail;
    }

    hr = client->GetMixFormat(&mixFormat);
    if (FAILED(hr) || mixFormat == nullptr)
    {
        lastError = "Could not obtain the Windows audio format";
        goto fail;
    }

    sampleRate.store(static_cast<double>(mixFormat->nSamplesPerSec));
    numInputChannels.store(static_cast<int>(mixFormat->nChannels));
    bitsPerSample = static_cast<int>(mixFormat->wBitsPerSample);
    bytesPerSample = juce::jmax(1, static_cast<int>(mixFormat->wBitsPerSample / 8));

    const auto* ext = asExtensible(mixFormat);
    if (ext != nullptr)
        isFloatFormat = (ext->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
    else
        isFloatFormat = (mixFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT);

    // Windows render-device loopback. This captures the audio that Windows
    // is rendering without opening an output stream of our own.
    constexpr REFERENCE_TIME bufferDuration = 1000000; // 100 ms

    hr = client->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        bufferDuration,
        0,
        mixFormat,
        nullptr);

    if (FAILED(hr))
    {
        lastError = "IAudioClient::Initialize(loopback) failed";
        goto fail;
    }

    eventHandle = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (eventHandle == nullptr)
    {
        lastError = "Could not create the WASAPI event";
        goto fail;
    }

    hr = client->SetEventHandle(eventHandle);
    if (FAILED(hr))
    {
        lastError = "Could not attach the WASAPI event";
        goto fail;
    }

    hr = client->GetService(IID_PPV_ARGS(&capture));
    if (FAILED(hr))
    {
        lastError = "Could not obtain the WASAPI capture service";
        goto fail;
    }

    outAudioClient = client;
    outCaptureClient = capture;
    outEndpointDevice = endpoint;
    outEvent = eventHandle;

    client = nullptr;
    capture = nullptr;
    endpoint = nullptr;
    eventHandle = nullptr;

    CoTaskMemFree(mixFormat);
    safeRelease(enumerator);
    return true;

fail:
    if (mixFormat != nullptr)
        CoTaskMemFree(mixFormat);
    if (eventHandle != nullptr)
        CloseHandle(eventHandle);
    safeRelease(capture);
    safeRelease(client);
    safeRelease(endpoint);
    safeRelease(enumerator);
    return false;
}

void AudioEngine::releaseLoopback(void*& audioClientPtr,
                                  void*& captureClientPtr,
                                  void*& endpointDevicePtr,
                                  void*& eventPtr)
{
    auto* capture = reinterpret_cast<IAudioCaptureClient*>(captureClientPtr);
    auto* client = reinterpret_cast<IAudioClient*>(audioClientPtr);
    auto* endpoint = reinterpret_cast<IMMDevice*>(endpointDevicePtr);

    if (client != nullptr)
        client->Stop();

    safeRelease(capture);
    safeRelease(client);
    safeRelease(endpoint);

    if (eventPtr != nullptr)
        CloseHandle(static_cast<HANDLE>(eventPtr));

    audioClientPtr = nullptr;
    captureClientPtr = nullptr;
    endpointDevicePtr = nullptr;
    eventPtr = nullptr;
}

void AudioEngine::captureThreadMain()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool comInitialisedHere = SUCCEEDED(hr);

    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
    {
        lastError = "COM initialization failed";
        initSucceeded.store(false);
        initComplete.store(true);
        initCv.notify_one();
        running.store(false);
        return;
    }

    void* localAudioClient = nullptr;
    void* localCaptureClient = nullptr;
    void* localEndpoint = nullptr;
    void* localEvent = nullptr;

    const bool initialized = initialiseLoopback(
        localAudioClient, localCaptureClient, localEndpoint, localEvent);

    initSucceeded.store(initialized);
    initComplete.store(true);
    initCv.notify_one();

    if (!initialized)
    {
        running.store(false);
        if (comInitialisedHere)
            CoUninitialize();
        return;
    }

    // Publish the handles only after the waiting start() call has observed
    // successful initialization. The capture thread remains the owner.
    audioClient = localAudioClient;
    captureClient = localCaptureClient;
    endpointDevice = localEndpoint;
    sampleReadyEvent = localEvent;

    auto* client = reinterpret_cast<IAudioClient*>(localAudioClient);
    auto* capture = reinterpret_cast<IAudioCaptureClient*>(localCaptureClient);
    auto eventHandle = static_cast<HANDLE>(localEvent);

    DWORD taskIndex = 0;
    HANDLE avrtHandle = AvSetMmThreadCharacteristicsW(L"Pro Audio", &taskIndex);

    hr = client->Start();
    if (FAILED(hr))
    {
        lastError = "WASAPI loopback could not start";
        running.store(false);
    }

    while (running.load())
    {
        const DWORD waitResult = WaitForSingleObject(eventHandle, 200);

        if (!running.load())
            break;

        if (waitResult != WAIT_OBJECT_0)
            continue;

        UINT32 packetLength = 0;
        if (FAILED(capture->GetNextPacketSize(&packetLength)))
            break;

        while (packetLength > 0 && running.load())
        {
            BYTE* data = nullptr;
            UINT32 frames = 0;
            DWORD flags = 0;
            UINT64 devicePosition = 0;
            UINT64 qpcPosition = 0;

            hr = capture->GetBuffer(&data, &frames, &flags,
                                    &devicePosition, &qpcPosition);
            if (FAILED(hr))
                break;

            processPacket(data, frames, flags);
            capture->ReleaseBuffer(frames);

            if (FAILED(capture->GetNextPacketSize(&packetLength)))
                packetLength = 0;
        }
    }

    if (client != nullptr)
        client->Stop();

    if (avrtHandle != nullptr)
        AvRevertMmThreadCharacteristics(avrtHandle);

    releaseLoopback(localAudioClient, localCaptureClient,
                    localEndpoint, localEvent);

    audioClient = nullptr;
    captureClient = nullptr;
    endpointDevice = nullptr;
    sampleReadyEvent = nullptr;
    numInputChannels.store(0);

    if (comInitialisedHere)
        CoUninitialize();
}

void AudioEngine::processPacket(const unsigned char* data,
                                unsigned int frames,
                                unsigned long flags)
{
    if (frames == 0)
        return;

    const int channels = juce::jmax(1, numInputChannels.load());
    leftBuffer.resize(frames);
    rightBuffer.resize(frames);

    if ((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0 || data == nullptr)
    {
        std::fill(leftBuffer.begin(), leftBuffer.end(), 0.0f);
        std::fill(rightBuffer.begin(), rightBuffer.end(), 0.0f);
    }
    else
    {
        for (UINT32 frame = 0; frame < frames; ++frame)
        {
            const BYTE* frameData = data + static_cast<size_t>(frame) *
                                    static_cast<size_t>(channels) *
                                    static_cast<size_t>(bytesPerSample);

            auto readSample = [&](int channel) -> float
            {
                const BYTE* p = frameData + static_cast<size_t>(channel) *
                                 static_cast<size_t>(bytesPerSample);

                if (isFloatFormat && bitsPerSample == 32)
                {
                    float value = 0.0f;
                    std::memcpy(&value, p, sizeof(value));
                    return value;
                }

                if (bitsPerSample == 16)
                {
                    int16_t value = 0;
                    std::memcpy(&value, p, sizeof(value));
                    return static_cast<float>(value) / 32768.0f;
                }

                if (bitsPerSample == 24)
                {
                    const int32_t value = static_cast<int32_t>(p[0]) |
                                          (static_cast<int32_t>(p[1]) << 8) |
                                          (static_cast<int32_t>(p[2]) << 16);
                    const int32_t signedValue = (value & 0x00800000) != 0
                                               ? value | 0xff000000
                                               : value;
                    return static_cast<float>(signedValue) / 8388608.0f;
                }

                if (bitsPerSample == 32)
                {
                    int32_t value = 0;
                    std::memcpy(&value, p, sizeof(value));
                    return static_cast<float>(value) / 2147483648.0f;
                }

                return 0.0f;
            };

            const float left = readSample(0);
            const float right = channels > 1 ? readSample(1) : left;

            leftBuffer[frame] = juce::jlimit(-1.0f, 1.0f, left);
            rightBuffer[frame] = juce::jlimit(-1.0f, 1.0f, right);
        }
    }

    BlockCallback callbackCopy;
    {
        const juce::ScopedLock lock(callbackLock);
        callbackCopy = blockCallback;
    }

    if (callbackCopy != nullptr)
    {
        const float* channelData[2] = { leftBuffer.data(), rightBuffer.data() };
        callbackCopy(channelData, channels > 1 ? 2 : 1,
                     static_cast<int>(frames), sampleRate.load());
    }
}

juce::String AudioEngine::getDeviceName() const
{
    const juce::ScopedLock lock(callbackLock);
    return deviceName;
}

juce::String AudioEngine::getLastError() const
{
    const juce::ScopedLock lock(callbackLock);
    return lastError;
}

void AudioEngine::setBlockCallback(BlockCallback callback)
{
    const juce::ScopedLock lock(callbackLock);
    blockCallback = std::move(callback);
}
