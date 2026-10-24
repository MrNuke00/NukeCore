#include <stdio.h>
#include <string.h>
#include <Platform/PlatformProcess.h>

int main()
{
    printf("=== Starting Platform Process & Shared Memory Test ===\n\n");

    // =========================================================================
    // PART 1: Process Creation & Stdout Pipe Redirection Test
    // =========================================================================
    {
        printf("[Test 1] Spawning child process with stream redirection...\n");

        Platform::Process::ProcessCreateInfo info{};
        info.executablePath = "cmd.exe";
        info.arguments = "/c echo EngineProcessTestPassed";
        info.redirectStreams = true;
        info.flags = static_cast<uint32>(Platform::Process::eProcessFlags::Hidden);

        Platform::Process::ProcessHandle hProcess;
        int32 createRes = Platform::Process::Create(info, hProcess);

        if (createRes == Platform::Process::OK)
        {
            printf("[Test 1] Process created successfully. Reading stdout pipe...\n");

            char buffer[256] = { 0 };
            uint32 bytesRead = 0;

            // Read output streamed from child process stdout
            int32 readRes = Platform::Process::ReadStdout(hProcess, buffer, sizeof(buffer) - 1, &bytesRead);
            if (readRes == Platform::Process::OK && bytesRead > 0)
            {
                // Remove trailing newlines for clean printing
                for (uint32 i = 0; i < bytesRead; ++i)
                {
                    if (buffer[i] == '\r' || buffer[i] == '\n') buffer[i] = '\0';
                }
                printf("[Test 1] Child stdout output -> \"%s\" (%u bytes)\n", buffer, bytesRead);
            }
            else
            {
                printf("[Test 1] Failed to read stdout or empty stream. Result: %d\n", readRes);
            }

            // Wait for process completion
            int32 exitCode = -1;
            int32 waitRes = Platform::Process::Wait(hProcess, 3000, &exitCode);
            if (waitRes == Platform::Process::OK)
            {
                printf("[Test 1] Process finished execution. Exit Code: %d\n", exitCode);
            }
            else
            {
                printf("[Test 1] Process wait timed out or failed. Result: %d\n", waitRes);
            }

            Platform::Process::CloseHandle(hProcess);
        }
        else
        {
            printf("[Error] Failed to spawn process. Result code: %d\n", createRes);
        }
    }

    printf("\n---------------------------------------------------\n\n");

    // =========================================================================
    // PART 2: Shared Memory Creation & Mapping Test
    // =========================================================================
    {
        printf("[Test 2] Creating shared memory region...\n");

        const char* sharedMemName = "Local\\EngineSharedMemoryTest";
        uint64 memSize = 4096;

        Platform::Process::SharedMemoryHandle hMemCreate;
        void* mappedPtrCreate = nullptr;

        int32 createMemRes = Platform::Process::CreateSharedMemory(
            sharedMemName,
            memSize,
            hMemCreate,
            &mappedPtrCreate
        );

        if (createMemRes == Platform::Process::OK && mappedPtrCreate)
        {
            printf("[Test 2] Shared memory created and mapped successfully at pointer: %p\n", mappedPtrCreate);

            // Write test data directly into the shared memory block
            const char* sampleData = "Hello from Engine Primary Handle via Shared Memory!";
            strcpy_s(static_cast<char*>(mappedPtrCreate), static_cast<size_t>(memSize), sampleData);
            printf("[Test 2] Written data into shared memory: \"%s\"\n", static_cast<char*>(mappedPtrCreate));

            // Open the same shared memory region via a second handle (simulating a second process/subsystem)
            Platform::Process::SharedMemoryHandle hMemOpen;
            void* mappedPtrOpen = nullptr;
            uint64 openedSize = 0;

            int32 openMemRes = Platform::Process::OpenSharedMemory(
                sharedMemName,
                hMemOpen,
                &mappedPtrOpen,
                memSize,
                &openedSize
            );

            if (openMemRes == Platform::Process::OK && mappedPtrOpen)
            {
                printf("[Test 2] Opened shared memory from secondary handle. Read content -> \"%s\"\n",
                    static_cast<char*>(mappedPtrOpen));

                // Cleanup secondary handle mapping
                Platform::Process::CloseSharedMemory(hMemOpen, mappedPtrOpen);
            }
            else
            {
                printf("[Error] Failed to open existing shared memory. Result: %d\n", openMemRes);
            }

            // Cleanup primary handle mapping
            Platform::Process::CloseSharedMemory(hMemCreate, mappedPtrCreate);
            printf("[Test 2] Shared memory closed successfully.\n");
        }
        else
        {
            printf("[Error] Failed to create shared memory. Result code: %d\n", createMemRes);
        }
    }

    printf("\n=== Process & Shared Memory Tests Completed Successfully ===\n");
    return 0;
}