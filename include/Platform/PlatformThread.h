#pragma once
#include <Platform/PlatformCore.h>


namespace Platform::Thread {

	enum eResult : int32
	{
		PLATFORM_RESULT(PLATFORM_THREAD_RESULT_BIT),

	};

};

/*


/// @brief Wrapper handles for threading and synchronization resources.
DEFINE_HANDLE(Thread);             // ThreadHandle



/// @brief Thread priority levels mapped to underlying OS schedulers.
enum class eThreadPriority : int32
{
	Idle = -2,
	Lowest = -1,
	Normal = 0,
	Highest = 1,
	TimeCritical = 2
};

/// @brief Parameters used to configure and spawn a new thread.
struct ThreadCreateInfo
{
	const char* name = nullptr;			// Debug/Profiler name for the thread
	uint32			stackSize = 0;		// Stack size in bytes (0 for default OS stack size)
	int32			cpuCore = -1;		// Specific CPU core affinity (-1 for any/unrestricted)
	eThreadPriority	priority = eThreadPriority::Normal;
	bool			createSuspended = false;
};

/// @brief Signature for thread entry point functions.
typedef uint32(*ThreadEntryPoint)(void* userData);


/// @brief Low-level platform thread management interface.
struct PLATFORM_THREAD_API PlatformThread
{
	PlatformThread() = delete;
public:

	static iResult Init();
	static void Fini();

public:

	/// @brief Spawns a new OS thread.
	/// @param entryFunction Pointer to the function the thread will execute.
	/// @param userData Custom pointer passed to the thread entry function.
	/// @param info Structural configuration parameters (name, stack size, priority, affinity).
	/// @param hThread Reference to the thread handle to be populated on success.
	static iResult Create(ThreadEntryPoint entryFunction, void* userData, const ThreadCreateInfo& info, ThreadHandle& hThread);

	/// @brief Blocks the calling thread until the target thread finishes execution.
	/// @param hThread The thread handle to wait for.
	/// @param outExitCode Optional pointer to receive the thread's return code.
	static iResult Join(ThreadHandle hThread, uint32* pExitCode = nullptr);

	/// @brief Detaches the thread, allowing it to run independently and clean up automatically upon exit.
	static iResult Detach(ThreadHandle hThread);

	/// @brief Closes and releases the OS-level thread handle resource.
	static iResult CloseHandle(ThreadHandle hThread);

	/// @brief Terminates the calling thread immediately (use with extreme caution).
	static void Exit(uint32 exitCode);

	/// @brief Suspends execution of the calling thread for a specified duration.
	/// @param milliseconds Sleep duration in milliseconds.
	static void Sleep(uint32 milliseconds);

	/// @brief Yields the remaining time slice of the calling thread to allow other threads to run.
	static void YieldExecution();

	/// @brief Gets the unique OS thread ID for the calling thread.
	static uint64 GetCurrentThreadId();

	/// @brief Gets the unique OS thread ID from a thread handle.
	static uint64 GetThreadId(ThreadHandle hThread);

	/// @brief Sets the execution priority of a target thread.
	static iResult SetPriority(ThreadHandle hThread, eThreadPriority priority);

	/// @brief Sets the CPU core affinity mask for a target thread.
	static iResult SetAffinity(ThreadHandle hThread, uint64 affinityMask);

	/// @brief Sets the debug name of a thread (visible in debuggers like Visual Studio / RenderDoc).
	static iResult SetName(ThreadHandle hThread, const char* name);
};


DEFINE_HANDLE(Mutex);              // MutexHandle
DEFINE_HANDLE(Semaphore);          // SemaphoreHandle
DEFINE_HANDLE(ConditionVariable);  // ConditionVariableHandle

/// @brief Platform synchronization primitives interface (Mutexes, Semaphores, Condition Variables).
struct PLATFORM_SYNCH_API PlatformSynch
{
	PlatformSynch() = delete;
public:

	// -------------------------------------------------------------------------
	// Mutex (Recursive / Non-recursive locking)
	// -------------------------------------------------------------------------
	static iResult CreateMutex(MutexHandle& hMutex, bool recursive = false);
	static void DestroyMutex(MutexHandle hMutex);
	static void LockMutex(MutexHandle hMutex);
	static bool TryLockMutex(MutexHandle hMutex);
	static void UnlockMutex(MutexHandle hMutex);

	// -------------------------------------------------------------------------
	// Semaphore (Counting semaphore for signaling / job throttling)
	// -------------------------------------------------------------------------
	static iResult CreateSemaphore(uint32 initialCount, uint32 maxCount, SemaphoreHandle& hSemaphore);
	static void DestroySemaphore(SemaphoreHandle hSemaphore);
	static iResult WaitSemaphore(SemaphoreHandle hSemaphore, uint32 timeoutMs = UINT32_MAX);
	static iResult SignalSemaphore(SemaphoreHandle hSemaphore, uint32 count = 1);

	// -------------------------------------------------------------------------
	// Condition Variable (Thread signaling based on state predicates)
	// -------------------------------------------------------------------------
	static iResult CreateConditionVariable(ConditionVariableHandle& hCondVar);
	static void DestroyConditionVariable(ConditionVariableHandle hCondVar);
	static iResult WaitConditionVariable(ConditionVariableHandle hCondVar, MutexHandle hMutex, uint32 timeoutMs = UINT32_MAX);
	static void SignalConditionVariable(ConditionVariableHandle hCondVar);
	static void BroadcastConditionVariable(ConditionVariableHandle hCondVar);
};


/// @brief Memory memory-order constraints wrapping standard C++ memory ordering semantics.
enum class eMemoryOrder : int32
{
	Relaxed = 0,
	Consume = 1,
	Acquire = 2,
	Release = 3,
	AcqRel = 4,
	SeqCst = 5
};

/// @brief Low-level atomic operations and hardware memory barrier utilities.
struct PLATFORM_ATOMIC_API PlatformAtomic
{
	PlatformAtomic() = delete;
public:

	/// @brief Inserts a full hardware memory fence (prevents instruction reordering by CPU and compiler).
	static void MemoryBarrier();

	/// @brief Inserts a compiler-only memory barrier (prevents compiler reordering without emitting CPU fence instructions).
	static void CompilerBarrier();

	// -------------------------------------------------------------------------
	// 32-bit Integer Atomics
	// -------------------------------------------------------------------------

	static int32	Load32(const volatile int32* destination, eMemoryOrder order = eMemoryOrder::SeqCst);
	static void		Store32(volatile int32* destination, int32 value, eMemoryOrder order = eMemoryOrder::SeqCst);

	static int32	Exchange32(volatile int32* destination, int32 exchangeValue, eMemoryOrder order = eMemoryOrder::SeqCst);
	static bool		CompareExchange32(volatile int32* destination, int32* expectedValue, int32 desiredValue, eMemoryOrder successOrder = eMemoryOrder::SeqCst, eMemoryOrder failOrder = eMemoryOrder::SeqCst);

	static int32	FetchAdd32(volatile int32* destination, int32 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
	static int32	FetchSub32(volatile int32* destination, int32 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
	static int32	FetchAnd32(volatile int32* destination, int32 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
	static int32	FetchOr32(volatile int32* destination, int32 operand, eMemoryOrder order = eMemoryOrder::SeqCst);

	// -------------------------------------------------------------------------
	// 64-bit Integer Atomics
	// -------------------------------------------------------------------------

	static int64	Load64(const volatile int64* destination, eMemoryOrder order = eMemoryOrder::SeqCst);
	static void		Store64(volatile int64* destination, int64 value, eMemoryOrder order = eMemoryOrder::SeqCst);

	static int64	Exchange64(volatile int64* destination, int64 exchangeValue, eMemoryOrder order = eMemoryOrder::SeqCst);
	static bool		CompareExchange64(volatile int64* destination, int64* expectedValue, int64 desiredValue, eMemoryOrder successOrder = eMemoryOrder::SeqCst, eMemoryOrder failOrder = eMemoryOrder::SeqCst);

	static int64	FetchAdd64(volatile int64* destination, int64 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
	static int64	FetchSub64(volatile int64* destination, int64 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
};

*/

