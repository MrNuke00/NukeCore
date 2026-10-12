#pragma once
#include <Platform/PlatformCore.h>

// PlatformFile
namespace Platform::File {

	enum eResult : int32
	{
		PLATFORM_RESULT(PLATFORM_FILE_RESULT_BIT),

	};

	DEFINE_HANDLE(File);	// FileHandle
	DEFINE_HANDLE(FileMap);	// FileMapHandle

	// File

	static int32 Open(PATH_C_STR Path, FileHandle& hFile);

	static void  Close(FileHandle hFile);

	static int32 Read(FileHandle hFile, void* buffer, uint64 BytesToRead, uint64* BytesRead = nullptr);

	static int32 Write(FileHandle hFile, const void* pBuffer, uint64 BytesToWrite, uint64* BytesWritten = nullptr);

	static int32 Seek(FileHandle hFile, int64 Offset, int32 Origin, uint64* pPosition = nullptr);

	static int32 Tell(FileHandle hFile, uint64& Position);

	static int32 Flush(FileHandle hFile);

	static int32 GetSize(FileHandle hFile, uint64& outSize);

	static int32 SetSize(FileHandle hFile, uint64 newSize);

	//

	static bool MapToMemory(PATH_C_STR Path, FileMapHandle& hMapping, void* pData, uint64& FileSize);

	static void UnmapMemory(FileMapHandle& hMapping);

	//

	static bool Exists(PATH_C_STR Path);

	static int32 DeleteFile(PATH_C_STR Path);

	static int32 Move(PATH_C_STR SourcePath, PATH_C_STR TargetPath, bool Overwrite = false);

	static int32 Copy(PATH_C_STR SourcePath, PATH_C_STR TargetPath, bool Overwrite = false);

	struct FileInfo
	{

	};

	static int32 GetFileInfo(PATH_C_STR Path, FileInfo& Info);

	static int32 GetFileInfo(FileHandle handle, FileInfo& Info);

	static int32 CreateDirectory(PATH_C_STR Path);

	static int32 DeleteDirectory(PATH_C_STR Path, bool Recursive = false);

	static bool  DirectoryExists(PATH_C_STR Path);

	// Function pointer callback for directory iteration (return false to stop iteration)
	using DirectoryIterCallback = bool(*)(const void* Entry, void* pUserData);

	static int32 IterateDirectory(PATH_C_STR Path, void* Callback, void* pUserData = nullptr);

};



