#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

// Remove-Item build -Recurse -Force; cmake -B build -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 -G Ninja
// cmake --build build --verbose; build/main.exe

const bool GFVL_DEBUG_MODE = true;
void error() {
  std::cout << "Error, Exiting program..." <<  "\n";
}

int main() {
  SDL_Init(SDL_INIT_VIDEO); // initialize video drivers

  SDL_Window *window = SDL_CreateWindow("goofyVLib Example", 800, 600, SDL_WINDOW_VULKAN); // initialize a window with VULKAN flags

  VkApplicationInfo appInfo{ // structure specifying application information
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, // honestly dont know why it exists its just the type of the struct
    .pNext = NULL, // can be a pointer to a structure extending this structure
    .pApplicationName = "goofyVLib Example", // name
    .applicationVersion = 1, // version of application
    .pEngineName = "goofyVLib", // engine name
    .engineVersion = 1, // engine version
    .apiVersion = VK_API_VERSION_1_4 // version of vulkan
  };

  // detect instance extensions
  uint32_t instanceExtensionCount = 0;
  const char *const *instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);

  if (GFVL_DEBUG_MODE) { // optionally print the info
    if (instanceExtensions == NULL) error();
    std::cout << "[GFVL] Detected extensions :\n";

    for (uint32_t i = 0; i < instanceExtensionCount; i++) {
      std::cout << "  " << instanceExtensions[i] << '\n';
    }
  }

  // specify parameters of our instance first
  VkInstanceCreateInfo instanceCreationInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // type of struct
    .pNext = NULL, // optionally extends struct
    .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR, // JIC
    .pApplicationInfo = &appInfo,
    //.enabledLayerCount;
    //.ppEnabledLayerNames;
    .enabledExtensionCount = instanceExtensionCount,
    .ppEnabledExtensionNames = instanceExtensions
  };
  
  SDL_Delay(3000);

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
