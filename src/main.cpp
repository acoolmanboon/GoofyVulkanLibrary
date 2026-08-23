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
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define print(message) std::cout << message << "\n";
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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

  GFVL::VertexLayout layout(sizeof(vertice));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, position));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, normal));
  layout.addAttribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertice, color));

  GFVL::APPLICATION_INFO appInfo = {
      .applicationName = "GoofyVLib example",
      .applicationVersion = 1,
      .width = 600,
      .height = 800,
      .preferredGPU = GFVL::PREFERRED_GPU_PERFORMANCE};

  GFVL::INSTANCE GFVLinstance(appInfo, layout, bindings, shaderStages);

  // Please delete your repository ahh code
  constexpr unsigned int width = 200;
  constexpr unsigned int length = 200;

  // 1 world unit = 1 meter
  constexpr float spacing = 5.5f;
  constexpr float scale = 4.0f;

  constexpr float continentFrequency = 0.00225f; // ~2km features
  constexpr float mountainFrequency = 0.00015f;  // ~650m mountain systems
  constexpr float detailFrequency = 0.012f;      // ~100m detail
  constexpr float colorFrequency = 0.002f;       // very large color regions

  constexpr float continentAmplitude = -4.40f;
  constexpr float mountainAmplitude = -6.20f;
  constexpr float detailAmplitude = -3.3f;

  constexpr float persistence = 0.25f;

  const siv::PerlinNoise::seed_type seed = 122u;
  const siv::PerlinNoise perlin(seed);

  std::vector<vertice> terrain;
  terrain.reserve((width - 1) * (length - 1) * 6);

  auto sampleHeight = [&](float xa, float za) -> float {
    float x = xa * scale;
    float z = za * scale;

    double continents =
        perlin.octave2D(
            x * continentFrequency,
            z * continentFrequency,
            1,
            persistence);

    double ridges =
        perlin.octave2D_01(
            x * mountainFrequency,
            z * mountainFrequency,
            6,
            0.67) *
        mountainAmplitude;

    ridges = powf(ridges, 3.0f);
    // if (ridges < 0.4) ridges = 0;

    double detail =
        perlin.octave2D(
            x * detailFrequency,
            z * detailFrequency,
            4,
            0.45);

    double craterNoise =
        perlin.octave2D_01(
            (x + 100) * 0.0035,
            (z + 100) * 0.0035,
            2,
            0.6);

    craterNoise = std::pow(craterNoise, 5.0);

    float h =
        static_cast<float>(
            continents * continentAmplitude +
            ridges +
            detail * detailAmplitude +
            craterNoise * 60);

    return h;
  };

  auto makeVertex = [&](const glm::vec3 &pos, const glm::vec3 &normal) {
    vertice v{};

    v.position[0] = pos.x;
    v.position[1] = pos.y;
    v.position[2] = pos.z;

    v.normal[0] = normal.x;
    v.normal[1] = normal.y;
    v.normal[2] = normal.z;

    float biome =
        static_cast<float>(
            perlin.octave2D(
                pos.x * colorFrequency,
                pos.z * colorFrequency,
                4,
                0.55));

    biome = glm::clamp(biome, 0.0f, 1.0f);

    glm::vec3 darkBasalt(0.20f, 0.20f, 0.22f);
    glm::vec3 basalt(0.34f, 0.34f, 0.36f);
    glm::vec3 lightRock(0.55f, 0.55f, 0.58f);

    glm::vec3 color;

    if (biome < 0.35f) {
      float t = biome / 0.35f;
      color = glm::mix(darkBasalt, basalt, t);
    } else {
      float t = (biome - 0.35f) / 0.65f;
      color = glm::mix(basalt, lightRock, t);
    }

    float elevation =
        glm::clamp(pos.y / 200.0f, 0.0f, 1.0f);

    color += glm::vec3(elevation * 0.08f);

    float frost =
        glm::smoothstep(
            170.0f,
            215.0f,
            pos.y);

    color = glm::mix(
        color,
        glm::vec3(0.93f, 0.93f, 0.95f),
        frost);

    float grain =
        static_cast<float>(
            perlin.octave2D(
                pos.x * 0.03,
                pos.z * 0.03,
                2,
                0.5));

    grain = (grain - 0.5f) * 0.08f;

    color += glm::vec3(grain);

    color = glm::clamp(color, glm::vec3(0.0f), glm::vec3(1.0f));

    v.color[0] = color.r;
    v.color[1] = color.g;
    v.color[2] = color.b;

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
      glm::vec3 n2 = glm::normalize(glm::cross(v11 - v10, v01 - v10));

      terrain.push_back(makeVertex(v00, -n1));
      terrain.push_back(makeVertex(v01, -n1));
      terrain.push_back(makeVertex(v10, -n1));

      terrain.push_back(makeVertex(v01, n2));
      terrain.push_back(makeVertex(v11, n2));
      terrain.push_back(makeVertex(v10, n2));
    }
  }

  GFVL::Mesh &terrainMesh = GFVLinstance.createMesh( GFVL::Mesh::CreateInfo{
    .verticeDataSize = terrain.size() * sizeof(vertice),
    .verticeCount = static_cast<uint32_t>(terrain.size()),
    .verticeData = terrain.data(),
    .indiceType = GFVL::Mesh::IndiceDataType::NotDefined,
    .memoryAllocation = GFVL::MeshBuffer::MemoryAllocation::DeviceOnly});

  std::vector<vertice> cubeOFDeath;
  insertCube(glm::vec3(12.5f, sampleHeight(12.5f, -35.0f) - 5.0f, -35.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(10, 10, 10), cubeOFDeath);
  GFVL::Mesh &cubeOfDeathMesh = GFVLinstance.createMesh(GFVL::Mesh::CreateInfo{
    .verticeDataSize = cubeOFDeath.size() * sizeof(vertice),
    .verticeCount = static_cast<uint32_t>(cubeOFDeath.size()), 
    .verticeData = cubeOFDeath.data(), 
    .indiceType = GFVL::Mesh::IndiceDataType::NotDefined,
    .memoryAllocation = GFVL::MeshBuffer::MemoryAllocation::DeviceOnly});

  // debug
  uint32_t verticeAmount = 0;
  for (const GFVL::Mesh &mesh : GFVLinstance.meshesToRender) {
    verticeAmount += mesh.size() / sizeof(vertice);
  }
  print("Vertices : " << verticeAmount)

  bool menu = false;
  bool flight = false;
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
      position.y = sampleHeight(position.x, position.z) - 1.75;
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
    GFVLinstance.frame();
  }

  return 0;
}