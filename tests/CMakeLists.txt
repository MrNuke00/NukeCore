# NukeCore\tests\CMakeLists.txt

enable_testing()

option(NUKECORE_BUILD_ALL_TESTS "Build All NukeCore tests" ON)

option(NUKECORE_BUILD_PLATFORM_TESTS "Build Platform NukeCore tests" OFF)
option(NUKECORE_BUILD_CORE_TESTS "Build Core NukeCore tests" ON)

if(NUKECORE_BUILD_PLATFORM_TESTS OR NUKECORE_BUILD_ALL_TESTS)
	include("${CMAKE_CURRENT_LIST_DIR}/Platform/Platform.cmake")
endif()

if(NUKECORE_BUILD_CORE_TESTS OR NUKECORE_BUILD_ALL_TESTS)
	include("${CMAKE_CURRENT_LIST_DIR}/Core/Core.cmake")
endif()





