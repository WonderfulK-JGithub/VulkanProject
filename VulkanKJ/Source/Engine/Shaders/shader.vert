#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)

layout(set = 0,binding = 0) uniform UBO
{
	mat4 view;
	mat4 projection;
	mat4 viewProjection;
} cameraData;

struct ModelInstance
{
	mat4 transform;
};

layout(std140,set = 0, binding = 1) readonly buffer storageBuffer
{
	ModelInstance instances[];
} InstanceData;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;

void main() {
	gl_Position = cameraData.viewProjection * InstanceData.instances[gl_InstanceIndex].transform * vec4(vertexPosition, 1.0);
	fragColor = vertexColor;
	fragTexCoord = vertexTexCoord;
	fragNormal = normalize((InstanceData.instances[gl_InstanceIndex].transform * vec4(vertexNormal, 0.0)).xyz);
}