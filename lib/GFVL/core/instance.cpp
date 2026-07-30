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

#include <GFVL_core.hpp>
#include <GFVL_definition.hpp>
#include "../include/GFVL.hpp"

using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
VkInstance InitializeVkInstance(APPLICATION_INFO applicationInfo) {
  CheckVkResult2(
    volkInitialize(),
    "Failed to initialize Volk!");
  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = applicationInfo.applicationName,
      .applicationVersion = applicationInfo.applicationVersion,
      .pEngineName = "goofyVLib",
      .engineVersion = GFVL_VERSION,
      .apiVersion = VK_API_VERSION_1_4};

  uint32_t instanceExtensionCount = 0;
  const char *const *instanceExtensions = SDL_Vulkan_GetInstanceExtensions(&instanceExtensionCount);
  if (instanceExtensions == NULL)
    THROW_EXCEPTION("No instance extensions found..")

  if (DEBUG_MODE) { // optionally print the info
    PRINT("Detected instance extensions :");
    for (uint32_t i = 0; i < instanceExtensionCount; i++)
      PRINT("  " << instanceExtensions[i]);
  }
  const char *validationLayer = "VK_LAYER_KHRONOS_validation";

  VkValidationFeatureEnableEXT enables[] = {
      VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
      VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};

  VkValidationFeaturesEXT features{
      .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
      .enabledValidationFeatureCount = 2,
      .pEnabledValidationFeatures = enables};

  VkInstanceCreateInfo instanceCreationInfo = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = &features,
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

  volkLoadInstance(instance);

  return instance;
}
VkSurfaceKHR InitializeVkSurface(VkInstance instance, SDL_Window *window) {
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
    THROW_EXCEPTION(SDL_GetError());
  return surface;
}

std::vector<SHADER> InitializeShaderStages(DEVICE &device, std::vector<SHADER_STAGE> &stages) {
  std::vector<SHADER> shaders;
  for (SHADER_STAGE &stage : stages) {
    shaders.emplace_back(device, stage.flags, stage.filename);
  }
  return shaders;
}

Mesh& INSTANCE::createMesh(Mesh::CreateInfo createInfo) {
  return meshesToRender.emplace_back(device, createInfo, commandPool);
}
void INSTANCE::setMouseLock(bool mouseLock) {
  SDL_SetWindowRelativeMouseMode(window, mouseLock);
}
INSTANCE::INSTANCE(APPLICATION_INFO applicationInfo, VERTEX_LAYOUT &layout, std::vector<UniformBufferBinding> &bindings, std::vector<SHADER_STAGE> &stages) :   instance(InitializeVkInstance(applicationInfo)),
                                                                                                                                                                window(SDL_CreateWindow(applicationInfo.applicationName, applicationInfo.width, applicationInfo.height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE)),
                                                                                                                                                                surface(InitializeVkSurface(this->instance, this->window)),
                                                                                                                                                                device(this->instance, this->surface, applicationInfo.preferredGPU),
                                                                                                                                                                swapchain(this->device, this->window, this->surface),
                                                                                                                                                                renderPass(this->device, this->swapchain),
                                                                                                                                                                shaderStages(InitializeShaderStages(device, stages)), 
                                                                                                                                                                bindings(bindings),
                                                                                                                                                                descriptorSetLayout(device, bindings),
                                                                                                                                                                pipeline(this->device, this->swapchain, layout, this->shaderStages, this->renderPass, {descriptorSetLayout.descriptorSetLayout}),
                                                                                                                                                                framebuffer(this->device, this->swapchain, this->renderPass),
                                                                                                                                                                maxFramesInFlight(applicationInfo.maxFramesInFlight) {
  VmaAllocatorCreateInfo allocatorCreateinfo{
      .physicalDevice = device.physicalDevice,
      .device = device.logicalDevice,
      .instance = instance,
      .vulkanApiVersion = VK_API_VERSION_1_4,
  };
  vmaCreateAllocator(&allocatorCreateinfo, &vmaAllocator);

  this->imagesInFlightFence = std::vector<VkFence>(this->swapchain.imageCount);

  frames.reserve(applicationInfo.maxFramesInFlight);
  for (int i = 0; i < applicationInfo.maxFramesInFlight; i++) {
    // Frame(DEVICE &device, VmaAllocator allocator, VkDescriptorSetLayout descriptorSetLayout, const std::vector<UniformBufferBinding> &bindings);
    frames.emplace_back(device, vmaAllocator, descriptorSetLayout.descriptorSetLayout, bindings);
  }

  renderFinishedSemaphores.reserve(swapchain.imageCount);
  for (int i = 0; i < swapchain.imageCount; i++) {
    renderFinishedSemaphores.emplace_back(device);
  }

  SDL_GetWindowSizeInPixels(this->window, &w, &h);
  this->aspectRatio = static_cast<float>(this->w) / static_cast<float>(this->h);

  VkCommandPoolCreateInfo commandPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = device.graphicsFamilyIndex};

  CheckVkResult2(
      vkCreateCommandPool(device.logicalDevice, &commandPoolCreateInfo, nullptr, &commandPool),
      "Failed to create command pool for Instance!");
}
void INSTANCE::pollInputs() {

  this->inputState.mouseState.xDelta = 0;
  this->inputState.mouseState.yDelta = 0;
  this->inputState.mouseState.moved = false;
  SDL_Event event;
  for (KeyState &state : this->inputState.keycodeStates) {
    state.isRepeated = true;
  }
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT)
      this->running = false;

    if (event.type == SDL_EVENT_WINDOW_RESIZED || event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
      this->framebufferResized = true;

    if (event.type == SDL_EVENT_KEY_DOWN) {
      this->inputState.keycodeStates[static_cast<size_t>(event.key.scancode)] = {.event = KeyEvent::Down, .isRepeated = false};
    }

    if (event.type == SDL_EVENT_KEY_UP) {
      this->inputState.keycodeStates[static_cast<size_t>(event.key.scancode)] = {.event = KeyEvent::Up, .isRepeated = false};
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
      this->inputState.mouseButtonStates[static_cast<size_t>(event.button.button)] = {.event = KeyEvent::Down, .clicks = event.button.clicks};
    }
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
      this->inputState.mouseButtonStates[static_cast<size_t>(event.button.button)] = {.event = KeyEvent::Up, .clicks = event.button.clicks};
    }

    if (event.type == SDL_EVENT_MOUSE_MOTION) {
      this->inputState.mouseState = {.x = event.motion.x, .y = event.motion.y, .xDelta = event.motion.xrel, .yDelta = event.motion.yrel, .moved = true};
    }
  }
}
void INSTANCE::frame() {
  Frame &currentFrame = frames[currentFrameIndex];
  currentFrame.updateUniformBuffers();
  if (framebufferResized) {
    vkDeviceWaitIdle(this->device.logicalDevice);
    this->swapchain.recreate(this->window, this->surface);
    this->framebuffer.recreate(this->swapchain, this->renderPass);
    this->swapchain.imageCount = this->swapchain.images.size();

    imagesInFlightFence = std::vector<VkFence>(this->swapchain.imageCount, 0);
    renderFinishedSemaphores.clear();
    renderFinishedSemaphores.reserve(swapchain.imageCount);
    for (int i = 0; i < swapchain.imageCount; ++i)
      renderFinishedSemaphores.emplace_back(device);

    framebufferResized = false;
    SDL_GetWindowSizeInPixels(this->window, &this->w, &this->h);
    aspectRatio = static_cast<float>(this->w) / static_cast<float>(this->h);
  }

  vkWaitForFences(this->device.logicalDevice, 1, &currentFrame.gpuFinishedFence.fence, VK_TRUE, UINT64_MAX);
  vkResetFences(this->device.logicalDevice, 1, &currentFrame.gpuFinishedFence.fence);

  uint32_t imageIndex;
  CheckVkResult(vkAcquireNextImageKHR(this->device.logicalDevice, this->swapchain.swapchain, UINT64_MAX, currentFrame.imageAvailableSemaphore.semaphore, VK_NULL_HANDLE, &imageIndex));
  if (imagesInFlightFence[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(
        this->device.logicalDevice,
        1,
        &imagesInFlightFence[imageIndex],
        VK_TRUE,
        UINT64_MAX);
  }

  imagesInFlightFence[imageIndex] = currentFrame.gpuFinishedFence.fence;
  CheckVkResult(vkResetCommandBuffer(currentFrame.commandBuffer, 0));

  VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  CheckVkResult(vkBeginCommandBuffer(currentFrame.commandBuffer, &beginInfo));

  VkClearValue clearColor{.color = {0.05f, 0.05f, 0.05f, 1.0f}};
  VkClearValue clearValues[2]{};
  clearValues[0].color = {{0.05f, 0.05f, 0.05f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};


  VkRenderPassBeginInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = this->renderPass.renderPass,
      .framebuffer = this->framebuffer.framebuffers[imageIndex],
      .renderArea = {
          .offset = {0, 0},
          .extent = this->swapchain.extent},
      .clearValueCount = 2,
      .pClearValues = clearValues};

  vkCmdBeginRenderPass(currentFrame.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(currentFrame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline.pipeline);

  vkCmdBindDescriptorSets(currentFrame.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipelineLayout, 0, 1, &currentFrame.descriptorSet, 0, nullptr);
  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = (float)swapchain.extent.width,
      .height = (float)swapchain.extent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
  vkCmdSetViewport(currentFrame.commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{.offset = {0, 0}, .extent = swapchain.extent}; // this just cuts off rendering if not in swapchain
  vkCmdSetScissor(currentFrame.commandBuffer, 0, 1, &scissor);
  VkDeviceSize offsets[] = {0};

  for (const GFVL::Mesh &mesh : meshesToRender) {
    VkDeviceSize offset = 0;

    vkCmdBindVertexBuffers(
        currentFrame.commandBuffer,
        0,
        1,
        &mesh.vertexBuffer_.buffer_,
        &offset);

    vkCmdDraw(
        currentFrame.commandBuffer,
        mesh.verticeCount(),
        1,
        0,
        0);
  }

  vkCmdEndRenderPass(currentFrame.commandBuffer);

  CheckVkResult(vkEndCommandBuffer(currentFrame.commandBuffer));

  VkSemaphore waitSemaphores[] = {currentFrame.imageAvailableSemaphore.semaphore};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[imageIndex].semaphore};

  VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = waitSemaphores,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &currentFrame.commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = signalSemaphores};

  CheckVkResult(vkQueueSubmit(this->device.graphicsQueue, 1, &submitInfo, currentFrame.gpuFinishedFence.fence));

  VkSwapchainKHR swapchains[] = {this->swapchain.swapchain};

  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signalSemaphores,
      .swapchainCount = 1,
      .pSwapchains = swapchains,
      .pImageIndices = &imageIndex};

  CheckVkResult(vkQueuePresentKHR(this->device.graphicsQueue, &presentInfo));

  vkWaitForFences(
      this->device.logicalDevice,
      1,
      &currentFrame.gpuFinishedFence.fence,
      VK_TRUE,
      UINT64_MAX);

  for (UniformBufferBinding &binding : bindings) {
    binding.hasUpdated = false;
  }
  this->currentFrameIndex = (this->currentFrameIndex + 1) % this->maxFramesInFlight;
}
INSTANCE::~INSTANCE() {
  vkDeviceWaitIdle(device.logicalDevice);
}
} // namespace GFVL
