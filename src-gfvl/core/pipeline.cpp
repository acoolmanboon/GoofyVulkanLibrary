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
#include <cstdint>

using namespace GFVL;

#define REPLACE_IF_NOT_EQUAL_TO(target, member, value) if (overrides.member != value) target.member = overrides.member
#define REPLACE_IF_NOT_NAN(target, member) if (!std::isnan(overrides.member)) target.member = overrides.member
// USER-DEFINED STUFF
namespace GFVL {
VkPipelineRasterizationStateCreateInfo Pipeline::getrasterizerCreateInfoOverridesFromConfig(Pipeline::RasterizerPreset preset, VkPipelineRasterizationStateCreateInfo overrides) {
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = {};
    switch(preset) {
        case(RasterizerPreset::Default):
            rasterizerCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0, 
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .cullMode = VK_CULL_MODE_BACK_BIT,
                .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.0f,
                .depthBiasClamp = 0.0f,
                .depthBiasSlopeFactor = 0.0f,
                .lineWidth = 1.0f};
            break;
        default:
            THROW_EXCEPTION("Invalid Rasterizer Preset!");
    }

    REPLACE_IF_NOT_EQUAL_TO(rasterizerCreateInfo, pNext, nullptr);
    REPLACE_IF_NOT_EQUAL_TO(rasterizerCreateInfo, flags, UINT32_MAX);
    REPLACE_IF_NOT_EQUAL_TO(rasterizerCreateInfo, depthBiasEnable, UINT32_MAX);
    REPLACE_IF_NOT_EQUAL_TO(rasterizerCreateInfo, rasterizerDiscardEnable, UINT32_MAX);
    REPLACE_IF_NOT_EQUAL_TO(rasterizerCreateInfo, polygonMode, VK_POLYGON_MODE_MAX_ENUM);
    REPLACE_IF_NOT_EQUAL_TO(rasterizerCreateInfo, cullMode, VK_CULL_MODE_FLAG_BITS_MAX_ENUM);
    REPLACE_IF_NOT_EQUAL_TO(rasterizerCreateInfo, frontFace, VK_FRONT_FACE_MAX_ENUM);
    REPLACE_IF_NOT_EQUAL_TO(rasterizerCreateInfo, depthBiasEnable, UINT32_MAX);

    REPLACE_IF_NOT_NAN(rasterizerCreateInfo, depthBiasConstantFactor);
    REPLACE_IF_NOT_NAN(rasterizerCreateInfo, depthBiasClamp);
    REPLACE_IF_NOT_NAN(rasterizerCreateInfo, depthBiasSlopeFactor);
    REPLACE_IF_NOT_NAN(rasterizerCreateInfo, lineWidth);
    
    return rasterizerCreateInfo;
}
Pipeline::Pipeline(Device &device, Swapchain &swapchain, VertexLayout &layout, std::vector<SHADER> &shaderStages, RENDERPASS &renderPass, std::vector<VkDescriptorSetLayout> descriptorSetLayouts, Pipeline::CreateInfo createInfo) : device(device) {
  PRINT("Attempting to create pipeline with " << shaderStages.size() << " shader stages and " << descriptorSetLayouts.size() << " layouts.");
  std::vector<VkPipelineShaderStageCreateInfo> stages(shaderStages.size());
  size_t index = 0;
  for (const SHADER& shader : shaderStages) {
    stages[index] = VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = shader.stage,
        .module = shader.shaderModule,
        .pName = "main"};
    index++;
  }

  std::vector<VkDynamicState> dynamicStates = {
      VK_DYNAMIC_STATE_VIEWPORT, // add more as needed,do later
      VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .flags = 0,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data()};

  for (int i = 0; i < descriptorSetLayouts.size(); i++) {
    if (descriptorSetLayouts[i] == VK_NULL_HANDLE) {
        std::swap(descriptorSetLayouts[i], descriptorSetLayouts.back());
        descriptorSetLayouts.pop_back();
    }
  } 

  VkPipelineLayoutCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size()),
      .pSetLayouts = descriptorSetLayouts.data()};

  CheckVkResult2(
    vkCreatePipelineLayout(device.logicalDevice, &info, nullptr, &this->pipelineLayout),
    "Failed to create pipeline layout!");

  // vertex input
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = static_cast<uint32_t>(layout.bindings.size()),
      .pVertexBindingDescriptions = layout.bindings.data(),
      .vertexAttributeDescriptionCount = static_cast<uint32_t>(layout.attributes.size()),
      .pVertexAttributeDescriptions = layout.attributes.data()};
  

  // input assembly
  VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  // viewport/scissor dynamic
  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1};

  // rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer = getrasterizerCreateInfoOverridesFromConfig(createInfo.rasterizerPreset, createInfo.rasterizerCreateInfoOverrides);

  // multisampling
  // me
  VkPipelineMultisampleStateCreateInfo multisampling{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
  };

  // color blending
  VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = VK_FALSE,
      .colorWriteMask =
          VK_COLOR_COMPONENT_R_BIT |
          VK_COLOR_COMPONENT_G_BIT |
          VK_COLOR_COMPONENT_B_BIT |
          VK_COLOR_COMPONENT_A_BIT};

  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  // pipeline
  VkPipelineDepthStencilStateCreateInfo depthStencil{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE};
  VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = stages.data(),

      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,

      .layout = pipelineLayout,
      .renderPass = renderPass.renderPass,
      .subpass = 0};

  CheckVkResult2(
    vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline),
    "Failed to create pipeline layout!");
  PRINT("Created graphics pipeline!");
}
    Pipeline::~Pipeline() {
        vkDestroyPipeline(this->device.logicalDevice, this->pipeline, nullptr);
        vkDestroyPipelineLayout(device.logicalDevice, this->pipelineLayout, nullptr);
    }
}
