# Goofy Vulkan Library
I'll probably run out of motivation to finish this.

GoofyVulkanLibrary is a library designed to abstract complicated Vulkan boilerplate into a small C++ library, bridging the gap between just writing straight Vulkan and getting on a game engine, or for developers who prefer to have a bare-bones environment.
Currently, the behaviour is quite fixed, but it is planned for the user to have more granular control when needed.
To compile this, use CMake. You will also have to compile the shaders yourself though

This library uses the Vulkan Memory Allocator (VMA) to handle memory allocation.

# Compilation
This uses CMake. It is compiled as a regular CMake project, however there are some options for building GFVL.

- **COMPILE_GFVL_LIBRARY** ON by default. This option compiles the GFVL library.
- **COMPILE_ALL_EXAMPLES** OFF by default. This will compile ALL examples in the example folder.

There are more options for building specific examples, but read the README in each example folder.