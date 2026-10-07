# NukeCore\tests\CMakeLists.txt

add_executable(NukeCoreTests
	main.cpp
)

target_link_libraries(NukeCoreTests
PRIVATE
	NukeCore
)



