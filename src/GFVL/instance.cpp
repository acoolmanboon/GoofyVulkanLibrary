#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
VkSurfaceKHR InitializeVkSurface(VkInstance instance, SDL_Window* window) {
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
    THROW_EXCEPTION(SDL_GetError());
  return surface;
}
VkInstance InitializeVkInstance(APPLICATION_INFO applicationInfo) {
  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = applicationInfo.applicationName,
      .applicationVersion = applicationInfo.applicationVersion,
      .pEngineName = "goofyVLib",
      .engineVersion = GOOFYVLIB_ITERATION,
      .apiVersion = VK_API_VERSION_1_3};

  uint32_t instanceExtensionCount = 0;
  const char *const *instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);
  if (instanceExtensions == NULL)
    THROW_EXCEPTION("No instance extensions found..")

  if (DEBUG_MODE) { // optionally print the info
    PRINT("Detected instance extensions :")
    for (uint32_t i = 0; i < instanceExtensionCount; i++)
      PRINT("  " << instanceExtensions[i])
  }
  const char *validationLayer = "VK_LAYER_KHRONOS_validation";

  VkInstanceCreateInfo instanceCreationInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = NULL,
      .flags = 0,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = 1,
      .ppEnabledLayerNames = &validationLayer,
      .enabledExtensionCount = instanceExtensionCount,
      .ppEnabledExtensionNames = instanceExtensions,
  };

  VkInstance instance;
  GFVL::CheckVkResult(vkCreateInstance(
      &instanceCreationInfo,
      NULL,
      &instance));

  return instance;
}

std::vector<SHADER> InitializeShaderStages(DEVICE& device, std::vector<SHADER_STAGE> &stages) {
  std::vector<SHADER> shaders;
  for (SHADER_STAGE& stage : stages) {
    shaders.emplace_back(device, stage.flags, stage.filename);
  }
  return shaders;
}

INSTANCE::INSTANCE(APPLICATION_INFO applicationInfo, VERTEX_LAYOUT &layout, std::vector<UNIFORM_BUFFER_BINDING> &bindings, std::vector<SHADER_STAGE> &stages) : instance(InitializeVkInstance(applicationInfo)),
                                                                                                                                                                window(SDL_CreateWindow(applicationInfo.applicationName, applicationInfo.width, applicationInfo.height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE)),
                                                                                                                                                                surface(InitializeVkSurface(this->instance, this->window)),
                                                                                                                                                                device(this->instance, this->surface, applicationInfo.preferredGPU),
                                                                                                                                                                swapchain(this->device, this->window, this->surface),
                                                                                                                                                                renderPass(this->device, this->swapchain),
                                                                                                                                                                uniformBuffer(this->device, bindings),
                                                                                                                                                                shaderStages(InitializeShaderStages(device, stages)),
                                                                                                                                                                pipeline(this->device, this->swapchain, layout, this->shaderStages, this->renderPass, {this->uniformBuffer.descriptorSetLayout}),
                                                                                                                                                                framebuffer(this->device, this->swapchain, this->renderPass),
                                                                                                                                                                commandPool(this->device, this->framebuffer)
{

}
INSTANCE::~INSTANCE() {

}
}
