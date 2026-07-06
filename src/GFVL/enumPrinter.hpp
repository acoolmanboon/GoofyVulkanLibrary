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

#include "GFVL.hpp"
#define ret(type_, case_) \
  case (type_::case_):    \
    return #case_;        \
    break;
#define reterr(type_, value)                                                                                                   \
  std::ostringstream oss;                                                                                                      \
  oss << "[GFVL] Error! Attempted to convert enum of type " << #type_ << " into string, value is " << static_cast<int>(value); \
  throw std::runtime_error(oss.str());

namespace GFVL {
inline std::string_view enumToString(const VertexBuffer::MemoryAllocation &value) {
  switch (value) {
    ret(VertexBuffer::MemoryAllocation, HostVisible)
    ret(VertexBuffer::MemoryAllocation, DeviceOnly)
    reterr(VertexBuffer::Type, value)
  }
  return "";
}
} // namespace GFVL

#endif