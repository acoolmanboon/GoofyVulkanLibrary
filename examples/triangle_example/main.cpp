// this is just an example script so use it anywhere

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <stdexcept>

#include <GFVL.hpp>
#include <vulkan/vulkan_core.h>

// This defines how your vertices will work in GFVL.
// Later in the script, the structure of vertice will be defined to GFVL.
struct vertice {
  float position[2];
  float color[3];
};

int main() {
  // This is where you tell GFVL where your shaders are. You need to compile the shader languages first.
  // I know that there are more shader stages available but for now I haven't learned that yet so I am only certain that it supports only a vertex and fragment shader for now.
  std::vector<GFVL::ShaderStage> shaderStages = {
      {.flags = VK_SHADER_STAGE_VERTEX_BIT,
       .filename = "vertex_shader.vert.spv"},
      {.flags = VK_SHADER_STAGE_FRAGMENT_BIT,
       .filename = "fragment_shader.frag.spv"}};

  std::vector<GFVL::UniformBufferBinding> bindings;
  // Now we define the layout of the "vertice" struct to GFVL.
  GFVL::VertexLayout layout = {
    .bindings = {
     {
      .binding = 0,
      .stride = static_cast<uint32_t>(sizeof(vertice)),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
     } 
    },
    .attributes = {
      {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(vertice, position)
      },
      {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(vertice, color)  
      }
    }
  };

  // This defines our actual application information.
  GFVL::AppInfo appInfo = {
      .applicationName = "Hello, Triangle!", // For now, this just sets the window name. 
      .applicationVersion = 1, // Arbitrary version number
      .width = 800, // The starting width of the window.
      .height = 600, // The starting height of the window
      .preferredGPU = GFVL::PreferredGPU::PowerSaving};

  GFVL::Pipeline::CreateInfo pipelineCreateInfo;
  GFVL::Instance GFVLinstance(appInfo, layout, bindings, shaderStages, pipelineCreateInfo); // With our initialization logic done, we create the instance.

  std::vector<vertice> triangle = {
      {.position = {0.0f, -0.5f}, .color = {1.0f, 0.0f, 0.0f}},
      {.position = {-0.5f, 0.5f}, .color = {0.0f, 1.0f, 0.0f}},
      {.position = {0.5f, 0.5f}, .color = {0.0f, 0.0f, 1.0f}},
  };

  GFVL::Mesh triangleMesh = GFVLinstance.createMesh(GFVL::Mesh::CreateInfo{
      .verticeDataSize = triangle.size() * sizeof(vertice),
      .verticeCount = static_cast<uint32_t>(triangle.size()),
      .verticeData = triangle.data(),
      .indiceType = GFVL::Mesh::IndiceDataType::NotDefined,
      .memoryAllocation = GFVL::MeshBuffer::MemoryAllocation::DeviceOnly});

  while (GFVLinstance.inputState.isRunning()) { // Make sure to encase the while loop in the .isRunning check.
    GFVLinstance.inputState.pollInputs(); // We need to call pollInputs first in every frame to 

    GFVLinstance.beginFrame();
    GFVLinstance.renderMesh(triangleMesh);
    GFVLinstance.endFrame();
  }

  return 0;
}