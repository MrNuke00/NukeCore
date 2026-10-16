#pragma once
#include <Platform/PlatformCore.h>


namespace Platform
{

    enum eResult : int32
    {
        PLATFORM_RESULT(PLATFORM_THREAD_RESULT_BIT),
        OutOfMemory,
        AccessDenied,
        NotSupported,
        Timeout
    };
	
    namespace Thread
    {
        DEFINE_HANDLE(Thread);

        enum class eThreadPriority : int32
        {
            Idle = -2,
            Lowest = -1,
            Normal = 0,
            Highest = 1,
            TimeCritical = 2
        };

        struct ThreadCreateInfo
        {
            const char* name = nullptr;               // Debug / Profiler thread name
            uint32            stackSize = 0;                // Stack size in bytes (0 for OS default)
            int32             cpuCore = -1;                 // CPU core affinity (-1 for any)
            eThreadPriority   priority = eThreadPriority::Normal;
            bool              createSuspended = false;
        };

        // Engine-agnostic thread entry signature
        typedef uint32(*ThreadEntryPoint)(void* userData);

        int32 Init();
        void  Fini();

        // Lifecycle
        int32 Create(ThreadEntryPoint entryFunction, void* userData, const ThreadCreateInfo& info, ThreadHandle& outThread);
        int32 Join(ThreadHandle hThread, uint32* pExitCode = nullptr);
        int32 Detach(ThreadHandle hThread);
        int32 CloseHandle(ThreadHandle hThread);
        void  Exit(uint32 exitCode);

        // Execution Control
        void  Sleep(uint32 milliseconds);
        void  YieldExecution();

        // Identification & Tuning
        uint64 GetCurrentThreadId();
        uint64 GetThreadId(ThreadHandle hThread);
        int32  SetPriority(ThreadHandle hThread, eThreadPriority priority);
        int32  SetAffinity(ThreadHandle hThread, uint64 affinityMask);
        int32  SetName(ThreadHandle hThread, const char* name);

    }

    namespace Sync
    {
        DEFINE_HANDLE(Mutex);
        DEFINE_HANDLE(Semaphore);
        DEFINE_HANDLE(ConditionVariable);

        // Mutex
        int32 CreateMutex(MutexHandle&  hMutex, bool recursive = false);
        void  DestroyMutex(MutexHandle  hMutex);
        void  LockMutex(MutexHandle     hMutex);
        bool  TryLockMutex(MutexHandle  hMutex);
        void  UnlockMutex(MutexHandle   hMutex);

        // Semaphore
        int32 CreateSemaphore(uint32 InitialCount, uint32 MaxCount, SemaphoreHandle& hSemaphore);
        void  DestroySemaphore(SemaphoreHandle hSemaphore);
        int32 WaitSemaphore(SemaphoreHandle hSemaphore, uint32 TimeoutMs = UINT32_MAX);
        int32 SignalSemaphore(SemaphoreHandle hSemaphore, uint32 Count = 1);

        // Condition Variable
        int32 CreateConditionVariable(ConditionVariableHandle& hCondVar);
        void  DestroyConditionVariable(ConditionVariableHandle hCondVar);
        int32 WaitConditionVariable(ConditionVariableHandle hCondVar, MutexHandle hMutex, uint32 timeoutMs = UINT32_MAX);
        void  SignalConditionVariable(ConditionVariableHandle hCondVar);
        void  BroadcastConditionVariable(ConditionVariableHandle hCondVar);
    }

    namespace Atomic
    {
        enum class eMemoryOrder : int32
        {
            // Just update this value. No order guarantees. Fastest, for simple counters.
            Relaxed = 0,

            // If next code depends on this data, don't read it early.
            Consume = 1,

            // Stop! Don't let code AFTER this line move BEFORE it. (Use when opening a lock / reading shared data)
            Acquire = 2,

            // Finish previous work first, then let others see this update. (Use when closing a lock / publishing data)
            Release = 3,

            // Both Acquire and Release combined. Used for read-modify-write operations.
            AcqRel = 4,

            // Strict global order. Every thread sees everything in the exact same sequence. Safest (Default).
            SeqCst = 5
        };

        // ==========================================
        // Memory Barriers
        // ==========================================
        void MemoryBarrier();
        void CompilerBarrier();

        // ==========================================
        // 8-bit & 16-bit Small Atomics
        // ==========================================
        int8   Load8(const volatile int8* destination, eMemoryOrder order = eMemoryOrder::SeqCst);
        void   Store8(volatile int8* destination, int8 value, eMemoryOrder order = eMemoryOrder::SeqCst);
        int8   Exchange8(volatile int8* destination, int8 exchangeValue, eMemoryOrder order = eMemoryOrder::SeqCst);

        int16  Load16(const volatile int16* destination, eMemoryOrder order = eMemoryOrder::SeqCst);
        void   Store16(volatile int16* destination, int16 value, eMemoryOrder order = eMemoryOrder::SeqCst);
        int16  Exchange16(volatile int16* destination, int16 exchangeValue, eMemoryOrder order = eMemoryOrder::SeqCst);

        // ==========================================
        // 32-bit Atomics (Integer & Float)
        // ==========================================
        // Integer
        int32  Load32(const volatile int32* destination, eMemoryOrder order = eMemoryOrder::SeqCst);
        void   Store32(volatile int32* destination, int32 value, eMemoryOrder order = eMemoryOrder::SeqCst);
        int32  Exchange32(volatile int32* destination, int32 exchangeValue, eMemoryOrder order = eMemoryOrder::SeqCst);
        bool   CompareExchange32(volatile int32* destination, int32* expectedValue, int32 desiredValue, eMemoryOrder successOrder = eMemoryOrder::SeqCst, eMemoryOrder failOrder = eMemoryOrder::SeqCst);
        int32  FetchAdd32(volatile int32* destination, int32 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        int32  FetchSub32(volatile int32* destination, int32 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        int32  FetchAnd32(volatile int32* destination, int32 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        int32  FetchOr32(volatile int32* destination, int32 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        int32  FetchXor32(volatile int32* destination, int32 operand, eMemoryOrder order = eMemoryOrder::SeqCst);

        // Float
        float  LoadFloat32(const volatile float* destination, eMemoryOrder order = eMemoryOrder::SeqCst);
        void   StoreFloat32(volatile float* destination, float value, eMemoryOrder order = eMemoryOrder::SeqCst);
        float  ExchangeFloat32(volatile float* destination, float exchangeValue, eMemoryOrder order = eMemoryOrder::SeqCst);
        bool   CompareExchangeFloat32(volatile float* destination, float* expectedValue, float desiredValue, eMemoryOrder successOrder = eMemoryOrder::SeqCst, eMemoryOrder failOrder = eMemoryOrder::SeqCst);
        float  FetchAddFloat32(volatile float* destination, float operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        float  FetchSubFloat32(volatile float* destination, float operand, eMemoryOrder order = eMemoryOrder::SeqCst);

        // ==========================================
        // 64-bit Atomics (Integer & Float)
        // ==========================================
        // Integer
        int64  Load64(const volatile int64* destination, eMemoryOrder order = eMemoryOrder::SeqCst);
        void   Store64(volatile int64* destination, int64 value, eMemoryOrder order = eMemoryOrder::SeqCst);
        int64  Exchange64(volatile int64* destination, int64 exchangeValue, eMemoryOrder order = eMemoryOrder::SeqCst);
        bool   CompareExchange64(volatile int64* destination, int64* expectedValue, int64 desiredValue, eMemoryOrder successOrder = eMemoryOrder::SeqCst, eMemoryOrder failOrder = eMemoryOrder::SeqCst);
        int64  FetchAdd64(volatile int64* destination, int64 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        int64  FetchSub64(volatile int64* destination, int64 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        int64  FetchAnd64(volatile int64* destination, int64 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        int64  FetchOr64(volatile int64* destination, int64 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        int64  FetchXor64(volatile int64* destination, int64 operand, eMemoryOrder order = eMemoryOrder::SeqCst);

        // Float (Double)
        float64 LoadFloat64(const volatile float64* destination, eMemoryOrder order = eMemoryOrder::SeqCst);
        void    StoreFloat64(volatile float64* destination, float64 value, eMemoryOrder order = eMemoryOrder::SeqCst);
        float64 ExchangeFloat64(volatile float64* destination, float64 exchangeValue, eMemoryOrder order = eMemoryOrder::SeqCst);
        bool    CompareExchangeFloat64(volatile float64* destination, float64* expectedValue, float64 desiredValue, eMemoryOrder successOrder = eMemoryOrder::SeqCst, eMemoryOrder failOrder = eMemoryOrder::SeqCst);
        float64 FetchAddFloat64(volatile float64* destination, float64 operand, eMemoryOrder order = eMemoryOrder::SeqCst);
        float64 FetchSubFloat64(volatile float64* destination, float64 operand, eMemoryOrder order = eMemoryOrder::SeqCst);

        // ==========================================
        // Pointer Atomics (Architecture-dependent size)
        // ==========================================
        void* LoadPointer(const volatile void** destination, eMemoryOrder order = eMemoryOrder::SeqCst);
        void   StorePointer(volatile void** destination, void* value, eMemoryOrder order = eMemoryOrder::SeqCst);
        void* ExchangePointer(volatile void** destination, void* exchangeValue, eMemoryOrder order = eMemoryOrder::SeqCst);
        bool   CompareExchangePointer(volatile void** destination, void** expectedValue, void* desiredValue, eMemoryOrder successOrder = eMemoryOrder::SeqCst, eMemoryOrder failOrder = eMemoryOrder::SeqCst);

        // ==========================================
        // Atomic Flag / Spinlocks
        // ==========================================
        bool   TestAndSet(volatile int8* flag, eMemoryOrder order = eMemoryOrder::SeqCst);
        void   ClearFlag(volatile int8* flag, eMemoryOrder order = eMemoryOrder::SeqCst);
    };

};
