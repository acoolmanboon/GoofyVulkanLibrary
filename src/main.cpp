#include "GFVL/GFVL.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

// Remove-Item build -Recurse -Force; cmake -B build -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 -G Ninja
// cmake --build build --verbose; build/main.exe
// glslc src/vertex_shader.vert -o src/vertex_shader.spv
// glslc src/fragment_shader.frag -o src/fragment_shader.spv

// glslc src/vertex_shader.vert -o src/vertex_shader.spv; glslc src/fragment_shader.frag -o src/fragment_shader.spv;  cmake --build build --verbose; build/main.exe

const char *VkResultToString(VkResult result) {
  switch (result) {
  case VK_SUCCESS:
    return "VK_SUCCESS";
  case VK_NOT_READY:
    return "VK_NOT_READY";
  case VK_TIMEOUT:
    return "VK_TIMEOUT";
  case VK_EVENT_SET:
    return "VK_EVENT_SET";
  case VK_EVENT_RESET:
    return "VK_EVENT_RESET";
  case VK_INCOMPLETE:
    return "VK_INCOMPLETE";

  case VK_ERROR_OUT_OF_HOST_MEMORY:
    return "VK_ERROR_OUT_OF_HOST_MEMORY";
  case VK_ERROR_OUT_OF_DEVICE_MEMORY:
    return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
  case VK_ERROR_INITIALIZATION_FAILED:
    return "VK_ERROR_INITIALIZATION_FAILED";
  case VK_ERROR_DEVICE_LOST:
    return "VK_ERROR_DEVICE_LOST";
  case VK_ERROR_MEMORY_MAP_FAILED:
    return "VK_ERROR_MEMORY_MAP_FAILED";
  case VK_ERROR_LAYER_NOT_PRESENT:
    return "VK_ERROR_LAYER_NOT_PRESENT";
  case VK_ERROR_EXTENSION_NOT_PRESENT:
    return "VK_ERROR_EXTENSION_NOT_PRESENT";
  case VK_ERROR_FEATURE_NOT_PRESENT:
    return "VK_ERROR_FEATURE_NOT_PRESENT";
  case VK_ERROR_INCOMPATIBLE_DRIVER:
    return "VK_ERROR_INCOMPATIBLE_DRIVER";
  case VK_ERROR_TOO_MANY_OBJECTS:
    return "VK_ERROR_TOO_MANY_OBJECTS";
  case VK_ERROR_FORMAT_NOT_SUPPORTED:
    return "VK_ERROR_FORMAT_NOT_SUPPORTED";

  default:
    return "UNKNOWN_VK_RESULT";
  }
}
VkResult CheckVkResult(VkResult result) {
  if (result < 0) {
    std::cout << "[GFVL] Error! : " << VkResultToString(result) << " (" << static_cast<int>(result) << ")\n";
    throw std::runtime_error("[GFVL] Error detected. read the above message");
  }
  return result;
}
// USER-DEFINED STUFF

struct vertice {
  float position[3];
  float color[3];
};
int main() {
  if (!SDL_Init(SDL_INIT_VIDEO))
    throw std::runtime_error(SDL_GetError());

  SDL_Window *window = SDL_CreateWindow(
      "goofyVLib Example",
      800,
      600,
      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  if (!window)
    throw std::runtime_error(SDL_GetError());

  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "goofyVLib Example",
      .applicationVersion = 1,
      .pEngineName = "goofyVLib",
      .engineVersion = 1,
      .apiVersion = VK_API_VERSION_1_3};

  VkInstance instance = GFVL::InitializeVkInstance(&appInfo);
  VkSurfaceKHR surface;
  if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface))
    std::cout << "[GFVL] SDL error : " << SDL_GetError() << '\n';

  GFVL::DEVICE device(instance, surface, GFVL::PREFERRED_GPU_POWER_SAVING);
  GFVL::SWAPCHAIN swapchain(device, window, surface);
  GFVL::RENDERPASS renderPass(device, swapchain);

  std::vector<GFVL::SHADER> shaderStages;
  shaderStages.emplace_back(device, VK_SHADER_STAGE_VERTEX_BIT, "src/vertex_shader.spv");
  shaderStages.emplace_back(device, VK_SHADER_STAGE_FRAGMENT_BIT, "src/fragment_shader.spv");

  GFVL::VERTEX_LAYOUT layout = {
      .binding = {.stride = sizeof(vertice)},
  };
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, position));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, color));

  GFVL::PIPELINE pipeline(device, swapchain, layout, shaderStages, renderPass);
  GFVL::FRAMEBUFFER framebuffers(device, swapchain, renderPass);
  std::vector<VkCommandBuffer> commandBuffers;

  commandBuffers.resize(framebuffers.framebuffers.size());

  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount =
          static_cast<uint32_t>(commandBuffers.size())};

  CheckVkResult(
      vkAllocateCommandBuffers(
          device.logicalDevice,
          &allocInfo,
          commandBuffers.data()));

  std::vector<vertice> vertices =
      {
          {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
          {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
          {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;

  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = sizeof(vertices[0]) * vertices.size(),
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

  CheckVkResult(
      vkCreateBuffer(
          device.logicalDevice,
          &bufferInfo,
          nullptr,
          &vertexBuffer));

  VkMemoryRequirements memRequirements;

  vkGetBufferMemoryRequirements(
      device.logicalDevice,
      vertexBuffer,
      &memRequirements);

  VkPhysicalDeviceMemoryProperties memoryProperties;

  vkGetPhysicalDeviceMemoryProperties(
      device.physicalDevice,
      &memoryProperties);

  uint32_t memoryType = UINT32_MAX;

  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((memRequirements.memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags &
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
        (memoryProperties.memoryTypes[i].propertyFlags &
         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
      memoryType = i;
      break;
    }
  }

  if (memoryType == UINT32_MAX)
    throw std::runtime_error("No suitable vertex memory");

  VkMemoryAllocateInfo allocVertexMemory{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = memoryType};

  CheckVkResult(
      vkAllocateMemory(
          device.logicalDevice,
          &allocVertexMemory,
          nullptr,
          &vertexBufferMemory));

  CheckVkResult(
      vkBindBufferMemory(
          device.logicalDevice,
          vertexBuffer,
          vertexBufferMemory,
          0));

  void *data;

  CheckVkResult(
      vkMapMemory(
          device.logicalDevice,
          vertexBufferMemory,
          0,
          bufferInfo.size,
          0,
          &data));

  memcpy(
      data,
      vertices.data(),
      bufferInfo.size);

  vkUnmapMemory(
      device.logicalDevice,
      vertexBufferMemory);

  // =============================
  // COMMAND BUFFER RECORDING
  // =============================

  for (size_t i = 0; i < commandBuffers.size(); i++) {

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    CheckVkResult(
        vkBeginCommandBuffer(
            commandBuffers[i],
            &beginInfo));

    VkClearValue clearColor{
        .color = {0.05f, 0.05f, 0.05f, 1.0f}};

    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.renderPass,
        .framebuffer = framebuffers.framebuffers[i],
        .renderArea =
            {
                .offset = {0, 0},
                .extent = swapchain.extent},
        .clearValueCount = 1,
        .pClearValues = &clearColor};

    vkCmdBeginRenderPass(
        commandBuffers[i],
        &renderPassInfo,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(
        commandBuffers[i],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.pipeline);

    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(
        commandBuffers[i],
        0,
        1,
        &vertexBuffer,
        offsets);
    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)swapchain.extent.width,
        .height = (float)swapchain.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f};

    vkCmdSetViewport(
        commandBuffers[i],
        0,
        1,
        &viewport);

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapchain.extent};

    vkCmdSetScissor(
        commandBuffers[i],
        0,
        1,
        &scissor);
    vkCmdDraw(
        commandBuffers[i],
        static_cast<uint32_t>(vertices.size()),
        1,
        0,
        0);

    vkCmdEndRenderPass(
        commandBuffers[i]);

    CheckVkResult(
        vkEndCommandBuffer(
            commandBuffers[i]));
  }

  // =============================
  // SYNCHRONIZATION
  // =============================

  VkSemaphore imageAvailable;
  VkSemaphore renderFinished;

  VkFence inFlightFence;

  VkSemaphoreCreateInfo semaphoreInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  CheckVkResult(
      vkCreateSemaphore(
          device.logicalDevice,
          &semaphoreInfo,
          nullptr,
          &imageAvailable));

  CheckVkResult(
      vkCreateSemaphore(
          device.logicalDevice,
          &semaphoreInfo,
          nullptr,
          &renderFinished));

  VkFenceCreateInfo fenceInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  CheckVkResult(
      vkCreateFence(
          device.logicalDevice,
          &fenceInfo,
          nullptr,
          &inFlightFence));

  // =============================
  // MAIN LOOP
  // =============================

  bool running = true;
  bool framebufferResized = false;

  while (running) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        running = false;

      if (event.type == SDL_EVENT_WINDOW_RESIZED ||
          event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        framebufferResized = true;
      }
    }

    if (framebufferResized && false) {
      vkDeviceWaitIdle(device.logicalDevice);

      framebufferResized = false;
    }

    vkWaitForFences(
        device.logicalDevice,
        1,
        &inFlightFence,
        VK_TRUE,
        UINT64_MAX);

    vkResetFences(
        device.logicalDevice,
        1,
        &inFlightFence);

    uint32_t imageIndex;

    CheckVkResult(
        vkAcquireNextImageKHR(
            device.logicalDevice,
            swapchain.swapchain,
            UINT64_MAX,
            imageAvailable,
            VK_NULL_HANDLE,
            &imageIndex));

    VkSemaphore waitSemaphores[] =
        {
            imageAvailable};

    VkPipelineStageFlags waitStages[] =
        {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSemaphore signalSemaphores[] =
        {
            renderFinished};

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,

        .pWaitDstStageMask = waitStages,

        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffers[imageIndex],

        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores};

    CheckVkResult(
        vkQueueSubmit(
            device.graphicsQueue,
            1,
            &submitInfo,
            inFlightFence));

    VkSwapchainKHR swapchains[] =
        {
            swapchain.swapchain};

    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,

        .swapchainCount = 1,
        .pSwapchains = swapchains,

        .pImageIndices = &imageIndex};

    CheckVkResult(
        vkQueuePresentKHR(
            device.graphicsQueue,
            &presentInfo));
  }

  // =============================
  // CLEANUP
  // =============================

  vkDeviceWaitIdle(device.logicalDevice);

  vkDestroyFence(
      device.logicalDevice,
      inFlightFence,
      nullptr);

  vkDestroySemaphore(
      device.logicalDevice,
      renderFinished,
      nullptr);

  vkDestroySemaphore(
      device.logicalDevice,
      imageAvailable,
      nullptr);

  vkDestroyBuffer(
      device.logicalDevice,
      vertexBuffer,
      nullptr);

  vkFreeMemory(
      device.logicalDevice,
      vertexBufferMemory,
      nullptr);
  return 0;
}