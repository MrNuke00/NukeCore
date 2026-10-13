# NukeCore\tests\CMakeLists.txt

add_executable(NukeCoreTests
	"PlatformFile/main.cpp"
)

target_link_libraries(NukeCoreTests
PRIVATE
	NukeCore
)



