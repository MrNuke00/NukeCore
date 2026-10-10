#pragma once
#include <Platform/PlatformCore.h>

#define PLATFORM_FILE_API API_DECL

namespace eResult {
	enum : iResult
	{
		PlatformFileResult = PLATFORM_FILE_RESULT_BIT,
		//...
	};
};

#define PATH(TEXT) (UTF8_STR(TEXT))

// File & Directory Information
struct FileInfo
{
	uint64 size				= 0;
	uint64 creationTime		= 0; // POSIX timestamp
	uint64 lastAccessTime	= 0;
	uint64 lastWriteTime	= 0;
	bool   isDirectory		= false;
	bool   isReadOnly		= false;
	bool   isHidden			= false;
};

enum class eSeekOrigin : int32
{
	Begin   = 0,
	Current = 1,
	End     = 2
};

using iFileOpenMode = int32;
enum eFileOpenMode : int32
{
	None		= 0,
	Read		= 1 << 0,
	Write		= 1 << 1,
	Append		= 1 << 2,
	Create		= 1 << 3, // Creates file if it does not exist
	Truncate	= 1 << 4, // Truncates existing file size to 0
	ShareRead	= 1 << 5, // Allow concurrent reads by other processes
	ShareWrite	= 1 << 6, // Allow concurrent writes by other processes
	Binary		= 1 << 7  // Open in binary mode
};

// Entry payload used when enumerating folders
struct DirectoryEntry
{
	const char8* name = nullptr;
	FileInfo     info;
};

DEFINE_HANDLE(File);	// FileHandle
DEFINE_HANDLE(FileMap); // FileMapHandle

// Function pointer callback for directory iteration (return false to stop iteration)
using DirectoryIterCallback = bool(*)(const DirectoryEntry& entry, void* userData);

START_PLATFORM_MODULE(PlatformFile, PLATFORM_FILE_API)

static iResult Open(FileHandle handle, PathStr path, iFileOpenMode mode);

static iResult Close(FileHandle handle);

static iResult Read(FileHandle handle, void* buffer, uint64 bytesToRead, uint64* bytesRead = nullptr);

static iResult Write(FileHandle handle, const void* buffer, uint64 bytesToWrite, uint64* bytesWritten = nullptr);

static iResult Seek(FileHandle handle, int64 offset, eSeekOrigin origin, uint64* newPosition = nullptr);

static iResult Tell(FileHandle handle, uint64& outPosition);

static iResult Flush(FileHandle handle);

static iResult GetSize(FileHandle handle, uint64& outSize);

static iResult SetSize(FileHandle handle, uint64 newSize);

static bool IsValid(FileHandle handle);

static iResult Exists(PathStr path, bool& outExists);

static iResult DeleteFile(PathStr path);

static iResult Move(PathStr sourcePath, PathStr targetPath, bool overwrite = false);

static iResult Copy(PathStr sourcePath, PathStr targetPath, bool overwrite = false);

static iResult GetFileInfo(PathStr path, FileInfo& outInfo);

static iResult GetFileInfo(FileHandle handle, FileInfo& outInfo);

static iResult CreateDirectory(PathStr path);

static iResult DeleteDirectory(PathStr path, bool recursive = false);

static iResult DirectoryExists(PathStr path, bool& outExists);

static iResult IterateDirectory(PathStr path, DirectoryIterCallback callback, void* userData = nullptr);

static bool MapToMemory(PathStr path, FileMapHandle& hMapping, void* pData, uint64& FileSize);

static void UnmapMemory(FileMapHandle& hMapping);

END_PLATFORM_MODULE();


