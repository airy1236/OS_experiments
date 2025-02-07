cmake_minimum_required(VERSION 3.2)

project(OS_Test)

set(CMAKE_CXX_STANDARD 17)

include_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/include
)

link_directories(
	SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/lib
)

add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/include/imgui)

add_executable(test_1 tests/test_1/main.cpp tests/glad.c)
add_executable(test_2 tests/test_2/main.cpp tests/glad.c)
add_executable(test_3 tests/test_3/main.cpp tests/glad.c)

foreach(exe 
	test_1
	test_2
	test_3)
target_link_libraries(${exe} glfw3.lib img)
endforeach()