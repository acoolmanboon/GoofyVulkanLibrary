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
#include <vector>
#include <vulkan/vulkan.h>

#include "../lib/GFVL_core.hpp"
using namespace GFVL;

namespace GFVL {
    
    CommandPool::CommandPool(DEVICE& device, Framebuffer& framebuffer) : device_(device) {
      VkCommandPoolCreateInfo poolInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
          .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          .queueFamilyIndex = device.graphicsFamilyIndex};

      CheckVkResult(vkCreateCommandPool(device.logicalDevice, &poolInfo, nullptr, &this->commandPool_));

      this->commandBuffers_.resize(framebuffer.framebuffers.size());

      VkCommandBufferAllocateInfo allocInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          .commandPool = this->commandPool_,
          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          .commandBufferCount = static_cast<uint32_t>(this->commandBuffers_.size())};

      CheckVkResult(vkAllocateCommandBuffers(device.logicalDevice, &allocInfo, this->commandBuffers_.data()));
    }
    const VkCommandBuffer &CommandPool::commandBuffer(size_t index) const {
      return commandBuffers_.at(index);
    }
    void CommandPool::recreate(Framebuffer& framebuffer) {
      vkDestroyCommandPool(this->device_.logicalDevice, this->commandPool_, nullptr);
      VkCommandPoolCreateInfo poolInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
          .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          .queueFamilyIndex = this->device_.graphicsFamilyIndex};

      CheckVkResult(vkCreateCommandPool(this->device_.logicalDevice, &poolInfo, nullptr, &this->commandPool_));

      this->commandBuffers_.resize(framebuffer.framebuffers.size());

      VkCommandBufferAllocateInfo allocInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          .commandPool = this->commandPool_,
          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          .commandBufferCount = static_cast<uint32_t>(this->commandBuffers_.size())};

      CheckVkResult(vkAllocateCommandBuffers(this->device_.logicalDevice, &allocInfo, this->commandBuffers_.data()));
    }
    CommandPool::~CommandPool() {
      vkDestroyCommandPool(this->device_.logicalDevice, this->commandPool_, nullptr);
      
    }
}
