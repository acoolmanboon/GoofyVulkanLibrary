#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstring>
#include <vector>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
    COMMAND_POOL::COMMAND_POOL(DEVICE& device) : device(device) {
      VkCommandPoolCreateInfo poolInfo{
          .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
          .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
          .queueFamilyIndex = device.graphicsFamilyIndex};

      CheckVkResult(vkCreateCommandPool(device.logicalDevice, &poolInfo, nullptr, &this->commandPool));
    }
    COMMAND_POOL::~COMMAND_POOL() {
      vkDestroyCommandPool(this->device.logicalDevice, this->commandPool, nullptr);
    }
}
