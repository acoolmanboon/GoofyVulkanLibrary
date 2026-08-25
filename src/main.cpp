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

#include <GFVL.hpp>
#include "GFVL_core.hpp"
#include "PerlinNoise.hpp"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define print(message) std::cout << message << "\n";
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// Please delete your repository ahh code
constexpr unsigned int width = 700;
constexpr unsigned int length = 700;

// 1 world unit = 1 meter
constexpr float spacing = 5.5f;

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

float getNoise(float x, float y, int octaves, float frequency, float amplitude, float persistence, float lacunarity, float scale, const siv::PerlinNoise &perlinNoise) {
  float totalHeight = 0.0f;

  float currentFrequency = frequency;
  float currentAmplitude = amplitude;

  for (int i = 0; i < octaves; i++) {
    float sampleX = (x / scale) * currentFrequency;
    float sampleY = (y / scale) * currentFrequency;

    totalHeight += perlinNoise.noise2D(sampleX, sampleY) * currentAmplitude;

    currentFrequency *= lacunarity;
    currentAmplitude *= persistence;
  }

  return totalHeight;
}

static float hash2D(int x, int y) {
  uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u;

  h = (h ^ (h >> 13)) * 1274126177u;
  h ^= h >> 16;

  return static_cast<float>(h) / static_cast<float>(UINT32_MAX);
}

static float smoothStep(float edge0, float edge1, float x) {
  x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return x * x * (3.0f - 2.0f * x);
}

float getHeight(float xA, float zA, siv::PerlinNoise perlinNoise) {
  const float x = xA;
  const float z = zA;

  const float terrain =
      getNoise(
          x,
          z,
          4,
          0.7f,
          28.0f,
          0.55f,
          1.9f,
          350.0f,
          perlinNoise);

  const float highlands =
      getNoise(
          x + 1000.0f,
          z + 1000.0f,
          3,
          1.0f,
          55.0f,
          0.55f,
          1.8f,
          900.0f,
          perlinNoise);

  const float detail =
      getNoise(
          x - 500.0f,
          z + 500.0f,
          4,
          1.5f,
          8.0f,
          0.55f,
          2.1f,
          90.0f,
          perlinNoise);

  return terrain + highlands + detail;
}

vertice makeVertice(
    const glm::vec3 &pos,
    const glm::vec3 &normal,
    const siv::PerlinNoise &perlinNoise) {
  vertice v{};

  v.position[0] = pos.x;
  v.position[1] = pos.y;
  v.position[2] = pos.z;

  v.normal[0] = normal.x;
  v.normal[1] = normal.y;
  v.normal[2] = normal.z;

  const float noise = perlinNoise.noise2D(pos.x * 0.045f, pos.z * 0.045f);

  const float slope = 1.0f - std::clamp(normal.y, 0.0f, 1.0f);

  const float variation = noise * 0.055f - slope * 0.035f;

  glm::vec3 color(0.42f + variation, 0.41f + variation, 0.38f + variation);

  if (noise < -0.25f) {
    color *= 0.78f;
  }
  if (noise > 0.35f) {
    color *= 1.08f;
  }

  color.r = std::clamp(color.r, 0.0f, 1.0f);
  color.g = std::clamp(color.g, 0.0f, 1.0f);
  color.b = std::clamp(color.b, 0.0f, 1.0f);

  v.color[0] = color.r;
  v.color[1] = color.g;
  v.color[2] = color.b;

  return v;
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

  std::vector<GFVL::UniformBufferBinding> bindings;
  bindings.reserve(2);
  GFVL::UniformBufferBinding &cameraBinding = bindings.emplace_back(GFVL::UniformBufferBinding{.size = sizeof(CameraUBO),
                                                                                               .binding = 0,
                                                                                               .arrayCount = 1,
                                                                                               .shaderStage = VK_SHADER_STAGE_ALL_GRAPHICS,
                                                                                               .ubo = &camera});

  GFVL::UniformBufferBinding &lightBinding = bindings.emplace_back(GFVL::UniformBufferBinding{.size = sizeof(LightingUBO),
                                                                                               .binding = 1,
                                                                                               .arrayCount = 1,
                                                                                               .shaderStage = VK_SHADER_STAGE_ALL_GRAPHICS,
                                                                                               .ubo = &lighting});

  GFVL::VertexLayout layout;
  layout.addBinding(0, static_cast<uint32_t>(sizeof(vertice)), VK_VERTEX_INPUT_RATE_VERTEX);
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, position), 0);
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, normal), 0);
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, color), 0);

  GFVL::APPLICATION_INFO appInfo = {
      .applicationName = "GoofyVLib example",
      .applicationVersion = 1,
      .width = 800,
      .height = 600,
      .preferredGPU = GFVL::PREFERRED_GPU_POWER_SAVING};

  GFVL::INSTANCE GFVLinstance(appInfo, layout, bindings, shaderStages);

  const siv::PerlinNoise::seed_type seed = 122u;
  const siv::PerlinNoise perlin(seed);

  std::vector<vertice> terrain;
  terrain.reserve((width - 1) * (length - 1) * 6);


  for (unsigned int z = 0; z < length - 1; ++z) {
    for (unsigned int x = 0; x < width - 1; ++x) {
      float x0 = (static_cast<float>(x) - width * 0.5f) * spacing;
      float x1 = x0 + spacing;

      float z0 = (static_cast<float>(z) - length * 0.5f) * spacing;
      float z1 = z0 + spacing;

      glm::vec3 v00{x0, getHeight(x0, z0, perlin), z0};
      glm::vec3 v10{x1, getHeight(x1, z0, perlin), z0};
      glm::vec3 v01{x0, getHeight(x0, z1, perlin), z1};
      glm::vec3 v11{x1, getHeight(x1, z1, perlin), z1};

      glm::vec3 n1 = glm::normalize(glm::cross(v01 - v00, v10 - v00));
      glm::vec3 n2 = glm::normalize(glm::cross(v11 - v10, v01 - v10));

      terrain.push_back(makeVertice(v00, -n1, perlin));
      terrain.push_back(makeVertice(v01, -n1, perlin));
      terrain.push_back(makeVertice(v10, -n1, perlin));

      terrain.push_back(makeVertice(v01, n2, perlin));
      terrain.push_back(makeVertice(v11, n2, perlin));
      terrain.push_back(makeVertice(v10, n2, perlin));
    }
  }

  GFVL::Mesh terrainMesh = GFVLinstance.createMesh( GFVL::Mesh::CreateInfo{
    .verticeDataSize = terrain.size() * sizeof(vertice),
    .verticeCount = static_cast<uint32_t>(terrain.size()),
    .verticeData = terrain.data(),
    .indiceType = GFVL::Mesh::IndiceDataType::NotDefined,
    .memoryAllocation = GFVL::MeshBuffer::MemoryAllocation::DeviceOnly});

  std::vector<vertice> cubeOFDeath;
  insertCube(glm::vec3(12.5f, getHeight(12.5f, -35.0f, perlin) - 5.0f, -35.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(10, 10, 10), cubeOFDeath);
  GFVL::Mesh cubeOfDeathMesh = GFVLinstance.createMesh(GFVL::Mesh::CreateInfo{
    .verticeDataSize = cubeOFDeath.size() * sizeof(vertice),
    .verticeCount = static_cast<uint32_t>(cubeOFDeath.size()), 
    .verticeData = cubeOFDeath.data(), 
    .indiceType = GFVL::Mesh::IndiceDataType::NotDefined,
    .memoryAllocation = GFVL::MeshBuffer::MemoryAllocation::DeviceOnly});

  bool menu = false;
  bool flight = false;
  float speed = 1.0f;

  uint64_t last_time = SDL_GetPerformanceCounter();
  float delta_time = 0.0; // In seconds

  glm::vec3 position(0, 50, -25);
  glm::quat angle;
  while (GFVLinstance.inputState.isRunning()) {
    uint64_t current_time = SDL_GetPerformanceCounter();
    delta_time = (double)(current_time - last_time) / (double)SDL_GetPerformanceFrequency();
    last_time = current_time;

    GFVLinstance.inputState.pollInputs();
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
      GFVLinstance.setMouseLock(menu);
    }

    float speed = GFVLinstance.inputState.isKeyDown(GFVL::Keycode::LSHIFT) ? 250.0f : 5.0f;
    glm::vec3 forward = angle * glm::vec3(0, 0, -1);
    glm::vec3 right = angle * glm::vec3(1, 0, 0);

    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::W))
      position += forward * speed * delta_time;
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::SPACE)) {
      lighting.lightPos = position;
      lightBinding.hasUpdated = true;
    }
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::S))
      position -= forward * speed * delta_time;
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::A))
      position -= right * speed * delta_time;
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::D))
      position += right * speed * delta_time;

    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::V) && !GFVLinstance.inputState.isKeyRepeated(GFVL::Keycode::V)) {
      flight = !flight;
    }

    if (!flight)
      position.y = getHeight(position.x, position.z, perlin) - 1.75;
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

    cameraBinding.hasUpdated = true;
    GFVLinstance.beginFrame();
    GFVLinstance.renderMesh(cubeOfDeathMesh);
    GFVLinstance.renderMesh(terrainMesh);
    GFVLinstance.endFrame();
  }

  return 0;
}