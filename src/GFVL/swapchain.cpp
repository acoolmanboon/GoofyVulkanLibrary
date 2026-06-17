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

#include "GFVL.hpp"
using namespace GFVL;

namespace GFVL {
SWAPCHAIN::SWAPCHAIN(DEVICE& device, SDL_Window *window, VkSurfaceKHR surface) : device(device) {
    // giggity
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, surface, &capabilities);

    uint32_t count = 0;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, surface, &count, nullptr);
    formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, surface, &count, formats.data());
    if (formats.empty())
      throw std::runtime_error("[GFVL] No surface formats available.");

    vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, surface, &count, nullptr);
    presentModes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, surface, &count, presentModes.data());

    VkFormat format;
    VkColorSpaceKHR colorSpace;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;

    // pick random format at first
    
    format = formats[0].format;
    colorSpace = formats[0].colorSpace;

    // try to get ideal format
    for (const VkSurfaceFormatKHR &surfaceFormat : formats) {
      if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        format = surfaceFormat.format;
        colorSpace = surfaceFormat.colorSpace;
        break;
      }
    }

    presentMode = VK_PRESENT_MODE_FIFO_KHR; // immediate mode for vsync off

    if (capabilities.currentExtent.width != UINT32_MAX) {
      extent = capabilities.currentExtent;
    } else {
      int width, height;
      SDL_GetWindowSizeInPixels(window, &width, &height);
      extent = {(uint32_t)width, (uint32_t)height};
    }

    uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
      imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = format,
        .imageColorSpace = colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE};

    if (device.graphicsFamilyIndex != device.presentFamilyIndex) {
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    }
    CheckVkResult(vkCreateSwapchainKHR(this->device.logicalDevice, &info, nullptr, &this->swapchain));
    vkGetSwapchainImagesKHR(this->device.logicalDevice, this->swapchain, &count, nullptr);

    this->images.resize(count);
    vkGetSwapchainImagesKHR(this->device.logicalDevice, this->swapchain, &count, this->images.data());

    this->imageViews.resize(count);

    for (size_t i = 0; i < count; i++) {
      VkImageViewCreateInfo vi{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .image = this->images[i],
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format = format,
          .components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                         VK_COMPONENT_SWIZZLE_IDENTITY,
                         VK_COMPONENT_SWIZZLE_IDENTITY,
                         VK_COMPONENT_SWIZZLE_IDENTITY},
          .subresourceRange = {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .levelCount = 1,
              .layerCount = 1}};

      CheckVkResult(vkCreateImageView(this->device.logicalDevice, &vi, nullptr, &this->imageViews[i]));
    }

    this->format = format;
    this->extent = extent;
    this->presentMode = presentMode;
    this->imageCount = count;
}
SWAPCHAIN::~SWAPCHAIN() {
  if (device.logicalDevice == VK_NULL_HANDLE)
    return;

  vkDeviceWaitIdle(device.logicalDevice);

  for (auto view : imageViews)
    vkDestroyImageView(device.logicalDevice, view, nullptr);

  if (swapchain != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(device.logicalDevice, swapchain, nullptr);
}
}
