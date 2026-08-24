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
#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>

using namespace GFVL;
// USER-DEFINED STUFF
namespace GFVL {
Mesh INSTANCE::createMesh(Mesh::CreateInfo createInfo) {
  return Mesh(device, createInfo, commandPool, vmaAllocator);
}
void INSTANCE::setMouseLock(bool mouseLock) {
  SDL_SetWindowRelativeMouseMode(window, mouseLock);
}
INSTANCE::INSTANCE(APPLICATION_INFO applicationInfo, VertexLayout &layout, std::vector<UniformBufferBinding> &bindings, std::vector<SHADER_STAGE> &stages) : instance(InitializeVkInstance(applicationInfo)),
                                                                                                                                                                          window(SDL_CreateWindow(applicationInfo.applicationName, applicationInfo.width, applicationInfo.height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE)),
                                                                                                                                                                          surface(InitializeVkSurface()),
                                                                                                                                                                          device(this->instance, this->surface, applicationInfo.preferredGPU),
                                                                                                                                                                          swapchain(this->device, this->window, this->surface),
                                                                                                                                                                          renderPass(this->device, this->swapchain),
                                                                                                                                                                          shaderStages(InitializeShaderStages(stages)),
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
void INSTANCE::beginFrame() {
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
  vkResetFences(this->device.logicalDevice, 1, &currentFrame.gpuFinishedFence.fence);

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


  
}
void INSTANCE::renderMesh(Mesh &mesh) {
  Frame &currentFrame = frames[currentFrameIndex];
  VkDeviceSize offset = 0;
  VkBuffer meshBuffer = mesh.meshBuffer_.buffer();

  vkCmdBindVertexBuffers(
      currentFrame.commandBuffer,
      0,
      1,
      &meshBuffer,
      &offset);

  if (mesh.indiceDataSize == 0) {
    vkCmdDraw(
        currentFrame.commandBuffer,
        mesh.verticeCount(),
        1,
        0,
        0);
  } else {
    vkCmdBindIndexBuffer2(
        currentFrame.commandBuffer,
        mesh.meshBuffer_.buffer(),
        mesh.indiceDataOffset,
        mesh.indiceDataSize,
        mesh.indiceDataType);
    vkCmdDrawIndexed(
        currentFrame.commandBuffer,
        mesh.indiceCount,
        1,
        0,
        0,
        0);
  }
}
void INSTANCE::endFrame() {
  Frame &currentFrame = frames[currentFrameIndex];
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
  vkDestroyCommandPool(device.logicalDevice, commandPool, nullptr);
}
} // namespace GFVL
