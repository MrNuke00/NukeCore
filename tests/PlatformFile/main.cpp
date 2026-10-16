
#include <iostream>
#include <Platform/PlatformFile.h>

// Helper callback for directory iteration
bool PrintDirectoryEntry(const Platform::File::DirectoryEntry& Entry, void* pUserData)
{
    std::wcout << L"    - [Entry] Name: " << Entry.Name
        << L" | IsDirectory: " << (Entry.IsDirectory ? L"True" : L"False")
        << L" | Size: " << Entry.Size << L" bytes" << std::endl;
    return true; // Continue iteration
}

int main()
{
    std::wcout << L"=== Starting Platform::File System Comprehensive Test ===" << std::endl;

    const wchar_t* testDir = L"TestEngineData";
    const wchar_t* testFilePath = L"TestEngineData\\test_archive.pkg";
    const wchar_t* copyFilePath = L"TestEngineData\\test_archive_copy.pkg";

    // ==========================================
    // 1. Test Directory Operations
    // ==========================================
    std::wcout << L"\n[1] Testing Directory Operations..." << std::endl;

    int32 dirRes = Platform::File::CreateDirectory(testDir);
    if (dirRes == Platform::File::eResult::OK) {
        std::wcout << L"  -> CreateDirectory succeeded." << std::endl;
    }

    if (Platform::File::DirectoryExists(testDir)) {
        std::wcout << L"  -> DirectoryExists verified." << std::endl;
    }

    // ==========================================
    // 2. Test File Open (Write Mode), Write, Flush, Close
    // ==========================================
    std::wcout << L"\n[2] Testing File Write & Open..." << std::endl;

    Platform::File::FileHandle hFile = {};
    int32 openWriteRes = Platform::File::Open(
        testFilePath,
        hFile,
        Platform::File::eFileMode::FileModeWrite | Platform::File::eFileMode::FileModeCreateNew
    );

    if (openWriteRes == Platform::File::eResult::OK && hFile) {
        std::wcout << L"  -> File opened for writing successfully." << std::endl;
    }
    else {
        std::wcout << L"  -> Failed to open file for writing! Error code: " << openWriteRes << std::endl;
        return 1;
    }

    const char* sampleData = "ENGINE_ASSET_PACKAGE_DATA_V1.0_GAMELOOP";
    uint64 bytesWritten = 0;
    int32 writeRes = Platform::File::Write(hFile, sampleData, 39, &bytesWritten);

    if (writeRes == Platform::File::eResult::OK) {
        std::wcout << L"  -> Successfully wrote " << bytesWritten << L" bytes." << std::endl;
    }

    Platform::File::Flush(hFile);
    Platform::File::Close(hFile);
    std::wcout << L"  -> File flushed and closed." << std::endl;

    // ==========================================
    // 3. Test FileInfo & Exists
    // ==========================================
    std::wcout << L"\n[3] Testing FileInfo & Exists..." << std::endl;

    if (Platform::File::Exists(testFilePath)) {
        std::wcout << L"  -> Exists(Path) verified." << std::endl;
    }

    Platform::File::FileInfo info = {};
    if (Platform::File::GetFileInfo(testFilePath, info) == Platform::File::eResult::OK) {
        std::wcout << L"  -> GetFileInfo (Path) Size: " << info.Size << L" bytes" << std::endl;
        std::wcout << L"  -> Permissions -> CanRead: " << (info.CanRead ? L"Yes" : L"No")
            << L", CanWrite: " << (info.CanWrite ? L"Yes" : L"No") << std::endl;
    }

    // Reopen handle to test handle-based GetFileInfo
    Platform::File::Open(testFilePath, hFile, Platform::File::eFileMode::FileModeRead);
    if (hFile) {
        Platform::File::FileInfo handleInfo = {};
        if (Platform::File::GetFileInfo(hFile, handleInfo) == Platform::File::eResult::OK) {
            std::wcout << L"  -> GetFileInfo (Handle) Size: " << handleInfo.Size << L" bytes" << std::endl;
        }
    }

    // ==========================================
    // 4. Test Read, Seek, Tell
    // ==========================================
    std::wcout << L"\n[4] Testing Read, Seek, and Tell..." << std::endl;

    char readBuffer[64] = { 0 };
    uint64 bytesRead = 0;

    // Seek to beginning and verify position via Tell
    Platform::File::Seek(hFile, 0, Platform::File::eSeekOrigin::Begin);
    uint64 currentPos = 0;
    Platform::File::Tell(hFile, currentPos);
    std::wcout << L"  -> Current Position (Tell) after seek begin: " << currentPos << std::endl;

    // Read first 6 bytes ("ENGINE")
    Platform::File::Read(hFile, readBuffer, 6, &bytesRead);
    readBuffer[bytesRead] = '\0';
    std::wcout << L"  -> Read " << bytesRead << L" bytes: " << readBuffer << std::endl;

    Platform::File::Tell(hFile, currentPos);
    std::wcout << L"  -> Current Position after read: " << currentPos << std::endl;

    Platform::File::Close(hFile);

    // ==========================================
    // 5. Test Copy & Management Utilities
    // ==========================================
    std::wcout << L"\n[5] Testing File Copy..." << std::endl;
    int32 copyRes = Platform::File::Copy(testFilePath, copyFilePath, true);
    if (copyRes == Platform::File::eResult::OK) {
        std::wcout << L"  -> File copied successfully." << std::endl;
    }

    // ==========================================
    // 6. Test Directory Iteration
    // ==========================================
    std::wcout << L"\n[6] Testing Directory Iteration on '" << testDir << L"':" << std::endl;
    Platform::File::IterateDirectory(testDir, PrintDirectoryEntry, nullptr);

    // ==========================================
    // 7. Test Memory Mapping (Direct Zero-Copy Access)
    // ==========================================
    std::wcout << L"\n[7] Testing Memory Mapping..." << std::endl;

    Platform::File::FileMapHandle hMapping = {};
    uint64 maxSize = 0;

    int32 mapCreateRes = Platform::File::CreateMemoryMap(testFilePath, hMapping, &maxSize);
    if (mapCreateRes == Platform::File::eResult::OK && hMapping) {
        std::wcout << L"  -> CreateMemoryMap succeeded. MaxSize: " << maxSize << L" bytes" << std::endl;

        void* pMappedData = nullptr;
        if (Platform::File::MapMemory(hMapping, pMappedData, 0, maxSize)) {
            std::wcout << L"  -> MapMemory mapped view successfully at pointer: " << pMappedData << std::endl;

            // Access data directly from disk via virtual memory paging (Zero-Copy!)
            char* mappedChars = static_cast<char*>(pMappedData);
            std::wcout << L"  -> Direct Memory Inspection: [ ";
            for (uint64 i = 0; i < maxSize; ++i) {
                std::cout << mappedChars[i];
            }
            std::cout << " ]" << std::endl;

            // Unmap using the handle-based safety implementation
            Platform::File::UnmapMemory(hMapping);
            std::wcout << L"  -> UnmapMemory executed via handle." << std::endl;
        }

        Platform::File::DestroyMemoryMap(hMapping);
        std::wcout << L"  -> DestroyMemoryMap completed." << std::endl;
    }
    else {
        std::wcout << L"  -> CreateMemoryMap failed with code: " << mapCreateRes << std::endl;
    }

    // ==========================================
    // 8. Cleanup
    // ==========================================
    std::wcout << L"\n[8] Cleaning up test files and directories..." << std::endl;
    Platform::File::DeleteFile(testFilePath);
    Platform::File::DeleteFile(copyFilePath);
    Platform::File::DeleteDirectory(testDir, true);
    std::wcout << L"  -> Cleanup finished." << std::endl;

    std::wcout << L"\n=== All Tests Completed Successfully! ===" << std::endl;
    return 0;
}