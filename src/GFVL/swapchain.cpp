#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL;

namespace GFVL {
void pickFormat(DEVICE &device, VkSurfaceKHR surface, VkFormat& format, VkColorSpaceKHR& colorSpace) {
  std::vector<VkSurfaceFormatKHR> formats;
  uint32_t count = 0;
  CheckVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, surface, &count, nullptr));
  formats.resize(count);
  CheckVkResult(vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, surface, &count, formats.data()));
  if (formats.empty())
    THROW_EXCEPTION("No VKFormats found.. What??")

  // pick random format at first

  format = formats[0].format;
  colorSpace = formats[0].colorSpace;

  // try to get ideal format
  for (const VkSurfaceFormatKHR &surfaceFormat : formats) {
    if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      PRINT("Found ideal format!")
      format = surfaceFormat.format;
      colorSpace = surfaceFormat.colorSpace;
      break;
    }
  }
}
VkPresentModeKHR pickPresentMode(DEVICE &device, VkSurfaceKHR surface) {
  uint32_t count = 0;

  std::vector<VkPresentModeKHR> presentModes;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, surface, &count, nullptr);
  presentModes.resize(count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(device.physicalDevice, surface, &count, presentModes.data());
  if (presentModes.empty())
    THROW_EXCEPTION("No VkPresentModeKHR's found.. What??")

  VkPresentModeKHR presentMode;
  presentMode = presentModes[0];
  for (const VkPresentModeKHR& mode : presentModes) {
    if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
  }
  return presentMode;
}
SWAPCHAIN::SWAPCHAIN(DEVICE& device, SDL_Window *window, VkSurfaceKHR surface) : device(device), presentMode(pickPresentMode(device, surface)) {
    // giggity
    PRINT("Attempting to create swapchain")

    VkFormat format;
    VkColorSpaceKHR colorSpace;
    pickFormat(this->device, surface, format, colorSpace);

    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, surface, &capabilities);

    VkExtent2D extent;
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
    uint32_t count = 0;
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
    PRINT("created swapchain")
}
void SWAPCHAIN::recreateSwapchain(SDL_Window *window, VkSurfaceKHR surface) {
  vkDeviceWaitIdle(device.logicalDevice);

  for (auto view : imageViews)
    vkDestroyImageView(device.logicalDevice, view, nullptr);

  imageViews.clear();
  images.clear();

  if (swapchain != VK_NULL_HANDLE)
    vkDestroySwapchainKHR(device.logicalDevice, swapchain, nullptr);

  VkSurfaceCapabilitiesKHR capabilities{};
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physicalDevice, surface, &capabilities);

  int width = 0;
  int height = 0;

  SDL_GetWindowSizeInPixels(window, &width, &height);

  while (width == 0 || height == 0) {
    SDL_GetWindowSizeInPixels(window, &width, &height);
    SDL_PumpEvents();
  }

  std::vector<VkSurfaceFormatKHR> formats;
  uint32_t count = 0;

  vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, surface, &count, nullptr);
  formats.resize(count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device.physicalDevice, surface, &count, formats.data());

  VkFormat format = formats[0].format;
  VkColorSpaceKHR colorSpace = formats[0].colorSpace;

  for (const auto &surfaceFormat : formats) {
    if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      format = surfaceFormat.format;
      colorSpace = surfaceFormat.colorSpace;
      break;
    }
  }

  VkExtent2D extent;

  if (capabilities.currentExtent.width != UINT32_MAX)
    extent = capabilities.currentExtent;
  else
    extent = {(uint32_t)width, (uint32_t)height};

  uint32_t imageCount = capabilities.minImageCount + 1;

  if (capabilities.maxImageCount > 0 &&
      imageCount > capabilities.maxImageCount)
    imageCount = capabilities.maxImageCount;

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

  if (device.graphicsFamilyIndex != device.presentFamilyIndex)
    info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

  CheckVkResult(vkCreateSwapchainKHR(device.logicalDevice, &info, nullptr, &swapchain));

  vkGetSwapchainImagesKHR(device.logicalDevice, swapchain, &count, nullptr);

  images.resize(count);

  vkGetSwapchainImagesKHR(device.logicalDevice, swapchain, &count, images.data());

  imageViews.resize(count);

  for (size_t i = 0; i < count; i++) {
    VkImageViewCreateInfo vi{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = images[i],
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY,
            VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1}};

    CheckVkResult(vkCreateImageView(device.logicalDevice, &vi, nullptr, &imageViews[i]));
  }

  this->format = format;
  this->extent = extent;
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
