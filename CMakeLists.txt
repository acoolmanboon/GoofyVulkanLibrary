cmake_minimum_required(VERSION 3.25)

project(goofyvulkanlibrary LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

option(COMPILE_GFVL_LIBRARY "Compile the GFVL library." ON)
if (COMPILE_GFVL_LIBRARY)
    add_subdirectory("src-gfvl/")
endif()

option(COMPILE_ALL_EXAMPLES "Compile ALL examples." OFF)

option(COMPILE_MOON_EXAMPLE "Compile the moon_example folder." OFF)
if (COMPILE_ALL_EXAMPLES OR COMPILE_MOON_EXAMPLE)
    add_subdirectory("examples/moon_example")
endif()