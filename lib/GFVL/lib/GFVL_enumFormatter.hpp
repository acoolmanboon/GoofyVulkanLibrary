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
#include <GFVL_core.hpp>
#include <GFVL_definition.hpp>


#define returnCase(type_, case_) \
  case (type_::case_):           \
    return #case_;               \
    break;

#define returnError(type_, value) \
  default:                        \
    THROW_EXCEPTION("Attempted to convert raw value of " << static_cast<int>(value) << " of enum type " #type_ " but the raw value is not a valid enum value.");

namespace GFVL {
inline std::string_view enumToString(const VertexBuffer::MemoryAllocation &value) {
  switch (value) {
    returnCase(VertexBuffer::MemoryAllocation, HostVisible);
    returnCase(VertexBuffer::MemoryAllocation, HostVisibleOpportunistic);
    returnCase(VertexBuffer::MemoryAllocation, DeviceOnly);
    returnError(VertexBuffer::MemoryAllocation, value);
  } 
}
} // namespace GFVL
