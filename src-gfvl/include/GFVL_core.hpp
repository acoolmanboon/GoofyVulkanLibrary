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
/**
 * @file GFVL_core.hpp
 * @brief Defines GFVL core functions.
 * @details Don't include this. Unless you wanna do some master hacking?
 */
#pragma once

#include <GFVL_definition.hpp>

namespace GFVL {
class PIPELINE;
enum PreferredGPU {
  PowerSaving,
  Performance,
};

struct AppInfo {
  const char *applicationName = "GFVL application";
  uint32_t applicationVersion = 1;
  int width = 800;
  int height = 600;
  uint32_t maxFramesInFlight = 2;
  PreferredGPU preferredGPU = PreferredGPU::Performance;
};

/**
 * @struct UniformBufferBinding
 * @brief Defines a binding in a uniform buffer.
 */
struct UniformBufferBinding {
  size_t size = 0;                                               ///< Size of the data to be passed to shader in bytes.
  uint32_t binding = 0;                                          ///< The binding that will be passed to your shader.
  uint32_t arrayCount = 1;                                       ///< If you are passing an array of data, for example a[1024] to shader, set this to 1024. Otherwise, leave this empty or set it to 1 to mark it as not being an array.
  VkShaderStageFlags shaderStage = VK_SHADER_STAGE_ALL_GRAPHICS; ///< What shaders can access this uniform buffer. Does not have to be set, default value will set to be accessible by all shaders. However, it is recommended to make them shader-specific.
  void *ubo = nullptr;                                           ///< Pointer to your data, this will be read automatically by the engine
  bool hasUpdated = true;                                        ///< Setting this to true tells GFVL to update the internal buffers. Set it to true after every change of the data.
};

struct ShaderStage {
  VkShaderStageFlagBits flags;
  const char *filename;
};

struct VertexLayout {
public:
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;
};

class Device {
public:
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice logicalDevice = VK_NULL_HANDLE;
  VkDeviceSize videoMemory = 0;

  uint32_t graphicsFamilyIndex = UINT32_MAX;
  uint32_t presentFamilyIndex = UINT32_MAX;

  VkQueue graphicsQueue = {};

  Device(VkInstance instance, VkSurfaceKHR surface, PreferredGPU preference);
  ~Device();

  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;

  Device(const Device &&) = delete;
  Device &operator=(const Device &&) = delete;
private:
  VkDeviceSize getDeviceVRAM(VkPhysicalDevice device);
  uint32_t getDeviceScore(VkPhysicalDevice device, PreferredGPU preference);
  bool enumerateQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t &graphicsFamilyIndex, uint32_t &presentFamilyIndex);
  VkBool32 hasRequiredDeviceExtensions(VkPhysicalDevice device);
};

class Semaphore {
public:
  VkSemaphore semaphore = VK_NULL_HANDLE;
  Semaphore(Device &device) : device_(device) {
    VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(this->device_.logicalDevice, &semaphoreInfo, nullptr, &this->semaphore);
  }

  Semaphore(const Semaphore &) = delete;
  Semaphore &operator=(const Semaphore &) = delete;

  Semaphore(Semaphore &&other) noexcept : device_(other.device_), semaphore(other.semaphore) {
    other.semaphore = VK_NULL_HANDLE;
  };
  Semaphore &operator=(Semaphore &&other) {
    ASSERTIF(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy semaphore with different devices");
    if (this == &other)
      return *this;

    if (this->semaphore != VK_NULL_HANDLE) {
      vkDestroySemaphore(this->device_.logicalDevice, this->semaphore, nullptr);
    }
    this->semaphore = other.semaphore;
    other.semaphore = VK_NULL_HANDLE;
    return *this;
  }

  ~Semaphore() {
    if (this->semaphore != VK_NULL_HANDLE)
      vkDestroySemaphore(this->device_.logicalDevice, this->semaphore, nullptr);
  }

private:
  Device &device_;
};

class Fence {
public:
  VkFence fence = VK_NULL_HANDLE;
  Fence(Device &device, VkFenceCreateFlags flags) : device_(device) {
    VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = flags};
    vkCreateFence(this->device_.logicalDevice, &fenceInfo, nullptr, &this->fence);
  }
  Fence(const Fence &) = delete;
  Fence &operator=(const Fence &) = delete;

  Fence(Fence &&other) noexcept : device_(other.device_), fence(other.fence) {
    other.fence = VK_NULL_HANDLE;
  };
  Fence &operator=(Fence &&other) {
    ASSERTIF(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy semaphore with different devices");
    if (this == &other)
      return *this;

    if (this->fence != VK_NULL_HANDLE) {
      vkDestroyFence(this->device_.logicalDevice, this->fence, nullptr);
    }
    this->fence = other.fence;
    other.fence = VK_NULL_HANDLE;
    return *this;
  }

  ~Fence() {
    if (this->fence != VK_NULL_HANDLE)
      vkDestroyFence(this->device_.logicalDevice, this->fence, nullptr);
  }

private:
  Device &device_;
};

class Swapchain {
public:
  VkSwapchainKHR swapchain{};

  VkFormat format{};
  VkExtent2D extent{};
  VkPresentModeKHR presentMode{};
  uint32_t imageCount{};

  std::vector<VkImage> images;
  std::vector<VkImageView> imageViews;

  bool framebufferResized = false;

  void recreate(SDL_Window *window, VkSurfaceKHR surface);

  Swapchain(Device &device, SDL_Window *window, VkSurfaceKHR surface);
  ~Swapchain();

  Swapchain(const Swapchain &) = delete;
  Swapchain &operator=(const Swapchain &) = delete;

  Swapchain(const Swapchain &&) = delete;
  Swapchain &operator=(const Swapchain &&) = delete;

private:
  Device &device_;
};

class SHADER {
public:
  VkShaderModule shaderModule = {};
  VkShaderStageFlagBits stage;

  SHADER(Device &device, VkShaderStageFlagBits stage, const char *filename);
  ~SHADER();

  SHADER(const SHADER &) = delete;
  SHADER &operator=(const SHADER &) = delete;

  SHADER(SHADER &&other) noexcept;
  SHADER &operator=(SHADER &&other) noexcept;

private:
  Device &device;
};

class RENDERPASS {
public:
  VkRenderPass renderPass = {};

  RENDERPASS(Device &device, Swapchain &swapchain);
  ~RENDERPASS();

  RENDERPASS(const RENDERPASS &) = delete;
  RENDERPASS &operator=(const RENDERPASS &) = delete;

  RENDERPASS(const RENDERPASS &&) = delete;
  RENDERPASS &operator=(const RENDERPASS &&) = delete;

private:
  Device &device;
};

class PIPELINE {
public:
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline = {};

  PIPELINE(Device &device, Swapchain &swapchain, VertexLayout &layout, std::vector<SHADER> &shaderStages, RENDERPASS &renderPass, std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
  ~PIPELINE();

  PIPELINE(const PIPELINE &) = delete;
  PIPELINE &operator=(const PIPELINE &) = delete;

  PIPELINE(const PIPELINE &&) = delete;
  PIPELINE &operator=(const PIPELINE &&) = delete;

private:
  Device &device;
};
/**
 * @class Framebuffer
 * @brief Stores Vulkan framebuffers.
 * @details
 */
class Framebuffer {
public:
  std::vector<VkFramebuffer> framebuffers;

  void recreate(Swapchain &swapchain, RENDERPASS &renderPass);
  Framebuffer(Device &device, Swapchain &swapchain, RENDERPASS &renderPass);
  ~Framebuffer();

  Framebuffer(const Framebuffer &) = delete;
  Framebuffer &operator=(const Framebuffer &) = delete;

  Framebuffer(const Framebuffer &&) = delete;
  Framebuffer &operator=(const Framebuffer &&) = delete;

private:
  Device &device;

  VkImage depthImage{};
  VkDeviceMemory depthMemory{};
  VkImageView depthImageView{};
  VkFormat depthFormat{};
};

/**
 * @class MeshBuffer
 * @brief Handles memory management for vertex buffers.
 * @details This allows for management of both in-VRAM buffers and CPU-readable data, but beware of trying to modify something with the wrong type.
 */
class MeshBuffer {
public:
  /**
   * @brief Defines memory allocation strategy.
   */
  enum class MemoryAllocation {
    DeviceOnly,  ///< Memory allocated will be in VRAM. Use for static-meshes. Fastest.
    HostVisible, ///< Memory allocated will be visible to CPU. Use for non-static meshes.
  };

  /**
   * @brief Configuration for creating MeshBuffer class.
   */
  struct CreateInfo {
    VkDeviceSize size;                                                 ///< Size of the allocated buffer memory in bytes.
    void *data;                                                        ///< Pointer to the data to copy into the buffer.
    MemoryAllocation memoryAllocation = MemoryAllocation::HostVisible; ///< Type of data, see definition.
    VkCommandPool commandPool;                                         ///< Command pool, used for alllocation of vertex buffers
    VmaAllocator allocator;                                            ///< VMA allocator
  };

  /**
   * @brief Creates a vertex buffer.
   * @param device A reference to your Device.
   * @param createinfo The creation information of the vertex buffer.
   */
  MeshBuffer(Device &device, const CreateInfo &createInfo); ///< Creates a vertex buffer.

  ~MeshBuffer(); ///< Destroys a vertex buffer and frees associated memory.

  MeshBuffer(const MeshBuffer &other) = delete;            ///< Copy constructor, removed as multiple vertex buffers will have the same buffer handles.
  MeshBuffer &operator=(const MeshBuffer &other) = delete; ///< Copy assignment operator, removed as multiple vertex buffers will have the same buffer handles.

  MeshBuffer(MeshBuffer &&other) noexcept;   ///< Move constructor, allowed but it will unbind vulkan resources of old object.
  MeshBuffer &operator=(MeshBuffer &&other); ///< Move assignment operator, allowed but it will unbind vulkan resources of old object.

  [[nodiscard]] VkDeviceSize size() const noexcept;
  [[nodiscard]] VkBuffer buffer() const noexcept;
  [[nodiscard]] MemoryAllocation memoryAllocation() const noexcept;

private:
  Device &device_;
  VmaAllocator allocator_;

  VkBuffer buffer_;
  VmaAllocation bufferMemory_;

  void *data_; ///< A pointer to the buffer data. Used for HostVisible memory.
  VkDeviceSize size_;
  MemoryAllocation memoryAllocation_;
};
class Frame {
private:
  struct FrameUniformBuffer {
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = nullptr;
    void *mappedMemory = nullptr;
  };

  void createCommandPool();
  void createDescriptorPool(uint32_t descriptorCount);

public:
  Semaphore imageAvailableSemaphore;
  Fence gpuFinishedFence;

  VkCommandPool commandPool;
  VkCommandBuffer commandBuffer;

  VkDescriptorPool descriptorPool;
  VkDescriptorSet descriptorSet; // turn into a std::vector for many sets of UBOs, for now, no.

  std::vector<FrameUniformBuffer> uniformBuffers;

  void updateUniformBuffers();

  Frame(Device &device, VmaAllocator allocator, VkDescriptorSetLayout descriptorSetLayout, const std::vector<UniformBufferBinding> &bindings);
  ~Frame();

  Frame(const Frame &) = delete;
  Frame &operator=(const Frame &) = delete;

  Frame(Frame &&other) = default;
  Frame &operator=(Frame &&) = delete; // reference member makes assignment awkward

private:
  Device &device;
  VmaAllocator allocator;
  const std::vector<UniformBufferBinding> &bindings;
};

/**
 * @brief Encapsulates a VkDescriptorSetLayout object.
 * @details Defines a descriptor set layout based on the provided list of UniformBufferBindings
 */
class DescriptorSetLayout {
public:
  VkDescriptorSetLayout descriptorSetLayout;

  DescriptorSetLayout(Device &device, const std::vector<UniformBufferBinding> &bindings);
  ~DescriptorSetLayout();

  DescriptorSetLayout(const DescriptorSetLayout &) = delete;
  DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

  DescriptorSetLayout(DescriptorSetLayout &&other) noexcept;
  DescriptorSetLayout &operator=(DescriptorSetLayout &&) = delete;

private:
  Device &device_;
  const std::vector<UniformBufferBinding> &bindings_;
};

std::vector<char> readFile(const std::string &filename);
const char *VkResultToString(VkResult result);
void PrintVkResult(VkResult result);
// DEPRECATED, USE CHECKVKRESULT2
VkResult CheckVkResult(VkResult result);
VkResult CheckVkResult2(VkResult result, const char *reason);
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createBuffer(Device &device, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);

} // namespace GFVL
