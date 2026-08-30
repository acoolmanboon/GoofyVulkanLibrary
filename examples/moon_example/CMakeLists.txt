cmake_minimum_required(VERSION 3.25)

project(moon_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# sources
file(GLOB_RECURSE EXAMPLE_SOURCES
	"*.cpp"
	"*.hpp"
)

# executable declaration
add_executable(exampleexecutable ${EXAMPLE_SOURCES})

# link the executable
target_link_libraries(exampleexecutable PRIVATE
	libgfvl
	$<$<BOOL:$<TARGET_EXISTS:sanitizerflags>>:sanitizerflags>
)
