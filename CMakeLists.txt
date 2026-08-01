cmake_minimum_required(VERSION 3.25)

project(GraphicsLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

find_package(SDL3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

get_target_property(
    SDL_INCLUDES
    SDL3::SDL3
    INTERFACE_INCLUDE_DIRECTORIES
)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp"
)
message(STATUS "SDL includes = ${SDL_INCLUDES}")
add_executable(main ${SOURCES})
target_include_directories(main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/GFVL/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/GFVL/include"
)

target_compile_options(main PRIVATE
    -fno-omit-frame-pointer
    -g
)
target_link_options(main PRIVATE 
    -g
)
target_link_libraries(main PRIVATE
    SDL3::SDL3
    Vulkan::Vulkan
)
get_target_property(INCS main INCLUDE_DIRECTORIES)
message(STATUS "main includes=${INCS}")