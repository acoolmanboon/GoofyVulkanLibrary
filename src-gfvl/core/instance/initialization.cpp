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
#include <GFVL_vkFunctionPointers.hpp>
#include <GFVL_enumFormatter.hpp>
#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>
#include <cstring>

using namespace GFVL;

// USER-DEFINED STUFF
VkBool32 vulkanDebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *userData) {
#ifndef GFVL_DEBUG_IMPLEMENTATION
  if (severity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    return VK_FALSE;
  }
#endif

#ifdef GFVL_SILENCE_GENERAL_MESSAGES
  if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT || messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
    return VK_FALSE;
#endif

  PRINT("Vulkan debug message : ");
  switch (severity) {
  case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT):
    PRINT(" Severity : Verbose");
    break;
  case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT):
    PRINT(" Severity : Info");
    break;
  case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT):
    PRINT(" Severity : Warning");
    break;
  case (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT):
    PRINT(" Severity : Error");
    break;
  default:
    THROW_WARNING("Invalid severity flag bits in vulkan debug callback! Flags hex : " << std::hex << severity << std::dec);
  }

  switch (messageType) {
  case (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT):
    PRINT(" Message type : General");
    break;
  case (VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT):
    PRINT(" Message type : Validation");
    break;
  case (VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT):
    PRINT(" Message type : Performance");
    break;
  default:
    THROW_WARNING("Invalid message type flag bits in vulkan debug callback! Flags hex : " << std::hex << messageType << std::dec);
  }
  if (callbackData->pMessageIdName != NULL)
    PRINT(" Message ID name : " << callbackData->pMessageIdName);

  PRINT(" Message ID number : 0x" << std::hex << callbackData->messageIdNumber << std::dec);

  if (callbackData->pMessage != NULL)
    PRINT(" Message : " << callbackData->pMessage);

  if (callbackData->objectCount != 0) {
    PRINT(" Involved objects (in order of importance)");
    for (uint32_t i = 0; i < callbackData->objectCount; i++) {
      const VkDebugUtilsObjectNameInfoEXT currentObject = callbackData->pObjects[i];
      PRINT("   Object " << i);
      PRINT("     Type : " << enumToString(currentObject.objectType) << "(" << currentObject.objectType << ")");
      PRINT("     Handle : 0x" << std::hex << currentObject.objectHandle << std::dec);
      if (currentObject.pObjectName != NULL)
        PRINT("     Object Name " << currentObject.pObjectName);
    }
  }
  std::cout << " \n";

  return VK_FALSE;
}
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
  uint32_t SDLinstanceExtensionCount = 0;
  const char *const *SDLinstanceExtensions = SDL_Vulkan_GetInstanceExtensions(&SDLinstanceExtensionCount);
  ASSERTIF(SDLinstanceExtensions == nullptr, "Failed to get SDL instance extensions! However, " << SDLinstanceExtensionCount << " instance extensions were detected. (If this is not zero, something is wrong)");
  std::vector<const char *> extensions(SDLinstanceExtensions, SDLinstanceExtensions + SDLinstanceExtensionCount);

  uint32_t availableInstanceExtensionCount = 0;
  CheckVkResult2(
      vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionCount, nullptr),
      "Failed to enumerate available instance extension count!");

  std::vector<VkExtensionProperties> availableInstanceExtensions(availableInstanceExtensionCount);
  CheckVkResult2(
      vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionCount, availableInstanceExtensions.data()),
      "Failed to enumerate available instance extensions!");

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
  bool foundVkDebugUtilsExtension = false;
#endif

  for (const VkExtensionProperties &availableExtension : availableInstanceExtensions) {
#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
    if (strcmp(availableExtension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      foundVkDebugUtilsExtension = true;
    }
#endif
  }

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
  if (foundVkDebugUtilsExtension == false) {
    THROW_WARNING("GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION is on, but the extension was not supported.");
  }
#endif

#ifdef GFVL_DEBUG_IMPLEMENTATION
  PRINT("SDL instance extensions :");
  for (uint32_t i = 0; i < SDLinstanceExtensionCount; i++)
    PRINT("  " << SDLinstanceExtensions[i]);
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
  if (!SDL_Init(SDL_INIT_VIDEO))
    THROW_EXCEPTION(SDL_GetError());

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

  void *debugUtilsMessengerCreateInfoPointer = nullptr;

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
  VkDebugUtilsMessengerEXT debugUtilsMessenger;

  VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = 0,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = vulkanDebugUtilsMessengerCallback,
      .pUserData = nullptr};

  debugUtilsMessengerCreateInfoPointer = &debugUtilsMessengerCreateInfo;

#endif

  VkValidationFeaturesEXT features = {
      .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
      .pNext = nullptr,
      .enabledValidationFeatureCount = static_cast<uint32_t>(enabledValidationFeatures.size()),
      .pEnabledValidationFeatures = enabledValidationFeatures.data(),
      .disabledValidationFeatureCount = 0,
      .pDisabledValidationFeatures = nullptr};

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
  CheckVkResult2(
      vkCreateInstance(&instanceCreationInfo, NULL, &instance),
      "Failed to create Vulkan instance!");

  VulkanFunctionPointers::initialize(instance);

  #ifdef  GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
  CheckVkResult2(
      VulkanFunctionPointers::vkCreateDebugUtilsMessengerEXT(
          instance,
          &debugUtilsMessengerCreateInfo,
          nullptr,
          &debugUtilsMessenger
          ),
      "Failed to create debug utils messenger!");
  #endif
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