cmake_minimum_required(VERSION 3.25)

project(libgfvl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE LIBGFVL_SOURCES
	"core/*.cpp"
	"include/*.hpp"
)

target_include_directories(libgfvl PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_include_directories(libgfvl PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)