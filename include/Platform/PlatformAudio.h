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

	int32	Init();

	void	Fini();


	struct AudioDeviceCreateInfo
	{
		// ...
	};

	int32 CreateDevice(const AudioDeviceCreateInfo& CreateInfo, AudioDeviceHandle& hDevice);

	void DestroyDevice(AudioDeviceHandle hDevice);


	struct SoundBufferCreateInfo
	{
		//...
	};

	int32 CreateBuffer(const SoundBufferCreateInfo& CreateInfo, SoundBufferHandle& hBuffer);

	void DestroyBuffer(SoundBufferHandle hBuffer);



	int32 CreateSource(SoundBufferHandle hBuffer, SoundSourceHandle& hSource);

	void DestroySource(SoundSourceHandle hSource);

	// -- Control functions for SoundSource

};

