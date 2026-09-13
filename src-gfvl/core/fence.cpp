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
  Fence::Fence(Device &device, VkFenceCreateFlags flags) : device_(device) {
    VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                .pNext = nullptr,
                                .flags = flags};
    CheckVkResult2(
      vkCreateFence(device_.logicalDevice, &fenceInfo, nullptr, &fence_),
      "Failed to create fence_!");

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
    VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = VK_OBJECT_TYPE_FENCE,
        .objectHandle = reinterpret_cast<uint64_t>(fence_),
        .pObjectName = "VkFence of Fence object"};
    CheckVkResult2(
        VulkanFunctionPointers::vkSetDebugUtilsObjectNameEXT(device.logicalDevice, &debugUtilsObjectNameInfo),
        "Failed to set debug utils name for VkFence of Fence object!");
#endif
  }

  Fence::Fence(Fence &&other) noexcept : device_(other.device_), fence_(other.fence_) {
    other.fence_ = VK_NULL_HANDLE;
  };

  Fence& Fence::operator=(Fence &&other) {
    ASSERTIF(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy semaphore with different devices");
    if (this == &other)
      return *this;

    if (this->fence_ != VK_NULL_HANDLE)
      vkDestroyFence(this->device_.logicalDevice, this->fence_, nullptr);
    
    this->fence_ = other.fence_;
    other.fence_ = VK_NULL_HANDLE;
    return *this;
  }

  Fence::~Fence() {
    if (fence_ != VK_NULL_HANDLE)
      vkDestroyFence(device_.logicalDevice, fence_, nullptr);
  }
}
