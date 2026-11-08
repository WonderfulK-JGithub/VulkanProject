#version 450

#pragma region Dependencies
#include "FragmentInputs/CubemapFragmentIn.glsl"
#include "PipelineLayouts/DefaultPipelineLayout.glsl"
#pragma endregion

layout(location = 0) out vec4 outColor;

//layout(set = 0,binding = 0) uniform samplerCube meshTexture[];

void main() {
	outColor = texture(cubemaps[0],fragForwards);
}