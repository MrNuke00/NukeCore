#pragma once
#include <Platform/PlatformCore.h>

// PlatformFile
namespace Platform::File
{

	enum eResult : int32
	{
		PLATFORM_RESULT(PLATFORM_FILE_RESULT_BIT),
		FileNotFound,
		FileAlreadyExists,
		AccessDenied,
		SharingViolation,
		InvalidParameter,
		EndOfFile,
		DiskFull
	};

	DEFINE_HANDLE(File);    // FileHandle
	DEFINE_HANDLE(FileMap); // FileMapHandle

	// File Open Modes (Prefixed to avoid colliding with Read/Write functions)
	enum eFileMode : uint32
	{
		FileModeRead		= 1	<< 0,
		FileModeWrite		= 1	<< 1,
		FileModeAppend		= 1	<< 2,
		FileModeCreateNew	= 1	<< 3
	};

	// Mode changed to uint32 to support bitwise flag combinations (e.g., Read | Write)
	int32 Open(PATH_C_STR Path, FileHandle& hFile, const uint32 Mode = eFileMode::FileModeRead);

	void  Close(FileHandle hFile);

	int32 Read(FileHandle hFile, void* buffer, uint64 BytesToRead, uint64* BytesRead = nullptr);

	int32 Write(FileHandle hFile, const void* pBuffer, uint64 BytesToWrite, uint64* BytesWritten = nullptr);

	enum eSeekOrigin : int32
	{
		Begin	= 0,	// From the start of the file
		Current	= 1,	// From the current file position
		End		= 2		// From the end of the file
	};

	int32 Seek(FileHandle hFile, int64 Offset, const eSeekOrigin Origin, uint64* pPosition = nullptr);

	int32 Tell(FileHandle hFile, uint64& Position);

	int32 Flush(FileHandle hFile);

	int32 GetSize(FileHandle hFile, uint64& outSize);

	int32 SetSize(FileHandle hFile, uint64 newSize);

	// Memory Mapping
	int32 CreateMemoryMap(PATH_C_STR Path, FileMapHandle& hMapping, uint64* MaxSize = nullptr);

	int32 DestroyMemoryMap(FileMapHandle& hMapping);

	bool  MapMemory(FileMapHandle hMapping, void*& outData, uint64 Offset, uint64 Size);

	void  UnmapMemory(FileMapHandle& hMapping);

	// Utilities & Management
	bool  Exists(PATH_C_STR Path);

	int32 DeleteFile(PATH_C_STR Path);

	int32 Move(PATH_C_STR SourcePath, PATH_C_STR TargetPath, bool Overwrite = false);

	int32 Copy(PATH_C_STR SourcePath, PATH_C_STR TargetPath, bool Overwrite = false);

	struct FileInfo
	{
		uint64 Size = 0;              // Total file size in bytes
		uint64 CreationTime = 0;      // Platform-specific file creation time
		uint64 LastAccessTime = 0;    // Platform-specific last read time
		uint64 LastWriteTime = 0;     // Platform-specific last modification time

		bool   IsDirectory = false;   // True if the path points to a directory
		bool   IsHidden = false;      // True if the file is marked as hidden

		// Access Rights / Permissions
		bool   CanRead = false;       // True if the file can be read
		bool   CanWrite = false;      // True if the file can be written to
		bool   CanExecute = false;    // True if the file can be executed
	};

	int32 GetFileInfo(PATH_C_STR Path, FileInfo& Info);

	int32 GetFileInfo(FileHandle handle, FileInfo& Info);

	int32 CreateDirectory(PATH_C_STR Path);

	int32 DeleteDirectory(PATH_C_STR Path, bool Recursive = false);

	bool  DirectoryExists(PATH_C_STR Path);

	// Iterate Directory

	struct DirectoryEntry
	{
		PATH_C_STR Name;          // File or directory name only (e.g., L"Config.ini")
		uint64     Size;          // Size of the file in bytes (0 if it's a directory)
		bool       IsDirectory;   // True if this entry is a subdirectory
	};

	// Function pointer callback for directory iteration (return false to stop iteration)
	using DirectoryIterCallback = bool(*)(const DirectoryEntry& Entry, void* pUserData);

	// Iterates through a directory up to a maximum depth level.
	int32 IterateDirectory(PATH_C_STR Path, DirectoryIterCallback pCallback, void* pUserData = nullptr);

};