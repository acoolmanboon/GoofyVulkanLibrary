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
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <vulkan/vulkan.h>

#include "../lib/GFVL_core.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
SHADER::SHADER(DEVICE &device, VkShaderStageFlagBits stage, const char *filename) : device(device) {
  this->stage = stage;
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
      THROW_EXCEPTION("failed to open file!");

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  VkShaderModuleCreateInfo shaderCreationInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = buffer.size(),
      .pCode = reinterpret_cast<const uint32_t *>(buffer.data())};
  CheckVkResult(vkCreateShaderModule(device.logicalDevice, &shaderCreationInfo, nullptr, &this->shaderModule));
  PRINT("succesfully created shader!")
}
SHADER::SHADER(SHADER &&other) noexcept : device(other.device), shaderModule(other.shaderModule), stage(other.stage) {
  other.shaderModule = VK_NULL_HANDLE; // this just prevents the vulkan shader module from being destroyed
}
SHADER::~SHADER() {
  if (shaderModule != VK_NULL_HANDLE) {
    vkDestroyShaderModule(device.logicalDevice, shaderModule, nullptr);
  }
}
}
