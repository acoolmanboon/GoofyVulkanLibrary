/**
 * @file GFVL.hpp
 * @brief Defines everything about GFVL.
 * @details This is probably the file you wanna include.
 */
#ifndef GFVL_CPP
#define GFVL_CPP
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#define PRINT(message) std::cout << "[GFVL] " << message << "\n";
#define THROW_EXCEPTION(reason)                 \
  do {                                          \
    std::ostringstream oss;                     \
    oss << "[GFVL] Error! Reason : " << reason; \
    throw std::runtime_error(oss.str());        \
  } while (0);
#define ASSERT(statement, message)               \
  if (statement) {                               \
    std::ostringstream oss;                      \
    oss << "[GFVL] Error! Reason : " << message; \
    throw std::runtime_error(oss.str());         \
  };
#define GOOFYVLIB_ITERATION 1 // internal application name

namespace GFVL {
const bool DEBUG_MODE = true;
class PIPELINE;
enum PREFERRED_GPU {
  PREFERRED_GPU_POWER_SAVING,
  PREFERRED_GPU_PERFORMANCE,
};

struct APPLICATION_INFO {
  const char *applicationName;
  uint32_t applicationVersion;
  int width;
  int height;
  PREFERRED_GPU preferredGPU;
};

struct UNIFORM_BUFFER_BINDING {
  size_t size;
  void *ubo;
};

struct SHADER_STAGE {
  VkShaderStageFlagBits flags;
  const char *filename;
};

class VERTEX_LAYOUT {
public:
  VkVertexInputBindingDescription binding;

  std::vector<VkVertexInputAttributeDescription> attributes;

  void addAttribute(VkFormat format, uint32_t offset);

  VERTEX_LAYOUT(uint32_t size);
};

class DEVICE {
public:
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice logicalDevice = VK_NULL_HANDLE;
  VkDeviceSize videoMemory = 0;

  uint32_t graphicsFamilyIndex = UINT32_MAX;
  uint32_t presentFamilyIndex = UINT32_MAX;

  VkQueue graphicsQueue = {};

  DEVICE(VkInstance instance, VkSurfaceKHR surface, PREFERRED_GPU preference);
  ~DEVICE();

  DEVICE(const DEVICE &) = delete;
  DEVICE &operator=(const DEVICE &) = delete;

  DEVICE(const DEVICE &&) = delete;
  DEVICE &operator=(const DEVICE &&) = delete;
};

class Semaphore {
public:
  VkSemaphore semaphore;
  Semaphore(DEVICE &device) : device_(device) {
    VkSemaphoreCreateInfo semaphoreInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(this->device_.logicalDevice, &semaphoreInfo, nullptr, &this->semaphore);
  }

  Semaphore(const Semaphore &) = delete;
  Semaphore &operator=(const Semaphore &) = delete;

  Semaphore(Semaphore &&other) noexcept : device_(other.device_), semaphore(other.semaphore) {
    other.semaphore = VK_NULL_HANDLE;
  };
  Semaphore &operator=(Semaphore &&other) {
    ASSERT(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy semaphore with different devices")
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
    vkDestroySemaphore(this->device_.logicalDevice, this->semaphore, nullptr);
  }

private:
  DEVICE &device_;
};

class Fence {
public:
  VkFence fence;
  Fence(DEVICE &device, VkFenceCreateFlags flags) : device_(device) {
    VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = flags};
    vkCreateFence(this->device_.logicalDevice, &fenceInfo, nullptr, &this->fence);
  }
  Fence(const Fence &) = delete;
  Fence &operator=(const Fence &) = delete;

  Fence(Fence &&other) noexcept : device_(other.device_), fence(other.fence) {
    other.fence = VK_NULL_HANDLE;
  };
  Fence &operator=(Fence &&other) {
    ASSERT(this->device_.logicalDevice != other.device_.logicalDevice, "Attempted to copy semaphore with different devices")
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
    vkDestroyFence(this->device_.logicalDevice, this->fence, nullptr);
  }
private:
  DEVICE& device_;
};

class SWAPCHAIN {
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
  SWAPCHAIN(DEVICE &device, SDL_Window *window, VkSurfaceKHR surface);
  ~SWAPCHAIN();

  SWAPCHAIN(const SWAPCHAIN &) = delete;
  SWAPCHAIN &operator=(const SWAPCHAIN &) = delete;

  SWAPCHAIN(const SWAPCHAIN &&) = delete;
  SWAPCHAIN &operator=(const SWAPCHAIN &&) = delete;

private:
  DEVICE &device;
};

class SHADER {
public:
  VkShaderModule shaderModule = {};
  VkShaderStageFlagBits stage;

  SHADER(DEVICE &device, VkShaderStageFlagBits stage, const char *filename);
  ~SHADER();

  SHADER(const SHADER &) = delete;
  SHADER &operator=(const SHADER &) = delete;

  SHADER(SHADER &&other) noexcept;
  SHADER &operator=(SHADER &&other) noexcept;

private:
  DEVICE &device;
};

class RENDERPASS {
public:
  VkRenderPass renderPass = {};

  RENDERPASS(DEVICE &device, SWAPCHAIN &swapchain);
  ~RENDERPASS();

  RENDERPASS(const RENDERPASS &) = delete;
  RENDERPASS &operator=(const RENDERPASS &) = delete;

  RENDERPASS(const RENDERPASS &&) = delete;
  RENDERPASS &operator=(const RENDERPASS &&) = delete;

private:
  DEVICE &device;
};

class BINDING {
public:
  DEVICE &device;
  VkBuffer buffer{};
  VkDeviceMemory memory{};
  VkDescriptorSet descriptorSet{};
  VkDescriptorSetLayoutBinding layout{};
  VkDescriptorBufferInfo bufferInfo{};
  void *data{};
  size_t size{};

  BINDING(DEVICE &device, size_t size, void *ubo, uint32_t binding);
  void update(void *ubo);
  ~BINDING();
};

class UNIFORM_BUFFER {
public:
  DEVICE &device;
  std::vector<BINDING> bindings;
  VkDescriptorSetLayout descriptorSetLayout{};
  VkDescriptorPool descriptorPool{};
  VkDescriptorSet descriptorSet{};

  UNIFORM_BUFFER(DEVICE &device, std::vector<UNIFORM_BUFFER_BINDING> &bindings);
  ~UNIFORM_BUFFER();

  BINDING &emplaceBinding(size_t size, void *ubo);
  void create();
  void bind(VkCommandBuffer &commandBuffer, PIPELINE &pipeline, uint32_t set);

  UNIFORM_BUFFER(const UNIFORM_BUFFER &) = delete;
  UNIFORM_BUFFER &operator=(const UNIFORM_BUFFER &) = delete;

  UNIFORM_BUFFER(const UNIFORM_BUFFER &&) = delete;
  UNIFORM_BUFFER &operator=(const UNIFORM_BUFFER &&) = delete;
};

class PIPELINE {
public:
  VkPipelineLayout pipelineLayout;
  VkPipeline pipeline = {};

  PIPELINE(DEVICE &device, SWAPCHAIN &swapchain, VERTEX_LAYOUT &layout, std::vector<SHADER> &shaderStages, RENDERPASS &renderPass, std::vector<VkDescriptorSetLayout> layouts);
  ~PIPELINE();

  PIPELINE(const PIPELINE &) = delete;
  PIPELINE &operator=(const PIPELINE &) = delete;

  PIPELINE(const PIPELINE &&) = delete;
  PIPELINE &operator=(const PIPELINE &&) = delete;

private:
  DEVICE &device;
};

class Framebuffer {
public:
  std::vector<VkFramebuffer> framebuffers;

  void recreate(SWAPCHAIN &swapchain, RENDERPASS &renderPass);
  Framebuffer(DEVICE &device, SWAPCHAIN &swapchain, RENDERPASS &renderPass);
  ~Framebuffer();

  Framebuffer(const Framebuffer &) = delete;
  Framebuffer &operator=(const Framebuffer &) = delete;

  Framebuffer(const Framebuffer &&) = delete;
  Framebuffer &operator=(const Framebuffer &&) = delete;

private:
  DEVICE &device;

  VkImage depthImage{};
  VkDeviceMemory depthMemory{};
  VkImageView depthImageView{};
  VkFormat depthFormat{};
};

/**
 * @class CommandPool
 * @brief Handles command pools.
 * @details This is temporarily small but it will become bigger in the future (jk idk why i made this).
 */
class CommandPool {
public:
  CommandPool(DEVICE &device, Framebuffer &framebuffer);
  void recreate(Framebuffer &framebuffer);
  ~CommandPool();

  const VkCommandBuffer &commandBuffer(size_t index) const;

  CommandPool(const CommandPool &) = delete;
  CommandPool &operator=(const CommandPool &) = delete;

  CommandPool(const CommandPool &&) = delete;
  CommandPool &operator=(const CommandPool &&) = delete;

private:
  DEVICE &device_;
  VkCommandPool commandPool_;
  std::vector<VkCommandBuffer> commandBuffers_;
};

/**
 * @class VertexBuffer
 * @brief Handles memory management for vertex buffers.
 * @details This allows for management of both in-VRAM buffers and CPU-readable data, but beware of trying to modify something with the wrong type.
 */
class VertexBuffer {
public:
  /**
   * @enum Type
   * @brief Defines memory allocation strategy.
   */
  enum class MemoryAllocation {
    HostVisibleOpportunistic, ///< Memory will be visible to CPU, but physically it is still in VRAM. Faster for GPU's that support ReBar.
    HostVisible,              ///< Memory allocated will be visible to CPU. Use for non-static meshes.
    DeviceOnly                ///< Memory allocated will be in VRAM. Use for static-meshes. Faster.
  };

  /**
   * @struct CreateInfo
   * @brief Configuration for creating VertexBuffer class.
   */
  struct CreateInfo {
    size_t size;                                           ///< Size of the allocated buffer memory in bytes.
    void *data;                                            ///< Pointer to the data to copy into the buffer.
    MemoryAllocation type = MemoryAllocation::HostVisible; ///< Type of data, see definition.
  };

  const void *data() const;            ///< Returns the pointer to the buffer data, only for memory allocation strategy of HostVisible
  const size_t size() const;           ///< Returns the allocated buffer size in buffer.
  const VkBuffer &buffer() const;      ///< Returns a reference to the Vulkan buffer handle.
  const MemoryAllocation type() const; ///< Returns the memory allocation strategy of the buffer.

  /**
   * @brief Creates a vertex buffer.
   * @param device A reference to your Device.
   * @param createinfo The creation information of the vertex buffer.
   */
  VertexBuffer(DEVICE &device, const CreateInfo &createInfo); ///< Creates a vertex buffer.

  ~VertexBuffer(); ///< Destroys a vertex buffer and frees associated memory.

  VertexBuffer(const VertexBuffer &other) = delete;            ///< Copy constructor, removed as multiple vertex buffers will have the same buffer handles.
  VertexBuffer &operator=(const VertexBuffer &other) = delete; ///< Copy assignment operator, removed as multiple vertex buffers will have the same buffer handles.

  VertexBuffer(VertexBuffer &&other) noexcept;   ///< Move constructor, allowed but it will unbind vulkan resources of old object.
  VertexBuffer &operator=(VertexBuffer &&other); ///< Move assignment operator, allowed but it will unbind vulkan resources of old object.

private:
  DEVICE &device_;              ///< Stores the device reference.
  VkBuffer buffer_;             ///< The Vulkan Buffer handle.
  VkDeviceMemory bufferMemory_; ///< The Vulkan memory handle.

  void *data_;            ///< A pointer to the buffer data. Used for HostVisible memory.
  size_t size_;           ///< Size of the memory. Used mainly for debugging.
  MemoryAllocation type_; ///< Type of the memory, not used functionalyl but used for type safety.
};

/**
 * @class Mesh
 * @brief An object containing vertice data.
 * @details Stores vertex data.
 */
class Mesh {
public:
  /**
   * @struct CreateInfo
   * @brief Configuration for creating Mesh class.
   */
  struct CreateInfo {
    size_t size;                                                                       ///< Size of the mesh in bytes.
    void *data;                                                                        ///< Pointer to mesh data.
    VertexBuffer::MemoryAllocation type = VertexBuffer::MemoryAllocation::HostVisible; ///< The memory allocation of this mesh. See documentation for details.
  };

  const size_t size() const;
  const VertexBuffer &vertexBuffer() const;

  /**
   * @brief Creates a mesh buffer.
   * @param device A reference to your Device.
   * @param createinfo The creation information of the vertex buffer.
   */
  Mesh(DEVICE &device, const CreateInfo &createInfo); ///< Creates a mesh.

  Mesh(const Mesh &other) = delete;            ///< Copy constructor, removed as multiple vertex buffers will have the same buffer handles.
  Mesh &operator=(const Mesh &other) = delete; ///< Copy assignment operator, removed as multiple vertex buffers will have the same buffer handles.

  Mesh(Mesh &&other) = default;  ///< Move constructor, allowed but it will unbind vulkan resources of old object.
  Mesh &operator=(Mesh &&other); ///< Move assignment operator, allowed but it will unbind vulkan resources of old object.

  ~Mesh(); ///< Destroys mesh and associated info

private:
  DEVICE &device_;            ///< Stores the device reference.
  size_t size_;               ///< Size of the mesh in bytes.
  VertexBuffer vertexBuffer_; ///< The buffer containing the actual memory
};

class INSTANCE {
public:
  VkInstance instance;
  SDL_Window *window;
  VkSurfaceKHR surface;
  DEVICE device;
  SWAPCHAIN swapchain;
  RENDERPASS renderPass;
  UNIFORM_BUFFER uniformBuffer;
  std::vector<SHADER> shaderStages;
  PIPELINE pipeline;
  Framebuffer framebuffer;
  CommandPool commandPool;

  INSTANCE(APPLICATION_INFO applicationInfo, VERTEX_LAYOUT &layout, std::vector<UNIFORM_BUFFER_BINDING> &bindings, std::vector<SHADER_STAGE> &stages);
  ~INSTANCE();

  INSTANCE(const INSTANCE &) = delete;
  INSTANCE &operator=(const INSTANCE &) = delete;

  INSTANCE(const INSTANCE &&) = delete;
  INSTANCE &operator=(const INSTANCE &&) = delete;
};

// defined in GFVL.cpp
std::vector<char> readFile(const std::string &filename);
const char *VkResultToString(VkResult result);
void PrintVkResult(VkResult result);
VkResult CheckVkResult(VkResult result);
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createBuffer(DEVICE &device, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);

} // namespace GFVL

#endif