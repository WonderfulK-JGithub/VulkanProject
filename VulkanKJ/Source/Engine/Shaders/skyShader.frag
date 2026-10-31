#version 450

layout(location = 0) in vec3 fragForwards;

layout(location = 0) out vec4 outColor;

layout(set = 1,binding = 0) uniform samplerCube meshTexture[];

void main() {
	outColor = texture(meshTexture[3],fragForwards);
}