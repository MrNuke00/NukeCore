#include <Platform/PlatformFile.h>
#include <WindowsCore.h>
#include <cstdio>

// Hidden internal tracking structure for Windows memory mapping
struct FWindowsFileMap
{
	HANDLE hFileMapping = nullptr;
	void* pMappedData = nullptr;
	uint64 FileSize = 0;
};

// PlatformFile
namespace Platform::File {

	int32 Open(PATH_C_STR Path, FileHandle& hFile, const uint32 Mode)
	{
		hFile.ptr = nullptr;

		DWORD desiredAccess = 0;
		DWORD creationDisposition = OPEN_EXISTING;
		DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

		if (Mode & eFileMode::FileModeRead)
		{
			desiredAccess |= GENERIC_READ;
		}
		if (Mode & eFileMode::FileModeWrite)
		{
			desiredAccess |= GENERIC_WRITE;
		}
		if (Mode & eFileMode::FileModeAppend)
		{
			desiredAccess |= FILE_APPEND_DATA;
		}

		if (Mode & eFileMode::FileModeCreateNew)
		{
			creationDisposition = CREATE_ALWAYS;
		}
		else if ((Mode & eFileMode::FileModeWrite) || (Mode & eFileMode::FileModeAppend))
		{
			creationDisposition = OPEN_ALWAYS;
		}

		HANDLE handle = CreateFileW(
			Path,
			desiredAccess,
			shareMode,
			nullptr,
			creationDisposition,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);

		if (handle == INVALID_HANDLE_VALUE)
		{
			DWORD err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
				return eResult::FileNotFound;
			if (err == ERROR_ACCESS_DENIED)
				return eResult::AccessDenied;
			if (err == ERROR_SHARING_VIOLATION)
				return eResult::SharingViolation;
			if (err == ERROR_ALREADY_EXISTS || err == ERROR_FILE_EXISTS)
				return eResult::FileAlreadyExists;
			return eResult::Error;
		}

		hFile.ptr = handle;
		return eResult::OK;
	}

	void Close(FileHandle hFile)
	{
		if (hFile)
		{
			CloseHandle(static_cast<HANDLE>(hFile.ptr));
			hFile.ptr = nullptr;
		}
	}

	int32 Read(FileHandle hFile, void* buffer, uint64 BytesToRead, uint64* BytesRead)
	{
		if (!hFile || !buffer)
			return eResult::UnvalidHandle;

		if (BytesToRead > static_cast<uint64>(MAXDWORD))
			return eResult::InvalidParameter;

		DWORD bytesReadResult = 0;
		BOOL success = ReadFile(
			static_cast<HANDLE>(hFile.ptr),
			buffer,
			static_cast<DWORD>(BytesToRead),
			&bytesReadResult,
			nullptr
		);

		if (!success)
		{
			DWORD err = GetLastError();
			if (err == ERROR_HANDLE_EOF)
			{
				if (BytesRead) *BytesRead = 0;
				return eResult::EndOfFile;
			}
			return eResult::Error;
		}

		if (BytesRead)
		{
			*BytesRead = static_cast<uint64>(bytesReadResult);
		}
		return eResult::OK;
	}

	int32 Write(FileHandle hFile, const void* pBuffer, uint64 BytesToWrite, uint64* BytesWritten)
	{
		if (!hFile || !pBuffer)
			return eResult::UnvalidHandle;

		if (BytesToWrite > static_cast<uint64>(MAXDWORD))
			return eResult::InvalidParameter;

		DWORD bytesWrittenResult = 0;
		BOOL success = WriteFile(
			static_cast<HANDLE>(hFile.ptr),
			pBuffer,
			static_cast<DWORD>(BytesToWrite),
			&bytesWrittenResult,
			nullptr
		);

		if (!success)
		{
			DWORD err = GetLastError();
			if (err == ERROR_DISK_FULL)
				return eResult::DiskFull;
			return eResult::Error;
		}

		if (BytesWritten)
		{
			*BytesWritten = static_cast<uint64>(bytesWrittenResult);
		}
		return eResult::OK;
	}

	int32 Seek(FileHandle hFile, int64 Offset, const eSeekOrigin Origin, uint64* pPosition)
	{
		if (!hFile)
			return eResult::UnvalidHandle;

		DWORD moveMethod = FILE_BEGIN;
		if (Origin == eSeekOrigin::Current)
			moveMethod = FILE_CURRENT;
		else if (Origin == eSeekOrigin::End)
			moveMethod = FILE_END;

		LARGE_INTEGER liDistance;
		liDistance.QuadPart = Offset;
		LARGE_INTEGER liNewPos;

		BOOL success = SetFilePointerEx(
			static_cast<HANDLE>(hFile.ptr),
			liDistance,
			&liNewPos,
			moveMethod
		);

		if (!success)
			return eResult::Error;

		if (pPosition)
		{
			*pPosition = static_cast<uint64>(liNewPos.QuadPart);
		}
		return eResult::OK;
	}

	int32 Tell(FileHandle hFile, uint64& Position)
	{
		return Seek(hFile, 0, eSeekOrigin::Current, &Position);
	}

	int32 Flush(FileHandle hFile)
	{
		if (!hFile)
			return eResult::UnvalidHandle;

		if (!FlushFileBuffers(static_cast<HANDLE>(hFile.ptr)))
			return eResult::Error;

		return eResult::OK;
	}

	int32 GetSize(FileHandle hFile, uint64& outSize)
	{
		if (!hFile)
			return eResult::UnvalidHandle;

		LARGE_INTEGER size;
		if (!GetFileSizeEx(static_cast<HANDLE>(hFile.ptr), &size))
			return eResult::Error;

		outSize = static_cast<uint64>(size.QuadPart);
		return eResult::OK;
	}

	int32 SetSize(FileHandle hFile, uint64 newSize)
	{
		if (!hFile)
			return eResult::UnvalidHandle;

		uint64 currentPos = 0;
		Tell(hFile, currentPos);

		if (Seek(hFile, static_cast<int64>(newSize), eSeekOrigin::Begin) != eResult::OK)
			return eResult::Error;

		BOOL success = SetEndOfFile(static_cast<HANDLE>(hFile.ptr));
		if (!success)
		{
			Seek(hFile, static_cast<int64>(currentPos), eSeekOrigin::Begin);
			return eResult::Error;
		}

		Seek(hFile, static_cast<int64>(currentPos), eSeekOrigin::Begin);
		return eResult::OK;
	}

	// Memory Mapping & Tracking

	int32 CreateMemoryMap(PATH_C_STR Path, FileMapHandle& hMapping, uint64* MaxSize)
	{
		if (hMapping)
		{
			return eResult::Error;
		}

		HANDLE hFile = CreateFileW(
			Path,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);

		if (hFile == INVALID_HANDLE_VALUE)
			return eResult::FileNotFound;

		LARGE_INTEGER liSize;
		if (!GetFileSizeEx(hFile, &liSize))
		{
			CloseHandle(hFile);
			return eResult::Error;
		}

		uint64 fileSize = static_cast<uint64>(liSize.QuadPart);
		if (MaxSize) *MaxSize = fileSize;

		if (fileSize == 0)
		{
			CloseHandle(hFile);
			return eResult::Error;
		}

		HANDLE hMapFile = CreateFileMappingW(
			hFile,
			nullptr,
			PAGE_READWRITE,
			static_cast<DWORD>(fileSize >> 32),
			static_cast<DWORD>(fileSize & 0xFFFFFFFF),
			nullptr
		);

		CloseHandle(hFile);

		if (!hMapFile)
			return eResult::Error;

		FWindowsFileMap* internalMap = new FWindowsFileMap();
		internalMap->hFileMapping = hMapFile;
		internalMap->pMappedData = nullptr;
		internalMap->FileSize = fileSize;

		hMapping.ptr = internalMap;
		return eResult::OK;
	}

	int32 DestroyMemoryMap(FileMapHandle& hMapping)
	{
		if (!hMapping)
			return eResult::UnvalidHandle;

		FWindowsFileMap* internalMap = static_cast<FWindowsFileMap*>(hMapping.ptr);

		if (internalMap->pMappedData)
		{
			UnmapViewOfFile(internalMap->pMappedData);
		}

		CloseHandle(internalMap->hFileMapping);
		delete internalMap;

		hMapping.ptr = nullptr;
		return eResult::OK;
	}

	bool MapMemory(FileMapHandle hMapping, void*& outData, uint64 Offset, uint64 Size)
	{
		outData = nullptr;
		if (!hMapping)
			return false;

		FWindowsFileMap* internalMap = static_cast<FWindowsFileMap*>(hMapping.ptr);

		if (internalMap->pMappedData != nullptr)
		{
			UnmapViewOfFile(internalMap->pMappedData);
			internalMap->pMappedData = nullptr;
		}

		void* pMapped = MapViewOfFile(
			internalMap->hFileMapping,
			FILE_MAP_READ | FILE_MAP_WRITE,
			static_cast<DWORD>(Offset >> 32),
			static_cast<DWORD>(Offset & 0xFFFFFFFF),
			static_cast<SIZE_T>(Size)
		);

		if (!pMapped)
			return false;

		internalMap->pMappedData = pMapped;
		outData = pMapped;
		return true;
	}

	void UnmapMemory(FileMapHandle& hMapping)
	{
		if (!hMapping)
			return;

		FWindowsFileMap* internalMap = static_cast<FWindowsFileMap*>(hMapping.ptr);
		if (internalMap->pMappedData)
		{
			UnmapViewOfFile(internalMap->pMappedData);
			internalMap->pMappedData = nullptr;
		}
	}

	// File Management & Utilities

	bool Exists(PATH_C_STR Path)
	{
		DWORD attrs = GetFileAttributesW(Path);
		if (attrs == INVALID_FILE_ATTRIBUTES)
			return false;
		return (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
	}

	int32 DeleteFile(PATH_C_STR Path)
	{
		if (!DeleteFileW(Path))
		{
			DWORD err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND)
				return eResult::FileNotFound;
			return eResult::Error;
		}
		return eResult::OK;
	}

	int32 Move(PATH_C_STR SourcePath, PATH_C_STR TargetPath, bool Overwrite)
	{
		DWORD flags = MOVEFILE_COPY_ALLOWED;
		if (Overwrite)
			flags |= MOVEFILE_REPLACE_EXISTING;

		if (!MoveFileExW(SourcePath, TargetPath, flags))
			return eResult::Error;

		return eResult::OK;
	}

	int32 Copy(PATH_C_STR SourcePath, PATH_C_STR TargetPath, bool Overwrite)
	{
		if (!CopyFileW(SourcePath, TargetPath, !Overwrite))
			return eResult::Error;

		return eResult::OK;
	}

	int32 GetFileInfo(PATH_C_STR Path, FileInfo& Info)
	{
		WIN32_FILE_ATTRIBUTE_DATA data;
		if (!GetFileAttributesExW(Path, GetFileExInfoStandard, &data))
		{
			DWORD err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
				return eResult::FileNotFound;
			return eResult::Error;
		}

		Info.Size = (static_cast<uint64>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;

		auto fileTimeToUint64 = [](const FILETIME& ft) -> uint64 {
			return (static_cast<uint64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
			};

		Info.CreationTime = fileTimeToUint64(data.ftCreationTime);
		Info.LastAccessTime = fileTimeToUint64(data.ftLastAccessTime);
		Info.LastWriteTime = fileTimeToUint64(data.ftLastWriteTime);

		Info.IsDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		Info.IsHidden = (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;

		Info.CanRead = true;
		Info.CanWrite = (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0;
		Info.CanExecute = true;

		return eResult::OK;
	}

	int32 GetFileInfo(FileHandle handle, FileInfo& Info)
	{
		if (!handle)
		{
			return eResult::UnvalidHandle;
		}

		BY_HANDLE_FILE_INFORMATION data;
		if (!GetFileInformationByHandle(static_cast<HANDLE>(handle.ptr), &data))
		{
			return eResult::Error;
		}

		Info.Size = (static_cast<uint64>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;

		auto fileTimeToUint64 = [](const FILETIME& ft) -> uint64 {
			return (static_cast<uint64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
			};

		Info.CreationTime = fileTimeToUint64(data.ftCreationTime);
		Info.LastAccessTime = fileTimeToUint64(data.ftLastAccessTime);
		Info.LastWriteTime = fileTimeToUint64(data.ftLastWriteTime);

		Info.IsDirectory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		Info.IsHidden = (data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0;

		Info.CanRead = true;
		Info.CanWrite = (data.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0;
		Info.CanExecute = true;

		return eResult::OK;
	}

	int32 CreateDirectory(PATH_C_STR Path)
	{
		if (DirectoryExists(Path))
			return eResult::OK;

		if (!CreateDirectoryW(Path, nullptr))
		{
			return eResult::Error;
		}
		return eResult::OK;
	}

	int32 DeleteDirectory(PATH_C_STR Path, bool Recursive)
	{
		if (!Recursive)
		{
			if (!RemoveDirectoryW(Path))
				return eResult::Error;
			return eResult::OK;
		}

		SHFILEOPSTRUCTW fileOp = {};
		fileOp.wFunc = FO_DELETE;

		wchar_t pathBuf[MAX_PATH + 2] = {};
		wcscpy_s(pathBuf, Path);
		pathBuf[wcslen(Path) + 1] = L'\0';
		fileOp.pFrom = pathBuf;
		fileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_ALLOWUNDO;

		int result = SHFileOperationW(&fileOp);
		if (result != 0)
			return eResult::Error;

		return eResult::OK;
	}

	bool DirectoryExists(PATH_C_STR Path)
	{
		DWORD attrs = GetFileAttributesW(Path);
		if (attrs == INVALID_FILE_ATTRIBUTES)
			return false;
		return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
	}

	// Iterate Directory

	int32 IterateDirectory(PATH_C_STR Path, DirectoryIterCallback pCallback, void* pUserData)
	{
		if (!pCallback)
			return eResult::InvalidParameter;

		wchar_t searchPath[MAX_PATH];
		swprintf_s(searchPath, L"%s\\*", Path);

		WIN32_FIND_DATAW findData;
		HANDLE hFind = FindFirstFileW(searchPath, &findData);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			DWORD err = GetLastError();
			if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
				return eResult::FileNotFound;
			return eResult::Error;
		}

		do
		{
			if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
			{
				continue;
			}

			DirectoryEntry entry;
			entry.Name = findData.cFileName;
			entry.IsDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			entry.Size = entry.IsDirectory ? 0 : ((static_cast<uint64>(findData.nFileSizeHigh) << 32) | findData.nFileSizeLow);

			bool continueIter = pCallback(entry, pUserData);
			if (!continueIter)
			{
				break;
			}

		} while (FindNextFileW(hFind, &findData) != 0);

		FindClose(hFind);
		return eResult::OK;
	}

};