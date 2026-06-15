#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL;

namespace GFVL {
  const char *VkResultToString(VkResult result) {
    switch (result) {
    case VK_SUCCESS:
      return "VK_SUCCESS";
    case VK_NOT_READY:
      return "VK_NOT_READY";
    case VK_TIMEOUT:
      return "VK_TIMEOUT";
    case VK_EVENT_SET:
      return "VK_EVENT_SET";
    case VK_EVENT_RESET:
      return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
      return "VK_INCOMPLETE";

    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
      return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
      return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
      return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
      return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return "VK_ERROR_FORMAT_NOT_SUPPORTED";

    default:
      return "UNKNOWN_VK_RESULT";
    }
  }
  void PrintVkResult(VkResult result) {
    std::cout << VkResultToString(result) << " (" << static_cast<int>(result) << ")\n";
  }
  VkResult CheckVkResult(VkResult result) {
    if (result < 0) {
      std::cout << "[GFVL] Error! : " << VkResultToString(result) << " (" << static_cast<int>(result) << ")\n";
      throw std::runtime_error("[GFVL] Error detected. read the above message");
    }
    return result;
  }

  VkInstance InitializeVkInstance(VkApplicationInfo *appInfo) {
    uint32_t instanceExtensionCount = 0;
    const char *const *instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);

    if (DEBUG_MODE) { // optionally print the info
      std::cout << "[GFVL] GFVL_InitializeVkInstance \n";
      if (instanceExtensions == NULL)
        std::cout << "[GFVL] No instance extensions supported.. What?" << "\n";
      std::cout << "[GFVL] Detected instance extensions :\n";
      for (uint32_t i = 0; i < instanceExtensionCount; i++)
        std::cout << "  " << instanceExtensions[i] << '\n';
    }

    VkInstanceCreateInfo instanceCreationInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pApplicationInfo = appInfo,
        .enabledExtensionCount = instanceExtensionCount,
        .ppEnabledExtensionNames = instanceExtensions};

    VkInstance instance;
    GFVL::CheckVkResult(vkCreateInstance(
        &instanceCreationInfo,
        NULL,
        &instance));

    return instance;
  }
}
