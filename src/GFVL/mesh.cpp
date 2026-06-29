#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstring>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
Mesh::Mesh(DEVICE &device, const CreateInfo &createInfo) : device(device), size_(createInfo.size), vertexBuffer_(device, VertexBuffer::CreateInfo{.size = createInfo.size, .data = createInfo.data, .type = createInfo.type, .commandBuffer = createInfo.commandBuffer}) {
}
size_t Mesh::size() const {
  return this->size_;
}
const VertexBuffer &Mesh::vertexBuffer() const {
  return this->vertexBuffer_;
}
Mesh::~Mesh() {
}
} // namespace GFVL
