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
#include <cstring>
#include <vulkan/vulkan.h>

#include "../include/GFVL.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
Mesh::Mesh(DEVICE &device, const CreateInfo &createInfo) : device_(device),
                                                           vertexBuffer_(device, VertexBuffer::CreateInfo{.size = createInfo.size, .data = createInfo.data, .memoryAllocation = createInfo.memoryAllocation}) {
}
size_t Mesh::size() const noexcept {
  return this->vertexBuffer_.size_;
}
VertexBuffer::MemoryAllocation Mesh::memoryAllocation() const noexcept {
  return this->vertexBuffer_.memoryAllocation_;
}
Mesh::~Mesh() {
}
} // namespace GFVL
