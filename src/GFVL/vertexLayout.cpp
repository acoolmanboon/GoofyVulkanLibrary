#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
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