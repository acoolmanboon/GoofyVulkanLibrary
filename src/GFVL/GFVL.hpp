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
namespace GFVL {
  const bool DEBUG_MODE = true;
  
  enum PREFERRED_GPU {
    PREFERRED_GPU_POWER_SAVING,
    PREFERRED_GPU_PERFORMANCE,
  };

  struct VERTEX_LAYOUT {
    VkVertexInputBindingDescription binding{
        .binding = 0,
        .stride = 0,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    std::vector<VkVertexInputAttributeDescription> attributes;

    void addAttribute(VkFormat format, uint32_t offset) {
      attributes.push_back({.location = static_cast<uint32_t>(attributes.size()),
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

    SWAPCHAIN(DEVICE& device, SDL_Window* window, VkSurfaceKHR surface);
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

    SHADER(SHADER&& other) noexcept;
    SHADER &operator=(SHADER&& other) noexcept;

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

  class PIPELINE {
  public:
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline = {};

    PIPELINE(DEVICE &device, SWAPCHAIN &swapchain, VERTEX_LAYOUT &layout, std::vector<SHADER> &shaderStages, RENDERPASS& renderPass);
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

    FRAMEBUFFER(DEVICE& device, SWAPCHAIN &swapchain, RENDERPASS &renderPass);
    ~FRAMEBUFFER();

    FRAMEBUFFER(const FRAMEBUFFER &) = delete;
    FRAMEBUFFER &operator=(const FRAMEBUFFER &) = delete;

    FRAMEBUFFER(const FRAMEBUFFER &&) = delete;
    FRAMEBUFFER &operator=(const FRAMEBUFFER &&) = delete;

  private:
    DEVICE &device;
  };

  class COMMAND_POOL {
  public:
    VkCommandPool commandPool;
    
    COMMAND_POOL(DEVICE& device);
    ~COMMAND_POOL();

    COMMAND_POOL(const COMMAND_POOL &) = delete;
    COMMAND_POOL &operator=(const COMMAND_POOL &) = delete;

    COMMAND_POOL(const COMMAND_POOL &&) = delete;
    COMMAND_POOL &operator=(const COMMAND_POOL &&) = delete;

  private:
    DEVICE &device;
  };
  VkInstance InitializeVkInstance(VkApplicationInfo *appInfo);
  // defined in GFVL.cpp
  std::vector<char> readFile(const std::string &filename);
  const char *VkResultToString(VkResult result);
  void PrintVkResult(VkResult result);
  VkResult CheckVkResult(VkResult result);
}

#endif