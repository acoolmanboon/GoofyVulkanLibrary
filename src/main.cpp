#include "GFVL/GFVL.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <glm/glm.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include<glm/gtc/matrix_transform.hpp>
#define print(message) std::cout << message << "\n";

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

void setViewport(VkCommandBuffer& commandBuffer, GFVL::SWAPCHAIN& swapchain) {
  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = (float)swapchain.extent.width,
      .height = (float)swapchain.extent.height,
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{.offset = {0, 0}, .extent = swapchain.extent}; // this just cuts off rendering if not in swapchain
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
// USER-DEFINED STUFF

struct vertice {
  float position[3];
  float color[3];
};
struct CameraUBO {
  glm::vec3 position;
  glm::vec3 angle;
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

  SDL_SetWindowRelativeMouseMode(window, true);

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

  GFVL::VERTEX_LAYOUT layout(sizeof(vertice));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, position));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, color));

  VkBuffer uniformBuffer;
  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = sizeof(CameraUBO),
      .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

  CheckVkResult(vkCreateBuffer(
      device.logicalDevice,
      &bufferInfo,
      nullptr,
      &uniformBuffer));

  VkDeviceMemory uniformBufferMemory;
  VkMemoryRequirements memRequirements;

  vkGetBufferMemoryRequirements(
      device.logicalDevice,
      uniformBuffer,
      &memRequirements);

  VkMemoryAllocateInfo memoryAllocatonInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex = GFVL::findMemoryType(device.physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), 
  };

  CheckVkResult(vkAllocateMemory(
      device.logicalDevice,
      &memoryAllocatonInfo,
      nullptr,
      &uniformBufferMemory));

  vkBindBufferMemory(
      device.logicalDevice,
      uniformBuffer,
      uniformBufferMemory,
      0);

  VkDescriptorSetLayoutBinding cameraBinding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT};

  VkDescriptorSetLayoutCreateInfo descriptorInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = 1,
      .pBindings = &cameraBinding};

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts(1);

  CheckVkResult(vkCreateDescriptorSetLayout(
      device.logicalDevice,
      &descriptorInfo,
      nullptr,
      &descriptorSetLayouts[0]));

  GFVL::PIPELINE pipeline(device, swapchain, layout, shaderStages, renderPass, descriptorSetLayouts);
  GFVL::FRAMEBUFFER framebuffers(device, swapchain, renderPass);
  CameraUBO camera{};

  camera.position = glm::vec3(0,0,-1);
  camera.angle = glm::vec3(0, 0, 0);
  void *data;

  vkMapMemory(
      device.logicalDevice,
      uniformBufferMemory,
      0,
      sizeof(CameraUBO),
      0,
      &data);

  memcpy(data, &camera, sizeof(CameraUBO));

  vkUnmapMemory(
      device.logicalDevice,
      uniformBufferMemory);

  // joke hed

  VkDescriptorPoolSize poolSize{
      .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = 1};

  VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1,
      .poolSizeCount = 1,
      .pPoolSizes = &poolSize};

  VkDescriptorPool descriptorPool;

  CheckVkResult(vkCreateDescriptorPool(
      device.logicalDevice,
      &poolInfo,
      nullptr,
      &descriptorPool));

  VkDescriptorSet cameraDescriptorSet;

  VkDescriptorSetAllocateInfo descriptorSetAllocationInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptorPool,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptorSetLayouts[0]};

  CheckVkResult(vkAllocateDescriptorSets(
      device.logicalDevice,
      &descriptorSetAllocationInfo,
      &cameraDescriptorSet));

  VkDescriptorBufferInfo descriptorBufferInfo{
      .buffer = uniformBuffer,
      .offset = 0,
      .range = sizeof(CameraUBO)};

  VkWriteDescriptorSet descriptorWrite{
      .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet = cameraDescriptorSet,
      .dstBinding = 0,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .pBufferInfo = &descriptorBufferInfo};

  vkUpdateDescriptorSets(
      device.logicalDevice,
      1,
      &descriptorWrite,
      0,
      nullptr);
  GFVL::COMMAND_POOL commandPool(device, framebuffers);
  std::vector<vertice> vertices =
      {
          // Front face (z = -0.5)
          {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
          {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
          {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},

          {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
          {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
          {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},

          // Back face (z = 0.5)
          {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
          {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},

          {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
          {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},

          // Left face (x = -0.5)
          {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
          {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
          {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},

          {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
          {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
          {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},

          // Right face (x = 0.5)
          {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
          {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},

          {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
          {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
          {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},

          // Bottom face (y = -0.5)
          {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
          {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},
          {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},

          {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}},
          {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
          {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},

          // Top face (y = 0.5)
          {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
          {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},

          {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
          {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},
          {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 1.0f}}};
  GFVL::VERTEX_BUFFER vertexBuffer(device, vertices.capacity() * sizeof(vertice), vertices.data());

  VkSemaphore imageAvailable; // can i render?
  VkSemaphore renderFinished; // am i done rendering my stuff?

  VkFence inFlightFence;
  VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  CheckVkResult(vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &imageAvailable));
  CheckVkResult(vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &renderFinished));

  VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
  CheckVkResult(vkCreateFence(device.logicalDevice, &fenceInfo, nullptr, &inFlightFence));

  bool running = true;
  bool framebufferResized = false;
  float speed = 1.0f;

  uint64_t last_time = SDL_GetPerformanceCounter();
  double delta_time = 0.0; // In seconds
  while (running) {
    uint64_t current_time = SDL_GetPerformanceCounter();
    delta_time = (double)(current_time - last_time) / (double)SDL_GetPerformanceFrequency();
    last_time = current_time;
    if (delta_time > 0.01) // jic lag
      delta_time = 0.1;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        running = false;

      if (event.type == SDL_EVENT_WINDOW_RESIZED ||
          event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
        framebufferResized = true;
    }

    if (framebufferResized) {
      vkDeviceWaitIdle(device.logicalDevice);
      swapchain.recreateSwapchain(window, surface);
      framebuffers.recreate(swapchain, renderPass);
      framebufferResized = false;
    }

    const bool *keyboard = SDL_GetKeyboardState(nullptr);
    float speed = 5.0f;
    float deltaTime = 1.0f / 60.0f; // replace with your real delta time

    glm::vec3 forward;

    forward.x = sin(camera.angle.y);
    forward.y = 0.0f;
    forward.z = -cos(camera.angle.y);
    if (keyboard[SDL_SCANCODE_W]) {
      camera.position += forward * speed * deltaTime;
    }

    if (keyboard[SDL_SCANCODE_S]) {
      camera.position -= forward * speed * deltaTime;
    }

    glm::vec3 right;

    right.x = cos(camera.angle.y);
    right.y = 0.0f;
    right.z = sin(camera.angle.y);

    if (keyboard[SDL_SCANCODE_A]) {
      camera.position -= right * speed * deltaTime;
    }

    if (keyboard[SDL_SCANCODE_D]) {
      camera.position += right * speed * deltaTime;
    }

    vkWaitForFences(device.logicalDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device.logicalDevice, 1, &inFlightFence);

    uint32_t imageIndex;
    CheckVkResult(vkAcquireNextImageKHR(device.logicalDevice, swapchain.swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex));

    VkCommandBuffer commandBuffer = commandPool.commandBuffers[imageIndex];
    CheckVkResult(vkResetCommandBuffer(commandBuffer, 0));

    VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    CheckVkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkClearValue clearColor{.color = {0.05f, 0.05f, 0.05f, 1.0f}};

    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.renderPass,
        .framebuffer = framebuffers.framebuffers[imageIndex],
        .renderArea = {
            .offset = {0, 0},
            .extent = swapchain.extent},
        .clearValueCount = 1,
        .pClearValues = &clearColor};

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline.pipelineLayout,
        0,
        1,
        &cameraDescriptorSet,
        0,
        nullptr);
    setViewport(commandBuffer, swapchain);
    VkDeviceSize offsets[] = {0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.vertexBuffer, offsets);

    vkMapMemory(
        device.logicalDevice,
        uniformBufferMemory,
        0,
        sizeof(CameraUBO),
        0,
        &data);

    memcpy(data, &camera, sizeof(CameraUBO));

    vkUnmapMemory(
        device.logicalDevice,
        uniformBufferMemory);

    vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    CheckVkResult(vkEndCommandBuffer(commandBuffer));

    VkSemaphore waitSemaphores[] = {imageAvailable};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinished};

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores};

    CheckVkResult(vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, inFlightFence));

    VkSwapchainKHR swapchains[] = {swapchain.swapchain};

    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex};

    CheckVkResult(vkQueuePresentKHR(device.graphicsQueue, &presentInfo));
  }

  vkDeviceWaitIdle(device.logicalDevice);

  vkDestroyFence(device.logicalDevice, inFlightFence, nullptr);

  vkDestroySemaphore(device.logicalDevice, renderFinished, nullptr);

  vkDestroySemaphore(device.logicalDevice, imageAvailable, nullptr);

  return 0;
}