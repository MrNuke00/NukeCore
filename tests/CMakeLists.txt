# NukeCore\tests\CMakeLists.txt

enable_testing()

add_executable(PlatformFileTests "PlatformFile/main.cpp")
target_link_libraries(PlatformFileTests PRIVATE NukeCore)
add_test(NAME PlatformFileTests COMMAND PlatformFileTests)

add_executable(PlatformMemoryTests "PlatformMemory/main.cpp")
target_link_libraries(PlatformMemoryTests PRIVATE NukeCore)
add_test(NAME PlatformMemoryTests COMMAND PlatformMemoryTests)

add_executable(PlatformTimeTests "PlatformTime/main.cpp")
target_link_libraries(PlatformTimeTests PRIVATE NukeCore)
add_test(NAME PlatformTimeTests COMMAND PlatformTimeTests)

add_executable(PlatformThreadTests "PlatformThread/main.cpp")
target_link_libraries(PlatformThreadTests PRIVATE NukeCore)
add_test(NAME PlatformThreadTests COMMAND PlatformThreadTests)

add_executable(PlatformNetworkTests "PlatformNetwork/main.cpp")
target_link_libraries(PlatformNetworkTests PRIVATE NukeCore)
add_test(NAME PlatformNetworkTests COMMAND PlatformNetworkTests)


