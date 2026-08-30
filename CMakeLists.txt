cmake_minimum_required(VERSION 3.25)

project(goofyvulkanlibrary LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

add_subdirectory("src-gfvl/")
add_subdirectory("examples/moon_example")