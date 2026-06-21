#version 450

layout(binding = 0) uniform UBO {
    vec3 playerPosition;
    vec3 cameraRotation; // x=pitch, y=yaw, z=roll
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;


mat4 rotationX(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return mat4(
        1,0,0,0,
        0,c,-s,0,
        0,s,c,0,
        0,0,0,1
    );
}

mat4 rotationY(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return mat4(
        c,0,s,0,
        0,1,0,0,
        -s,0,c,0,
        0,0,0,1
    );
}

mat4 rotationZ(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return mat4(
        c,-s,0,0,
        s,c,0,0,
        0,0,1,0,
        0,0,0,1
    );
}


void main()
{
    vec3 position = inPosition;

    // Move the world relative to the camera
    position -= ubo.playerPosition;


    // Camera rotation is inverted.
    // Camera turning right means the world moves left.
    mat4 cameraRotation =
        rotationZ(-ubo.cameraRotation.z) *
        rotationX(-ubo.cameraRotation.x) *
        rotationY(-ubo.cameraRotation.y);


    vec3 cameraSpace =
        (cameraRotation * vec4(position, 1.0)).xyz;


    float fov = 2.0;
    float aspect = 800.0 / 600.0;
    float near = 0.1;
    float far = 100.0;

    mat4 projection = mat4(
        fov/aspect, 0, 0, 0,
        0, fov, 0, 0,
        0, 0, -(far+near)/(far-near), -1,
        0, 0, -(2*far*near)/(far-near), 0
    );


    gl_Position = projection * vec4(cameraSpace, 1.0);

    outColor = inColor;
}