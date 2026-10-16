#include <Platform/PlatformProcess.h>
#include <windows.h>
#include <stdio.h>

namespace Platform::Process
{
    // =========================================================================
    // Low-Level Kernel Process Object (Allocated directly via OS VirtualAlloc)
    // =========================================================================
    struct ProcessContext
    {
        HANDLE hProcess;         // OS Process HANDLE
        HANDLE hStdinPipeWrite;  // Parent writes here -> Child reads stdin
        HANDLE hStdoutPipeRead;  // Child writes here -> Parent reads stdout
        HANDLE hStderrPipeRead;  // Child writes here -> Parent reads stderr
    };

    // =========================================================================
    // Process API Implementation
    // =========================================================================
    int32 Create(const ProcessCreateInfo& info, ProcessHandle& hProcess)
    {
        hProcess.ptr = nullptr;
        if (!info.executablePath) return eResult::InvalidArgument;

        // Allocate process context directly from the OS (Zero C++ Heap, Zero Pools)
        ProcessContext* ctx = static_cast<ProcessContext*>(
            ::VirtualAlloc(nullptr, sizeof(ProcessContext), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
            );
        if (!ctx) return eResult::Error;

        *ctx = ProcessContext{}; // Zero-initialize

        SECURITY_ATTRIBUTES sa{};
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;

        HANDLE hChildStdinRead = nullptr, hChildStdinWrite = nullptr;
        HANDLE hChildStdoutRead = nullptr, hChildStdoutWrite = nullptr;
        HANDLE hChildStderrRead = nullptr, hChildStderrWrite = nullptr;

        STARTUPINFOW si{};
        si.cb = sizeof(STARTUPINFOW);

        if (info.redirectStreams)
        {
            si.dwFlags |= STARTF_USESTDHANDLES;

            if (!::CreatePipe(&hChildStdinRead, &hChildStdinWrite, &sa, 0))
            {
                ::VirtualFree(ctx, 0, MEM_RELEASE);
                return eResult::Error;
            }
            ::SetHandleInformation(hChildStdinWrite, HANDLE_FLAG_INHERIT, 0);

            if (!::CreatePipe(&hChildStdoutRead, &hChildStdoutWrite, &sa, 0))
            {
                if (hChildStdinRead)  ::CloseHandle(hChildStdinRead);
                if (hChildStdinWrite) ::CloseHandle(hChildStdinWrite);
                ::VirtualFree(ctx, 0, MEM_RELEASE);
                return eResult::Error;
            }
            ::SetHandleInformation(hChildStdoutRead, HANDLE_FLAG_INHERIT, 0);

            if (!::CreatePipe(&hChildStderrRead, &hChildStderrWrite, &sa, 0))
            {
                if (hChildStdinRead)   ::CloseHandle(hChildStdinRead);
                if (hChildStdinWrite)  ::CloseHandle(hChildStdinWrite);
                if (hChildStdoutRead)  ::CloseHandle(hChildStdoutRead);
                if (hChildStdoutWrite) ::CloseHandle(hChildStdoutWrite);
                ::VirtualFree(ctx, 0, MEM_RELEASE);
                return eResult::Error;
            }
            ::SetHandleInformation(hChildStderrRead, HANDLE_FLAG_INHERIT, 0);

            si.hStdInput = hChildStdinRead;
            si.hStdOutput = hChildStdoutWrite;
            si.hStdError = hChildStderrWrite;
        }

        wchar_t widePath[MAX_PATH];
        ::MultiByteToWideChar(CP_UTF8, 0, info.executablePath, -1, widePath, MAX_PATH);

        wchar_t wideArgs[1024] = { 0 };
        if (info.arguments)
        {
            ::MultiByteToWideChar(CP_UTF8, 0, info.arguments, -1, wideArgs, 1024);
        }

        wchar_t wideCmdLine[3072] = { 0 };
        if (info.arguments && info.arguments[0] != '\0')
        {
            swprintf_s(wideCmdLine, L"\"%s\" %s", widePath, wideArgs);
        }
        else
        {
            swprintf_s(wideCmdLine, L"\"%s\"", widePath);
        }

        wchar_t wideWorkDir[MAX_PATH] = { 0 };
        const wchar_t* pWorkDir = nullptr;
        if (info.workingDir)
        {
            ::MultiByteToWideChar(CP_UTF8, 0, info.workingDir, -1, wideWorkDir, MAX_PATH);
            pWorkDir = wideWorkDir;
        }

        PROCESS_INFORMATION pi{};
        DWORD creationFlags = (info.flags & static_cast<uint32>(eProcessFlags::CreateSuspended)) ? CREATE_SUSPENDED : 0;
        if (info.flags & static_cast<uint32>(eProcessFlags::Hidden))
        {
            si.dwFlags |= STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
            creationFlags |= CREATE_NO_WINDOW;
        }

        BOOL success = ::CreateProcessW(
            nullptr,
            wideCmdLine,
            nullptr, nullptr,
            TRUE,
            creationFlags,
            nullptr,
            pWorkDir,
            &si,
            &pi
        );

        if (hChildStdinRead)   ::CloseHandle(hChildStdinRead);
        if (hChildStdoutWrite) ::CloseHandle(hChildStdoutWrite);
        if (hChildStderrWrite) ::CloseHandle(hChildStderrWrite);

        if (!success)
        {
            if (hChildStdinWrite)  ::CloseHandle(hChildStdinWrite);
            if (hChildStdoutRead)  ::CloseHandle(hChildStdoutRead);
            if (hChildStderrRead)  ::CloseHandle(hChildStderrRead);
            ::VirtualFree(ctx, 0, MEM_RELEASE);
            return eResult::Error;
        }

        ctx->hProcess = pi.hProcess;
        ctx->hStdinPipeWrite = hChildStdinWrite;
        ctx->hStdoutPipeRead = hChildStdoutRead;
        ctx->hStderrPipeRead = hChildStderrRead;

        ::CloseHandle(pi.hThread); // Detach primary thread handle

        hProcess.ptr = ctx;
        return eResult::OK;
    }

    int32 CloseHandle(ProcessHandle hProcess)
    {
        if (!hProcess) return eResult::UnvalidHandle;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);

        if (ctx->hStdinPipeWrite)  ::CloseHandle(ctx->hStdinPipeWrite);
        if (ctx->hStdoutPipeRead)  ::CloseHandle(ctx->hStdoutPipeRead);
        if (ctx->hStderrPipeRead)  ::CloseHandle(ctx->hStderrPipeRead);
        if (ctx->hProcess)         ::CloseHandle(ctx->hProcess);

        // Release OS virtual memory block directly
        ::VirtualFree(ctx, 0, MEM_RELEASE);
        return eResult::OK;
    }

    int32 Wait(ProcessHandle hProcess, uint32 timeoutMs, int32* outExitCode)
    {
        if (!hProcess) return eResult::UnvalidHandle;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);

        DWORD timeout = (timeoutMs == UINT32_MAX) ? INFINITE : static_cast<DWORD>(timeoutMs);
        DWORD result = ::WaitForSingleObject(ctx->hProcess, timeout);

        if (result == WAIT_OBJECT_0)
        {
            if (outExitCode)
            {
                DWORD code = 0;
                if (::GetExitCodeProcess(ctx->hProcess, &code))
                {
                    *outExitCode = static_cast<int32>(code);
                }
            }
            return eResult::OK;
        }
        return (result == WAIT_TIMEOUT) ? eResult::Timeout : eResult::Error;
    }

    int32 Terminate(ProcessHandle hProcess, int32 exitCode)
    {
        if (!hProcess) return eResult::UnvalidHandle;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);
        return ::TerminateProcess(ctx->hProcess, static_cast<UINT>(exitCode)) ? eResult::OK : eResult::Error;
    }

    int32 IsRunning(ProcessHandle hProcess, bool& outIsRunning)
    {
        if (!hProcess) return eResult::UnvalidHandle;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);

        DWORD result = ::WaitForSingleObject(ctx->hProcess, 0);
        if (result == WAIT_TIMEOUT) { outIsRunning = true; return eResult::OK; }
        if (result == WAIT_OBJECT_0) { outIsRunning = false; return eResult::OK; }
        return eResult::Error;
    }

    int32 GetExitCode(ProcessHandle hProcess, int32& outExitCode)
    {
        if (!hProcess) return eResult::UnvalidHandle;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);

        DWORD code = 0;
        if (::GetExitCodeProcess(ctx->hProcess, &code))
        {
            outExitCode = static_cast<int32>(code);
            return eResult::OK;
        }
        return eResult::Error;
    }

    uint64 GetCurrentProcessId()
    {
        return static_cast<uint64>(::GetCurrentProcessId());
    }

    uint64 GetProcessId(ProcessHandle hProcess)
    {
        if (!hProcess) return 0;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);
        return static_cast<uint64>(::GetProcessId(ctx->hProcess));
    }

    int32 Suspend(ProcessHandle hProcess)
    {
        if (!hProcess) return eResult::UnvalidHandle;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);

        typedef LONG(NTAPI* pfnNtSuspendProcess)(HANDLE);
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (hNtdll)
        {
            auto NtSuspendProcess = (pfnNtSuspendProcess)GetProcAddress(hNtdll, "NtSuspendProcess");
            if (NtSuspendProcess)
            {
                NTSTATUS status = NtSuspendProcess(ctx->hProcess);
                return (status >= 0) ? eResult::OK : eResult::Error;
            }
        }
        return eResult::NotSupported;
    }

    int32 Resume(ProcessHandle hProcess)
    {
        if (!hProcess) return eResult::UnvalidHandle;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);

        typedef LONG(NTAPI* pfnNtResumeProcess)(HANDLE);
        HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
        if (hNtdll)
        {
            auto NtResumeProcess = (pfnNtResumeProcess)GetProcAddress(hNtdll, "NtResumeProcess");
            if (NtResumeProcess)
            {
                NTSTATUS status = NtResumeProcess(ctx->hProcess);
                return (status >= 0) ? eResult::OK : eResult::Error;
            }
        }
        return eResult::NotSupported;
    }

    int32 ReadMemory(ProcessHandle hProcess, uptr remoteAddress, void* buffer, uint64 size, uint64* outBytesRead)
    {
        if (!hProcess || !buffer) return eResult::InvalidArgument;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);

        SIZE_T bytesRead = 0;
        BOOL success = ::ReadProcessMemory(
            ctx->hProcess,
            reinterpret_cast<LPCVOID>(remoteAddress),
            buffer,
            static_cast<SIZE_T>(size),
            &bytesRead
        );
        if (!success) return eResult::AccessDenied;
        if (outBytesRead) *outBytesRead = static_cast<uint64>(bytesRead);
        return eResult::OK;
    }

    int32 WriteMemory(ProcessHandle hProcess, uptr remoteAddress, const void* buffer, uint64 size, uint64* outBytesWritten)
    {
        if (!hProcess || !buffer) return eResult::InvalidArgument;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);

        SIZE_T bytesWritten = 0;
        BOOL success = ::WriteProcessMemory(
            ctx->hProcess,
            reinterpret_cast<LPVOID>(remoteAddress),
            buffer,
            static_cast<SIZE_T>(size),
            &bytesWritten
        );
        if (!success) return eResult::AccessDenied;
        if (outBytesWritten) *outBytesWritten = static_cast<uint64>(bytesWritten);
        return eResult::OK;
    }

    // =========================================================================
    // Stream I/O Operations Implementation
    // =========================================================================
    int32 ReadStdout(ProcessHandle hProcess, void* buffer, uint32 size, uint32* outBytesRead)
    {
        if (!hProcess || !buffer) return eResult::InvalidArgument;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);
        if (!ctx->hStdoutPipeRead) return eResult::UnvalidHandle;

        DWORD bytesRead = 0;
        BOOL success = ::ReadFile(ctx->hStdoutPipeRead, buffer, static_cast<DWORD>(size), &bytesRead, nullptr);
        if (!success)
        {
            DWORD err = ::GetLastError();
            if (err == ERROR_BROKEN_PIPE) return eResult::ConnectionClosed;
            return eResult::Error;
        }
        if (outBytesRead) *outBytesRead = static_cast<uint32>(bytesRead);
        return eResult::OK;
    }

    int32 ReadStderr(ProcessHandle hProcess, void* buffer, uint32 size, uint32* outBytesRead)
    {
        if (!hProcess || !buffer) return eResult::InvalidArgument;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);
        if (!ctx->hStderrPipeRead) return eResult::UnvalidHandle;

        DWORD bytesRead = 0;
        BOOL success = ::ReadFile(ctx->hStderrPipeRead, buffer, static_cast<DWORD>(size), &bytesRead, nullptr);
        if (!success)
        {
            DWORD err = ::GetLastError();
            if (err == ERROR_BROKEN_PIPE) return eResult::ConnectionClosed;
            return eResult::Error;
        }
        if (outBytesRead) *outBytesRead = static_cast<uint32>(bytesRead);
        return eResult::OK;
    }

    int32 WriteStdin(ProcessHandle hProcess, const void* data, uint32 size, uint32* outBytesWritten)
    {
        if (!hProcess || !data) return eResult::InvalidArgument;
        ProcessContext* ctx = static_cast<ProcessContext*>(hProcess.ptr);
        if (!ctx->hStdinPipeWrite) return eResult::UnvalidHandle;

        DWORD bytesWritten = 0;
        BOOL success = ::WriteFile(ctx->hStdinPipeWrite, data, static_cast<DWORD>(size), &bytesWritten, nullptr);
        if (!success) return eResult::Error;
        if (outBytesWritten) *outBytesWritten = static_cast<uint32>(bytesWritten);
        return eResult::OK;
    }

    // =========================================================================
    // Shared Memory API Implementation
    // =========================================================================
    int32 CreateSharedMemory(const char* name, uint64 size, SharedMemoryHandle& hSharedMemory, void** ppMappedPtr)
    {
        hSharedMemory.ptr = nullptr;
        if (!name || size == 0 || !ppMappedPtr) return eResult::InvalidArgument;

        wchar_t wideName[256];
        ::MultiByteToWideChar(CP_UTF8, 0, name, -1, wideName, 256);

        DWORD highSize = static_cast<DWORD>(size >> 32);
        DWORD lowSize = static_cast<DWORD>(size & 0xFFFFFFFF);

        HANDLE hMap = ::CreateFileMappingW(
            INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE,
            highSize,
            lowSize,
            wideName
        );

        if (!hMap)
        {
            return (::GetLastError() == ERROR_ALREADY_EXISTS) ? eResult::AlreadyExists : eResult::Error;
        }

        void* mapped = ::MapViewOfFile(
            hMap,
            FILE_MAP_ALL_ACCESS,
            0, 0,
            static_cast<SIZE_T>(size)
        );

        if (!mapped)
        {
            ::CloseHandle(hMap);
            return eResult::Error;
        }

        hSharedMemory.ptr = hMap;
        *ppMappedPtr = mapped;
        return eResult::OK;
    }

    int32 OpenSharedMemory(const char* name, SharedMemoryHandle& hSharedMemory, void** ppMappedPtr, uint64 size, uint64* outSize)
    {
        hSharedMemory.ptr = nullptr;
        if (!name || !ppMappedPtr) return eResult::InvalidArgument;

        wchar_t wideName[256];
        ::MultiByteToWideChar(CP_UTF8, 0, name, -1, wideName, 256);

        HANDLE hMap = ::OpenFileMappingW(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            wideName
        );

        if (!hMap) return eResult::ProcessNotFound;

        void* mapped = ::MapViewOfFile(
            hMap,
            FILE_MAP_ALL_ACCESS,
            0, 0,
            static_cast<SIZE_T>(size)
        );

        if (!mapped)
        {
            ::CloseHandle(hMap);
            return eResult::Error;
        }

        hSharedMemory.ptr = hMap;
        *ppMappedPtr = mapped;
        if (outSize) *outSize = size;
        return eResult::OK;
    }

    void CloseSharedMemory(SharedMemoryHandle hSharedMemory, void* pMappedPtr)
    {
        if (!hSharedMemory) return;
        if (pMappedPtr) ::UnmapViewOfFile(pMappedPtr);
        if (hSharedMemory.ptr) ::CloseHandle(static_cast<HANDLE>(hSharedMemory.ptr));
    }
}