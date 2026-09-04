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
#include <GFVL_definition.hpp>
#include <GFVL_core.hpp>
#include <cstddef>

using namespace GFVL;

namespace GFVL {

VkFormat Framebuffer::getDepthFormat() {
  std::vector<VkFormat> candidates = {
  VK_FORMAT_D32_SFLOAT, // 32-bit flboating point, best if supported
  VK_FORMAT_X8_D24_UNORM_PACK32, // 24-bit depth with 8 unused bit
  VK_FORMAT_D16_UNORM, // 16-bit

  // has depth but also stencil, i dont think we got stencils yet
  VK_FORMAT_D32_SFLOAT_S8_UINT,
  VK_FORMAT_D24_UNORM_S8_UINT,
  VK_FORMAT_D16_UNORM_S8_UINT,
  };

  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(device_.physicalDevice, format, &props);

    if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
      return format;
  }

  THROW_EXCEPTION("No depth format found.");
}


VkImage Framebuffer::createDepthImage(const Swapchain &swapchain, VmaAllocation &imageMemory) {
  VkImage image;

  VkImageCreateInfo imageCreateInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = depthFormat_,
      .extent = {swapchain.extent.width, swapchain.extent.height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  VkMemoryRequirements memReq;
  vkGetImageMemoryRequirements(device_.logicalDevice, image, &memReq);

  VmaAllocationCreateInfo allocationCreateInfo{
      .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
  };

  CheckVkResult2(vmaCreateImage(allocator_, &imageCreateInfo,
                                &allocationCreateInfo, &image, &imageMemory,
                                nullptr),
                 "Failed to create a framebuffer depth image!");

  return image;
}

VkImageView Framebuffer::createDepthImageView() {
  VkImageViewCreateInfo viewInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = depthImage_,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = depthFormat_,
      .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  VkImageView view;
  CheckVkResult2(
      vkCreateImageView(device_.logicalDevice, &viewInfo, nullptr, &view),
      "Failed to create framebuffer depth image view!");
  return view;
}

Framebuffer::Framebuffer(Device &device, Swapchain &swapchain, RENDERPASS &renderPass, VmaAllocator allocator) : device_(device),
                                                                                                                 allocator_(allocator),
                                                                                                                 depthFormat_(getDepthFormat()),
                                                                                                                 depthImage_(createDepthImage(swapchain, depthImageMemory_)),
                                                                                                                 depthImageView_(createDepthImageView())

{
  framebuffers.resize(swapchain.imageViews.size());

  for (size_t i = 0; i < swapchain.imageViews.size(); i++) {

    VkImageView attachments[] = {swapchain.imageViews[i], depthImageView_};

    VkFramebufferCreateInfo info{.sType =
                                     VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                                 .renderPass = renderPass.renderPass,
                                 .attachmentCount = 2,
                                 .pAttachments = attachments,
                                 .width = swapchain.extent.width,
                                 .height = swapchain.extent.height,
                                 .layers = 1};

    CheckVkResult2(vkCreateFramebuffer(device.logicalDevice, &info, nullptr,
                                       &framebuffers[i]),
                   "Failed to create framebuffer!");
  }
}
Framebuffer::Framebuffer(Framebuffer &&other) noexcept : device_(other.device_),
                                                         framebuffers(other.framebuffers),
                                                         allocator_(other.allocator_),
                                                         depthImage_(other.depthImage_),
                                                         depthImageMemory_(other.depthImageMemory_),
                                                         depthImageView_(other.depthImageView_),
                                                         depthFormat_(other.depthFormat_) {
  other.framebuffers.clear();
  other.depthImage_ = nullptr;
  other.depthImageMemory_ = nullptr;
  other.depthImageView_ = nullptr;
}

Framebuffer &Framebuffer::operator=(Framebuffer &&other) {
  if (this == &other) 
    return *this;
  
  if (this->device_ != other.device_)
    THROW_EXCEPTION("Attempted to use move assignment operator on two frame buffers with different devices");

  vkDeviceWaitIdle(device_.logicalDevice);

  for (VkFramebuffer framebuffer : framebuffers)
    if (framebuffer != nullptr)
      vkDestroyFramebuffer(device_.logicalDevice, framebuffer, nullptr);

  if (depthImageView_)
    vkDestroyImageView(device_.logicalDevice, depthImageView_, nullptr);

  if (depthImage_)
    vmaDestroyImage(allocator_, depthImage_, depthImageMemory_);

  this->framebuffers = other.framebuffers;
  other.framebuffers.clear();

  this->allocator_ = other.allocator_;

  this->depthImage_ = other.depthImage_;
  other.depthImage_ = nullptr;

  this->depthImageMemory_ = other.depthImageMemory_;
  other.depthImageMemory_ = nullptr;

  this->depthImageView_ = other.depthImageView_;
  other.depthImageView_ = nullptr;
  
  this->depthFormat_ = other.depthFormat_;
  return *this;
}

Framebuffer::~Framebuffer() {
  vkDeviceWaitIdle(device_.logicalDevice);

  for (VkFramebuffer framebuffer : framebuffers)
    if (framebuffer != nullptr) 
      vkDestroyFramebuffer(device_.logicalDevice, framebuffer, nullptr);

  if (depthImageView_)
    vkDestroyImageView(device_.logicalDevice, depthImageView_, nullptr);

  if (depthImage_)
    vmaDestroyImage(allocator_, depthImage_, depthImageMemory_);
}
}