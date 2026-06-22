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

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include<glm/gtc/matrix_transform.hpp>
#define print(message) std::cout << message << "\n";
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// Remove-Item build -Recurse -Force; cmake -B build -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 -G Ninja
// cmake --build build --verbose; build/main.exe
// glslc src/vertex_shader.vert -o src/vertex_shader.spv
// glslc src/fragment_shader.frag -o src/fragment_shader.spv

// rm -Force src/fragment_shader.spv; rm -Force src/vertex_shader.spv; glslc src/vertex_shader.vert -o src/vertex_shader.spv; glslc src/fragment_shader.frag -o src/fragment_shader.spv;  cmake --build build --verbose; build/main.exe

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
  float normal[3];
  float color[3];
};
struct CameraUBO {
  glm::mat4 MVP;
  alignas(16) glm::vec3 viewPos;
  float padding;
};
void insertCube(glm::vec3 position, glm::vec3 color, glm::vec3 scale, std::vector<vertice> &vertices) {
  std::vector<vertice> cube = {
      // Front face (z = -0.5)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},

      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},

      // Back face (z = 0.5)
      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

      // Left face (x = -0.5)
      {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
      {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},

      {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},

      // Right face (x = 0.5)
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},

      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},

      // Bottom face (y = -0.5)
      {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},

      {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
      {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},

      // Top face (y = 0.5)
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},

      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}};

  for (vertice &vert : cube) {
    vert.position[0] *= scale.x;
    vert.position[1] *= scale.y;
    vert.position[2] *= scale.z;

    vert.position[0] += position.x;
    vert.position[1] += position.y;
    vert.position[2] += position.z;

    vert.color[0] = color.x;
    vert.color[1] = color.y;
    vert.color[2] = color.z;
  }

  vertices.insert(vertices.end(), cube.begin(), cube.end());
}
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
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, normal));
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
      .stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS};

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
  std::vector<vertice> vertices;
  for (int x = -25; x < 50; x++) {
    for (int y = -25; y < 50; y++) {
      for (int z = -25; z < 50; z++) {
        insertCube(glm::vec3(
          static_cast<float>(x) * 8.0f,
          static_cast<float>(y) * 8.0f,
          static_cast<float>(z) * 8.0f),
           glm::vec3(1.0f, 1.0f, 1.0f), 
           glm::vec3(1.8f, 1.8f, 1.8f), vertices
          );
      }
    }
  }
  print(vertices.size())
  GFVL::VERTEX_BUFFER vertexBuffer(device, vertices.size() * sizeof(vertice), vertices.data());

  // melica.uwu
  // dino_potato__  
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

  float aspect = 0;
  int w, h;
  SDL_GetWindowSizeInPixels(window, &w, &h);
  aspect = static_cast<float>(w) / static_cast<float>(h);

  uint64_t last_time = SDL_GetPerformanceCounter();
  float delta_time = 0.0; // In seconds

  glm::vec3 position(0,0,0);
  glm::quat angle ;
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
      if (event.type == SDL_EVENT_MOUSE_MOTION) {
        static float yaw = 0.0f;
        static float pitch = 0.0f;

        float sens = 0.002f;

        yaw -= event.motion.xrel * sens;
        pitch += event.motion.yrel * sens;

        pitch = glm::clamp(pitch, -1.5f, 1.5f);

        glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
        glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));

        angle = glm::normalize(qYaw * qPitch);
      }
    }

    if (framebufferResized) {
      vkDeviceWaitIdle(device.logicalDevice);
      swapchain.recreateSwapchain(window, surface);
      framebuffers.recreate(swapchain, renderPass);
      framebufferResized = false;
      int w, h;
      SDL_GetWindowSizeInPixels(window, &w, &h);
      aspect = static_cast<float>(w) / static_cast<float>(h);
    }

    const bool *keyboard = SDL_GetKeyboardState(nullptr);
    float speed = 5.0f;
    glm::vec3 forward = angle * glm::vec3(0, 0, -1);
    glm::vec3 right = angle * glm::vec3(1, 0, 0);

    if (keyboard[SDL_SCANCODE_W]) {
      position += forward * speed * delta_time;
    }

    if (keyboard[SDL_SCANCODE_S]) {
      position -= forward * speed * delta_time;
    }
    if (keyboard[SDL_SCANCODE_A]) {
      position -= right * speed * delta_time;
    }

    if (keyboard[SDL_SCANCODE_D]) {
      position += right * speed * delta_time;
    } 

    glm::mat4 proj = glm::perspectiveRH_ZO(
        glm::radians(90.0f),
        aspect,
        0.01f,
        100000.0f);

    glm::mat4 view =
        glm::mat4_cast(glm::conjugate(angle)) *
        glm::translate(glm::mat4(1.0f), -position);

    camera.MVP = proj * view;
    camera.viewPos = position;

    vkWaitForFences(device.logicalDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(device.logicalDevice, 1, &inFlightFence);

    uint32_t imageIndex;
    CheckVkResult(vkAcquireNextImageKHR(device.logicalDevice, swapchain.swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex));

    VkCommandBuffer commandBuffer = commandPool.commandBuffers[imageIndex];
    CheckVkResult(vkResetCommandBuffer(commandBuffer, 0));

    VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    CheckVkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkClearValue clearColor{.color = {0.05f, 0.05f, 0.05f, 1.0f}};
    VkClearValue clearValues[2]{};
    clearValues[0].color = {{0.05f, 0.05f, 0.05f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass.renderPass,
        .framebuffer = framebuffers.framebuffers[imageIndex],
        .renderArea = {
            .offset = {0, 0},
            .extent = swapchain.extent},
        .clearValueCount = 2,
        .pClearValues = clearValues};

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
  vkDestroyBuffer(device.logicalDevice, uniformBuffer, nullptr);
  vkFreeMemory(device.logicalDevice, uniformBufferMemory, nullptr);
  for (auto& layout : descriptorSetLayouts) {
    vkDestroyDescriptorSetLayout(device.logicalDevice, layout, nullptr);
  }
  vkDestroyDescriptorPool(device.logicalDevice, descriptorPool, nullptr);

  vkDestroyFence(device.logicalDevice, inFlightFence, nullptr);

  vkDestroySemaphore(device.logicalDevice, renderFinished, nullptr);

  vkDestroySemaphore(device.logicalDevice, imageAvailable, nullptr);

  return 0;
}