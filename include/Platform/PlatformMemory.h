#pragma once
#include <Platform/PlatformCore.h>

/*

struct PlatformMemory
{
    PlatformMemory() = delete;

public:
    // =========================================================================
    // Heap Memory Operations
    // =========================================================================

    /// @brief Allocates a block of memory from the process heap.
    /// @param size Size in bytes of the memory block to allocate.
    /// @return Pointer to the allocated memory block, or nullptr on failure.
    static void* Alloc(uint64 size);

    /// @brief Frees a memory block previously allocated from the process heap.
    /// @param ptr Pointer to the memory block to be freed. Safe if nullptr.
    static void Free(void* ptr);

    /// @brief Reallocates or resizes a previously allocated heap memory block.
    /// @param ptr Pointer to the original memory block (allocates new memory if nullptr).
    /// @param newSize New size in bytes for the memory block (frees block if 0).
    /// @return Pointer to the resized memory block, or nullptr on failure.
    static void* Realloc(void* ptr, uint64 newSize);

    // =========================================================================
    // Aligned Memory Operations
    // =========================================================================

    /// @brief Allocates heap memory aligned to a specified byte boundary (power of 2).
    /// @param size Size in bytes of the memory block to allocate.
    /// @param alignment Memory alignment boundary in bytes (e.g., 16, 32, 64).
    /// @return Pointer to the aligned memory block, or nullptr on failure.
    static void* AllocAligned(uint64 size, uint64 alignment);

    /// @brief Frees an aligned memory block previously allocated with AllocAligned.
    /// @param ptr Pointer to the aligned memory block to free. Safe if nullptr.
    static void FreeAligned(void* ptr);

    // =========================================================================
    // Virtual Memory Operations
    // =========================================================================

    /// @brief Allocates page-aligned virtual memory directly from the operating system.
    /// @param size Size in bytes of the virtual memory region to commit and reserve.
    /// @return Base pointer to the allocated virtual memory, or nullptr on failure.
    static void* VirtualAlloc(uint64 size);

    /// @brief Releases a region of virtual memory previously allocated by VirtualAlloc.
    /// @param ptr Base pointer to the virtual memory block to release.
    /// @param size Unused/Reserved parameter (Win32 release operation requires 0).
    static void VirtualFree(void* ptr, uint64 size = 0);

    // =========================================================================
    // Raw Memory Operations
    // =========================================================================

    /// @brief Copies a raw block of memory from a source buffer to a destination buffer.
    /// @param dest Pointer to the destination buffer.
    /// @param src Pointer to the source buffer.
    /// @param size Number of bytes to copy.
    static void Copy(void* dest, const void* src, uint64 size);

    /// @brief Fills a block of memory with a specified byte value.
    /// @param dest Pointer to the memory block to fill.
    /// @param value Byte value to set across the memory range.
    /// @param size Number of bytes to fill.
    static void Set(void* dest, uint8 value, uint64 size);

    /// @brief Clears a block of memory by setting all of its bytes to zero.
    /// @param dest Pointer to the memory block to zero out.
    /// @param size Number of bytes to zero out.
    static void Zero(void* dest, uint64 size);

    /// @brief Compares two memory blocks byte-by-byte for equality.
    /// @param ptr1 Pointer to the first memory block.
    /// @param ptr2 Pointer to the second memory block.
    /// @param size Number of bytes to compare.
    /// @return 0 if both memory blocks are identical, or 1 if they differ.
    static int32 Compare(const void* ptr1, const void* ptr2, uint64 size);
};

*/