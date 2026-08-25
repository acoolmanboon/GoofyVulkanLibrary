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

#define VMA_IMPLEMENTATION

#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>

using namespace GFVL;

namespace GFVL {
  /**
  * @brief Returns a string of a VkResult.
  *
  * @param result A VkResult value to convert into a string.
  * @return const char* The converted string.
  */
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

  /**
  * @brief Prints a VkResult.
  *
  * @param result
  */
  void PrintVkResult(VkResult result) {
    std::cout << VkResultToString(result) << " (" << static_cast<int>(result) << ")\n";
  }

  /**
  * @brief Checks a VkResult, if result is not VK_SUCCESS, throw an error.
  * @details THIS IS DEPRECATED, USE CHECKVKRESULT2
  * @param result A VkResult to check.
  * @return VkResult Simply passes the result parameter to the output.
  */
  VkResult CheckVkResult(VkResult result) {
    if (result != VK_SUCCESS) {
      std::cout << "[GFVL] Error! : " << VkResultToString(result) << " (" << static_cast<int>(result) << ")\n";
      throw std::runtime_error("[GFVL] Error detected. read the above message");
    }
    return result;
  }

  /**
   * @brief Checks a VkResult, if result is not VK_SUCCESS, throw an error with a message.
   *
   * @param result A VkResult to check.
   * @param reason A message to print when the error is encountered.
   * @return VkResult Simply passes the result parameter to the output.
   */
  VkResult CheckVkResult2(VkResult result, const char *reason) {
    if (result != VK_SUCCESS) {
      std::cout << "[GFVL] Error! : " << VkResultToString(result) << " (" << static_cast<int>(result) << ")\n";
      std::cout << "[GFVL] Error cause : " << reason << "\n";
      throw std::runtime_error("[GFVL] Error detected. read the above message");
    }
    return result;
  }

  /**
   * @brief Finds memory with suitable flag bitmask.
   *
   * @param physicalDevice a Vulkan physical device
   * @param typeFilter memoryTypeBits of VkMemoryRequirements
   * @param properties The flags to use for the bitmask.
   * @return uint32_t The index of the memory type
   */
  uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
      bool typeSupported = typeFilter & (1 << i);
      bool propertiesSupported = (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;

      if (typeSupported && propertiesSupported)
        return i;
    }

    THROW_EXCEPTION("Failed to find memory type during findMemoryType");
  }

  /**
   * @brief Creates a Vulkan buffer object.
   * 
   * @param device a GFVL device.
   * @param size Size of the buffer to create in bytes.
   * @param usage Flags of how this buffer will be used
   * @param properties The memory type to allocate this buffer to
   * @param buffer Pointer to the buffer to create
   * @param bufferMemory Pointer to the device memory to create.

   */
  void createBuffer(DEVICE& device, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = static_cast<VkDeviceSize>(size),
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    CheckVkResult2(
      vkCreateBuffer(device.logicalDevice, &bufferInfo, nullptr, &buffer),
      "Failed to create a VkBuffer in createBuffer function!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device.logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = findMemoryType(device.physicalDevice, memRequirements.memoryTypeBits, properties)
    };


    CheckVkResult2(
      vkAllocateMemory(device.logicalDevice, &allocInfo, nullptr, &bufferMemory),
      "");

    CheckVkResult2(
      vkBindBufferMemory(device.logicalDevice, buffer, bufferMemory, 0),
      "");

    #ifdef GFVL_DEBUG_IMPLEMENTATION

    #endif
  }
}
