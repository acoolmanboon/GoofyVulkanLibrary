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
VertexBuffer::VertexBuffer(DEVICE &device, const VertexBuffer::CreateInfo &createInfo) : device_(device), size_(createInfo.size), type_(createInfo.type) {
  if (createInfo.type == VertexBuffer::MemoryAllocation::HostVisibleOpportunistic) {
    createBuffer(
        device,
        createInfo.size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        this->buffer_,
        this->bufferMemory_);

    CheckVkResult(vkMapMemory(
        device.logicalDevice,
        bufferMemory_,
        0,
        createInfo.size,
        0,
        &this->data_));

    memcpy(this->data_, createInfo.data, createInfo.size);

    vkUnmapMemory(device.logicalDevice, bufferMemory_);
  } else if (createInfo.type == VertexBuffer::MemoryAllocation::HostVisible) {
    createBuffer(
        device,
        createInfo.size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        this->buffer_,
        this->bufferMemory_);

    CheckVkResult(vkMapMemory(
        device.logicalDevice,
        bufferMemory_,
        0,
        createInfo.size,
        0,
        &this->data_));

    memcpy(this->data_, createInfo.data, createInfo.size);

    vkUnmapMemory(device.logicalDevice, bufferMemory_);

  } else if (createInfo.type == VertexBuffer::MemoryAllocation::DeviceOnly) {
    createBuffer(
        device,
        createInfo.size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        this->buffer_,
        this->bufferMemory_);

    void *stagingData = nullptr;
    CheckVkResult(vkMapMemory(
        device.logicalDevice,
        this->bufferMemory_,
        0,
        createInfo.size,
        0,
        &stagingData));
    memcpy(stagingData, createInfo.data, createInfo.size);
    vkUnmapMemory(
        device.logicalDevice,
        this->bufferMemory_);

  } else {
    THROW_EXCEPTION("Invalid VERTEX_BUFFER_TYPE!");
  }
}
const void *VertexBuffer::data() const {
  ASSERT(this->type_ != VertexBuffer::MemoryAllocation::HostVisible, "Attempted to access vertex buffer without parameter HOST_VISIBLE, current enum is " << enumToString(this->type_));
  return this->data_;
}
const size_t VertexBuffer::size() const {
  return this->size_;
}
const VkBuffer &VertexBuffer::buffer() const {
  return this->buffer_;
}
const VertexBuffer::MemoryAllocation VertexBuffer::type() const {
  return this->type_;
}
VertexBuffer::VertexBuffer(VertexBuffer &&other) noexcept
    : device_(other.device_),
      buffer_(other.buffer_),
      bufferMemory_(other.bufferMemory_),
      data_(other.data_),
      size_(other.size_),
      type_(other.type_) {
  other.buffer_ = VK_NULL_HANDLE;
  other.bufferMemory_ = VK_NULL_HANDLE;
  other.data_ = nullptr;
  other.size_ = 0;
}
VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other) {
  ASSERT(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy buffers with different devices")
  if (this == &other)
    return *this;

  if (buffer_ != VK_NULL_HANDLE) {
    vkDestroyBuffer(device_.logicalDevice, buffer_, nullptr);
  }

  if (bufferMemory_ != VK_NULL_HANDLE) {
    vkFreeMemory(device_.logicalDevice, bufferMemory_, nullptr);
  }

  this->buffer_ = other.buffer_;
  this->bufferMemory_ = other.bufferMemory_;
  this->data_ = other.data_;
  this->size_ = other.size_;
  this->type_ = other.type_;

  other.buffer_ = VK_NULL_HANDLE;
  other.bufferMemory_ = VK_NULL_HANDLE;
  other.data_ = nullptr;
  other.size_ = 0;

  return *this;
}
VertexBuffer::~VertexBuffer() {
  vkDestroyBuffer(this->device_.logicalDevice, this->buffer_, nullptr);
  vkFreeMemory(this->device_.logicalDevice, this->bufferMemory_, nullptr);
}
} // namespace GFVL
