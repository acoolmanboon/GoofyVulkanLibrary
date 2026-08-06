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
 * @file GFVL_enumFormatter.hpp
 * @brief A helper to print enum names in GFVL.hpp.
 * @details No need to include this one, it's an internal helper.
 */
#pragma once

#include <GFVL.hpp>
#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>

#define typeString(type_) inline std::string_view enumToString(const type_ &value)

#define returnCase(type_, case_) \
  case (type_::case_):           \
    return #case_;               \
    break;

#define returnError(type_, value) \
  default:                        \
    THROW_EXCEPTION("Attempted to convert raw value of " << static_cast<int>(value) << " of enum type " #type_ " but the raw value is not a valid enum value."); // ONLY use this if you have defined all possible cases

#define returnDefault (type_, value) default : return "";

/*
example

typeString(type__) {
  switch (value) {
    returnCase(type__, unknown);
    returnError(type__, value);
  }
}
*/
namespace GFVL {
typeString(VertexBuffer::MemoryAllocation) {
  switch (value) {
    returnCase(VertexBuffer::MemoryAllocation, HostVisible);
    returnCase(VertexBuffer::MemoryAllocation, DeviceOnly);
    returnError(VertexBuffer::MemoryAllocation, value);
  }
}
typeString(VkObjectType) {
  switch (value) {
    returnCase(VkObjectType, VK_OBJECT_TYPE_UNKNOWN);
    returnCase(VkObjectType, VK_OBJECT_TYPE_INSTANCE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_PHYSICAL_DEVICE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_DEVICE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_QUEUE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_SEMAPHORE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_COMMAND_BUFFER);
    returnCase(VkObjectType, VK_OBJECT_TYPE_FENCE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_DEVICE_MEMORY);
    returnCase(VkObjectType, VK_OBJECT_TYPE_BUFFER);
    returnCase(VkObjectType, VK_OBJECT_TYPE_IMAGE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_EVENT);
    returnCase(VkObjectType, VK_OBJECT_TYPE_QUERY_POOL);
    returnCase(VkObjectType, VK_OBJECT_TYPE_BUFFER_VIEW);
    returnCase(VkObjectType, VK_OBJECT_TYPE_IMAGE_VIEW);
    returnCase(VkObjectType, VK_OBJECT_TYPE_SHADER_MODULE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_PIPELINE_CACHE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_PIPELINE_LAYOUT);
    returnCase(VkObjectType, VK_OBJECT_TYPE_RENDER_PASS);
    returnCase(VkObjectType, VK_OBJECT_TYPE_PIPELINE);
    returnCase(VkObjectType, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT);
    returnCase(VkObjectType, VK_OBJECT_TYPE_SAMPLER);
    returnCase(VkObjectType, VK_OBJECT_TYPE_DESCRIPTOR_POOL);
    returnCase(VkObjectType, VK_OBJECT_TYPE_DESCRIPTOR_SET);
    returnCase(VkObjectType, VK_OBJECT_TYPE_FRAMEBUFFER);
    returnCase(VkObjectType, VK_OBJECT_TYPE_COMMAND_POOL);
    returnError(VkObjectType, value);
  }
}

} // namespace GFVL
