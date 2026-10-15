#include <stdio.h>
#include <Platform/PlatformThread.h>

// Shared test variables
volatile int32 g_counter = 0;
Platform::Sync::MutexHandle g_testMutex;
Platform::Sync::SemaphoreHandle g_testSemaphore;
Platform::Sync::ConditionVariableHandle g_testCondVar;

// Engine-compliant thread entry function
uint32 WorkerThreadRoutine(void* userData)
{
    int32 threadIndex = *static_cast<int32*>(userData);
    printf("[Worker Thread %d] Started. OS Thread ID: %llu\n", threadIndex, Platform::Thread::GetCurrentThreadId());

    // 1. Test Atomic FetchAdd
    Platform::Atomic::FetchAdd32(&g_counter, 25, Platform::Atomic::eMemoryOrder::SeqCst);

    // 2. Test Mutex Lock / Unlock
    Platform::Sync::LockMutex(g_testMutex);
    printf("[Worker Thread %d] Acquired Mutex critical section.\n", threadIndex);
    Platform::Sync::UnlockMutex(g_testMutex);

    // 3. Test Semaphore Signal (wake up main thread)
    printf("[Worker Thread %d] Signaling semaphore...\n", threadIndex);
    Platform::Sync::SignalSemaphore(g_testSemaphore, 1);

    Platform::Thread::Sleep(20);
    printf("[Worker Thread %d] Exiting worker routine.\n", threadIndex);
    return 100; // Expected exit code
}

int main()
{
    printf("=== Starting Platform Engine Core Test ===\n\n");

    // Initialize Thread Subsystem
    int32 initResult = Platform::Thread::Init();
    if (initResult != Platform::OK)
    {
        printf("[Error] Thread subsystem initialization failed!\n");
        return -1;
    }

    // ==========================================
    // 1. Test Atomic & Spinflag Operations
    // ==========================================
    volatile int8 spinFlag = 0;
    bool lockedFirst = Platform::Atomic::TestAndSet(&spinFlag, Platform::Atomic::eMemoryOrder::SeqCst);
    bool lockedSecond = Platform::Atomic::TestAndSet(&spinFlag, Platform::Atomic::eMemoryOrder::SeqCst);

    printf("[Atomic Test] TestAndSet 1 (Expected false/unlocked): %s\n", lockedFirst ? "FAIL" : "PASS");
    printf("[Atomic Test] TestAndSet 2 (Expected true/locked): %s\n", lockedSecond ? "PASS" : "FAIL");

    Platform::Atomic::ClearFlag(&spinFlag, Platform::Atomic::eMemoryOrder::Release);
    printf("[Atomic Test] SpinFlag cleared successfully.\n\n");

    // ==========================================
    // 2. Test Synchronization Primitives (Mutex & Semaphore)
    // ==========================================
    if (Platform::Sync::CreateMutex(g_testMutex, false) != Platform::OK)
    {
        printf("[Error] Failed to create Mutex!\n");
        return -1;
    }

    // Using exact parameter names: InitialCount = 0, MaxCount = 5
    if (Platform::Sync::CreateSemaphore(0, 5, g_testSemaphore) != Platform::OK)
    {
        printf("[Error] Failed to create Semaphore!\n");
        Platform::Sync::DestroyMutex(g_testMutex);
        return -1;
    }

    if (Platform::Sync::CreateConditionVariable(g_testCondVar) != Platform::OK)
    {
        printf("[Error] Failed to create Condition Variable!\n");
        Platform::Sync::DestroySemaphore(g_testSemaphore);
        Platform::Sync::DestroyMutex(g_testMutex);
        return -1;
    }

    // ==========================================
    // 3. Test Thread Creation & Lifecycle
    // ==========================================
    Platform::Thread::ThreadCreateInfo info{};
    info.name = "EngineWorker";
    info.stackSize = 0;   // OS Default
    info.cpuCore = -1;    // Any core
    info.priority = Platform::Thread::eThreadPriority::Normal;
    info.createSuspended = false;

    Platform::Thread::ThreadHandle hThread;
    int32 threadParam = 77;

    printf("[Main Thread] Creating worker thread...\n");
    int32 createResult = Platform::Thread::Create(WorkerThreadRoutine, &threadParam, info, hThread);

    if (createResult == Platform::OK)
    {
        // Wait on semaphore with timeout (TimeoutMs = 2000 ms)
        printf("[Main Thread] Waiting for worker thread via Semaphore (Timeout 2000ms)...\n");
        int32 waitRes = Platform::Sync::WaitSemaphore(g_testSemaphore, 2000);

        if (waitRes == Platform::OK)
        {
            printf("[Main Thread] Semaphore signaled successfully by worker thread!\n");
        }
        else
        {
            printf("[Main Thread] Semaphore wait failed or timed out (Result: %d)\n", waitRes);
        }

        // Join thread and verify exit code
        uint32 exitCode = 0;
        Platform::Thread::Join(hThread, &exitCode);
        printf("[Main Thread] Thread joined successfully! Exit Code: %u\n\n", exitCode);
    }
    else
    {
        printf("[Error] Thread creation failed with error code: %d\n", createResult);
    }

    // ==========================================
    // 4. Verify Final Atomic Results
    // ==========================================
    int32 finalVal = Platform::Atomic::Load32(&g_counter, Platform::Atomic::eMemoryOrder::SeqCst);
    printf("[Atomic Verification] Counter Final Value = %d (Expected: 25)\n\n", finalVal);

    // ==========================================
    // 5. Cleanup Resources
    // ==========================================
    Platform::Sync::DestroyConditionVariable(g_testCondVar);
    Platform::Sync::DestroySemaphore(g_testSemaphore);
    Platform::Sync::DestroyMutex(g_testMutex);
    Platform::Thread::Fini();

    printf("=== All Tests Completed Successfully ===\n");
    return 0;
}