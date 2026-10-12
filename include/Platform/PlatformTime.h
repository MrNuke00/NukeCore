#pragma once
#include <Platform/PlatformCore.h>

/*

/// @brief Represents a broken-down calendar date and clock time snapshot.
struct DateTime
{
	uint16 Year;
	uint16 Month;        // 1 - 12
	uint16 DayOfWeek;    // 0 = Sunday, 1 = Monday, ..., 6 = Saturday
	uint16 Day;          // 1 - 31
	uint16 Hour;         // 0 - 23
	uint16 Minute;       // 0 - 59
	uint16 Second;       // 0 - 59
	uint16 Millisecond;  // 0 - 999
};

/// @brief Specifies unit precision formats for timing conversions.
enum class eTimeUnit : uint8
{
	Microseconds,
	Milliseconds,
	Seconds,
	Minutes,
	Hours
};

struct PLATFORM_TIME_API PlatformTime
{
	PlatformTime() = delete;

public:

	static void Init();

	static void Fini();

public:
	/// @brief Queries the hardware performance counter frequency (ticks per second).
	/// @details Logic: Uses Win32 QueryPerformanceFrequency to query the high-resolution timer frequency.
	///          This value is fixed at system boot and cached in a thread-safe static variable.
	/// @return The total hardware counter ticks per second.
	static uint64 GetFrequency();

	/// @brief Gets the raw current CPU/Hardware cycle counter tick value.
	/// @details Logic: Calls QueryPerformanceCounter to read the current hardware timer tick count.
	///          Used as a raw baseline for precise scope profiling and calculating frame deltas.
	/// @return Current raw tick count.
	static uint64 GetCycles();

	/// @brief Calculates elapsed absolute system time in seconds since system startup.
	/// @details Logic: Converts the current raw ticks from GetCycles() to floating-point seconds by
	///          dividing by the cached clock frequency (GetFrequency()).
	/// @return Absolute high-precision time in seconds (double).
	static float64 GetSeconds();

	/// @brief Calculates elapsed absolute system time in milliseconds since system startup.
	/// @details Logic: Multiplies current cycles by 1000.0 before dividing by frequency to retain
	///          floating-point precision without premature truncation.
	/// @return Absolute high-precision time in milliseconds (double).
	static float64 GetMilliseconds();

	/// @brief Converts raw CPU tick count/cycles into floating-point milliseconds.
	/// @param cycles Total raw tick count difference (EndCycles - StartCycles).
	/// @details Logic: Multiplies cycles by 1000.0 then divides by clock frequency for microsecond precision.
	/// @return Duration converted into milliseconds (double).
	static float64 CyclesToMilliseconds(uint64 cycles);

	/// @brief Retrieves the local system calendar date and time.
	/// @param outDateTime Output reference to populate with broken-down date and time attributes.
	/// @details Logic: Queries Win32 GetLocalTime (which accounts for timezone and daylight saving time),
	///          and maps Win32 SYSTEMTIME fields directly to the engine's custom DateTime struct.
	static void GetLocalTime(DateTime& outDateTime);

	/// @brief Retrieves the UTC (Coordinated Universal Time) calendar date and time.
	/// @param outDateTime Output reference to populate with UTC date and time attributes.
	/// @details Logic: Queries Win32 GetSystemTime and maps fields to the engine's custom DateTime struct.
	static void GetUtcTime(DateTime& outDateTime);

};

*/

