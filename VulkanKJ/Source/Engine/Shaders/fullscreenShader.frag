#version 450

#extension GL_EXT_nonuniform_qualifier : require 
#extension GL_GOOGLE_include_directive : require

#pragma region Dependencies
#include "FragmentInputs/FullscreenFragmentIn.glsl"
#include "PipelineLayouts/DefaultPipelineLayout.glsl"
#pragma endregion

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Constants {
	uint baseTextureIndex;
}push;

void main() {
	outColor = texture(textures[push.baseTextureIndex],uv);
}