#pragma once
#include <Platform/PlatformCore.h>

namespace Platform::Process {

	enum eResult : int32
	{
		PLATFORM_RESULT(PLATFORM_PROCESS_RESULT_BIT),

	};

};

/*

#define PLATFORM_PROCESS_API API_DECL
#define PLATFORM_SHARED_MEMORY_API API_DECL

/// @brief Wrapper handle for platform process resources.
DEFINE_HANDLE(Process); // ProcessHandle



/// @brief Parameters used to launch a new process.
struct ProcessCreateInfo
{
	const char* executablePath = nullptr; // Full or relative path to executable
	const char* arguments      = nullptr; // Null-terminated command line arguments
	const char* workingDir     = nullptr; // Working directory (nullptr for current working dir)
	uint32_t    flags          = 0;       // Bitwise combination of eProcessFlags
};

/// @brief Creation flags for configuring process behavior.
enum class eProcessFlags : uint32_t
{
	None            = 0,
	Detached        = 1 << 0, // Run independently without child job binding
	Hidden          = 1 << 1, // Do not open or display a visible window/console
	CreateSuspended = 1 << 2  // Create process in a suspended state
};

struct PLATFORM_PROCESS_API PlatformProcess
{
	PlatformProcess() = delete;
public:

	static iResult Init();
	static void Fini();

public:

	/// @brief Spawns a new process with the specified configuration.
	/// @param info Structural parameters for launching the executable.
	/// @param outProcess Reference to the process handle to be populated on success.
	static iResult Create(const ProcessCreateInfo& info, ProcessHandle& outProcess);

	/// @brief Closes and releases the OS-level process handle resources.
	/// @note This does NOT terminate the actual process if it is still running.
	/// @param process The handle of the process to close.
	static iResult CloseHandle(ProcessHandle process);

	/// @brief Waits for a process to finish execution up to a specified timeout.
	/// @param process The process handle.
	/// @param timeoutMs Timeout in milliseconds (use UINT32_MAX for infinite wait).
	/// @param outExitCode Optional pointer to receive the process exit code.
	static iResult Wait(ProcessHandle process, uint32_t timeoutMs, int32_t* outExitCode = nullptr);

	/// @brief Forcibly terminates a running process.
	/// @param process The process handle.
	/// @param exitCode Exit code to terminate the process with.
	static iResult Terminate(ProcessHandle process, int32_t exitCode = -1);

	/// @brief Checks whether the target process is still running.
	/// @param process The process handle.
	/// @param outIsRunning Reference to store true if running, false if finished.
	static iResult IsRunning(ProcessHandle process, bool& outIsRunning);

	/// @brief Retrieves the exit code of a finished process.
	/// @param process The process handle.
	/// @param outExitCode Reference to store the exit code.
	static iResult GetExitCode(ProcessHandle process, int32_t& outExitCode);

	/// @brief Gets the OS process ID for the calling process.
	static uint64_t GetCurrentProcessId();

	/// @brief Gets the OS process ID from a process handle.
	/// @param process The process handle.
	static uint64_t GetProcessId(ProcessHandle process);

	/// @brief Freezes all threads in a target process.
	static iResult Suspend(ProcessHandle process);

	/// @brief Unfreezes a suspended target process.
	static iResult Resume(ProcessHandle process);

	/// @brief Reads raw bytes from the address space of a target process.
	/// @param process Handle to the target process (must have read permissions).
	/// @param remoteAddress Memory address inside the target process to read from.
	/// @param buffer Local destination buffer to store the copied data.
	/// @param size Number of bytes to read.
	/// @param outBytesRead Optional pointer to receive actual bytes copied.
	static iResult ReadMemory(ProcessHandle process, uintptr_t remoteAddress, void* buffer, uint64_t size, uint64_t* outBytesRead = nullptr);

	/// @brief Writes raw bytes into the address space of a target process.
	static iResult WriteMemory(ProcessHandle process, uintptr_t remoteAddress, const void* buffer, uint64_t size, uint64_t* outBytesWritten = nullptr);

};



DEFINE_HANDLE(SharedMemory); // SharedMemoryHandle

struct PLATFORM_SHARED_MEMORY_API PlatformSharedMemory
{
	PlatformSharedMemory() = delete;

public:

	/// @brief Creates a shared memory region accessible by name.
	/// @param name Unique string identifier across the OS (e.g., "Local\\MyGameBuffer").
	/// @param size Size in bytes of the shared region.
	/// @param outHandle Output handle.
	/// @param outMappedPtr Output direct pointer to access the shared struct/buffer.
	static iResult Create(const char* name, uint64_t size, SharedMemoryHandle& outHandle, void** outMappedPtr);

	/// @brief Opens an existing shared memory region created by another process.
	/// @param name Unique OS string identifier.
	/// @param outHandle Output handle.
	/// @param outMappedPtr Output pointer to the mapped memory.
	/// @param size Bytes to map (Pass 0 to automatically map the ENTIRE region).
	/// @param outSize Optional pointer to receive the actual size mapped by the OS.
	static iResult Open(const char* name, SharedMemoryHandle& outHandle, void** outMappedPtr, uint64_t size = 0, uint64_t* outSize = nullptr);

	/// @brief Unmaps and closes the shared memory.
	static void Close(SharedMemoryHandle handle, void* mappedPtr);

};

*/



