#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstring>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
MESH::MESH(size_t vertice_size, uint32_t vertice_count) : vertice_size(vertice_size), vertice_count(vertice_count) {
  this->data = malloc(vertice_count * vertice_size);
  this->mesh_size = vertice_count * vertice_size;
}
MESH::~MESH() {
    free(this->data);
}
}
