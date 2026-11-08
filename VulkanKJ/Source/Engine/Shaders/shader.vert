#version 450

#extension GL_GOOGLE_include_directive : require

#include "defines.glsl"

#pragma region Dependencies
#include "VertexInputs/MeshVertex.glsl"
#include "VertexOutputs/RegularVertexOut.glsl"

#include "PipelineLayouts/DefaultPipelineLayout.glsl"
#pragma endregion

void main() {
	gl_Position = cameraData.viewProjection * InstanceData.instances[gl_InstanceIndex].transform * vec4(vertexPosition, 1.0);
	fragColor = vertexColor;
	fragTexCoord = vertexTexCoord;
	fragNormal = normalize((InstanceData.instances[gl_InstanceIndex].transform * vec4(vertexNormal, 0.0)).xyz);
}