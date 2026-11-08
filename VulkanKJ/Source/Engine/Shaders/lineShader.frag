#version 450

#extension GL_EXT_nonuniform_qualifier : require 

#pragma region Dependencies
#include "FragmentInputs/ColorOnlyFragmentIn.glsl"
#include "PipelineLayouts/DefaultPipelineLayout.glsl"
#pragma endregion

layout(location = 0) out vec4 outColor;

void main() {
	outColor = fragColor;
}