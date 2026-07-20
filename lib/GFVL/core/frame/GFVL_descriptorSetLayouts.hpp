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
 * @file GFVL_uniformBuffer.hpp
 * @brief PLACEHOLDER
 * @details Don't include this. Unless you wanna do some master hacking?
 */
#ifndef GFVL_FRAME_HPP
#define GFVL_FRAME_HPP
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "../../lib/GFVL_core.hpp"
#include "../../lib/vk_mem_alloc.h"

/*
instance(InitializeVkInstance(applicationInfo)),
window(SDL_CreateWindow(applicationInfo.applicationName, applicationInfo.width, applicationInfo.height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE)),
surface(InitializeVkSurface(this->instance, this->window)),
device(this->instance, this->surface, applicationInfo.preferredGPU),
swapchain(this->device, this->window, this->surface),
renderPass(this->device, this->swapchain),
uniformBuffer(this->device, bindings),
shaderStages(InitializeShaderStages(device, stages)),
pipeline(this->device, this->swapchain, layout, this->shaderStages, this->renderPass, {this->uniformBuffer.descriptorSetLayout}),
framebuffer(this->device, this->swapchain, this->renderPass),
commandPool(this->device, this->framebuffer),
maxFramesInFlight(applicationInfo.maxFramesInFlight) {
*/
namespace GFVL {
/**
 * @struct UniformBufferBinding
 * @brief Defines a binding in a uniform buffer.
 */
struct UniformBufferBinding {
  size_t size = 0;                                               ///< Size of the data to be passed to shader in bytes.
  uint32_t binding = 0;                                          ///< The binding that will be passed to your shader.
  uint32_t arrayCount = 1;                                       ///< If you are passing an array of data, for example a[1024] to shader, set this to 1024. Otherwise, leave this empty or set it to 1 to mark it as not being an array.
  VkShaderStageFlags shaderStage = VK_SHADER_STAGE_ALL_GRAPHICS; ///< What shaders can access this uniform buffer. Does not have to be set, default value will set to be accessible by all shaders. However, it is recommended to make them shader-specific.
  void *ubo = nullptr;                                           ///< Pointer to your data, this will be read automatically by the engine
  bool needsUpdate = false;                                      ///< When true, the engine will update its internal values. You need to set this to true after changing data
};

/**
 * @brief
 *
 */
class DescriptorSetLayout {
public: 
  VkDescriptorSetLayout descriptorSetLayout;

  DescriptorSetLayout(DEVICE &device, std::vector<UniformBufferBinding> &bindings) : device(device),
                                                                                     bindings(bindings) {
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
    descriptorSetLayoutBindings.reserve(this->bindings.size());

    for (const UniformBufferBinding &binding : this->bindings) {
      ASSERTIF(binding.size == 0, "Binding size must not be 0 in uniform buffer binding!")
      ASSERTIF(binding.ubo == nullptr, "Binding UBO pointer must not be nullptr in uniform buffer binding!")
      descriptorSetLayoutBindings.emplace_back(VkDescriptorSetLayoutBinding{
          .binding = binding.binding,
          .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = binding.arrayCount,
          .stageFlags = binding.shaderStage,
          .pImmutableSamplers = nullptr});
      
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size()),
      .pBindings = descriptorSetLayoutBindings.data(),
    };

    CheckVkResult2(
      vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout),
      "Failed to create descriptor set layout in Frame creation!");
  }
  ~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(device.logicalDevice, descriptorSetLayout, nullptr)
  }

private:
  DEVICE &device;
  std::vector<UniformBufferBinding> &bindings;
};
} // namespace GFVL
#endif