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
#include <vulkan/vulkan.h>

#include "../lib/GFVL_core.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
   VERTEX_LAYOUT::VERTEX_LAYOUT(uint32_t size) {
    this->binding = {
        .binding = 0,
        .stride = size,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
   }
   void VERTEX_LAYOUT::addAttribute(VkFormat format, uint32_t offset) {
     attributes.push_back({.location = static_cast<uint32_t>(attributes.size()),
                           .binding = binding.binding,
                           .format = format,
                           .offset = offset});
   }
}