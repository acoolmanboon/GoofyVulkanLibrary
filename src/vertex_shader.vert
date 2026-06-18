#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat3 player;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;

void main()
{
    gl_Position = vec4(inPosition, 1.0);
    outColor = vec3(inColor);
}