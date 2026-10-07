# NukeCore\CMakeLists.txt

cmake_minimum_required (VERSION 3.20)


# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("NukeCore"
	VERSION 0.1.0
	DESCRIPTION "Core runtime for the Nuke Engine"
	LANGUAGES CXX
)

include("cmake/Helpers.cmake")

# ---- Options ----
option(NUKECORE_BUILD_TESTS		"Build NukeCore tests"		ON)

# ---- Library ----
NUKECORE_GET_CPP_SOURCES(SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/Platform
)

add_library(NukeCore STATIC ${SOURCES})

add_library(Nuke::Core ALIAS NukeCore)

target_compile_features(NukeCore PUBLIC cxx_std_20)

# --- Detect platform (WIN32 → APPLE → UNIX, order matters!) ---
if(WIN32)
    set(PLATFORM_NAME "Windows")
    set(PLATFORM_DEFINE PLATFORM_WINDOWS=1)
elseif(APPLE)
    set(PLATFORM_NAME "Apple")
    set(PLATFORM_DEFINE PLATFORM_APPLE=1)
elseif(UNIX)
    set(PLATFORM_NAME "Linux")
    set(PLATFORM_DEFINE PLATFORM_LINUX=1)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

target_compile_definitions(NukeCore
    PUBLIC
        PLATFORM_NAME="${PLATFORM_NAME}"
        ${PLATFORM_DEFINE}
)

target_include_directories(NukeCore
	PUBLIC
	# Include directories for consumers of the library
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
	# Platform-specific include directories
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Platform/${PLATFORM_NAME}/Public>
        $<INSTALL_INTERFACE:Platform/${PLATFORM_NAME}/Public>
	PRIVATE
	# Include directories for building the library itself
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/Platform/${PLATFORM_NAME}/Private
)

# Optional: warnings
if (MSVC)
	target_compile_options(NukeCore PRIVATE /W4)
else()
	target_compile_options(NukeCore PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---- Install / Export ----
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS NukeCore
	EXPORT NukeCoreTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/NukeCore
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT NukeCoreTargets
	FILE NukeCoreTargets.cmake
	NAMESPACE Nuke::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NukeCore
)

configure_package_config_file(
	cmake/NukeCoreConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/NukeCoreConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NukeCore
)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/NukeCoreConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/NukeCoreConfig.cmake
	${CMAKE_CURRENT_BINARY_DIR}/NukeCoreConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NukeCore
)

# ---- Subprojects ----
if (NUKECORE_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
