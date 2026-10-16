#include <Platform/PlatformAudio.h>
#include <windows.h>
#include <xaudio2.h>
#include <new>

namespace Platform::Audio {

    // =========================================================================
    // Module-Level Static State for Dynamic DLL Loading
    // =========================================================================
    static HMODULE               g_hXAudio2Dll = nullptr;
    typedef HRESULT(WINAPI* XAudio2CreateFn)(IXAudio2**, UINT32, XAUDIO2_PROCESSOR);
    static XAudio2CreateFn       g_XAudio2CreateFunc = nullptr;

    // =========================================================================
    // Sound Event Context (VirtualAlloc-backed doubly-linked node)
    // =========================================================================
    struct SoundEventContext
    {
        SoundSourceHandle     hSource;
        float                 timeSeconds;
        AudioCallbackFunction callback;
        void* pUserData;
        bool                  triggered;
        SoundEventContext* next;
        SoundEventContext* prev;
    };

    // =========================================================================
    // Native XAudio2 Voice Callback Bridge
    // =========================================================================
    struct AudioVoiceCallbackBridge : public IXAudio2VoiceCallback
    {
        SoundSourceHandle   hSourceOwner;
        SoundEventContext* pFirstEvent = nullptr;

        STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32) override {}
        STDMETHOD_(void, OnVoiceProcessingPassEnd)() override {}

        STDMETHOD_(void, OnStreamEnd)() override {}
        STDMETHOD_(void, OnBufferStart)(void*) override {}

        STDMETHOD_(void, OnBufferEnd)(void* pBufferContext) override
        {
            if (pBufferContext)
            {
                SoundEventContext* eventCtx = static_cast<SoundEventContext*>(pBufferContext);
                if (eventCtx && eventCtx->callback && !eventCtx->triggered)
                {
                    eventCtx->triggered = true;
                    eventCtx->callback(hSourceOwner, eventCtx->pUserData);
                }
            }
        }

        STDMETHOD_(void, OnLoopEnd)(void*) override
        {
            SoundEventContext* curr = pFirstEvent;
            while (curr)
            {
                curr->triggered = false;
                curr = curr->next;
            }
        }

        STDMETHOD_(void, OnVoiceError)(void*, HRESULT) override {}
    };

    // Forward declaration for buffer context pointer inside source context
    struct SoundBufferContext;

    // =========================================================================
    // Sound Source Context with Explicit Constructor & Placement New Support
    // =========================================================================
    struct SoundSourceContext
    {
        IXAudio2SourceVoice* pSourceVoice;
        SoundBufferContext* pBufferCtx;
        AudioVoiceCallbackBridge  callbackBridge;
        float                     channelVolumes[8];
        uint32                    channelCount;
        bool                      isLooping;

        SoundSourceContext()
        {
            pSourceVoice = nullptr;
            pBufferCtx = nullptr;
            channelCount = 0;
            isLooping = false;
            for (uint32 i = 0; i < 8; ++i) {
                channelVolumes[i] = 1.0f;
            }
            callbackBridge.hSourceOwner.ptr = this;
            callbackBridge.pFirstEvent = nullptr;
        }

        ~SoundSourceContext()
        {
            SoundEventContext* curr = callbackBridge.pFirstEvent;
            while (curr)
            {
                SoundEventContext* next = curr->next;
                ::VirtualFree(curr, 0, MEM_RELEASE);
                curr = next;
            }
        }
    };

    // =========================================================================
    // Kernel-Style Contexts (VirtualAlloc-backed)
    // =========================================================================
    struct AudioDeviceContext
    {
        IXAudio2* pXAudio2;
        IXAudio2MasteringVoice* pMasterVoice;
    };

    struct SoundBufferContext
    {
        void* pAudioData;
        uint32        dataSize;
        uint16        channels;
        uint32        sampleRate;
        uint16        bitsPerSample;
        uint64        totalSamples;
        eSampleFormat format;
    };

    // =========================================================================
    // Subsystem Lifecycle (DLL Loading & COM Initialization)
    // =========================================================================
    int32 Init()
    {
        HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        {
            return eResult::Error;
        }

        // Explicitly load XAudio2_9.dll at subsystem boot
        g_hXAudio2Dll = ::LoadLibraryW(L"XAudio2_9.dll");
        if (!g_hXAudio2Dll)
        {
            ::CoUninitialize();
            return eResult::DeviceNotFound;
        }

        g_XAudio2CreateFunc = (XAudio2CreateFn)::GetProcAddress(g_hXAudio2Dll, "XAudio2Create");
        if (!g_XAudio2CreateFunc)
        {
            ::FreeLibrary(g_hXAudio2Dll);
            g_hXAudio2Dll = nullptr;
            ::CoUninitialize();
            return eResult::DeviceNotFound;
        }

        return eResult::OK;
    }

    void Fini()
    {
        if (g_hXAudio2Dll)
        {
            ::FreeLibrary(g_hXAudio2Dll);
            g_hXAudio2Dll = nullptr;
            g_XAudio2CreateFunc = nullptr;
        }

        ::CoUninitialize();
    }

    // =========================================================================
    // Audio Device API
    // =========================================================================
    int32 CreateDevice(const AudioDeviceCreateInfo& createInfo, AudioDeviceHandle& hDevice)
    {
        hDevice.ptr = nullptr;

        if (!g_XAudio2CreateFunc)
        {
            return eResult::NotInitialized;
        }

        AudioDeviceContext* ctx = static_cast<AudioDeviceContext*>(
            ::VirtualAlloc(nullptr, sizeof(AudioDeviceContext), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
            );
        if (!ctx) return eResult::OutOfMemory;

        *ctx = AudioDeviceContext{};

        HRESULT hr = g_XAudio2CreateFunc(&(ctx->pXAudio2), 0, XAUDIO2_DEFAULT_PROCESSOR);
        if (FAILED(hr) || !ctx->pXAudio2)
        {
            ::VirtualFree(ctx, 0, MEM_RELEASE);
            return eResult::DeviceNotFound;
        }

        hr = ctx->pXAudio2->CreateMasteringVoice(&(ctx->pMasterVoice), createInfo.channels, createInfo.sampleRate);
        if (FAILED(hr) || !ctx->pMasterVoice)
        {
            ctx->pXAudio2->Release();
            ::VirtualFree(ctx, 0, MEM_RELEASE);
            return eResult::Error;
        }

        hDevice.ptr = ctx;
        return eResult::OK;
    }

    void DestroyDevice(AudioDeviceHandle hDevice)
    {
        if (!hDevice.ptr) return;
        AudioDeviceContext* ctx = static_cast<AudioDeviceContext*>(hDevice.ptr);

        if (ctx->pMasterVoice) { ctx->pMasterVoice->DestroyVoice(); }
        if (ctx->pXAudio2) { ctx->pXAudio2->Release(); }
        ::VirtualFree(ctx, 0, MEM_RELEASE);
    }

    // =========================================================================
    // Sound Buffer API
    // =========================================================================
    int32 CreateBuffer(AudioDeviceHandle hDevice, const SoundBufferCreateInfo& createInfo, SoundBufferHandle& hBuffer)
    {
        hBuffer.ptr = nullptr;
        if (!hDevice.ptr || !createInfo.pAudioData || createInfo.dataSize == 0) return eResult::InvalidArgument;

        uint16 bitsPerSample = (createInfo.format == eSampleFormat::Float32) ? 32 : 16;
        uint64 totalAllocSize = sizeof(SoundBufferContext) + createInfo.dataSize;

        uint8* rawAlloc = static_cast<uint8*>(
            ::VirtualAlloc(nullptr, static_cast<SIZE_T>(totalAllocSize), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
            );
        if (!rawAlloc) return eResult::OutOfMemory;

        SoundBufferContext* ctx = reinterpret_cast<SoundBufferContext*>(rawAlloc);
        void* destAudioData = rawAlloc + sizeof(SoundBufferContext);

        ::memcpy(destAudioData, createInfo.pAudioData, createInfo.dataSize);

        ctx->pAudioData = destAudioData;
        ctx->dataSize = createInfo.dataSize;
        ctx->channels = createInfo.channels;
        ctx->sampleRate = createInfo.sampleRate;
        ctx->bitsPerSample = bitsPerSample;
        ctx->format = createInfo.format;

        uint32 blockAlign = (createInfo.channels * bitsPerSample) / 8;
        ctx->totalSamples = (blockAlign > 0) ? (createInfo.dataSize / blockAlign) : 0;

        hBuffer.ptr = ctx;
        return eResult::OK;
    }

    void DestroyBuffer(SoundBufferHandle hBuffer)
    {
        if (!hBuffer.ptr) return;
        ::VirtualFree(hBuffer.ptr, 0, MEM_RELEASE);
    }

    // =========================================================================
    // Sound Source API (Placement New + Explicit vtable binding)
    // =========================================================================
    int32 CreateSource(AudioDeviceHandle hDevice, SoundBufferHandle hBuffer, SoundSourceHandle& hSource)
    {
        hSource.ptr = nullptr;
        if (!hDevice.ptr || !hBuffer.ptr) return eResult::InvalidArgument;

        AudioDeviceContext* deviceCtx = static_cast<AudioDeviceContext*>(hDevice.ptr);
        SoundBufferContext* bufferCtx = static_cast<SoundBufferContext*>(hBuffer.ptr);

        void* rawAlloc = ::VirtualAlloc(nullptr, sizeof(SoundSourceContext), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!rawAlloc) return eResult::OutOfMemory;

        SoundSourceContext* sourceCtx = new (rawAlloc) SoundSourceContext();
        sourceCtx->pBufferCtx = bufferCtx;
        sourceCtx->channelCount = bufferCtx->channels;

        WAVEFORMATEX wfx{};
        wfx.wFormatTag = (bufferCtx->format == eSampleFormat::Float32) ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
        wfx.nChannels = bufferCtx->channels;
        wfx.nSamplesPerSec = bufferCtx->sampleRate;
        wfx.wBitsPerSample = bufferCtx->bitsPerSample;
        wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
        wfx.cbSize = 0;

        HRESULT hr = deviceCtx->pXAudio2->CreateSourceVoice(
            &(sourceCtx->pSourceVoice),
            &wfx,
            0,
            XAUDIO2_DEFAULT_FREQ_RATIO,
            &(sourceCtx->callbackBridge)
        );

        if (FAILED(hr) || !sourceCtx->pSourceVoice)
        {
            sourceCtx->~SoundSourceContext();
            ::VirtualFree(sourceCtx, 0, MEM_RELEASE);
            return eResult::Error;
        }

        hSource.ptr = sourceCtx;
        return eResult::OK;
    }

    void DestroySource(SoundSourceHandle hSource)
    {
        if (!hSource.ptr) return;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);

        if (sourceCtx->pSourceVoice)
        {
            sourceCtx->pSourceVoice->Stop();
            sourceCtx->pSourceVoice->DestroyVoice();
            sourceCtx->pSourceVoice = nullptr;
        }

        sourceCtx->~SoundSourceContext();
        ::VirtualFree(sourceCtx, 0, MEM_RELEASE);
    }

    // =========================================================================
    // Sound Source Control Functions
    // =========================================================================
    int32 Play(SoundSourceHandle hSource)
    {
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice || !sourceCtx->pBufferCtx) return eResult::InvalidArgument;

        sourceCtx->pSourceVoice->Stop();
        sourceCtx->pSourceVoice->FlushSourceBuffers();

        SoundBufferContext* bufCtx = sourceCtx->pBufferCtx;
        uint32 blockAlign = (bufCtx->channels * bufCtx->bitsPerSample) / 8;
        if (blockAlign == 0) return eResult::InvalidFormat;

        SoundEventContext* curr = sourceCtx->callbackBridge.pFirstEvent;
        while (curr)
        {
            curr->triggered = false;
            curr = curr->next;
        }

        if (!sourceCtx->callbackBridge.pFirstEvent)
        {
            XAUDIO2_BUFFER xbuf{};
            xbuf.pAudioData = static_cast<const BYTE*>(bufCtx->pAudioData);
            xbuf.AudioBytes = bufCtx->dataSize;
            xbuf.Flags = XAUDIO2_END_OF_STREAM;
            xbuf.LoopCount = sourceCtx->isLooping ? XAUDIO2_LOOP_INFINITE : 0;

            HRESULT hr = sourceCtx->pSourceVoice->SubmitSourceBuffer(&xbuf);
            return SUCCEEDED(hr) ? (sourceCtx->pSourceVoice->Start() == S_OK ? eResult::OK : eResult::Error) : eResult::Error;
        }

        const uint8* currentAudioPtr = static_cast<const uint8*>(bufCtx->pAudioData);
        uint32 remainingBytes = bufCtx->dataSize;
        uint32 currentOffsetBytes = 0;

        curr = sourceCtx->callbackBridge.pFirstEvent;
        while (curr)
        {
            uint32 targetByteOffset = static_cast<uint32>(curr->timeSeconds * static_cast<float>(bufCtx->sampleRate)) * blockAlign;
            targetByteOffset = (targetByteOffset / blockAlign) * blockAlign;

            if (targetByteOffset > bufCtx->dataSize) targetByteOffset = bufCtx->dataSize;
            if (targetByteOffset < currentOffsetBytes) targetByteOffset = currentOffsetBytes;

            uint32 chunkBytes = targetByteOffset - currentOffsetBytes;

            if (chunkBytes > 0)
            {
                XAUDIO2_BUFFER xbuf{};
                xbuf.pAudioData = currentAudioPtr;
                xbuf.AudioBytes = chunkBytes;
                xbuf.pContext = curr;

                sourceCtx->pSourceVoice->SubmitSourceBuffer(&xbuf);

                currentAudioPtr += chunkBytes;
                remainingBytes -= chunkBytes;
                currentOffsetBytes += chunkBytes;
            }
            curr = curr->next;
        }

        if (remainingBytes > 0)
        {
            XAUDIO2_BUFFER xbuf{};
            xbuf.pAudioData = currentAudioPtr;
            xbuf.AudioBytes = remainingBytes;
            xbuf.Flags = XAUDIO2_END_OF_STREAM;
            xbuf.LoopCount = sourceCtx->isLooping ? XAUDIO2_LOOP_INFINITE : 0;

            sourceCtx->pSourceVoice->SubmitSourceBuffer(&xbuf);
        }

        HRESULT hr = sourceCtx->pSourceVoice->Start();
        return SUCCEEDED(hr) ? eResult::OK : eResult::Error;
    }

    int32 Stop(SoundSourceHandle hSource)
    {
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice) return eResult::InvalidArgument;

        HRESULT hr = sourceCtx->pSourceVoice->Stop();
        sourceCtx->pSourceVoice->FlushSourceBuffers();
        return SUCCEEDED(hr) ? eResult::OK : eResult::Error;
    }

    int32 Pause(SoundSourceHandle hSource)
    {
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice) return eResult::InvalidArgument;

        HRESULT hr = sourceCtx->pSourceVoice->Stop();
        return SUCCEEDED(hr) ? eResult::OK : eResult::Error;
    }

    int32 SetVolume(SoundSourceHandle hSource, float volume)
    {
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice) return eResult::InvalidArgument;

        HRESULT hr = sourceCtx->pSourceVoice->SetVolume(volume);
        return SUCCEEDED(hr) ? eResult::OK : eResult::Error;
    }

    int32 SetPitch(SoundSourceHandle hSource, float pitch)
    {
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice) return eResult::InvalidArgument;

        HRESULT hr = sourceCtx->pSourceVoice->SetFrequencyRatio(pitch);
        return SUCCEEDED(hr) ? eResult::OK : eResult::Error;
    }

    int32 SetLooping(SoundSourceHandle hSource, bool loop)
    {
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        sourceCtx->isLooping = loop;
        return eResult::OK;
    }

    int32 IsPlaying(SoundSourceHandle hSource, bool& outIsPlaying)
    {
        outIsPlaying = false;
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice) return eResult::InvalidArgument;

        XAUDIO2_VOICE_STATE state{};
        sourceCtx->pSourceVoice->GetState(&state);
        outIsPlaying = (state.BuffersQueued > 0);
        return eResult::OK;
    }

    int32 SetPan(SoundSourceHandle hSource, float pan)
    {
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice || !sourceCtx->pBufferCtx) return eResult::InvalidArgument;

        if (pan < -1.0f) pan = -1.0f;
        if (pan > 1.0f)  pan = 1.0f;

        uint32 channels = sourceCtx->pBufferCtx->channels;
        HRESULT hr = S_OK;

        if (channels == 1)
        {
            float leftGain = (pan <= 0.0f) ? 1.0f : (1.0f - pan);
            float rightGain = (pan >= 0.0f) ? 1.0f : (1.0f + pan);
            float matrix[2] = { leftGain, rightGain };
            hr = sourceCtx->pSourceVoice->SetOutputMatrix(nullptr, 1, 2, matrix);
        }
        else if (channels == 2)
        {
            float leftMult = (pan <= 0.0f) ? 1.0f : (1.0f - pan);
            float rightMult = (pan >= 0.0f) ? 1.0f : (1.0f + pan);
            float matrix[4] = { leftMult, 0.0f, 0.0f, rightMult };
            hr = sourceCtx->pSourceVoice->SetOutputMatrix(nullptr, 2, 2, matrix);
        }
        else
        {
            return eResult::InvalidFormat;
        }

        return SUCCEEDED(hr) ? eResult::OK : eResult::Error;
    }

    int32 SetAllChannelVolumes(SoundSourceHandle hSource, uint32 channelCount, const float* pVolumes)
    {
        if (!hSource.ptr || !pVolumes) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice) return eResult::InvalidArgument;

        uint32 countToApply = (channelCount > sourceCtx->channelCount) ? sourceCtx->channelCount : channelCount;
        for (uint32 i = 0; i < countToApply; ++i)
        {
            sourceCtx->channelVolumes[i] = pVolumes[i];
        }

        HRESULT hr = sourceCtx->pSourceVoice->SetChannelVolumes(sourceCtx->channelCount, sourceCtx->channelVolumes);
        return SUCCEEDED(hr) ? eResult::OK : eResult::Error;
    }

    int32 SetChannelVolume(SoundSourceHandle hSource, uint32 channelIndex, float volume)
    {
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice) return eResult::InvalidArgument;

        if (channelIndex >= sourceCtx->channelCount) return eResult::InvalidArgument;

        sourceCtx->channelVolumes[channelIndex] = volume;

        HRESULT hr = sourceCtx->pSourceVoice->SetChannelVolumes(sourceCtx->channelCount, sourceCtx->channelVolumes);
        return SUCCEEDED(hr) ? eResult::OK : eResult::Error;
    }

    // =========================================================================
    // Synchronization & Timing Functions
    // =========================================================================
    int32 GetPlaybackPosition(SoundSourceHandle hSource, float& outSecondsElapsed)
    {
        outSecondsElapsed = 0.0f;
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pSourceVoice || !sourceCtx->pBufferCtx) return eResult::InvalidArgument;

        XAUDIO2_VOICE_STATE state{};
        sourceCtx->pSourceVoice->GetState(&state);

        float sampleRate = static_cast<float>(sourceCtx->pBufferCtx->sampleRate);
        if (sampleRate <= 0.0f) return eResult::Error;

        outSecondsElapsed = static_cast<float>(state.SamplesPlayed) / sampleRate;
        return eResult::OK;
    }

    int32 GetDuration(SoundSourceHandle hSource, float& outTotalSeconds)
    {
        outTotalSeconds = 0.0f;
        if (!hSource.ptr) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx->pBufferCtx) return eResult::InvalidArgument;
        SoundBufferContext* bufferCtx = sourceCtx->pBufferCtx;

        float sampleRate = static_cast<float>(bufferCtx->sampleRate);
        if (sampleRate <= 0.0f) return eResult::Error;

        outTotalSeconds = static_cast<float>(bufferCtx->totalSamples) / sampleRate;
        return eResult::OK;
    }

    int32 GetPlaybackProgress(SoundSourceHandle hSource, float& outProgress01)
    {
        outProgress01 = 0.0f;
        if (!hSource.ptr) return eResult::InvalidArgument;

        float elapsed = 0.0f;
        float duration = 0.0f;

        if (GetPlaybackPosition(hSource, elapsed) != eResult::OK) return eResult::Error;
        if (GetDuration(hSource, duration) != eResult::OK) return eResult::Error;

        if (duration <= 0.0f) return eResult::OK;

        outProgress01 = elapsed / duration;
        if (outProgress01 > 1.0f) outProgress01 = 1.0f;

        return eResult::OK;
    }

    // =========================================================================
    // Handle-Based Event API
    // =========================================================================
    int32 CreateTimedEvent(SoundSourceHandle hSource, float timeSeconds, AudioCallbackFunction pCallback, void* pUserData, SoundEventHandle& hEvent)
    {
        hEvent.ptr = nullptr;
        if (!hSource.ptr || !pCallback) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);

        SoundEventContext* eventCtx = static_cast<SoundEventContext*>(
            ::VirtualAlloc(nullptr, sizeof(SoundEventContext), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
            );
        if (!eventCtx) return eResult::OutOfMemory;

        *eventCtx = SoundEventContext{ hSource, timeSeconds, pCallback, pUserData, false, nullptr, nullptr };

        if (!sourceCtx->callbackBridge.pFirstEvent || sourceCtx->callbackBridge.pFirstEvent->timeSeconds >= timeSeconds)
        {
            eventCtx->next = sourceCtx->callbackBridge.pFirstEvent;
            if (sourceCtx->callbackBridge.pFirstEvent)
            {
                sourceCtx->callbackBridge.pFirstEvent->prev = eventCtx;
            }
            sourceCtx->callbackBridge.pFirstEvent = eventCtx;
        }
        else
        {
            SoundEventContext* curr = sourceCtx->callbackBridge.pFirstEvent;
            while (curr->next && curr->next->timeSeconds < timeSeconds)
            {
                curr = curr->next;
            }
            eventCtx->next = curr->next;
            if (curr->next)
            {
                curr->next->prev = eventCtx;
            }
            curr->next = eventCtx;
            eventCtx->prev = curr;
        }

        hEvent.ptr = eventCtx;
        return eResult::OK;
    }

    int32 CreateProgressEvent(SoundSourceHandle hSource, float progress01, AudioCallbackFunction pCallback, void* pUserData, SoundEventHandle& hEvent)
    {
        hEvent.ptr = nullptr;
        if (!hSource.ptr || progress01 < 0.0f || progress01 > 1.0f) return eResult::InvalidArgument;
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(hSource.ptr);
        if (!sourceCtx || !sourceCtx->pBufferCtx) return eResult::InvalidArgument;

        float totalSeconds = 0.0f;
        if (GetDuration(hSource, totalSeconds) != eResult::OK) return eResult::Error;

        float targetSeconds = progress01 * totalSeconds;
        return CreateTimedEvent(hSource, targetSeconds, pCallback, pUserData, hEvent);
    }

    void DestroyEvent(SoundEventHandle hEvent)
    {
        if (!hEvent.ptr) return;
        SoundEventContext* eventCtx = static_cast<SoundEventContext*>(hEvent.ptr);
        SoundSourceContext* sourceCtx = static_cast<SoundSourceContext*>(eventCtx->hSource.ptr);

        if (sourceCtx)
        {
            if (eventCtx->prev)
            {
                eventCtx->prev->next = eventCtx->next;
            }
            else
            {
                sourceCtx->callbackBridge.pFirstEvent = eventCtx->next;
            }

            if (eventCtx->next)
            {
                eventCtx->next->prev = eventCtx->prev;
            }
        }

        ::VirtualFree(eventCtx, 0, MEM_RELEASE);
        hEvent.ptr = nullptr;
    }
}