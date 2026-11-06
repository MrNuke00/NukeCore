


add_executable(ArrayTest "${CMAKE_CURRENT_LIST_DIR}/Array/main.cpp")
target_link_libraries(ArrayTest PUBLIC NukeCore)
add_test(NAME ArrayTest COMMAND ArrayTest)

add_executable(AssertTest "${CMAKE_CURRENT_LIST_DIR}/Assert/main.cpp")
target_link_libraries(AssertTest PUBLIC NukeCore)
add_test(NAME AssertTest COMMAND AssertTest)

add_executable(HashTableTest "${CMAKE_CURRENT_LIST_DIR}/HashTable/main.cpp")
target_link_libraries(HashTableTest PUBLIC NukeCore)
add_test(NAME HashTableTest COMMAND HashTableTest)