#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

// Remove-Item build -Recurse -Force; cmake -B build -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 -G Ninja
// cmake --build build --verbose; build/main.exe

const bool GFVL_DEBUG_MODE = true;
enum GFVL_PREFERRED_GPU {
  GFVL_PREFERRED_GPU_INTEGRATED_GRAPHICS,
  GFVL_PREFERRED_GPU_DEDICATED_GRAPHICS,
};

void error() {
  std::cout << "Error, Exiting program..." <<  "\n";
}

#include <iostream>
#include <vulkan/vulkan.h>

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
  if (result != VK_SUCCESS) std::cout << "[GFVL] Error! : " << VkResultToString(result) << " (" << static_cast<int>(result) << ")\n";
  if (GFVL_DEBUG_MODE) std::cout << "[GFVL] Debug : " << VkResultToString(result) << " (" << static_cast<int>(result) << ")\n";

  return result;
}

VkInstance GFVL_InitializeVkInstance(VkApplicationInfo* appInfo) {
  uint32_t instanceExtensionCount = 0;
  const char *const *instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);

  if (GFVL_DEBUG_MODE) { // optionally print the info
    if (instanceExtensions == NULL) std::cout << "[GFVL] No instance extensions supported.. What?" << "\n";
    std::cout << "[GFVL] Detected instance extensions :\n";
    for (uint32_t i = 0; i < instanceExtensionCount; i++) std::cout << "  " << instanceExtensions[i] << '\n';
  }

  VkInstanceCreateInfo instanceCreationInfo = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .pNext = NULL,
    .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
    .pApplicationInfo = appInfo,
    .enabledExtensionCount = instanceExtensionCount,
    .ppEnabledExtensionNames = instanceExtensions
  };

  VkInstance instance;
  CheckVkResult(vkCreateInstance(
    &instanceCreationInfo,
    NULL,
    &instance
  ));

  return instance;
}
VkSurfaceKHR GFVL_InitializeVkSurface(VkInstance instance, SDL_Window *window) {
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)) std::cout << "[GFVL] SDL error : " << SDL_GetError() << '\n';
  return surface;
}
std::vector<VkPhysicalDevice> GFVL_EnumeratePhysicalDevices(VkInstance instance) {
  uint32_t availablePhysicalDevicesCount = 0;

  CheckVkResult(vkEnumeratePhysicalDevices(instance, &availablePhysicalDevicesCount, nullptr));
  std::cout << "[GFVL] Found " << availablePhysicalDevicesCount << " supported devices. \n";

  if (availablePhysicalDevicesCount == 0) std::cout << "[GFVL] No Vulkan-compatible GPUs found.\n";

  std::vector<VkPhysicalDevice> availablePhysicalDevices(availablePhysicalDevicesCount);
  CheckVkResult(vkEnumeratePhysicalDevices(instance, &availablePhysicalDevicesCount, availablePhysicalDevices.data()));

  if (GFVL_DEBUG_MODE) {
    std::cout << "[GFVL] Detected devices:\n";

    for (uint32_t i = 0; i < availablePhysicalDevicesCount; i++) {
      VkPhysicalDeviceProperties props{};
      vkGetPhysicalDeviceProperties(availablePhysicalDevices[i], &props);

      std::cout << "  Device " << i << '\n';
      std::cout << "    Name: " << props.deviceName << '\n';
      std::cout << "    API Version: " << VK_VERSION_MAJOR(props.apiVersion) << '.' << VK_VERSION_MINOR(props.apiVersion) << '.' << VK_VERSION_PATCH(props.apiVersion) << '\n';
      std::cout << "    Driver Version: " << props.driverVersion << '\n';
      std::cout << "    Vendor ID: " << props.vendorID << '\n';
      std::cout << "    Device ID: " << props.deviceID << '\n';

      switch (props.deviceType) {
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        std::cout << "    Type: Discrete GPU\n";
        break;

      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        std::cout << "    Type: Integrated GPU\n";
        break;

      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        std::cout << "    Type: Virtual GPU\n";
        break;

      case VK_PHYSICAL_DEVICE_TYPE_CPU:
        std::cout << "    Type: CPU\n";
        break;

      default:
        std::cout << "    Type: Other\n";
        break;
      }

      std::cout << '\n';
    }
  }
}
std::vector<VkQueueFamilyProperties> GFVL_EnumerateDeviceQueueFamilyProperties(VkPhysicalDevice device) {
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
  if (GFVL_DEBUG_MODE) {
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
      std::cout << "Queue Family " << i << '\n';
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) std::cout << " Graphics\n";
      if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) std::cout << " Compute\n";
      if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) std::cout << " Transfer\n";
    }
  }
  return queueFamilies;
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
    .engineVersion = 1, // engine versions
    .apiVersion = VK_API_VERSION_1_3 // version of vulkan
  };

  VkInstance instance = GFVL_InitializeVkInstance(&appInfo);
  VkSurfaceKHR surface = GFVL_InitializeVkSurface(instance, window);
  std::vector<VkPhysicalDevice> availablePhysicalDevices = GFVL_EnumeratePhysicalDevices(instance);
  
  uint32_t selectedPhysicalDeviceIndex = 0;
  VkPhysicalDevice selectedPhysicalDevice = availablePhysicalDevices[selectedPhysicalDeviceIndex];
  std::vector<VkQueueFamilyProperties> queueFamilies = GFVL_EnumerateDeviceQueueFamilyProperties(selectedPhysicalDevice);

  uint32_t graphicsQueueFamily = UINT32_MAX;

  for (uint32_t i = 0; i < queueFamilies.size() ; i++) {
    VkBool32 presentSupport = VK_FALSE;

    vkGetPhysicalDeviceSurfaceSupportKHR(selectedPhysicalDevice, i, surface, &presentSupport);

    bool graphicsSupport = queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

    if (graphicsSupport && presentSupport) {
      graphicsQueueFamily = i;
      break;
    }
  }
  if (graphicsQueueFamily == UINT32_MAX) {
    std::cout << "[GFVL] No graphics queue found\n";
    return 1;
  }
  float queuePriority = 1.0f;

  VkDeviceQueueCreateInfo queueCreateInfo{
    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
    .queueFamilyIndex = graphicsQueueFamily,
    .queueCount = 1,
    .pQueuePriorities = &queuePriority,
  };

  uint32_t deviceExtensionCount = 0;

  vkEnumerateDeviceExtensionProperties(selectedPhysicalDevice, nullptr, &deviceExtensionCount, nullptr);

  std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);

  vkEnumerateDeviceExtensionProperties(selectedPhysicalDevice, nullptr, &deviceExtensionCount, deviceExtensions.data());

  if (GFVL_DEBUG_MODE) {
    std::cout << "[GFVL] Available device extensions:\n";

    for (const auto &ext : deviceExtensions) {
      std::cout << "  " << ext.extensionName << '\n';
    }
  }

  std::vector<const char *> enabledDeviceExtensionsGoon;

  for (const auto &ext : deviceExtensions) {
    if (strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
      enabledDeviceExtensionsGoon.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
  }

  uint32_t enabledDeviceExtensionCount = static_cast<uint32_t>(enabledDeviceExtensionsGoon.size());

  const char *const *enabledDeviceExtensions = enabledDeviceExtensionsGoon.data();

  VkDeviceCreateInfo logicalDeviceCreationInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = NULL,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledExtensionCount = enabledDeviceExtensionCount,
      .ppEnabledExtensionNames = enabledDeviceExtensions,
  };
  VkDevice logicalDevice;
  vkCreateDevice(selectedPhysicalDevice, &logicalDeviceCreationInfo, NULL, &logicalDevice);
  

  SDL_Delay(3000);
  vkDestroyDevice(logicalDevice, nullptr);
  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyInstance(instance, nullptr);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
