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
#include "../lib/GFVL_core.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <vulkan/vulkan.h>

using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
BINDING::BINDING(DEVICE &device, size_t size, void *ubo, uint32_t binding) : device(device), size(size) {
  PRINT("Creating binding at binding " << binding)
  VkBufferCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
  CheckVkResult(vkCreateBuffer(device.logicalDevice, &createInfo, nullptr, &this->buffer));

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(device.logicalDevice, this->buffer, &requirements);

  VkMemoryAllocateInfo allocation{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = requirements.size, .memoryTypeIndex = findMemoryType(device.physicalDevice, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};
  CheckVkResult(vkAllocateMemory(device.logicalDevice, &allocation, nullptr, &this->memory));

  CheckVkResult(vkBindBufferMemory(device.logicalDevice, this->buffer, this->memory, 0));

  this->layout = {.binding = binding, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS};

  this->bufferInfo = {.buffer = this->buffer, .offset = 0, .range = size};

  vkMapMemory(device.logicalDevice, this->memory, 0, size, 0, &this->data);

  memcpy(this->data, ubo, size);
}
void BINDING::update(void *ubo) {
  memcpy(this->data, ubo, this->size);
}
BINDING::~BINDING() {
  PRINT("Binding " << this->layout.binding << " was destroyed")
  vkUnmapMemory(device.logicalDevice, this->memory);
  vkDestroyBuffer(device.logicalDevice, this->buffer, nullptr);
  vkFreeMemory(device.logicalDevice, this->memory, nullptr);
}
UNIFORM_BUFFER::UNIFORM_BUFFER(DEVICE &device, std::vector<UNIFORM_BUFFER_BINDING>& bindings) : device(device) {
  this->bindings.reserve(16);
  uint32_t currentBinding = 0;
  for (UNIFORM_BUFFER_BINDING &binding : bindings) {
    if (bindings.size() == 16)
      THROW_EXCEPTION("You cannot have more than 16 bindings!");
    this->bindings.emplace_back(device, binding.size, binding.ubo, currentBinding);
    currentBinding++;
  }
  this->create();
}

void UNIFORM_BUFFER::create() {
  std::vector<VkDescriptorSetLayoutBinding> layouts;

  for (auto &binding : bindings)
    layouts.push_back(binding.layout);

  VkDescriptorSetLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, .bindingCount = static_cast<uint32_t>(layouts.size()), .pBindings = layouts.data()};
  CheckVkResult(vkCreateDescriptorSetLayout(device.logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout));

  VkDescriptorPoolSize poolSize{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(bindings.size())};
  VkDescriptorPoolCreateInfo poolInfo{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, .maxSets = 1, .poolSizeCount = 1, .pPoolSizes = &poolSize};

  CheckVkResult(vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPool));

  VkDescriptorSetAllocateInfo allocation{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, .descriptorPool = descriptorPool, .descriptorSetCount = 1, .pSetLayouts = &descriptorSetLayout};

  CheckVkResult(vkAllocateDescriptorSets(device.logicalDevice, &allocation, &descriptorSet));

  std::vector<VkWriteDescriptorSet> writes;
  for (auto &binding : bindings) {
    writes.push_back({.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, .dstSet = descriptorSet, .dstBinding = binding.layout.binding, .descriptorCount = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .pBufferInfo = &binding.bufferInfo});
  }

  vkUpdateDescriptorSets(device.logicalDevice, writes.size(), writes.data(), 0, nullptr);
  PRINT("Succesfully created uniform buffers")
}
void UNIFORM_BUFFER::bind(VkCommandBuffer &commandBuffer, PIPELINE &pipeline, uint32_t set) {
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, set, 1, &descriptorSet, 0, nullptr);
}
UNIFORM_BUFFER::~UNIFORM_BUFFER() {
  vkDestroyDescriptorSetLayout(device.logicalDevice, descriptorSetLayout, nullptr);
  vkDestroyDescriptorPool(device.logicalDevice, descriptorPool, nullptr);
}
} // namespace GFVL
