#pragma once
#include <Platform/PlatformCore.h>

#define PLATFORM_AUDIO_API API_DECL

/// @brief Wrapper handles for audio resources.
DEFINE_HANDLE(AudioDevice);     // AudioDeviceHandle
DEFINE_HANDLE(SoundBuffer);     // SoundBufferHandle
DEFINE_HANDLE(SoundSource);     // SoundSourceHandle

namespace eResult {
	enum : iResult
	{
		PlatformAudioResult = PLATFORM_AUDIO_RESULT_BIT,
		//... 
	};
};

/// @brief Low-level platform audio management and playback interface.
struct PLATFORM_AUDIO_API PlatformAudio
{
	PlatformAudio() = delete;
public:

	static iResult Init();
	static void Fini();

public:

	/// @brief Creates and initializes an audio playback device.
	static iResult CreateDevice(const AudioDeviceCreateInfo& info, AudioDeviceHandle& outDevice);

	/// @brief Destroys and releases an audio device.
	static void DestroyDevice(AudioDeviceHandle device);

	/// @brief Creates an in-memory sound buffer from raw PCM data.
	static iResult CreateBuffer(const SoundBufferCreateInfo& info, SoundBufferHandle& hBuffer);

	/// @brief Destroys and releases a sound buffer resource.
	static void DestroyBuffer(SoundBufferHandle buffer);

	/// @brief Creates an active sound source instance linked to a sound buffer.
	static iResult CreateSource(SoundBufferHandle buffer, SoundSourceHandle& hSource);

	/// @brief Destroys and releases an active sound source.
	static void DestroySource(SoundSourceHandle source);

	/// @brief Starts or resumes playback of a sound source.
	static iResult Play(SoundSourceHandle source);

	/// @brief Pauses playback of a sound source.
	static iResult Pause(SoundSourceHandle source);

	/// @brief Stops playback of a sound source and resets its play cursor.
	static iResult Stop(SoundSourceHandle source);

	/// @brief Retrieves the current playback state of a sound source.
	static iResult GetState(SoundSourceHandle source, eAudioState& outState);

	/// @brief Sets the volume level of a sound source (Range: 0.0f [mute] to 1.0f [full volume]).
	static iResult SetVolume(SoundSourceHandle source, float volume);

	/// @brief Sets the pitch/playback speed multiplier of a sound source (Default: 1.0f).
	static iResult SetPitch(SoundSourceHandle source, float pitch);

	/// @brief Enables or disables looping playback for a sound source.
	static iResult SetLooping(SoundSourceHandle source, bool loop);
};

