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

  void recreateSwapchain(SDL_Window *window, VkSurfaceKHR surface);
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

class FRAMEBUFFER {
public:
  std::vector<VkFramebuffer> framebuffers;

  void recreate(SWAPCHAIN &swapchain, RENDERPASS &renderPass);
  FRAMEBUFFER(DEVICE &device, SWAPCHAIN &swapchain, RENDERPASS &renderPass);
  ~FRAMEBUFFER();

  FRAMEBUFFER(const FRAMEBUFFER &) = delete;
  FRAMEBUFFER &operator=(const FRAMEBUFFER &) = delete;

  FRAMEBUFFER(const FRAMEBUFFER &&) = delete;
  FRAMEBUFFER &operator=(const FRAMEBUFFER &&) = delete;

private:
  DEVICE &device;

  VkImage depthImage{};
  VkDeviceMemory depthMemory{};
  VkImageView depthImageView{};
  VkFormat depthFormat{};
};

class COMMAND_POOL {
public:
  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  COMMAND_POOL(DEVICE &device, FRAMEBUFFER &framebuffer);
  ~COMMAND_POOL();

  COMMAND_POOL(const COMMAND_POOL &) = delete;
  COMMAND_POOL &operator=(const COMMAND_POOL &) = delete;

  COMMAND_POOL(const COMMAND_POOL &&) = delete;
  COMMAND_POOL &operator=(const COMMAND_POOL &&) = delete;

private:
  DEVICE &device;
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
  enum class Type {
    HostVisible, ///< Memory allocated will be visible to CPU. Use for non-static meshes like terrain.
    DeviceOnly   ///< Memory allocated will be in VRAM. Use for static-meshes. Faster.
  };

  /**
   * @struct CreateInfo
   * @brief Configuration for creating VertexBuffer class.
   */
  struct CreateInfo {
    size_t size;                    ///< Size of the buffer in bytes.
    void *data;                     ///< Pointer to your data.
    Type type;                      ///< Type of data, see definition.
    VkCommandBuffer *commandBuffer; ///< Optional, used for static meshes.
  };

  void *data() const;      ///< Returns the pointer to data, if using HOST_VISIBLE buffer.
  size_t size() const;     ///< Re  turns size of buffer in bytes.
  const VkBuffer &buffer() const; ///< Returns the buffer
  Type type() const;       ///< Returns type of buffer.

  /**
   * @brief Creates a vertex buffer.
   * @param device A reference to your Device.
   * @param createinfo The creation information of the vertex buffer.
   */
  VertexBuffer(DEVICE &device, const CreateInfo &createInfo);

  ~VertexBuffer(); ///< Destroys a vertex buffer and frees associated memory.

  VertexBuffer(const VertexBuffer &) = delete;
  VertexBuffer &operator=(const VertexBuffer &) = delete;
  VertexBuffer(const VertexBuffer &&) = delete;
  VertexBuffer &operator=(const VertexBuffer &&) = delete;

private:
  DEVICE &device;              ///< Stores the device reference.
  VkBuffer buffer_;            ///< The buffer of the vertex buffer
  VkDeviceMemory bufferMemory; ///< The actual memory stored.

  void *data_;  ///< A pointer to the data. Used for HOST_VISIBLE memory.
  size_t size_; ///< Size of the memory. Used mainly for debugging.
  Type type_;   ///< Type of the memory, not used functionalyl but used for type safety.
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
    size_t size;                    ///< Size of the mesh in bytes.
    void *data;                     ///< Pointer to mesh data.
    VertexBuffer::Type type;        ///< The memory allocation of this mesh. See documentation for details.
    VkCommandBuffer *commandBuffer; ///< Command buffer, for static meshes.
  };

  size_t size() const;
  const VertexBuffer &vertexBuffer() const;

  /**
   * @brief Creates a mesh buffer.
   * @param device A reference to your Device.
   * @param createinfo The creation information of the vertex buffer.
   */
  Mesh(DEVICE &device, const CreateInfo &createInfo);

  ~Mesh(); ///< Destroys mesh and associated info

private:
  DEVICE &device;             ///< Stores the device reference.
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
  FRAMEBUFFER framebuffer;
  COMMAND_POOL commandPool;

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