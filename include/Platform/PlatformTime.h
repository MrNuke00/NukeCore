#pragma once
#include <Platform/PlatformCore.h>

// PlatformTime
namespace Platform::Time
{
	// Returns the hardware counter frequency (ticks per second)
	uint64 GetFrequency();

	// Returns the current raw hardware counter value (monotonic clock)
	uint64 GetCycles();

	// time(second) = cycles / frequency

	struct DateTime
	{
		uint16 Year;
		uint16 Month;       // 1 - 12 (Normalized for cross-platform safety)
		uint16 DayOfWeek;   // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
		uint16 Day;         // 1 - 31
		uint16 Hour;        // 0 - 23
		uint16 Minute;      // 0 - 59
		uint16 Second;      // 0 - 59
		uint16 Millisecond; // 0 - 999
	};

	// Queries the OS for current local calendar time (handles timezone/DST)
	void GetLocalTime(DateTime& outDateTime);

	// Queries the OS for current Coordinated Universal Time (UTC)
	void GetUtcTime(DateTime& outDateTime);
};