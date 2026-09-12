cmake_minimum_required(VERSION 3.25)

project(triangle_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# sources
file(GLOB_RECURSE EXAMPLE_SOURCES
	"*.cpp"
	"*.hpp"
)

# executable declaration
add_executable(triangle_example ${EXAMPLE_SOURCES})
target_compile_options(triangle_example PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(triangle_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

# link the executable
target_link_libraries(triangle_example PRIVATE
	libgfvl
	$<$<BOOL:$<TARGET_EXISTS:sanitizerflags>>:sanitizerflags>
)
