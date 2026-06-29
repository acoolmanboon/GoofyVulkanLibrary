#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
#include "enumPrinter.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
VertexBuffer::VertexBuffer(DEVICE &device, const VertexBuffer::CreateInfo &createInfo) : device(device), size_(createInfo.size), type_(createInfo.type) {
  if (createInfo.type == VertexBuffer::Type::HostVisible) {
    createBuffer(
        device,
        createInfo.size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        this->buffer_,
        this->bufferMemory);

    vkMapMemory(
        device.logicalDevice,
        bufferMemory,
        0,
        createInfo.size,
        0,
        &this->data_);

    memcpy(this->data_, createInfo.data, createInfo.size);

    vkUnmapMemory(device.logicalDevice, bufferMemory);

  } else if (createInfo.type == VertexBuffer::Type::DeviceOnly) {

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    createBuffer(
        device,
        createInfo.size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *stagingData = nullptr;
    vkMapMemory(
        device.logicalDevice,
        stagingBufferMemory,
        0,
        createInfo.size,
        0,
        &stagingData);
    memcpy(stagingData, createInfo.data, createInfo.size);
    vkUnmapMemory(
        device.logicalDevice,
        stagingBufferMemory);

    createBuffer(
        device,
        createInfo.size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        this->buffer_,
        this->bufferMemory);

    VkBufferCopy copyRegion{
      .srcOffset = 0,
      .dstOffset = 0,
      .size = createInfo.size
    };

    vkCmdCopyBuffer(
        *createInfo.commandBuffer,
        stagingBuffer,
        this->buffer_,
        1,
        &copyRegion);
  } else {
    THROW_EXCEPTION("Invalid VERTEX_BUFFER_TYPE!");
  }
}
void* VertexBuffer::data() const {
  ASSERT(this->type_ != VertexBuffer::Type::HostVisible, "Attempted to access vertex buffer without parameter HOST_VISIBLE, current enum is " << enumToString(this->type_));
  return this->data_;
}
size_t VertexBuffer::size() const {
  return this->size_;
}
const VkBuffer &VertexBuffer::buffer() const {
  return this->buffer_;
}
VertexBuffer::Type VertexBuffer::type() const {
  return this->type_;
}
VertexBuffer::~VertexBuffer() {
  vkDestroyBuffer(
      this->device.logicalDevice,
      this->buffer_,
      nullptr);

  vkFreeMemory(
      this->device.logicalDevice,
      this->bufferMemory,
      nullptr);
}
} // namespace GFVL
