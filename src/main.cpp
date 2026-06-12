#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

// Remove-Item build -Recurse -Force; cmake -B build -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 -G Ninja
// cmake --build build --verbose; build/main.exe

const bool GFVL_DEBUG_MODE = true;
enum GFVL_PREFERRED_GPU {
  GFVL_PREFERRED_GPU_POWER_SAVING,
  GFVL_PREFERRED_GPU_PERFORMANCE,
};
struct GFVL_PHYSICAL_DEVICE {
  VkPhysicalDevice device = VK_NULL_HANDLE;
  uint32_t graphicsFamilyIndex = UINT32_MAX;
  uint32_t presentFamilyIndex = UINT32_MAX;
  VkDeviceSize videoMemory = 0;
};
struct GFVL_Swapchain {
  VkSwapchainKHR swapchain{};
  VkDevice device{};
  VkPhysicalDevice physicalDevice{};
  VkSurfaceKHR surface{};
  SDL_Window *window{};

  VkFormat format{};
  VkExtent2D extent{};
  VkPresentModeKHR presentMode{};
  uint32_t imageCount{};

  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;

  bool framebufferResized = false;
};

struct GFVL_SwapchainSupport {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct GFVL_SwapchainConfig {
  VkFormat format;
  VkColorSpaceKHR colorSpace;
  VkPresentModeKHR presentMode;
  VkExtent2D extent;
  uint32_t imageCount;
};
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
    throw std::runtime_error("[GFVL] Error detected.");
  }
  return result;
}
VkInstance GFVL_InitializeVkInstance(VkApplicationInfo *appInfo) {
  uint32_t instanceExtensionCount = 0;
  const char *const *instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);

  if (GFVL_DEBUG_MODE) { // optionally print the info
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
      .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
      .pApplicationInfo = appInfo,
      .enabledExtensionCount = instanceExtensionCount,
      .ppEnabledExtensionNames = instanceExtensions};

  VkInstance instance;
  CheckVkResult(vkCreateInstance(
      &instanceCreationInfo,
      NULL,
      &instance));

  return instance;
}
VkSurfaceKHR GFVL_InitializeVkSurface(VkInstance instance, SDL_Window *window) {
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
    std::cout << "[GFVL] SDL error : " << SDL_GetError() << '\n';
  return surface;
}

VkDeviceSize GFVL_GetDeviceVRAM(VkPhysicalDevice device) {
  VkPhysicalDeviceMemoryProperties memoryProperties{};
  vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

  VkDeviceSize totalDedicatedMemory = 0;

  for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++) {
    const VkMemoryHeap &heap = memoryProperties.memoryHeaps[i];

    if ((heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
      totalDedicatedMemory += heap.size;
  }

  return totalDedicatedMemory;
}
bool GFVL_EnumerateQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t &graphicsFamilyIndex, uint32_t &presentFamilyIndex) {
  graphicsFamilyIndex = UINT32_MAX;
  presentFamilyIndex = UINT32_MAX;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  if (queueFamilyCount == 0)
    return false;

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    const bool graphicsSupport =
        (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;

    VkBool32 presentationSupport = VK_FALSE;
    CheckVkResult(
        vkGetPhysicalDeviceSurfaceSupportKHR(
            device,
            i,
            surface,
            &presentationSupport));

    if (graphicsSupport && graphicsFamilyIndex == UINT32_MAX)
      graphicsFamilyIndex = i;

    if (presentationSupport == VK_TRUE && presentFamilyIndex == UINT32_MAX)
      presentFamilyIndex = i;

    /*if (GFVL_DEBUG_MODE) {
      std::cout << "[GFVL] Queue Family " << i << '\n';
      std::cout << "  Graphics: " << (graphicsSupport ? "Yes" : "No") << '\n';
      std::cout << "  Present: " << (presentationSupport ? "Yes" : "No") << '\n';
      std::cout << "  Queue Count: " << queueFamilies[i].queueCount << '\n';
    }*/
  }

  return graphicsFamilyIndex != UINT32_MAX &&
         presentFamilyIndex != UINT32_MAX;
}
bool GFVL_HasRequiredDeviceExtensions(VkPhysicalDevice device) {
  uint32_t extensionCount = 0;

  CheckVkResult(
      vkEnumerateDeviceExtensionProperties(
          device,
          nullptr,
          &extensionCount,
          nullptr));

  std::vector<VkExtensionProperties> extensions(extensionCount);

  CheckVkResult(
      vkEnumerateDeviceExtensionProperties(
          device,
          nullptr,
          &extensionCount,
          extensions.data()));

  for (const auto &extension : extensions) {
    if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
      return true;
  }

  return false;
}
int GFVL_CalculateDeviceScore(VkPhysicalDevice device, GFVL_PREFERRED_GPU preference) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(device, &properties);

  int score = 0;

  switch (properties.deviceType) {
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    score += preference == GFVL_PREFERRED_GPU_PERFORMANCE ? 1000 : 100;
    break;

  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    score += preference == GFVL_PREFERRED_GPU_POWER_SAVING ? 1000 : 500;
    break;

  case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    score += 250;
    break;

  case VK_PHYSICAL_DEVICE_TYPE_CPU:
    score += 1;
    break;

  default:
    break;
  }

  score += static_cast<int>(
      GFVL_GetDeviceVRAM(device) / (1024ull * 1024ull * 1024ull));

  return score;
}
GFVL_PHYSICAL_DEVICE GFVL_EvaluatePhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface, GFVL_PREFERRED_GPU preference) {
  uint32_t graphicsFamilyIndex = UINT32_MAX;
  uint32_t presentFamilyIndex = UINT32_MAX;

  if (!GFVL_EnumerateQueueFamilies(
          device,
          surface,
          graphicsFamilyIndex,
          presentFamilyIndex))
    return {};

  if (!GFVL_HasRequiredDeviceExtensions(device))
    return {};

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(device, &properties);

  const VkDeviceSize dedicatedVideoMemory =
      GFVL_GetDeviceVRAM(device);

  if (GFVL_DEBUG_MODE) {
    std::cout << "[GFVL] Device\n";
    std::cout << "  Name: " << properties.deviceName << '\n';
    // std::cout << "  API Version: " << VK_VERSION_MAJOR(properties.apiVersion) << '.' << VK_VERSION_MINOR(properties.apiVersion) << '.' << VK_VERSION_PATCH(properties.apiVersion) << '\n';
    // std::cout << "  Driver Version: " << properties.driverVersion << '\n';
    // std::cout << "  Vendor ID: " << properties.vendorID << '\n';
    // std::cout << "  Device ID: " << properties.deviceID << '\n';
    std::cout << "  Dedicated VRAM: " << dedicatedVideoMemory / (1024ull * 1024ull) << " MB\n";

    switch (properties.deviceType) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      std::cout << "  Type: Discrete GPU\n";
      break;

    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      std::cout << "  Type: Integrated GPU\n";
      break;

    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      std::cout << "  Type: Virtual GPU\n";
      break;

    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      std::cout << "  Type: CPU\n";
      break;

    default:
      std::cout << "  Type: Other\n";
      break;
    }

    std::cout << "  Score: "
              << GFVL_CalculateDeviceScore(device, preference)
              << '\n';
  }

  return {
      .device = device,
      .graphicsFamilyIndex = graphicsFamilyIndex,
      .presentFamilyIndex = presentFamilyIndex,
      .videoMemory = dedicatedVideoMemory};
}
GFVL_PHYSICAL_DEVICE GFVL_InitializePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, GFVL_PREFERRED_GPU preference) {
  uint32_t physicalDeviceCount = 0;

  CheckVkResult(
      vkEnumeratePhysicalDevices(
          instance,
          &physicalDeviceCount,
          nullptr));

  if (physicalDeviceCount == 0)
    throw std::runtime_error(
        "[GFVL] No Vulkan-compatible GPUs found.");

  std::vector<VkPhysicalDevice> physicalDevices(
      physicalDeviceCount);

  CheckVkResult(
      vkEnumeratePhysicalDevices(
          instance,
          &physicalDeviceCount,
          physicalDevices.data()));

  if (GFVL_DEBUG_MODE)
    std::cout << "[GFVL] Found "
              << physicalDeviceCount
              << " Vulkan-compatible devices.\n";

  GFVL_PHYSICAL_DEVICE bestDevice{};
  int bestScore = -1;

  for (const VkPhysicalDevice device : physicalDevices) {
    const GFVL_PHYSICAL_DEVICE candidate =
        GFVL_EvaluatePhysicalDevice(
            device,
            surface,
            preference);

    if (candidate.device == VK_NULL_HANDLE)
      continue;

    const int candidateScore =
        GFVL_CalculateDeviceScore(
            device,
            preference);

    if (candidateScore > bestScore) {
      bestScore = candidateScore;
      bestDevice = candidate;
    }
  }

  if (bestDevice.device == VK_NULL_HANDLE)
    throw std::runtime_error(
        "[GFVL] No suitable Vulkan device found.");

  if (GFVL_DEBUG_MODE) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(
        bestDevice.device,
        &properties);

    std::cout << "[GFVL] Selected Device: "
              << properties.deviceName
              << '\n';

    std::cout << "[GFVL] Graphics Queue Family: "
              << bestDevice.graphicsFamilyIndex
              << '\n';

    std::cout << "[GFVL] Present Queue Family: "
              << bestDevice.presentFamilyIndex
              << '\n';

    std::cout << "[GFVL] Dedicated VRAM: "
              << bestDevice.videoMemory / (1024ull * 1024ull)
              << " MB\n";

    std::cout << "[GFVL] Final Score: "
              << bestScore
              << '\n';
  }

  return bestDevice;
}
VkDeviceQueueCreateInfo GFVL_InitializeQueueCreation(uint32_t graphicsFamilyIndex, VkSurfaceKHR surface, VkPhysicalDevice device, uint32_t *familyIndex) {
  constexpr float queuePriority = 1.0f;

  VkDeviceQueueCreateInfo queueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = graphicsFamilyIndex,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};

  if (GFVL_DEBUG_MODE) {
    std::cout << "[GFVL] Queue creation info initialized.\n";
    std::cout << "[GFVL] Family Index: " << queueCreateInfo.queueFamilyIndex << '\n';
  }

  return queueCreateInfo;
}
std::vector<const char *> GFVL_EnumerateDeviceExtensions(VkPhysicalDevice device) {
  uint32_t deviceExtensionCount = 0;
  CheckVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr));

  std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
  CheckVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, deviceExtensions.data()));
  /* this prints too damn much
  if (GFVL_DEBUG_MODE) {
    std::cout << "[GFVL] Available device extensions:\n";
    for (const auto &ext : deviceExtensions)
      std::cout << "  " << ext.extensionName << '\n';
  }
  */
  std::vector<const char *> enabledDeviceExtensions;
  for (const auto &ext : deviceExtensions) {
    if (strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
      enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      if (GFVL_DEBUG_MODE)
        std::cout << "[GFVL] Enabling extension: " << VK_KHR_SWAPCHAIN_EXTENSION_NAME << '\n';
    }
  }

  if (enabledDeviceExtensions.empty())
    throw std::runtime_error("[GFVL] Required extension VK_KHR_swapchain not found.");
  if (GFVL_DEBUG_MODE)
    std::cout << "[GFVL] Enabled device extension count: " << enabledDeviceExtensions.size() << '\n';

  return enabledDeviceExtensions;
}
GFVL_SwapchainConfig GFVL_SelectSwapchainConfig(const GFVL_SwapchainSupport &swapchainSupport, SDL_Window *window, bool vsync) {
  GFVL_SwapchainConfig swapchainConfiguration{};

  swapchainConfiguration.format = swapchainSupport.formats[0].format;
  swapchainConfiguration.colorSpace = swapchainSupport.formats[0].colorSpace;

  for (const VkSurfaceFormatKHR &surfaceFormat : swapchainSupport.formats) {
    if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB) {
      swapchainConfiguration.format = surfaceFormat.format;
      swapchainConfiguration.colorSpace = surfaceFormat.colorSpace;
      break;
    }
  }

  swapchainConfiguration.presentMode = VK_PRESENT_MODE_FIFO_KHR;

  if (!vsync) {
    for (VkPresentModeKHR presentationMode : swapchainSupport.presentModes) {
      if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) {
        swapchainConfiguration.presentMode = presentationMode;
        break;
      }
    }
  }

  if (swapchainSupport.capabilities.currentExtent.width != UINT32_MAX) {
    swapchainConfiguration.extent = swapchainSupport.capabilities.currentExtent;
  } else {
    int width, height;
    SDL_GetWindowSizeInPixels(window, &width, &height);
    swapchainConfiguration.extent = {(uint32_t)width, (uint32_t)height};
  }

  uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;

  if (swapchainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapchainSupport.capabilities.maxImageCount) {
    imageCount = swapchainSupport.capabilities.maxImageCount;
  }

  swapchainConfiguration.imageCount = imageCount;

  return swapchainConfiguration;
}
GFVL_SwapchainSupport GFVL_QuerySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
  GFVL_SwapchainSupport s{};

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &s.capabilities);

  uint32_t count = 0;

  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr);
  s.formats.resize(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, s.formats.data());

  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr);
  s.presentModes.resize(count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, s.presentModes.data());

  return s;
}
void GFVL_BuildSwapchain(GFVL_Swapchain &sc, const GFVL_SwapchainConfig &c) {
  VkSwapchainCreateInfoKHR info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = sc.surface,
      .minImageCount = c.imageCount,
      .imageFormat = c.format,
      .imageColorSpace = c.colorSpace,
      .imageExtent = c.extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = c.presentMode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE};

  CheckVkResult(vkCreateSwapchainKHR(sc.device, &info, nullptr, &sc.swapchain));

  uint32_t count = 0;
  vkGetSwapchainImagesKHR(sc.device, sc.swapchain, &count, nullptr);

  sc.images.resize(count);
  vkGetSwapchainImagesKHR(sc.device, sc.swapchain, &count, sc.images.data());

  sc.imageViews.resize(count);

  for (size_t i = 0; i < count; i++) {
    VkImageViewCreateInfo vi{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = sc.images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = c.format,
        .components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY,
                       VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1}};

    CheckVkResult(vkCreateImageView(sc.device, &vi, nullptr, &sc.imageViews[i]));
  }

  sc.format = c.format;
  sc.extent = c.extent;
  sc.presentMode = c.presentMode;
  sc.imageCount = count;
}
void GFVL_DestroySwapchain(GFVL_Swapchain &sc) {
  vkDeviceWaitIdle(sc.device);

  for (auto v : sc.imageViews)
    vkDestroyImageView(sc.device, v, nullptr);

  if (sc.swapchain)
    vkDestroySwapchainKHR(sc.device, sc.swapchain, nullptr);

  sc.imageViews.clear();
  sc.images.clear();
}
void GFVL_RecreateSwapchain(GFVL_Swapchain &sc, bool vsync) {
  GFVL_DestroySwapchain(sc);

  auto support = GFVL_QuerySwapchainSupport(sc.physicalDevice, sc.surface);
  auto config = GFVL_SelectSwapchainConfig(support, sc.window, vsync);

  GFVL_BuildSwapchain(sc, config);
}
int main() {
  if (!SDL_Init(SDL_INIT_VIDEO))
    throw std::runtime_error(SDL_GetError());

  SDL_Window *window = SDL_CreateWindow(
      "goofyVLib Example",
      800,
      600,
      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  if (!window)
    throw std::runtime_error(SDL_GetError());

  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "goofyVLib Example",
      .applicationVersion = 1,
      .pEngineName = "goofyVLib",
      .engineVersion = 1,
      .apiVersion = VK_API_VERSION_1_3};

  VkInstance instance = GFVL_InitializeVkInstance(&appInfo);
  VkSurfaceKHR surface = GFVL_InitializeVkSurface(instance, window);

  GFVL_PHYSICAL_DEVICE physicalDevice =
      GFVL_InitializePhysicalDevice(
          instance,
          surface,
          GFVL_PREFERRED_GPU_POWER_SAVING);

  uint32_t graphicsFamilyIndex = 0;

  VkDeviceQueueCreateInfo queueInfo =
      GFVL_InitializeQueueCreation(
          physicalDevice.graphicsFamilyIndex,
          surface,
          physicalDevice.device,
          &graphicsFamilyIndex);

  std::vector<const char *> deviceExtensions =
      GFVL_EnumerateDeviceExtensions(physicalDevice.device);

  VkDevice device;

  VkDeviceCreateInfo deviceInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueInfo,
      .enabledExtensionCount = (uint32_t)deviceExtensions.size(),
      .ppEnabledExtensionNames = deviceExtensions.data()};

  CheckVkResult(
      vkCreateDevice(
          physicalDevice.device,
          &deviceInfo,
          nullptr,
          &device));

  VkQueue graphicsQueue;
  vkGetDeviceQueue(device, graphicsFamilyIndex, 0, &graphicsQueue);

  GFVL_Swapchain swapchain{};
  swapchain.device = device;
  swapchain.physicalDevice = physicalDevice.device;
  swapchain.surface = surface;
  swapchain.window = window;

  auto initialSupport = GFVL_QuerySwapchainSupport(physicalDevice.device, surface);

  auto initialConfig =
      GFVL_SelectSwapchainConfig(
          initialSupport,
          window,
          true);

  GFVL_BuildSwapchain(swapchain, initialConfig);

  bool running = true;
  bool framebufferResized = false;

  while (running) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {

      case SDL_EVENT_QUIT:
        running = false;
        break;

      case SDL_EVENT_WINDOW_RESIZED:
      case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        framebufferResized = true;
        break;
      }
    }

    if (framebufferResized) {
      vkDeviceWaitIdle(device);

      GFVL_RecreateSwapchain(swapchain, true);

      framebufferResized = false;

      std::cout << "[GFVL] Swapchain recreated due to resize\n";
    }

  }

  vkDeviceWaitIdle(device);

  GFVL_DestroySwapchain(swapchain);

  vkDestroyDevice(device, nullptr);
  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyInstance(instance, nullptr);

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}