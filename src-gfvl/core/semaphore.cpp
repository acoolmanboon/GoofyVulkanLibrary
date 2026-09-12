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
#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>

using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
  Semaphore::Semaphore(Device &device) : device_(device) {
    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    CheckVkResult2(
      vkCreateSemaphore(this->device_.logicalDevice, &semaphoreInfo, nullptr,&this->semaphore),
      "Failed to create semaphore!");
  }

  Semaphore::Semaphore(Semaphore &&other) noexcept : device_(other.device_), semaphore(other.semaphore) {
    other.semaphore = VK_NULL_HANDLE;
  };
  Semaphore& Semaphore::operator=(Semaphore &&other) {
    ASSERTIF(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy semaphore with different devices");
    if (this == &other)
      return *this;

    if (this->semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(this->device_.logicalDevice, this->semaphore, nullptr);
    }
    this->semaphore = other.semaphore;
    other.semaphore = VK_NULL_HANDLE;
    return *this;
  }

  Semaphore::~Semaphore() {
    if (this->semaphore != VK_NULL_HANDLE)
      vkDestroySemaphore(this->device_.logicalDevice, this->semaphore, nullptr);
  } 
}
