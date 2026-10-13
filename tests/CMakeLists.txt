# NukeCore\tests\CMakeLists.txt

enable_testing()

add_executable(PlatformFileTests "PlatformFile/main.cpp")
target_link_libraries(PlatformFileTests PRIVATE NukeCore)
add_test(NAME PlatformFileTests COMMAND PlatformFileTests)


add_executable(PlatformMemoryTests "PlatformMemory/main.cpp")
target_link_libraries(PlatformMemoryTests PRIVATE NukeCore)
add_test(NAME PlatformMemoryTests COMMAND PlatformMemoryTests)
