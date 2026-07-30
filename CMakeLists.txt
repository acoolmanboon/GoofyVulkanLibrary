cmake_minimum_required(VERSION 3.25)

project(GraphicsLib LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp"
)

add_library(volk STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/GFVL/lib/volk.c"
)

target_include_directories(volk PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/GFVL/lib"
)

add_executable(main ${SOURCES})

target_include_directories(main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/GFVL/lib"
)

target_compile_options(main PRIVATE
    -fno-omit-frame-pointer
    -g
)

target_link_options(main PRIVATE
    -g
)

target_link_libraries(main PRIVATE
    volk
    SDL3::SDL3
    Vulkan::Vulkan
)