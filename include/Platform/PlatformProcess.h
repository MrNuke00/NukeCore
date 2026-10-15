#pragma once
#include <Platform/PlatformCore.h>

namespace Platform::Process
{
    enum eResult : int32
    {
        PLATFORM_RESULT(PLATFORM_PROCESS_RESULT_BIT),
        ProcessNotFound,
        AccessDenied,
        Timeout,
        AlreadyExists,
        NotSupported,
        ConnectionClosed
    };

    // Standard engine handle macros
    DEFINE_HANDLE(Process);
    DEFINE_HANDLE(SharedMemory);

    enum class eProcessFlags : uint32
    {
        None = 0,
        Detached = 1 << 0,
        Hidden = 1 << 1,
        CreateSuspended = 1 << 2
    };

    struct ProcessCreateInfo
    {
        const char* executablePath = nullptr;
        const char* arguments = nullptr;
        const char* workingDir = nullptr;
        uint32      flags = 0;
        bool        redirectStreams = true; // Enable stdin/stdout/stderr pipes
    };

    // Process API
    int32 Create(const ProcessCreateInfo& info, ProcessHandle& hProcess);
    int32 CloseHandle(ProcessHandle hProcess);
    int32 Wait(ProcessHandle hProcess, uint32 timeoutMs, int32* outExitCode = nullptr);
    int32 Terminate(ProcessHandle hProcess, int32 exitCode = -1);
    int32 IsRunning(ProcessHandle hProcess, bool& outIsRunning);
    int32 GetExitCode(ProcessHandle hProcess, int32& outExitCode);
    uint64 GetCurrentProcessId();
    uint64 GetProcessId(ProcessHandle hProcess);
    int32 Suspend(ProcessHandle hProcess);
    int32 Resume(ProcessHandle hProcess);
    int32 ReadMemory(ProcessHandle hProcess, uptr remoteAddress, void* buffer, uint64 size, uint64* outBytesRead = nullptr);
    int32 WriteMemory(ProcessHandle hProcess, uptr remoteAddress, const void* buffer, uint64 size, uint64* outBytesWritten = nullptr);

    // Stream I/O Operations
    int32 ReadStdout(ProcessHandle hProcess, void* buffer, uint32 size, uint32* outBytesRead);
    int32 ReadStderr(ProcessHandle hProcess, void* buffer, uint32 size, uint32* outBytesRead);
    int32 WriteStdin(ProcessHandle hProcess, const void* data, uint32 size, uint32* outBytesWritten);

    // Shared Memory API
    int32 CreateSharedMemory(const char* name, uint64 size, SharedMemoryHandle& hSharedMemory, void** ppMappedPtr);
    int32 OpenSharedMemory(const char* name, SharedMemoryHandle& hSharedMemory, void** ppMappedPtr, uint64 size = 0, uint64* outSize = nullptr);
    void  CloseSharedMemory(SharedMemoryHandle hSharedMemory, void* pMappedPtr);
}