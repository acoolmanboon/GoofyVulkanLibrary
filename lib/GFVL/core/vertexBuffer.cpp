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
VertexBuffer::VertexBuffer(DEVICE &device, const VertexBuffer::CreateInfo &createInfo) : device_(device), size_(createInfo.size), memoryAllocation_(createInfo.memoryAllocation) {
  if (createInfo.memoryAllocation == VertexBuffer::MemoryAllocation::HostVisibleOpportunistic) {
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
  } else if (createInfo.memoryAllocation == VertexBuffer::MemoryAllocation::HostVisible) {
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

  } else if (createInfo.memoryAllocation == VertexBuffer::MemoryAllocation::DeviceOnly) {
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    createBuffer(
        device,
        createInfo.size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingMemory);

    void *stagingData;
    CheckVkResult(vkMapMemory(
        device.logicalDevice,
        stagingMemory,
        0,
        createInfo.size,
        0,
        &stagingData));

    memcpy(stagingData, createInfo.data, createInfo.size);

    vkUnmapMemory(device.logicalDevice, stagingMemory);

    createBuffer(
        device,
        createInfo.size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        this->buffer_,
        this->bufferMemory_);

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = createInfo.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    VkCommandBuffer cmd;
    CheckVkResult(vkAllocateCommandBuffers(
        device.logicalDevice,
        &allocInfo,
        &cmd));

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

    CheckVkResult(vkBeginCommandBuffer(cmd, &beginInfo));

    VkBufferCopy region{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = createInfo.size};

    vkCmdCopyBuffer(
        cmd,
        stagingBuffer,
        this->buffer_,
        1,
        &region);

    CheckVkResult(vkEndCommandBuffer(cmd));

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd};

    CheckVkResult(vkQueueSubmit(
        device.graphicsQueue,
        1,
        &submitInfo,
        VK_NULL_HANDLE));

    CheckVkResult(vkQueueWaitIdle(device.graphicsQueue));

    vkFreeCommandBuffers(
        device.logicalDevice,
        createInfo.commandPool,
        1,
        &cmd);

    vkDestroyBuffer(device.logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(device.logicalDevice, stagingMemory, nullptr);
  } else {
    THROW_EXCEPTION("Invalid VERTEX_BUFFER_TYPE!");
  }
}
VertexBuffer::VertexBuffer(VertexBuffer &&other) noexcept
    : device_(other.device_),
      buffer_(other.buffer_),
      bufferMemory_(other.bufferMemory_),
      data_(other.data_),
      size_(other.size_),
      memoryAllocation_(other.memoryAllocation_) {
  other.buffer_ = VK_NULL_HANDLE;
  other.bufferMemory_ = VK_NULL_HANDLE;
  other.data_ = nullptr;
  other.size_ = 0;
}
VertexBuffer &VertexBuffer::operator=(VertexBuffer &&other) {
  ASSERTIF(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy buffers with different devices")
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
  this->memoryAllocation_ = other.memoryAllocation_;

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
