#include <Platform/PlatformThread.h>
#include <windows.h>
#include <intrin.h>

// Ensure proper linkage for modern Windows APIs (like SetThreadDescription)
#pragma comment(lib, "Kernel32.lib")

#pragma intrinsic(_ReadWriteBarrier)
#pragma intrinsic(_InterlockedExchange8)
#pragma intrinsic(_InterlockedExchange16)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedCompareExchange8)
#pragma intrinsic(_InterlockedCompareExchange16)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedExchangeAdd64)
#pragma intrinsic(_InterlockedAnd)
#pragma intrinsic(_InterlockedAnd64)
#pragma intrinsic(_InterlockedOr)
#pragma intrinsic(_InterlockedOr64)
#pragma intrinsic(_InterlockedXor)
#pragma intrinsic(_InterlockedXor64)

namespace Platform
{
    // =========================================================================
    // THREAD IMPLEMENTATION (Windows Kernel)
    // =========================================================================
    namespace Thread
    {
        struct ThreadStartupData
        {
            ThreadEntryPoint entryFunction;
            void* userData;
            char  name[64];
        };

        static DWORD WINAPI WindowsThreadThunk(LPVOID lpParam)
        {
            ThreadStartupData* startupData = reinterpret_cast<ThreadStartupData*>(lpParam);
            ThreadEntryPoint entry = startupData->entryFunction;
            void* userData = startupData->userData;

            uint32 exitCode = entry(userData);
            return static_cast<DWORD>(exitCode);
        }

        int32 Init()
        {
            return eResult::OK;
        }

        void Fini()
        {
            // Cleanup global thread subsystem resources if any
        }

        int32 Create(ThreadEntryPoint entryFunction, void* userData, const ThreadCreateInfo& info, ThreadHandle& outThread)
        {
            if (!entryFunction)
            {
                return eResult::InvalidArgument;
            }

            ThreadStartupData* startupData = new ThreadStartupData{};
            startupData->entryFunction = entryFunction;
            startupData->userData = userData;

            if (info.name)
            {
                for (int i = 0; i < 63 && info.name[i] != '\0'; ++i)
                {
                    startupData->name[i] = info.name[i];
                    startupData->name[i + 1] = '\0';
                }
            }
            else
            {
                startupData->name[0] = '\0';
            }

            DWORD creationFlags = info.createSuspended ? CREATE_SUSPENDED : 0;
            DWORD threadId = 0;

            HANDLE hWinThread = ::CreateThread(
                nullptr,
                info.stackSize,
                WindowsThreadThunk,
                startupData,
                creationFlags,
                &threadId
            );

            if (!hWinThread)
            {
                delete startupData;
                return eResult::Error;
            }

            if (info.priority != eThreadPriority::Normal)
            {
                int winPriority = THREAD_PRIORITY_NORMAL;
                switch (info.priority)
                {
                case eThreadPriority::Idle:         winPriority = THREAD_PRIORITY_IDLE; break;
                case eThreadPriority::Lowest:       winPriority = THREAD_PRIORITY_LOWEST; break;
                case eThreadPriority::Normal:       winPriority = THREAD_PRIORITY_NORMAL; break;
                case eThreadPriority::Highest:      winPriority = THREAD_PRIORITY_HIGHEST; break;
                case eThreadPriority::TimeCritical: winPriority = THREAD_PRIORITY_TIME_CRITICAL; break;
                }
                ::SetThreadPriority(hWinThread, winPriority);
            }

            if (info.cpuCore >= 0)
            {
                DWORD_PTR mask = (1ULL << info.cpuCore);
                ::SetThreadAffinityMask(hWinThread, mask);
            }

            if (info.name && info.name[0] != '\0')
            {
                wchar_t wideName[64];
                for (int i = 0; i < 64; ++i)
                {
                    wideName[i] = static_cast<wchar_t>(startupData->name[i]);
                    if (startupData->name[i] == '\0') break;
                }

                typedef HRESULT(WINAPI* SetThreadDescriptionFn)(HANDLE, PCWSTR);
                HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
                if (hKernel32)
                {
                    auto pSetThreadDescription = (SetThreadDescriptionFn)GetProcAddress(hKernel32, "SetThreadDescription");
                    if (pSetThreadDescription)
                    {
                        pSetThreadDescription(hWinThread, wideName);
                    }
                }
            }

            outThread.ptr = hWinThread;
            return eResult::OK;
        }

        int32 Join(ThreadHandle hThread, uint32* pExitCode)
        {
            HANDLE hWinThread = reinterpret_cast<HANDLE>(hThread.ptr);
            if (!hWinThread) return eResult::UnvalidHandle;

            DWORD waitResult = ::WaitForSingleObject(hWinThread, INFINITE);
            if (waitResult != WAIT_OBJECT_0)
            {
                return eResult::Error;
            }

            if (pExitCode)
            {
                DWORD code = 0;
                if (::GetExitCodeThread(hWinThread, &code))
                {
                    *pExitCode = static_cast<uint32>(code);
                }
            }

            ::CloseHandle(hWinThread);
            return eResult::OK;
        }

        int32 Detach(ThreadHandle hThread)
        {
            HANDLE hWinThread = reinterpret_cast<HANDLE>(hThread.ptr);
            if (!hWinThread) return eResult::UnvalidHandle;

            ::CloseHandle(hWinThread);
            return eResult::OK;
        }

        int32 CloseHandle(ThreadHandle hThread)
        {
            HANDLE hWinThread = reinterpret_cast<HANDLE>(hThread.ptr);
            if (!hWinThread) return eResult::UnvalidHandle;

            if (::CloseHandle(hWinThread))
            {
                return eResult::OK;
            }
            return eResult::Error;
        }

        void Exit(uint32 exitCode)
        {
            ::ExitThread(static_cast<DWORD>(exitCode));
        }

        void Sleep(uint32 milliseconds)
        {
            ::Sleep(milliseconds);
        }

        void YieldExecution()
        {
            ::SwitchToThread();
        }

        uint64 GetCurrentThreadId()
        {
            return static_cast<uint64>(::GetCurrentThreadId());
        }

        uint64 GetThreadId(ThreadHandle hThread)
        {
            HANDLE hWinThread = reinterpret_cast<HANDLE>(hThread.ptr);
            if (!hWinThread) return 0;
            return static_cast<uint64>(::GetThreadId(hWinThread));
        }

        int32 SetPriority(ThreadHandle hThread, eThreadPriority priority)
        {
            HANDLE hWinThread = reinterpret_cast<HANDLE>(hThread.ptr);
            if (!hWinThread) return eResult::UnvalidHandle;

            int winPriority = THREAD_PRIORITY_NORMAL;
            switch (priority)
            {
            case eThreadPriority::Idle:         winPriority = THREAD_PRIORITY_IDLE; break;
            case eThreadPriority::Lowest:       winPriority = THREAD_PRIORITY_LOWEST; break;
            case eThreadPriority::Normal:       winPriority = THREAD_PRIORITY_NORMAL; break;
            case eThreadPriority::Highest:      winPriority = THREAD_PRIORITY_HIGHEST; break;
            case eThreadPriority::TimeCritical: winPriority = THREAD_PRIORITY_TIME_CRITICAL; break;
            }

            if (::SetThreadPriority(hWinThread, winPriority))
            {
                return eResult::OK;
            }
            return eResult::Error;
        }

        int32 SetAffinity(ThreadHandle hThread, uint64 affinityMask)
        {
            HANDLE hWinThread = reinterpret_cast<HANDLE>(hThread.ptr);
            if (!hWinThread) return eResult::UnvalidHandle;

            DWORD_PTR result = ::SetThreadAffinityMask(hWinThread, static_cast<DWORD_PTR>(affinityMask));
            if (result != 0)
            {
                return eResult::OK;
            }
            return eResult::Error;
        }

        int32 SetName(ThreadHandle hThread, const char* name)
        {
            HANDLE hWinThread = reinterpret_cast<HANDLE>(hThread.ptr);
            if (!hWinThread || !name) return eResult::InvalidArgument;

            wchar_t wideName[64];
            for (int i = 0; i < 64; ++i)
            {
                wideName[i] = static_cast<wchar_t>(name[i]);
                if (name[i] == '\0') break;
            }

            typedef HRESULT(WINAPI* SetThreadDescriptionFn)(HANDLE, PCWSTR);
            HMODULE hKernel32 = GetModuleHandleW(L"kernel32.dll");
            if (hKernel32)
            {
                auto pSetThreadDescription = (SetThreadDescriptionFn)GetProcAddress(hKernel32, "SetThreadDescription");
                if (pSetThreadDescription)
                {
                    HRESULT hr = pSetThreadDescription(hWinThread, wideName);
                    return SUCCEEDED(hr) ? eResult::OK : eResult::Error;
                }
            }
            return eResult::NotSupported;
        }
    }

    // =========================================================================
    // SYNCHRONIZATION IMPLEMENTATION (Windows Kernel Sync Objects)
    // =========================================================================
    namespace Sync
    {
#undef CreateMutex
        int32 CreateMutex(MutexHandle& hMutex, bool recursive)
        {
            CRITICAL_SECTION* cs = new CRITICAL_SECTION{};
            ::InitializeCriticalSectionAndSpinCount(cs, 4000);
            hMutex.ptr = cs;
            return eResult::OK;
        }

        void DestroyMutex(MutexHandle hMutex)
        {
            CRITICAL_SECTION* cs = reinterpret_cast<CRITICAL_SECTION*>(hMutex.ptr);
            if (cs)
            {
                ::DeleteCriticalSection(cs);
                delete cs;
            }
        }

        void LockMutex(MutexHandle hMutex)
        {
            CRITICAL_SECTION* cs = reinterpret_cast<CRITICAL_SECTION*>(hMutex.ptr);
            if (cs)
            {
                ::EnterCriticalSection(cs);
            }
        }

        bool TryLockMutex(MutexHandle hMutex)
        {
            CRITICAL_SECTION* cs = reinterpret_cast<CRITICAL_SECTION*>(hMutex.ptr);
            if (cs)
            {
                return ::TryEnterCriticalSection(cs) != FALSE;
            }
            return false;
        }

        void UnlockMutex(MutexHandle hMutex)
        {
            CRITICAL_SECTION* cs = reinterpret_cast<CRITICAL_SECTION*>(hMutex.ptr);
            if (cs)
            {
                ::LeaveCriticalSection(cs);
            }
        }

#undef CreateSemaphore
        int32 CreateSemaphore(uint32 initialCount, uint32 maxCount, SemaphoreHandle& hSemaphore)
        {
            // Replaced CreateSemaphoreA with CreateSemaphoreW (Unicode wide-character version)
            HANDLE hWinSemaphore = ::CreateSemaphoreW(nullptr, static_cast<LONG>(initialCount), static_cast<LONG>(maxCount), nullptr);
            if (!hWinSemaphore)
            {
                return eResult::Error;
            }
            hSemaphore.ptr = hWinSemaphore;
            return eResult::OK;
        }

        void DestroySemaphore(SemaphoreHandle hSemaphore)
        {
            HANDLE hWinSemaphore = reinterpret_cast<HANDLE>(hSemaphore.ptr);
            if (hWinSemaphore)
            {
                ::CloseHandle(hWinSemaphore);
            }
        }

        int32 WaitSemaphore(SemaphoreHandle hSemaphore, uint32 timeoutMs)
        {
            HANDLE hWinSemaphore = reinterpret_cast<HANDLE>(hSemaphore.ptr);
            if (!hWinSemaphore) return eResult::UnvalidHandle;

            DWORD timeout = (timeoutMs == UINT32_MAX) ? INFINITE : static_cast<DWORD>(timeoutMs);
            DWORD result = ::WaitForSingleObject(hWinSemaphore, timeout);

            if (result == WAIT_OBJECT_0)
            {
                return eResult::OK;
            }
            else if (result == WAIT_TIMEOUT)
            {
                return eResult::Timeout;
            }
            return eResult::Error;
        }

        int32 SignalSemaphore(SemaphoreHandle hSemaphore, uint32 count)
        {
            HANDLE hWinSemaphore = reinterpret_cast<HANDLE>(hSemaphore.ptr);
            if (!hWinSemaphore) return eResult::UnvalidHandle;

            if (::ReleaseSemaphore(hWinSemaphore, static_cast<LONG>(count), nullptr))
            {
                return eResult::OK;
            }
            return eResult::Error;
        }

        int32 CreateConditionVariable(ConditionVariableHandle& hCondVar)
        {
            CONDITION_VARIABLE* cv = new CONDITION_VARIABLE{};
            ::InitializeConditionVariable(cv);
            hCondVar.ptr = cv;
            return eResult::OK;
        }

        void DestroyConditionVariable(ConditionVariableHandle hCondVar)
        {
            CONDITION_VARIABLE* cv = reinterpret_cast<CONDITION_VARIABLE*>(hCondVar.ptr);
            if (cv)
            {
                delete cv;
            }
        }

        int32 WaitConditionVariable(ConditionVariableHandle hCondVar, MutexHandle hMutex, uint32 timeoutMs)
        {
            CONDITION_VARIABLE* cv = reinterpret_cast<CONDITION_VARIABLE*>(hCondVar.ptr);
            CRITICAL_SECTION* cs = reinterpret_cast<CRITICAL_SECTION*>(hMutex.ptr);

            if (!cv || !cs) return eResult::InvalidArgument;

            DWORD timeout = (timeoutMs == UINT32_MAX) ? INFINITE : static_cast<DWORD>(timeoutMs);
            BOOL result = ::SleepConditionVariableCS(cv, cs, timeout);

            if (result)
            {
                return eResult::OK;
            }
            else
            {
                DWORD err = ::GetLastError();
                if (err == ERROR_TIMEOUT)
                {
                    return eResult::Timeout;
                }
            }
            return eResult::Error;
        }

        void SignalConditionVariable(ConditionVariableHandle hCondVar)
        {
            CONDITION_VARIABLE* cv = reinterpret_cast<CONDITION_VARIABLE*>(hCondVar.ptr);
            if (cv)
            {
                ::WakeConditionVariable(cv);
            }
        }

        void BroadcastConditionVariable(ConditionVariableHandle hCondVar)
        {
            CONDITION_VARIABLE* cv = reinterpret_cast<CONDITION_VARIABLE*>(hCondVar.ptr);
            if (cv)
            {
                ::WakeAllConditionVariable(cv);
            }
        }
    }

    // =========================================================================
    // ATOMIC IMPLEMENTATION (Hardware Instructions & Barriers)
    // =========================================================================
    namespace Atomic
    {
#undef MemoryBarrier
        void MemoryBarrier()
        {
            __faststorefence();
        }

        void CompilerBarrier()
        {
            _ReadWriteBarrier();
        }

        // 8-bit Small Atomics
        int8 Load8(const volatile int8* destination, eMemoryOrder order)
        {
            CompilerBarrier();
            int8 value = *destination;
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            return value;
        }

        void Store8(volatile int8* destination, int8 value, eMemoryOrder order)
        {
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            else
            {
                CompilerBarrier();
            }
            *destination = value;
            if (order == eMemoryOrder::Release || order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
        }

        int8 Exchange8(volatile int8* destination, int8 exchangeValue, eMemoryOrder order)
        {
            return _InterlockedExchange8(reinterpret_cast<volatile char*>(destination), exchangeValue);
        }

        // 16-bit Small Atomics
        int16 Load16(const volatile int16* destination, eMemoryOrder order)
        {
            CompilerBarrier();
            int16 value = *destination;
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            return value;
        }

        void Store16(volatile int16* destination, int16 value, eMemoryOrder order)
        {
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            else
            {
                CompilerBarrier();
            }
            *destination = value;
            if (order == eMemoryOrder::Release || order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
        }

        int16 Exchange16(volatile int16* destination, int16 exchangeValue, eMemoryOrder order)
        {
            return _InterlockedExchange16(reinterpret_cast<volatile SHORT*>(destination), exchangeValue);
        }

        // 32-bit Integer Atomics
        int32 Load32(const volatile int32* destination, eMemoryOrder order)
        {
            CompilerBarrier();
            int32 value = *destination;
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            return value;
        }

        void Store32(volatile int32* destination, int32 value, eMemoryOrder order)
        {
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            else
            {
                CompilerBarrier();
            }
            *destination = value;
            if (order == eMemoryOrder::Release || order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
        }

        int32 Exchange32(volatile int32* destination, int32 exchangeValue, eMemoryOrder order)
        {
            return _InterlockedExchange(reinterpret_cast<volatile LONG*>(destination), exchangeValue);
        }

        bool CompareExchange32(volatile int32* destination, int32* expectedValue, int32 desiredValue, eMemoryOrder successOrder, eMemoryOrder failOrder)
        {
            int32 original = *expectedValue;
            int32 prev = _InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(destination), desiredValue, original);
            if (prev == original)
            {
                return true;
            }
            else
            {
                *expectedValue = prev;
                return false;
            }
        }

        int32 FetchAdd32(volatile int32* destination, int32 operand, eMemoryOrder order)
        {
            return _InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(destination), operand);
        }

        int32 FetchSub32(volatile int32* destination, int32 operand, eMemoryOrder order)
        {
            return _InterlockedExchangeAdd(reinterpret_cast<volatile LONG*>(destination), -operand);
        }

        int32 FetchAnd32(volatile int32* destination, int32 operand, eMemoryOrder order)
        {
            return _InterlockedAnd(reinterpret_cast<volatile LONG*>(destination), operand);
        }

        int32 FetchOr32(volatile int32* destination, int32 operand, eMemoryOrder order)
        {
            return _InterlockedOr(reinterpret_cast<volatile LONG*>(destination), operand);
        }

        int32 FetchXor32(volatile int32* destination, int32 operand, eMemoryOrder order)
        {
            return _InterlockedXor(reinterpret_cast<volatile LONG*>(destination), operand);
        }

        // 32-bit Float Atomics
        float LoadFloat32(const volatile float* destination, eMemoryOrder order)
        {
            int32 intVal = Load32(reinterpret_cast<const volatile int32*>(destination), order);
            return *reinterpret_cast<const float*>(&intVal);
        }

        void StoreFloat32(volatile float* destination, float value, eMemoryOrder order)
        {
            int32 intVal = *reinterpret_cast<int32*>(&value);
            Store32(reinterpret_cast<volatile int32*>(destination), intVal, order);
        }

        float ExchangeFloat32(volatile float* destination, float exchangeValue, eMemoryOrder order)
        {
            int32 exVal = *reinterpret_cast<int32*>(&exchangeValue);
            int32 prev = Exchange32(reinterpret_cast<volatile int32*>(destination), exVal, order);
            return *reinterpret_cast<float*>(&prev);
        }

        bool CompareExchangeFloat32(volatile float* destination, float* expectedValue, float desiredValue, eMemoryOrder successOrder, eMemoryOrder failOrder)
        {
            int32* expInt = reinterpret_cast<int32*>(expectedValue);
            int32 desInt = *reinterpret_cast<int32*>(&desiredValue);
            return CompareExchange32(reinterpret_cast<volatile int32*>(destination), expInt, desInt, successOrder, failOrder);
        }

        float FetchAddFloat32(volatile float* destination, float operand, eMemoryOrder order)
        {
            float current;
            float desired;
            do
            {
                current = LoadFloat32(destination, eMemoryOrder::Relaxed);
                desired = current + operand;
            } while (!CompareExchangeFloat32(destination, &current, desired, order, eMemoryOrder::Relaxed));
            return current;
        }

        float FetchSubFloat32(volatile float* destination, float operand, eMemoryOrder order)
        {
            return FetchAddFloat32(destination, -operand, order);
        }

        // 64-bit Integer Atomics
        int64 Load64(const volatile int64* destination, eMemoryOrder order)
        {
            CompilerBarrier();
            int64 value = *destination;
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            return value;
        }

        void Store64(volatile int64* destination, int64 value, eMemoryOrder order)
        {
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            else
            {
                CompilerBarrier();
            }
            *destination = value;
            if (order == eMemoryOrder::Release || order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
        }

        int64 Exchange64(volatile int64* destination, int64 exchangeValue, eMemoryOrder order)
        {
            return _InterlockedExchange64(reinterpret_cast<volatile LONGLONG*>(destination), exchangeValue);
        }

        bool CompareExchange64(volatile int64* destination, int64* expectedValue, int64 desiredValue, eMemoryOrder successOrder, eMemoryOrder failOrder)
        {
            int64 original = *expectedValue;
            int64 prev = _InterlockedCompareExchange64(reinterpret_cast<volatile LONGLONG*>(destination), desiredValue, original);
            if (prev == original)
            {
                return true;
            }
            else
            {
                *expectedValue = prev;
                return false;
            }
        }

        int64 FetchAdd64(volatile int64* destination, int64 operand, eMemoryOrder order)
        {
            return _InterlockedExchangeAdd64(reinterpret_cast<volatile LONGLONG*>(destination), operand);
        }

        int64 FetchSub64(volatile int64* destination, int64 operand, eMemoryOrder order)
        {
            return _InterlockedExchangeAdd64(reinterpret_cast<volatile LONGLONG*>(destination), -operand);
        }

        int64 FetchAnd64(volatile int64* destination, int64 operand, eMemoryOrder order)
        {
            return _InterlockedAnd64(reinterpret_cast<volatile LONGLONG*>(destination), operand);
        }

        int64 FetchOr64(volatile int64* destination, int64 operand, eMemoryOrder order)
        {
            return _InterlockedOr64(reinterpret_cast<volatile LONGLONG*>(destination), operand);
        }

        int64 FetchXor64(volatile int64* destination, int64 operand, eMemoryOrder order)
        {
            return _InterlockedXor64(reinterpret_cast<volatile LONGLONG*>(destination), operand);
        }

        // 64-bit Float Atomics (Double)
        float64 LoadFloat64(const volatile float64* destination, eMemoryOrder order)
        {
            int64 intVal = Load64(reinterpret_cast<const volatile int64*>(destination), order);
            return *reinterpret_cast<const float64*>(&intVal);
        }

        void StoreFloat64(volatile float64* destination, float64 value, eMemoryOrder order)
        {
            int64 intVal = *reinterpret_cast<int64*>(&value);
            Store64(reinterpret_cast<volatile int64*>(destination), intVal, order);
        }

        float64 ExchangeFloat64(volatile float64* destination, float64 exchangeValue, eMemoryOrder order)
        {
            int64 exVal = *reinterpret_cast<int64*>(&exchangeValue);
            int64 prev = Exchange64(reinterpret_cast<volatile int64*>(destination), exVal, order);
            return *reinterpret_cast<float64*>(&prev);
        }

        bool CompareExchangeFloat64(volatile float64* destination, float64* expectedValue, float64 desiredValue, eMemoryOrder successOrder, eMemoryOrder failOrder)
        {
            int64* expInt = reinterpret_cast<int64*>(expectedValue);
            int64 desInt = *reinterpret_cast<int64*>(&desiredValue);
            return CompareExchange64(reinterpret_cast<volatile int64*>(destination), expInt, desInt, successOrder, failOrder);
        }

        float64 FetchAddFloat64(volatile float64* destination, float64 operand, eMemoryOrder order)
        {
            float64 current;
            float64 desired;
            do
            {
                current = LoadFloat64(destination, eMemoryOrder::Relaxed);
                desired = current + operand;
            } while (!CompareExchangeFloat64(destination, &current, desired, order, eMemoryOrder::Relaxed));
            return current;
        }

        float64 FetchSubFloat64(volatile float64* destination, float64 operand, eMemoryOrder order)
        {
            return FetchAddFloat64(destination, -operand, order);
        }

        // Pointer Atomics
        void* LoadPointer(const volatile void** destination, eMemoryOrder order)
        {
            CompilerBarrier();
            void* value = const_cast<void*>(*destination);
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            return value;
        }

        void StorePointer(volatile void** destination, void* value, eMemoryOrder order)
        {
            if (order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
            else
            {
                CompilerBarrier();
            }
            *destination = value;
            if (order == eMemoryOrder::Release || order == eMemoryOrder::SeqCst)
            {
                MemoryBarrier();
            }
        }

        void* ExchangePointer(volatile void** destination, void* exchangeValue, eMemoryOrder order)
        {
#if defined(_WIN64)
            return _InterlockedExchangePointer(const_cast<void* volatile*>(destination), exchangeValue);
#else
            return reinterpret_cast<void*>(_InterlockedExchange(reinterpret_cast<volatile LONG*>(destination), reinterpret_cast<LONG>(exchangeValue)));
#endif
        }

        bool CompareExchangePointer(volatile void** destination, void** expectedValue, void* desiredValue, eMemoryOrder successOrder, eMemoryOrder failOrder)
        {
            void* original = *expectedValue;
#if defined(_WIN64)
            void* prev = _InterlockedCompareExchangePointer(const_cast<void* volatile*>(destination), desiredValue, original);
#else
            void* prev = reinterpret_cast<void*>(_InterlockedCompareExchange(reinterpret_cast<volatile LONG*>(destination), reinterpret_cast<LONG>(desiredValue), reinterpret_cast<LONG>(original)));
#endif
            if (prev == original)
            {
                return true;
            }
            else
            {
                *expectedValue = prev;
                return false;
            }
        }

        // Atomic Flag / Spinlocks
        bool TestAndSet(volatile int8* flag, eMemoryOrder order)
        {
            char prev = _InterlockedExchange8(reinterpret_cast<volatile char*>(flag), 1);
            return prev != 0;
        }

        void ClearFlag(volatile int8* flag, eMemoryOrder order)
        {
            Store8(flag, 0, eMemoryOrder::Release);
        }
    }
}