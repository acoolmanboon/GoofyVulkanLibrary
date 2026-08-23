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

#include <GFVL.hpp>
#include <GFVL_core.hpp>
#include <GFVL_definition.hpp>
#include <cstring>

using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
VkDeviceSize Mesh::getIndiceDataSize(Mesh::IndiceDataType indiceDataType, uint32_t indiceCount) {
  VkDeviceSize indiceDataSize = 0;
  switch(indiceDataType) {
    case(Mesh::IndiceDataType::NotDefined):
      return 0;
    case (Mesh::IndiceDataType::UInt16):
      return static_cast<VkDeviceSize>(indiceCount) << 1;
    case (Mesh::IndiceDataType::UInt32):
      return static_cast<VkDeviceSize>(indiceCount) << 2;
    default:
      THROW_EXCEPTION("Attempted to use invalid indice data type of " << static_cast<int>(indiceDataType)); // OH MY GOD BEHIND YOU
  }
}
void *packData(size_t verticeDataSize, void* verticeData, size_t indiceDataSize, void* indiceData) {
  if (indiceDataSize == 0)
    return 0;
  void* data = malloc(verticeDataSize + indiceDataSize);
  memcpy(data, verticeData, verticeDataSize);
  memcpy(reinterpret_cast<uint8_t*>(data) + verticeDataSize, indiceData, indiceDataSize);
  return data;
}
Mesh::Mesh(DEVICE &device, const CreateInfo &createInfo, VkCommandPool commandPool, VmaAllocator allocator) : device_(device),
                                                                                                              indiceDataSize(getIndiceDataSize(
                                                                                                                  createInfo.indiceType,
                                                                                                                  createInfo.indiceCount)),
                                                                                                              packedData(packData(
                                                                                                                createInfo.verticeDataSize,
                                                                                                                createInfo.verticeData,
                                                                                                                (size_t)indiceDataSize,
                                                                                                                createInfo.indiceData
                                                                                                              )),
                                                                                                              meshBuffer_(device, MeshBuffer::CreateInfo{
                                                                                                                                      .size = indiceDataSize + createInfo.verticeDataSize,
                                                                                                                                      .data = (packedData == 0) ? createInfo.verticeData : packedData,
                                                                                                                                      .memoryAllocation = createInfo.memoryAllocation,
                                                                                                                                      .commandPool = commandPool,
                                                                                                                                      .allocator = allocator}),
                                                                                                              verticeCount_(createInfo.verticeCount) 
{
  if (packedData)
    free(packedData);                                                                     
}
VkDeviceSize Mesh::size() const noexcept {
  return this->meshBuffer_.size();
}
uint32_t Mesh::verticeCount() const noexcept {
  return this->verticeCount_;
}
MeshBuffer::MemoryAllocation Mesh::memoryAllocation() const noexcept {
  return this->meshBuffer_.memoryAllocation();
}
Mesh::~Mesh() {
}
} // namespace GFVL
