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
#include <GFVL_core.hpp>
#include <GFVL_definition.hpp>
#include <GFVL_vkFunctionPointers.hpp>

using namespace GFVL;

namespace GFVL {
  Semaphore::Semaphore(Device &device) : device_(device) {
    VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                                        .pNext = nullptr,
                                        .flags = 0};
    CheckVkResult2(
      vkCreateSemaphore(device_.logicalDevice, &semaphoreInfo, nullptr, &semaphore_),
      "Failed to create semaphore_!");

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
    VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = VK_OBJECT_TYPE_SEMAPHORE,
        .objectHandle = reinterpret_cast<uint64_t>(semaphore_),
        .pObjectName = "VkSemaphore of Semaphore object"};
    CheckVkResult2(
        VulkanFunctionPointers::vkSetDebugUtilsObjectNameEXT(device.logicalDevice, &debugUtilsObjectNameInfo),
        "Failed to set debug utils name for VkSemaphore of Semaphore object!");
#endif
  }

  Semaphore::Semaphore(Semaphore &&other) noexcept : device_(other.device_), semaphore_(other.semaphore_) {
    other.semaphore_ = VK_NULL_HANDLE;
  };
  Semaphore& Semaphore::operator=(Semaphore &&other) {
    ASSERTIF(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy semaphore_ with different devices");
    if (this == &other)
      return *this;

    if (this->semaphore_ != VK_NULL_HANDLE) {
      vkDestroySemaphore(this->device_.logicalDevice, this->semaphore_, nullptr);
    }
    this->semaphore_ = other.semaphore_;
    other.semaphore_ = VK_NULL_HANDLE;
    return *this;
  }

  Semaphore::~Semaphore() {
    if (semaphore_ != VK_NULL_HANDLE)
      vkDestroySemaphore(device_.logicalDevice, semaphore_, nullptr);
  } 
}
