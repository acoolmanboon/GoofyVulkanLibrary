#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
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
