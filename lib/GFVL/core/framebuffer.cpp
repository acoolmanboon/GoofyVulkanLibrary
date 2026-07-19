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
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "../lib/GFVL_core.hpp"

using namespace GFVL;

namespace {

  
VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
  std::vector<VkFormat> candidates = {
      VK_FORMAT_D32_SFLOAT,
      VK_FORMAT_D32_SFLOAT_S8_UINT,
      VK_FORMAT_D24_UNORM_S8_UINT};

  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

    if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
      return format;
  }

  throw std::runtime_error("No supported depth format found");
}

void createImage(
    VkDevice device,
    VkPhysicalDevice physicalDevice,
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageUsageFlags usage,
    VkImage &image,
    VkDeviceMemory &memory) {

  VkImageCreateInfo imageInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {width, height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  CheckVkResult(vkCreateImage(device, &imageInfo, nullptr, &image));

  VkMemoryRequirements memReq;
  vkGetImageMemoryRequirements(device, image, &memReq);

  VkMemoryAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memReq.size,
      .memoryTypeIndex = GFVL::findMemoryType(
          physicalDevice,
          memReq.memoryTypeBits,
          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};

  CheckVkResult(vkAllocateMemory(device, &allocInfo, nullptr, &memory));
  vkBindImageMemory(device, image, memory, 0);
  PRINT("succesfully created image!")
}

VkImageView createImageView(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspect) {

  VkImageViewCreateInfo viewInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .subresourceRange = {
          .aspectMask = aspect,
          .baseMipLevel = 0,
          .levelCount = 1,
          .baseArrayLayer = 0,
          .layerCount = 1}};

  VkImageView view;
  CheckVkResult(vkCreateImageView(device, &viewInfo, nullptr, &view));
  return view;
}

} // namespace

Framebuffer::Framebuffer(DEVICE &device, Swapchain &swapchain, RENDERPASS &renderPass)
    : device(device) {

  auto findDepthFormat = [&](VkPhysicalDevice phys) -> VkFormat {
    std::vector<VkFormat> candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT};

    for (auto f : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(phys, f, &props);

      if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        return f;
    }

    throw std::runtime_error("No depth format found");
  };

  auto createImage = [&](uint32_t w, uint32_t h, VkFormat format,
                         VkImageUsageFlags usage,
                         VkImage &image,
                         VkDeviceMemory &memory) {
    VkImageCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {w, h, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

    CheckVkResult(vkCreateImage(device.logicalDevice, &info, nullptr, &image));

    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(device.logicalDevice, image, &req);

    VkMemoryAllocateInfo alloc{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = req.size,
        .memoryTypeIndex = GFVL::findMemoryType(
            device.physicalDevice,
            req.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};

    CheckVkResult(vkAllocateMemory(device.logicalDevice, &alloc, nullptr, &memory));
    vkBindImageMemory(device.logicalDevice, image, memory, 0);
  };

  auto createView = [&](VkImage image, VkFormat format) -> VkImageView {
    VkImageViewCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1}};

    VkImageView view;
    CheckVkResult(vkCreateImageView(device.logicalDevice, &info, nullptr, &view));
    return view;
  };

  depthFormat = findDepthFormat(device.physicalDevice);

  createImage(
      swapchain.extent.width,
      swapchain.extent.height,
      depthFormat,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      depthImage,
      depthMemory);

  depthImageView = createView(depthImage, depthFormat);

  recreate(swapchain, renderPass);
}

void Framebuffer::recreate(Swapchain &swapchain, RENDERPASS &renderPass) {
  vkDeviceWaitIdle(device.logicalDevice);

  for (auto fb : framebuffers)
    vkDestroyFramebuffer(device.logicalDevice, fb, nullptr);

  framebuffers.clear();

  vkDestroyImageView(device.logicalDevice, depthImageView, nullptr);
  vkDestroyImage(device.logicalDevice, depthImage, nullptr);
  vkFreeMemory(device.logicalDevice, depthMemory, nullptr);

  depthFormat = VK_FORMAT_D32_SFLOAT; // idk
  
  auto createImage = [&](uint32_t w, uint32_t h) {
    VkImageCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = depthFormat,
        .extent = {w, h, 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

    CheckVkResult(vkCreateImage(device.logicalDevice, &info, nullptr, &depthImage));

    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(device.logicalDevice, depthImage, &req);

    VkMemoryAllocateInfo alloc{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = req.size,
        .memoryTypeIndex = GFVL::findMemoryType(
            device.physicalDevice,
            req.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};

    CheckVkResult(vkAllocateMemory(device.logicalDevice, &alloc, nullptr, &depthMemory));
    vkBindImageMemory(device.logicalDevice, depthImage, depthMemory, 0);
  };

  auto createView = [&]() {
    VkImageViewCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = depthImage,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = depthFormat,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1}};

    CheckVkResult(vkCreateImageView(device.logicalDevice, &info, nullptr, &depthImageView));
  };

  createImage(swapchain.extent.width, swapchain.extent.height);
  createView();

  framebuffers.resize(swapchain.imageViews.size());

  for (size_t i = 0; i < swapchain.imageViews.size(); i++) {

    VkImageView attachments[] = {
        swapchain.imageViews[i],
        depthImageView};

    VkFramebufferCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass.renderPass,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .width = swapchain.extent.width,
        .height = swapchain.extent.height,
        .layers = 1};

    CheckVkResult(vkCreateFramebuffer(
        device.logicalDevice,
        &info,
        nullptr,
        &framebuffers[i]));
  }
}

Framebuffer::~Framebuffer() {
  vkDeviceWaitIdle(device.logicalDevice);

  for (auto fb : framebuffers)
    vkDestroyFramebuffer(device.logicalDevice, fb, nullptr);

  vkDestroyImageView(device.logicalDevice, depthImageView, nullptr);
  vkDestroyImage(device.logicalDevice, depthImage, NULL);
  vkFreeMemory(device.logicalDevice, depthMemory, NULL);
}