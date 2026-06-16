#ifndef GFVL_CPP
#define GFVL_CPP
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#define DEBUG_PRINT(message) std::cout << "[GFVL] " << message << "\n";
namespace GFVL {
  const bool DEBUG_MODE = true;
  enum PREFERRED_GPU {
    PREFERRED_GPU_POWER_SAVING,
    PREFERRED_GPU_PERFORMANCE,
  };

  class DEVICE {
    public:
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    VkDeviceSize videoMemory = 0;
    uint32_t graphicsFamilyIndex = UINT32_MAX;
    uint32_t presentFamilyIndex = UINT32_MAX;

    DEVICE(VkInstance instance, VkSurfaceKHR surface, PREFERRED_GPU preference);
    ~DEVICE();
  };

  class SWAPCHAIN {
    public:
    VkSwapchainKHR swapchain{};
    VkDevice device{};
    VkPhysicalDevice physicalDevice{};
    VkSurfaceKHR surface{};
    SDL_Window *window{};

    VkFormat format{};
    VkExtent2D extent{};
    VkPresentModeKHR presentMode{};
    uint32_t imageCount{};

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    bool framebufferResized = false;

    SWAPCHAIN(const DEVICE& device, SDL_Window* window, VkSurfaceKHR surface);
    ~SWAPCHAIN();
  };

  VkInstance InitializeVkInstance(VkApplicationInfo *appInfo);

  // defined in GFVL.cpp
  std::vector<char> readFile(const std::string &filename);
  const char *VkResultToString(VkResult result);
  void PrintVkResult(VkResult result);
  VkResult CheckVkResult(VkResult result);
}

#endif