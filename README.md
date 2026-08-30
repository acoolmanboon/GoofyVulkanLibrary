# Goofy Vulkan Library
I'll probably run out of motivation to finish this.

GoofyVulkanLibrary is a library designed to wrap Vulkan, it's just for quick and dirty applications using Vulkan
To compile this, use CMake. You will also have to compile the shaders yourself though

This library uses the Vulkan Memory Allocator (VMA) to handle memory allocation.

# Compilation
This uses CMake. It is compiled as a regular CMake project, however there are some options for building GFVL.

- **COMPILE_GFVL_LIBRARY** ON by default. This option compiles the GFVL library.
- **COMPILE_ALL_EXAMPLES** OFF by default. This will compile ALL examples in the example folder.

There are more options for building specific examples, but read the README in each example folder.