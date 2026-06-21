#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
    VERTEX_BUFFER::VERTEX_BUFFER(DEVICE& device, VkDeviceSize size, void* inputData) : device(device), size(size) {
      this->bufferInfo = {
          .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
          .size = size,
          .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
          .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

      CheckVkResult(vkCreateBuffer(this->device.logicalDevice, &this->bufferInfo, nullptr, &this->vertexBuffer));

      VkMemoryRequirements memRequirements;

      vkGetBufferMemoryRequirements(this->device.logicalDevice, this->vertexBuffer, &memRequirements);

      VkPhysicalDeviceMemoryProperties memoryProperties;

      vkGetPhysicalDeviceMemoryProperties(this->device.physicalDevice, &memoryProperties);

      uint32_t memoryType = UINT32_MAX;

      for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memoryProperties.memoryTypes[i].propertyFlags &
             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (memoryProperties.memoryTypes[i].propertyFlags &
             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
          memoryType = i;
          break;
        }
      }

      if (memoryType == UINT32_MAX)
        ERROR("No vertex memory")

      VkMemoryAllocateInfo allocVertexMemory{
          .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
          .allocationSize = memRequirements.size,
          .memoryTypeIndex = memoryType};

      CheckVkResult(vkAllocateMemory(this->device.logicalDevice, &allocVertexMemory, nullptr, &this->vertexBufferMemory));

      CheckVkResult(vkBindBufferMemory(this->device.logicalDevice, this->vertexBuffer, this->vertexBufferMemory, 0));

      CheckVkResult(vkMapMemory(device.logicalDevice, vertexBufferMemory, 0, this->bufferInfo.size, 0, &this->data));

      memcpy(this->data, inputData, this->bufferInfo.size);

      vkUnmapMemory(device.logicalDevice, vertexBufferMemory);
    }
    VERTEX_BUFFER::~VERTEX_BUFFER() {
      vkDestroyBuffer(
          this->device.logicalDevice,
          this->vertexBuffer,
          nullptr);

      vkFreeMemory(
          this->device.logicalDevice,
          this->vertexBufferMemory,
          nullptr);
    }
}
