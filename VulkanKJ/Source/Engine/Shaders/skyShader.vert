#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)
#extension GL_GOOGLE_include_directive : require

#pragma region Dependencies
#include "VertexOutputs/CubemapVertexOut.glsl"
#include "PipelineLayouts/DefaultPipelineLayout.glsl"
#pragma endregion

const vec2 screen_corners[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0,  1.0),
	vec2( 1.0,  1.0),
	vec2( 1.0, -1.0),
	vec2(-1.0, -1.0)
);

void main() {
	vec2 pos = screen_corners[gl_VertexIndex];
	gl_Position = vec4(pos, 0.0, 1.0);
	fragForwards = normalize(cameraData.forward + pos.x * cameraData.right - pos.y * cameraData.up).xyz;
}