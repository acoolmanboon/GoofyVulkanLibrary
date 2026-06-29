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

// cmake -B build -DCMAKE_BUILD_TYPE=Debug
// cmake -B build -DCMAKE_BUILD_TYPE=Release
// cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
// rm -Force build/main.exe; rm -Force src/fragment_shader.spv; rm -Force src/vertex_shader.spv; glslc src/vertex_shader.vert -o src/vertex_shader.spv; glslc src/fragment_shader.frag -o src/fragment_shader.spv;  cmake --build build --verbose; build/main.exe

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
struct LightingUBO {
  alignas(16) glm::vec3 lightPos;
  alignas(16) glm::vec3 lightColor;
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

  // owned by user
  std::vector<GFVL::SHADER_STAGE> shaderStages = {
    {
      .flags = VK_SHADER_STAGE_VERTEX_BIT,
      .filename = "src/vertex_shader.spv"
    },
    {
      .flags = VK_SHADER_STAGE_FRAGMENT_BIT,
      .filename = "src/fragment_shader.spv"
    }
  };

  CameraUBO camera;
  LightingUBO lighting = {.lightPos = glm::vec3(0.0f, 0.0f, 0.0f), .lightColor = glm::vec3(1.0f, 1.0f, 1.0f)};

  std::vector<GFVL::UNIFORM_BUFFER_BINDING> bindings = {
    {
      .size = sizeof(CameraUBO),
      .ubo = &camera
    },
    {
      .size = sizeof(LightingUBO),
      .ubo = &lighting
    }
  };
  
  GFVL::VERTEX_LAYOUT layout(sizeof(vertice));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, position));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, normal));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, color));

  GFVL::APPLICATION_INFO appInfo = {
    .applicationName = "GoofyVLib example",
    .applicationVersion = 1,
    .width = 600,
    .height = 800,
    .preferredGPU = GFVL::PREFERRED_GPU_POWER_SAVING
  };

  GFVL::INSTANCE GFVLinstance(appInfo, layout, bindings, shaderStages);

  std::vector<vertice> vertices;
  int start = -3;
  int end = 3;
  int steps = end - start;
  int cubes = steps*steps*steps;

  float xI = 0;
  float yI = 0;
  float zI = 0;

  float spacing = 50.0f;
  float scale = 20.0f;
  
  for (int x = start; x < end; x++) {
    for (int y = start; y < end; y++) {
      for (int z = start; z < end; z++) {
        if (x == 0 && y == 0 && z == 0)
          continue;
        insertCube(glm::vec3(
          static_cast<float>(x) * spacing,
          static_cast<float>(y) * spacing,
          static_cast<float>(z) * spacing),
          glm::vec3(
            xI/steps,
            yI/steps,
            zI/steps
          ), 
          glm::vec3(scale), vertices
        );
        //print("xI : " << (xI/steps) << "yI : " << (yI/steps) << "zI : " << (zI/steps))
        zI++;
      }
      zI = 0;
      yI++;
    }
    yI = 0;
    xI++;
  }

  GFVL::Mesh mesh(
    GFVLinstance.device, 
    GFVL::Mesh::CreateInfo{.size = vertices.size() * sizeof(vertice), .data = vertices.data(), .type = GFVL::VertexBuffer::Type::HostVisible, .commandBuffer = nullptr}
  );
  VkSemaphore imageAvailable; // can i render?
  VkSemaphore renderFinished; // am i done rendering my stuff?

  VkFence inFlightFence;
  VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

  CheckVkResult(vkCreateSemaphore(GFVLinstance.device.logicalDevice, &semaphoreInfo, nullptr, &imageAvailable));
  CheckVkResult(vkCreateSemaphore(GFVLinstance.device.logicalDevice, &semaphoreInfo, nullptr, &renderFinished));

  VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
  CheckVkResult(vkCreateFence(GFVLinstance.device.logicalDevice, &fenceInfo, nullptr, &inFlightFence));

  bool running = true;
  bool menu = false;
  bool framebufferResized = false;
  float speed = 1.0f;

  float aspect = 0;
  int w, h;
  SDL_GetWindowSizeInPixels(GFVLinstance.window, &w, &h);
  aspect = static_cast<float>(w) / static_cast<float>(h);

  uint64_t last_time = SDL_GetPerformanceCounter();
  float delta_time = 0.0; // In seconds

  glm::vec3 position(0,0,0);
  glm::quat angle ;
  while (running) {
    uint64_t current_time = SDL_GetPerformanceCounter();
    delta_time = (double)(current_time - last_time) / (double)SDL_GetPerformanceFrequency();
    last_time = current_time;

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
      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.repeat == 0) {
          if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
            menu = !menu;
            SDL_SetWindowRelativeMouseMode(GFVLinstance.window, menu);
          }
        }
      }
    }

    if (framebufferResized) {
      vkDeviceWaitIdle(GFVLinstance.device.logicalDevice);
      GFVLinstance.swapchain.recreateSwapchain(GFVLinstance.window, GFVLinstance.surface);
      GFVLinstance.framebuffer.recreate(GFVLinstance.swapchain, GFVLinstance.renderPass);
      framebufferResized = false;
      int w, h;
      SDL_GetWindowSizeInPixels(GFVLinstance.window, &w, &h);
      aspect = static_cast<float>(w) / static_cast<float>(h);
    }

    const bool *keyboard = SDL_GetKeyboardState(nullptr);
    float speed = keyboard[SDL_SCANCODE_LSHIFT] ? 250.0f : 5.0f;
    glm::vec3 forward = angle * glm::vec3(0, 0, -1);
    glm::vec3 right = angle * glm::vec3(1, 0, 0);

    if (keyboard[SDL_SCANCODE_W]) 
      position += forward * speed * delta_time;
    if (keyboard[SDL_SCANCODE_SPACE]) {
      lighting.lightPos = position;
      GFVLinstance.uniformBuffer.bindings[1].update(&lighting);
    }
    if (keyboard[SDL_SCANCODE_S]) 
      position -= forward * speed * delta_time;
    if (keyboard[SDL_SCANCODE_A]) 
      position -= right * speed * delta_time;
    if (keyboard[SDL_SCANCODE_D]) 
      position += right * speed * delta_time;
    

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

    vkWaitForFences(GFVLinstance.device.logicalDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(GFVLinstance.device.logicalDevice, 1, &inFlightFence);

    uint32_t imageIndex;
    CheckVkResult(vkAcquireNextImageKHR(GFVLinstance.device.logicalDevice, GFVLinstance.swapchain.swapchain, UINT64_MAX, imageAvailable, VK_NULL_HANDLE, &imageIndex));

    VkCommandBuffer commandBuffer = GFVLinstance.commandPool.commandBuffers[imageIndex];
    CheckVkResult(vkResetCommandBuffer(commandBuffer, 0));

    VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    CheckVkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    VkClearValue clearColor{.color = {0.05f, 0.05f, 0.05f, 1.0f}};
    VkClearValue clearValues[2]{};
    clearValues[0].color = {{0.05f, 0.05f, 0.05f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = GFVLinstance.renderPass.renderPass,
        .framebuffer = GFVLinstance.framebuffer.framebuffers[imageIndex],
        .renderArea = {
            .offset = {0, 0},
            .extent = GFVLinstance.swapchain.extent},
        .clearValueCount = 2,
        .pClearValues = clearValues};

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GFVLinstance.pipeline.pipeline);

    GFVLinstance.uniformBuffer.bindings[0].update(&camera);
    GFVLinstance.uniformBuffer.bind(commandBuffer, GFVLinstance.pipeline, 0);
    setViewport(commandBuffer, GFVLinstance.swapchain);
    VkDeviceSize offsets[] = {0};


    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mesh.vertexBuffer().buffer(), offsets);

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

    CheckVkResult(vkQueueSubmit(GFVLinstance.device.graphicsQueue, 1, &submitInfo, inFlightFence));

    VkSwapchainKHR swapchains[] = {GFVLinstance.swapchain.swapchain};

    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &imageIndex};

    CheckVkResult(vkQueuePresentKHR(GFVLinstance.device.graphicsQueue, &presentInfo));
  } 

  vkDeviceWaitIdle(GFVLinstance.device.logicalDevice);


  vkDestroyFence(GFVLinstance.device.logicalDevice, inFlightFence, nullptr);

  vkDestroySemaphore(GFVLinstance.device.logicalDevice, renderFinished, nullptr);

  vkDestroySemaphore(GFVLinstance.device.logicalDevice, imageAvailable, nullptr);

  return 0;
}