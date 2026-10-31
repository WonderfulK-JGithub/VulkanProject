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
layout(location = 1) in vec4 vertexColor;

layout(location = 0) out vec4 fragColor;

void main() {
	gl_Position = cameraData.viewProjection * InstanceData.instances[gl_InstanceIndex].transform * vec4(vertexPosition, 1.0);
	fragColor = vertexColor;
}