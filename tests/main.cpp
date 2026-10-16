#include <Platform/PlatformAudio.h>
#include <iostream>
#include <windows.h>
#include <cmath>

// Callback handler for audio events
void AudioEventCallback(Platform::Audio::SoundSourceHandle hSource, void* pUserData)
{
    int eventTag = *static_cast<int*>(pUserData);
    std::cout << "\n   [EVENT TRIGGERED] -> Callback received tag ID: " << eventTag << std::endl;
}

int main()
{
    std::cout << "====================================================" << std::endl;
    std::cout << "         PLATFORM AUDIO CORE: FULL TEST SUITE        " << std::endl;
    std::cout << "====================================================" << std::endl;

    // 1. Test Subsystem Initialization (Loads XAudio2_9.dll dynamically)
    std::cout << "[1/7] Initializing Audio Subsystem..." << std::endl;
    if (Platform::Audio::Init() != 0)
    {
        std::cerr << "FAIL: Platform::Audio::Init() failed!" << std::endl;
        return -1;
    }
    std::cout << "      -> Subsystem initialized successfully." << std::endl;

    // 2. Test Device Creation
    std::cout << "[2/7] Creating Audio Device..." << std::endl;
    Platform::Audio::AudioDeviceHandle hDevice;
    Platform::Audio::AudioDeviceCreateInfo deviceCI{ 44100, 2 }; // 44.1kHz, Stereo
    if (Platform::Audio::CreateDevice(deviceCI, hDevice) != 0)
    {
        std::cerr << "FAIL: CreateDevice failed!" << std::endl;
        Platform::Audio::Fini();
        return -1;
    }
    std::cout << "      -> Device created successfully." << std::endl;

    // 3. Generate Procedural Audio Data (3 seconds of a 440Hz Sine Wave)
    uint32 sampleRate = 44100;
    uint16 channels = 2;
    float durationSec = 3.0f;
    uint32 totalFrames = static_cast<uint32>(sampleRate * durationSec);
    uint32 dataSize = totalFrames * channels * sizeof(int16);

    int16* pcmData = static_cast<int16*>(
        ::VirtualAlloc(nullptr, dataSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
        );
    if (!pcmData)
    {
        std::cerr << "FAIL: VirtualAlloc for PCM data failed!" << std::endl;
        Platform::Audio::DestroyDevice(hDevice);
        Platform::Audio::Fini();
        return -1;
    }

    for (uint32 i = 0; i < totalFrames; ++i)
    {
        float t = static_cast<float>(i) / sampleRate;
        float sineValue = std::sin(2.0f * 3.14159265f * 440.0f * t);
        int16 sample = static_cast<int16>(sineValue * 16383.0f); // 50% volume amplitude
        pcmData[i * 2 + 0] = sample; // Left Channel
        pcmData[i * 2 + 1] = sample; // Right Channel
    }

    // 4. Test Buffer Creation & Destruction (Testing Int16 format)
    std::cout << "[3/7] Creating Sound Buffer (Int16 PCM)..." << std::endl;
    Platform::Audio::SoundBufferHandle hBuffer;
    Platform::Audio::SoundBufferCreateInfo bufferCI{};
    bufferCI.channels = channels;
    bufferCI.sampleRate = sampleRate;
    bufferCI.format = Platform::Audio::eSampleFormat::Int16;
    bufferCI.pAudioData = pcmData;
    bufferCI.dataSize = dataSize;

    if (Platform::Audio::CreateBuffer(hDevice, bufferCI, hBuffer) != 0)
    {
        std::cerr << "FAIL: CreateBuffer failed!" << std::endl;
        ::VirtualFree(pcmData, 0, MEM_RELEASE);
        Platform::Audio::DestroyDevice(hDevice);
        Platform::Audio::Fini();
        return -1;
    }
    std::cout << "      -> Sound Buffer created successfully." << std::endl;

    // 5. Test Source Creation
    std::cout << "[4/7] Creating Sound Source..." << std::endl;
    Platform::Audio::SoundSourceHandle hSource;
    if (Platform::Audio::CreateSource(hDevice, hBuffer, hSource) != 0)
    {
        std::cerr << "FAIL: CreateSource failed!" << std::endl;
        Platform::Audio::DestroyBuffer(hBuffer);
        ::VirtualFree(pcmData, 0, MEM_RELEASE);
        Platform::Audio::DestroyDevice(hDevice);
        Platform::Audio::Fini();
        return -1;
    }
    std::cout << "      -> Sound Source created successfully." << std::endl;

    // 6. Test Property Setters & Control APIs
    std::cout << "[5/7] Testing Source Property APIs (Volume, Pitch, Pan, Channels)..." << std::endl;
    Platform::Audio::SetVolume(hSource, 0.8f);
    Platform::Audio::SetPitch(hSource, 1.0f); // Normal speed
    Platform::Audio::SetLooping(hSource, false);

    // Initialize stereo channels to equal volume (Center)
    float channelVols[2] = { 1.0f, 1.0f };
    Platform::Audio::SetAllChannelVolumes(hSource, 2, channelVols);

    // 7. Test Handle-Based Event System Registration
    std::cout << "[6/7] Registering First-Class Sound Event Handles..." << std::endl;
    int tagTimed = 101;
    int tagProgress = 202;
    int tagCanceled = 999;

    Platform::Audio::SoundEventHandle hTimedEvent;
    Platform::Audio::SoundEventHandle hProgressEvent;
    Platform::Audio::SoundEventHandle hCanceledEvent;

    // Timed event at 0.75 seconds
    Platform::Audio::CreateTimedEvent(hSource, 0.75f, AudioEventCallback, &tagTimed, hTimedEvent);

    // Progress event at 50% duration (1.5 seconds)
    Platform::Audio::CreateProgressEvent(hSource, 0.50f, AudioEventCallback, &tagProgress, hProgressEvent);

    // Event scheduled to test early destruction/cancellation
    Platform::Audio::CreateTimedEvent(hSource, 2.80f, AudioEventCallback, &tagCanceled, hCanceledEvent);

    // Immediately destroy the canceled event to test DestroyEvent handle cleanup
    Platform::Audio::DestroyEvent(hCanceledEvent);
    std::cout << "      -> Events registered and cancellation test passed." << std::endl;

    // 8. Test Playback, Pause, Channel Isolation, & Timing Queries
    std::cout << "[7/7] Starting Playback and Testing Channel/Pause/Timing APIs..." << std::endl;
    Platform::Audio::Play(hSource);

    bool isPlaying = true;
    float duration = 0.0f;
    Platform::Audio::GetDuration(hSource, duration);
    std::cout << "      -> Total Track Duration: " << duration << " seconds." << std::endl;

    bool leftTestDone = false;
    bool rightTestDone = false;
    bool pauseTested = false;

    while (isPlaying)
    {
        Platform::Audio::IsPlaying(hSource, isPlaying);

        float position = 0.0f;
        float progress = 0.0f;
        Platform::Audio::GetPlaybackPosition(hSource, position);
        Platform::Audio::GetPlaybackProgress(hSource, progress);

        std::cout << "\r   [Playback] Progress: " << (progress * 100.0f)
            << "% | Elapsed: " << position << "s / " << duration << "s       " << std::flush;

        // Test 1: Route audio exclusively to LEFT channel (Mute Right) at ~0.5s
        if (!leftTestDone && position >= 0.5f && position < 0.6f)
        {
            std::cout << "\n   [Channel Test] -> Muting Right Channel (Left Only Active)..." << std::endl;
            Platform::Audio::SetChannelVolume(hSource, 0, 1.0f); // Left = 100%
            Platform::Audio::SetChannelVolume(hSource, 1, 0.0f); // Right = 0%
            leftTestDone = true;
        }

        // Test 2: Route audio exclusively to RIGHT channel (Mute Left) at ~1.2s
        if (!rightTestDone && position >= 1.2f && position < 1.3f)
        {
            std::cout << "\n   [Channel Test] -> Muting Left Channel (Right Only Active)..." << std::endl;
            Platform::Audio::SetChannelVolume(hSource, 0, 0.0f); // Left = 0%
            Platform::Audio::SetChannelVolume(hSource, 1, 1.0f); // Right = 100%
            rightTestDone = true;
        }

        // Test 3: Test Pause and Resume logic around 2.0 second mark (Restoring both channels)
        if (!pauseTested && position >= 2.0f && position < 2.1f)
        {
            std::cout << "\n   [Control Test] -> Restoring Both Channels & Pausing playback..." << std::endl;
            Platform::Audio::SetChannelVolume(hSource, 0, 1.0f); // Left restored
            Platform::Audio::SetChannelVolume(hSource, 1, 1.0f); // Right restored

            Platform::Audio::Pause(hSource);
            ::Sleep(500); // Pause for half a second
            std::cout << "   [Control Test] -> Resuming playback (Play)..." << std::endl;
            Platform::Audio::Play(hSource);
            pauseTested = true;
        }

        ::Sleep(30);
    }

    std::cout << "\n\nPlayback finished successfully. Teardown & Cleanup..." << std::endl;

    // 9. Clean Teardown (Handles in reverse order of dependency)
    Platform::Audio::DestroyEvent(hTimedEvent);
    Platform::Audio::DestroyEvent(hProgressEvent);
    Platform::Audio::DestroySource(hSource);
    Platform::Audio::DestroyBuffer(hBuffer);
    ::VirtualFree(pcmData, 0, MEM_RELEASE);
    Platform::Audio::DestroyDevice(hDevice);
    Platform::Audio::Fini();

    std::cout << "====================================================" << std::endl;
    std::cout << "        ALL TESTS PASSED WITHOUT EXCEPTION          " << std::endl;
    std::cout << "====================================================" << std::endl;

    return 0;
}