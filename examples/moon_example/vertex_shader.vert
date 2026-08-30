#version 450

layout(binding = 0) uniform transform {
    mat4 MVP;
    vec3 viewPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outPosition;

void main()
{
    gl_Position = ubo.MVP * vec4(inPosition, 1.0);
    outColor = inColor;
    outNormal = inNormal;
    outPosition = inPosition;
}