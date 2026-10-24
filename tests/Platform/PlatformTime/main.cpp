#include <iostream>
#include <Platform/PlatformTime.h>

int main()
{
    std::wcout << L"=== Starting Platform::Time System Comprehensive Test ===" << std::endl;

    // ==========================================
    // 1. Test High-Resolution Performance Timing
    // ==========================================
    std::wcout << L"\n[1] Testing High-Resolution Hardware Timers (GetFrequency, GetCycles)..." << std::endl;

    uint64 freq = Platform::Time::GetFrequency();
    std::wcout << L"  -> Hardware Frequency (ticks per second): " << freq << std::endl;

    uint64 startCycles = Platform::Time::GetCycles();
    std::wcout << L"  -> Start Counter Cycles: " << startCycles << std::endl;

    // Simulate a small workload (busy wait loop)
    volatile int dummy = 0;
    for (int i = 0; i < 5000000; ++i)
    {
        dummy += i;
    }

    uint64 endCycles = Platform::Time::GetCycles();
    std::wcout << L"  -> End Counter Cycles:   " << endCycles << std::endl;

    uint64 deltaCycles = endCycles - startCycles;
    double elapsedMilliseconds = (static_cast<double>(deltaCycles) * 1000.0) / static_cast<double>(freq);
    double elapsedSeconds = static_cast<double>(deltaCycles) / static_cast<double>(freq);

    std::wcout << L"  -> Delta Cycles: " << deltaCycles << std::endl;
    std::wcout << L"  -> Elapsed Time: " << elapsedMilliseconds << L" ms (" << elapsedSeconds << L" seconds)" << std::endl;

    // ==========================================
    // 2. Test Local Wall-Clock Calendar Time
    // ==========================================
    std::wcout << L"\n[2] Testing Local Calendar Time (GetLocalTime)..." << std::endl;

    Platform::Time::DateTime localTime;
    Platform::Time::GetLocalTime(localTime);

    std::wcout << L"  -> Local Date/Time: "
        << localTime.Year << L"-"
        << (localTime.Month < 10 ? L"0" : L"") << localTime.Month << L"-"
        << (localTime.Day < 10 ? L"0" : L"") << localTime.Day << L" "
        << (localTime.Hour < 10 ? L"0" : L"") << localTime.Hour << L":"
        << (localTime.Minute < 10 ? L"0" : L"") << localTime.Minute << L":"
        << (localTime.Second < 10 ? L"0" : L"") << localTime.Second << L"."
        << localTime.Millisecond << std::endl;
    std::wcout << L"  -> Day of Week (0 = Sunday ... 6 = Saturday): " << localTime.DayOfWeek << std::endl;

    // ==========================================
    // 3. Test UTC Calendar Time
    // ==========================================
    std::wcout << L"\n[3] Testing UTC Calendar Time (GetUtcTime)..." << std::endl;

    Platform::Time::DateTime utcTime;
    Platform::Time::GetUtcTime(utcTime);

    std::wcout << L"  -> UTC Date/Time:   "
        << utcTime.Year << L"-"
        << (utcTime.Month < 10 ? L"0" : L"") << utcTime.Month << L"-"
        << (utcTime.Day < 10 ? L"0" : L"") << utcTime.Day << L" "
        << (utcTime.Hour < 10 ? L"0" : L"") << utcTime.Hour << L":"
        << (utcTime.Minute < 10 ? L"0" : L"") << utcTime.Minute << L":"
        << (utcTime.Second < 10 ? L"0" : L"") << utcTime.Second << L"."
        << utcTime.Millisecond << std::endl;

    std::wcout << L"\n=== All Platform::Time Tests Completed Successfully! ===" << std::endl;
    return 0;
}