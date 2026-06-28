#ifndef GFVL_CPP
#define GFVL_CPP
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#define DEBUG_PRINT(message) std::cout << "[GFVL] " << message << "\n";
#define ERROR(message) throw std::runtime_error(message);
#define GOOFYVLIB_ITERATION 1 // internal application name

namespace GFVL {
const bool DEBUG_MODE = true;
class PIPELINE;
enum PREFERRED_GPU {
  PREFERRED_GPU_POWER_SAVING,
  PREFERRED_GPU_PERFORMANCE,
};

enum VERTEX_BUFFER_TYPE {
  VERTEX_BUFFER_TYPE_CPU_READABLE,
  VERTEX_BUFFER_TYPE_STATIC
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
  void* ubo;
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

  BINDING& emplaceBinding(size_t size, void *ubo);
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

class VERTEX_BUFFER {
public:
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  VkDeviceSize size;
  void *data;
  VERTEX_BUFFER_TYPE type;
  
  VERTEX_BUFFER(DEVICE &device, VkDeviceSize size, void *inputData);
  ~VERTEX_BUFFER();

  VERTEX_BUFFER(const VERTEX_BUFFER &) = delete;
  VERTEX_BUFFER &operator=(const VERTEX_BUFFER &) = delete;

  VERTEX_BUFFER(const VERTEX_BUFFER &&) = delete;
  VERTEX_BUFFER &operator=(const VERTEX_BUFFER &&) = delete;

private:
  DEVICE &device;
  VkBufferCreateInfo bufferInfo;
};

class MESH {
public:
  size_t vertice_size;
  uint32_t vertice_count;
  size_t mesh_size;
  VERTEX_BUFFER vertexBuffer;

  MESH(void* data, uint32_t size, DEVICE& device);
  ~MESH();

private:
  DEVICE& device;
};

class INSTANCE {
public:
  VkInstance instance;
  SDL_Window* window;
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