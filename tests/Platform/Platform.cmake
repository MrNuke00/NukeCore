# NukeCore\tests\Platform\Platform.cmake




add_executable(PlatformFileTests "${CMAKE_CURRENT_LIST_DIR}/PlatformFile/main.cpp")
target_link_libraries(PlatformFileTests PRIVATE NukeCore)
add_test(NAME PlatformFileTests COMMAND PlatformFileTests)

add_executable(PlatformMemoryTests "${CMAKE_CURRENT_LIST_DIR}/PlatformMemory/main.cpp")
target_link_libraries(PlatformMemoryTests PRIVATE NukeCore)
add_test(NAME PlatformMemoryTests COMMAND PlatformMemoryTests)

add_executable(PlatformTimeTests "${CMAKE_CURRENT_LIST_DIR}/PlatformTime/main.cpp")
target_link_libraries(PlatformTimeTests PRIVATE NukeCore)
add_test(NAME PlatformTimeTests COMMAND PlatformTimeTests)

add_executable(PlatformThreadTests "${CMAKE_CURRENT_LIST_DIR}/PlatformThread/main.cpp")
target_link_libraries(PlatformThreadTests PRIVATE NukeCore)
add_test(NAME PlatformThreadTests COMMAND PlatformThreadTests)

add_executable(PlatformNetworkTests "${CMAKE_CURRENT_LIST_DIR}/PlatformNetwork/main.cpp")
target_link_libraries(PlatformNetworkTests PRIVATE NukeCore)
add_test(NAME PlatformNetworkTests COMMAND PlatformNetworkTests)

add_executable(PlatformAudioTests "${CMAKE_CURRENT_LIST_DIR}/PlatformAudio/main.cpp")
target_link_libraries(PlatformAudioTests PRIVATE NukeCore)
add_test(NAME PlatformAudioTests COMMAND PlatformAudioTests)

add_executable(PlatformSurfaceTests "${CMAKE_CURRENT_LIST_DIR}/PlatformSurface/main.cpp")
target_link_libraries(PlatformSurfaceTests PRIVATE NukeCore)
add_test(NAME PlatformSurfaceTests COMMAND PlatformSurfaceTests)

add_executable(PlatformInputTests "${CMAKE_CURRENT_LIST_DIR}/PlatformInput/main.cpp")
target_link_libraries(PlatformInputTests PRIVATE NukeCore)
add_test(NAME PlatformInputTests COMMAND PlatformInputTests)
