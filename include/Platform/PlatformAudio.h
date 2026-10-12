#pragma once
#include <Platform/PlatformCore.h>

// PlatformAudio
namespace Platform::Audio {

	enum eResult : int32
	{
		PLATFORM_RESULT(PLATFORM_AUDIO_RESULT_BIT),

	};

	DEFINE_HANDLE(AudioDevice);     // AudioDeviceHandle : Device how ably Sound
	DEFINE_HANDLE(SoundBuffer);     // SoundBufferHandle : Row Data of Sound 
	DEFINE_HANDLE(SoundSource);     // SoundSourceHandle : Control of Sound

	static int32	Init();

	static void		Fini();


	struct AudioDeviceCreateInfo
	{
		// ...
	};

	static int32 CreateDevice(const AudioDeviceCreateInfo& CreateInfo, AudioDeviceHandle& hDevice);

	static void DestroyDevice(AudioDeviceHandle hDevice);


	struct SoundBufferCreateInfo
	{
		//...
	};

	static int32 CreateBuffer(const SoundBufferCreateInfo& CreateInfo, SoundBufferHandle& hBuffer);

	static void DestroyBuffer(SoundBufferHandle hBuffer);



	static int32 CreateSource(SoundBufferHandle hBuffer, SoundSourceHandle& hSource);

	static void DestroySource(SoundSourceHandle hSource);

	// -- Control functions for SoundSource

};

