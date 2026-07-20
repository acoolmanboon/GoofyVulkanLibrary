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
 * @file GFVL_definition.hpp
 * @brief Defines some helper stuff for GFVL.
 * @details Don't include this. Unless you wanna do some master hacking?
 */
#ifndef GFVL_DEFINITION_HPP
#define GFVL_DEFINITION_HPP
#define PRINT(message) std::cout << "[GFVL] " << message << "\n";
#define THROW_EXCEPTION(reason)                 \
  do {                                          \
    std::ostringstream oss;                     \
    oss << "[GFVL] Error! Reason : " << reason; \
    throw std::runtime_error(oss.str());        \
  } while (0);
#define ASSERTIF(statement, message)               \
  if (statement) {                               \
    std::ostringstream oss;                      \
    oss << "[GFVL] Error! Reason : " << message; \
    throw std::runtime_error(oss.str());         \
  };

#define GFVL_VERSION 1 // internal application name
#define DEBUG_MODE true
#endif