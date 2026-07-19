/*
GoofyVulkanLibrary. A vulkan wrapper, designed to allow users to code Vulkan applications without high boilerplate.
Copyright (C) 2026 acoolmanboon

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include "../lib/GFVL_core.hpp"
using namespace GFVL;

VkDeviceSize getDeviceVRAM(VkPhysicalDevice device) {
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
uint32_t getDeviceScore(VkPhysicalDevice device, PREFERRED_GPU preference) {
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(device, &properties);

  uint32_t score = 0;

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

  score += static_cast<int>(getDeviceVRAM(device) / (1024ull * 1024ull * 1024ull));

  return score;
}
VkBool32 enumerateQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t &graphicsFamilyIndex, uint32_t &presentFamilyIndex) {
  graphicsFamilyIndex = UINT32_MAX;
  presentFamilyIndex = UINT32_MAX;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  if (queueFamilyCount == 0)
    return false;

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    const VkBool32 graphicsSupport = (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
    VkBool32 presentationSupport = VK_FALSE;
    CheckVkResult(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationSupport));

    if (graphicsSupport && presentationSupport) {
      PRINT("Found a queue family with both graphics support and presentation support!")
      graphicsFamilyIndex = i;
      presentFamilyIndex = i;
      break;
    }
    if (graphicsSupport && graphicsFamilyIndex == UINT32_MAX)
      graphicsFamilyIndex = i;

    if (presentationSupport && presentFamilyIndex == UINT32_MAX)
      presentFamilyIndex = i;
  }

  return graphicsFamilyIndex != UINT32_MAX && presentFamilyIndex != UINT32_MAX;
}
VkBool32 hasRequiredDeviceExtensions(VkPhysicalDevice device) {
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
std::vector<const char *> enumerateDeviceExtensions(VkPhysicalDevice device) {
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
  for (const VkExtensionProperties &ext : deviceExtensions) {
    if (strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
      enabledDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
      PRINT("Enabling extension: " << VK_KHR_SWAPCHAIN_EXTENSION_NAME)
    }
  }

  if (enabledDeviceExtensions.empty()) {
    throw std::runtime_error("[GFVL] Required extension VK_KHR_swapchain not found.");
  }
  PRINT("Enabled device extension count: " << enabledDeviceExtensions.size())

  return enabledDeviceExtensions;
}

namespace GFVL {
DEVICE::DEVICE(VkInstance instance, VkSurfaceKHR surface, PREFERRED_GPU preference) {
  uint32_t physicalDeviceCount = 0;
  CheckVkResult(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
  if (physicalDeviceCount == 0)
    throw std::runtime_error("[GFVL] No Vulkan-compatible GPUs found.");
  std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

  CheckVkResult(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

  if (DEBUG_MODE)
    std::cout << "[GFVL] Found " << physicalDeviceCount << " Vulkan-compatible devices.\n";

  int bestScore = -1;

  for (const VkPhysicalDevice physicalDevice : physicalDevices) {
    uint32_t graphicsFamilyIndex = UINT32_MAX;
    uint32_t presentFamilyIndex = UINT32_MAX;
    if (!enumerateQueueFamilies(physicalDevice, surface, graphicsFamilyIndex, presentFamilyIndex))
      continue;
    if (!hasRequiredDeviceExtensions(physicalDevice))
      continue;

    const int candidateScore = getDeviceScore(physicalDevice, preference);

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    const VkDeviceSize dedicatedVideoMemory = getDeviceVRAM(physicalDevice);
    if (DEBUG_MODE) {
      std::cout << "[GFVL] Device\n";
      std::cout << "  Name: " << properties.deviceName << '\n';
      std::cout << "  API Version: " << VK_VERSION_MAJOR(properties.apiVersion) << '.' << VK_VERSION_MINOR(properties.apiVersion) << '.' << VK_VERSION_PATCH(properties.apiVersion) << '\n';
      std::cout << "  Driver Version: " << properties.driverVersion << '\n';
      std::cout << "  Vendor ID: " << properties.vendorID << '\n';
      std::cout << "  Device ID: " << properties.deviceID << '\n';
      std::cout << "  Graphics Family Index: " << graphicsFamilyIndex << '\n';
      std::cout << "  Present Family Index: " << presentFamilyIndex << '\n';
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

      std::cout << "  Score: " << getDeviceScore(physicalDevice, preference) << '\n';
    }
    if (candidateScore > bestScore) {
      bestScore = candidateScore;
      this->physicalDevice = physicalDevice;
      this->videoMemory = dedicatedVideoMemory;
      this->graphicsFamilyIndex = graphicsFamilyIndex;
      this->presentFamilyIndex = presentFamilyIndex;
    }
  }
  if (this->physicalDevice == VK_NULL_HANDLE)
    throw std::runtime_error("[GFVL] No suitable Vulkan device found.");

  if (DEBUG_MODE) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);

    std::cout << "[GFVL] Selected Device: " << properties.deviceName << '\n';
    std::cout << "[GFVL] Graphics Queue Family: " << this->graphicsFamilyIndex << '\n';
    std::cout << "[GFVL] Present Queue Family: " << this->presentFamilyIndex << '\n';
    std::cout << "[GFVL] Dedicated VRAM: " << this->videoMemory / (1024ull * 1024ull) << " MB\n";
    std::cout << "[GFVL] Final Score: " << bestScore << '\n';
  }

  const float queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  // why the hell does it indent like this when i paste the code
  queueCreateInfos.push_back({.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                              .queueFamilyIndex = graphicsFamilyIndex,
                              .queueCount = 1,
                              .pQueuePriorities = &queuePriority});

  if (presentFamilyIndex != graphicsFamilyIndex) {
    queueCreateInfos.push_back({.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                .queueFamilyIndex = presentFamilyIndex,
                                .queueCount = 1,
                                .pQueuePriorities = &queuePriority});
  }

  std::vector<const char *> deviceExtensions = enumerateDeviceExtensions(this->physicalDevice);
  
  VkDeviceCreateInfo deviceInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
      .pQueueCreateInfos = queueCreateInfos.data(),
      .enabledExtensionCount = (uint32_t)deviceExtensions.size(),
      .ppEnabledExtensionNames = deviceExtensions.data()};

  CheckVkResult(vkCreateDevice(this->physicalDevice, &deviceInfo, nullptr, &this->logicalDevice));
  vkGetDeviceQueue(this->logicalDevice, this->graphicsFamilyIndex, 0, &this->graphicsQueue);
}
DEVICE::~DEVICE() {
  if (logicalDevice != VK_NULL_HANDLE) {
    vkDestroyDevice(logicalDevice, nullptr);
    logicalDevice = VK_NULL_HANDLE;
  }
}
} // namespace GFVL