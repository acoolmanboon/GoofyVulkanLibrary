// this is just an example script so use it anywhere

#include <cstddef>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <GFVL.hpp>
#include <vulkan/vulkan_core.h>
#include "PerlinNoise.hpp"

// GLM uses OpenGL standards where depth is -1 to 1 but this makes it fit Vulkan specifications.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// Settings for the map
constexpr unsigned int mapWidth = 700; // The width of the map in tiles.
constexpr unsigned int mapLength = 700; // The length of the map in tiles.
constexpr float tileSize = 5.5f; // The "size" of a tile. There isn't any defined real-world size for a tile.

// Actual GFVL stuff

// This defines how your vertices will work in GFVL.
// Later in the script, the structure of vertice will be defined to GFVL.
struct vertice {
  float position[3];
  float normal[3];
  float color[3];
};

// This defines the structure of the data of a Uniform Buffer Object
// UBOs are basically just global values passed to the shader.
// We use two UBOs because the CameraUBO is updated every frame while the LightingUBO updates when the "Space" key is hit.

// alignas is used here because of funny vulkan behaiour idk why but the data has to be aligned
struct CameraUBO {
  glm::mat4 MVP;
  alignas(16) glm::vec3 viewPos;
  float padding;
};

struct LightingUBO {
  alignas(16) glm::vec3 lightPos;
  alignas(16) glm::vec3 lightColor;
};

// This function generates a cube using the vertice structure.
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

// This function is used instead of the perlin noise library as it does not proide lacunarity or persistence.
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

// This just maps a position to a random number.
static float hash2D(int x, int y) {
  uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u;

  h = (h ^ (h >> 13)) * 1274126177u;
  h ^= h >> 16;

  return static_cast<float>(h) / static_cast<float>(UINT32_MAX);
}

// Interpolation function.
static float smoothStep(float edge0, float edge1, float x) {
  x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return x * x * (3.0f - 2.0f * x);
}

// This function returns the heightmap at any given position, by combining multiple perlin noise functions.
float getHeight(float x, float z, siv::PerlinNoise perlinNoise) {
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

// This function takes the data generated by other functions and generates a vertice.
vertice makeVertice(const glm::vec3 &pos, const glm::vec3 &normal, const siv::PerlinNoise &perlinNoise) {
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
  // This is where you tell GFVL where your shaders are. You need to compile the shader languages first.
  // I know that there are more shader stages available but for now I haven't learned that yet so I am only certain that it supports only a vertex and fragment shader for now.
  std::vector<GFVL::SHADER_STAGE> shaderStages = {
      {.flags = VK_SHADER_STAGE_VERTEX_BIT,
       .filename = "vertex_shader.spv"},
      {.flags = VK_SHADER_STAGE_FRAGMENT_BIT,
       .filename = "fragment_shader.spv"}};

  // These are simply your data containers for the UBO data.
  CameraUBO camera = {
    .MVP = glm::mat4(0.0f),
    .viewPos = glm::vec3(0.0f)
  };
  LightingUBO lighting = {
    .lightPos = glm::vec3(0.0f, 0.0f, 0.0f), 
    .lightColor = glm::vec3(1.0f, 1.0f, 1.0f)
  };

  // After creating the CPU-side data containers, we define the GFVL implementation to use the bindings.
  std::vector<GFVL::UniformBufferBinding> bindings;
  bindings.reserve(2);
  GFVL::UniformBufferBinding &cameraBinding = bindings.emplace_back(GFVL::UniformBufferBinding{.size = sizeof(CameraUBO),
                                                                                               .binding = 0, // The binding to pass to the shader.
                                                                                               .arrayCount = 1, // This is used when the UBO is an array.
                                                                                               .shaderStage = VK_SHADER_STAGE_ALL_GRAPHICS, // This makes the UBO available to all graphics shaders.
                                                                                               .ubo = &camera}); // Link your actual CPU-sided ubo buffer.

  GFVL::UniformBufferBinding &lightBinding = bindings.emplace_back(GFVL::UniformBufferBinding{.size = sizeof(LightingUBO),
                                                                                               .binding = 1,
                                                                                               .arrayCount = 1,
                                                                                               .shaderStage = VK_SHADER_STAGE_FRAGMENT_BIT, // However, it's recommended to restrict the shader stage only to where it's actually used for optimization.
                                                                                               .ubo = &lighting});

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
        .location = offsetof(vertice, position),
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT
      },
      {
        .location = offsetof(vertice, normal),
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT
      },
      {
        .location = offsetof(vertice, color),
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT
      }
    }
  };
  
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
  terrain.reserve((mapWidth - 1) * (mapLength - 1) * 6);


  for (unsigned int z = 0; z < mapLength - 1; ++z) {
    for (unsigned int x = 0; x < mapWidth - 1; ++x) {
      float x0 = (static_cast<float>(x) - mapWidth * 0.5f) * tileSize;
      float x1 = x0 + tileSize;

      float z0 = (static_cast<float>(z) - mapLength * 0.5f) * tileSize;
      float z1 = z0 + tileSize;

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

    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::SPACE)) {
      lighting.lightPos = position;
      lightBinding.hasUpdated = true;
    }

        if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::W) || GFVLinstance.inputState.isKeyDown(GFVL::Keycode::UP))
      position += forward * speed * delta_time;
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::S) || GFVLinstance.inputState.isKeyDown(GFVL::Keycode::DOWN) )
      position -= forward * speed * delta_time;
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::A) || GFVLinstance.inputState.isKeyDown(GFVL::Keycode::LEFT))
      position -= right * speed * delta_time;
    if (GFVLinstance.inputState.isKeyDown(GFVL::Keycode::D) || GFVLinstance.inputState.isKeyDown(GFVL::Keycode::RIGHT))
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