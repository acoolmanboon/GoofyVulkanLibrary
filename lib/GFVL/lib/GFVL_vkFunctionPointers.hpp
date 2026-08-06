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
 * @file GFVL_vkFunctionPointers.hpp
 * @brief A helper to cache function pointers in Vulkan.
 * @details No need to include this one, it's an internal helper.
 */

// i really dont even know what to call this script
#pragma once

#include <GFVL.hpp>
#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>

#define definePointer(name_) inline static PFN_##name_ name_
#define initializePointer(name_, instance_)                                        \
  name_ = reinterpret_cast<PFN_##name_>(vkGetInstanceProcAddr(instance_, #name_)); \
  if (name_ == nullptr)                                                            \
    throw std::runtime_error("Failed to load function pointer PFN_" #name_ "!, are you sure it exists?")

namespace GFVL {
struct VulkanFunctionPointers {
#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
  definePointer(vkCreateDebugUtilsMessengerEXT);
  definePointer(vkSetDebugUtilsObjectNameEXT);
#endif

  static void initialize(VkInstance instance) {
#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
    initializePointer(vkCreateDebugUtilsMessengerEXT, instance);
    initializePointer(vkSetDebugUtilsObjectNameEXT, instance);
#endif
  }
};
} // namespace GFVL
