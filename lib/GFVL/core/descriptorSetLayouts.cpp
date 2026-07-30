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
 * @file GFVL_descriptorSetLayouts.hpp
 * @brief Handles Vulkan descriptor layouts.
 * @details Don't include this. Unless+ you wanna do some master hacking?
 */

#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>

namespace GFVL {
/**
 * @brief Construct a new Descriptor Set Layout:: Descriptor Set Layout object
 * 
 * @param device A GFVL device reference.
 * @param bindings The list of bindings to create a descriptor set layout for
 */
DescriptorSetLayout::DescriptorSetLayout(DEVICE &device, std::vector<UniformBufferBinding> &bindings) : device(device),
                                                                                                        bindings(bindings) {
  std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
  descriptorSetLayoutBindings.reserve(this->bindings.size());

  for (const UniformBufferBinding &binding : this->bindings) {
    ASSERTIF(binding.ubo == nullptr, "UniformBufferBinding ubo cannot be nullptr!")
    ASSERTIF(binding.size == 0, "UniformBufferBinding size cannot be 0 bytes!")
    ASSERTIF(binding.shaderStage == 0, "UniformBufferBinding shader stage has no flags! This should not be possible unless it is explicitly initialized as such.")
    ASSERTIF(binding.arrayCount == 0, "UniformBufferBinding array count is 0. This should not be possible unless you explicitly initialized it to 0.")

    descriptorSetLayoutBindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = binding.binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = binding.arrayCount,
        .stageFlags = binding.shaderStage,
        .pImmutableSamplers = nullptr});
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size()),
      .pBindings = descriptorSetLayoutBindings.data(),
  };

  CheckVkResult2(
      vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout),
      "Failed to create descriptor set layout in DescriptorSetLayout creation!");

  #ifdef GFVL_ENABLE_VK_DEBUG_EXTENSION
  VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
    .pNext = nullptr,
    .objectType = VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
    .objectHandle = reinterpret_cast<uint64_t>(&descriptorSetLayout),
    .pObjectName = "DescriptorSetLayout classes vkDescriptorSetLayout object"
  };
  CheckVkResult2(
      vkSetDebugUtilsObjectNameEXT(device.logicalDevice, &debugUtilsObjectNameInfo),
      "Failed to set debug utils name for Descriptor Set Layout object!");
  #endif
}
/**
 * @brief Destroy the Descriptor Set Layout:: Descriptor Set Layout object
 */
DescriptorSetLayout::~DescriptorSetLayout() {
  //vkDestroyDescriptorSetLayout(device.logicalDevice, descriptorSetLayout, nullptr);
}
} // namespace GFVL