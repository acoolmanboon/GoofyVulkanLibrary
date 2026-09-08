/*
GoofyVulkanLibrary. A vulkan wrapper, designed to allow users to code Vulkan
applications without high boilerplate. Copyright (C) 2026 acoolmanboon

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
#include <GFVL_core.hpp>
#include <GFVL_definition.hpp>
#include <GFVL_vkFunctionPointers.hpp>
#include <cstddef>


using namespace GFVL;

namespace GFVL {

VkImage Framebuffer::createDepthImage(const Swapchain &swapchain) {
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

  VmaAllocationCreateInfo allocationCreateInfo{
      .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
  };

  CheckVkResult2(vmaCreateImage(allocator_, &imageCreateInfo,
                                &allocationCreateInfo, &image, &depthImageMemory_,
                                nullptr),
                 "Failed to create a framebuffer depth image!");

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
  VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_IMAGE,
      .objectHandle = reinterpret_cast<uint64_t>(image),
      .pObjectName = "VkImage in Framebuffer class"};
  CheckVkResult2(
      VulkanFunctionPointers::vkSetDebugUtilsObjectNameEXT(
          device_.logicalDevice, &debugUtilsObjectNameInfo),
      "Failed to set debug utils name for VkImage in Framebuffer class!");
#endif

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

  VkImageView imageView;
  CheckVkResult2(
      vkCreateImageView(device_.logicalDevice, &viewInfo, nullptr, &imageView),
      "Failed to create framebuffer depth image view!");

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
  VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
      .pNext = nullptr,
      .objectType = VK_OBJECT_TYPE_IMAGE_VIEW,
      .objectHandle = reinterpret_cast<uint64_t>(imageView),
      .pObjectName = "VkImageView in Framebuffer class"};
  CheckVkResult2(
      VulkanFunctionPointers::vkSetDebugUtilsObjectNameEXT(
          device_.logicalDevice, &debugUtilsObjectNameInfo),
      "Failed to set debug utils name for VkImageView in Framebuffer class!");
#endif

  return imageView;
}

Framebuffer::Framebuffer(Device &device, Swapchain &swapchain,
                         RENDERPASS &renderPass, VmaAllocator allocator,
                         VkFormat depthFormat)
    : device_(device), allocator_(allocator), depthFormat_(depthFormat),
      depthImage_(createDepthImage(swapchain)),
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

#ifdef GFVL_ENABLE_VK_DEBUG_UTILS_EXTENSION
    VkDebugUtilsObjectNameInfoEXT debugUtilsObjectNameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = VK_OBJECT_TYPE_FRAMEBUFFER,
        .objectHandle = reinterpret_cast<uint64_t>(framebuffers[i]),
        .pObjectName = "VkFramebuffer in Framebuffer class"};
    CheckVkResult2(VulkanFunctionPointers::vkSetDebugUtilsObjectNameEXT(
                       device_.logicalDevice, &debugUtilsObjectNameInfo),
                   "Failed to set debug utils name for VkFramebuffer in "
                   "Framebuffer class!");
#endif
  }
}
Framebuffer::Framebuffer(Framebuffer &&other) noexcept
    : device_(other.device_), framebuffers(other.framebuffers),
      allocator_(other.allocator_), depthImage_(other.depthImage_),
      depthImageMemory_(other.depthImageMemory_),
      depthImageView_(other.depthImageView_), depthFormat_(other.depthFormat_) {
  other.framebuffers.clear();
  other.depthImage_ = nullptr;
  other.depthImageMemory_ = nullptr;
  other.depthImageView_ = nullptr;
}

Framebuffer &Framebuffer::operator=(Framebuffer &&other) {
  if (this == &other)
    return *this;

  if (this->device_ != other.device_)
    THROW_EXCEPTION("Attempted to use move assignment operator on two frame "
                    "buffers with different devices");

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
} // namespace GFVL