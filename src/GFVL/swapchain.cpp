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
SWAPCHAIN::SWAPCHAIN(DEVICE &device, SDL_Window *window) {
    // query swapchain support
    std::vector<VkSurfaceFormatKHR> formats = {};
    std::vector<VkPresentModeKHR> presentModes = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, surface, &this->capabilities);

    uint32_t count = 0;

    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, surface, &count, nullptr);
    formats.resize(count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, surface, &count, formats.data());

    vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, surface, &count, nullptr);
    presentModes.resize(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, surface, &count, presentModes.data());

    // select swapchain config

    // pick random format at first
    this->format = formats[0].format;
    this->colorSpace = formats[0].colorSpace;
    this->presentMode = VK_PRESENT_MODE_FIFO_KHR; // immediate mode for vsync off

    // try to get ideal format
    for (const VkSurfaceFormatKHR &surfaceFormat : formats) {
      if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        this->format = surfaceFormat.format;
        this->colorSpace = surfaceFormat.colorSpace;
        break;
      }
    }

    for (const VkPresentModeKHR presentMode : presentModes) {
      if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        this->presentMode = presentMode;
        break;
      }
    }

    if (this->capabilities.currentExtent.width != UINT32_MAX) { // check if resolution isnt locked
      this->extent = this->capabilities.currentExtent;
    } else {
      int width, height;
      SDL_GetWindowSizeInPixels(window, &width, &height);
      this->extent = {(uint32_t)width, (uint32_t)height};
    }

    uint32_t imageCount = this->capabilities.minImageCount + 1;

    if (this->capabilities.maxImageCount > 0 && imageCount > this->capabilities.maxImageCount) {
      imageCount = this->capabilities.maxImageCount;
    }

    this->imageCount = imageCount;

    // ouu shii

    VkSwapchainCreateInfoKHR info{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = this->surface,
        .minImageCount = this->imageCount,
        .imageFormat = this->format,
        .imageColorSpace = this->colorSpace,
        .imageExtent = this->extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = this->capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = this->presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE};

    CheckVkResult(vkCreateSwapchainKHR(this->device, &info, nullptr, &this->swapchain));

    vkGetSwapchainImagesKHR(this->device, this->swapchain, &count, nullptr);

    this->images.resize(count);
    vkGetSwapchainImagesKHR(this->device, this->swapchain, &count, this->images.data());

    this->imageViews.resize(count);

    for (size_t i = 0; i < count; i++) {
      VkImageViewCreateInfo vi{
          .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .image = this->images[i],
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format = this->format,
          .components = {VK_COMPONENT_SWIZZLE_IDENTITY,
                         VK_COMPONENT_SWIZZLE_IDENTITY,
                         VK_COMPONENT_SWIZZLE_IDENTITY,
                         VK_COMPONENT_SWIZZLE_IDENTITY},
          .subresourceRange = {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .levelCount = 1,
              .layerCount = 1}};

      CheckVkResult(vkCreateImageView(this->device, &vi, nullptr, &this->imageViews[i]));
    }

    this->imageCount = count;
}
SWAPCHAIN::~SWAPCHAIN(){
  vkDeviceWaitIdle(this->device);

  for (VkImageView v : this->imageViews)
    vkDestroyImageView(this->device, v, nullptr);

  if (this->swapchain)
    vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);

  this->imageViews.clear();
  this->images.clear();
}
} 
