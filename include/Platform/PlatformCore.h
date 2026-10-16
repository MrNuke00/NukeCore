// PlatformCore.h
#pragma once
#include <Platform/PlatformCoreMacro.h>
#include <Platform/PlatformMacro.h>

#define DEFINE_HANDLE(NAME)										\
	struct NAME##Handle {										\
		void* ptr = nullptr;									\
		using THandle = NAME##Handle;							\
		/* Allow: if (handle) */								\
		constexpr explicit operator bool() const {				\
			return ptr != nullptr;								\
		}														\
		/* Allow: if (handle1 == handle2) */					\
		constexpr bool operator==(const THandle& other) const {	\
			return ptr == other.ptr;							\
		}														\
		constexpr bool operator!=(const THandle& other) const {	\
			return ptr != other.ptr;							\
		}														\
	}



#define PLATFORM_XX_RESULT_BIT			(0xFF0000)

#define PLATFORM_FILE_RESULT_BIT		(0x810000)
#define PLATFORM_THREAD_RESULT_BIT		(0x820000)
#define PLATFORM_PROCESS_RESULT_BIT		(0x830000)
#define PLATFORM_NETWORK_RESULT_BIT		(0x840000)
#define PLATFORM_SURFACE_RESULT_BIT		(0x850000)
#define PLATFORM_AUDIO_RESULT_BIT		(0x860000)


#define PLATFORM_START_API(NAME)\
	struct Platform##NAME {\
		Platform##NAME() = delete;\
	public:


#define PLATFORM_END_API(NAME) }


#define PLATFORM_RESULT(C)	\
	OK = 0,						\
	Error = 1,					\
	UnvalidHandle = 2,			\
	InvalidArgument = 3,		\
	Unknown = INT32_MIN,		\
	__ = C						\





