#include <Platform/PlatformMemory.h>
#include <WindowsCore.h>

namespace Platform::Memory
{
	// Cache the default process heap handle for fast lookups
	static HANDLE GetEngineHeap()
	{
		static HANDLE hHeap = GetProcessHeap();
		return hHeap;
	}

	void* Alloc(uint64 size)
	{
		if (size == 0) return nullptr;
		return HeapAlloc(GetEngineHeap(), 0, static_cast<SIZE_T>(size));
	}

	void Free(void* ptr)
	{
		if (ptr)
		{
			HeapFree(GetEngineHeap(), 0, ptr);
		}
	}

	void* Realloc(void* ptr, uint64 size)
	{
		if (!ptr)
		{
			return Alloc(size);
		}
		if (size == 0)
		{
			Free(ptr);
			return nullptr;
		}
		return HeapReAlloc(GetEngineHeap(), 0, ptr, static_cast<SIZE_T>(size));
	}

	void* AllocAligned(uint64 size, uint64 alignment)
	{
		if (size == 0) return nullptr;
		// _aligned_malloc requires alignment to be a power of 2 and a multiple of sizeof(void*)
		if (alignment < sizeof(void*))
		{
			alignment = sizeof(void*);
		}
		return _aligned_malloc(static_cast<size_t>(size), static_cast<size_t>(alignment));
	}

	void FreeAligned(void* ptr)
	{
		if (ptr)
		{
			_aligned_free(ptr);
		}
	}

	void* VirtualAlloc(uint64 size)
	{
		if (size == 0) return nullptr;

		// Reserve and commit address space in one go at OS page boundaries
		return ::VirtualAlloc(
			nullptr,
			static_cast<SIZE_T>(size),
			MEM_RESERVE | MEM_COMMIT,
			PAGE_READWRITE
		);
	}

	void VirtualFree(void* ptr, uint64 size)
	{
		if (ptr)
		{
			// MEM_RELEASE requires the size parameter to be 0 on Windows
			::VirtualFree(ptr, 0, MEM_RELEASE);
		}
	}

	void Copy(void* dest, const void* src, uint64 size)
	{
		// For high-performance engines, you can later swap this out with 
		// SSE/AVX optimized block copying routines (e.g., _mm256_loadu_si256)
		std::memcpy(dest, src, static_cast<size_t>(size));
	}

	void Set(void* dest, uint32 value, uint64 size)
	{
		std::memset(dest, static_cast<int>(value), static_cast<size_t>(size));
	}

	void Zero(void* dest, uint64 size)
	{
		// Fast-path zeroing (OS or compiler optimized)
		std::memset(dest, 0, static_cast<size_t>(size));
	}

	int32 Compare(const void* ptr1, const void* ptr2, uint64 size)
	{
		int result = std::memcmp(ptr1, ptr2, static_cast<size_t>(size));
		return static_cast<int32>(result);
	}
};