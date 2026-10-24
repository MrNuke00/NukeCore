#pragma once
#include <Platform/PlatformCore.h>

namespace Platform::Memory
{
	// General Heap / Pool Allocation
	void*   Alloc(uint64 size);
	void    Free(void* ptr);
	void*   Realloc(void* ptr, uint64 size);

	// Aligned Allocation (Essential for SIMD / GPU buffers)
	void*   AllocAligned(uint64 size, uint64 alignment);
	void    FreeAligned(void* ptr);

	// OS-Level Virtual Memory (Pages / Large Reserves)
	void*   VirtualAlloc(uint64 size);
	void    VirtualFree(void* ptr, uint64 size = 0);

	// Low-Level Memory Utilities (No-STL / High-Performance Intrinsics)
	void    Copy(void* dest, const void* src, uint64 size);
	void	Move(void* dest, const void* src, uint64 size);
	void    Set(void* dest, uint32 value, uint64 size);
	void    Zero(void* dest, uint64 size);
	int32   Compare(const void* ptr1, const void* ptr2, uint64 size);
};

