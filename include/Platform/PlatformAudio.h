#pragma once
#include <Platform/PlatformCore.h>

namespace Platform::Audio
{

    enum eResult : int32
    {
        PLATFORM_RESULT(PLATFORM_AUDIO_RESULT_BIT),
        DeviceNotFound,
        NotInitialized,
        InvalidFormat,
        OutOfMemory,
        AccessDenied
    };

    DEFINE_HANDLE(AudioDevice);     // AudioDeviceHandle : Device that plays sound
    DEFINE_HANDLE(SoundBuffer);     // SoundBufferHandle : Raw Data of Sound 
    DEFINE_HANDLE(SoundSource);     // SoundSourceHandle : Control of Sound

    // Subsystem Lifecycle
    int32 Init();
    void  Fini();

    struct AudioDeviceCreateInfo
    {
        uint32 sampleRate = 44100;
        uint32 channels = 2;
    };

    int32 CreateDevice(const AudioDeviceCreateInfo& createInfo, AudioDeviceHandle& hDevice);
    void  DestroyDevice(AudioDeviceHandle hDevice);

    enum class eSampleFormat : uint32
    {
        Int16,     // 16-bit signed integer (Standard PCM)
        Float32    // 32-bit IEEE floating point
    };

    struct SoundBufferCreateInfo
    {
        uint16        channels      = 2;      // e.g., 1 = Mono, 2 = Stereo
        uint32        sampleRate    = 44100;  // e.g., 44100 Hz or 48000 Hz
        eSampleFormat format        = eSampleFormat::Int16;
        const void*   pAudioData    = nullptr; // Raw interleaved PCM byte array
        uint32        dataSize      = 0;       // Total size in bytes (dataSize = totalFrames * channels * (bitsPerSample / 8))
    };

    int32 CreateBuffer(AudioDeviceHandle hDevice, const SoundBufferCreateInfo& createInfo, SoundBufferHandle& hBuffer);
    void  DestroyBuffer(SoundBufferHandle hBuffer);

    int32 CreateSource(AudioDeviceHandle hDevice, SoundBufferHandle hBuffer, SoundSourceHandle& hSource);
    void  DestroySource(SoundSourceHandle hSource);

    // -- Control functions for SoundSource
    int32 Play(SoundSourceHandle hSource);
    int32 Stop(SoundSourceHandle hSource);
    int32 Pause(SoundSourceHandle hSource);
    int32 SetVolume(SoundSourceHandle hSource, float volume); // 0.0f to 1.0f
    int32 SetPitch(SoundSourceHandle hSource, float pitch);   // 1.0f is normal speed
    int32 SetLooping(SoundSourceHandle hSource, bool loop);
    int32 IsPlaying(SoundSourceHandle hSource, bool& outIsPlaying);

    int32 SetPan(SoundSourceHandle hSource, float pan);                          // -1.0f (Left) to 1.0f (Right)

    // Set volume for all channels at once
    int32 SetAllChannelVolumes(SoundSourceHandle hSource, uint32 channelCount, const float* pVolumes);

    // Set volume for a single specific channel index (e.g., channel 0 = Left)
    int32 SetChannelVolume(SoundSourceHandle hSource, uint32 channelIndex, float volume);

    // -- Synchronization & Timing Functions
    int32 GetPlaybackPosition(SoundSourceHandle hSource, float& outSecondsElapsed);
    int32 GetPlaybackProgress(SoundSourceHandle hSource, float& outProgress01); // 0.0f (Start) to 1.0f (End)
    int32 GetDuration(SoundSourceHandle hSource, float& outTotalSeconds);


    DEFINE_HANDLE(SoundEvent); //SoundEventHandle

    // Clean callback function signature tied directly to the source
    typedef void (*AudioCallbackFunction)(SoundSourceHandle hSource, void* pUserData);

    // Create a Event to fire at a specific timestamp in seconds (e.g. 1.5f seconds in)
    int32 CreateTimedEvent(SoundSourceHandle hSource, float timeSeconds, AudioCallbackFunction pCallback, void* pUserData, SoundEventHandle& hEvent);

    // Create a Event to fire at a specific progress percentage [0.0f - 1.0f]
    int32 CreateProgressEvent(SoundSourceHandle hSource, float progress01, AudioCallbackFunction pCallback, void* pUserData, SoundEventHandle& hEvent);

    // Explicitly destroy or cancel the event if needed before it fires
    void DestroyEvent(SoundEventHandle hEvent);

};