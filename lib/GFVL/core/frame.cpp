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
/**
 * @file GFVL_frame.hpp
 * @brief PLACEHOLDER
 * @details Don't include this. Unless you wanna do some master hacking?
 */
#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>

namespace GFVL {
void Frame::createCommandPool() {
  VkCommandPoolCreateInfo commandPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      .queueFamilyIndex = device.graphicsFamilyIndex};

  CheckVkResult2(
      vkCreateCommandPool(device.logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool),
      "Failed to create command pool in Frame creation!");

  VkCommandBufferAllocateInfo commandBufferAllocationInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

  CheckVkResult2(
      vkAllocateCommandBuffers(device.logicalDevice, &commandBufferAllocationInfo, &commandBuffer),
      "Failed to allocate command buffers in Frame creation!");
}
void Frame::createDescriptorPool(uint32_t descriptorCount) {
  VkDescriptorPoolSize poolSize{
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = descriptorCount};

  VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize};

  CheckVkResult2(
      vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPool),
      "Failed to create descriptor pool in Frame creation!");
}
void Frame::updateUniformBuffers() {
  for (size_t i = 0; i < bindings.size(); i++) {
    if (!bindings[i].hasUpdated)
      continue;

    memcpy(uniformBuffers[i].mappedMemory, bindings[i].ubo,  bindings[i].size);
  }
}                 
Frame::Frame(DEVICE &device, VmaAllocator allocator, VkDescriptorSetLayout descriptorSetLayout, const std::vector<UniformBufferBinding> &bindings) : device(device),
                                                                                                                                                     bindings(bindings),
                                                                                                                                                     allocator(allocator),
                                                                                                                                                     imageAvailableSemaphore(device),
                                                                                                                                                     gpuFinishedFence(device, VK_FENCE_CREATE_SIGNALED_BIT) {

  createCommandPool();
  createDescriptorPool(static_cast<uint32_t>(bindings.size()));

  VkDescriptorSetAllocateInfo allocation{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayout};

  CheckVkResult2(
      vkAllocateDescriptorSets(device.logicalDevice, &allocation, &descriptorSet),
      "Failed to allocate descriptor sets in Frame creation!");

  uniformBuffers.reserve(bindings.size());

  for (const UniformBufferBinding &binding : bindings) {
    ASSERTIF(binding.ubo == nullptr, "UniformBufferBinding ubo cannot be nullptr!")
    ASSERTIF(binding.size == 0, "UniformBufferBinding size cannot be 0 bytes!")
    ASSERTIF(binding.shaderStage == 0, "UniformBufferBinding shader stage has no flags! This should not be possible unless it is explicitly initialized as such.")
    ASSERTIF(binding.arrayCount == 0, "UniformBufferBinding array count is 0. This should not be possible unless you explicitly initialized it to 0.")
    ASSERTIF(binding.arrayCount != 1, "UniformBufferBinding array count being 1 is only implemented.")
    // this may or may not work
    FrameUniformBuffer uniformBuffer{};

    VkBufferCreateInfo bufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = binding.size,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    VmaAllocationCreateInfo allocationCreateInfo{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO};

    VmaAllocationInfo allocationInfo{};

    CheckVkResult2(vmaCreateBuffer(
                       allocator,
                       &bufferCreateInfo,
                       &allocationCreateInfo,
                       &uniformBuffer.buffer,
                       &uniformBuffer.allocation,
                       &allocationInfo),
                   "Failed to create frame uniform buffer!");

    uniformBuffer.mappedMemory = allocationInfo.pMappedData;

    memcpy(uniformBuffer.mappedMemory, binding.ubo, binding.size);

    uniformBuffers.emplace_back(uniformBuffer);
  }
  std::vector<VkDescriptorBufferInfo> descriptorBufferInfos(bindings.size());

  for (size_t i = 0; i < bindings.size(); i++) {
    descriptorBufferInfos[i] = {
        .buffer = uniformBuffers[i].buffer,
        .offset = 0,
        .range = bindings[i].size};
  }

  std::vector<VkWriteDescriptorSet> writes;
  writes.reserve(bindings.size());

  for (size_t descriptorBufferInfoIndex = 0; descriptorBufferInfoIndex < bindings.size(); descriptorBufferInfoIndex++) {
    writes.push_back(
        VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = bindings[descriptorBufferInfoIndex].binding,
            .descriptorCount = bindings[descriptorBufferInfoIndex].arrayCount,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &descriptorBufferInfos[descriptorBufferInfoIndex]});
  }

  vkUpdateDescriptorSets(device.logicalDevice, writes.size(), writes.data(), 0, nullptr);
}
Frame::~Frame() {
  vkDestroyDescriptorPool(device.logicalDevice, descriptorPool, nullptr);

  vkDestroyCommandPool(device.logicalDevice, commandPool, nullptr);

  for (FrameUniformBuffer &buffer : uniformBuffers) {
    vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
  }
}
} // namespace GFVL
