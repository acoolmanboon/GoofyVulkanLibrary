#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include "GFVL/GFVL.hpp"
// Remove-Item build -Recurse -Force; cmake -B build -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 -G Ninja
// cmake --build build --verbose; build/main.exe
// glslc src/vertex_shader.vert -o src/vertex_shader.spv
// glslc src/fragment_shader.frag -o src/fragment_shader.spv

// glslc src/vertex_shader.vert -o src/vertex_shader.spv; glslc src/fragment_shader.frag -o src/fragment_shader.spv;  cmake --build build --verbose; build/main.exe

std::vector<char> readFile(const std::string &filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
  }
  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);
  file.seekg(0);
  file.read(buffer.data(), fileSize);
  file.close();

  return buffer;
}

VkShaderModule createShaderModule(const std::vector<char> &code, VkDevice device) {
  VkShaderModuleCreateInfo shaderCreationInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.size(),
      .pCode = reinterpret_cast<const uint32_t *>(code.data())
    };
  VkShaderModule shaderModule;
  GFVL::CheckVkResult(vkCreateShaderModule(device, &shaderCreationInfo, nullptr, &shaderModule));
  return shaderModule;
}


// USER-DEFINED STUFF
struct GFVL_VERTEX_LAYOUT {
  VkVertexInputBindingDescription binding{
      .binding = 0,
      .stride = 0,
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

  std::vector<VkVertexInputAttributeDescription> attributes;

  void addAttribute(VkFormat format, uint32_t offset) {
    attributes.push_back({
      .location = static_cast<uint32_t>(attributes.size()),
      .binding = binding.binding,
      .format = format,
      .offset = offset});
  }

  VkPipelineVertexInputStateCreateInfo getInfo() {
    return {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1, // TBD : add multiple bindings
        .pVertexBindingDescriptions = &binding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size()),
        .pVertexAttributeDescriptions = attributes.data()};
  }
};

struct vertice {
  float position[3];
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

  GFVL::DEVICE device(GFVL::PREFERRED_GPU_POWER_SAVING, instance, surface, {});
  VkQueue graphicsQueue;
  vkGetDeviceQueue(device.device, device.graphicsFamilyIndex, 0, &graphicsQueue);

  GFVL::SWAPCHAIN swapchain(device, window);
  
  // i hate my life
  VkShaderModule vertexShaderModule = createShaderModule(readFile("src/vertex_shader.spv"), device.device);
  VkShaderModule fragmentShaderModule = createShaderModule(readFile("src/fragment_shader.spv"), device.device);

  VkPipelineShaderStageCreateInfo vertexShaderStageInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vertexShaderModule,
      .pName = "main"
    };

  VkPipelineShaderStageCreateInfo fragmentShaderStageInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = fragmentShaderModule,
      .pName = "main"
    };

  std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  // OUUU SHII OUU SHIIII
  GFVL_VERTEX_LAYOUT vertexLayout = {
    .binding = {.stride = sizeof(vertice)},
  };

  vertexLayout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, position));

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = swapchain.format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  colorAttachment.initialLayout =VK_IMAGE_LAYOUT_UNDEFINED;

  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef{};

  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};

  subpass.pipelineBindPoint =
      VK_PIPELINE_BIND_POINT_GRAPHICS;

  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments =
      &colorAttachmentRef;

  VkRenderPassCreateInfo renderPassInfo{};

  renderPassInfo.sType =
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments =
      &colorAttachment;

  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses =
      &subpass;

  VkRenderPass renderPass;

  GFVL::CheckVkResult(
    vkCreateRenderPass(
        device.device,
        &renderPassInfo,
        nullptr,
        &renderPass));

  VkPipelineLayout pipelineLayout;

  VkPipelineLayoutCreateInfo info{
      .sType =
          VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

  vkCreatePipelineLayout(
      device.device,
      &info,
      nullptr,
      &pipelineLayout);

  VkPipeline graphicsPipeline;

  // vertex input
  VkPipelineVertexInputStateCreateInfo vertexInputInfo =
      vertexLayout.getInfo();

  // input assembly
  // That morning just me and you, with azure views for two
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  // viewport/scissor dynamic
  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1};

  // rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE, // VK_CULL_MODE_BACK_BIT
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE};

  // multisampling
  VkPipelineMultisampleStateCreateInfo multisampling{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
  };

  // color blending
  VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = VK_FALSE,
      .colorWriteMask =
          VK_COLOR_COMPONENT_R_BIT |
          VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT |
          VK_COLOR_COMPONENT_A_BIT};

  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  // shader stages
  VkPipelineShaderStageCreateInfo stages[] = {
      vertexShaderStageInfo,
      fragmentShaderStageInfo};

  // pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

      .stageCount = 2,
      .pStages = stages,

      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,

      .layout = pipelineLayout,
      .renderPass = renderPass,
      .subpass = 0};

  GFVL::CheckVkResult(
      vkCreateGraphicsPipelines(
          device.device,
          VK_NULL_HANDLE,
          1,
          &pipelineInfo,
          nullptr,
          &graphicsPipeline));

  std::vector<VkFramebuffer> framebuffers;

  framebuffers.resize(swapchain.imageViews.size());

  for (size_t i = 0; i < swapchain.imageViews.size(); i++) {

    VkImageView attachments[] = {
        swapchain.imageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .width = swapchain.extent.width,
        .height = swapchain.extent.height,
        .layers = 1};

    GFVL::CheckVkResult(
        vkCreateFramebuffer(
            device.device,
            &framebufferInfo,
            nullptr,
            &framebuffers[i]));
  }
  VkCommandPool commandPool;

  VkCommandPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = device.graphicsFamilyIndex};

  GFVL::CheckVkResult(
      vkCreateCommandPool(
          device.device,
          &poolInfo,
          nullptr,
          &commandPool));
  std::vector<VkCommandBuffer> commandBuffers;

  commandBuffers.resize(framebuffers.size());

  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount =
          static_cast<uint32_t>(commandBuffers.size())};

  GFVL::CheckVkResult(
      vkAllocateCommandBuffers(
          device.device,
          &allocInfo,
          commandBuffers.data()));

  std::vector<vertice> vertices =
      {
          {{0.0f, -0.5f, 0.0f}},
          {{0.5f, 0.5f, 0.0f}},
          {{-0.5f, 0.5f, 0.0f}}};

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;

  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = sizeof(vertices[0]) * vertices.size(),
      .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

  GFVL::CheckVkResult(
      vkCreateBuffer(
          device.device,
          &bufferInfo,
          nullptr,
          &vertexBuffer));

  VkMemoryRequirements memRequirements;

  vkGetBufferMemoryRequirements(
      device.device,
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

  GFVL::CheckVkResult(
      vkAllocateMemory(
          device.device,
          &allocVertexMemory,
          nullptr,
          &vertexBufferMemory));

  GFVL::CheckVkResult(
      vkBindBufferMemory(
          device.device,
          vertexBuffer,
          vertexBufferMemory,
          0));

  void *data;

  GFVL::CheckVkResult(
      vkMapMemory(
          device.device,
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
      device.device,
      vertexBufferMemory);

  // =============================
  // COMMAND BUFFER RECORDING
  // =============================

  for (size_t i = 0; i < commandBuffers.size(); i++) {

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

    GFVL::CheckVkResult(
        vkBeginCommandBuffer(
            commandBuffers[i],
            &beginInfo));

    VkClearValue clearColor{
        .color = {0.05f, 0.05f, 0.05f, 1.0f}};

    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass,
        .framebuffer = framebuffers[i],
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
        graphicsPipeline);

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

    GFVL::CheckVkResult(
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

  GFVL::CheckVkResult(
      vkCreateSemaphore(
          device.device,
          &semaphoreInfo,
          nullptr,
          &imageAvailable));

  GFVL::CheckVkResult(
      vkCreateSemaphore(
          device.device,
          &semaphoreInfo,
          nullptr,
          &renderFinished));

  VkFenceCreateInfo fenceInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  GFVL::CheckVkResult(
      vkCreateFence(
          device.device,
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
      vkDeviceWaitIdle(device.device);

      //GFVL_RecreateSwapchain(swapchain);

      framebufferResized = false;
    }

    vkWaitForFences(
        device.device,
        1,
        &inFlightFence,
        VK_TRUE,
        UINT64_MAX);

    vkResetFences(
        device.device,
        1,
        &inFlightFence);

    uint32_t imageIndex;

    GFVL::CheckVkResult(
        vkAcquireNextImageKHR(
            device.device,
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

    GFVL::CheckVkResult(
        vkQueueSubmit(
            graphicsQueue,
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

    GFVL::CheckVkResult(
        vkQueuePresentKHR(
            graphicsQueue,
            &presentInfo));
  }

  // =============================
  // CLEANUP
  // =============================

  vkDeviceWaitIdle(device.device);

  vkDestroyFence(
      device.device,
      inFlightFence,
      nullptr);

  vkDestroySemaphore(
      device.device,
      renderFinished,
      nullptr);

  vkDestroySemaphore(
      device.device,
      imageAvailable,
      nullptr);

  vkDestroyBuffer(
      device.device,
      vertexBuffer,
      nullptr);

  vkFreeMemory(
      device.device,
      vertexBufferMemory,
      nullptr);
  return 0;
}