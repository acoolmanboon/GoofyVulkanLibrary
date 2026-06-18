#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstring>
#include <vector>
#include <vulkan/vulkan.h>

#include "GFVL.hpp"
using namespace GFVL;

// USER-DEFINED STUFF
namespace GFVL {
    FRAMEBUFFER::FRAMEBUFFER(DEVICE& device, SWAPCHAIN& swapchain, RENDERPASS& renderPass) : device(device) {
      this->framebuffers.resize(swapchain.imageViews.size());

      for (size_t i = 0; i < swapchain.imageViews.size(); i++) {

        VkImageView attachments[] = {
            swapchain.imageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass.renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = swapchain.extent.width,
            .height = swapchain.extent.height,
            .layers = 1};

        CheckVkResult(
            vkCreateFramebuffer(
                device.logicalDevice,
                &framebufferInfo,
                nullptr,
                &this->framebuffers[i]));
      }
    }
    void FRAMEBUFFER::recreate(SWAPCHAIN &swapchain, RENDERPASS &renderPass) {
      vkDeviceWaitIdle(device.logicalDevice);

      for (auto framebuffer : framebuffers)
        vkDestroyFramebuffer(device.logicalDevice, framebuffer, nullptr);

      framebuffers.clear();

      framebuffers.resize(swapchain.imageViews.size());

      for (size_t i = 0; i < swapchain.imageViews.size(); i++) {
        VkImageView attachments[] = {
            swapchain.imageViews[i]};

        VkFramebufferCreateInfo framebufferInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = renderPass.renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = swapchain.extent.width,
            .height = swapchain.extent.height,
            .layers = 1};

        CheckVkResult(vkCreateFramebuffer(device.logicalDevice, &framebufferInfo, nullptr, &framebuffers[i]));
      }
    }
    FRAMEBUFFER::~FRAMEBUFFER() {
        for (VkFramebuffer& framebuffer : this->framebuffers) {
            vkDestroyFramebuffer(this->device.logicalDevice, framebuffer, nullptr);
        }
    }
}
