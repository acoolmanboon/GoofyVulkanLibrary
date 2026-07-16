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
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "../include/GFVL.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
VkSurfaceKHR InitializeVkSurface(VkInstance instance, SDL_Window *window) {
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
      .engineVersion = GFVL_VERSION,
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

std::vector<SHADER> InitializeShaderStages(DEVICE &device, std::vector<SHADER_STAGE> &stages) {
  std::vector<SHADER> shaders;
  for (SHADER_STAGE &stage : stages) {
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
                                                                                                                                                                commandPool(this->device, this->framebuffer),
                                                                                                                                                                maxFramesInFlight(applicationInfo.maxFramesInFlight) {
  this->imageAvailableSemaphore.reserve(this->maxFramesInFlight);
  for (uint8_t i = 0; i < this->maxFramesInFlight; i++)
    this->imageAvailableSemaphore.emplace_back(this->device);

  this->renderFinishedSemaphore.reserve(this->swapchain.imageCount);
  for (uint8_t i = 0; i < this->swapchain.imageCount; i++)
    this->renderFinishedSemaphore.emplace_back(this->device);

  this->inFlightFence.reserve(this->maxFramesInFlight);
  for (uint8_t i = 0; i < this->maxFramesInFlight; i++)
    inFlightFence.emplace_back(this->device, VK_FENCE_CREATE_SIGNALED_BIT);

  this->imagesInFlightFence = std::vector<VkFence>(this->swapchain.imageCount);

  SDL_GetWindowSizeInPixels(this->window, &w, &h);
  this->aspectRatio = static_cast<float>(this->w) / static_cast<float>(this->h);
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
  if (framebufferResized) {
    vkDeviceWaitIdle(this->device.logicalDevice);
    this->swapchain.recreate(this->window, this->surface);
    this->framebuffer.recreate(this->swapchain, this->renderPass);
    this->commandPool.recreate(this->framebuffer);
    this->swapchain.imageCount = this->swapchain.images.size();

    this->imageAvailableSemaphore.clear();
    this->imageAvailableSemaphore.reserve(this->maxFramesInFlight);
    for (uint8_t i = 0; i < this->maxFramesInFlight; i++)
      this->imageAvailableSemaphore.emplace_back(this->device);

    this->renderFinishedSemaphore.clear();
    this->renderFinishedSemaphore.reserve(this->swapchain.imageCount);
    for (uint8_t i = 0; i < this->swapchain.imageCount; i++)
      this->renderFinishedSemaphore.emplace_back(this->device);

    inFlightFence.clear();
    inFlightFence.reserve(this->maxFramesInFlight);
    for (uint8_t i = 0; i < this->maxFramesInFlight; i++)
      inFlightFence.emplace_back(this->device, VK_FENCE_CREATE_SIGNALED_BIT);

    imagesInFlightFence = std::vector<VkFence>(this->swapchain.imageCount, 0);
    framebufferResized = false;
    SDL_GetWindowSizeInPixels(this->window, &this->w, &this->h);
    aspectRatio = static_cast<float>(this->w) / static_cast<float>(this->h);
  }

  vkWaitForFences(this->device.logicalDevice, 1, &inFlightFence[currentFrame].fence, VK_TRUE, UINT64_MAX);
  vkResetFences(this->device.logicalDevice, 1, &inFlightFence[currentFrame].fence);

  uint32_t imageIndex;
  CheckVkResult(vkAcquireNextImageKHR(this->device.logicalDevice, this->swapchain.swapchain, UINT64_MAX, imageAvailableSemaphore[currentFrame].semaphore, VK_NULL_HANDLE, &imageIndex));
  if (imagesInFlightFence[imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(
        this->device.logicalDevice,
        1,
        &imagesInFlightFence[imageIndex],
        VK_TRUE,
        UINT64_MAX);
  }

  imagesInFlightFence[imageIndex] = inFlightFence[currentFrame].fence;
  this->commandBuffer = this->commandPool.commandBuffer(imageIndex);
  CheckVkResult(vkResetCommandBuffer(this->commandBuffer, 0));

  VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  CheckVkResult(vkBeginCommandBuffer(this->commandBuffer, &beginInfo));

  VkClearValue clearColor{.color = {0.05f, 0.05f, 0.05f, 1.0f}};
  VkClearValue clearValues[2]{};
  clearValues[0].color = {{0.05f, 0.05f, 0.05f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  std::vector<VkBuffer> giggityBuffer(this->meshesToRender.size());
  std::vector<VkDeviceMemory> giggityBufferMemory(this->meshesToRender.size());
  uint32_t giggityIndex = 0;
  uint32_t totalSize = 0;

  for (const GFVL::Mesh &mesh : this->meshesToRender) {
    if (mesh.vertexBuffer_.memoryAllocation_ == GFVL::VertexBuffer::MemoryAllocation::DeviceOnly) {
      GFVL::createBuffer(
          this->device,
          mesh.size(),
          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
          giggityBuffer[giggityIndex],
          giggityBufferMemory[giggityIndex]);

      VkBufferCopy copyRegion{
          .srcOffset = 0,
          .dstOffset = 0,
          .size = mesh.size()};

      vkCmdCopyBuffer(
          commandBuffer,
          mesh.vertexBuffer_.buffer_,
          giggityBuffer[giggityIndex],
          1,
          &copyRegion);

      giggityIndex++;
    } else {
      giggityBuffer[giggityIndex] = mesh.vertexBuffer_.buffer_;
      giggityBufferMemory[giggityIndex] = mesh.vertexBuffer_.bufferMemory_;
    }
    totalSize += mesh.size();
  }

  VkRenderPassBeginInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = this->renderPass.renderPass,
      .framebuffer = this->framebuffer.framebuffers[imageIndex],
      .renderArea = {
          .offset = {0, 0},
          .extent = this->swapchain.extent},
      .clearValueCount = 2,
      .pClearValues = clearValues};

  vkCmdBeginRenderPass(this->commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(this->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline.pipeline);

  // GFVLinstance.uniformBuffer.bindings[0].update(&camera);
  // GFVLinstance.uniformBuffer.bind(GFVLinstance.commandBuffer, GFVLinstance.pipeline, 0);

  this->uniformBuffer.bind(this->commandBuffer, this->pipeline, 0); // TEMPORARY
  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = (float)swapchain.extent.width,
      .height = (float)swapchain.extent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
  vkCmdSetViewport(this->commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{.offset = {0, 0}, .extent = swapchain.extent}; // this just cuts off rendering if not in swapchain
  vkCmdSetScissor(this->commandBuffer, 0, 1, &scissor);
  VkDeviceSize offsets[] = {0};

  vkCmdBindVertexBuffers(this->commandBuffer, 0, giggityBuffer.size(), giggityBuffer.data(), offsets);

  vkCmdDraw(this->commandBuffer, totalSize, 1, 0, 0);

  vkCmdEndRenderPass(this->commandBuffer);

  CheckVkResult(vkEndCommandBuffer(this->commandBuffer));

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphore[currentFrame].semaphore};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSemaphore signalSemaphores[] = {renderFinishedSemaphore[imageIndex].semaphore};

  VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = waitSemaphores,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &this->commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = signalSemaphores};

  CheckVkResult(vkQueueSubmit(this->device.graphicsQueue, 1, &submitInfo, inFlightFence[currentFrame].fence));

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
      &inFlightFence[currentFrame].fence,
      VK_TRUE,
      UINT64_MAX);
  for (uint32_t i = 0; i < giggityBuffer.size(); i++) {
    vkDestroyBuffer(this->device.logicalDevice, giggityBuffer[i], nullptr);
    vkFreeMemory(this->device.logicalDevice, giggityBufferMemory[i], nullptr);
  }

  this->currentFrame = (this->currentFrame + 1) % this->maxFramesInFlight;
}
INSTANCE::~INSTANCE() {
  vkDeviceWaitIdle(device.logicalDevice);
}
} // namespace GFVL
