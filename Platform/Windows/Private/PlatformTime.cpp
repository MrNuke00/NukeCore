#include <Platform/PlatformTime.h>
#include <WindowsCore.h>

namespace Platform::Time
{
	uint64 GetFrequency()
	{
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		return static_cast<uint64>(frequency.QuadPart);
	};

	uint64 GetCycles()
	{
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return static_cast<uint64>(counter.QuadPart);
	};

	// Internal helper to translate Win32 SYSTEMTIME into engine-agnostic DateTime
	static void ConvertSystemTimeToDateTime(const SYSTEMTIME& sysTime, DateTime& outDateTime)
	{
		outDateTime.Year		= sysTime.wYear;
		outDateTime.Month		= sysTime.wMonth;       // 1 - 12
		outDateTime.DayOfWeek	= sysTime.wDayOfWeek;   // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
		outDateTime.Day			= sysTime.wDay;         // 1 - 31
		outDateTime.Hour		= sysTime.wHour;        // 0 - 23
		outDateTime.Minute		= sysTime.wMinute;      // 0 - 59
		outDateTime.Second		= sysTime.wSecond;      // 0 - 59
		outDateTime.Millisecond	= sysTime.wMilliseconds;// 0 - 999
	};

	void GetLocalTime(DateTime& outDateTime)
	{
		SYSTEMTIME sysTime;
		::GetLocalTime(&sysTime);
		ConvertSystemTimeToDateTime(sysTime, outDateTime);
	};

	void GetUtcTime(DateTime& outDateTime)
	{
		SYSTEMTIME sysTime;
		::GetSystemTime(&sysTime);
		ConvertSystemTimeToDateTime(sysTime, outDateTime);
	};
};