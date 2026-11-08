layout(set = 0,binding = 0) uniform sampler2D textures[];
layout(set = 0,binding = 1) uniform samplerCube cubemaps[];

layout(set = 1,binding = 0) uniform UBO
{
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec4 right;
    vec4 up;
    vec4 forward;
} cameraData;

struct ModelInstance
{
    mat4 transform;
};

layout(std140,set = 2, binding = 0) readonly buffer storageBuffer
{
    ModelInstance instances[];
} InstanceData;

