#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL; 
VkDeviceSize GetDeviceVRAM(VkPhysicalDevice device) {
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
bool EnumerateQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t &graphicsFamilyIndex, uint32_t &presentFamilyIndex) {
  graphicsFamilyIndex = UINT32_MAX;
  presentFamilyIndex = UINT32_MAX;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  if (queueFamilyCount == 0)
    return false;

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    const bool graphicsSupport = (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
    VkBool32 presentationSupport = VK_FALSE;
    CheckVkResult(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport));

    if (graphicsSupport && graphicsFamilyIndex == UINT32_MAX)
      graphicsFamilyIndex = i;

    if (presentationSupport == VK_TRUE && presentFamilyIndex == UINT32_MAX)
      presentFamilyIndex = i;

    /*if (DEBUG_MODE) {
      std::cout << "[GFVL] Queue Family " << i << '\n';
      std::cout << "  Graphics: " << (graphicsSupport ? "Yes" : "No") << '\n';
      std::cout << "  Present: " << (presentationSupport ? "Yes" : "No") << '\n';
      std::cout << "  Queue Count: " << queueFamilies[i].queueCount << '\n';
    }*/
  }

  return graphicsFamilyIndex != UINT32_MAX && presentFamilyIndex != UINT32_MAX;
}
bool HasRequiredDeviceExtensions(VkPhysicalDevice device) {
  uint32_t extensionCount = 0;

  CheckVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr));

  std::vector<VkExtensionProperties> extensions(extensionCount);

  CheckVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data()));

  for (const VkExtensionProperties &extension : extensions) {
    if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
      return true;
  }

  return false;
}
int CalculateDeviceScore(VkPhysicalDevice device, PREFERRED_GPU preference) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(device, &properties);

  int score = 0;

  switch (properties.deviceType) {
  case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    score += preference == PREFERRED_GPU_PERFORMANCE ? 1000 : 100;
    break;

  case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    score += preference == PREFERRED_GPU_POWER_SAVING ? 1000 : 500;
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

  score += static_cast<int>(GetDeviceVRAM(device) / (1024ull * 1024ull * 1024ull));

  return score;
}
DEVICE EvaluatePhysicalDevice(VkPhysicalDevice device, VkSurfaceKHR surface, PREFERRED_GPU preference) {
  uint32_t graphicsFamilyIndex = UINT32_MAX;
  uint32_t presentFamilyIndex = UINT32_MAX;

  if (!EnumerateQueueFamilies(device, surface, graphicsFamilyIndex, presentFamilyIndex))
    return {
        PREFERRED_GPU_PERFORMANCE,
        VkInstance{},
        VkSurfaceKHR{},
        PHYSICAL_DEVICE{.physicalDevice = VK_NULL_HANDLE, .graphicsFamilyIndex = 0, .presentFamilyIndex = 0, .videoMemory = 0},
    };
  if (!HasRequiredDeviceExtensions(device))
    return {
        PREFERRED_GPU_PERFORMANCE,
        VkInstance{},
        VkSurfaceKHR{},
        PHYSICAL_DEVICE{.physicalDevice = VK_NULL_HANDLE, .graphicsFamilyIndex = 0, .presentFamilyIndex = 0, .videoMemory = 0},
    };

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(device, &properties);

  const VkDeviceSize dedicatedVideoMemory = GetDeviceVRAM(device);

  if (DEBUG_MODE) {
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

    std::cout << "  Score: " << CalculateDeviceScore(device, preference) << '\n';
  }
  return {
      PREFERRED_GPU_PERFORMANCE,
      VkInstance{},
      VkSurfaceKHR{},
      PHYSICAL_DEVICE{.physicalDevice = device, .graphicsFamilyIndex = graphicsFamilyIndex, .presentFamilyIndex = presentFamilyIndex, .videoMemory = dedicatedVideoMemory},
  };
}
void InitializePhysicalDevice(DEVICE &device, VkInstance instance, VkSurfaceKHR surface, PREFERRED_GPU preference) {
  uint32_t physicalDeviceCount = 0;

  CheckVkResult(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));

  if (physicalDeviceCount == 0)
    throw std::runtime_error("[GFVL] No Vulkan-compatible GPUs found.");

  std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

  CheckVkResult(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));
  if (DEBUG_MODE) std::cout << "[GFVL] Found " << physicalDeviceCount << " Vulkan-compatible devices.\n";

  int bestScore = -1;

  for (const VkPhysicalDevice indexedPhysicalDevice : physicalDevices) {
    const DEVICE candidate = EvaluatePhysicalDevice(indexedPhysicalDevice, surface, preference);

    if (candidate.physicalDevice == VK_NULL_HANDLE)
      continue;

    const int candidateScore = CalculateDeviceScore(indexedPhysicalDevice, preference);

    if (candidateScore > bestScore) {
      bestScore = candidateScore;
      device = candidate;
    }
  }

  if (device.physicalDevice == VK_NULL_HANDLE) 
    throw std::runtime_error("[GFVL] No suitable Vulkan device found.");

  if (DEBUG_MODE) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device.physicalDevice, &properties);

    std::cout << "[GFVL] Selected Device: " << properties.deviceName << '\n';
    std::cout << "[GFVL] Graphics Queue Family: " << device.graphicsFamilyIndex << '\n';
    std::cout << "[GFVL] Present Queue Family: " << device.presentFamilyIndex << '\n';
    std::cout << "[GFVL] Dedicated VRAM: " << device.videoMemory / (1024ull * 1024ull) << " MB\n";
    std::cout << "[GFVL] Final Score: " << bestScore << '\n';
  }
}
std::vector<const char *> EnumerateDeviceExtensions(VkPhysicalDevice device) {
  uint32_t deviceExtensionCount = 0;
  CheckVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr));

  std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
  CheckVkResult(vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, deviceExtensions.data()));
  /* this prints too damn much
  if (DEBUG_MODE) {
    std::cout << "[GFVL] Available device extensions:\n";
    for (const auto &ext : deviceExtensions)
      std::cout << "  " << ext.extensionName << '\n';
  }
  */
  std::vector<const char *> enabledDeviceExtensions;

  for (const VkExtensionProperties &extension : deviceExtensions) {
    if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
      enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      if (DEBUG_MODE)
        std::cout << "[GFVL] Enabling extension: " << VK_KHR_SWAPCHAIN_EXTENSION_NAME << '\n';
    }
  }

  if (enabledDeviceExtensions.empty())
    throw std::runtime_error("[GFVL] Required extension VK_KHR_swapchain not found.");

  if (DEBUG_MODE)
    std::cout << "[GFVL] Enabled device extension count: " << enabledDeviceExtensions.size() << '\n';

  return enabledDeviceExtensions;
}
namespace GFVL {

DEVICE::DEVICE(PREFERRED_GPU preferred_gpu, VkInstance instance, VkSurfaceKHR surface, PHYSICAL_DEVICE override) {
  if (override.physicalDevice != VK_NULL_HANDLE) {
    InitializePhysicalDevice(*this, instance, surface, preferred_gpu);

    constexpr float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = this->graphicsFamilyIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};

    std::vector<const char *> deviceExtensions = EnumerateDeviceExtensions(this->physicalDevice);
    VkDeviceCreateInfo logicalDeviceCreationInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = (uint32_t)deviceExtensions.size(),
        .ppEnabledExtensionNames = deviceExtensions.data()};
    CheckVkResult(vkCreateDevice(this->physicalDevice, &logicalDeviceCreationInfo, nullptr, &this->device));
    this->physicalDevice = override.physicalDevice;
    this->graphicsFamilyIndex = override.graphicsFamilyIndex;
    this->presentFamilyIndex = override.presentFamilyIndex;
    this->videoMemory = override.videoMemory;
  }
}
DEVICE::~DEVICE() {
  vkDestroyDevice(this->device, nullptr)
}
}
