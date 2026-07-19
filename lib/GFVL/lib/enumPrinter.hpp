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
 * @file enumPrinter.hpp
 * @brief A helper to print enum names in GFVL.hpp.
 * @details No need to include this one, it's an internal helper.
 */
#ifndef ENUMPRINTER_HPP
#define ENUMPRINTER_HPP
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "../include/GFVL.hpp"
#define ret(type_, case_) \
  case (type_::case_):    \
    return #case_;        \
    break;
#define reterr(type_, value)                                                                                                   \
  std::ostringstream oss;                                                                                                      \
  oss << "[GFVL] Error! Attempted to convert enum of type " << #type_ << " into string, value is " << static_cast<int>(value); \
  throw std::runtime_error(oss.str())

namespace GFVL {
inline std::string_view enumToString(const VertexBuffer::MemoryAllocation &value) {
  switch (value) {
    ret(VertexBuffer::MemoryAllocation, HostVisible);
    ret(VertexBuffer::MemoryAllocation, HostVisibleOpportunistic);
    ret(VertexBuffer::MemoryAllocation, DeviceOnly);
    reterr(VertexBuffer::Type, value);
  }
  return "";
}
} // namespace GFVL

#endif