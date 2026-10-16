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

add_executable(PlatformAudioTests "PlatformAudio/main.cpp")
target_link_libraries(PlatformAudioTests PRIVATE NukeCore)
add_test(NAME PlatformAudioTests COMMAND PlatformAudioTests)


add_executable(PlatformSurfaceTests "PlatformSurface/main.cpp")
target_link_libraries(PlatformSurfaceTests PRIVATE NukeCore)
add_test(NAME PlatformSurfaceTests COMMAND PlatformSurfaceTests)



add_executable(PlatformInputTests "PlatformInput/main.cpp")
target_link_libraries(PlatformInputTests PRIVATE NukeCore)
add_test(NAME PlatformInputTests COMMAND PlatformInputTests)

