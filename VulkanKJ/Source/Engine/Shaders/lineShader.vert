#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)
#extension GL_GOOGLE_include_directive : require

#pragma region Dependencies
#include "VertexInputs/LineVertex.glsl"
#include "VertexOutputs/ColorOnlyVertexOut.glsl"

#include "PipelineLayouts/DefaultPipelineLayout.glsl"
#pragma endregion

void main() {
	gl_Position = cameraData.viewProjection * InstanceData.instances[gl_InstanceIndex].transform * vec4(vertexPosition, 1.0);
	fragColor = vertexColor;
}