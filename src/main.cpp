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

#include "../lib/GFVL/include/GFVL.hpp"
#include "../lib/GFVL/lib/GFVL_core.hpp"
#include "PerlinNoise.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <glm/glm.hpp>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define print(message) std::cout << message << "\n";
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// Remove-Item build -Recurse -Force; cmake -B build -DCMAKE_PREFIX_PATH=C:/msys64/ucrt64 -G Ninja
// cmake --build build --verbose; build/main.exe
// glslc src/vertex_shader.vert -o src/vertex_shader.spv
// glslc src/fragment_shader.frag -o src/fragment_shader.spv

// cmake -B build -DCMAKE_BUILD_TYPE=Debug
// cmake -B build -DCMAKE_BUILD_TYPE=Release
// cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
// rm -Force build/main.exe; rm -Force src/fragment_shader.spv; rm -Force src/vertex_shader.spv; glslc src/vertex_shader.vert -o src/vertex_shader.spv; glslc src/fragment_shader.frag -o src/fragment_shader.spv;  cmake --build build --verbose; build/main.exe
// USER-DEFINED STUFF

struct vertice {
  float position[3];
  float normal[3];
  float color[3];
};
struct CameraUBO {
  glm::mat4 MVP;
  alignas(16) glm::vec3 viewPos;
  float padding;
};
struct LightingUBO {
  alignas(16) glm::vec3 lightPos;
  alignas(16) glm::vec3 lightColor;
};
void insertCube(glm::vec3 position, glm::vec3 color, glm::vec3 scale, std::vector<vertice> &vertices) {
  std::vector<vertice> cube = {
      // Front face (z = -0.5)
      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},

      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},

      // Back face (z = 0.5)
      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},

      // Left face (x = -0.5)
      {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
      {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},

      {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
      {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},

      // Right face (x = 0.5)
      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},

      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},

      // Bottom face (y = -0.5)
      {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
      {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},

      {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
      {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
      {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},

      // Top face (y = 0.5)
      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},

      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}}};

  for (vertice &vert : cube) {
    vert.position[0] *= scale.x;
    vert.position[1] *= scale.y;
    vert.position[2] *= scale.z;

    vert.position[0] += position.x;
    vert.position[1] += position.y;
    vert.position[2] += position.z;

    vert.color[0] = color.x;
    vert.color[1] = color.y;
    vert.color[2] = color.z;
  }

  vertices.insert(vertices.end(), cube.begin(), cube.end());
} 
int main() {
  if (!SDL_Init(SDL_INIT_VIDEO))
    throw std::runtime_error(SDL_GetError());

  // owned by user
  std::vector<GFVL::SHADER_STAGE> shaderStages = {
      {.flags = VK_SHADER_STAGE_VERTEX_BIT,
       .filename = "src/vertex_shader.spv"},
      {.flags = VK_SHADER_STAGE_FRAGMENT_BIT,
       .filename = "src/fragment_shader.spv"}};

  CameraUBO camera;
  LightingUBO lighting = {.lightPos = glm::vec3(0.0f, 0.0f, 0.0f), .lightColor = glm::vec3(1.0f, 1.0f, 1.0f)};

  std::vector<GFVL::UNIFORM_BUFFER_BINDING> bindings = {
      {.size = sizeof(CameraUBO),
       .ubo = &camera},
      {.size = sizeof(LightingUBO),
       .ubo = &lighting}};

  GFVL::VERTEX_LAYOUT layout(sizeof(vertice));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, position));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, normal));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, color));

  GFVL::APPLICATION_INFO appInfo = {
      .applicationName = "GoofyVLib example",
      .applicationVersion = 1,
      .width = 600,
      .height = 800,
      .preferredGPU = GFVL::PREFERRED_GPU_POWER_SAVING};

  GFVL::INSTANCE GFVLinstance(appInfo, layout, bindings, shaderStages);
  /*
  std::vector<vertice> vertices;
  int start = -7;
  int end = 7;
  int steps = end - start;
  int cubes = steps * steps * steps;

  float xI = 0;
  float yI = 0;
  float zI = 0;

  float spacing = 50.0f;
  float scale = 20.0f;

  for (int x = start; x < end; x++) {
    for (int y = start; y < end; y++) {
      for (int z = start; z < end; z++) {
        if (x == 0 && y == 0 && z == 0)
          continue;
        insertCube(glm::vec3(
                       static_cast<float>(x) * spacing,
                       static_cast<float>(y) * spacing,
                       static_cast<float>(z) * spacing),
                   glm::vec3(
                       xI / steps,
                       yI / steps,
                       zI / steps),
                   glm::vec3(scale), vertices);
        // print("xI : " << (xI/steps) << "yI : " << (yI/steps) << "zI : " << (zI/steps))
        zI++;
      }
      zI = 0;
      yI++;
    }
    yI = 0;
    xI++;
  }
  GFVLinstance.meshesToRender.emplace_back(GFVL::Mesh(
      GFVLinstance.device,
      GFVL::Mesh::CreateInfo{.size = vertices.size() * sizeof(vertice), .data = vertices.data(), .memoryAllocation = GFVL::VertexBuffer::MemoryAllocation::DeviceOnly}));
  */
  
  std::vector<vertice> cubeOFDeath;
  insertCube(glm::vec3(12.5f, 2.0f, -35.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(10, 10, 10), cubeOFDeath);
  //GFVLinstance.meshesToRender.emplace_back(GFVL::Mesh(GFVLinstance.device, GFVL::Mesh::CreateInfo{.size = cubeOFDeath.size() * sizeof(vertice), .data = cubeOFDeath.data(), .memoryAllocation = GFVL::VertexBuffer::MemoryAllocation::DeviceOnly}));

  // Please delete your repository ahh code
  constexpr unsigned int width = 250;
  constexpr unsigned int length = 250;
  constexpr float spacing = 10.0f;
  constexpr float frequency = 0.01f;
  constexpr float amplitude = 50.0f;
  constexpr float persistence = 0.8f;

  const siv::PerlinNoise::seed_type seed = 123456u;
  const siv::PerlinNoise perlin{seed};

  std::vector<vertice> terrain;
  terrain.reserve((width - 1) * (length - 1) * 6);

  auto sampleHeight = [&](float x, float z) -> float {
    return static_cast<float>(
        perlin.octave2D(x * frequency, z * frequency, 4, persistence) * amplitude);
  };

  auto makeVertex = [](const glm::vec3 &pos, const glm::vec3 &normal) {
    vertice v{};
    v.position[0] = pos.x;
    v.position[1] = pos.y;
    v.position[2] = pos.z;

    v.normal[0] = normal.x;
    v.normal[1] = normal.y;
    v.normal[2] = normal.z;

    v.color[0] = 0.25f;
    v.color[1] = 0.4f;
    v.color[2] = 0.0625f;

    return v;
  };

  for (unsigned int z = 0; z < length - 1; ++z) {
    for (unsigned int x = 0; x < width - 1; ++x) {
      float x0 = (static_cast<float>(x) - width * 0.5f) * spacing;
      float x1 = x0 + spacing;

      float z0 = (static_cast<float>(z) - length * 0.5f) * spacing;
      float z1 = z0 + spacing;

      glm::vec3 v00{x0, sampleHeight(x0, z0), z0};
      glm::vec3 v10{x1, sampleHeight(x1, z0), z0};
      glm::vec3 v01{x0, sampleHeight(x0, z1), z1};
      glm::vec3 v11{x1, sampleHeight(x1, z1), z1};

      glm::vec3 n1 = glm::normalize(glm::cross(v01 - v00, v10 - v00));

      terrain.push_back(makeVertex(v00, -n1));
      terrain.push_back(makeVertex(v01, -n1));
      terrain.push_back(makeVertex(v10, -n1));

      glm::vec3 n2 = glm::normalize(glm::cross(v11 - v10, v01 - v10));
      terrain.push_back(makeVertex(v01, n2));
      terrain.push_back(makeVertex(v11, n2));
      terrain.push_back(makeVertex(v10, n2));
    }
  }
  GFVLinstance.meshesToRender.emplace_back(
      GFVL::Mesh(
          GFVLinstance.device,
          GFVL::Mesh::CreateInfo{
              .size = terrain.size() * sizeof(vertice),
              .data = terrain.data(),
              .memoryAllocation = GFVL::VertexBuffer::MemoryAllocation::DeviceOnly}));

  // debug
  uint32_t verticeAmount = 0;
  for (const GFVL::Mesh &mesh : GFVLinstance.meshesToRender) {
    verticeAmount += mesh.size() / sizeof(vertice);
  }
  print("Vertices : " << verticeAmount)

  bool menu = false;
  float speed = 1.0f;

  uint64_t last_time = SDL_GetPerformanceCounter();
  float delta_time = 0.0; // In seconds

  glm::vec3 position(0, 50, -25);
  glm::quat angle;
  while (GFVLinstance.running) {
    uint64_t current_time = SDL_GetPerformanceCounter();
    delta_time = (double)(current_time - last_time) / (double)SDL_GetPerformanceFrequency();
    last_time = current_time;
  
    GFVLinstance.pollInputs();
    //GFVLinstance.uniformBuffer.bind(GFVLinstance.commandBuffer, GFVLinstance.pipeline, 0);
    if (GFVLinstance.inputState.isMouseMoved()) {
      GFVL::MouseState mouseState = GFVLinstance.inputState.getMouseState();

      static float yaw = 0.0f;
      static float pitch = 0.0f;

      float sens = 0.002f;

      yaw -= mouseState.xDelta * sens;
      pitch += mouseState.yDelta * sens;

      pitch = glm::clamp(pitch, -1.5f, 1.5f);

      glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
      glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));

      angle = glm::normalize(qYaw * qPitch);
    }
    
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::ESCAPE) && !GFVLinstance.inputState.isKeyRepeated(GFVL::Keycode::ESCAPE)) {
      menu = !menu;
      SDL_SetWindowRelativeMouseMode(GFVLinstance.window, menu);
    }

    float speed = GFVLinstance.inputState.isKeyDown(GFVL::Keycode::LSHIFT) ? 250.0f : 5.0f;
    glm::vec3 forward = angle * glm::vec3(0, 0, -1);
    glm::vec3 right = angle * glm::vec3(1, 0, 0);

    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::W))
      position += forward * speed * delta_time;
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::SPACE)) {
      lighting.lightPos = position;
      GFVLinstance.uniformBuffer.bindings[1].update(&lighting);
    }
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::S))
      position -= forward * speed * delta_time;
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::A))
      position -= right * speed * delta_time;
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::D))
      position += right * speed * delta_time;

    glm::mat4 proj = glm::perspectiveRH_ZO(
        glm::radians(90.0f),
        GFVLinstance.aspectRatio,
        0.01f,
        100000.0f);

    glm::mat4 view =
        glm::mat4_cast(glm::conjugate(angle)) *
        glm::translate(glm::mat4(1.0f), -position);

    camera.MVP = proj * view;
    camera.viewPos = position;

    GFVLinstance.uniformBuffer.bindings[0].update(&camera);

    GFVLinstance.frame();
  }

  return 0;
}