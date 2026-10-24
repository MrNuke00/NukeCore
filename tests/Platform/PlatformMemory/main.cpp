#include <iostream>
#include <Platform/PlatformMemory.h>

int main()
{
    std::wcout << L"=== Starting Platform::Memory System Comprehensive Test ===" << std::endl;

    // ==========================================
    // 1. Test Standard Heap Allocation & Realloc
    // ==========================================
    std::wcout << L"\n[1] Testing Standard Heap Allocation (Alloc, Realloc, Free)..." << std::endl;

    uint64 allocSize = 128;
    void* mem1 = Platform::Memory::Alloc(allocSize);
    if (mem1)
    {
        std::wcout << L"  -> Alloc(128) succeeded at address: " << mem1 << std::endl;
    }

    // Test Realloc
    uint64 newAllocSize = 256;
    void* mem1_re = Platform::Memory::Realloc(mem1, newAllocSize);
    if (mem1_re)
    {
        std::wcout << L"  -> Realloc(256) succeeded at address: " << mem1_re << std::endl;
        mem1 = mem1_re; // Update reference point
    }

    Platform::Memory::Free(mem1);
    std::wcout << L"  -> Free completed successfully." << std::endl;

    // ==========================================
    // 2. Test Aligned Allocation (SIMD / Custom Alignment)
    // ==========================================
    std::wcout << L"\n[2] Testing Aligned Allocation (AllocAligned, FreeAligned)..." << std::endl;

    uint64 alignment = 64; // e.g., 64-byte alignment for AVX-512 / GPU buffers
    uint64 alignedSize = 1024;
    void* alignedMem = Platform::Memory::AllocAligned(alignedSize, alignment);
    if (alignedMem)
    {
        uintptr_t addrVal = reinterpret_cast<uintptr_t>(alignedMem);
        bool isAligned = (addrVal % alignment == 0);
        std::wcout << L"  -> AllocAligned(1024, 64) succeeded at address: " << alignedMem
            << L" | Is Properly Aligned: " << (isAligned ? L"True" : L"False") << std::endl;

        Platform::Memory::FreeAligned(alignedMem);
        std::wcout << L"  -> FreeAligned completed successfully." << std::endl;
    }

    // ==========================================
    // 3. Test OS Virtual Memory Allocation
    // ==========================================
    std::wcout << L"\n[3] Testing OS Virtual Memory (VirtualAlloc, VirtualFree)..." << std::endl;

    uint64 pageSize = 4096 * 2; // Allocate 2 system pages (8 KB)
    void* virtMem = Platform::Memory::VirtualAlloc(pageSize);
    if (virtMem)
    {
        std::wcout << L"  -> VirtualAlloc(8192) succeeded at address: " << virtMem << std::endl;

        // Write test values directly into virtual memory pages
        char* charPtr = static_cast<char*>(virtMem);
        charPtr[0] = 'N';
        charPtr[1] = 'U';
        charPtr[2] = 'K';
        charPtr[3] = 'E';
        std::wcout << L"  -> Wrote characters to virtual memory: " << charPtr[0] << charPtr[1] << charPtr[2] << charPtr[3] << std::endl;

        Platform::Memory::VirtualFree(virtMem, 0);
        std::wcout << L"  -> VirtualFree completed successfully." << std::endl;
    }

    // ==========================================
    // 4. Test Memory Utilities (Zero, Set, Copy, Compare)
    // ==========================================
    std::wcout << L"\n[4] Testing Low-Level Memory Utilities..." << std::endl;

    const uint64 utilSize = 64;
    char* sourceBuf = static_cast<char*>(Platform::Memory::Alloc(utilSize));
    char* destBuf = static_cast<char*>(Platform::Memory::Alloc(utilSize));

    // Test Zero
    Platform::Memory::Zero(sourceBuf, utilSize);
    std::wcout << L"  -> Zero() initialized source buffer to 0." << std::endl;

    // Test Set
    Platform::Memory::Set(sourceBuf, 'A', 32);
    Platform::Memory::Set(sourceBuf + 32, 'B', 32);
    std::wcout << L"  -> Set() filled buffer contents ('A' and 'B')." << std::endl;

    // Test Copy
    Platform::Memory::Copy(destBuf, sourceBuf, utilSize);
    std::wcout << L"  -> Copy() duplicated source buffer to destination buffer." << std::endl;

    // Test Compare (Equal)
    int32 cmpResultEqual = Platform::Memory::Compare(sourceBuf, destBuf, utilSize);
    std::wcout << L"  -> Compare() identical buffers result (expect 0): " << cmpResultEqual << std::endl;

    // Modify destBuf to test non-equal comparison
    destBuf[0] = 'X';
    int32 cmpResultDiff = Platform::Memory::Compare(sourceBuf, destBuf, utilSize);
    std::wcout << L"  -> Compare() modified buffers result (expect non-zero): " << cmpResultDiff << std::endl;

    // Cleanup utility buffers
    Platform::Memory::Free(sourceBuf);
    Platform::Memory::Free(destBuf);

    std::wcout << L"\n=== All Platform::Memory Tests Completed Successfully! ===" << std::endl;
    return 0;
}