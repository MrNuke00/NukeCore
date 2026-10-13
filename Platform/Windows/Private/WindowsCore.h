#pragma once
#include <Platform/PlatformCore.h>

// Prevent Windows SDK from defining 'byte' to avoid redefinition conflicts with C++17 std::byte
#ifndef _BYTE_DEFINED_
#define _BYTE_DEFINED_
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif

#include <windows.h>
#include <shellapi.h>

// Undefine Windows macro pollution that breaks engine function names
#undef DeleteFile
#undef CreateDirectory
#undef MoveFile
#undef CopyFile

