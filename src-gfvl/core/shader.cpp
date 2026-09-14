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
#include <GFVL_vkFunctionPointers.hpp>


using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
  Shader::Shader(Device &device, CreateInfo createInfo) : device_(device), stage_(createInfo.stage) {
    std::ifstream file(createInfo.fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        THROW_EXCEPTION("failed to open file at path " << createInfo.fileName);

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo shaderCreationInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = buffer.size(),
        .pCode = reinterpret_cast<const uint32_t *>(buffer.data())};

    CheckVkResult2(
      vkCreateShaderModule(device.logicalDevice, &shaderCreationInfo, nullptr, &shaderModule_),
      "Failed to create shader module!");

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
    VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = VK_OBJECT_TYPE_SHADER_MODULE,
        .objectHandle = reinterpret_cast<uint64_t>(shaderModule_),
        .pObjectName = "VkShaderModule of Shader object"};
    CheckVkResult2(
        VulkanFunctionPointers::vkSetDebugUtilsObjectNameEXT(device.logicalDevice, &debugUtilsObjectNameInfo),
        "Failed to set debug utils name for VkShaderModule of Shader object!");
#endif
  }

  Shader::Shader(Shader &&other) noexcept : device_(other.device_), shaderModule_(other.shaderModule_), stage_(other.stage_) {
    other.shaderModule_ = VK_NULL_HANDLE;
  }

  Shader& Shader::operator=(Shader &&other) {
    ASSERTIF(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy shader with different devices");
    if (this == &other)
      return *this;

    this->stage_ = other.stage_;

    if (this->shaderModule_ != VK_NULL_HANDLE)
      vkDestroyShaderModule(this->device_.logicalDevice, this->shaderModule_, nullptr);

    this->shaderModule_ = other.shaderModule_;
    other.shaderModule_ = VK_NULL_HANDLE;
    return *this;
  }

  Shader::~Shader() {
    if (shaderModule_ != VK_NULL_HANDLE) 
      vkDestroyShaderModule(device_.logicalDevice, shaderModule_, nullptr);
  }
}
