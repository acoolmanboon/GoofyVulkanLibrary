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

using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
VertexBuffer::VertexBuffer(DEVICE &device, const VertexBuffer::CreateInfo &createInfo) : device_(device),
                                                                                         allocator_(createInfo.allocator),
                                                                                         size_(createInfo.size),
                                                                                         memoryAllocation_(createInfo.memoryAllocation) {
  if (memoryAllocation_ == MemoryAllocation::HostVisible) {
    VkBufferCreateInfo bufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = static_cast<VkDeviceSize>(size_),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &device_.graphicsFamilyIndex,
    };

    VmaAllocationCreateInfo allocationCreateInfo{
        .flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
    };

    CheckVkResult2(
        vmaCreateBuffer(
            allocator_,
            &bufferCreateInfo,
            &allocationCreateInfo,
            &buffer_,
            &bufferMemory_,
            nullptr),
        "Failed to create vertex buffer!");

    CheckVkResult2(
        vmaMapMemory(
            allocator_,
            bufferMemory_,
            &data_),
        "Failed to map vertex buffer!");

    memcpy(data_, createInfo.data, size_);

    vmaUnmapMemory(allocator_, bufferMemory_);
  } else if (memoryAllocation_ == MemoryAllocation::DeviceOnly) {
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VkBufferCreateInfo stagingCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = static_cast<VkDeviceSize>(size_),
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &device_.graphicsFamilyIndex,
    };

    VmaAllocationCreateInfo stagingAllocationInfo{
        .flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT |
                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
    };

    CheckVkResult2(
        vmaCreateBuffer(
            allocator_,
            &stagingCreateInfo,
            &stagingAllocationInfo,
            &stagingBuffer,
            &stagingAllocation,
            nullptr),
        "Failed to create staging buffer!");

    void *stagingData = nullptr;

    CheckVkResult2(
        vmaMapMemory(
            allocator_,
            stagingAllocation,
            &stagingData),
        "Failed to map staging buffer!");

    memcpy(stagingData, createInfo.data, size_);

    vmaUnmapMemory(
        allocator_,
        stagingAllocation);

    VkBufferCreateInfo vertexBufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = static_cast<VkDeviceSize>(size_),
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 1,
        .pQueueFamilyIndices = &device_.graphicsFamilyIndex,
    };

    VmaAllocationCreateInfo vertexAllocationInfo{
        .flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    };

    CheckVkResult2(
        vmaCreateBuffer(
            allocator_,
            &vertexBufferCreateInfo,
            &vertexAllocationInfo,
            &buffer_,
            &bufferMemory_,
            nullptr),
        "Failed to create vertex buffer!");

    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = createInfo.commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;

    CheckVkResult(vkAllocateCommandBuffers(
        device.logicalDevice,
        &allocInfo,
        &cmd));

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    CheckVkResult(vkBeginCommandBuffer(cmd, &beginInfo));

    VkBufferCopy copyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = static_cast<VkDeviceSize>(size_),
    };

    vkCmdCopyBuffer(
        cmd,
        stagingBuffer,
        buffer_,
        1,
        &copyRegion);

    CheckVkResult(vkEndCommandBuffer(cmd));

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

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

    vmaDestroyBuffer(
        allocator_,
        stagingBuffer,
        stagingAllocation);
  } else {
    THROW_EXCEPTION("Invalid VertexBuffer::MemoryAllocation!");
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
  ASSERTIF(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy buffers with different devices");
  if (this == &other)
    return *this;

  if (buffer_ != VK_NULL_HANDLE && bufferMemory_ != VK_NULL_HANDLE) {
    vmaDestroyBuffer(allocator_, buffer_, bufferMemory_);
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
  vmaDestroyBuffer(allocator_, buffer_, bufferMemory_);
}
} // namespace GFVL
