#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
    CommandPool::CommandPool(DEVICE& device, Framebuffer& framebuffer) : device_(device) {
      VkCommandPoolCreateInfo poolInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
          .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          .queueFamilyIndex = device.graphicsFamilyIndex};

      CheckVkResult(vkCreateCommandPool(device.logicalDevice, &poolInfo, nullptr, &this->commandPool));

      this->commandBuffers.resize(framebuffer.framebuffers.size());

      VkCommandBufferAllocateInfo allocInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          .commandPool = this->commandPool,
          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())};

      CheckVkResult(vkAllocateCommandBuffers(device.logicalDevice, &allocInfo, this->commandBuffers.data()));
    }
    CommandPool::~CommandPool() {
      vkDestroyCommandPool(this->device.logicalDevice, this->commandPool, nullptr);
      
    }
}
