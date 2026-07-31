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

#include "../../include/GFVL.hpp"
#include <GFVL_core.hpp>
#include <GFVL_definition.hpp>

using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
std::vector<VkValidationFeatureEnableEXT> getEnabledValidationFeatures() {
  std::vector<VkValidationFeatureEnableEXT> enables = {VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT};

#ifdef GFVL_ENABLE_VK_CORE_VALIDATION
  enables.push_back(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);
#endif
#ifdef GFVL_ENABLE_VK_GPU_ASSISTED_VALIDATION
  enables.push_back(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
#endif

  return enables;
}
std::vector<const char *> getEnabledInstanceExtensions() {
  uint32_t instanceExtensionCount = 0;
  const char *const *instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);

  if (instanceExtensions == nullptr)
    THROW_EXCEPTION("No instance extensions found..")

  std::vector<const char *> extensions(instanceExtensions, instanceExtensions + instanceExtensionCount);

#ifdef GFVL_DEBUG_IMPLEMENTATION
  PRINT("Detected instance extensions :");
  for (uint32_t i = 0; i < instanceExtensionCount; i++)
    PRINT("  " << instanceExtensions[i]);
#endif

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
  extensions.push_back("VK_EXT_debug_utils");
#endif
  return extensions;
}
std::vector<const char *> getEnabledLayers() {
  std::vector<const char *> enabledLayers;
#ifdef GFVL_ENABLE_VK_VALIDATION_LAYERS
  enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif
      return enabledLayers;
}
VkInstance INSTANCE::InitializeVkInstance(APPLICATION_INFO applicationInfo) {
  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = applicationInfo.applicationName,
      .applicationVersion = applicationInfo.applicationVersion,
      .pEngineName = "goofyVLib",
      .engineVersion = GFVL_VERSION,
      .apiVersion = VK_API_VERSION_1_4};

  std::vector<VkValidationFeatureEnableEXT> enabledValidationFeatures = getEnabledValidationFeatures();
  std::vector<const char *> enabledInstanceExtensions = getEnabledInstanceExtensions();
  std::vector<const char *> enabledLayers = getEnabledLayers();

  VkValidationFeaturesEXT features = {
      .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
      .enabledValidationFeatureCount = static_cast<uint32_t>(enabledValidationFeatures.size()),
      .pEnabledValidationFeatures = enabledValidationFeatures.data()};

  VkInstanceCreateInfo instanceCreationInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = &features,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
      .ppEnabledLayerNames = enabledLayers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(enabledInstanceExtensions.size()),
      .ppEnabledExtensionNames = enabledInstanceExtensions.data(),
  };

  VkInstance instance;
  GFVL::CheckVkResult(vkCreateInstance(
      &instanceCreationInfo,
      NULL,
      &instance));

  return instance;
}
VkSurfaceKHR INSTANCE::InitializeVkSurface() {
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
    THROW_EXCEPTION(SDL_GetError());
  return surface;
}

std::vector<SHADER> INSTANCE::InitializeShaderStages(std::vector<SHADER_STAGE> &stages) {
  std::vector<SHADER> shaders;
  for (SHADER_STAGE &stage : stages) {
    shaders.emplace_back(device, stage.flags, stage.filename);
  }
  return shaders;
}
} // namespace GFVL