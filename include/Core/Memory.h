#pragma once
#include <Core/Core.h>
#include <Platform/PlatformMemory.h>

// Header-only template definitions. These MUST live in the header because
// HeapAllocator<T> / Array<T> instantiate them from other translation units.
// (Template definitions in a .cpp are only visible to that one TU -> LNK2019.)

namespace NK::Core::Memory
{
    /**
    * @brief Allocates raw, uninitialized heap memory for a given element count.
    * @param Count Number of elements of type T to allocate space for.
    * @return Pointer to the allocated memory block, or nullptr on failure.
    */
    template<typename T>
    T* Alloc(uint64 Count)
    {
        if (Count == 0) return nullptr;
        return static_cast<T*>(Platform::Memory::Alloc(static_cast<size_t>(Count * sizeof(T))));
    }

    /**
    * @brief Frees a previously allocated heap memory block.
    * @param ptr Pointer to the memory block to free.
    */
    template<typename T>
    void Free(T* ptr)
    {
        if (ptr)
            Platform::Memory::Free(static_cast<void*>(ptr));
    }

    /**
    * @brief Reallocates an existing memory block to a new element count.
    * @param ptr Pointer to the existing memory block.
    * @param Count New number of elements of type T for the resized block.
    * @return Pointer to the reallocated memory block (may have moved).
    */
    template<typename T>
    T* Realloc(T* ptr, uint64 Count)
    {
        if (Count == 0) { Free(ptr); return nullptr; }
        return static_cast<T*>(Platform::Memory::Realloc(
            static_cast<void*>(ptr), static_cast<size_t>(Count * sizeof(T))));
    }

    /**
    * @brief Allocates memory aligned to a specific byte boundary.
    * @param Count Number of elements of type T to allocate space for.
    * @param alignment Desired byte alignment boundary (must be a power of 2 and >= sizeof(void*)).
    * @return Pointer to the aligned memory block.
    */
    template<typename T>
    T* AllocAligned(uint64 Count, uint64 alignment)
    {
        if (Count == 0) return nullptr;
        return static_cast<T*>(Platform::Memory::AllocAligned(
            static_cast<size_t>(Count * sizeof(T)), static_cast<size_t>(alignment)));
    }

    /**
    * @brief Frees an aligned memory block allocated via AllocAligned.
    * @param ptr Pointer to the aligned memory block to free.
    */
    template<typename T>
    void FreeAligned(T* ptr)
    {
        if (ptr)
            Platform::Memory::FreeAligned(static_cast<void*>(ptr));
    }

    /**
    * @brief Allocates raw memory directly from the OS kernel via virtual memory pages.
    * @param Count Number of elements of type T to reserve/commit.
    * @return Pointer to the virtual memory block.
    */
    template<typename T>
    T* VirtualAlloc(uint64 Count)
    {
        if (Count == 0) return nullptr;
        return static_cast<T*>(Platform::Memory::VirtualAlloc(static_cast<size_t>(Count * sizeof(T))));
    }

    /**
    * @brief Frees or releases virtual memory allocated via VirtualAlloc.
    * @param ptr Pointer to the virtual memory block.
    * @param Count Optional element count required by some OS release routines.
    */
    template<typename T>
    void VirtualFree(T* ptr, uint64 Count = 0)
    {
        if (ptr)
            Platform::Memory::VirtualFree(static_cast<void*>(ptr),
                static_cast<size_t>(Count * sizeof(T)));
    }

    /**
    * @brief Copies data from source to destination (non-overlapping memory regions).
    * @param Dest Pointer to the destination buffer.
    * @param Src Pointer to the source buffer.
    * @param Count Number of elements of type T to copy.
    */
    template<typename T>
    void Copy(T* Dest, const T* Src, uint64 Count)
    {
        if (Count == 0) return;
        Platform::Memory::Copy(static_cast<void*>(Dest),
            static_cast<const void*>(Src),
            static_cast<size_t>(Count * sizeof(T)));
    }

    /**
    * @brief Moves data safely from source to destination (supports overlapping memory regions).
    * @param Dest Pointer to the destination buffer.
    * @param Src Pointer to the source buffer.
    * @param Count Number of elements of type T to move.
    */
    template<typename T>
    void Move(T* Dest, const T* Src, uint64 Count)
    {
        if (Count == 0) return;
        Platform::Memory::Move(static_cast<void*>(Dest),
            static_cast<const void*>(Src),
            static_cast<size_t>(Count * sizeof(T)));
    }

    /**
    * @brief Sets a block of memory to a specified byte value.
    * @param Dest Pointer to the destination buffer.
    * @param value Byte value to set.
    * @param Count Number of elements of type T to set.
    */
    template<typename T>
    void Set(T* Dest, uint32 value, uint64 Count)
    {
        if (Count == 0) return;
        Platform::Memory::Set(static_cast<void*>(Dest), value,
            static_cast<size_t>(Count * sizeof(T)));
    }

    /**
    * @brief Clears a block of memory by setting all bytes to zero.
    * @param Dest Pointer to the destination buffer.
    * @param Count Number of elements of type T to zero out.
    */
    template<typename T>
    void Zero(T* Dest, uint64 Count)
    {
        if (Count == 0) return;
        Platform::Memory::Set(static_cast<void*>(Dest), 0,
            static_cast<size_t>(Count * sizeof(T)));
    }

    /**
    * @brief Compares two memory blocks byte by byte.
    * @param ptr1 Pointer to the first memory block.
    * @param ptr2 Pointer to the second memory block.
    * @param Count Number of elements of type T to compare.
    * @return 0 if equal, negative if ptr1 < ptr2, positive if ptr1 > ptr2.
    */
    template<typename T>
    int32 Compare(const T* ptr1, const T* ptr2, uint64 Count)
    {
        if (Count == 0) return 0;
        return Platform::Memory::Compare(static_cast<const void*>(ptr1),
            static_cast<const void*>(ptr2),
            static_cast<size_t>(Count * sizeof(T)));
    }

    /**
    * @brief Fills a range of elements with a specific value using object assignment.
    * @param Dest Pointer to the destination buffer.
    * @param Count Number of elements to fill.
    * @param value Reference to the value to assign across the range.
    */
    template<typename T>
    void Fill(T* Dest, uint64 Count, const T& value)
    {
        for (uint64 i = 0; i < Count; ++i)
            Dest[i] = value;
    }

    /**
    * @brief Swaps two objects efficiently using move semantics.
    * @param a Reference to the first object.
    * @param b Reference to the second object.
    */
    template<typename T>
    void Swap(T& a, T& b)
    {
        if (&a == &b) return;
        T Temp = static_cast<T&&>(a);
        a = static_cast<T&&>(b);
        b = static_cast<T&&>(Temp);
    }
}