#version 450

layout(binding = 0) uniform UBO {
    mat4 MVP;
    vec3 viewPos;
} ubo;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inPosition;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 lightPos = vec3(100.0, 100.0, 50.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0); // do not go below 0.1f  
    vec3 normal = normalize(inNormal);
    float ambient = 0.05;
    float shininess = 128.0;

    vec3 lightDir   = normalize(lightPos - inPosition);
    vec3 viewDir    = normalize(ubo.viewPos - inPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = lightColor * spec * 0.35;
    float diffuse = max(dot(normal, lightDir),0.0);

    vec3 lighting = ambient * inColor + diffuse * lightColor * inColor + specular;

    //outColor = vec4(inColor, 1.0);
    outColor = vec4(lighting,1.0);
}